
#pragma once

// samplesynth.h — orchestral sample database and sequencing layer for pure0
//
// Designed for SOL-style libraries whose filenames follow:
//
//     instrument-articulation-pitch-dynamic[-extra].wav
//
// Example:
//     SOL/Winds/Ob/ordinario/Ob-ord-A#3-ff.wav
//
// The database is symbolic and lazy:
// - loaddb scans folders recursively and stores metadata + file paths
// - audio is loaded only when needed by db-load / sol-* primitives
//
// Registration:
//     add_samplesynth(env);

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

static std::string escape_string(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '"':  out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            default:   out.push_back(c); break;
        }
    }
    return out;
}

static std::string repr(const ExprPtr& x) {
    if (is_symbol(x)) return as_symbol(x);
    if (is_string(x)) return "\"" + escape_string(as_string(x)) + "\"";
    if (is_lambda(x)) return "<lambda>";
    if (is_proc(x))   return "<proc>";
    if (is_list(x)) {
        const auto& xs = std::get<Expr::List>(x->v);
        std::ostringstream oss;
        oss << "(";
        for (std::size_t i = 0; i < xs.size(); ++i) {
            if (i) oss << " ";
            oss << repr(xs[i]);
        }
        oss << ")";
        return oss.str();
    }
    Vector v = as_vec(x);
    std::ostringstream oss;
    oss << "(" << "vec";
    for (std::size_t i = 0; i < v.size(); ++i) oss << " " << v[i];
    oss << ")";
    return oss.str();
}

static ExprPtr kv(const std::string& key, const ExprPtr& value) {
    Expr::List kv;
    kv.push_back(make_symbol(key));
    kv.push_back(value);
    return make_list(kv);
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
        const auto& kv = std::get<Expr::List>(item->v);
        if (kv.size() != 2) continue;
        if (is_symbol(kv[0]) && as_symbol(kv[0]) == key) return true;
    }
    return false;
}

static ExprPtr entry_get(const ExprPtr& entry, const std::string& key) {
    if (!is_list(entry)) throw std::runtime_error("db entry must be a list");
    const auto& xs = std::get<Expr::List>(entry->v);
    for (const auto& item : xs) {
        if (!is_list(item)) continue;
        const auto& kv = std::get<Expr::List>(item->v);
        if (kv.size() != 2) continue;
        if (is_symbol(kv[0]) && as_symbol(kv[0]) == key) return kv[1];
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
        const int off = ev.first;
        const Vector& sig = ev.second;
        for (std::size_t i = 0; i < sig.size(); ++i) out[(std::size_t)off + i] += sig[i];
    }
    return out;
}

static int beat_to_samples(double sr, double bpm, double beats) {
    return (int)std::llround((60.0 * beats * sr) / bpm);
}


static Vector mono_fold(const Vector& interleaved, int nch) {
    if (nch <= 1) return interleaved;
    if (interleaved.size() % (std::size_t)nch != 0)
        throw std::runtime_error("wav data size not divisible by channel count");
    const std::size_t frames = interleaved.size() / (std::size_t)nch;
    Vector mono(frames);
    for (std::size_t f = 0; f < frames; ++f) {
        double acc = 0.0;
        for (int ch = 0; ch < nch; ++ch) acc += interleaved[f * (std::size_t)nch + (std::size_t)ch];
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
    if (target_sr > 0 && src_sr > 0 && src_sr != target_sr) {
        sig = resample(sig, (double)target_sr / (double)src_sr);
    }
    return sig;
}

static Vector fit_sample_to_length(const Vector& x, std::size_t new_len) {
    if (new_len >= x.size()) return x;
    if (new_len == 0) return Vector(0.0, 0);

    Vector y(new_len);
    for (std::size_t i = 0; i < new_len; ++i) y[i] = x[i];

    const std::size_t fade = std::min<std::size_t>(
        std::max<std::size_t>(32, new_len / 8),
        new_len
    );
    const std::size_t start = new_len - fade;
    for (std::size_t i = 0; i < fade; ++i) {
        const double g = 1.0 - (double)i / (double)std::max<std::size_t>(1, fade - 1);
        y[start + i] *= g;
    }
    return y;
}

static Vector extract_db_signal(const ExprPtr& x, int target_sr = 0) {
    if (is_string(x)) return load_mono_resampled(as_string(x), target_sr);
    if (is_list(x) && entry_has_key(x, "path")) {
        return load_mono_resampled(entry_get_string(x, "path"), target_sr);
    }
    throw std::runtime_error("expected db entry or string path");
}

static std::vector<ExprPtr> db_entries(const ExprPtr& db) {
    if (!is_list(db)) throw std::runtime_error("database must be a list");
    return std::get<Expr::List>(db->v);
}

static std::vector<ExprPtr> filter_exact(const std::vector<ExprPtr>& xs,
                                                     const std::string& key,
                                                     const std::string& val) {
    std::vector<ExprPtr> out;
    for (const auto& e : xs) {
        try {
            if (entry_get_string(e, key) == val) out.push_back(e);
        } catch (...) {
        }
    }
    return out;
}


static ExprPtr choose_random(const std::vector<ExprPtr>& xs) {
    if (xs.empty()) return make_list({});
    std::uniform_int_distribution<std::size_t> dist(0, xs.size() - 1);
    return xs[dist(rng())];
}

// -----------------------------------------------------------------------------
// primitives: db creation / persistence / inspection
// -----------------------------------------------------------------------------

static Proc fn_loaddb() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1 || !is_string(args[0]))
            throw std::runtime_error("loaddb expects: root");
        const std::filesystem::path root = as_string(args[0]);
        if (!std::filesystem::exists(root))
            throw std::runtime_error("loaddb: root does not exist");

        Expr::List db;
        for (const auto& de : std::filesystem::recursive_directory_iterator(root)) {
            if (!de.is_regular_file()) continue;
            const std::filesystem::path p = de.path();
            if (!is_wav_path(p)) continue;

            std::string instrument, articulation, pitch, dynamic;
            if (!parse_sol_filename(p.stem().string(), instrument, articulation, pitch, dynamic))
                continue;

            int midi = 0;
            try {
                midi = pitch_to_midi(pitch);
            } catch (...) {
                continue;
            }

            std::string family = "";
            try {
                auto rel = std::filesystem::relative(p, root);
                auto it = rel.begin();
                if (it != rel.end()) family = it->string();
            } catch (...) {
            }

            db.push_back(entry_make(
                family,
                instrument,
                articulation,
                pitch,
                midi,
                dynamic,
                std::filesystem::absolute(p).string()
            ));
        }

        return make_list(db);
    };
}

