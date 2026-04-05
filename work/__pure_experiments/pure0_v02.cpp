#include <cmath>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <valarray>
#include <variant>
#include <vector>

using namespace std;
using Vector = valarray<double>;
using Val = variant<double, Vector>;

struct VM;

struct Word {
    bool builtin = false;
    function<void(VM&)> fn;
    vector<string> body;
};

struct VM {
    vector<Val> st;
    unordered_map<string, Word> dict;

    VM() {
        dict["sr"] = Word{false, {}, {"44100"}};
        add_stack();
        add_math();
        add_core();
    }

    static bool numtok(const string& s) {
        char* e = nullptr;
        strtod(s.c_str(), &e);
        return e && *e == '\0';
    }

    static vector<string> tok(const string& s) {
        vector<string> t;
        string c;
        auto flush = [&] { if (!c.empty()) t.push_back(c), c.clear(); };
        for (char x : s) {
            if (isspace((unsigned char)x)) flush();
            else if (x == ':' || x == ';' || x == '{' || x == '}') {
                flush();
                t.push_back(string(1, x));
            } else c += x;
        }
        flush();
        return t;
    }

    Val pop() {
        if (st.empty()) throw runtime_error("stack underflow");
        Val v = st.back();
        st.pop_back();
        return v;
    }

    double pop_num() {
        Val v = pop();
        if (holds_alternative<double>(v)) return get<double>(v);
        Vector s = get<Vector>(v);
        if (s.size() != 1) throw runtime_error("expected scalar");
        return s[0];
    }

    Vector pop_sig() {
        Val v = pop();
        return holds_alternative<double>(v) ? Sig(get<double>(v), 1) : get<Vector>(v);
    }

    void push(double x) { st.push_back(x); }
    void push(const Vector& x) { st.push_back(x); }

    double sr() {
        auto it = dict.find("sr");
        if (it == dict.end()) return 44100.0;
        exec(it->second.body);
        return pop_num();
    }

    static Vector bc(const Vector& a, size_t n) {
        if (a.size() == n) return a;
        if (a.size() == 1) return Sig(a[0], n);

        Vector r(n);
        size_t m = min(a.size(), n);
        for (size_t i = 0; i < m; ++i) r[i] = a[i];
        for (size_t i = m; i < n; ++i) r[i] = 0.0;
        return r;
    }

    static Vector bin(const Vector& a, const Vector& b, function<double(double,double)> f) {
        size_t n = max(a.size(), b.size());
        Vector x = bc(a, n), y = bc(b, n), r(n);
        for (size_t i = 0; i < n; ++i) r[i] = f(x[i], y[i]);
        return r;
    }

    static Vector un(const Vector& a, function<double(double)> f) {
        Vector r(a.size());
        for (size_t i = 0; i < a.size(); ++i) r[i] = f(a[i]);
        return r;
    }

    Word B(function<void(VM&)> fn) {
        return Word{true, fn, {}};
    }

    void add_stack() {
        dict["dup"] = B([](VM& vm) {
            Val v = vm.pop();
            vm.st.push_back(v);
            vm.st.push_back(v);
        });
        dict["swap"] = B([](VM& vm) {
            Val b = vm.pop(), a = vm.pop();
            vm.st.push_back(b);
            vm.st.push_back(a);
        });
        dict["drop"] = B([](VM& vm) {
            (void)vm.pop();
        });
        dict["top"] = B([](VM& vm) {
            if (vm.st.empty()) throw runtime_error("empty stack");
            vm.st.push_back(vm.st.back());
        });
        dict[".s"] = B([](VM& vm) {
            cout << "stack(" << vm.st.size() << ")\n";
            for (size_t i = 0; i < vm.st.size(); ++i) {
                cout << "  [" << i << "] ";
                if (holds_alternative<double>(vm.st[i])) cout << get<double>(vm.st[i]);
                else {
                    auto s = get<Vector>(vm.st[i]);
                    cout << "[";
                    for (size_t k = 0; k < min<size_t>(8, s.size()); ++k) {
                        if (k) cout << ' ';
                        cout << s[k];
                    }
                    if (s.size() > 8) cout << " ...";
                    cout << "]";
                }
                cout << "\n";
            }
        });
        dict[".v"] = B([](VM& vm) {
            cout << "words:\n";
            for (auto& kv : vm.dict) cout << "  " << kv.first << "\n";
        });
    }

