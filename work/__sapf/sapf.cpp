#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

struct Value {
    enum Type { NUM, SIG } type = NUM;
    double num = 0.0;
    std::vector<double> sig;
};

static Value make_num(double x) {
    Value v;
    v.type = Value::NUM;
    v.num = x;
    return v;
}

static Value make_sig(std::vector<double> x) {
    Value v;
    v.type = Value::SIG;
    v.sig = std::move(x);
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
        vars["out"] = 0.0;
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

    std::vector<double> pop_sig() {
        Value v = pop();
        if (v.type != Value::SIG) throw std::runtime_error("expected signal");
        return v.sig;
    }

    void push(const Value& v) { st.push_back(v); }
    void push_num(double x) { st.push_back(make_num(x)); }

    int sample_rate() const {
        auto it = vars.find("sr");
        if (it == vars.end()) return 44100;
        return static_cast<int>(it->second);
    }

    std::string signal_preview(const std::vector<double>& x) const {
        std::ostringstream ss;
        ss << "[";
        std::size_t n = x.size() < 8 ? x.size() : 8;
        for (std::size_t i = 0; i < n; ++i) {
            if (i) ss << ' ';
            ss << x[i];
        }
        if (x.size() > n) ss << " ...";
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
            if (st[i].type == Value::NUM) {
                std::cout << st[i].num;
            } else {
                std::cout << signal_preview(st[i].sig);
            }
            std::cout << "\n";
        }
    }

    void print_vars() const {
        std::cout << "vars:\n";
        for (const auto& kv : vars) {
            std::cout << "  " << kv.first << " = " << kv.second << "\n";
        }
    }

    static std::vector<double> promote(const Value& v, std::size_t n) {
        if (v.type == Value::SIG) {
            if (v.sig.size() != n) throw std::runtime_error("signal length mismatch");
            return v.sig;
        }
        return std::vector<double>(n, v.num);
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

    static void write_wav(const std::string& path, const std::vector<double>& x, int sr) {
        std::ofstream f(path, std::ios::binary);
        if (!f) throw std::runtime_error("cannot open output wav");

        int32_t data_size = static_cast<int32_t>(x.size() * 2);
        int32_t riff_size = 36 + data_size;
        int16_t fmt = 1, ch = 1, bits = 16, block = 2;
        int32_t byte_rate = sr * block;
        int32_t sub = 16;

        f.write("RIFF", 4);
        f.write(reinterpret_cast<char*>(&riff_size), 4);
        f.write("WAVE", 4);
        f.write("fmt ", 4);
        f.write(reinterpret_cast<char*>(&sub), 4);
        f.write(reinterpret_cast<char*>(&fmt), 2);
        f.write(reinterpret_cast<char*>(&ch), 2);
        f.write(reinterpret_cast<char*>(&sr), 4);
        f.write(reinterpret_cast<char*>(&byte_rate), 4);
        f.write(reinterpret_cast<char*>(&block), 2);
        f.write(reinterpret_cast<char*>(&bits), 2);
        f.write("data", 4);
        f.write(reinterpret_cast<char*>(&data_size), 4);

        for (double s : x) {
            if (s > 1.0) s = 1.0;
            if (s < -1.0) s = -1.0;
            int16_t q = static_cast<int16_t>(std::lrint(s * 32767.0));
            f.write(reinterpret_cast<char*>(&q), 2);
        }
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

    std::vector<double> run_block_signal(double amp, double dur, const std::vector<std::string>& body) {
        int sr = sample_rate();
        std::size_t n = static_cast<std::size_t>(std::max(0.0, dur * sr));
        std::vector<double> out(n, 0.0);

        for (std::size_t k = 0; k < n; ++k) {
            VM inner = *this;
            inner.st.clear();
            double t = static_cast<double>(k) / static_cast<double>(sr);
            double ph = (n > 1) ? static_cast<double>(k) / static_cast<double>(n - 1) : 0.0;
            inner.push_num(amp);
            inner.push_num(t);
            inner.push_num(ph);
            inner.exec_tokens(body);
            out[k] = inner.pop_num();
        }

        return out;
    }

    std::vector<double> make_env_const(double amp, double dur) {
        int sr = sample_rate();
        std::size_t n = static_cast<std::size_t>(std::max(0.0, dur * sr));
        return std::vector<double>(n, amp);
    }

    std::vector<double> make_env_tri(double amp, double dur) {
        int sr = sample_rate();
        std::size_t n = static_cast<std::size_t>(std::max(0.0, dur * sr));
        std::vector<double> r(n);
        for (std::size_t k = 0; k < n; ++k) {
            double ph = (n > 1) ? static_cast<double>(k) / static_cast<double>(n - 1) : 0.0;
            r[k] = amp * (1.0 - std::fabs(2.0 * ph - 1.0));
        }
        return r;
    }

    std::vector<double> make_env_up(double amp, double dur) {
        int sr = sample_rate();
        std::size_t n = static_cast<std::size_t>(std::max(0.0, dur * sr));
        std::vector<double> r(n);
        for (std::size_t k = 0; k < n; ++k) {
            double ph = (n > 1) ? static_cast<double>(k) / static_cast<double>(n - 1) : 1.0;
            r[k] = amp * ph;
        }
        return r;
    }

    std::vector<double> make_env_down(double amp, double dur) {
        int sr = sample_rate();
        std::size_t n = static_cast<std::size_t>(std::max(0.0, dur * sr));
        std::vector<double> r(n);
        for (std::size_t k = 0; k < n; ++k) {
            double ph = (n > 1) ? static_cast<double>(k) / static_cast<double>(n - 1) : 0.0;
            r[k] = amp * (1.0 - ph);
        }
        return r;
    }

    std::vector<double> make_adsr(double amp, double dur, double a, double d, double s, double r) {
        int sr = sample_rate();
        std::size_t n = static_cast<std::size_t>(std::max(0.0, dur * sr));
        std::vector<double> out(n, 0.0);

        std::size_t na = static_cast<std::size_t>(std::max(0.0, a * sr));
        std::size_t nd = static_cast<std::size_t>(std::max(0.0, d * sr));
        std::size_t nr = static_cast<std::size_t>(std::max(0.0, r * sr));
        std::size_t ns = (n > na + nd + nr) ? (n - na - nd - nr) : 0;

        std::size_t idx = 0;
        for (std::size_t i = 0; i < na && idx < n; ++i, ++idx) {
            double ph = (na > 1) ? static_cast<double>(i) / static_cast<double>(na - 1) : 1.0;
            out[idx] = amp * ph;
        }
        for (std::size_t i = 0; i < nd && idx < n; ++i, ++idx) {
            double ph = (nd > 1) ? static_cast<double>(i) / static_cast<double>(nd - 1) : 1.0;
            out[idx] = amp * (1.0 + (s - 1.0) * ph);
        }
        for (std::size_t i = 0; i < ns && idx < n; ++i, ++idx) {
            out[idx] = amp * s;
        }
        double start_rel = (idx > 0) ? (out[idx - 1] / amp) : s;
        for (std::size_t i = 0; i < nr && idx < n; ++i, ++idx) {
            double ph = (nr > 1) ? static_cast<double>(i) / static_cast<double>(nr - 1) : 1.0;
            out[idx] = amp * start_rel * (1.0 - ph);
        }
        while (idx < n) {
            out[idx++] = 0.0;
        }
        return out;
    }

    std::vector<double> make_bpf(double dur, const std::vector<double>& pts) {
        if (pts.size() < 4 || (pts.size() % 2) != 0) {
            throw std::runtime_error("bpf expects x y pairs (at least 2 points)");
        }

        int sr = sample_rate();
        std::size_t n = static_cast<std::size_t>(std::max(0.0, dur * sr));
        std::vector<double> out(n, 0.0);

        for (std::size_t k = 0; k < n; ++k) {
            double ph = (n > 1) ? static_cast<double>(k) / static_cast<double>(n - 1) : 0.0;
            double y = pts[1];

            if (ph <= pts[0]) {
                y = pts[1];
            } else if (ph >= pts[pts.size() - 2]) {
                y = pts[pts.size() - 1];
            } else {
                for (std::size_t i = 0; i + 3 < pts.size(); i += 2) {
                    double x0 = pts[i];
                    double y0 = pts[i + 1];
                    double x1 = pts[i + 2];
                    double y1 = pts[i + 3];
                    if (ph >= x0 && ph <= x1) {
                        double u = (x1 > x0) ? ((ph - x0) / (x1 - x0)) : 0.0;
                        y = y0 + (y1 - y0) * u;
                        break;
                    }
                }
            }
            out[k] = y;
        }

        return out;
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

            if (!body.empty() && body[0] == "var") {
                if (body.size() != 2 || !is_number(body[1])) {
                    throw std::runtime_error("variable definition must be ': name var number ;'");
                }
                vars[name] = std::stod(body[1]);
            } else {
                words[name] = body;
            }
            return;
        }

        if (t == "{") {
            double dur = pop_num();
            double amp = pop_num();
            auto body = collect_braced_body(tok, i);
            push(make_sig(run_block_signal(amp, dur, body)));
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

        if (t == "var") return;
        if (t == "t") return;
        if (t == "ph") return;
        if (t == "amp") return;

        if (t == "dup") { Value v = pop(); push(v); push(v); return; }
        if (t == "swap") { Value b = pop(); Value a = pop(); push(b); push(a); return; }
        if (t == "drop") { (void)pop(); return; }

        if (t == ".s") { print_stack(); return; }
        if (t == ".v") { print_vars(); return; }

        if (t == "+" || t == "-" || t == "*" || t == "/") {
            Value b = pop();
            Value a = pop();

            if (a.type == Value::NUM && b.type == Value::NUM) {
                if (t == "+") push_num(a.num + b.num);
                else if (t == "-") push_num(a.num - b.num);
                else if (t == "*") push_num(a.num * b.num);
                else push_num(a.num / b.num);
                return;
            }

            std::size_t n = (a.type == Value::SIG) ? a.sig.size() : b.sig.size();
            std::vector<double> av = promote(a, n);
            std::vector<double> bv = promote(b, n);
            std::vector<double> r(n);
            for (std::size_t k = 0; k < n; ++k) {
                if (t == "+") r[k] = av[k] + bv[k];
                else if (t == "-") r[k] = av[k] - bv[k];
                else if (t == "*") r[k] = av[k] * bv[k];
                else r[k] = av[k] / bv[k];
            }
            push(make_sig(std::move(r)));
            return;
        }

        if (t == "pi") { push_num(3.14159265358979323846); return; }
        if (t == "sin") { push_num(std::sin(pop_num())); return; }
        if (t == "cos") { push_num(std::cos(pop_num())); return; }

        if (t == "env-") {
            double dur = pop_num();
            double amp = pop_num();
            push(make_sig(make_env_const(amp, dur)));
            return;
        }
        if (t == "env<>") {
            double dur = pop_num();
            double amp = pop_num();
            push(make_sig(make_env_tri(amp, dur)));
            return;
        }
        if (t == "env<") {
            double dur = pop_num();
            double amp = pop_num();
            push(make_sig(make_env_up(amp, dur)));
            return;
        }
        if (t == "env>") {
            double dur = pop_num();
            double amp = pop_num();
            push(make_sig(make_env_down(amp, dur)));
            return;
        }

        if (t == "adsr") {
            double r = pop_num();
            double s = pop_num();
            double d = pop_num();
            double a = pop_num();
            double dur = pop_num();
            double amp = pop_num();
            push(make_sig(make_adsr(amp, dur, a, d, s, r)));
            return;
        }

        if (t == "bpf") {
            double dur = pop_num();
            double npts = pop_num();
            int n = static_cast<int>(npts);
            if (n < 2) throw std::runtime_error("bpf needs at least 2 points");
            std::vector<double> pts(static_cast<std::size_t>(2 * n));
            for (int i2 = 2 * n - 1; i2 >= 0; --i2) pts[static_cast<std::size_t>(i2)] = pop_num();
            push(make_sig(make_bpf(dur, pts)));
            return;
        }

        if (t == "osc") {
            Value f = pop();
            Value a = pop();
            std::size_t n = 0;
            if (f.type == Value::SIG) n = f.sig.size();
            else if (a.type == Value::SIG) n = a.sig.size();
            else throw std::runtime_error("osc expects at least one signal input");

            std::vector<double> fv = promote(f, n);
            std::vector<double> av = promote(a, n);
            std::vector<double> out(n);
            double phase = 0.0;
            int sr = sample_rate();
            for (std::size_t k = 0; k < n; ++k) {
                phase += 2.0 * 3.14159265358979323846 * fv[k] / static_cast<double>(sr);
                out[k] = std::sin(phase) * av[k];
            }
            push(make_sig(std::move(out)));
            return;
        }

        if (t == "vmul") {
            auto b = pop_sig();
            auto a = pop_sig();
            if (a.size() != b.size()) throw std::runtime_error("vmul size mismatch");
            for (std::size_t k = 0; k < a.size(); ++k) a[k] *= b[k];
            push(make_sig(std::move(a)));
            return;
        }

        if (t == "vadd") {
            auto b = pop_sig();
            auto a = pop_sig();
            if (a.size() != b.size()) throw std::runtime_error("vadd size mismatch");
            for (std::size_t k = 0; k < a.size(); ++k) a[k] += b[k];
            push(make_sig(std::move(a)));
            return;
        }

        if (t == "mix") {
            int n = static_cast<int>(pop_num());
            if (n <= 0) throw std::runtime_error("mix expects positive count");
            std::vector<std::vector<double>> xs;
            xs.reserve(static_cast<std::size_t>(n));
            for (int j = 0; j < n; ++j) xs.push_back(pop_sig());
            std::size_t m = xs[0].size();
            std::vector<double> out(m, 0.0);
            for (int j = 0; j < n; ++j) {
                if (xs[static_cast<std::size_t>(j)].size() != m) throw std::runtime_error("mix size mismatch");
                for (std::size_t k = 0; k < m; ++k) out[k] += xs[static_cast<std::size_t>(j)][k];
            }
            push(make_sig(std::move(out)));
            return;
        }

        if (t == "concat") {
            auto b = pop_sig();
            auto a = pop_sig();
            a.insert(a.end(), b.begin(), b.end());
            push(make_sig(std::move(a)));
            return;
        }

        if (t == "delay") {
            double d = pop_num();
            auto x = pop_sig();
            int sr = sample_rate();
            std::size_t n0 = static_cast<std::size_t>(std::max(0.0, d * sr));
            std::vector<double> out(n0, 0.0);
            out.insert(out.end(), x.begin(), x.end());
            push(make_sig(std::move(out)));
            return;
        }

        if (t == "render") {
            auto x = pop_sig();
            std::string path = "out.wav";
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
    std::cout << "sap repl — type words, Ctrl-D to exit\n";
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
        if (argc <= 1) {
            repl(vm);
        } else {
            for (int i = 1; i < argc; ++i) run_file(vm, argv[i]);
        }
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
