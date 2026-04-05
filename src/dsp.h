// dsp.h — pure0 DSP library
// Copyright (c) 2026 - 2030 Carmine-Emanuele Cella. All rights reserved.
//
// Primitives: osc, fft, ifft, car2pol, pol2car, window, conv, resample,
//             delay, comb, allpass, reson, filter, filterdesign,
//             wavread, wavwrite, mix
//
// Call add_dsp(env) from main after make_environment().

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

// ── Cooley-Tukey FFT ─────────────────────────────────────────────────────────
// buf: interleaved (re, im) pairs, length = 2*N, N must be a power of 2.
// sign = -1 → forward,  +1 → inverse (unnormalized; caller divides by N).

static void fft(double* buf, int N, int sign) {
    // bit-reversal permutation
    for (int i = 1, j = 0; i < N; ++i) {
        int bit = N >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) {
            std::swap(buf[2*i],   buf[2*j]);
            std::swap(buf[2*i+1], buf[2*j+1]);
        }
    }
    // butterfly stages
    for (int len = 2; len <= N; len <<= 1) {
        double ang = sign * 2.0 * M_PI / len;
        double wRe = std::cos(ang), wIm = std::sin(ang);
        for (int i = 0; i < N; i += len) {
            double tRe = 1.0, tIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int u = 2*(i+j), v = 2*(i+j+len/2);
                double uRe = buf[u], uIm = buf[u+1];
                double vRe = buf[v], vIm = buf[v+1];
                double xRe = tRe*vRe - tIm*vIm;
                double xIm = tRe*vIm + tIm*vRe;
                buf[u]   = uRe + xRe;  buf[u+1] = uIm + xIm;
                buf[v]   = uRe - xRe;  buf[v+1] = uIm - xIm;
                double nRe = tRe*wRe - tIm*wIm;
                tIm = tRe*wIm + tIm*wRe;
                tRe = nRe;
            }
        }
    }
}

static int next_pow2(int n) {
    if (n <= 1) return 1;
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

// ── internal helpers ──────────────────────────────────────────────────────────

static Vector conv(const Vector& x, const Vector& y) {
    int sx = (int)x.size(), sy = (int)y.size();
    if (sx == 0 || sy == 0) return Vector(0.0, 0);
    int conv_len = sx + sy - 1;
    int N = next_pow2(conv_len);
    std::vector<double> X(2*N, 0.0), Y(2*N, 0.0), R(2*N, 0.0);
    for (int i = 0; i < N; ++i) {
        X[2*i] = i < sx ? x[i] : 0.0;
        Y[2*i] = i < sy ? y[i] : 0.0;
    }
    fft(X.data(), N, -1);
    fft(Y.data(), N, -1);
    for (int i = 0; i < N; ++i) {
        double xr=X[2*i], xi=X[2*i+1], yr=Y[2*i], yi=Y[2*i+1];
        R[2*i]   = xr*yr - xi*yi;
        R[2*i+1] = xr*yi + xi*yr;
    }
    fft(R.data(), N, +1);
    Vector out(conv_len);
    for (int i = 0; i < conv_len; ++i) out[i] = R[2*i] / N;
    return out;
}

static Vector resample(const Vector& x, double factor) {
    int in_len = (int)x.size();
    if (in_len == 0 || factor <= 0.0) return Vector(0.0, 0);
    int out_len = std::max(1, (int)std::floor(in_len * factor + 0.5));
    int N1 = next_pow2(in_len);
    int N2 = next_pow2(out_len);
    std::vector<double> X(2*N1, 0.0);
    for (int i = 0; i < N1; ++i) X[2*i] = i < in_len ? x[i] : 0.0;
    fft(X.data(), N1, -1);
    std::vector<double> Y(2*N2, 0.0);
    int Nc = std::min(N1/2, N2/2);
    Y[0] = X[0]; Y[1] = X[1];
    for (int k = 1; k < Nc; ++k) {
        Y[2*k]       = X[2*k];       Y[2*k+1]       = X[2*k+1];
        Y[2*(N2-k)]  = X[2*(N1-k)];  Y[2*(N2-k)+1]  = X[2*(N1-k)+1];
    }
    if (N1 % 2 == 0 && N2 % 2 == 0) {
        Y[N2] = X[N1]; Y[N2+1] = X[N1+1];
    }
    fft(Y.data(), N2, +1);
    Vector out(out_len);
    for (int i = 0; i < out_len; ++i) out[i] = Y[2*i] / N2;
    return out;
}

// ── WAV I/O helpers ───────────────────────────────────────────────────────────

static void wav_write_le16(std::ostream& o, uint16_t v) {
    o.put((char)(v & 0xff)); o.put((char)((v >> 8) & 0xff));
}
static void wav_write_le32(std::ostream& o, uint32_t v) {
    o.put((char)(v & 0xff)); o.put((char)((v >> 8) & 0xff));
    o.put((char)((v >> 16) & 0xff)); o.put((char)((v >> 24) & 0xff));
}
static uint16_t wav_read_le16(std::istream& in) {
    uint8_t a = (uint8_t)in.get(), b = (uint8_t)in.get();
    return (uint16_t)(a | (b << 8));
}
static uint32_t wav_read_le32(std::istream& in) {
    uint8_t a=(uint8_t)in.get(), b=(uint8_t)in.get(),
            c=(uint8_t)in.get(), d=(uint8_t)in.get();
    return (uint32_t)(a | (b<<8) | (c<<16) | (d<<24));
}

// ── DSP primitives ────────────────────────────────────────────────────────────

// (osc sr freq table)
// freq  : vector of instantaneous Hz values, one per output sample
// table : wavetable with guard point (last sample == first sample)
// returns a vector of the same length as freq
static Proc fn_osc() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 3)
            throw std::runtime_error("osc expects: sr freq table");
        double sr    = as_scalar(args[0]);
        Vector freqs = as_vec(args[1]);
        Vector table = as_vec(args[2]);
        int tN = (int)table.size() - 1; // guard point
        if (tN <= 0) throw std::runtime_error("osc: table must have at least 2 samples");
        double fn = sr / tN;
        double phi = 0.0;
        Vector out(freqs.size());
        for (std::size_t i = 0; i < freqs.size(); ++i) {
            int ip = (int)phi;
            double fp = phi - ip;
            out[i] = (1.0 - fp) * table[ip] + fp * table[ip + 1];
            phi += freqs[i] / fn;
            while (phi >= tN) phi -= tN;
            while (phi <  0)  phi += tN;
        }
        return make_vec(out);
    };
}