static Proc fn_savedb() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 2 || !is_string(args[1]))
            throw std::runtime_error("savedb expects: db path");
        std::ofstream out(as_string(args[1]));
        if (!out) throw std::runtime_error("savedb: cannot open " + as_string(args[1]));
        out << repr(args[0]);
        return args[0];
    };
}

static Proc fn_readdb() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1 || !is_string(args[0]))
            throw std::runtime_error("readdb expects: path");
        std::ifstream in(as_string(args[0]));
        if (!in) throw std::runtime_error("readdb: cannot open " + as_string(args[0]));
        TokenStream ts{in};
        if (ts.eof()) return make_list({});
        return parse_expr(ts);
    };
}

static Proc fn_db_stats() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("db-stats expects: db");
        const auto xs = db_entries(args[0]);
        Expr::List out;
        out.push_back(kv("entries", make_scalar((double)xs.size())));
        return make_list(out);
    };
}

static Proc fn_db_instruments() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("db-instruments expects: db");
        const auto xs = db_entries(args[0]);
        std::vector<std::string> names;
        for (const auto& e : xs) {
            try { names.push_back(entry_get_string(e, "instrument")); }
            catch (...) {}
        }
        std::sort(names.begin(), names.end());
        names.erase(std::unique(names.begin(), names.end()), names.end());
        Expr::List out;
        for (const auto& s : names) out.push_back(make_string(s));
        return make_list(out);
    };
}

static Proc fn_db_pitches() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 2 || args.size() > 4 || !is_string(args[1]))
            throw std::runtime_error("db-pitches expects: db instrument [articulation] [dynamic]");
        auto xs = db_entries(args[0]);
        xs = filter_exact(xs, "instrument", as_string(args[1]));
        if (args.size() >= 3 && is_string(args[2])) xs = filter_exact(xs, "articulation", as_string(args[2]));
        if (args.size() >= 4 && is_string(args[3])) xs = filter_exact(xs, "dynamic", as_string(args[3]));
        std::vector<std::string> names;
        for (const auto& e : xs) {
            try { names.push_back(entry_get_string(e, "pitch")); }
            catch (...) {}
        }
        std::sort(names.begin(), names.end());
        names.erase(std::unique(names.begin(), names.end()), names.end());
        Expr::List out;
        for (const auto& s : names) out.push_back(make_string(s));
        return make_list(out);
    };
}

// -----------------------------------------------------------------------------
// primitives: db querying
// -----------------------------------------------------------------------------

static Proc fn_db_filter() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 3 || !is_string(args[1]))
            throw std::runtime_error("db-filter expects: db key value");
        const auto xs = db_entries(args[0]);
        const std::string key = as_string(args[1]);
        const std::string val = to_string_value(args[2]);
        Expr::List out;
        for (const auto& e : xs) {
            try {
                if (entry_get_string(e, key) == val) out.push_back(e);
            } catch (...) {
            }
        }
        return make_list(out);
    };
}

static Proc fn_db_pick() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 5)
            throw std::runtime_error("db-pick expects: db instrument articulation pitch dynamic");
        auto xs = db_entries(args[0]);
        xs = filter_exact(xs, "instrument", to_string_value(args[1]));
        xs = filter_exact(xs, "articulation", to_string_value(args[2]));
        xs = filter_exact(xs, "pitch", to_string_value(args[3]));
        xs = filter_exact(xs, "dynamic", to_string_value(args[4]));
        if (xs.empty()) return make_list({});
        return xs.front();
    };
}

