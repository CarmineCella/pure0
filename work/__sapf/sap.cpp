#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

struct Value {
    enum Type { NUM, SIG, STR } type = NUM;
    double num = 0.0;
    std::vector<std::vector<double>> sig;
    std::string str;
};

static Value make_num(double x) {
    Value v;
    v.type = Value::NUM;
    v.num = x;
    return v;
}

static Value make_sig(std::vector<std::vector<double>> x) {
    Value v;
    v.type = Value::SIG;
    v.sig = std::move(x);
    return v;
}

static Value make_str(const std::string& s) {
    Value v;
    v.type = Value::STR;
    v.str = s;
    return v;
}

static bool is_number(const std::string& s) {
    char* end = nullptr;
    std::strtod(s.c_str(), &end);
    return end && *end == '\0';
}

static std::vector<std::string> tokenize(const std::string& src) {
    std::vector<std::string> t;
    std::string cur;

    auto flush_cur = [&]() {
        if (!cur.empty()) {
            t.push_back(cur);
            cur.clear();
        }
    };

    for (std::size_t i = 0; i < src.size(); ++i) {
        char c = src[i];

        if (c == '\\') {
            flush_cur();
            while (i < src.size() && src[i] != '\n') ++i;
            continue;
        }

        if (c == '"') {
            flush_cur();
            std::string s;
            ++i;
            while (i < src.size() && src[i] != '"') {
                if (src[i] == '\\' && i + 1 < src.size()) {
                    ++i;
                    char e = src[i];
                    if (e == 'n') s.push_back('\n');
                    else if (e == 't') s.push_back('\t');
                    else s.push_back(e);
                } else {
                    s.push_back(src[i]);
                }
                ++i;
            }
            t.push_back("\"" + s + "\"");
            continue;
        }

        if (std::isspace(static_cast<unsigned char>(c))) {
            flush_cur();
            continue;
        }

        if (c == '{' || c == '}' || c == ':' || c == ';') {
            flush_cur();
            t.push_back(std::string(1, c));
            continue;
        }

        cur.push_back(c);
    }

    flush_cur();
    return t;
}

struct VM {
    std::unordered_map<std::string, std::vector<std::string>> words;
    std::unordered_map<std::string, double> vars;
    std::vector<Value> st;

    VM() {
        vars["sr"] = 44100.0;
        vars["ch"] = 1.0;
    }

    Value pop() {
        if (st.empty()) throw std::runtime_error("stack underflow");
        Value v = st.back();
        st.pop_back();
        return v;
    }

    double pop_num() {
        Value v = pop();
        if (v.type != Value::NUM) throw std::runtime_error("expected number");
        return v.num;
    }

    std::string pop_str() {
        Value v = pop();
        if (v.type != Value::STR) throw std::runtime_error("expected string");
        return v.str;
    }

    std::vector<std::vector<double>> pop_sig() {
        Value v = pop();
        if (v.type != Value::SIG) throw std::runtime_error("expected signal");
        return v.sig;
    }

    void push(const Value& v) { st.push_back(v); }
    void push_num(double x) { st.push_back(make_num(x)); }
    void push_str(const std::string& s) { st.push_back(make_str(s)); }

    int sample_rate() const {
        auto it = vars.find("sr");
        if (it == vars.end()) return 44100;
        return static_cast<int>(it->second);
    }

    int channels() const {
        auto it = vars.find("ch");
        if (it == vars.end()) return 1;
        return static_cast<int>(it->second);
    }

    static bool is_string_token(const std::string& s) {
        return s.size() >= 2 && s.front() == '"' && s.back() == '"';
    }

    std::string signal_preview(const std::vector<std::vector<double>>& x) const {
        std::ostringstream ss;
        ss << "[";
        if (!x.empty()) {
            std::size_t n = x[0].size() < 6 ? x[0].size() : 6;
            for (std::size_t i = 0; i < n; ++i) {
                if (i) ss << ' ';
                ss << x[0][i];
            }
            if (x[0].size() > n) ss << " ...";
            if (x.size() > 1) ss << " |ch=" << x.size();
        }
        ss << "]";
        return ss.str();
    }