// (fft sig) → interleaved complex vector [re0 im0 re1 im1 ...]
// Zero-pads to next power of 2. Spectrum length = 2 * next_pow2(len(sig)).
static Proc fn_fft() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("fft expects: sig");
        Vector sig = as_vec(args[0]);
        int d = (int)sig.size();
        int N = next_pow2(d);
        std::vector<double> buf(2*N, 0.0);
        for (int i = 0; i < d; ++i) buf[2*i] = sig[i];
        fft(buf.data(), N, -1);
        Vector out(2*N);
        for (int i = 0; i < 2*N; ++i) out[i] = buf[i];
        return make_vec(out);
    };
}

// (ifft spectrum) → real signal
// spectrum: interleaved complex [re0 im0 re1 im1 ...]
static Proc fn_ifft() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("ifft expects: spectrum");
        Vector spec = as_vec(args[0]);
        int len = (int)spec.size();
        if (len % 2) throw std::runtime_error("ifft: spectrum length must be even");
        int N = len / 2;
        std::vector<double> buf(len);
        for (int i = 0; i < len; ++i) buf[i] = spec[i];
        fft(buf.data(), N, +1);
        Vector out(N);
        for (int i = 0; i < N; ++i) out[i] = buf[2*i] / N;
        return make_vec(out);
    };
}

// (car2pol spectrum) — cartesian (re, im) → polar (magnitude, phase)
static Proc fn_car2pol() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("car2pol expects: spectrum");
        Vector v = as_vec(args[0]);
        int N = (int)v.size() / 2;
        Vector out(2*N);
        for (int i = 0; i < N; ++i) {
            double re = v[2*i], im = v[2*i+1];
            out[2*i]   = std::sqrt(re*re + im*im);
            out[2*i+1] = std::atan2(im, re);
        }
        return make_vec(out);
    };
}

// (pol2car spectrum) — polar (magnitude, phase) → cartesian (re, im)
static Proc fn_pol2car() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("pol2car expects: spectrum");
        Vector v = as_vec(args[0]);
        int N = (int)v.size() / 2;
        Vector out(2*N);
        for (int i = 0; i < N; ++i) {
            double mag = v[2*i], phase = v[2*i+1];
            out[2*i]   = mag * std::cos(phase);
            out[2*i+1] = mag * std::sin(phase);
        }
        return make_vec(out);
    };
}

