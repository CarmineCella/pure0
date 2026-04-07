// sampsynth.h — reduced orchestral sample database + inst-gran layer for pure0
#ifndef SAMPSYNTH_H
#define SAMPSYNTH_H

#include "core.h"
#include "dsp.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <random>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// -----------------------------------------------------------------------------
// helpers
// -----------------------------------------------------------------------------

static std::mt19937& rng() {
    static std::mt19937 rng{std::random_device{}()};
    return rng;
}

static double urand(double a, double b) {
    if (b < a) std::swap(a, b);
    std::uniform_real_distribution<double> dist(a, b);
    return dist(rng());
}

static std::string to_lower(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

static bool is_wav_path(const std::filesystem::path& p) {
    if (!p.has_extension()) return false;
    return to_lower(p.extension().string()) == ".wav";
}

static std::vector<std::string> split(const std::string& s, char sep) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == sep) {
            out.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    out.push_back(cur);
    return out;
}

static ExprPtr kv(const std::string& key, const ExprPtr& value) {
    Expr::List kvs;
    kvs.push_back(make_symbol(key));
    kvs.push_back(value);
    return make_list(kvs);
}

static ExprPtr entry_make(const std::string& family,
                          const std::string& instrument,
                          const std::string& articulation,
                          const std::string& pitch,
                          int midi,
                          const std::string& dynamic,
                          const std::string& path) {
    Expr::List entry;
    entry.push_back(kv("family",       make_string(family)));
    entry.push_back(kv("instrument",   make_string(instrument)));
    entry.push_back(kv("articulation", make_string(articulation)));
    entry.push_back(kv("pitch",        make_string(pitch)));
    entry.push_back(kv("midi",         make_scalar((double)midi)));
    entry.push_back(kv("dynamic",      make_string(dynamic)));
    entry.push_back(kv("path",         make_string(path)));
    return make_list(entry);
}

static bool entry_has_key(const ExprPtr& entry, const std::string& key) {
    if (!is_list(entry)) return false;
    const auto& xs = std::get<Expr::List>(entry->v);
    for (const auto& item : xs) {
        if (!is_list(item)) continue;
        const auto& pair = std::get<Expr::List>(item->v);
        if (pair.size() != 2) continue;
        if (is_symbol(pair[0]) && as_symbol(pair[0]) == key) return true;
    }
    return false;
}

static ExprPtr entry_get(const ExprPtr& entry, const std::string& key) {
    if (!is_list(entry)) throw std::runtime_error("db entry must be a list");
    const auto& xs = std::get<Expr::List>(entry->v);
    for (const auto& item : xs) {
        if (!is_list(item)) continue;
        const auto& pair = std::get<Expr::List>(item->v);
        if (pair.size() != 2) continue;
        if (is_symbol(pair[0]) && as_symbol(pair[0]) == key) return pair[1];
    }
    throw std::runtime_error("db entry missing key: " + key);
}

static std::string entry_get_string(const ExprPtr& entry, const std::string& key) {
    ExprPtr x = entry_get(entry, key);
    if (is_string(x)) return as_string(x);
    if (is_symbol(x)) return as_symbol(x);
    return to_string_value(x);
}

static int entry_get_int(const ExprPtr& entry, const std::string& key) {
    return (int)std::lround(as_scalar(entry_get(entry, key)));
}

static int pitch_class(const std::string& name) {
    const std::string s = to_lower(name);
    if (s == "c")  return 0;
    if (s == "c#" || s == "db") return 1;
    if (s == "d")  return 2;
    if (s == "d#" || s == "eb") return 3;
    if (s == "e")  return 4;
    if (s == "f")  return 5;
    if (s == "f#" || s == "gb") return 6;
    if (s == "g")  return 7;
    if (s == "g#" || s == "ab") return 8;
    if (s == "a")  return 9;
    if (s == "a#" || s == "bb") return 10;
    if (s == "b" || s == "cb")  return 11;
    throw std::runtime_error("cannot parse pitch class: " + name);
}