static Proc fn_db_rand() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 1 || args.size() > 4)
            throw std::runtime_error("db-rand expects: db [instrument] [articulation] [dynamic]");
        auto xs = db_entries(args[0]);
        if (args.size() >= 2) xs = filter_exact(xs, "instrument", to_string_value(args[1]));
        if (args.size() >= 3) xs = filter_exact(xs, "articulation", to_string_value(args[2]));
        if (args.size() >= 4) xs = filter_exact(xs, "dynamic", to_string_value(args[3]));
        return choose_random(xs);
    };
}

static Proc fn_db_nearest() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 4 || args.size() > 5)
            throw std::runtime_error("db-nearest expects: db instrument midi dynamic [articulation]");
        auto xs = db_entries(args[0]);
        xs = filter_exact(xs, "instrument", to_string_value(args[1]));
        xs = filter_exact(xs, "dynamic", to_string_value(args[3]));
        if (args.size() == 5) xs = filter_exact(xs, "articulation", to_string_value(args[4]));
        const int midi = (int)std::lround(as_scalar(args[2]));
        if (xs.empty()) return make_list({});

        int best_dist = std::numeric_limits<int>::max();
        ExprPtr best = xs.front();
        for (const auto& e : xs) {
            int m = entry_get_int(e, "midi");
            int d = std::abs(m - midi);
            if (d < best_dist) {
                best_dist = d;
                best = e;
            }
        }
        return best;
    };
}

// -----------------------------------------------------------------------------
// primitives: load audio
// -----------------------------------------------------------------------------

static Proc fn_db_load() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.empty() || args.size() > 2)
            throw std::runtime_error("db-load expects: entry-or-path [target_sr]");
        int target_sr = (args.size() == 2) ? (int)std::lround(as_scalar(args[1])) : 0;
        return make_vec(extract_db_signal(args[0], target_sr));
    };
}

static Proc fn_sample() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.empty() || args.size() > 2 || !is_string(args[0]))
            throw std::runtime_error("sample expects: path [target_sr]");
        int target_sr = (args.size() == 2) ? (int)std::lround(as_scalar(args[1])) : 0;
        return make_vec(load_mono_resampled(as_string(args[0]), target_sr));
    };
}

static Proc fn_reverse_sample() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("sample-reverse expects: sig");
        Vector x = as_vec(args[0]);
        Vector y(x.size());
        for (std::size_t i = 0; i < x.size(); ++i) y[i] = x[x.size() - 1 - i];
        return make_vec(y);
    };
}

static Proc fn_trim_sample() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 3) throw std::runtime_error("sample-trim expects: sig start end");
        Vector x = as_vec(args[0]);
        int a = std::max(0, (int)std::lround(as_scalar(args[1])));
        int b = std::min((int)x.size(), (int)std::lround(as_scalar(args[2])));
        if (b < a) b = a;
        Vector y((std::size_t)(b - a));
        for (int i = a; i < b; ++i) y[(std::size_t)(i - a)] = x[(std::size_t)i];
        return make_vec(y);
    };
}

static Proc fn_fadeout_sample() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 2) throw std::runtime_error("sample-fade expects: sig n");
        Vector x = as_vec(args[0]);
        int n = std::max(0, (int)std::lround(as_scalar(args[1])));
        if (n == 0 || x.size() == 0) return make_vec(x);
        Vector y = x;
        int m = std::min<int>((int)x.size(), n);
        for (int i = 0; i < m; ++i) {
            double g = 1.0 - (double)i / (double)std::max(1, m - 1);
            y[x.size() - (std::size_t)m + (std::size_t)i] *= g;
        }
        return make_vec(y);
    };
}

static Proc fn_bank() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        return make_list(args);
    };
}

static Proc fn_pick() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 2) throw std::runtime_error("pick expects: bank index");
        if (!is_list(args[0])) throw std::runtime_error("pick: bank must be a list");
        const auto& xs = std::get<Expr::List>(args[0]->v);
        if (xs.empty()) return make_list({});
        int idx = (int)std::lround(as_scalar(args[1]));
        if (idx < 0) idx = 0;
        idx %= (int)xs.size();
        return xs[(std::size_t)idx];
    };
}

// -----------------------------------------------------------------------------
// primitives: sequencing
// -----------------------------------------------------------------------------


