#ifndef MUSIC_H
#define MUSIC_H

#include "core.h"
#include "dsp.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

static double clamp(double x, double lo, double hi) {
    return std::max(lo, std::min(hi, x));
}

static int iround(double x) {
    return (int)std::floor(x + 0.5);
}

static double frand() {
    static thread_local std::mt19937 rng((unsigned)std::random_device{}());
    static thread_local std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng);
}

static double frand_signed() {
    return 2.0 * frand() - 1.0;
}

static Vector take(const Vector& x, std::size_t n) {
    Vector r(n);
    std::size_t m = std::min<std::size_t>(x.size(), n);
    for (std::size_t i = 0; i < m; ++i) r[i] = x[i];
    for (std::size_t i = m; i < n; ++i) r[i] = 0.0;
    return r;
}

static Vector repeat_to(const Vector& x, std::size_t n) {
    if (n == 0) return Vector(0.0, 0);
    if (x.size() == 0) return Vector(0.0, n);
    Vector r(n);
    for (std::size_t i = 0; i < n; ++i) r[i] = x[i % x.size()];
    return r;
}

static Vector noise(std::size_t n, double amp = 1.0) {
    Vector r(n);
    for (std::size_t i = 0; i < n; ++i) r[i] = amp * frand_signed();
    return r;
}

static Vector impulse(std::size_t n, std::size_t pos, double amp = 1.0) {
    Vector r(0.0, n);
    if (pos < n) r[pos] = amp;
    return r;
}

static Vector line(std::size_t n, double a, double b) {
    if (n == 0) return Vector(0.0, 0);
    if (n == 1) return Vector(a, 1);
    Vector r(n);
    for (std::size_t i = 0; i < n; ++i) {
        double t = (double)i / (double)(n - 1);
        r[i] = a + (b - a) * t;
    }
    return r;
}

static double mtof_scalar(double midi) {
    return 440.0 * std::pow(2.0, (midi - 69.0) / 12.0);
}

static Vector decay_env(std::size_t n, double tau, double sr) {
    if (n == 0) return Vector(0.0, 0);
    tau = std::max(tau, 1.0 / std::max(1.0, sr));
    Vector e(n);
    for (std::size_t i = 0; i < n; ++i) e[i] = std::exp(-(double)i / (tau * sr));
    return e;
}

static Vector cos_env(std::size_t n) {
    if (n == 0) return Vector(0.0, 0);
    if (n == 1) return Vector(1.0, 1);
    Vector w(n);
    for (std::size_t i = 0; i < n; ++i) {
        double t = (double)i / (double)(n - 1);
        w[i] = 0.5 - 0.5 * std::cos(2.0 * M_PI * t);
    }
    return w;
}

static Vector normalize(const Vector& x, double peak = 1.0) {
    double mx = 0.0;
    for (std::size_t i = 0; i < x.size(); ++i) mx = std::max(mx, std::abs(x[i]));
    if (mx <= 0.0) return x;
    return x * (peak / mx);
}

static std::vector<double> as_number_list(const ExprPtr& x) {
    std::vector<double> out;
    if (is_list(x)) {
        const auto& xs = std::get<Expr::List>(x->v);
        out.reserve(xs.size());
        for (const auto& e : xs) out.push_back(as_scalar(e));
        return out;
    }
    if (is_vec(x)) {
        Vector v = as_vec(x);
        out.reserve(v.size());
        for (std::size_t i = 0; i < v.size(); ++i) out.push_back(v[i]);
        return out;
    }
    throw std::runtime_error("expected list or vector of numbers");
}

static std::vector<int> as_int_list(const ExprPtr& x) {
    std::vector<double> tmp = as_number_list(x);
    std::vector<int> out(tmp.size());
    for (std::size_t i = 0; i < tmp.size(); ++i) out[i] = (int)std::floor(tmp[i]);
    return out;
}

static int beat_samples(double sr, double bpm, double beats) {
    if (sr <= 0.0) throw std::runtime_error("sr must be > 0");
    if (bpm <= 0.0) throw std::runtime_error("bpm must be > 0");
    return iround((60.0 / bpm) * beats * sr);
}

static Vector seq_values(std::size_t n, const std::vector<double>& vals) {
    if (vals.empty()) return Vector(0.0, n);
    Vector r(n);
    for (std::size_t i = 0; i < n; ++i) r[i] = vals[i % vals.size()];
    return r;
}

static Vector hold_values(const std::vector<double>& vals, std::size_t seglen) {
    if (seglen == 0) throw std::runtime_error("seglen must be > 0");
    Vector r(vals.size() * seglen);
    for (std::size_t i = 0; i < vals.size(); ++i)
        for (std::size_t j = 0; j < seglen; ++j) r[i * seglen + j] = vals[i];
    return r;
}

static std::vector<int> euclid_pattern(int pulses, int steps) {
    if (steps <= 0) throw std::runtime_error("euclid: steps must be > 0");
    if (pulses < 0) pulses = 0;
    if (pulses > steps) pulses = steps;
    std::vector<int> out((std::size_t)steps, 0);
    for (int i = 0; i < steps; ++i) {
        int a = (i * pulses) / steps;
        int b = ((i + 1) * pulses) / steps;
        out[(std::size_t)i] = (b > a) ? 1 : 0;
    }
    return out;
}

static Vector trigger_train(int total_samples, int step_samples, const std::vector<int>& pattern, double amp) {
    if (total_samples < 0) total_samples = 0;
    if (step_samples <= 0) throw std::runtime_error("step duration must be > 0 samples");
    Vector out(0.0, (std::size_t)total_samples);
    if (pattern.empty()) return out;
    for (std::size_t i = 0; i < pattern.size(); ++i) {
        if (!pattern[i]) continue;
        int pos = (int)i * step_samples;
        if (pos >= 0 && pos < total_samples) out[(std::size_t)pos] += amp;
    }
    return out;
}

static Vector gen_table(const std::vector<double>& coeffs) {
    if (coeffs.empty()) throw std::runtime_error("table coefficients cannot be empty");
    std::vector<ExprPtr> args;
    args.reserve(coeffs.size() + 1);
    args.push_back(make_scalar(4096.0));
    for (double c : coeffs) args.push_back(make_scalar(c));
    return as_vec(fn_gen()(args, nullptr));
}

