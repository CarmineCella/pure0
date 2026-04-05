#include <cmath>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <valarray>
#include <variant>
#include <vector>

using namespace std;
using Vector = valarray<double>;

struct Expr {
    variant<double, string, vector<Expr>> v;
};

struct Env;
struct Lambda;

using Value = variant<double, Vector, shared_ptr<Lambda>>;
using Builtin = function<Value(const vector<Value>&)>;

struct Lambda {
    vector<string> params;
    Expr body;
    shared_ptr<Env> env;
};

struct Env {
    unordered_map<string, Value> m;
    unordered_map<string, Builtin> b;
    shared_ptr<Env> up;

    Value getv(const string& k) {
        auto it = m.find(k);
        if (it != m.end()) return it->second;
        if (up) return up->getv(k);
        throw runtime_error("unknown symbol: " + k);
    }

    Builtin getb(const string& k) {
        auto it = b.find(k);
        if (it != b.end()) return it->second;
        if (up) return up->getb(k);
        throw runtime_error("unknown function: " + k);
    }

    bool hasv(const string& k) {
        if (m.count(k)) return true;
        return up ? up->hasv(k) : false;
    }

    bool hasb(const string& k) {
        if (b.count(k)) return true;
        return up ? up->hasb(k) : false;
    }

    void setv(const string& k, const Value& v) { m[k] = v; }
    void setb(const string& k, const Builtin& f) { b[k] = f; }
};

static bool is_num(const string& s) {
    char* e = nullptr;
    strtod(s.c_str(), &e);
    return e && *e == '\0';
}

static vector<string> tokenize(const string& src) {
    vector<string> t;
    string cur;
    auto flush = [&] { if (!cur.empty()) t.push_back(cur), cur.clear(); };
    for (char c : src) {
        if (isspace((unsigned char)c)) flush();
        else if (c == '(' || c == ')') { flush(); t.push_back(string(1, c)); }
        else cur += c;
    }
    flush();
    return t;
}

static Expr parse_expr(const vector<string>& t, size_t& i) {
    if (i >= t.size()) throw runtime_error("unexpected eof");
    string s = t[i++];
    if (s == "(") {
        vector<Expr> xs;
        while (i < t.size() && t[i] != ")") xs.push_back(parse_expr(t, i));
        if (i >= t.size()) throw runtime_error("missing )");
        ++i;
        return {xs};
    }
    if (is_num(s)) return {strtod(s.c_str(), nullptr)};
    return {s};
}

static vector<Expr> parse_many(const vector<string>& t) {
    vector<Expr> xs;
    size_t i = 0;
    while (i < t.size()) xs.push_back(parse_expr(t, i));
    return xs;
}

static Vector to_sig(const Value& v) {
    if (holds_alternative<double>(v)) return Sig(get<double>(v), 1);
    if (holds_alternative<Vector>(v)) return get<Vector>(v);
    throw runtime_error("expected number or signal");
}