static Proc fn_sample_pat() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 5)
            throw std::runtime_error("sample-pat expects: sr bpm beats pattern sample");
        const double sr = as_scalar(args[0]);
        const double bpm = as_scalar(args[1]);
        const double beats = as_scalar(args[2]);
        const Vector pattern = as_vec(args[3]);
        const Vector sig = extract_db_signal(args[4], (int)std::lround(sr));

        if (pattern.size() == 0) return make_vec(Vector(0.0, 0));
        const int total = beat_to_samples(sr, bpm, beats);
        const int step_len = std::max(1, (int)std::llround((double)total / (double)pattern.size()));

        std::vector<std::pair<int, Vector>> events;
        for (std::size_t i = 0; i < pattern.size(); ++i) {
            if (pattern[i] == 0.0) continue;
            Vector s = fit_sample_to_length(sig, (std::size_t)step_len);
            if (pattern[i] != 1.0) {
                for (std::size_t j = 0; j < s.size(); ++j) s[j] *= pattern[i];
            }
            events.push_back({(int)i * step_len, s});
        }
        return make_vec(mix_overlay(events));
    };
}



static Proc fn_sample_seq() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 6)
            throw std::runtime_error("sample-seq expects: sr bpm beats pattern bank indexes");
        const double sr = as_scalar(args[0]);
        const double bpm = as_scalar(args[1]);
        const double beats = as_scalar(args[2]);
        const Vector pattern = as_vec(args[3]);
        if (!is_list(args[4])) throw std::runtime_error("sample-seq: bank must be a list");
        const auto& bank = std::get<Expr::List>(args[4]->v);
        const Vector indexes = as_vec(args[5]);

        if (pattern.size() == 0 || bank.empty()) return make_vec(Vector(0.0, 0));
        const int total = beat_to_samples(sr, bpm, beats);
        const int step_len = std::max(1, (int)std::llround((double)total / (double)pattern.size()));

        std::vector<std::pair<int, Vector>> events;
        int note_i = 0;
        for (std::size_t i = 0; i < pattern.size(); ++i) {
            if (pattern[i] == 0.0) continue;
            int which = 0;
            if (indexes.size() > 0) {
                which = (int)std::lround(indexes[(std::size_t)(note_i % (int)indexes.size())]);
            }
            while (which < 0) which += (int)bank.size();
            which %= (int)bank.size();

            Vector s = extract_db_signal(bank[(std::size_t)which], (int)std::lround(sr));
            s = fit_sample_to_length(s, (std::size_t)step_len);

            if (pattern[i] != 1.0) {
                for (std::size_t j = 0; j < s.size(); ++j) s[j] *= pattern[i];
            }
            events.push_back({(int)i * step_len, s});
            ++note_i;
        }
        return make_vec(mix_overlay(events));
    };
}


static Proc fn_sol_note() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 5 && args.size() != 6)
            throw std::runtime_error("sol-note expects: db instrument articulation midi dynamic [target_sr]");
        auto xs = db_entries(args[0]);
        xs = filter_exact(xs, "instrument", to_string_value(args[1]));
        xs = filter_exact(xs, "articulation", to_string_value(args[2]));
        xs = filter_exact(xs, "dynamic", to_string_value(args[4]));
        if (xs.empty()) return make_vec(Vector(0.0, 0));

        const int midi = (int)std::lround(as_scalar(args[3]));
        int best_dist = std::numeric_limits<int>::max();
        ExprPtr best = xs.front();
        for (const auto& e : xs) {
            const int m = entry_get_int(e, "midi");
            const int d = std::abs(m - midi);
            if (d < best_dist) {
                best_dist = d;
                best = e;
            }
        }

        const int target_sr = (args.size() == 6) ? (int)std::lround(as_scalar(args[5])) : 0;
        return make_vec(extract_db_signal(best, target_sr));
    };
}


static Proc fn_sol_pat() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 8)
            throw std::runtime_error("sol-pat expects: sr bpm beats pattern db instrument articulation dynamic");
        const double sr = as_scalar(args[0]);
        const double bpm = as_scalar(args[1]);
        const double beats = as_scalar(args[2]);
        const Vector pattern = as_vec(args[3]);

        auto xs = db_entries(args[4]);
        xs = filter_exact(xs, "instrument", to_string_value(args[5]));
        xs = filter_exact(xs, "articulation", to_string_value(args[6]));
        xs = filter_exact(xs, "dynamic", to_string_value(args[7]));
        if (pattern.size() == 0 || xs.empty()) return make_vec(Vector(0.0, 0));

        const int total = beat_to_samples(sr, bpm, beats);
        const int step_len = std::max(1, (int)std::llround((double)total / (double)pattern.size()));
        std::vector<std::pair<int, Vector>> events;

        for (std::size_t i = 0; i < pattern.size(); ++i) {
            if (pattern[i] == 0.0) continue;
            ExprPtr e = choose_random(xs);
            Vector s = extract_db_signal(e, (int)std::lround(sr));
            s = fit_sample_to_length(s, (std::size_t)step_len);
            if (pattern[i] != 1.0) {
                for (std::size_t j = 0; j < s.size(); ++j) s[j] *= pattern[i];
            }
            events.push_back({(int)i * step_len, s});
        }
        return make_vec(mix_overlay(events));
    };
}


static Proc fn_sol_randpat() {
    return fn_sol_pat();
}


