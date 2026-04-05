#include <cmath>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <valarray>
#include <variant>
#include <vector>

using Vector = std::valarray<double>;
struct Symbol {
    std::string s;
};
struct String {
    std::string s;
};
struct Env;
struct Expr;
using ExprPtr = std::shared_ptr<Expr>;
struct Lambda;
using LambdaPtr = std::shared_ptr<Lambda>;
using Proc = std::function<ExprPtr(const std::vector<ExprPtr>&, std::shared_ptr<Env>)>;
struct Expr {
    using List = std::vector<ExprPtr>;
    std::variant<Symbol, String, Vector, LambdaPtr, Proc, List> v;
    Expr() = default;
    explicit Expr(const Symbol& x) : v(x) {}
    explicit Expr(const String& x) : v(x) {}
    explicit Expr(const Vector& x) : v(x) {}
    explicit Expr(const LambdaPtr& x) : v(x) {}
    explicit Expr(const Proc& x) : v(x) {}
    explicit Expr(const List& x) : v(x) {}
};
struct Lambda {
    std::vector<std::string> params;
    ExprPtr body;
    std::shared_ptr<Env> env;
};

struct Env {
    std::unordered_map<std::string, ExprPtr> table;
    std::shared_ptr<Env> parent;
    ExprPtr get(const std::string& key) const {
        auto it = table.find(key);
        if (it != table.end()) return it->second;
        if (parent) return parent->get(key);
        throw std::runtime_error("unknown symbol: " + key);
    }
    void set(const std::string& key, ExprPtr value) {
        table[key] = std::move(value);
    }
};

static ExprPtr make_symbol(const std::string& s) { return std::make_shared<Expr>(Symbol{s}); }
static ExprPtr make_string(const std::string& s) { return std::make_shared<Expr>(String{s}); }
static ExprPtr make_vec(const Vector& x) { return std::make_shared<Expr>(x);}
static ExprPtr make_scalar(double x) { return make_vec(Sig(x, 1)); }
static ExprPtr make_lambda(const LambdaPtr& x) { return std::make_shared<Expr>(x); }
static ExprPtr make_proc(const Proc& x) {  return std::make_shared<Expr>(x); }
static ExprPtr make_list(const Expr::List& x) { return std::make_shared<Expr>(x); }
static bool is_symbol(const ExprPtr& x) { return std::holds_alternative<Symbol>(x->v); }
static bool is_string(const ExprPtr& x) { return std::holds_alternative<String>(x->v); }
static bool is_sig(const ExprPtr& x) { return std::holds_alternative<Vector>(x->v); }
static bool is_lambda(const ExprPtr& x) { return std::holds_alternative<LambdaPtr>(x->v); }
static bool is_proc(const ExprPtr& x) { return std::holds_alternative<Proc>(x->v); }
static bool is_list(const ExprPtr& x) { return std::holds_alternative<Expr::List>(x->v); }
static const std::string& as_symbol(const ExprPtr& x) { return std::get<Symbol>(x->v).s; }
static const std::string& as_string(const ExprPtr& x) { return std::get<String>(x->v).s; }
static Vector as_sig(const ExprPtr& x) {
    if (!is_sig(x)) throw std::runtime_error("expected numeric signal");
    return std::get<Vector>(x->v);
}
static double as_scalar(const ExprPtr& x) {
    Vector s = as_sig(x);
    if (s.size() != 1) throw std::runtime_error("expected scalar");
    return s[0];
}
static bool truthy(const ExprPtr& x) { return as_scalar(x) != 0.0;}