// (window n a0 a1 a2) — generalized cosine window
// Hann:     (window n 0.5  0.5  0.0)
// Hamming:  (window n 0.54 0.46 0.0)
// Blackman: (window n 0.42 0.5  0.08)
// Rect:     (window n 1.0  0.0  0.0)
static Proc fn_window() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 4) throw std::runtime_error("window expects: n a0 a1 a2");
        int N = (int)as_scalar(args[0]);
        double a0 = as_scalar(args[1]);
        double a1 = as_scalar(args[2]);
        double a2 = as_scalar(args[3]);
        if (N <= 0) throw std::runtime_error("window: n must be > 0");
        Vector w(N);
        for (int i = 0; i < N; ++i) {
            double t = 2.0 * M_PI * i / (N - 1);
            w[i] = a0 - a1 * std::cos(t) + a2 * std::cos(2.0 * t);
        }
        return make_vec(w);
    };
}

// (conv x y) — linear convolution via FFT overlap-add
static Proc fn_conv() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 2) throw std::runtime_error("conv expects: x y");
        return make_vec(conv(as_vec(args[0]), as_vec(args[1])));
    };
}

// (resample sig factor) — frequency-domain resampling
// factor > 1 → upsample,  factor < 1 → downsample
static Proc fn_resample() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 2) throw std::runtime_error("resample expects: sig factor");
        return make_vec(resample(as_vec(args[0]), as_scalar(args[1])));
    };
}

// (delay sig d) — fractional delay via linear interpolation
// d: delay in samples (may be fractional)
static Proc fn_delay() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 2) throw std::runtime_error("delay expects: sig d");
        Vector x = as_vec(args[0]);
        double D = as_scalar(args[1]);
        int N = (int)x.size();
        Vector y(N);
        for (int n = 0; n < N; ++n) {
            double pos = n - D;
            if (pos < 0.0) { y[n] = 0.0; continue; }
            int i0 = (int)std::floor(pos);
            double frac = pos - i0;
            if (i0 >= N - 1) y[n] = x[N-1];
            else             y[n] = (1.0 - frac) * x[i0] + frac * x[i0+1];
        }
        return make_vec(y);
    };
}

// (comb sig d g) — feedback comb filter: y[n] = x[n] + g * y[n-d]
static Proc fn_comb() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 3) throw std::runtime_error("comb expects: sig d g");
        Vector x = as_vec(args[0]);
        int D    = (int)as_scalar(args[1]);
        double g = as_scalar(args[2]);
        if (D < 0) throw std::runtime_error("comb: d must be >= 0");
        int N = (int)x.size();
        Vector y(N);
        for (int n = 0; n < N; ++n) {
            double fb = (n - D >= 0) ? g * y[n - D] : 0.0;
            y[n] = x[n] + fb;
        }
        return make_vec(y);
    };
}

// (allpass sig d g) — Schroeder allpass: y[n] = -g*x[n] + x[n-d] + g*y[n-d]
static Proc fn_allpass() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 3) throw std::runtime_error("allpass expects: sig d g");
        Vector x = as_vec(args[0]);
        int D    = (int)as_scalar(args[1]);
        double g = as_scalar(args[2]);
        if (D < 0) throw std::runtime_error("allpass: d must be >= 0");
        int N = (int)x.size();
        Vector y(N);
        for (int n = 0; n < N; ++n) {
            double xD = (n - D >= 0) ? x[n - D] : 0.0;
            double yD = (n - D >= 0) ? y[n - D] : 0.0;
            y[n] = -g * x[n] + xD + g * yD;
        }
        return make_vec(y);
    };
}

// (reson sig sr freq tau) — second-order resonant filter
// tau: decay time in seconds (controls Q)
// output length = sr * tau samples
static Proc fn_reson() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 4) throw std::runtime_error("reson expects: sig sr freq tau");
        Vector sig  = as_vec(args[0]);
        double sr   = as_scalar(args[1]);
        double freq = as_scalar(args[2]);
        double tau  = as_scalar(args[3]);
        double om     = 2.0 * M_PI * (freq / sr);
        double radius = std::exp(-2.0 * M_PI / (tau * sr));
        double a1     = -2.0 * radius * std::cos(om);
        double a2     = radius * radius;
        double gain   = radius * std::sin(om);
        int samps  = (int)(sr * tau);
        int insize = (int)sig.size();
        Vector out(samps);
        double y1 = 0.0, y2 = 0.0;
        for (int i = 0; i < samps; ++i) {
            double x = i < insize ? sig[i] : 0.0;
            double v = gain * x - a1 * y1 - a2 * y2;
            y2 = y1; y1 = v;
            out[i] = v;
        }
        return make_vec(out);
    };
}