static Proc fn_sol_patnotes() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 9)
            throw std::runtime_error("sol-patnotes expects: sr bpm beats pattern db instrument articulation notes dynamic");
        const double sr = as_scalar(args[0]);
        const double bpm = as_scalar(args[1]);
        const double beats = as_scalar(args[2]);
        const Vector pattern = as_vec(args[3]);
        auto xs = db_entries(args[4]);
        xs = filter_exact(xs, "instrument", to_string_value(args[5]));
        xs = filter_exact(xs, "articulation", to_string_value(args[6]));
        xs = filter_exact(xs, "dynamic", to_string_value(args[8]));
        const Vector notes = as_vec(args[7]);

        if (pattern.size() == 0 || xs.empty() || notes.size() == 0) return make_vec(Vector(0.0, 0));

        const int total = beat_to_samples(sr, bpm, beats);
        const int step_len = std::max(1, (int)std::llround((double)total / (double)pattern.size()));
        std::vector<std::pair<int, Vector>> events;
        int note_idx = 0;

        for (std::size_t i = 0; i < pattern.size(); ++i) {
            if (pattern[i] == 0.0) continue;
            const int target = (int)std::lround(notes[(std::size_t)(note_idx % (int)notes.size())]);

            int best_dist = std::numeric_limits<int>::max();
            ExprPtr best = xs.front();
            for (const auto& e : xs) {
                const int m = entry_get_int(e, "midi");
                const int d = std::abs(m - target);
                if (d < best_dist) {
                    best_dist = d;
                    best = e;
                }
            }

            Vector s = extract_db_signal(best, (int)std::lround(sr));
            s = fit_sample_to_length(s, (std::size_t)step_len);
            if (pattern[i] != 1.0) {
                for (std::size_t j = 0; j < s.size(); ++j) s[j] *= pattern[i];
            }
            events.push_back({(int)i * step_len, s});
            ++note_idx;
        }

        return make_vec(mix_overlay(events));
    };
}



static Proc fn_sol_arp() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 10)
            throw std::runtime_error("sol-arp expects: sr bpm beats db instrument articulation notes dynamic subdiv mode");
        const double sr = as_scalar(args[0]);
        const double bpm = as_scalar(args[1]);
        const double beats = as_scalar(args[2]);
        auto xs = db_entries(args[3]);
        xs = filter_exact(xs, "instrument", to_string_value(args[4]));
        xs = filter_exact(xs, "articulation", to_string_value(args[5]));
        xs = filter_exact(xs, "dynamic", to_string_value(args[7]));
        const Vector notes = as_vec(args[6]);
        const int subdiv = std::max(1, (int)std::lround(as_scalar(args[8])));
        const int mode = (int)std::lround(as_scalar(args[9]));

        if (xs.empty() || notes.size() == 0) return make_vec(Vector(0.0, 0));

        std::vector<int> order;
        if (mode == 1) {
            for (int i = (int)notes.size() - 1; i >= 0; --i) order.push_back(i);
        } else if (mode == 2) {
            for (int i = 0; i < (int)notes.size(); ++i) order.push_back(i);
            for (int i = (int)notes.size() - 2; i > 0; --i) order.push_back(i);
            if (order.empty()) order.push_back(0);
        } else {
            for (int i = 0; i < (int)notes.size(); ++i) order.push_back(i);
        }

        const int n_events = std::max(1, (int)std::llround(beats * subdiv));
        const int event_step = std::max(1, beat_to_samples(sr, bpm, 1.0 / (double)subdiv));
        std::vector<std::pair<int, Vector>> events;

        for (int i = 0; i < n_events; ++i) {
            const int target = (int)std::lround(notes[(std::size_t)order[(std::size_t)(i % (int)order.size())]]);
            int best_dist = std::numeric_limits<int>::max();
            ExprPtr best = xs.front();
            for (const auto& e : xs) {
                const int m = entry_get_int(e, "midi");
                const int d = std::abs(m - target);
                if (d < best_dist) {
                    best_dist = d;
                    best = e;
                }
            }
            Vector s = extract_db_signal(best, (int)std::lround(sr));
            s = fit_sample_to_length(s, (std::size_t)event_step);
            events.push_back({i * event_step, s});
        }

        return make_vec(mix_overlay(events));
    };
}



static std::vector<std::vector<std::string>> parse_orchestra(const ExprPtr& orchestra) {
    if (!is_list(orchestra)) throw std::runtime_error("orchgran: orchestra must be a list");
    const auto& xs = std::get<Expr::List>(orchestra->v);
    std::vector<std::vector<std::string>> groups;
    if (xs.empty()) return groups;

    if (is_string(xs[0]) || is_symbol(xs[0])) {
        for (const auto& x : xs) {
            if (!(is_string(x) || is_symbol(x)))
                throw std::runtime_error("orchgran: mixed orchestra syntax");
            groups.push_back({to_string_value(x)});
        }
        return groups;
    }

    for (const auto& g : xs) {
        if (!is_list(g)) throw std::runtime_error("orchgran: nested orchestra groups must be lists");
        const auto& gs = std::get<Expr::List>(g->v);
        if (gs.empty()) continue;
        std::vector<std::string> group;
        for (const auto& item : gs) {
            if (!(is_string(item) || is_symbol(item)))
                throw std::runtime_error("orchgran: instrument names must be strings or symbols");
            group.push_back(to_string_value(item));
        }
        groups.push_back(group);
    }
    return groups;
}