    void add_math() {
        auto bop = [&](const string& n, auto f) {
            dict[n] = B([f](VM& vm) {
                Vector b = vm.pop_sig(), a = vm.pop_sig();
                vm.push(bin(a, b, f));
            });
        };

        bop("+", [](double a, double b) { return a + b; });
        bop("-", [](double a, double b) { return a - b; });
        bop("*", [](double a, double b) { return a * b; });
        bop("/", [](double a, double b) { return a / b; });

        dict["sin"] = B([](VM& vm) { vm.push(un(vm.pop_sig(), [](double x) { return std::sin(x); })); });
        dict["cos"] = B([](VM& vm) { vm.push(un(vm.pop_sig(), [](double x) { return std::cos(x); })); });
        dict["exp"] = B([](VM& vm) { vm.push(un(vm.pop_sig(), [](double x) { return std::exp(x); })); });
        dict["sqrt"] = B([](VM& vm) { vm.push(un(vm.pop_sig(), [](double x) { return std::sqrt(x); })); });
        dict["abs"] = B([](VM& vm) { vm.push(un(vm.pop_sig(), [](double x) { return std::fabs(x); })); });
        dict["pow"] = B([](VM& vm) {
            Vector b = vm.pop_sig(), a = vm.pop_sig();
            vm.push(bin(a, b, [](double x, double y) { return std::pow(x, y); }));
        });
        dict["pi"] = B([](VM& vm) { vm.push(3.14159265358979323846); });
    }

    void add_core() {
        dict["concat"] = B([](VM& vm) {
            Vector b = vm.pop_sig(), a = vm.pop_sig();
            Vector r(a.size() + b.size());
            for (size_t i = 0; i < a.size(); ++i) r[i] = a[i];
            for (size_t i = 0; i < b.size(); ++i) r[i + a.size()] = b[i];
            vm.push(r);
        });

        dict["delay"] = B([](VM& vm) {
            double sec = vm.pop_num();
            Vector x = vm.pop_sig();
            size_t n0 = max<size_t>(0, sec * vm.sr());
            Vector r(x.size() + n0);
            for (size_t i = 0; i < n0; ++i) r[i] = 0.0;
            for (size_t i = 0; i < x.size(); ++i) r[i + n0] = x[i];
            vm.push(r);
        });

        dict["render"] = B([](VM& vm) {
            Vector x = vm.pop_sig();
            string name = "out.wav";
            if (!vm.st.empty() && holds_alternative<Vector>(vm.st.back())) {
                Vector l = x, r = vm.pop_sig();
                if (l.size() != r.size()) throw runtime_error("stereo size mismatch");
                wav2(name, l, r, (int)vm.sr());
            } else {
                wav1(name, x, (int)vm.sr());
            }
            cout << "wrote " << name << "\n";
        });
    }

    void exec(const vector<string>& t) {
        for (size_t i = 0; i < t.size();) {
            string s = t[i++];

            if (s == ":") {
                if (i >= t.size()) throw runtime_error("missing word name");
                string name = t[i++];
                vector<string> body;
                while (i < t.size() && t[i] != ";") body.push_back(t[i++]);
                if (i == t.size()) throw runtime_error("missing ;");
                ++i;
                dict[name] = Word{false, {}, body};
                continue;
            }

            if (s == "{") {
                vector<string> body;
                int d = 1;
                while (i < t.size() && d > 0) {
                    if (t[i] == "{") d++;
                    else if (t[i] == "}") d--;
                    if (d > 0) body.push_back(t[i]);
                    ++i;
                }
                if (d) throw runtime_error("missing }");

                double dur = pop_num(), amp = pop_num(), SR = sr();
                size_t n = max<size_t>(1, dur * SR);

                VM in = *this;
                in.st.clear();

                Vector tvec(n), phvec(n);
                for (size_t k = 0; k < n; ++k) {
                    tvec[k] = (double)k / SR;
                    phvec[k] = (n == 1) ? 0.0 : (double)k / (n - 1);
                }

                in.dict["t"].builtin = true;
                in.dict["t"].fn = [tvec](VM& vm) { vm.push(tvec); };
                in.dict["t"].body.clear();

                in.dict["ph"].builtin = true;
                in.dict["ph"].fn = [phvec](VM& vm) { vm.push(phvec); };
                in.dict["ph"].body.clear();

                in.dict["amp"].builtin = true;
                in.dict["amp"].fn = [amp](VM& vm) { vm.push(amp); };
                in.dict["amp"].body.clear();

                in.exec(body);

                Val v = in.pop();
                if (holds_alternative<double>(v)) {
                    push(Sig(get<double>(v), n));
                } else {
                    Vector out = get<Vector>(v);
                    if (out.size() == 1) out = Sig(out[0], n);
                    else if (out.size() != n) throw runtime_error("block result has wrong size");
                    push(out);
                }
                continue;
            }

            if (numtok(s)) {
                push(strtod(s.c_str(), nullptr));
                continue;
            }

            auto it = dict.find(s);
            if (it == dict.end()) throw runtime_error("unknown word: " + s);
            if (it->second.builtin) it->second.fn(*this);
            else exec(it->second.body);
        }
    }