static Vector sine_table() {
    static Vector table = gen_table({1.0});
    return table;
}

static Vector saw_table() {
    static Vector table = gen_table({1.0, 0.5, 1.0 / 3.0, 0.25, 0.2, 1.0 / 6.0, 1.0 / 7.0, 0.125});
    return table;
}

static Vector tri_table() {
    static Vector table = gen_table({1.0, 0.0, -1.0 / 9.0, 0.0, 1.0 / 25.0, 0.0, -1.0 / 49.0});
    return table;
}

static Vector square_table() {
    static Vector table = gen_table({1.0, 0.0, 1.0 / 3.0, 0.0, 1.0 / 5.0, 0.0, 1.0 / 7.0, 0.0, 1.0 / 9.0});
    return table;
}

static const Vector& shape_table_ref(int shape) {
    if (shape == 1) {
        static Vector saw = saw_table();
        return saw;
    }
    if (shape == 2) {
        static Vector tri = tri_table();
        return tri;
    }
    if (shape == 3) {
        static Vector sq = square_table();
        return sq;
    }
    static Vector sine = sine_table();
    return sine;
}

static Vector voice_from_freqs(double sr, const Vector& freqs, const Vector& table) {
    return as_vec(fn_osc()({make_scalar(sr), make_vec(freqs), make_vec(table)}, nullptr));
}

static Vector filter(const Vector& sig, const std::string& type, double sr, double f0,
                           double q = 0.707, double gain_db = 0.0) {
    f0 = clamp(f0, 10.0, sr * 0.49);
    ExprPtr coeffs = fn_iirdesign()({make_string(type), make_scalar(sr), make_scalar(f0), make_scalar(q), make_scalar(gain_db)}, nullptr);
    const auto& xs = std::get<Expr::List>(coeffs->v);
    Vector b = as_vec(xs[0]);
    Vector a = as_vec(xs[1]);
    return as_vec(fn_iir()({make_vec(sig), make_vec(b), make_vec(a)}, nullptr));
}

static Vector mix_zero(const std::vector<Vector>& xs) {
    if (xs.empty()) return Vector(0.0, 0);
    std::vector<ExprPtr> mix_args;
    mix_args.reserve(xs.size() * 2);
    for (const auto& x : xs) {
        mix_args.push_back(make_scalar(0.0));
        mix_args.push_back(make_vec(x));
    }
    return as_vec(fn_mix()(mix_args, nullptr));
}

static Vector tone(double sr, double hz, double dur, double amp, const Vector& table) {
    std::size_t n = (std::size_t)std::max(0, iround(sr * dur));
    Vector freqs(hz, n);
    return voice_from_freqs(sr, freqs, table) * amp;
}

static Vector sweep_tone(double sr, double f0, double f1, double dur, double amp, const Vector& table) {
    std::size_t n = (std::size_t)std::max(0, iround(sr * dur));
    Vector freqs = line(n, f0, f1);
    return voice_from_freqs(sr, freqs, table) * amp;
}

static Vector apply_env(const Vector& sig, const Vector& env) {
    return sig * take(env, sig.size());
}

static Vector taper(const Vector& sig, double sr, double attack_s, double release_s) {
    Vector out = sig;
    std::size_t n = out.size();
    if (n == 0) return out;
    std::size_t a = (std::size_t)std::max(0, iround(sr * attack_s));
    std::size_t r = (std::size_t)std::max(0, iround(sr * release_s));
    a = std::min<std::size_t>(a, n);
    r = std::min<std::size_t>(r, n);
    if (a > 1) for (std::size_t i = 0; i < a; ++i) out[i] *= (double)i / (double)(a - 1);
    if (r > 1) {
        for (std::size_t i = 0; i < r; ++i) {
            std::size_t j = n - r + i;
            out[j] *= 1.0 - (double)i / (double)(r - 1);
        }
    }
    return out;
}

static Vector note_voice(double sr, double hz, double dur, double amp, int shape) {
    std::size_t n = (std::size_t)std::max(1, iround(sr * dur));
    const Vector& main_table = shape_table_ref(shape);
    Vector body = tone(sr, hz, dur, 1.0, main_table);
    Vector sub = tone(sr, hz * 0.5, dur, 1.0, sine_table());
    Vector sig = (shape == 0) ? body : (body * 0.78 + sub * 0.22);
    sig = taper(sig, sr, 0.004, std::min(0.03, dur * 0.35));
    return sig * amp;
}

static Vector bass_voice(double sr, double hz, double dur, double amp, double wobble_hz, int shape) {
    std::size_t n = (std::size_t)std::max(1, iround(sr * dur));
    Vector main = tone(sr, hz, dur, 1.0, shape_table_ref(shape));
    Vector sub = tone(sr, hz * 0.5, dur, 1.0, sine_table());
    Vector lfo = tone(sr, std::max(0.01, wobble_hz), dur, 1.0, sine_table());
    Vector wob(n);
    for (std::size_t i = 0; i < n; ++i) wob[i] = 0.35 + 0.65 * ((lfo[i] + 1.0) * 0.5);
    Vector sig = main * 0.68 + sub * 0.32;
    sig = sig * wob;
    sig = filter(sig, "lowpass", sr, std::max(80.0, hz * 3.5), 0.707, 0.0);
    sig = taper(sig, sr, 0.003, std::min(0.08, dur * 0.4));
    return sig * amp;
}

static Vector kick(double sr, double dur, double f0, double f1, double tau, double amp) {
    std::size_t n = (std::size_t)std::max(1, iround(sr * dur));
    Vector body = sweep_tone(sr, f0, f1, dur, 1.0, sine_table());
    Vector env = decay_env(n, tau, sr);
    Vector click = noise(std::min<std::size_t>(n, (std::size_t)iround(0.004 * sr)), 0.15);
    Vector sig = apply_env(body, env);
    for (std::size_t i = 0; i < click.size(); ++i) sig[i] += click[i];
    return normalize(sig * amp, 1.0);
}