// (iir sig b a) — direct-form II IIR/FIR filter
// b: numerator coefficients, a: denominator coefficients (a[0] normalized to 1)
static Proc fn_iir() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 3) throw std::runtime_error("iir expects: sig b a");
        Vector x = as_vec(args[0]);
        Vector b = as_vec(args[1]);
        Vector a = as_vec(args[2]);
        int N = (int)x.size(), Nb = (int)b.size(), Na = (int)a.size();
        if (Nb == 0) throw std::runtime_error("iir: b is empty");
        if (Na == 0) throw std::runtime_error("iir: a is empty");
        double a0 = a[0];
        if (a0 == 0.0) throw std::runtime_error("iir: a[0] is zero");
        Vector bn = b / a0, an = a / a0;
        Vector y(N);
        for (int n = 0; n < N; ++n) {
            double acc = 0.0;
            for (int k = 0; k < Nb; ++k) if (n-k >= 0) acc += bn[k] * x[n-k];
            for (int k = 1; k < Na; ++k) if (n-k >= 0) acc -= an[k] * y[n-k];
            y[n] = acc;
        }
        return make_vec(y);
    };
}

// (iirdesign type fs f0 Q gain_db)
// type: "lowpass" | "highpass" | "notch" | "peak" | "lowshelf" | "highshelf"
// returns (list b a) — each is a 3-element vector
static Proc fn_iirdesign() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 5)
            throw std::runtime_error("iirdesign expects: type fs f0 Q gain_db");
        if (!is_string(args[0]))
            throw std::runtime_error("iirdesign: type must be a string");
        const std::string& type = as_string(args[0]);
        double fs  = as_scalar(args[1]);
        double f0  = as_scalar(args[2]);
        double Q   = as_scalar(args[3]);
        double dBg = as_scalar(args[4]);
        if (fs <= 0 || f0 <= 0 || f0 >= fs/2)
            throw std::runtime_error("iirdesign: invalid fs or f0");
        if (Q <= 0) throw std::runtime_error("iirdesign: Q must be > 0");

        double w0    = 2.0 * M_PI * f0 / fs;
        double cosw  = std::cos(w0), sinw = std::sin(w0);
        double alpha = sinw / (2.0 * Q);
        double A     = std::pow(10.0, dBg / 40.0);
        double b0, b1, b2, a0, a1, a2;

        if (type == "lowpass") {
            b0=(1-cosw)/2; b1=1-cosw;    b2=(1-cosw)/2;
            a0=1+alpha;    a1=-2*cosw;   a2=1-alpha;
        } else if (type == "highpass") {
            b0=(1+cosw)/2; b1=-(1+cosw); b2=(1+cosw)/2;
            a0=1+alpha;    a1=-2*cosw;   a2=1-alpha;
        } else if (type == "notch") {
            b0=1;          b1=-2*cosw;   b2=1;
            a0=1+alpha;    a1=-2*cosw;   a2=1-alpha;
        } else if (type == "peak" || type == "peaking") {
            b0=1+alpha*A;  b1=-2*cosw;   b2=1-alpha*A;
            a0=1+alpha/A;  a1=-2*cosw;   a2=1-alpha/A;
        } else if (type == "lowshelf" || type == "loshelf") {
            double s = 2.0*std::sqrt(A)*alpha;
            b0=A*((A+1)-(A-1)*cosw+s); b1=2*A*((A-1)-(A+1)*cosw); b2=A*((A+1)-(A-1)*cosw-s);
            a0=(A+1)+(A-1)*cosw+s;     a1=-2*((A-1)+(A+1)*cosw);   a2=(A+1)+(A-1)*cosw-s;
        } else if (type == "highshelf" || type == "hishelf") {
            double s = 2.0*std::sqrt(A)*alpha;
            b0=A*((A+1)+(A-1)*cosw+s); b1=-2*A*((A-1)+(A+1)*cosw); b2=A*((A+1)+(A-1)*cosw-s);
            a0=(A+1)-(A-1)*cosw+s;     a1=2*((A-1)-(A+1)*cosw);     a2=(A+1)-(A-1)*cosw-s;
        } else {
            throw std::runtime_error("iirdesign: unknown type: " + type);
        }

        b0/=a0; b1/=a0; b2/=a0; a1/=a0; a2/=a0;
        Vector bv(3), av(3);
        bv[0]=b0; bv[1]=b1; bv[2]=b2;
        av[0]=1.0; av[1]=a1; av[2]=a2;
        Expr::List result;
        result.push_back(make_vec(bv));
        result.push_back(make_vec(av));
        return make_list(result);
    };
}