static double eval_scalar_curve(const ExprPtr& curve, double t, const std::string& ctx) {
    if (!is_list(curve)) throw std::runtime_error(ctx + ": curve must be a list");
    const auto& xs = std::get<Expr::List>(curve->v);
    if (xs.empty()) throw std::runtime_error(ctx + ": empty curve");
    if (!is_list(xs[0])) throw std::runtime_error(ctx + ": malformed curve");

    double v = 0.0;
    bool initialized = false;
    for (const auto& e : xs) {
        if (!is_list(e)) throw std::runtime_error(ctx + ": malformed entry");
        const auto& kv = std::get<Expr::List>(e->v);
        if (kv.size() < 2) throw std::runtime_error(ctx + ": malformed entry");
        double te = as_scalar(kv[0]);
        if (!initialized || te <= t) {
            v = as_scalar(kv[1]);
            initialized = true;
        } else {
            break;
        }
    }
    if (!initialized) throw std::runtime_error(ctx + ": could not evaluate curve");
    return v;
}

static std::pair<int, int> eval_octave_curve(const ExprPtr& curve, double t) {
    if (!is_list(curve)) throw std::runtime_error("octave-range: curve must be a list");
    const auto& xs = std::get<Expr::List>(curve->v);
    if (xs.empty()) throw std::runtime_error("octave-range: empty curve");
    if (!is_list(xs[0])) throw std::runtime_error("octave-range: malformed curve");

    int lo = 3, hi = 5;
    bool initialized = false;
    for (const auto& e : xs) {
        if (!is_list(e)) throw std::runtime_error("octave-range: malformed entry");
        const auto& kv = std::get<Expr::List>(e->v);
        if (kv.size() < 3) throw std::runtime_error("octave-range: entry must be (t lo hi)");
        double te = as_scalar(kv[0]);
        if (!initialized || te <= t) {
            lo = (int)std::lround(as_scalar(kv[1]));
            hi = (int)std::lround(as_scalar(kv[2]));
            initialized = true;
        } else {
            break;
        }
    }
    if (hi < lo) std::swap(lo, hi);
    return {lo, hi};
}

static ExprPtr eval_payload_schedule(const ExprPtr& sched, double t, const std::string& ctx) {
    if (!is_list(sched)) throw std::runtime_error(ctx + ": schedule must be a list");
    const auto& xs = std::get<Expr::List>(sched->v);
    if (xs.empty()) throw std::runtime_error(ctx + ": empty schedule");
    ExprPtr payload;
    bool initialized = false;
    for (const auto& e : xs) {
        if (!is_list(e)) throw std::runtime_error(ctx + ": malformed entry");
        const auto& kv = std::get<Expr::List>(e->v);
        if (kv.size() < 2) throw std::runtime_error(ctx + ": malformed entry");
        double te = as_scalar(kv[0]);
        if (!initialized || te <= t) {
            payload = kv[1];
            initialized = true;
        } else {
            break;
        }
    }
    if (!initialized) throw std::runtime_error(ctx + ": could not evaluate schedule");
    return payload;
}

static int choose_midi_from_chord(const Vector& chord, int lo_oct, int hi_oct, int rand_oct) {
    if (chord.size() == 0) throw std::runtime_error("orchgran: active chord is empty");
    std::uniform_int_distribution<int> ndist(0, (int)chord.size() - 1);
    int src = (int)std::lround(chord[(std::size_t)ndist(rng())]);
    int pc = ((src % 12) + 12) % 12;

    std::uniform_int_distribution<int> odist(lo_oct, hi_oct);
    int oct = odist(rng());

    if (rand_oct > 0) {
        std::uniform_int_distribution<int> rdist(-rand_oct, rand_oct);
        oct += rdist(rng());
    }
    return 12 * (oct + 1) + pc;
}

static std::string choose_string_from_list(const ExprPtr& xs, const std::string& ctx) {
    if (!is_list(xs)) throw std::runtime_error(ctx + ": expected a list");
    const auto& ls = std::get<Expr::List>(xs->v);
    if (ls.empty()) throw std::runtime_error(ctx + ": empty list");
    std::uniform_int_distribution<int> dist(0, (int)ls.size() - 1);
    const auto& item = ls[(std::size_t)dist(rng())];
    if (!(is_string(item) || is_symbol(item)))
        throw std::runtime_error(ctx + ": expected strings or symbols");
    return to_string_value(item);
}