static Vector kick_click(double sr, double dur, double f0, double f1, double tau, double click_amt, double amp) {
    std::size_t n = (std::size_t)std::max(1, iround(sr * dur));
    Vector body = sweep_tone(sr, f0, f1, dur, 1.0, sine_table());
    body = apply_env(body, decay_env(n, tau, sr));
    std::size_t cn = std::min<std::size_t>(n, (std::size_t)iround(0.006 * sr));
    Vector click = noise(cn, 1.0);
    click = filter(click, "highpass", sr, 2500.0, 0.707, 0.0);
    click = apply_env(click, decay_env(cn, 0.006, sr));
    for (std::size_t i = 0; i < cn; ++i) body[i] += click[i] * click_amt;
    return normalize(body * amp, 1.0);
}

static Vector kick_sub(double sr, double dur, double f0, double f1, double tau, double amp) {
    std::size_t n = (std::size_t)std::max(1, iround(sr * dur));
    Vector main = sweep_tone(sr, f0, f1, dur, 1.0, sine_table());
    Vector sub = sweep_tone(sr, f0 * 0.5, std::max(18.0, f1 * 0.5), dur, 1.0, sine_table());
    Vector sig = main * 0.6 + sub * 0.4;
    sig = apply_env(sig, decay_env(n, tau, sr));
    sig = filter(sig, "lowpass", sr, std::max(70.0, f0 * 2.2), 0.707, 0.0);
    return normalize(sig * amp, 1.0);
}

static Vector snare(double sr, double dur, double tone_hz, double noise_mix, double tau, double amp) {
    std::size_t n = (std::size_t)std::max(1, iround(sr * dur));
    Vector ton = tone(sr, tone_hz, dur, 1.0 - noise_mix, tri_table());
    Vector noi  = noise(n, noise_mix);
    Vector sig  = ton + noi;
    Vector env  = decay_env(n, tau, sr);
    sig = apply_env(sig, env);
    sig = as_vec(fn_reson()({make_vec(sig), make_scalar(sr), make_scalar(1800.0), make_scalar(std::max(0.02, tau))}, nullptr));
    return normalize(sig * amp, 1.0);
}

static Vector hat(double sr, double dur, double hp_hz, double tau, double amp) {
    std::size_t n = (std::size_t)std::max(1, iround(sr * dur));
    Vector sig = noise(n);
    sig = filter(sig, "highpass", sr, hp_hz, 0.707, 0.0);
    sig = apply_env(sig, decay_env(n, tau, sr));
    return normalize(sig * amp, 1.0);
}

static Vector hat_dark(double sr, double dur, double hp_hz, double tau, double amp) {
    std::size_t n = (std::size_t)std::max(1, iround(sr * dur));
    Vector sig = noise(n);
    sig = filter(sig, "highpass", sr, hp_hz, 0.9, 0.0);
    sig = filter(sig, "peak", sr, std::max(1500.0, hp_hz * 1.2), 1.2, 3.0);
    sig = apply_env(sig, decay_env(n, tau, sr));
    return normalize(sig * amp, 1.0);
}

static Vector hat_metal(double sr, double dur, double base_hz, double tau, double amp) {
    std::size_t n = (std::size_t)std::max(1, iround(sr * dur));
    Vector f1(base_hz, n);
    Vector f2(base_hz * 1.414, n);
    Vector f3(base_hz * 1.732, n);
    Vector f4(base_hz * 2.618, n);
    Vector o1 = voice_from_freqs(sr, f1, square_table());
    Vector o2 = voice_from_freqs(sr, f2, square_table());
    Vector o3 = voice_from_freqs(sr, f3, tri_table());
    Vector o4 = voice_from_freqs(sr, f4, square_table());
    Vector sig = o1 * 0.35 + o2 * 0.25 + o3 * 0.2 + o4 * 0.2 + noise(n, 0.12);
    sig = filter(sig, "highpass", sr, std::max(2000.0, base_hz * 0.55), 0.707, 0.0);
    sig = apply_env(sig, decay_env(n, tau, sr));
    return normalize(sig * amp, 1.0);
}

static Vector drone(double sr, double hz, double dur, double amp, double beat_hz) {
    std::size_t n = (std::size_t)std::max(1, iround(sr * dur));
    Vector base(hz, n);
    Vector sig1 = voice_from_freqs(sr, base, saw_table());
    Vector sig2 = voice_from_freqs(sr, base * 0.5, sine_table());
    Vector sig3 = voice_from_freqs(sr, base * 1.5, tri_table());
    Vector slow = voice_from_freqs(sr, Vector(std::max(0.01, beat_hz), n), sine_table());
    Vector mod(n);
    for (std::size_t i = 0; i < n; ++i) mod[i] = 0.6 + 0.4 * ((slow[i] + 1.0) * 0.5);
    Vector sig = sig1 * 0.55 + sig2 * 0.25 + sig3 * 0.2;
    sig = sig * mod;
    Vector fade = cos_env(std::max<std::size_t>(8, std::min<std::size_t>(n, (std::size_t)iround(0.04 * sr))));
    for (std::size_t i = 0; i < fade.size() && i < n; ++i) sig[i] *= fade[i];
    for (std::size_t i = 0; i < fade.size() && i < n; ++i) sig[n - 1 - i] *= fade[fade.size() - 1 - i];
    return sig * amp;
}

static Vector drone_noise(double sr, double hz, double dur, double amp, double lfo_hz) {
    std::size_t n = (std::size_t)std::max(1, iround(sr * dur));
    Vector src = noise(n, 1.0);
    Vector r1 = as_vec(fn_reson()({make_vec(src), make_scalar(sr), make_scalar(std::max(30.0, hz)), make_scalar(1.2)}, nullptr));
    Vector r2 = as_vec(fn_reson()({make_vec(src), make_scalar(sr), make_scalar(std::max(40.0, hz * 1.5)), make_scalar(1.0)}, nullptr));
    Vector r3 = as_vec(fn_reson()({make_vec(src), make_scalar(sr), make_scalar(std::max(50.0, hz * 2.0)), make_scalar(0.8)}, nullptr));
    Vector sig = mix_zero({r1 * 0.5, r2 * 0.3, r3 * 0.2});
    sig = take(sig, n);
    Vector lfo = voice_from_freqs(sr, Vector(std::max(0.01, lfo_hz), n), sine_table());
    Vector env(n);
    for (std::size_t i = 0; i < n; ++i) env[i] = 0.45 + 0.55 * ((lfo[i] + 1.0) * 0.5);
    sig = sig * env;
    sig = filter(sig, "lowpass", sr, std::max(120.0, hz * 3.0), 0.707, 0.0);
    sig = taper(sig, sr, 0.04, 0.08);
    return normalize(sig * amp, 1.0);
}