static std::string to_string_value(const ExprPtr& x) {
    if (is_symbol(x)) return as_symbol(x);
    if (is_string(x)) return as_string(x);
    if (is_lambda(x)) return "<lambda>";
    if (is_proc(x)) return "<proc>";
    if (is_list(x)) {
        const auto& xs = std::get<Expr::List>(x->v);
        std::ostringstream oss;
        oss << "(";
        for (std::size_t i = 0; i < xs.size(); ++i) {
            if (i) oss << " ";
            oss << to_string_value(xs[i]);
        }
        oss << ")";
        return oss.str();
    }
    Vector s = as_sig(x);
    std::ostringstream oss;
    if (s.size() == 1) oss << s[0];
    else {
        oss << "[";
        for (std::size_t i = 0; i < s.size(); ++i) {
            if (i) oss << " ";
            oss << s[i];
        }
        oss << "]";
    }
    return oss.str();
}
static bool numtok(const std::string& s) {
    char* e = nullptr;
    std::strtod(s.c_str(), &e);
    return e && *e == '\0';
}

static std::vector<std::string> tokenize(const std::string& src) {
    std::vector<std::string> t;
    std::string cur;
    auto flush = [&] { if (!cur.empty()) t.push_back(cur), cur.clear(); };
    for (std::size_t i = 0; i < src.size(); ++i) {
        char c = src[i];
        if (std::isspace((unsigned char)c)) flush();
        else if (c == ';') {
            flush();
            while (i < src.size() && src[i] != '\n') ++i;
        } else if (c == '(' || c == ')' || c == '\'') {
            flush();
            t.push_back(std::string(1, c));
        } else if (c == '"') {
            flush();
            std::string s;
            ++i;
            while (i < src.size() && src[i] != '"') {
                if (src[i] == '\\' && i + 1 < src.size()) {
                    ++i;
                    char e = src[i];
                    if (e == 'n') s.push_back('\n');
                    else if (e == 't') s.push_back('\t');
                    else s.push_back(e);
                } else s.push_back(src[i]);
                ++i;
            }
            t.push_back("\"" + s + "\"");
        } else cur.push_back(c);
    }
    flush();
    return t;
}

static ExprPtr parse_expr(const std::vector<std::string>& t, std::size_t& i) {
    if (i >= t.size()) throw std::runtime_error("unexpected eof");
    std::string s = t[i++];
    if (s == "'") {
        Expr::List q;
        q.push_back(make_symbol("quote"));
        q.push_back(parse_expr(t, i));
        return make_list(q);
    }
    if (s == "(") {
        Expr::List xs;
        while (i < t.size() && t[i] != ")") xs.push_back(parse_expr(t, i));
        if (i >= t.size()) throw std::runtime_error("missing )");
        ++i;
        return make_list(xs);
    }
    if (s.size() >= 2 && s.front() == '"'
            && s.back() == '"') return make_string(s.substr(1, s.size() - 2));
    if (numtok(s)) return make_scalar(std::strtod(s.c_str(), nullptr));
    return make_symbol(s);
}
static std::vector<ExprPtr> parse_many(const std::vector<std::string>& t) {
    std::vector<ExprPtr> xs;
    std::size_t i = 0;
    while (i < t.size()) xs.push_back(parse_expr(t, i));
    return xs;
}

static Vector broadcast(const Vector& a, std::size_t n) {
    if (a.size() == n) return a;
    if (a.size() == 1) return Sig(a[0], n);
    Vector r(n);
    std::size_t m = std::min<std::size_t>(a.size(), n);
    for (std::size_t i = 0; i < m; ++i) r[i] = a[i];
    for (std::size_t i = m; i < n; ++i) r[i] = 0.0;
    return r;
}
static Vector binop(const Vector& a, const Vector& b,
                 const std::function<double(double, double)>& f) {
    std::size_t n = std::max<std::size_t>(a.size(), b.size());
    Vector x = broadcast(a, n), y = broadcast(b, n), r(n);
    for (std::size_t i = 0; i < n; ++i) r[i] = f(x[i], y[i]);
    return r;
}
static Vector unop(const Vector& a, const std::function<double(double)>& f) {
    Vector r(a.size());
    for (std::size_t i = 0; i < a.size(); ++i) r[i] = f(a[i]);
    return r;
}