// (mix offset1 sig1 offset2 sig2 ...) — overlay signals at sample offsets
// Arguments come in pairs: (offset signal). Returns a single mixed vector.
static Proc fn_mix() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() % 2 != 0)
            throw std::runtime_error("mix expects pairs: offset sig ...");
        std::vector<double> out;
        for (std::size_t i = 0; i < args.size(); i += 2) {
            int offset = (int)as_scalar(args[i]);
            if (offset < 0) throw std::runtime_error("mix: offset must be >= 0");
            Vector sig = as_vec(args[i+1]);
            int end = offset + (int)sig.size();
            if (end > (int)out.size()) out.resize((std::size_t)end, 0.0);
            for (int t = 0; t < (int)sig.size(); ++t) out[t + offset] += sig[t];
        }
        Vector v(out.size());
        for (std::size_t i = 0; i < out.size(); ++i) v[i] = out[i];
        return make_vec(v);
    };
}

// (wavwrite sig sr path [nch=1])
// Writes a 16-bit PCM WAV. sig is interleaved if nch > 1.
// Peak-normalizes before writing so the loudest sample maps to ±32767.
static Proc fn_wavwrite() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 3 || args.size() > 4)
            throw std::runtime_error("wavwrite expects: sig sr path [nch=1]");
        Vector sig = as_vec(args[0]);
        int sr     = (int)as_scalar(args[1]);
        if (!is_string(args[2])) throw std::runtime_error("wavwrite: path must be a string");
        const std::string& path = as_string(args[2]);
        int nch = (args.size() == 4) ? (int)as_scalar(args[3]) : 1;
        if (nch < 1) throw std::runtime_error("wavwrite: nch must be >= 1");

        int n_samples = (int)sig.size();
        // peak normalize
        double mx = 0.0;
        for (int i = 0; i < n_samples; ++i) mx = std::max(mx, std::abs(sig[i]));
        double scale = (mx > 0.0) ? (32767.0 / mx) : 32767.0;

        std::ofstream f(path, std::ios::binary);
        if (!f) throw std::runtime_error("wavwrite: cannot open " + path);
        uint32_t data_bytes = (uint32_t)(n_samples * 2);
        f.write("RIFF", 4);
        wav_write_le32(f, 36 + data_bytes);
        f.write("WAVE", 4);
        f.write("fmt ", 4);
        wav_write_le32(f, 16);
        wav_write_le16(f, 1);                          // PCM
        wav_write_le16(f, (uint16_t)nch);
        wav_write_le32(f, (uint32_t)sr);
        wav_write_le32(f, (uint32_t)(sr * nch * 2));   // byte rate
        wav_write_le16(f, (uint16_t)(nch * 2));        // block align
        wav_write_le16(f, 16);                         // bits/sample
        f.write("data", 4);
        wav_write_le32(f, data_bytes);
        for (int i = 0; i < n_samples; ++i) {
            int16_t s = (int16_t)std::max(-32768.0, std::min(32767.0, sig[i] * scale));
            f.put((char)(s & 0xff));
            f.put((char)((s >> 8) & 0xff));
        }
        return make_scalar(0.0);
    };
}