static Vector drone_reson(double sr, double hz, double dur, double amp, double tau, double lfo_hz) {
    std::size_t n = (std::size_t)std::max(1, iround(sr * dur));
    Vector exc = noise((std::size_t)std::max(8, iround(sr * 0.03)), 1.0);
    Vector r1 = as_vec(fn_reson()({make_vec(exc), make_scalar(sr), make_scalar(std::max(25.0, hz)), make_scalar(std::max(0.2, tau))}, nullptr));
    Vector r2 = as_vec(fn_reson()({make_vec(exc), make_scalar(sr), make_scalar(std::max(35.0, hz * 1.25)), make_scalar(std::max(0.15, tau * 0.8))}, nullptr));
    Vector r3 = as_vec(fn_reson()({make_vec(exc), make_scalar(sr), make_scalar(std::max(45.0, hz * 1.5)), make_scalar(std::max(0.12, tau * 0.6))}, nullptr));
    Vector bed = mix_zero({r1 * 0.5, r2 * 0.3, r3 * 0.2});
    Vector sig = repeat_to(bed, n);
    Vector lfo = voice_from_freqs(sr, Vector(std::max(0.01, lfo_hz), n), sine_table());
    for (std::size_t i = 0; i < n; ++i) sig[i] *= 0.6 + 0.4 * ((lfo[i] + 1.0) * 0.5);
    sig = filter(sig, "peak", sr, std::max(100.0, hz * 2.0), 1.1, 4.0);
    sig = taper(sig, sr, 0.03, 0.08);
    return normalize(sig * amp, 1.0);
}

static Vector pad_minor(double sr, double root_midi, double dur, double amp, double lfo_hz) {
    double m1 = root_midi;
    double m2 = root_midi + 3.0;
    double m3 = root_midi + 7.0;
    double m4 = root_midi + 10.0;
    double h1 = mtof_scalar(m1);
    double h2 = mtof_scalar(m2);
    double h3 = mtof_scalar(m3);
    double h4 = mtof_scalar(m4);
    Vector v1 = note_voice(sr, h1, dur, 1.0, 1);
    Vector v2 = note_voice(sr, h2, dur, 1.0, 2);
    Vector v3 = note_voice(sr, h3, dur, 1.0, 1);
    Vector v4 = note_voice(sr, h4, dur, 1.0, 0);
    Vector sig = mix_zero({v1 * 0.34, v2 * 0.24, v3 * 0.24, v4 * 0.18});
    std::size_t n = sig.size();
    Vector lfo = voice_from_freqs(sr, Vector(std::max(0.01, lfo_hz), n), sine_table());
    for (std::size_t i = 0; i < n; ++i) sig[i] *= 0.55 + 0.45 * ((lfo[i] + 1.0) * 0.5);
    sig = filter(sig, "lowpass", sr, std::max(500.0, h3 * 3.0), 0.8, 0.0);
    sig = taper(sig, sr, 0.08, 0.18);
    return normalize(sig * amp, 1.0);
}

static Vector stab_minor(double sr, double root_midi, double dur, double amp, int shape) {
    double m1 = root_midi;
    double m2 = root_midi + 3.0;
    double m3 = root_midi + 7.0;
    Vector v1 = note_voice(sr, mtof_scalar(m1), dur, 1.0, shape);
    Vector v2 = note_voice(sr, mtof_scalar(m2), dur, 1.0, shape);
    Vector v3 = note_voice(sr, mtof_scalar(m3), dur, 1.0, shape);
    Vector sig = mix_zero({v1 * 0.4, v2 * 0.3, v3 * 0.3});
    sig = filter(sig, "lowpass", sr, mtof_scalar(m3) * 2.8, 0.9, 0.0);
    sig = taper(sig, sr, 0.003, std::min(0.12, dur * 0.5));
    return normalize(sig * amp, 1.0);
}

static Vector bass_sub(double sr, double hz, double dur, double amp, double wobble_hz) {
    std::size_t n = (std::size_t)std::max(1, iround(sr * dur));
    Vector sub = tone(sr, hz, dur, 1.0, sine_table());
    Vector overtone = tone(sr, hz * 2.0, dur, 1.0, tri_table());
    Vector lfo = voice_from_freqs(sr, Vector(std::max(0.01, wobble_hz), n), sine_table());
    Vector sig = sub * 0.82 + overtone * 0.18;
    for (std::size_t i = 0; i < n; ++i) sig[i] *= 0.7 + 0.3 * ((lfo[i] + 1.0) * 0.5);
    sig = taper(sig, sr, 0.003, std::min(0.08, dur * 0.35));
    return normalize(sig * amp, 1.0);
}

static Vector stereo_interleave(const Vector& left, const Vector& right) {
    std::size_t n = std::max<std::size_t>(left.size(), right.size());
    Vector l = broadcast(left, n);
    Vector r = broadcast(right, n);
    Vector out(n * 2);
    for (std::size_t i = 0; i < n; ++i) {
        out[2 * i] = l[i];
        out[2 * i + 1] = r[i];
    }
    return out;
}

static Vector pat(double sr, double bpm, double beats, const std::vector<double>& pattern, const Vector& event) {
    int total = beat_samples(sr, bpm, beats);
    if (total <= 0) return Vector(0.0, 0);
    if (pattern.empty() || event.size() == 0) return Vector(0.0, (std::size_t)total);
    int step_samples = std::max(1, iround((double)total / (double)pattern.size()));
    std::vector<ExprPtr> mix_args;
    for (std::size_t i = 0; i < pattern.size(); ++i) {
        if (pattern[i] == 0.0) continue;
        mix_args.push_back(make_scalar((double)((int)i * step_samples)));
        mix_args.push_back(make_vec(event * pattern[i]));
    }
    if (mix_args.empty()) return Vector(0.0, (std::size_t)total);
    Vector out = as_vec(fn_mix()(mix_args, nullptr));
    return take(out, (std::size_t)total);
}