static void write_wav_mono(const std::string& path, const Vector& x, int sr) {
    std::ofstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("cannot open wav for writing: " + path);
    int32_t ds = static_cast<int32_t>(x.size()) * 2, rs = 36 + ds;
    int16_t fmt = 1, ch = 1, bps = 16, ba = 2;
    int32_t br = sr * ba, sub = 16;
    f.write("RIFF", 4);
    f.write(reinterpret_cast<char*>(&rs), 4);
    f.write("WAVE", 4);
    f.write("fmt ", 4);
    f.write(reinterpret_cast<char*>(&sub), 4);
    f.write(reinterpret_cast<char*>(&fmt), 2);
    f.write(reinterpret_cast<char*>(&ch), 2);
    f.write(reinterpret_cast<char*>(&sr), 4);
    f.write(reinterpret_cast<char*>(&br), 4);
    f.write(reinterpret_cast<char*>(&ba), 2);
    f.write(reinterpret_cast<char*>(&bps), 2);
    f.write("data", 4);
    f.write(reinterpret_cast<char*>(&ds), 4);
    for (double s : x) {
        if (s > 1.0) s = 1.0;
        if (s < -1.0) s = -1.0;
        int16_t q = static_cast<int16_t>(std::lrint(s * 32767.0));
        f.write(reinterpret_cast<char*>(&q), 2);
    }
}
static void write_wav_stereo(const std::string& path, const Vector& l, const Vector& r,
                             int sr) {
    if (l.size() != r.size()) throw std::runtime_error("stereo render size mismatch");
    std::ofstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("cannot open wav for writing: " + path);
    int32_t ds = static_cast<int32_t>(l.size()) * 4, rs = 36 + ds;
    int16_t fmt = 1, ch = 2, bps = 16, ba = 4;
    int32_t br = sr * ba, sub = 16;
    f.write("RIFF", 4);
    f.write(reinterpret_cast<char*>(&rs), 4);
    f.write("WAVE", 4);
    f.write("fmt ", 4);
    f.write(reinterpret_cast<char*>(&sub), 4);
    f.write(reinterpret_cast<char*>(&fmt), 2);
    f.write(reinterpret_cast<char*>(&ch), 2);
    f.write(reinterpret_cast<char*>(&sr), 4);
    f.write(reinterpret_cast<char*>(&br), 4);
    f.write(reinterpret_cast<char*>(&ba), 2);
    f.write(reinterpret_cast<char*>(&bps), 2);
    f.write("data", 4);
    f.write(reinterpret_cast<char*>(&ds), 4);
    for (std::size_t i = 0; i < l.size(); ++i) {
        double a = l[i], b = r[i];
        if (a > 1.0) a = 1.0;
        if (a < -1.0) a = -1.0;
        if (b > 1.0) b = 1.0;
        if (b < -1.0) b = -1.0;
        int16_t q1 = static_cast<int16_t>(std::lrint(a * 32767.0));
        int16_t q2 = static_cast<int16_t>(std::lrint(b * 32767.0));
        f.write(reinterpret_cast<char*>(&q1), 2);
        f.write(reinterpret_cast<char*>(&q2), 2);
    }
}