    void print_stack() const {
        std::cout << "stack(" << st.size() << "):";
        if (st.empty()) {
            std::cout << " <empty>\n";
            return;
        }
        std::cout << "\n";
        for (std::size_t i = 0; i < st.size(); ++i) {
            std::cout << "  [" << i << "] ";
            if (st[i].type == Value::NUM) std::cout << st[i].num;
            else if (st[i].type == Value::STR) std::cout << '"' << st[i].str << '"';
            else std::cout << signal_preview(st[i].sig);
            std::cout << "\n";
        }
    }

    void print_vars() const {
        std::cout << "vars:\n";
        for (const auto& kv : vars) {
            std::cout << "  " << kv.first << " = " << kv.second << "\n";
        }
        std::cout << "words:\n";
        for (const auto& kv : words) {
            std::cout << "  " << kv.first << "\n";
        }
    }

    static std::vector<std::vector<double>> promote(const Value& v, std::size_t ch, std::size_t n) {
        if (v.type == Value::SIG) {
            if (v.sig.size() != ch) throw std::runtime_error("channel mismatch");
            for (std::size_t c = 0; c < ch; ++c) {
                if (v.sig[c].size() != n) throw std::runtime_error("signal length mismatch");
            }
            return v.sig;
        }
        return std::vector<std::vector<double>>(ch, std::vector<double>(n, v.num));
    }

    void exec_tokens(const std::vector<std::string>& tok) {
        std::size_t i = 0;
        while (i < tok.size()) exec_one(tok, i);
    }

    void exec_word(const std::string& w) {
        auto it = words.find(w);
        if (it == words.end()) throw std::runtime_error("unknown word: " + w);
        exec_tokens(it->second);
    }

    static void write_wav(const std::string& path, const std::vector<std::vector<double>>& x, int sr) {
        if (x.empty()) throw std::runtime_error("cannot write empty signal");
        std::size_t ch = x.size();
        std::size_t n = x[0].size();
        for (std::size_t c = 1; c < ch; ++c) {
            if (x[c].size() != n) throw std::runtime_error("wav_write channel size mismatch");
        }

        std::ofstream f(path, std::ios::binary);
        if (!f) throw std::runtime_error("cannot open output wav");

        int16_t fmt = 1;
        int16_t channels = static_cast<int16_t>(ch);
        int16_t bits = 16;
        int16_t block = static_cast<int16_t>(channels * bits / 8);
        int32_t byte_rate = sr * block;
        int32_t data_size = static_cast<int32_t>(n * block);
        int32_t riff_size = 36 + data_size;
        int32_t sub = 16;

        f.write("RIFF", 4);
        f.write(reinterpret_cast<char*>(&riff_size), 4);
        f.write("WAVE", 4);
        f.write("fmt ", 4);
        f.write(reinterpret_cast<char*>(&sub), 4);
        f.write(reinterpret_cast<char*>(&fmt), 2);
        f.write(reinterpret_cast<char*>(&channels), 2);
        f.write(reinterpret_cast<char*>(&sr), 4);
        f.write(reinterpret_cast<char*>(&byte_rate), 4);
        f.write(reinterpret_cast<char*>(&block), 2);
        f.write(reinterpret_cast<char*>(&bits), 2);
        f.write("data", 4);
        f.write(reinterpret_cast<char*>(&data_size), 4);

        for (std::size_t i = 0; i < n; ++i) {
            for (std::size_t c = 0; c < ch; ++c) {
                double s = x[c][i];
                if (s > 1.0) s = 1.0;
                if (s < -1.0) s = -1.0;
                int16_t q = static_cast<int16_t>(std::lrint(s * 32767.0));
                f.write(reinterpret_cast<char*>(&q), 2);
            }
        }
    }