static int pitch_to_midi(const std::string& pitch) {
    if (pitch.empty()) throw std::runtime_error("empty pitch");
    std::string pc;
    std::string oct;
    pc.push_back(pitch[0]);
    std::size_t i = 1;
    if (i < pitch.size() && (pitch[i] == '#' || pitch[i] == 'b')) {
        pc.push_back(pitch[i]);
        ++i;
    }
    while (i < pitch.size()) {
        oct.push_back(pitch[i]);
        ++i;
    }
    if (oct.empty()) throw std::runtime_error("pitch has no octave: " + pitch);
    int octave = std::stoi(oct);
    int pclass = pitch_class(pc);
    return 12 * (octave + 1) + pclass;
}

static bool parse_sol_filename(const std::string& stem,
                               std::string& instrument,
                               std::string& articulation,
                               std::string& pitch,
                               std::string& dynamic) {
    const std::vector<std::string> parts = split(stem, '-');
    if (parts.size() < 4) return false;
    instrument   = parts[0];
    articulation = parts[1];
    pitch        = parts[2];
    dynamic      = parts[3];
    return !(instrument.empty() || articulation.empty() || pitch.empty() || dynamic.empty());
}

static Vector mix_overlay(const std::vector<std::pair<int, Vector>>& events) {
    std::size_t out_len = 0;
    for (const auto& ev : events) {
        if (ev.first < 0) throw std::runtime_error("negative mix offset");
        out_len = std::max(out_len, (std::size_t)ev.first + ev.second.size());
    }
    Vector out(0.0, out_len);
    for (const auto& ev : events) {
        for (std::size_t i = 0; i < ev.second.size(); ++i)
            out[(std::size_t)ev.first + i] += ev.second[i];
    }
    return out;
}

static Vector mono_fold(const Vector& interleaved, int nch) {
    if (nch <= 1) return interleaved;
    if (interleaved.size() % (std::size_t)nch != 0)
        throw std::runtime_error("wav data size not divisible by channel count");
    const std::size_t frames = interleaved.size() / (std::size_t)nch;
    Vector mono(frames);
    for (std::size_t f = 0; f < frames; ++f) {
        double acc = 0.0;
        for (int ch = 0; ch < nch; ++ch)
            acc += interleaved[f * (std::size_t)nch + (std::size_t)ch];
        mono[f] = acc / (double)nch;
    }
    return mono;
}

static Vector read_wav_signal(const std::string& path, int* out_sr = nullptr, int* out_nch = nullptr) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("wavread: cannot open " + path);

    char id4[4];
    f.read(id4, 4);
    if (std::memcmp(id4, "RIFF", 4)) throw std::runtime_error("wavread: not a RIFF file");
    wav_read_le32(f);
    f.read(id4, 4);
    if (std::memcmp(id4, "WAVE", 4)) throw std::runtime_error("wavread: not a WAVE file");

    int sr = 44100, nch = 1, bits = 16;
    bool got_data = false;
    std::vector<double> samples;

    while (f) {
        char chunk[4];
        f.read(chunk, 4);
        if (!f) break;
        uint32_t sz = wav_read_le32(f);

        if (!std::memcmp(chunk, "fmt ", 4)) {
            wav_read_le16(f);
            nch  = (int)wav_read_le16(f);
            sr   = (int)wav_read_le32(f);
            wav_read_le32(f);
            wav_read_le16(f);
            bits = (int)wav_read_le16(f);
            if (sz > 16) f.seekg(sz - 16, std::ios::cur);
        } else if (!std::memcmp(chunk, "data", 4)) {
            got_data = true;
            if (bits == 8) {
                samples.resize(sz);
                for (uint32_t i = 0; i < sz; ++i) {
                    uint8_t u = (uint8_t)f.get();
                    samples[i] = ((double)u - 128.0) / 128.0;
                }
            } else if (bits == 16) {
                const std::size_t n = sz / 2;
                samples.resize(n);
                for (std::size_t i = 0; i < n; ++i) {
                    int16_t s = (int16_t)wav_read_le16(f);
                    samples[i] = (double)s / 32768.0;
                }
            } else if (bits == 24) {
                const std::size_t n = sz / 3;
                samples.resize(n);
                for (std::size_t i = 0; i < n; ++i) {
                    uint8_t b0 = (uint8_t)f.get();
                    uint8_t b1 = (uint8_t)f.get();
                    uint8_t b2 = (uint8_t)f.get();
                    int32_t v = (int32_t)(b0 | (b1 << 8) | (b2 << 16));
                    if (v & 0x800000) v |= ~0xFFFFFF;
                    samples[i] = (double)v / 8388608.0;
                }
            } else {
                throw std::runtime_error("wavread: unsupported bit depth");
            }
            if (sz & 1u) f.get();
        } else {
            f.seekg(sz + (sz & 1u), std::ios::cur);
        }
    }

    if (!got_data) throw std::runtime_error("wavread: no data chunk in " + path);

    Vector sig(samples.size());
    for (std::size_t i = 0; i < samples.size(); ++i) sig[i] = samples[i];
    if (out_sr)  *out_sr  = sr;
    if (out_nch) *out_nch = nch;
    return sig;
}