static ExprPtr eval(ExprPtr expr, std::shared_ptr<Env> env);
static ExprPtr eval_sequence(const std::vector<ExprPtr>& xs, std::shared_ptr<Env> env) {
    ExprPtr result = make_scalar(0.0);
    for (const auto& x : xs) result = eval(x, env);
    return result;
}
static ExprPtr make_partial_lambda(const LambdaPtr& lam,
                                   const std::vector<ExprPtr>& bound_args) {
    auto local = std::make_shared<Env>();
    local->parent = lam->env;
    for (std::size_t i = 0; i < bound_args.size();
            ++i) local->set(lam->params[i], bound_args[i]);
    std::vector<std::string> rest(lam->params.begin() + bound_args.size(), lam->params.end());
    return make_lambda(std::make_shared<Lambda>(Lambda{rest, lam->body, local}));
}
static ExprPtr apply_callable(const ExprPtr& fn, const std::vector<ExprPtr>& args,
                              std::shared_ptr<Env> env) {
    if (is_proc(fn)) return std::get<Proc>(fn->v)(args, env);
    if (!is_lambda(fn)) throw std::runtime_error("attempt to call non-function");
    auto lam = std::get<LambdaPtr>(fn->v);
    if (args.size() < lam->params.size()) return make_partial_lambda(lam, args);
    if (args.size() == lam->params.size()) {
        auto local = std::make_shared<Env>();
        local->parent = lam->env;
        for (std::size_t i = 0; i < args.size(); ++i) local->set(lam->params[i], args[i]);
        return eval(lam->body, local);
    }
    std::vector<ExprPtr> first(args.begin(), args.begin() + lam->params.size());
    std::vector<ExprPtr> rest(args.begin() + lam->params.size(), args.end());
    ExprPtr result = apply_callable(fn, first, env);
    return apply_callable(result, rest, env);
}
static ExprPtr eval(ExprPtr expr, std::shared_ptr<Env> env) {
    while (true) {
        if (is_sig(expr) || is_string(expr) || is_lambda(expr) || is_proc(expr)) return expr;
        if (is_symbol(expr)) return env->get(as_symbol(expr));
        auto xs = std::get<Expr::List>(expr->v);
        if (xs.empty()) throw std::runtime_error("empty application");
        if (is_symbol(xs[0])) {
            const std::string& head = as_symbol(xs[0]);
            if (head == "quote") {
                if (xs.size() != 2) throw std::runtime_error("quote expects 1 argument");
                return xs[1];
            }
            if (head == "def") {
                if (xs.size() != 3 || !is_symbol(xs[1])) throw std::runtime_error("bad def");
                auto value = eval(xs[2], env);
                env->set(as_symbol(xs[1]), value);
                return value;
            }
            if (head == "lambda") {
                if (xs.size() < 3 || !is_list(xs[1])) throw std::runtime_error("bad lambda");
                std::vector<std::string> ps;
                for (const auto& p : std::get<Expr::List>(xs[1]->v)) {
                    if (!is_symbol(p)) throw std::runtime_error("lambda params must be symbols");
                    ps.push_back(as_symbol(p));
                }
                ExprPtr body;
                if (xs.size() == 3) body = xs[2];
                else {
                    Expr::List begin_list;
                    begin_list.push_back(make_symbol("begin"));
                    for (std::size_t i = 2; i < xs.size(); ++i) begin_list.push_back(xs[i]);
                    body = make_list(begin_list);
                }
                return make_lambda(std::make_shared<Lambda>(Lambda{ps, body, env}));
            }
            if (head == "if") {
                if (xs.size() != 4) throw std::runtime_error("if expects 3 arguments");
                expr = truthy(eval(xs[1], env)) ? xs[2] : xs[3];
                continue;
            }
            if (head == "begin") {
                if (xs.size() == 1) return make_scalar(0.0);
                for (std::size_t i = 1; i + 1 < xs.size(); ++i) (void)eval(xs[i], env);
                expr = xs.back();
                continue;
            }
            if (head == "sig") {
                if (xs.size() != 4) throw std::runtime_error("sig expects (sig amp dur expr)");
                double amp = as_scalar(eval(xs[1], env));
                double dur = as_scalar(eval(xs[2], env));
                double sr = as_scalar(env->get("sr"));
                std::size_t n = std::max<std::size_t>(1, static_cast<std::size_t>(dur * sr));
                Vector t(n), ph(n);
                for (std::size_t i = 0; i < n; ++i) {
                    t[i] = (double)i / sr;
                    ph[i] = (n == 1) ? 0.0 : (double)i / (n - 1);
                }
                auto local = std::make_shared<Env>();
                local->parent = env;
                local->set("t", make_vec(t));
                local->set("ph", make_vec(ph));
                local->set("amp", make_scalar(amp));
                ExprPtr v = eval(xs[3], local);
                Vector out = as_sig(v);
                if (out.size() == 1) out = Sig(out[0], n);
                else if (out.size() != n) throw std::runtime_error("sig result has wrong size");
                return make_vec(out);
            }
            if (head == "eval") {
                if (xs.size() != 2) throw std::runtime_error("eval expects 1 argument");
                expr = eval(xs[1], env);
                continue;
            }
            if (head == "apply") {
                if (xs.size() != 3) throw std::runtime_error("apply expects 2 arguments");
                ExprPtr f = eval(xs[1], env);
                ExprPtr lst = eval(xs[2], env);
                if (!is_list(lst)) throw std::runtime_error("apply expects a list as second argument");
                return apply_callable(f, std::get<Expr::List>(lst->v), env);
            }
        }
        ExprPtr f = eval(xs[0], env);
        std::vector<ExprPtr> args;
        args.reserve(xs.size() - 1);
        for (std::size_t i = 1; i < xs.size(); ++i) args.push_back(eval(xs[i], env));
        if (is_lambda(f)) {
            auto lam = std::get<LambdaPtr>(f->v);
            if (args.size() < lam->params.size()) return make_partial_lambda(lam, args);
            if (args.size() == lam->params.size()) {
                auto local = std::make_shared<Env>();
                local->parent = lam->env;
                for (std::size_t i = 0; i < args.size(); ++i) local->set(lam->params[i], args[i]);
                env = local;
                expr = lam->body;
                continue;
            }
            std::vector<ExprPtr> first(args.begin(), args.begin() + lam->params.size());
            std::vector<ExprPtr> rest(args.begin() + lam->params.size(), args.end());
            auto local = std::make_shared<Env>();
            local->parent = lam->env;
            for (std::size_t i = 0; i < first.size(); ++i) local->set(lam->params[i], first[i]);
            ExprPtr result = eval(lam->body, local);
            return apply_callable(result, rest, env);
        }
        return apply_callable(f, args, env);
    }
}