    static void wav1(const string& p, const Vector& x, int sr) {
        ofstream f(p, ios::binary);
        if (!f) throw runtime_error("cannot open wav");

        int32_t ds = (int32_t)x.size() * 2, rs = 36 + ds;
        int16_t fmt = 1, ch = 1, bps = 16, ba = 2;
        int32_t br = sr * ba, sub = 16;

        f.write("RIFF", 4);
        f.write((char*)&rs, 4);
        f.write("WAVE", 4);
        f.write("fmt ", 4);
        f.write((char*)&sub, 4);
        f.write((char*)&fmt, 2);
        f.write((char*)&ch, 2);
        f.write((char*)&sr, 4);
        f.write((char*)&br, 4);
        f.write((char*)&ba, 2);
        f.write((char*)&bps, 2);
        f.write("data", 4);
        f.write((char*)&ds, 4);

        for (double s : x) {
            if (s > 1) s = 1;
            if (s < -1) s = -1;
            int16_t q = (int16_t)lrint(s * 32767.0);
            f.write((char*)&q, 2);
        }
    }

    static void wav2(const string& p, const Vector& l, const Vector& r, int sr) {
        ofstream f(p, ios::binary);
        if (!f) throw runtime_error("cannot open wav");

        int32_t ds = (int32_t)l.size() * 4, rs = 36 + ds;
        int16_t fmt = 1, ch = 2, bps = 16, ba = 4;
        int32_t br = sr * ba, sub = 16;

        f.write("RIFF", 4);
        f.write((char*)&rs, 4);
        f.write("WAVE", 4);
        f.write("fmt ", 4);
        f.write((char*)&sub, 4);
        f.write((char*)&fmt, 2);
        f.write((char*)&ch, 2);
        f.write((char*)&sr, 4);
        f.write((char*)&br, 4);
        f.write((char*)&ba, 2);
        f.write((char*)&bps, 2);
        f.write("data", 4);
        f.write((char*)&ds, 4);

        for (size_t i = 0; i < l.size(); ++i) {
            double a = l[i], b = r[i];
            if (a > 1) a = 1;
            if (a < -1) a = -1;
            if (b > 1) b = 1;
            if (b < -1) b = -1;
            int16_t q1 = (int16_t)lrint(a * 32767.0), q2 = (int16_t)lrint(b * 32767.0);
            f.write((char*)&q1, 2);
            f.write((char*)&q2, 2);
        }
    }
};

int main(int argc, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    string src;

    VM vm;
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            ifstream in(argv[i]);
            if (!in) {
                cerr << "cannot open input " << argv[i] << "\n";
                continue;
            }
            src.assign((istreambuf_iterator<char>(in)), {});
            try {
                vm.exec(VM::tok(src));
            } catch (exception& e) {
                cerr << "error: " << e.what() << "\n";
                continue;
            }
        }
    } else {
        string line;
        cout << "[pure0, v0.2]\n";
        while (cout << ">> " && getline(cin, line)) {
            try {
                vm.exec(VM::tok(line));
            } catch (exception& e) {
                cerr << "error: " << e.what() << "\n";
            }
        }
    }

    return 0;
}