static Vector load_mono_resampled(const std::string& path, int target_sr = 0) {
    int src_sr = 0, nch = 1;
    Vector sig = read_wav_signal(path, &src_sr, &nch);
    sig = mono_fold(sig, nch);
    if (target_sr > 0 && src_sr > 0 && src_sr != target_sr)
        sig = resample(sig, (double)target_sr / (double)src_sr);
    return sig;
}

static Vector extract_db_signal(const ExprPtr& x, int target_sr = 0) {
    if (is_string(x)) return load_mono_resampled(as_string(x), target_sr);
    if (is_list(x) && entry_has_key(x, "path"))
        return load_mono_resampled(entry_get_string(x, "path"), target_sr);
    throw std::runtime_error("expected db entry or string path");
}

static std::vector<ExprPtr> db_entries(const ExprPtr& db) {
    if (!is_list(db)) throw std::runtime_error("database must be a list");
    return std::get<Expr::List>(db->v);
}

static Expr::List unique_field_values(const ExprPtr& db, const std::string& key) {
    const auto xs = db_entries(db);
    std::vector<std::string> names;
    names.reserve(xs.size());
    for (const auto& e : xs) {
        try { names.push_back(entry_get_string(e, key)); } catch (...) {}
    }
    std::sort(names.begin(), names.end());
    names.erase(std::unique(names.begin(), names.end()), names.end());
    Expr::List out;
    for (const auto& s : names) out.push_back(make_string(s));
    return out;
}

// -----------------------------------------------------------------------------
// db creation / querying / loading
// -----------------------------------------------------------------------------

static Proc fn_db_loadscan() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1 || !is_string(args[0]))
            throw std::runtime_error("db-load expects: root");
        const std::filesystem::path root = as_string(args[0]);
        if (!std::filesystem::exists(root))
            throw std::runtime_error("db-load: root does not exist");

        Expr::List db;
        for (const auto& de : std::filesystem::recursive_directory_iterator(root)) {
            if (!de.is_regular_file()) continue;
            const auto& p = de.path();
            if (!is_wav_path(p)) continue;

            std::string instrument, articulation, pitch, dynamic;
            if (!parse_sol_filename(p.stem().string(), instrument, articulation, pitch, dynamic))
                continue;

            int midi = 0;
            try { midi = pitch_to_midi(pitch); } catch (...) { continue; }

            std::string family = "";
            try {
                auto rel = std::filesystem::relative(p, root);
                auto it = rel.begin();
                if (it != rel.end()) family = it->string();
            } catch (...) {}

            db.push_back(entry_make(
                family, instrument, articulation, pitch, midi, dynamic,
                std::filesystem::absolute(p).string()
            ));
        }
        return make_list(db);
    };
}

static Proc fn_db_query() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 2 || !is_string(args[1]))
            throw std::runtime_error("db-query expects: catalog regex");
        const auto xs = db_entries(args[0]);
        const std::regex re(as_string(args[1]));
        Expr::List out;
        for (const auto& e : xs) {
            if (std::regex_search(to_string_value(e), re))
                out.push_back(e);
        }
        return make_list(out);
    };
}

static Proc fn_db_pickload() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.empty() || args.size() > 2)
            throw std::runtime_error("db-pick expects: entry-or-path [target_sr]");
        int target_sr = (args.size() == 2) ? (int)std::lround(as_scalar(args[1])) : 0;
        return make_vec(extract_db_signal(args[0], target_sr));
    };
}

static Proc fn_db_instruments() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("db-instruments expects: catalog");
        return make_list(unique_field_values(args[0], "instrument"));
    };
}