static Proc make_numeric_reducer(const std::function<double(double, double)>& f,
                                 double init, bool unary_from_init = false) {
    return [f, init, unary_from_init](const std::vector<ExprPtr>& args,
    std::shared_ptr<Env>) -> ExprPtr {
        if (args.empty()) return make_scalar(init);
        Vector r = as_sig(args[0]);
        if (args.size() == 1 && unary_from_init) r = binop(Sig(init, 1), r, f);
        else for (std::size_t i = 1; i < args.size(); ++i) r = binop(r, as_sig(args[i]), f);
        return make_vec(r);
    };
}
static Proc make_numeric_unary(const std::function<double(double)>& f) {
    return [f](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("unary primitive expects 1 argument");
        return make_vec(unop(as_sig(args[0]), f));
    };
}
static Proc fn_print() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr { for (std::size_t i = 0; i < args.size(); ++i) {
    if (i) std::cout << " ";
        std::cout << to_string_value(args[i]);
    }
    std::cout << "\n";
              return args.empty() ? make_scalar(0.0) : args.back();
                                                                                 };
}
static Proc fn_read() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr { 
        if (args.size() != 1 || !is_string(args[0])) throw std::runtime_error("read expects one string path"); 
        std::ifstream in(as_string(args[0])); 
        if (!in) throw std::runtime_error("cannot open file for reading: " + as_string(args[0])); 
        std::ostringstream ss; ss << in.rdbuf(); return make_string(ss.str()); 
    };
}
static Proc fn_write() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr { 
        if (args.size() != 2 || !is_string(args[0])) throw std::runtime_error("write expects path and content"); 
        std::ofstream out(as_string(args[0])); 
        if (!out) throw std::runtime_error("cannot open file for writing: " + as_string(args[0])); 
        out << to_string_value(args[1]); return args[1]; 
    };
}
static Proc fn_load() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env> env) -> ExprPtr { 
        if (args.size() != 1 || !is_string(args[0])) throw std::runtime_error("load expects one string path"); 
        std::ifstream in(as_string(args[0])); 
        if (!in) throw std::runtime_error("cannot open file for loading: " + as_string(args[0])); 
        std::ostringstream ss; ss << in.rdbuf(); 
        auto exprs = parse_many(tokenize(ss.str())); 
        return eval_sequence(exprs, env); };
}
static Proc fn_list() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr { return make_list(args); };
}
static Proc fn_nth() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr { 
        if (args.size() != 2) throw std::runtime_error("nth expects 2 args");
        std::size_t n = static_cast<std::size_t>(as_scalar(args[1]));
        if (is_list(args[0])) {
            const auto& xs = std::get<Expr::List>(args[0]->v);
                if (n >= xs.size()) throw std::runtime_error("nth out of range");
                return xs[n];
            }
        if (is_string(args[0])) {
        const auto& s = as_string(args[0]);
            if (n >= s.size()) throw std::runtime_error("nth out of range");
            return make_string(std::string(1, s[n]));
        }
        Vector x = as_sig(args[0]);
                if (n >= x.size()) throw std::runtime_error("nth out of range");
                return make_scalar(x[n]);
    };
}
static Proc fn_len() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr { 
        if (args.size() != 1) throw std::runtime_error("len expects 1 arg"); 
        if (is_list(args[0])) return make_scalar((double)std::get<Expr::List>(args[0]->v).size()); 
        if (is_string(args[0])) return make_scalar((double)as_string(args[0]).size()); 
        return make_scalar((double)as_sig(args[0]).size()); 
    };
}
static Proc fn_osc() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr { 
        if (args.size() != 1) throw std::runtime_error("osc expects one argument"); 
        Vector phase = as_sig(args[0]); Vector r(phase.size()); 
        for (std::size_t i = 0; i < phase.size(); ++i) r[i] = std::sin(2.0 * 3.14159265358979323846 * phase[i]); 
        return make_vec(r); 
    };
}
static Proc fn_shift() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env> env) -> ExprPtr { 
        if (args.size() != 2) throw std::runtime_error("delay expects 2 args"); 
        Vector x = as_sig(args[0]); double sec = as_scalar(args[1]); 
        double sr = as_scalar(env->get("sr")); 
        std::size_t n0 = std::max<std::size_t>(0, static_cast<std::size_t>(sec * sr)); 
        Vector r(x.size() + n0); 
        for (std::size_t i = 0; i < n0; ++i) r[i] = 0.0; 
        for (std::size_t i = 0; i < x.size(); ++i) r[i + n0] = x[i]; 
        return make_vec(r);
    };
}
static Proc fn_cat() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr { 
        if (args.size() != 2) throw std::runtime_error("cat expects 2 args"); 
        Vector a = as_sig(args[0]), b = as_sig(args[1]); 
        Vector r(a.size() + b.size()); 
        for (std::size_t i = 0; i < a.size(); ++i) r[i] = a[i]; 
        for (std::size_t i = 0; i < b.size(); ++i) r[i + a.size()] = b[i]; 
        return make_vec(r);
    };
}
static Proc fn_bpf() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr { 
        if (args.size() < 3 || (args.size() % 2) == 0) throw std::runtime_error("bpf expects x y pairs and phase signal as last arg");
        Vector phase = as_sig(args.back());
        std::vector<double> pts;
        pts.reserve(args.size() - 1);
        for (std::size_t i = 0; i + 1 < args.size(); ++i) pts.push_back(as_scalar(args[i]));
        if (pts.size() < 4) throw std::runtime_error("bpf needs at least 2 points");
        Vector out(phase.size());
        for (std::size_t k = 0; k < phase.size(); ++k) {
            double ph = phase[k], y = pts[1];
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
            out[k] = y;
        }
       return make_vec(out);
    };
}
static Proc fn_render() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env> env) -> ExprPtr { 
        if (args.empty() || args.size() > 3) throw std::runtime_error("render expects 1, 2, or 3 args");
        std::string path = "out.wav";
        std::size_t start = 0;
        if (is_string(args[0])) {
            path = as_string(args[0]);
            start = 1;
        }
        int sr = (int)as_scalar(env->get("sr"));
        if (args.size() - start == 1) {
            Vector x = as_sig(args[start]);
            write_wav_mono(path, x, sr);
            return args[start];
        }
        if (args.size() - start == 2) {
            Vector l = as_sig(args[start]), r = as_sig(args[start + 1]);
            write_wav_stereo(path, l, r, sr);
            return args[start];
        }
        throw std::runtime_error("render expects mono or stereo signal(s)");
    };
}