static Vector patnotes(double sr, double bpm, double beats,
                             const std::vector<double>& pattern,
                             const std::vector<double>& midi_notes,
                             double note_beats, double amp, int shape) {
    int total = beat_samples(sr, bpm, beats);
    if (total <= 0) return Vector(0.0, 0);
    if (pattern.empty() || midi_notes.empty()) return Vector(0.0, (std::size_t)total);
    int step_samples = std::max(1, iround((double)total / (double)pattern.size()));
    double step_beats = beats / (double)pattern.size();
    double dur_beats = (note_beats > 0.0) ? note_beats : step_beats;
    double dur_sec = (60.0 / bpm) * dur_beats;
    std::vector<ExprPtr> mix_args;
    std::size_t note_index = 0;
    for (std::size_t i = 0; i < pattern.size(); ++i) {
        double w = pattern[i];
        if (w == 0.0) continue;
        double midi = midi_notes[note_index % midi_notes.size()];
        ++note_index;
        double hz = mtof_scalar(midi);
        Vector ev = note_voice(sr, hz, dur_sec, amp * std::abs(w), shape);
        mix_args.push_back(make_scalar((double)((int)i * step_samples)));
        mix_args.push_back(make_vec(ev));
    }
    if (mix_args.empty()) return Vector(0.0, (std::size_t)total);
    return take(as_vec(fn_mix()(mix_args, nullptr)), (std::size_t)total);
}

static Vector bassline(double sr, double bpm, double beats,
                             const std::vector<double>& pattern,
                             const std::vector<double>& midi_notes,
                             double note_beats, double amp,
                             double wobble_hz, int shape) {
    int total = beat_samples(sr, bpm, beats);
    if (total <= 0) return Vector(0.0, 0);
    if (pattern.empty() || midi_notes.empty()) return Vector(0.0, (std::size_t)total);
    int step_samples = std::max(1, iround((double)total / (double)pattern.size()));
    double step_beats = beats / (double)pattern.size();
    double dur_beats = (note_beats > 0.0) ? note_beats : step_beats;
    double dur_sec = (60.0 / bpm) * dur_beats;
    std::vector<ExprPtr> mix_args;
    std::size_t note_index = 0;
    for (std::size_t i = 0; i < pattern.size(); ++i) {
        double w = pattern[i];
        if (w == 0.0) continue;
        double midi = midi_notes[note_index % midi_notes.size()];
        ++note_index;
        double hz = mtof_scalar(midi);
        Vector ev = bass_voice(sr, hz, dur_sec, amp * std::abs(w), wobble_hz, shape);
        mix_args.push_back(make_scalar((double)((int)i * step_samples)));
        mix_args.push_back(make_vec(ev));
    }
    if (mix_args.empty()) return Vector(0.0, (std::size_t)total);
    return take(as_vec(fn_mix()(mix_args, nullptr)), (std::size_t)total);
}

static std::vector<double> arp_order(const std::vector<double>& notes, int mode) {
    if (notes.empty()) return {};
    if (mode == 1) return std::vector<double>(notes.rbegin(), notes.rend());
    if (mode == 2 && notes.size() > 1) {
        std::vector<double> out = notes;
        for (std::size_t i = notes.size() - 2; i > 0; --i) out.push_back(notes[i]);
        return out;
    }
    return notes;
}

static Vector arp(double sr, double bpm, double beats,
                        const std::vector<double>& midi_notes,
                        double subdiv, double note_beats,
                        double amp, int shape, int mode) {
    int total = beat_samples(sr, bpm, beats);
    if (total <= 0) return Vector(0.0, 0);
    if (midi_notes.empty()) return Vector(0.0, (std::size_t)total);
    subdiv = std::max(1.0, subdiv);
    double step_beats = 1.0 / subdiv;
    int count = std::max(1, (int)std::floor(beats * subdiv + 1e-9));
    int step_samples = std::max(1, beat_samples(sr, bpm, step_beats));
    double dur_sec = (60.0 / bpm) * ((note_beats > 0.0) ? note_beats : step_beats);
    std::vector<double> order = arp_order(midi_notes, mode);
    std::vector<ExprPtr> mix_args;
    for (int i = 0; i < count; ++i) {
        double midi = order[(std::size_t)i % order.size()];
        double hz = mtof_scalar(midi);
        Vector ev = note_voice(sr, hz, dur_sec, amp, shape);
        mix_args.push_back(make_scalar((double)(i * step_samples)));
        mix_args.push_back(make_vec(ev));
    }
    if (mix_args.empty()) return Vector(0.0, (std::size_t)total);
    return take(as_vec(fn_mix()(mix_args, nullptr)), (std::size_t)total);
}

static Proc fn_mtof() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("mtof expects 1 argument");
        Vector midi = as_vec(args[0]);
        Vector hz(midi.size());
        for (std::size_t i = 0; i < midi.size(); ++i) hz[i] = mtof_scalar(midi[i]);
        return make_vec(hz);
    };
}

static Proc fn_ftom() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("ftom expects 1 argument");
        Vector hz = as_vec(args[0]);
        Vector midi(hz.size());
        for (std::size_t i = 0; i < hz.size(); ++i) midi[i] = 69.0 + 12.0 * std::log2(std::max(hz[i], 1e-12) / 440.0);
        return make_vec(midi);
    };
}

static Proc fn_seq() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 2) throw std::runtime_error("seq expects: n v1 [v2 ...]");
        std::size_t n = (std::size_t)std::max(0.0, as_scalar(args[0]));
        std::vector<double> vals;
        vals.reserve(args.size() - 1);
        for (std::size_t i = 1; i < args.size(); ++i) vals.push_back(as_scalar(args[i]));
        return make_vec(seq_values(n, vals));
    };
}

static Proc fn_hold() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 2) throw std::runtime_error("hold expects: pattern seglen");
        std::vector<double> vals = as_number_list(args[0]);
        std::size_t seglen = (std::size_t)std::max(0.0, as_scalar(args[1]));
        return make_vec(hold_values(vals, seglen));
    };
}