static ExprPtr find_best_sample(const std::vector<ExprPtr>& db,
                                const std::string& instrument,
                                const std::string& style,
                                const std::string& dynamic,
                                int midi) {
    std::vector<ExprPtr> xs = filter_exact(db, "instrument", instrument);
    if (xs.empty()) return make_list({});

    std::vector<ExprPtr> strict = filter_exact(xs, "articulation", style);
    strict = filter_exact(strict, "dynamic", dynamic);

    std::vector<ExprPtr> no_dyn = filter_exact(xs, "articulation", style);
    std::vector<ExprPtr> no_style = filter_exact(xs, "dynamic", dynamic);

    std::vector<ExprPtr> pool;
    if (!strict.empty()) pool = strict;
    else if (!no_dyn.empty()) pool = no_dyn;
    else if (!no_style.empty()) pool = no_style;
    else pool = xs;

    if (pool.empty()) return make_list({});

    int best_dist = std::numeric_limits<int>::max();
    ExprPtr best = pool.front();
    for (const auto& e : pool) {
        int m = entry_get_int(e, "midi");
        int d = std::abs(m - midi);
        if (d < best_dist) {
            best_dist = d;
            best = e;
        }
    }
    return best;
}

static ExprPtr make_event(double t,
                          double requested_len,
                          int midi,
                          const std::string& style,
                          const std::string& dynamic,
                          const std::vector<std::string>& instruments,
                          const std::vector<std::string>& paths) {
    Expr::List out;

    Expr::List insts;
    for (const auto& s : instruments) insts.push_back(make_string(s));

    Expr::List pths;
    for (const auto& s : paths) pths.push_back(make_string(s));

    out.push_back(kv("time",        make_scalar(t)));
    out.push_back(kv("length",      make_scalar(requested_len)));
    out.push_back(kv("midi",        make_scalar((double)midi)));
    out.push_back(kv("style",       make_string(style)));
    out.push_back(kv("dynamic",     make_string(dynamic)));
    out.push_back(kv("instruments", make_list(insts)));
    out.push_back(kv("paths",       make_list(pths)));

    return make_list(out);
}

static Proc fn_orchgran() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 13) {
            throw std::runtime_error(
                "orchgran expects: sr total_dur orchestra "
                "density_curve rand_density_curve length_curve rand_length_curve "
                "octave_range_curve rand_octave_curve "
                "chord_schedule style_schedule dynamic_schedule db"
            );
        }

        double sr = as_scalar(args[0]);
        double total_dur = as_scalar(args[1]);
        if (sr <= 0.0 || total_dur <= 0.0)
            throw std::runtime_error("orchgran: sr and total_dur must be > 0");

        std::vector<std::vector<std::string>> voices = parse_orchestra(args[2]);
        if (voices.empty()) throw std::runtime_error("orchgran: empty orchestra");

        const ExprPtr& density_curve      = args[3];
        const ExprPtr& rand_density_curve = args[4];
        const ExprPtr& length_curve       = args[5];
        const ExprPtr& rand_length_curve  = args[6];
        const ExprPtr& octave_curve       = args[7];
        const ExprPtr& rand_oct_curve     = args[8];
        const ExprPtr& chord_sched        = args[9];
        const ExprPtr& style_sched        = args[10];
        const ExprPtr& dyn_sched          = args[11];
        std::vector<ExprPtr> db           = db_entries(args[12]);

        std::vector<double> free_until(voices.size(), 0.0);
        std::unordered_map<std::string, Vector> cache;
        std::vector<std::pair<int, Vector>> audio_events;
        Expr::List event_log;

        double t = 0.0;
        while (t < total_dur) {
            double dens = eval_scalar_curve(density_curve, t, "density");
            double rdens = eval_scalar_curve(rand_density_curve, t, "random-density");
            double glen = eval_scalar_curve(length_curve, t, "length");
            double rglen = eval_scalar_curve(rand_length_curve, t, "random-length");
            std::pair<int, int> oct_rng = eval_octave_curve(octave_curve, t);
            int rand_oct = std::max(0, (int)std::lround(eval_scalar_curve(rand_oct_curve, t, "random-octave")));

            if (dens <= 0.0) throw std::runtime_error("orchgran: density must stay > 0");

            double dens_factor = 1.0 + urand(-rdens, rdens);
            if (dens_factor <= 0.001) dens_factor = 0.001;
            double dt = 1.0 / (dens * dens_factor);

            int chosen_voice = -1;
            for (std::size_t i = 0; i < voices.size(); ++i) {
                if (free_until[i] <= t) {
                    chosen_voice = (int)i;
                    break;
                }
            }

            if (chosen_voice >= 0) {
                double len_factor = 1.0 + urand(-rglen, rglen); 
                if (len_factor <= 0.05) len_factor = 0.05;
                double requested_len = glen * len_factor;

                Vector chord = as_vec(eval_payload_schedule(chord_sched, t, "chord-schedule"));
                std::string style = choose_string_from_list(
                    eval_payload_schedule(style_sched, t, "style-schedule"),
                    "style-schedule"
                );
                std::string dyn = choose_string_from_list(
                    eval_payload_schedule(dyn_sched, t, "dynamic-schedule"),
                    "dynamic-schedule"
                );

                int midi = choose_midi_from_chord(chord, oct_rng.first, oct_rng.second, rand_oct);

                std::vector<std::pair<int, Vector>> local;
                std::vector<std::string> paths_used;

                for (const auto& instr : voices[(std::size_t)chosen_voice]) {
                    ExprPtr entry = find_best_sample(db, instr, style, dyn, midi);
                    if (!is_list(entry) || std::get<Expr::List>(entry->v).empty()) continue;

                    std::string path = entry_get_string(entry, "path");
                    paths_used.push_back(path);

                    auto it = cache.find(path);
                    if (it == cache.end()) {
                        cache[path] = extract_db_signal(entry, (int)std::lround(sr));
                        it = cache.find(path);
                    }

                    Vector sig = it->second;
                    std::size_t requested_samps =
                        (std::size_t)std::max<double>(0.0, std::llround(requested_len * sr));
                    if (requested_samps < sig.size()) sig = fit_sample_to_length(sig, requested_samps);

                    local.push_back({(int)std::llround(t * sr), sig});
                }

                if (!local.empty()) {
                    for (const auto& ev : local) audio_events.push_back(ev);
                    event_log.push_back(make_event(
                        t,
                        requested_len,
                        midi,
                        style,
                        dyn,
                        voices[(std::size_t)chosen_voice],
                        paths_used
                    ));
                    free_until[(std::size_t)chosen_voice] = t + requested_len;
                }
            }

            t += dt;
        }

        Vector sig = mix_overlay(audio_events);

        Expr::List out;
        out.push_back(make_vec(sig));
        out.push_back(make_list(event_log));
        return make_list(out);
    };
}