    static std::vector<std::vector<double>> read_wav(const std::string& path) {
        std::ifstream f(path, std::ios::binary);
        if (!f) throw std::runtime_error("cannot open input wav");
        char riff[4], wave[4], fmt_[4], data_[4];
        int32_t riff_size = 0, sub = 0, data_size = 0, sr = 0, byte_rate = 0;
        int16_t fmt = 0, ch = 0, block = 0, bits = 0;

        f.read(riff, 4);
        f.read(reinterpret_cast<char*>(&riff_size), 4);
        f.read(wave, 4);
        f.read(fmt_, 4);
        f.read(reinterpret_cast<char*>(&sub), 4);
        f.read(reinterpret_cast<char*>(&fmt), 2);
        f.read(reinterpret_cast<char*>(&ch), 2);
        f.read(reinterpret_cast<char*>(&sr), 4);
        f.read(reinterpret_cast<char*>(&byte_rate), 4);
        f.read(reinterpret_cast<char*>(&block), 2);
        f.read(reinterpret_cast<char*>(&bits), 2);

        if (std::string(riff, 4) != "RIFF" || std::string(wave, 4) != "WAVE") {
            throw std::runtime_error("unsupported wav");
        }

        if (sub > 16) f.seekg(sub - 16, std::ios::cur);

        while (true) {
            if (!f.read(data_, 4)) throw std::runtime_error("wav data chunk not found");
            f.read(reinterpret_cast<char*>(&data_size), 4);
            if (std::string(data_, 4) == "data") break;
            f.seekg(data_size, std::ios::cur);
        }

        if (fmt != 1 || bits != 16) throw std::runtime_error("only 16-bit PCM wav supported");

        std::size_t n = static_cast<std::size_t>(data_size / block);
        std::vector<std::vector<double>> out(static_cast<std::size_t>(ch), std::vector<double>(n, 0.0));
        for (std::size_t i = 0; i < n; ++i) {
            for (int c = 0; c < ch; ++c) {
                int16_t q = 0;
                f.read(reinterpret_cast<char*>(&q), 2);
                out[static_cast<std::size_t>(c)][i] = static_cast<double>(q) / 32768.0;
            }
        }
        return out;
    }

    std::vector<std::string> collect_braced_body(const std::vector<std::string>& tok, std::size_t& i) {
        std::vector<std::string> body;
        int depth = 1;
        while (i < tok.size() && depth > 0) {
            if (tok[i] == "{") depth++;
            else if (tok[i] == "}") depth--;
            if (depth > 0) body.push_back(tok[i]);
            ++i;
        }
        if (depth != 0) throw std::runtime_error("unclosed { ... }");
        return body;
    }

    std::vector<std::vector<double>> run_block_signal(double amp, double dur, const std::vector<std::string>& body) {
        int sr = sample_rate();
        int ch = channels();
        std::size_t n = static_cast<std::size_t>(std::max(0.0, dur * sr));
        std::vector<std::vector<double>> out(static_cast<std::size_t>(ch), std::vector<double>(n, 0.0));

        for (std::size_t k = 0; k < n; ++k) {
            VM inner = *this;
            inner.st.clear();
            double t = static_cast<double>(k) / static_cast<double>(sr);
            double ph = (n > 1) ? static_cast<double>(k) / static_cast<double>(n - 1) : 0.0;
            inner.vars["amp"] = amp;
            inner.vars["t"] = t;
            inner.vars["ph"] = ph;
            inner.exec_tokens(body);
            Value v = inner.pop();

            if (v.type == Value::NUM) {
                for (int c = 0; c < ch; ++c) out[static_cast<std::size_t>(c)][k] = v.num;
            } else if (v.type == Value::SIG) {
                if (static_cast<int>(v.sig.size()) != ch) throw std::runtime_error("block returned wrong number of channels");
                for (int c = 0; c < ch; ++c) {
                    if (v.sig[static_cast<std::size_t>(c)].size() != 1) {
                        throw std::runtime_error("block signal return must be single-sample per channel");
                    }
                    out[static_cast<std::size_t>(c)][k] = v.sig[static_cast<std::size_t>(c)][0];
                }
            } else {
                throw std::runtime_error("block must leave a number on stack");
            }
        }

        return out;
    }