static Proc fn_loop() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 2) throw std::runtime_error("loop expects: sig n");
        Vector x = as_vec(args[0]);
        std::size_t n = (std::size_t)std::max(0.0, as_scalar(args[1]));
        return make_vec(repeat_to(x, n));
    };
}

static Proc fn_impulse() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 2 || args.size() > 3) throw std::runtime_error("impulse expects: n pos [amp=1]");
        std::size_t n = (std::size_t)std::max(0.0, as_scalar(args[0]));
        std::size_t pos = (std::size_t)std::max(0.0, as_scalar(args[1]));
        double amp = (args.size() == 3) ? as_scalar(args[2]) : 1.0;
        return make_vec(impulse(n, pos, amp));
    };
}

static Proc fn_noise() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 1 || args.size() > 2) throw std::runtime_error("noise expects: n [amp=1]");
        std::size_t n = (std::size_t)std::max(0.0, as_scalar(args[0]));
        double amp = (args.size() == 2) ? as_scalar(args[1]) : 1.0;
        return make_vec(noise(n, amp));
    };
}

static Proc fn_pulse() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 2 || args.size() > 4) throw std::runtime_error("pulse expects: n step [width=1] [amp=1]");
        std::size_t n = (std::size_t)std::max(0.0, as_scalar(args[0]));
        std::size_t step = (std::size_t)std::max(1.0, as_scalar(args[1]));
        std::size_t width = (args.size() >= 3) ? (std::size_t)std::max(1.0, as_scalar(args[2])) : 1;
        double amp = (args.size() == 4) ? as_scalar(args[3]) : 1.0;
        Vector r(0.0, n);
        for (std::size_t i = 0; i < n; i += step)
            for (std::size_t j = 0; j < width && i + j < n; ++j) r[i + j] = amp;
        return make_vec(r);
    };
}

static Proc fn_euclid() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 2 || args.size() > 4) throw std::runtime_error("euclid expects: pulses steps [rotation=0] [amp=1]");
        int pulses = (int)as_scalar(args[0]);
        int steps  = (int)as_scalar(args[1]);
        int rot    = (args.size() >= 3) ? (int)as_scalar(args[2]) : 0;
        double amp = (args.size() == 4) ? as_scalar(args[3]) : 1.0;
        std::vector<int> pat = euclid_pattern(pulses, steps);
        Vector out((std::size_t)steps);
        for (int i = 0; i < steps; ++i) {
            int j = (i - rot) % steps;
            if (j < 0) j += steps;
            out[(std::size_t)i] = pat[(std::size_t)j] ? amp : 0.0;
        }
        return make_vec(out);
    };
}

static Proc fn_steps() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 4 || args.size() > 5) throw std::runtime_error("steps expects: sr bpm beats pattern [amp=1]");
        double sr = as_scalar(args[0]);
        double bpm = as_scalar(args[1]);
        double beats = as_scalar(args[2]);
        double amp = (args.size() == 5) ? as_scalar(args[4]) : 1.0;
        std::vector<int> pat = as_int_list(args[3]);
        int total = beat_samples(sr, bpm, beats);
        int step_samples = (pat.empty() ? total : std::max(1, iround((double)total / (double)pat.size())));
        return make_vec(trigger_train(total, step_samples, pat, amp));
    };
}

static Proc fn_beatsamps() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 3) throw std::runtime_error("beatsamps expects: sr bpm beats");
        return make_scalar((double)beat_samples(as_scalar(args[0]), as_scalar(args[1]), as_scalar(args[2])));
    };
}

static Proc fn_tone() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 4 || args.size() > 5) throw std::runtime_error("tone expects: sr hz dur amp [shape=0]");
        double sr  = as_scalar(args[0]);
        double hz  = as_scalar(args[1]);
        double dur = as_scalar(args[2]);
        double amp = as_scalar(args[3]);
        int shape  = (args.size() == 5) ? (int)as_scalar(args[4]) : 0;
        return make_vec(tone(sr, hz, dur, amp, shape_table_ref(shape)));
    };
}

static Proc fn_drone() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 4 || args.size() > 5) throw std::runtime_error("drone expects: sr hz dur amp [lfo_hz=0.1]");
        double sr  = as_scalar(args[0]);
        double hz  = as_scalar(args[1]);
        double dur = as_scalar(args[2]);
        double amp = as_scalar(args[3]);
        double lfo = (args.size() == 5) ? as_scalar(args[4]) : 0.1;
        return make_vec(drone(sr, hz, dur, amp, lfo));
    };
}

static Proc fn_drone_noise() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 4 || args.size() > 5) throw std::runtime_error("drone_noise expects: sr hz dur amp [lfo_hz=0.08]");
        double sr  = as_scalar(args[0]);
        double hz  = as_scalar(args[1]);
        double dur = as_scalar(args[2]);
        double amp = as_scalar(args[3]);
        double lfo = (args.size() == 5) ? as_scalar(args[4]) : 0.08;
        return make_vec(drone_noise(sr, hz, dur, amp, lfo));
    };
}

static Proc fn_drone_reson() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 4 || args.size() > 6) throw std::runtime_error("drone_reson expects: sr hz dur amp [tau=1.2] [lfo_hz=0.07]");
        double sr  = as_scalar(args[0]);
        double hz  = as_scalar(args[1]);
        double dur = as_scalar(args[2]);
        double amp = as_scalar(args[3]);
        double tau = (args.size() >= 5) ? as_scalar(args[4]) : 1.2;
        double lfo = (args.size() == 6) ? as_scalar(args[5]) : 0.07;
        return make_vec(drone_reson(sr, hz, dur, amp, tau, lfo));
    };
}

static Proc fn_kick() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 2 || args.size() > 6) throw std::runtime_error("kick expects: sr dur [f0=140] [f1=42] [tau=0.18] [amp=1]");
        double sr  = as_scalar(args[0]);
        double dur = as_scalar(args[1]);
        double f0  = (args.size() >= 3) ? as_scalar(args[2]) : 140.0;
        double f1  = (args.size() >= 4) ? as_scalar(args[3]) : 42.0;
        double tau = (args.size() >= 5) ? as_scalar(args[4]) : 0.18;
        double amp = (args.size() == 6) ? as_scalar(args[5]) : 1.0;
        return make_vec(kick(sr, dur, f0, f1, tau, amp));
    };
}