// (wavread path) → (list signal sr nch)
// signal: interleaved samples normalized to [-1, 1]
// Supports 8-bit, 16-bit, 24-bit PCM WAV files.
static Proc fn_wavread() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1 || !is_string(args[0]))
            throw std::runtime_error("wavread expects: path");
        std::ifstream f(as_string(args[0]), std::ios::binary);
        if (!f) throw std::runtime_error("wavread: cannot open " + as_string(args[0]));

        char id4[4];
        f.read(id4, 4);
        if (std::memcmp(id4, "RIFF", 4)) throw std::runtime_error("wavread: not a RIFF file");
        wav_read_le32(f); // file size (ignored)
        f.read(id4, 4);
        if (std::memcmp(id4, "WAVE", 4)) throw std::runtime_error("wavread: not a WAVE file");

        int sr = 44100, nch = 1, bits = 16;
        while (f) {
            char chunk[4]; f.read(chunk, 4);
            if (!f) break;
            uint32_t sz = wav_read_le32(f);
            if (!std::memcmp(chunk, "fmt ", 4)) {
                wav_read_le16(f);              // audio format
                nch  = (int)wav_read_le16(f);
                sr   = (int)wav_read_le32(f);
                wav_read_le32(f);              // byte rate
                wav_read_le16(f);              // block align
                bits = (int)wav_read_le16(f);
                if (sz > 16) f.seekg(sz - 16, std::ios::cur);
            } else if (!std::memcmp(chunk, "data", 4)) {
                int bytes_per_sample = bits / 8;
                int n_samples = (int)(sz / (uint32_t)bytes_per_sample);
                Vector sig(n_samples);
                if (bits == 16) {
                    for (int i = 0; i < n_samples; ++i) {
                        uint8_t lo = (uint8_t)f.get(), hi = (uint8_t)f.get();
                        int16_t s = (int16_t)(lo | (hi << 8));
                        sig[i] = s / 32768.0;
                    }
                } else if (bits == 8) {
                    for (int i = 0; i < n_samples; ++i)
                        sig[i] = ((uint8_t)f.get() - 128) / 128.0;
                } else if (bits == 24) {
                    for (int i = 0; i < n_samples; ++i) {
                        uint8_t a=(uint8_t)f.get(), b=(uint8_t)f.get(), c=(uint8_t)f.get();
                        int32_t s = (int32_t)(a | (b<<8) | (c<<16));
                        if (s & 0x800000) s |= (int32_t)0xFF000000;
                        sig[i] = s / 8388608.0;
                    }
                } else {
                    throw std::runtime_error("wavread: unsupported bit depth " + std::to_string(bits));
                }
                Expr::List result;
                result.push_back(make_vec(sig));
                result.push_back(make_scalar((double)sr));
                result.push_back(make_scalar((double)nch));
                return make_list(result);
            } else {
                f.seekg(sz, std::ios::cur);
            }
        }
        throw std::runtime_error("wavread: no data chunk found in " + as_string(args[0]));
    };
}

// (gen n c1 c2 c3 ...) — additive wavetable via gen10
// Generates n+1 samples (with guard point) as a sum of sines:
//   sample[i] = sum_j( cj * sin(2*pi*(j+1)*i/n) ) / sum(cj)
// The guard point (last sample == first sample) is required by osc.
// Example: (gen 4096 1) → pure sine  (gen 4096 1 0.5 0.25) → 3 harmonics
static Proc fn_gen() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() < 2)
            throw std::runtime_error("gen expects: n c1 [c2 ...]");
        int n = (int)as_scalar(args[0]);
        if (n <= 0) throw std::runtime_error("gen: n must be > 0");
        int nh = (int)args.size() - 1;
        std::vector<double> coeffs(nh);
        double norm = 0.0;
        for (int j = 0; j < nh; ++j) {
            coeffs[j] = as_scalar(args[j + 1]);
            norm += std::abs(coeffs[j]);
        }
        if (norm == 0.0) norm = 1.0;
        Vector table(n + 1); // +1 guard point
        for (int i = 0; i < n; ++i) {
            double acc = 0.0;
            for (int j = 0; j < nh; ++j)
                acc += coeffs[j] * std::sin(2.0 * M_PI * (j + 1) * i / n);
            table[i] = acc / norm;
        }
        table[n] = table[0]; // guard point
        return make_vec(table);
    };
}

// ── registration ──────────────────────────────────────────────────────────────

static void add_dsp(std::shared_ptr<Env> env) {
    env->set("gen",          make_proc(fn_gen()));
    env->set("osc",          make_proc(fn_osc()));
    env->set("fft",          make_proc(fn_fft()));
    env->set("ifft",         make_proc(fn_ifft()));
    env->set("car2pol",      make_proc(fn_car2pol()));
    env->set("pol2car",      make_proc(fn_pol2car()));
    env->set("window",       make_proc(fn_window()));
    env->set("conv",         make_proc(fn_conv()));
    env->set("resample",     make_proc(fn_resample()));
    env->set("delay",        make_proc(fn_delay()));
    env->set("comb",         make_proc(fn_comb()));
    env->set("allpass",      make_proc(fn_allpass()));
    env->set("reson",        make_proc(fn_reson()));
    env->set("iir",          make_proc(fn_iir()));
    env->set("iirdesign",    make_proc(fn_iirdesign()));
    env->set("wavread",      make_proc(fn_wavread()));
    env->set("wavwrite",     make_proc(fn_wavwrite()));
    env->set("mix",          make_proc(fn_mix()));
}

// eof