    std::vector<std::vector<double>> make_env_const(double amp, double dur) {
        int sr = sample_rate();
        int ch = channels();
        std::size_t n = static_cast<std::size_t>(std::max(0.0, dur * sr));
        return std::vector<std::vector<double>>(static_cast<std::size_t>(ch), std::vector<double>(n, amp));
    }

    std::vector<std::vector<double>> make_env_tri(double amp, double dur) {
        int sr = sample_rate();
        int ch = channels();
        std::size_t n = static_cast<std::size_t>(std::max(0.0, dur * sr));
        std::vector<double> mono(n);
        for (std::size_t k = 0; k < n; ++k) {
            double ph = (n > 1) ? static_cast<double>(k) / static_cast<double>(n - 1) : 0.0;
            mono[k] = amp * (1.0 - std::fabs(2.0 * ph - 1.0));
        }
        return std::vector<std::vector<double>>(static_cast<std::size_t>(ch), mono);
    }

    std::vector<std::vector<double>> make_env_up(double amp, double dur) {
        int sr = sample_rate();
        int ch = channels();
        std::size_t n = static_cast<std::size_t>(std::max(0.0, dur * sr));
        std::vector<double> mono(n);
        for (std::size_t k = 0; k < n; ++k) {
            double ph = (n > 1) ? static_cast<double>(k) / static_cast<double>(n - 1) : 1.0;
            mono[k] = amp * ph;
        }
        return std::vector<std::vector<double>>(static_cast<std::size_t>(ch), mono);
    }

    std::vector<std::vector<double>> make_env_down(double amp, double dur) {
        int sr = sample_rate();
        int ch = channels();
        std::size_t n = static_cast<std::size_t>(std::max(0.0, dur * sr));
        std::vector<double> mono(n);
        for (std::size_t k = 0; k < n; ++k) {
            double ph = (n > 1) ? static_cast<double>(k) / static_cast<double>(n - 1) : 0.0;
            mono[k] = amp * (1.0 - ph);
        }
        return std::vector<std::vector<double>>(static_cast<std::size_t>(ch), mono);
    }

    std::vector<std::vector<double>> make_adsr(double amp, double dur, double a, double d, double s, double r) {
        int sr = sample_rate();
        int ch = channels();
        std::size_t n = static_cast<std::size_t>(std::max(0.0, dur * sr));
        std::vector<double> mono(n, 0.0);

        std::size_t na = static_cast<std::size_t>(std::max(0.0, a * sr));
        std::size_t nd = static_cast<std::size_t>(std::max(0.0, d * sr));
        std::size_t nr = static_cast<std::size_t>(std::max(0.0, r * sr));
        std::size_t ns = (n > na + nd + nr) ? (n - na - nd - nr) : 0;

        std::size_t idx = 0;
        for (std::size_t i = 0; i < na && idx < n; ++i, ++idx) {
            double ph = (na > 1) ? static_cast<double>(i) / static_cast<double>(na - 1) : 1.0;
            mono[idx] = amp * ph;
        }
        for (std::size_t i = 0; i < nd && idx < n; ++i, ++idx) {
            double ph = (nd > 1) ? static_cast<double>(i) / static_cast<double>(nd - 1) : 1.0;
            mono[idx] = amp * (1.0 + (s - 1.0) * ph);
        }
        for (std::size_t i = 0; i < ns && idx < n; ++i, ++idx) mono[idx] = amp * s;
        double start_rel = (idx > 0 && amp != 0.0) ? (mono[idx - 1] / amp) : s;
        for (std::size_t i = 0; i < nr && idx < n; ++i, ++idx) {
            double ph = (nr > 1) ? static_cast<double>(i) / static_cast<double>(nr - 1) : 1.0;
            mono[idx] = amp * start_rel * (1.0 - ph);
        }
        while (idx < n) mono[idx++] = 0.0;
        return std::vector<std::vector<double>>(static_cast<std::size_t>(ch), mono);
    }