static double to_num(const Value& v) {
    if (holds_alternative<double>(v)) return get<double>(v);
    if (holds_alternative<Vector>(v)) {
        Vector s = get<Vector>(v);
        if (s.size() != 1) throw runtime_error("expected scalar");
        return s[0];
    }
    throw runtime_error("expected scalar");
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

static void wav1(const string& p, const Vector& x, int sr) {
    ofstream f(p, ios::binary);
    if (!f) throw runtime_error("cannot open wav");
    int32_t ds = (int32_t)x.size() * 2, rs = 36 + ds;
    int16_t fmt = 1, ch = 1, bps = 16, ba = 2;
    int32_t br = sr * ba, sub = 16;
    f.write("RIFF", 4); f.write((char*)&rs, 4); f.write("WAVE", 4); f.write("fmt ", 4);
    f.write((char*)&sub, 4); f.write((char*)&fmt, 2); f.write((char*)&ch, 2);
    f.write((char*)&sr, 4); f.write((char*)&br, 4); f.write((char*)&ba, 2); f.write((char*)&bps, 2);
    f.write("data", 4); f.write((char*)&ds, 4);
    for (double s : x) {
        if (s > 1) s = 1; if (s < -1) s = -1;
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
    f.write("RIFF", 4); f.write((char*)&rs, 4); f.write("WAVE", 4); f.write("fmt ", 4);
    f.write((char*)&sub, 4); f.write((char*)&fmt, 2); f.write((char*)&ch, 2);
    f.write((char*)&sr, 4); f.write((char*)&br, 4); f.write((char*)&ba, 2); f.write((char*)&bps, 2);
    f.write("data", 4); f.write((char*)&ds, 4);
    for (size_t i = 0; i < l.size(); ++i) {
        double a = l[i], b = r[i];
        if (a > 1) a = 1; if (a < -1) a = -1;
        if (b > 1) b = 1; if (b < -1) b = -1;
        int16_t q1 = (int16_t)lrint(a * 32767.0), q2 = (int16_t)lrint(b * 32767.0);
        f.write((char*)&q1, 2); f.write((char*)&q2, 2);
    }
}

static Value eval(const Expr& e, shared_ptr<Env> env);

static Value call_value(const Value& f, const vector<Value>& args) {
    if (!holds_alternative<shared_ptr<Lambda>>(f)) throw runtime_error("not callable");
    auto lam = get<shared_ptr<Lambda>>(f);
    if (args.size() != lam->params.size()) throw runtime_error("arity mismatch");
    auto local = make_shared<Env>();
    local->up = lam->env;
    for (size_t i = 0; i < args.size(); ++i) local->setv(lam->params[i], args[i]);
    return eval(lam->body, local);
}

static Value eval_list(const vector<Expr>& xs, shared_ptr<Env> env) {
    if (xs.empty()) throw runtime_error("empty list");
    if (holds_alternative<string>(xs[0].v)) {
        string head = get<string>(xs[0].v);

        if (head == "def") {
            if (xs.size() != 3 || !holds_alternative<string>(xs[1].v)) throw runtime_error("bad def");
            string name = get<string>(xs[1].v);
            Value v = eval(xs[2], env);
            env->setv(name, v);
            return v;
        }

        if (head == "lambda") {
            if (xs.size() != 3 || !holds_alternative<vector<Expr>>(xs[1].v)) throw runtime_error("bad lambda");
            vector<string> ps;
            for (auto& p : get<vector<Expr>>(xs[1].v)) {
                if (!holds_alternative<string>(p.v)) throw runtime_error("lambda params must be symbols");
                ps.push_back(get<string>(p.v));
            }
            return make_shared<Lambda>(Lambda{ps, xs[2], env});
        }

        if (head == "sig") {
            if (xs.size() != 4) throw runtime_error("sig expects (sig amp dur expr)");
            double amp = to_num(eval(xs[1], env));
            double dur = to_num(eval(xs[2], env));
            double SR = to_num(env->getv("sr"));
            size_t n = max<size_t>(1, dur * SR);

            Vector t(n), ph(n);
            for (size_t i = 0; i < n; ++i) {
                t[i] = (double)i / SR;
                ph[i] = (n == 1) ? 0.0 : (double)i / (n - 1);
            }

            auto local = make_shared<Env>();
            local->up = env;
            local->setv("t", t);
            local->setv("ph", ph);
            local->setv("amp", amp);

            Value v = eval(xs[3], local);
            if (holds_alternative<double>(v)) return Sig(get<double>(v), n);
            Vector out = to_sig(v);
            if (out.size() == 1) return Sig(out[0], n);
            if (out.size() != n) throw runtime_error("sig result has wrong size");
            return out;
        }

        if (env->hasb(head)) {
            vector<Value> args;
            for (size_t i = 1; i < xs.size(); ++i) args.push_back(eval(xs[i], env));
            return env->getb(head)(args);
        }
    }

    Value f = eval(xs[0], env);
    vector<Value> args;
    for (size_t i = 1; i < xs.size(); ++i) args.push_back(eval(xs[i], env));
    return call_value(f, args);
}

static Value eval(const Expr& e, shared_ptr<Env> env) {
    if (holds_alternative<double>(e.v)) return get<double>(e.v);
    if (holds_alternative<string>(e.v)) return env->getv(get<string>(e.v));
    return eval_list(get<vector<Expr>>(e.v), env);
}

static Builtin mk_bin(function<double(double,double)> f) {
    return [f](const vector<Value>& args) -> Value {
        if (args.empty()) return 0.0;
        Vector r = to_sig(args[0]);
        for (size_t i = 1; i < args.size(); ++i) r = bin(r, to_sig(args[i]), f);
        return r.size() == 1 ? Value(r[0]) : Value(r);
    };
}

static Builtin mk_un(function<double(double)> f) {
    return [f](const vector<Value>& args) -> Value {
        if (args.size() != 1) throw runtime_error("arity 1 expected");
        Vector r = un(to_sig(args[0]), f);
        return r.size() == 1 ? Value(r[0]) : Value(r);
    };
}

auto make_environment() {
    auto g = make_shared<Env>();
    g->setv("sr", 44100.0);
    g->setv("pi", 3.14159265358979323846);

    g->setb("+", mk_bin([](double a, double b){ return a + b; }));
    g->setb("-", mk_bin([](double a, double b){ return a - b; }));
    g->setb("*", mk_bin([](double a, double b){ return a * b; }));
    g->setb("/", mk_bin([](double a, double b){ return a / b; }));
    g->setb("pow", mk_bin([](double a, double b){ return std::pow(a, b); }));

    g->setb("sin", mk_un([](double x){ return std::sin(x); }));
    g->setb("cos", mk_un([](double x){ return std::cos(x); }));
    g->setb("exp", mk_un([](double x){ return std::exp(x); }));
    g->setb("sqrt", mk_un([](double x){ return std::sqrt(x); }));
    g->setb("abs", mk_un([](double x){ return std::fabs(x); }));

    g->setb("concat", [](const vector<Value>& args) -> Value {
        if (args.size() != 2) throw runtime_error("concat expects 2 args");
        Vector a = to_sig(args[0]), b = to_sig(args[1]), r(a.size() + b.size());
        for (size_t i = 0; i < a.size(); ++i) r[i] = a[i];
        for (size_t i = 0; i < b.size(); ++i) r[i + a.size()] = b[i];
        return r;
    });

    g->setb("delay", [g](const vector<Value>& args) -> Value {
        if (args.size() != 2) throw runtime_error("delay expects 2 args");
        Vector x = to_sig(args[0]);
        double sec = to_num(args[1]);
        size_t n0 = max<size_t>(0, sec * to_num(g->getv("sr")));
        Vector r(x.size() + n0);
        for (size_t i = 0; i < n0; ++i) r[i] = 0.0;
        for (size_t i = 0; i < x.size(); ++i) r[i + n0] = x[i];
        return r;
    });

    g->setb("render", [g](const vector<Value>& args) -> Value {
        if (args.size() == 1) {
            Vector x = to_sig(args[0]);
            wav1("out.wav", x, (int)to_num(g->getv("sr")));
            cout << "wrote out.wav\n";
            return x;
        }
        if (args.size() == 2) {
            Vector l = to_sig(args[0]), r = to_sig(args[1]);
            if (l.size() != r.size()) throw runtime_error("stereo size mismatch");
            wav2("out.wav", l, r, (int)to_num(g->getv("sr")));
            cout << "wrote out.wav\n";
            return l;
        }
        throw runtime_error("render expects 1 or 2 args");
    });
   return g;
}
int main(int argc, char** argv) {
    string src;
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            ifstream in(argv[i]);
            if (!in) {
                cerr << "cannot open input " << argv[i] << "\n";
                continue;
            }
            src.append((istreambuf_iterator<char>(in)), {});
            src.push_back('\n');
        }
    } else {
        src.assign((istreambuf_iterator<char>(cin)), {});
    }

    auto g = make_environment();
    try {
        auto exprs = parse_many(tokenize(src));
        for (auto& e : exprs) {
            Value v = eval(e, g);
            if (argc == 1) {
                if (holds_alternative<double>(v)) cout << get<double>(v) << "\n";
                else if (holds_alternative<Vector>(v)) {
                    auto s = get<Vector>(v);
                    cout << "[";
                    for (size_t i = 0; i < min<size_t>(8, s.size()); ++i) {
                        if (i) cout << ' ';
                        cout << s[i];
                    }
                    if (s.size() > 8) cout << " ...";
                    cout << "]\n";
                }
            }
        }
    } catch (exception& e) {
        cerr << "error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