static Proc fn_db_playingstyles() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("db-playingstyles expects: catalog");
        return make_list(unique_field_values(args[0], "articulation"));
    };
}

static Proc fn_db_dynamics() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("db-dynamics expects: catalog");
        return make_list(unique_field_values(args[0], "dynamic"));
    };
}

static Proc fn_db_pitches() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("db-pitches expects: catalog");
        return make_list(unique_field_values(args[0], "pitch"));
    };
}

// -----------------------------------------------------------------------------
// continuous schedule sampling helpers
// -----------------------------------------------------------------------------

static double clamp01(double x) {
    if (x < 0.0) return 0.0;
    if (x > 1.0) return 1.0;
    return x;
}

static double continuous_pos(double t, double total_dur, std::size_t n) {
    if (n <= 1 || total_dur <= 0.0) return 0.0;
    double u = clamp01(t / total_dur);
    return u * (double)(n - 1);
}

static double sample_scalar_curve(const ExprPtr& curve, double t, double total_dur) {
    Vector v = as_vec(curve);
    if (v.size() == 0) throw std::runtime_error("inst-gran: empty scalar curve");
    if (v.size() == 1) return v[0];
    double p = continuous_pos(t, total_dur, v.size());
    std::size_t i0 = (std::size_t)std::floor(p);
    std::size_t i1 = std::min<std::size_t>(i0 + 1, v.size() - 1);
    double u = p - (double)i0;
    return v[i0] + (v[i1] - v[i0]) * u;
}

static Vector sample_vec_schedule(const ExprPtr& sched, double t, double total_dur) {
    if (!is_list(sched)) throw std::runtime_error("inst-gran: expected list of vectors");
    const auto& xs = std::get<Expr::List>(sched->v);
    if (xs.empty()) throw std::runtime_error("inst-gran: empty vector schedule");
    if (xs.size() == 1) return as_vec(xs[0]);

    double p = continuous_pos(t, total_dur, xs.size());
    std::size_t i0 = (std::size_t)std::floor(p);
    std::size_t i1 = std::min<std::size_t>(i0 + 1, xs.size() - 1);
    double u = p - (double)i0;

    Vector a = as_vec(xs[i0]);
    Vector b = as_vec(xs[i1]);
    std::size_t n = std::max<std::size_t>(a.size(), b.size());
    a = broadcast(a, n);
    b = broadcast(b, n);

    Vector out(n);
    for (std::size_t i = 0; i < n; ++i)
        out[i] = a[i] + (b[i] - a[i]) * u;
    return out;
}

static std::vector<std::string> sample_stringlist_schedule(const ExprPtr& sched, double t, double total_dur) {
    if (!is_list(sched)) throw std::runtime_error("inst-gran: expected list of lists");
    const auto& xs = std::get<Expr::List>(sched->v);
    if (xs.empty()) throw std::runtime_error("inst-gran: empty list schedule");
    if (xs.size() == 1) {
        if (!is_list(xs[0])) throw std::runtime_error("inst-gran: schedule item must be a list");
        const auto& ys = std::get<Expr::List>(xs[0]->v);
        std::vector<std::string> out;
        for (const auto& y : ys) out.push_back(to_string_value(y));
        return out;
    }

    double p = continuous_pos(t, total_dur, xs.size());
    std::size_t idx = (std::size_t)std::llround(p);
    if (idx >= xs.size()) idx = xs.size() - 1;

    if (!is_list(xs[idx])) throw std::runtime_error("inst-gran: schedule item must be a list");
    const auto& ys = std::get<Expr::List>(xs[idx]->v);
    std::vector<std::string> out;
    for (const auto& y : ys) out.push_back(to_string_value(y));
    return out;
}

static std::vector<std::vector<std::string>> parse_orchestra(const ExprPtr& orchestra) {
    if (!is_list(orchestra)) throw std::runtime_error("inst-gran: orchestra must be a list");
    const auto& xs = std::get<Expr::List>(orchestra->v);
    std::vector<std::vector<std::string>> groups;
    if (xs.empty()) return groups;

    if (is_string(xs[0]) || is_symbol(xs[0])) {
        for (const auto& x : xs) {
            if (!(is_string(x) || is_symbol(x)))
                throw std::runtime_error("inst-gran: mixed orchestra syntax");
            groups.push_back({to_string_value(x)});
        }
        return groups;
    }

    for (const auto& g : xs) {
        if (!is_list(g)) throw std::runtime_error("inst-gran: each voice must be a list");
        const auto& ys = std::get<Expr::List>(g->v);
        std::vector<std::string> voice;
        for (const auto& y : ys) {
            if (!(is_string(y) || is_symbol(y)))
                throw std::runtime_error("inst-gran: instruments must be strings/symbols");
            voice.push_back(to_string_value(y));
        }
        groups.push_back(voice);
    }
    return groups;
}