static std::shared_ptr<Env> make_environment() {
    auto env = std::make_shared<Env>();
    env->set("sr", make_scalar(44100.0));
    env->set("pi", make_scalar(3.14159265358979323846));
    env->set("+", make_proc(make_numeric_reducer([](double a, double b) {
        return a + b;
    }, 0.0)));
    env->set("-", make_proc(make_numeric_reducer([](double a, double b) {
        return a - b;
    }, 0.0, true)));
    env->set("*", make_proc(make_numeric_reducer([](double a, double b) {
        return a * b;
    }, 1.0)));
    env->set("/", make_proc(make_numeric_reducer([](double a, double b) {
        return a / b;
    }, 1.0, true)));
    env->set("pow", make_proc(make_numeric_reducer([](double a, double b) {
        return std::pow(a, b);
    }, 1.0)));
    env->set("sin", make_proc(make_numeric_unary([](double x) {
        return std::sin(x);
    })));
    env->set("cos", make_proc(make_numeric_unary([](double x) {
        return std::cos(x);
    })));
    env->set("exp", make_proc(make_numeric_unary([](double x) {
        return std::exp(x);
    })));
    env->set("abs", make_proc(make_numeric_unary([](double x) {
        return std::fabs(x);
    })));
    env->set("sqrt", make_proc(make_numeric_unary([](double x) {
        return std::sqrt(x);
    })));
    env->set("print", make_proc(fn_print()));
    env->set("read", make_proc(fn_read()));
    env->set("write", make_proc(fn_write()));
    env->set("load", make_proc(fn_load()));
    env->set("list", make_proc(fn_list()));
    env->set("nth", make_proc(fn_nth()));
    env->set("len", make_proc(fn_len()));
    env->set("osc", make_proc(fn_osc()));
    env->set("delay", make_proc(fn_shift()));
    env->set("cat", make_proc(fn_cat()));
    env->set("bpf", make_proc(fn_bpf()));
    env->set("render", make_proc(fn_render()));
    return env;
}

int main(int argc, char** argv) {
    auto env = make_environment();
    try {
        if (argc > 1) {
            for (int i = 1; i < argc; ++i) {
                std::ifstream in(argv[i]);
                if (!in) {
                    std::cerr << "cannot open input " << argv[i] << "\n";
                    continue;
                }
                std::ostringstream ss;
                ss << in.rdbuf();
                auto exprs = parse_many(tokenize(ss.str()));
                (void)eval_sequence(exprs, env);
            }
        } else {
            std::cout << "[pure0, v0.2]\n\nmicro language for sound computing\n\n";
            std::string line;
            while (std::cout << ">> " && std::getline(std::cin, line)) {
                try {
                    auto exprs = parse_many(tokenize(line));
                    ExprPtr result = eval_sequence(exprs, env);
                    std::cout << to_string_value(result) << "\n";
                } catch (const std::exception& e) {
                    std::cerr << "error: " << e.what() << "\n";
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