static Proc fn_orchgran_signal() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1 || !is_list(args[0]))
            throw std::runtime_error("orchgran-signal expects: orchgran-result");
        const auto& xs = std::get<Expr::List>(args[0]->v);
        if (xs.size() != 2) throw std::runtime_error("orchgran-signal: malformed result");
        return xs[0];
    };
}

static Proc fn_orchgran_events() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1 || !is_list(args[0]))
            throw std::runtime_error("orchgran-events expects: orchgran-result");
        const auto& xs = std::get<Expr::List>(args[0]->v);
        if (xs.size() != 2) throw std::runtime_error("orchgran-events: malformed result");
        return xs[1];
    };
}

static Proc fn_orchgran_saveevents() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 2 || !is_string(args[1]))
            throw std::runtime_error("orchgran-saveevents expects: events path");
        std::ofstream out(as_string(args[1]));
        if (!out) throw std::runtime_error("orchgran-saveevents: cannot open output file");
        out << repr(args[0]);
        return args[0];
    };
}

// -----------------------------------------------------------------------------
// registration
// -----------------------------------------------------------------------------

static void add_samplesynth(std::shared_ptr<Env> env) {
    env->set("loaddb",          make_proc(fn_loaddb()));
    env->set("savedb",          make_proc(fn_savedb()));
    env->set("readdb",          make_proc(fn_readdb()));
    env->set("db-stats",        make_proc(fn_db_stats()));
    env->set("db-instruments",  make_proc(fn_db_instruments()));
    env->set("db-pitches",      make_proc(fn_db_pitches()));
    env->set("db-filter",       make_proc(fn_db_filter()));
    env->set("db-pick",         make_proc(fn_db_pick()));
    env->set("db-rand",         make_proc(fn_db_rand()));
    env->set("db-nearest",      make_proc(fn_db_nearest()));
    env->set("db-load",         make_proc(fn_db_load()));
    env->set("sample",          make_proc(fn_sample()));
    env->set("sample-reverse",  make_proc(fn_reverse_sample()));
    env->set("sample-trim",     make_proc(fn_trim_sample()));
    env->set("sample-fade",     make_proc(fn_fadeout_sample()));
    env->set("sample-fadeout",  make_proc(fn_fadeout_sample()));
    env->set("bank",            make_proc(fn_bank()));
    env->set("pick",            make_proc(fn_pick()));
    env->set("sample-pat",      make_proc(fn_sample_pat()));
    env->set("sample-seq",      make_proc(fn_sample_seq()));
    env->set("sol-note",        make_proc(fn_sol_note()));
    env->set("sol-pat",         make_proc(fn_sol_pat()));
    env->set("sol-randpat",     make_proc(fn_sol_randpat()));
    env->set("sol-patnotes",    make_proc(fn_sol_patnotes()));
    env->set("sol-arp",         make_proc(fn_sol_arp()));
    env->set("orchgran",            make_proc(fn_orchgran()));
    env->set("orchgran-signal",     make_proc(fn_orchgran_signal()));
    env->set("orchgran-events",     make_proc(fn_orchgran_events()));
    env->set("orchgran-saveevents", make_proc(fn_orchgran_saveevents()));    
}

// eof