static Proc fn_kick_click() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 2 || args.size() > 7) throw std::runtime_error("kick_click expects: sr dur [f0=170] [f1=36] [tau=0.18] [click=0.4] [amp=1]");
        double sr  = as_scalar(args[0]);
        double dur = as_scalar(args[1]);
        double f0  = (args.size() >= 3) ? as_scalar(args[2]) : 170.0;
        double f1  = (args.size() >= 4) ? as_scalar(args[3]) : 36.0;
        double tau = (args.size() >= 5) ? as_scalar(args[4]) : 0.18;
        double click = (args.size() >= 6) ? as_scalar(args[5]) : 0.4;
        double amp = (args.size() == 7) ? as_scalar(args[6]) : 1.0;
        return make_vec(kick_click(sr, dur, f0, f1, tau, click, amp));
    };
}

static Proc fn_kick_sub() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 2 || args.size() > 6) throw std::runtime_error("kick_sub expects: sr dur [f0=95] [f1=28] [tau=0.28] [amp=1]");
        double sr  = as_scalar(args[0]);
        double dur = as_scalar(args[1]);
        double f0  = (args.size() >= 3) ? as_scalar(args[2]) : 95.0;
        double f1  = (args.size() >= 4) ? as_scalar(args[3]) : 28.0;
        double tau = (args.size() >= 5) ? as_scalar(args[4]) : 0.28;
        double amp = (args.size() == 6) ? as_scalar(args[5]) : 1.0;
        return make_vec(kick_sub(sr, dur, f0, f1, tau, amp));
    };
}

static Proc fn_snare() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 2 || args.size() > 6) throw std::runtime_error("snare expects: sr dur [tone=190] [noise_mix=0.75] [tau=0.12] [amp=1]");
        double sr = as_scalar(args[0]);
        double dur = as_scalar(args[1]);
        double tone = (args.size() >= 3) ? as_scalar(args[2]) : 190.0;
        double noise_mix = (args.size() >= 4) ? as_scalar(args[3]) : 0.75;
        double tau = (args.size() >= 5) ? as_scalar(args[4]) : 0.12;
        double amp = (args.size() == 6) ? as_scalar(args[5]) : 1.0;
        noise_mix = clamp(noise_mix, 0.0, 1.0);
        return make_vec(snare(sr, dur, tone, noise_mix, tau, amp));
    };
}

static Proc fn_hat() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 2 || args.size() > 5) throw std::runtime_error("hat expects: sr dur [hp_hz=7000] [tau=0.05] [amp=1]");
        double sr = as_scalar(args[0]);
        double dur = as_scalar(args[1]);
        double hp = (args.size() >= 3) ? as_scalar(args[2]) : 7000.0;
        double tau = (args.size() >= 4) ? as_scalar(args[3]) : 0.05;
        double amp = (args.size() == 5) ? as_scalar(args[4]) : 1.0;
        return make_vec(hat(sr, dur, hp, tau, amp));
    };
}

static Proc fn_hat_dark() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 2 || args.size() > 5) throw std::runtime_error("hat_dark expects: sr dur [hp_hz=4500] [tau=0.08] [amp=1]");
        double sr = as_scalar(args[0]);
        double dur = as_scalar(args[1]);
        double hp = (args.size() >= 3) ? as_scalar(args[2]) : 4500.0;
        double tau = (args.size() >= 4) ? as_scalar(args[3]) : 0.08;
        double amp = (args.size() == 5) ? as_scalar(args[4]) : 1.0;
        return make_vec(hat_dark(sr, dur, hp, tau, amp));
    };
}

static Proc fn_hat_metal() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 2 || args.size() > 5) throw std::runtime_error("hat_metal expects: sr dur [base_hz=7600] [tau=0.05] [amp=1]");
        double sr = as_scalar(args[0]);
        double dur = as_scalar(args[1]);
        double base = (args.size() >= 3) ? as_scalar(args[2]) : 7600.0;
        double tau = (args.size() >= 4) ? as_scalar(args[3]) : 0.05;
        double amp = (args.size() == 5) ? as_scalar(args[4]) : 1.0;
        return make_vec(hat_metal(sr, dur, base, tau, amp));
    };
}

static Proc fn_pad_minor() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 4 || args.size() > 5) throw std::runtime_error("pad_minor expects: sr root_midi dur amp [lfo_hz=0.08]");
        double sr = as_scalar(args[0]);
        double root_midi = as_scalar(args[1]);
        double dur = as_scalar(args[2]);
        double amp = as_scalar(args[3]);
        double lfo = (args.size() == 5) ? as_scalar(args[4]) : 0.08;
        return make_vec(pad_minor(sr, root_midi, dur, amp, lfo));
    };
}

static Proc fn_stab_minor() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 4 || args.size() > 5) throw std::runtime_error("stab_minor expects: sr root_midi dur amp [shape=1]");
        double sr = as_scalar(args[0]);
        double root_midi = as_scalar(args[1]);
        double dur = as_scalar(args[2]);
        double amp = as_scalar(args[3]);
        int shape = (args.size() == 5) ? (int)as_scalar(args[4]) : 1;
        return make_vec(stab_minor(sr, root_midi, dur, amp, shape));
    };
}

static Proc fn_bass_sub() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 4 || args.size() > 5) throw std::runtime_error("bass_sub expects: sr hz dur amp [wobble_hz=1.2]");
        double sr = as_scalar(args[0]);
        double hz = as_scalar(args[1]);
        double dur = as_scalar(args[2]);
        double amp = as_scalar(args[3]);
        double wobble = (args.size() == 5) ? as_scalar(args[4]) : 1.2;
        return make_vec(bass_sub(sr, hz, dur, amp, wobble));
    };
}