    std::vector<std::vector<double>> make_bpf(double dur, const std::vector<double>& pts) {
        if (pts.size() < 4 || (pts.size() % 2) != 0) throw std::runtime_error("bpf expects x y pairs");
        int sr = sample_rate();
        int ch = channels();
        std::size_t n = static_cast<std::size_t>(std::max(0.0, dur * sr));
        std::vector<double> mono(n, 0.0);
        for (std::size_t k = 0; k < n; ++k) {
            double ph = (n > 1) ? static_cast<double>(k) / static_cast<double>(n - 1) : 0.0;
            double y = pts[1];
            if (ph <= pts[0]) y = pts[1];
            else if (ph >= pts[pts.size() - 2]) y = pts[pts.size() - 1];
            else {
                for (std::size_t i = 0; i + 3 < pts.size(); i += 2) {
                    double x0 = pts[i], y0 = pts[i + 1], x1 = pts[i + 2], y1 = pts[i + 3];
                    if (ph >= x0 && ph <= x1) {
                        double u = (x1 > x0) ? ((ph - x0) / (x1 - x0)) : 0.0;
                        y = y0 + (y1 - y0) * u;
                        break;
                    }
                }
            }
            mono[k] = y;
        }
        return std::vector<std::vector<double>>(static_cast<std::size_t>(ch), mono);
    }