static ExprPtr make_event(double t,
                          double dur,
                          int midi,
                          const std::string& style,
                          const std::string& dyn,
                          const std::vector<std::string>& instruments,
                          const std::vector<std::string>& paths) {
    Expr::List e;
    Expr::List ins;
    for (const auto& s : instruments) ins.push_back(make_string(s));
    Expr::List pths;
    for (const auto& s : paths) pths.push_back(make_string(s));
    e.push_back(kv("time",        make_scalar(t)));
    e.push_back(kv("dur",         make_scalar(dur)));
    e.push_back(kv("midi",        make_scalar((double)midi)));
    e.push_back(kv("style",       make_string(style)));
    e.push_back(kv("dynamic",     make_string(dyn)));
    e.push_back(kv("instruments", make_list(ins)));
    e.push_back(kv("paths",       make_list(pths)));
    return make_list(e);
}

// -----------------------------------------------------------------------------
// orchgran
// -----------------------------------------------------------------------------

static Proc fn_orchgran() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 13)
            throw std::runtime_error("inst-gran expects: sr dur orchestra density rand-density length rand-length octave-range rand-octave chords styles dynamics db");

        const double sr = as_scalar(args[0]);
        const double total_dur = as_scalar(args[1]);
        const auto voices = parse_orchestra(args[2]);

        const ExprPtr density_curve      = args[3];
        const ExprPtr rand_density_curve = args[4];
        const ExprPtr length_curve       = args[5];
        const ExprPtr rand_length_curve  = args[6];
        const ExprPtr octave_range_curve = args[7];
        const ExprPtr rand_octave_curve  = args[8];
        const ExprPtr chord_schedule     = args[9];
        const ExprPtr style_schedule     = args[10];
        const ExprPtr dynamic_schedule   = args[11];
        const auto db = db_entries(args[12]);

        if (voices.empty()) return make_list({make_vec(Vector(0.0, 0)), make_list({})});

        std::vector<double> free_until(voices.size(), 0.0);
        std::vector<std::pair<int, Vector>> audio_events;
        Expr::List event_log;

        double t = 0.0;
        while (t < total_dur) {
            const double density = std::max(1e-6, sample_scalar_curve(density_curve, t, total_dur));
            const double rand_density = std::max(0.0, sample_scalar_curve(rand_density_curve, t, total_dur));
            double dt = 1.0 / density;
            dt *= 1.0 + urand(-rand_density, rand_density);
            if (dt <= 1e-4) dt = 1e-4;

            const double rand_len = std::max(0.0, sample_scalar_curve(rand_length_curve, t, total_dur));
            const double requested_len = std::max(0.01, sample_scalar_curve(length_curve, t, total_dur) *
                                                         (1.0 + urand(-rand_len, rand_len)));

            const Vector octave_range = sample_vec_schedule(octave_range_curve, t, total_dur);
            const double rand_oct = std::max(0.0, sample_scalar_curve(rand_octave_curve, t, total_dur));
            const Vector chord = sample_vec_schedule(chord_schedule, t, total_dur);
            const auto styles = sample_stringlist_schedule(style_schedule, t, total_dur);
            const auto dyns = sample_stringlist_schedule(dynamic_schedule, t, total_dur);

            if (chord.size() > 0 && !styles.empty() && !dyns.empty()) {
                std::vector<int> available;
                for (std::size_t i = 0; i < voices.size(); ++i)
                    if (free_until[i] <= t) available.push_back((int)i);

                if (!available.empty()) {
                    std::uniform_int_distribution<int> voice_dist(0, (int)available.size() - 1);
                    const int chosen_voice = available[(std::size_t)voice_dist(rng())];

                    std::uniform_int_distribution<int> chord_dist(0, (int)chord.size() - 1);
                    int midi = (int)std::lround(chord[(std::size_t)chord_dist(rng())]);

                    const int oct_lo = (octave_range.size() >= 1) ? (int)std::lround(octave_range[0]) : 3;
                    const int oct_hi = (octave_range.size() >= 2) ? (int)std::lround(octave_range[1]) : oct_lo;
                    const int oct_span = std::max(0, oct_hi - oct_lo);
                    midi += 12 * (oct_lo + (oct_span > 0 ? (int)std::floor(urand(0.0, (double)oct_span + 0.999)) : 0));
                    midi += 12 * (int)std::lround(urand(-rand_oct, rand_oct));

                    std::uniform_int_distribution<int> style_dist(0, (int)styles.size() - 1);
                    std::uniform_int_distribution<int> dyn_dist(0, (int)dyns.size() - 1);
                    const std::string style = styles[(std::size_t)style_dist(rng())];
                    const std::string dyn = dyns[(std::size_t)dyn_dist(rng())];

                    std::vector<std::pair<int, Vector>> local;
                    std::vector<std::string> paths_used;

                    for (const auto& inst : voices[(std::size_t)chosen_voice]) {
                        int best_dist = std::numeric_limits<int>::max();
                        ExprPtr best = nullptr;
                        for (const auto& e : db) {
                            try {
                                if (entry_get_string(e, "instrument") != inst) continue;
                                if (entry_get_string(e, "articulation") != style) continue;
                                if (entry_get_string(e, "dynamic") != dyn) continue;
                                int m = entry_get_int(e, "midi");
                                int d = std::abs(m - midi);
                                if (!best || d < best_dist) {
                                    best = e;
                                    best_dist = d;
                                }
                            } catch (...) {}
                        }
                        if (!best) continue;

                        Vector sig = extract_db_signal(best, (int)std::lround(sr));
                        int want = std::max(1, (int)std::lround(requested_len * sr));
                        if ((int)sig.size() > want) sig = take(sig, (std::size_t)want);
                        local.push_back({(int)std::lround(t * sr), sig});
                        paths_used.push_back(entry_get_string(best, "path"));
                    }

                    if (!local.empty()) {
                        for (const auto& ev : local) audio_events.push_back(ev);
                        event_log.push_back(make_event(
                            t, requested_len, midi, style, dyn,
                            voices[(std::size_t)chosen_voice], paths_used
                        ));
                        free_until[(std::size_t)chosen_voice] = t + requested_len;
                    }
                }
            }

            t += dt;
        }

        Expr::List out;
        out.push_back(make_vec(mix_overlay(audio_events)));
        out.push_back(make_list(event_log));
        return make_list(out);
    };
}