static Proc fn_bmix() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 4 || ((args.size() - 2) % 2) != 0)
            throw std::runtime_error("bmix expects: sr bpm beat1 sig1 [beat2 sig2 ...]");
        double sr = as_scalar(args[0]);
        double bpm = as_scalar(args[1]);
        std::vector<ExprPtr> mix_args;
        mix_args.reserve(args.size() - 2);
        for (std::size_t i = 2; i < args.size(); i += 2) {
            int offset = beat_samples(sr, bpm, as_scalar(args[i]));
            mix_args.push_back(make_scalar((double)offset));
            mix_args.push_back(args[i + 1]);
        }
        return fn_mix()(mix_args, nullptr);
    };
}

static Proc fn_pat() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 5) throw std::runtime_error("pat expects: sr bpm beats pattern event");
        double sr = as_scalar(args[0]);
        double bpm = as_scalar(args[1]);
        double beats = as_scalar(args[2]);
        std::vector<double> pattern = as_number_list(args[3]);
        Vector event = as_vec(args[4]);
        return make_vec(pat(sr, bpm, beats, pattern, event));
    };
}

static Proc fn_patnotes() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 7 || args.size() > 8)
            throw std::runtime_error("patnotes expects: sr bpm beats pattern midi_notes note_beats amp [shape=0]");
        double sr = as_scalar(args[0]);
        double bpm = as_scalar(args[1]);
        double beats = as_scalar(args[2]);
        std::vector<double> pattern = as_number_list(args[3]);
        std::vector<double> notes = as_number_list(args[4]);
        double note_beats = as_scalar(args[5]);
        double amp = as_scalar(args[6]);
        int shape = (args.size() == 8) ? (int)as_scalar(args[7]) : 0;
        return make_vec(patnotes(sr, bpm, beats, pattern, notes, note_beats, amp, shape));
    };
}

static Proc fn_bassline() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 7 || args.size() > 9)
            throw std::runtime_error("bassline expects: sr bpm beats pattern midi_notes note_beats amp [wobble_hz=2] [shape=1]");
        double sr = as_scalar(args[0]);
        double bpm = as_scalar(args[1]);
        double beats = as_scalar(args[2]);
        std::vector<double> pattern = as_number_list(args[3]);
        std::vector<double> notes = as_number_list(args[4]);
        double note_beats = as_scalar(args[5]);
        double amp = as_scalar(args[6]);
        double wobble_hz = (args.size() >= 8) ? as_scalar(args[7]) : 2.0;
        int shape = (args.size() == 9) ? (int)as_scalar(args[8]) : 1;
        return make_vec(bassline(sr, bpm, beats, pattern, notes, note_beats, amp, wobble_hz, shape));
    };
}

static Proc fn_arp() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 7 || args.size() > 9)
            throw std::runtime_error("arp expects: sr bpm beats midi_notes subdiv note_beats amp [shape=0] [mode=0]");
        double sr = as_scalar(args[0]);
        double bpm = as_scalar(args[1]);
        double beats = as_scalar(args[2]);
        std::vector<double> notes = as_number_list(args[3]);
        double subdiv = as_scalar(args[4]);
        double note_beats = as_scalar(args[5]);
        double amp = as_scalar(args[6]);
        int shape = (args.size() >= 8) ? (int)as_scalar(args[7]) : 0;
        int mode = (args.size() == 9) ? (int)as_scalar(args[8]) : 0;
        return make_vec(arp(sr, bpm, beats, notes, subdiv, note_beats, amp, shape, mode));
    };
}

static Proc fn_gate() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 2) throw std::runtime_error("gate expects: sig mask");
        return make_vec(as_vec(args[0]) * as_vec(args[1]));
    };
}

static Proc fn_norm() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 1 || args.size() > 2) throw std::runtime_error("norm expects: sig [peak=1]");
        double peak = (args.size() == 2) ? as_scalar(args[1]) : 1.0;
        return make_vec(normalize(as_vec(args[0]), peak));
    };
}

static Proc fn_chop() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 2) throw std::runtime_error("chop expects: sig n");
        Vector x = as_vec(args[0]);
        std::size_t n = (std::size_t)std::max(0.0, as_scalar(args[1]));
        return make_vec(take(x, n));
    };
}

static Proc fn_stereo() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 2) throw std::runtime_error("stereo expects: left right");
        return make_vec(stereo_interleave(as_vec(args[0]), as_vec(args[1])));
    };
}

static void add_music(std::shared_ptr<Env> env) {
    env->set("mtof",        make_proc(fn_mtof()));
    env->set("ftom",        make_proc(fn_ftom()));
    env->set("seq",         make_proc(fn_seq()));
    env->set("hold",        make_proc(fn_hold()));
    env->set("loop",        make_proc(fn_loop()));
    env->set("impulse",     make_proc(fn_impulse()));
    env->set("noise",       make_proc(fn_noise()));
    env->set("pulse",       make_proc(fn_pulse()));
    env->set("euclid",      make_proc(fn_euclid()));
    env->set("steps",       make_proc(fn_steps()));
    env->set("beatsamps",   make_proc(fn_beatsamps()));
    env->set("tone",        make_proc(fn_tone()));
    env->set("drone",       make_proc(fn_drone()));
    env->set("drone_noise", make_proc(fn_drone_noise()));
    env->set("drone_reson", make_proc(fn_drone_reson()));
    env->set("kick",        make_proc(fn_kick()));
    env->set("kick_click",  make_proc(fn_kick_click()));
    env->set("kick_sub",    make_proc(fn_kick_sub()));
    env->set("snare",       make_proc(fn_snare()));
    env->set("hat",         make_proc(fn_hat()));
    env->set("hat_dark",    make_proc(fn_hat_dark()));
    env->set("hat_metal",   make_proc(fn_hat_metal()));
    env->set("pad_minor",   make_proc(fn_pad_minor()));
    env->set("stab_minor",  make_proc(fn_stab_minor()));
    env->set("bass_sub",    make_proc(fn_bass_sub()));
    env->set("bmix",        make_proc(fn_bmix()));
    env->set("pat",         make_proc(fn_pat()));
    env->set("patnotes",    make_proc(fn_patnotes()));
    env->set("bassline",    make_proc(fn_bassline()));
    env->set("arp",         make_proc(fn_arp()));
    env->set("gate",        make_proc(fn_gate()));
    env->set("norm",        make_proc(fn_norm()));
    env->set("chop",        make_proc(fn_chop()));
    env->set("stereo",      make_proc(fn_stereo()));
}

#endif