    void exec_one(const std::vector<std::string>& tok, std::size_t& i) {
        const std::string t = tok[i++];

        if (t == ":") {
            if (i >= tok.size()) throw std::runtime_error("expected word name after :");
            std::string name = tok[i++];
            std::vector<std::string> body;
            while (i < tok.size() && tok[i] != ";") body.push_back(tok[i++]);
            if (i >= tok.size()) throw std::runtime_error("missing ;");
            ++i;
            words[name] = body;
            return;
        }

        if (t == "{") {
            double dur = pop_num();
            double amp = pop_num();
            auto body = collect_braced_body(tok, i);
            push(make_sig(run_block_signal(amp, dur, body)));
            return;
        }

        if (is_string_token(t)) {
            push_str(t.substr(1, t.size() - 2));
            return;
        }

        if (is_number(t)) {
            push_num(std::stod(t));
            return;
        }

        auto vit = vars.find(t);
        if (vit != vars.end()) {
            push_num(vit->second);
            return;
        }

        if (t == "dup") { Value v = pop(); push(v); push(v); return; }
        if (t == "swap") { Value b = pop(); Value a = pop(); push(b); push(a); return; }
        if (t == "drop") { (void)pop(); return; }
        if (t == "def") { double x = pop_num(); std::string n = pop_str(); vars[n] = x; return; }
        if (t == ".") {
            Value v = pop();
            if (v.type == Value::NUM) std::cout << v.num << "\n";
            else if (v.type == Value::STR) std::cout << v.str << "\n";
            else std::cout << signal_preview(v.sig) << "\n";
            return;
        }
        if (t == ".s") { print_stack(); return; }
        if (t == ".v") { print_vars(); return; }

        if (t == "+" || t == "-" || t == "*" || t == "/") {
            Value b = pop(), a = pop();
            if (a.type == Value::NUM && b.type == Value::NUM) {
                if (t == "+") push_num(a.num + b.num);
                else if (t == "-") push_num(a.num - b.num);
                else if (t == "*") push_num(a.num * b.num);
                else push_num(a.num / b.num);
                return;
            }
            if (a.type == Value::STR || b.type == Value::STR) {
                if (t != "+") throw std::runtime_error("only + allowed on strings");
                std::string sa = (a.type == Value::STR) ? a.str : std::to_string(a.num);
                std::string sb = (b.type == Value::STR) ? b.str : std::to_string(b.num);
                push_str(sa + sb);
                return;
            }
            std::size_t ch = (a.type == Value::SIG) ? a.sig.size() : b.sig.size();
            std::size_t n = (a.type == Value::SIG) ? a.sig[0].size() : b.sig[0].size();
            auto av = promote(a, ch, n);
            auto bv = promote(b, ch, n);
            std::vector<std::vector<double>> r(ch, std::vector<double>(n, 0.0));
            for (std::size_t c = 0; c < ch; ++c) {
                for (std::size_t k = 0; k < n; ++k) {
                    if (t == "+") r[c][k] = av[c][k] + bv[c][k];
                    else if (t == "-") r[c][k] = av[c][k] - bv[c][k];
                    else if (t == "*") r[c][k] = av[c][k] * bv[c][k];
                    else r[c][k] = av[c][k] / bv[c][k];
                }
            }
            push(make_sig(std::move(r)));
            return;
        }

        if (t == "pi") { push_num(3.14159265358979323846); return; }
        if (t == "sin") { push_num(std::sin(pop_num())); return; }
        if (t == "cos") { push_num(std::cos(pop_num())); return; }

        if (t == "env-") { double dur = pop_num(); double amp = pop_num(); push(make_sig(make_env_const(amp, dur))); return; }
        if (t == "env<>") { double dur = pop_num(); double amp = pop_num(); push(make_sig(make_env_tri(amp, dur))); return; }
        if (t == "env<") { double dur = pop_num(); double amp = pop_num(); push(make_sig(make_env_up(amp, dur))); return; }
        if (t == "env>") { double dur = pop_num(); double amp = pop_num(); push(make_sig(make_env_down(amp, dur))); return; }

        if (t == "adsr") {
            double r = pop_num(), s = pop_num(), d = pop_num(), a = pop_num(), dur = pop_num(), amp = pop_num();
            push(make_sig(make_adsr(amp, dur, a, d, s, r)));
            return;
        }

        if (t == "bpf") {
            double dur = pop_num();
            int npts = static_cast<int>(pop_num());
            if (npts < 2) throw std::runtime_error("bpf needs at least 2 points");
            std::vector<double> pts(static_cast<std::size_t>(2 * npts));
            for (int j = 2 * npts - 1; j >= 0; --j) pts[static_cast<std::size_t>(j)] = pop_num();
            push(make_sig(make_bpf(dur, pts)));
            return;
        }

        if (t == "osc") {
            Value f = pop();
            std::size_t ch = (f.type == Value::SIG) ? f.sig.size() : static_cast<std::size_t>(channels());
            std::size_t n = (f.type == Value::SIG) ? f.sig[0].size() : 0;
            if (n == 0) throw std::runtime_error("osc expects a frequency signal");
            auto fv = promote(f, ch, n);
            std::vector<std::vector<double>> out(ch, std::vector<double>(n, 0.0));
            int sr = sample_rate();
            for (std::size_t c = 0; c < ch; ++c) {
                double phase = 0.0;
                for (std::size_t k = 0; k < n; ++k) {
                    phase += 2.0 * 3.14159265358979323846 * fv[c][k] / static_cast<double>(sr);
                    out[c][k] = std::sin(phase);
                }
            }
            push(make_sig(std::move(out)));
            return;
        }

        if (t == "vmul" || t == "vadd") {
            auto b = pop_sig();
            auto a = pop_sig();
            if (a.size() != b.size()) throw std::runtime_error("channel mismatch");
            if (a.empty()) throw std::runtime_error("empty signals");
            if (a[0].size() != b[0].size()) throw std::runtime_error(std::string(t) + " size mismatch");
            for (std::size_t c = 0; c < a.size(); ++c) {
                for (std::size_t k = 0; k < a[c].size(); ++k) {
                    if (t == "vmul") a[c][k] *= b[c][k];
                    else a[c][k] += b[c][k];
                }
            }
            push(make_sig(std::move(a)));
            return;
        }

        if (t == "mix") {
            int n = static_cast<int>(pop_num());
            if (n <= 0) throw std::runtime_error("mix expects positive count");
            std::vector<std::vector<std::vector<double>>> xs;
            xs.reserve(static_cast<std::size_t>(n));
            for (int j = 0; j < n; ++j) xs.push_back(pop_sig());
            std::size_t ch = xs[0].size();
            std::size_t m = xs[0][0].size();
            std::vector<std::vector<double>> out(ch, std::vector<double>(m, 0.0));
            for (int j = 0; j < n; ++j) {
                if (xs[static_cast<std::size_t>(j)].size() != ch) throw std::runtime_error("mix channel mismatch");
                for (std::size_t c = 0; c < ch; ++c) {
                    if (xs[static_cast<std::size_t>(j)][c].size() != m) throw std::runtime_error("mix size mismatch");
                    for (std::size_t k = 0; k < m; ++k) out[c][k] += xs[static_cast<std::size_t>(j)][c][k];
                }
            }
            push(make_sig(std::move(out)));
            return;
        }

        if (t == "concat") {
            auto b = pop_sig();
            auto a = pop_sig();
            if (a.size() != b.size()) throw std::runtime_error("concat channel mismatch");
            for (std::size_t c = 0; c < a.size(); ++c) a[c].insert(a[c].end(), b[c].begin(), b[c].end());
            push(make_sig(std::move(a)));
            return;
        }

        if (t == "delay") {
            double d = pop_num();
            auto x = pop_sig();
            int sr = sample_rate();
            std::size_t n0 = static_cast<std::size_t>(std::max(0.0, d * sr));
            for (std::size_t c = 0; c < x.size(); ++c) x[c].insert(x[c].begin(), n0, 0.0);
            push(make_sig(std::move(x)));
            return;
        }

        if (t == "stereo") {
            auto r = pop_sig();
            auto l = pop_sig();
            if (l.size() != 1 || r.size() != 1) throw std::runtime_error("stereo expects two mono signals");
            if (l[0].size() != r[0].size()) throw std::runtime_error("stereo size mismatch");
            std::vector<std::vector<double>> out(2);
            out[0] = std::move(l[0]);
            out[1] = std::move(r[0]);
            push(make_sig(std::move(out)));
            return;
        }

        if (t == "wavread") {
            std::string path = pop_str();
            push(make_sig(read_wav(path)));
            return;
        }

        if (t == "render") {
            Value top = pop();
            std::string path = "out.wav";
            std::vector<std::vector<double>> x;
            if (top.type == Value::STR) {
                path = top.str;
                x = pop_sig();
            } else if (top.type == Value::SIG) {
                x = top.sig;
            } else {
                throw std::runtime_error("render expects signal or signal plus filename");
            }
            write_wav(path, x, sample_rate());
            std::cout << "wrote " << path << "\n";
            return;
        }

        exec_word(t);
    }
};

static void run_file(VM& vm, const std::string& path) {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("cannot open input file: " + path);
    std::stringstream ss;
    ss << f.rdbuf();
    vm.exec_tokens(tokenize(ss.str()));
}

static void repl(VM& vm) {
    std::cout << "pure0 repl — type words, Ctrl-D to exit\n";
    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        try {
            vm.exec_tokens(tokenize(line));
        } catch (const std::exception& e) {
            std::cerr << "error: " << e.what() << "\n";
        }
    }
}

int main(int argc, char** argv) {
    VM vm;
    try {
        if (argc <= 1) repl(vm);
        else for (int i = 1; i < argc; ++i) run_file(vm, argv[i]);
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