static Proc fn_orchgran_render() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1 || !is_list(args[0]))
            throw std::runtime_error("inst-gran-render expects: inst-gran-result");
        const auto& xs = std::get<Expr::List>(args[0]->v);
        if (xs.size() != 2) throw std::runtime_error("inst-gran-render: malformed result");
        return xs[0];
    };
}

static Proc fn_orchgran_score() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1 || !is_list(args[0]))
            throw std::runtime_error("inst-gran-score expects: inst-gran-result");
        const auto& xs = std::get<Expr::List>(args[0]->v);
        if (xs.size() != 2) throw std::runtime_error("inst-gran-score: malformed result");
        return xs[1];
    };
}

// -----------------------------------------------------------------------------
// registration
// -----------------------------------------------------------------------------

static void add_sampsynth(std::shared_ptr<Env> env) {
    env->set("db-load",            make_proc(fn_db_loadscan()));
    env->set("db-query",           make_proc(fn_db_query()));
    env->set("db-pick",            make_proc(fn_db_pickload()));
    env->set("db-instruments",     make_proc(fn_db_instruments()));
    env->set("db-playingstyles",   make_proc(fn_db_playingstyles()));
    env->set("db-dynamics",        make_proc(fn_db_dynamics()));
    env->set("db-pitches",         make_proc(fn_db_pitches()));
    env->set("inst-gran",          make_proc(fn_orchgran()));
    env->set("inst-gran-render",   make_proc(fn_orchgran_render()));
    env->set("inst-gran-score",    make_proc(fn_orchgran_score()));
}

#endif // SAMPSYNTH_H
