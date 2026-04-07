// core.h

#ifndef CORE_H
#define CORE_H

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <functional>
#include <regex>
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
        throw std::runtime_error("unknown symbol " + key);
    }
    void set(const std::string& key, ExprPtr value) {
        table[key] = std::move(value);
    }
};

static ExprPtr make_symbol(const std::string& s) { return std::make_shared<Expr>(Symbol{s}); }
static ExprPtr make_string(const std::string& s) { return std::make_shared<Expr>(String{s}); }
static ExprPtr make_vec(const Vector& x) { return std::make_shared<Expr>(x);}
static ExprPtr make_scalar(double x) { return make_vec(Vector(x, 1)); }
static ExprPtr make_lambda(const LambdaPtr& x) { return std::make_shared<Expr>(x); }
static ExprPtr make_proc(const Proc& x) {  return std::make_shared<Expr>(x); }
static ExprPtr make_list(const Expr::List& x) { return std::make_shared<Expr>(x); }
static bool is_symbol(const ExprPtr& x) { return std::holds_alternative<Symbol>(x->v); }
static bool is_string(const ExprPtr& x) { return std::holds_alternative<String>(x->v); }
static bool is_vec(const ExprPtr& x) { return std::holds_alternative<Vector>(x->v); }
static bool is_lambda(const ExprPtr& x) { return std::holds_alternative<LambdaPtr>(x->v); }
static bool is_proc(const ExprPtr& x) { return std::holds_alternative<Proc>(x->v); }
static bool is_list(const ExprPtr& x) { return std::holds_alternative<Expr::List>(x->v); }
static const std::string& as_symbol(const ExprPtr& x) { return std::get<Symbol>(x->v).s; }
static const std::string& as_string(const ExprPtr& x) { return std::get<String>(x->v).s; }
static Vector as_vec(const ExprPtr& x) {
    if (!is_vec(x)) throw std::runtime_error("expected vector");
    return std::get<Vector>(x->v);
}
static double as_scalar(const ExprPtr& x) {
    Vector s = as_vec(x);
    if (s.size() != 1) throw std::runtime_error("expected scalar");
    return s[0];
}
static bool truthy(const ExprPtr& x) { return as_scalar(x) != 0.0;}

static std::string to_string_value(const ExprPtr& x) {
    if (is_symbol(x)) return as_symbol(x);
    if (is_string(x)) return as_string(x);
    if (is_lambda(x)) return "<lambda>";
    if (is_proc(x))   return "<proc>";

    std::ostringstream oss;
    if (is_list(x)) {
        const auto& xs = std::get<Expr::List>(x->v);
        oss << "(";
        for (std::size_t i = 0; i < xs.size(); ++i) {
            if (i) oss << " ";
            oss << to_string_value(xs[i]);
        }
        oss << ")";
    } else {
        const Vector s = as_vec(x);
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

struct TokenStream {
    std::istream& in;
    std::string   peeked;
    bool          has_peeked = false;

    std::string get() {
        if (has_peeked) { has_peeked = false; return std::move(peeked); }
        return read();
    }
    const std::string& peek() {
        if (!has_peeked) { peeked = read(); has_peeked = true; }
        return peeked;
    }
    bool eof() { return peek().empty(); }

private:
    std::string read() {
        char c;
        for (;;) {
            if (!in.get(c)) return "";
            if (std::isspace((unsigned char)c)) continue;
            if (c == ';') { while (in.get(c) && c != '\n') {} continue; }
            break;
        }
        if (c == '(' || c == ')' || c == '\'')
            return std::string(1, c);
        if (c == '"') {
            std::string s = "\"";
            while (in.get(c) && c != '"') {
                if (c == '\\' && in.peek() != EOF) {
                    char e = static_cast<char>(in.get());
                    if (e == 'n') s.push_back('\n');
                    else if (e == 't') s.push_back('\t');
                    else s.push_back(e);
                } else s.push_back(c);
            }
            s.push_back('"');
            return s;
        }
        std::string tok(1, c);
        while (in.peek() != EOF) {
            char p = static_cast<char>(in.peek());
            if (std::isspace((unsigned char)p) || p == '(' || p == ')' ||
                p == '\'' || p == '"' || p == ';')
                break;
            tok.push_back(static_cast<char>(in.get()));
        }
        return tok;
    }
};

static ExprPtr parse_expr(TokenStream& ts) {
    std::string s = ts.get();
    if (s.empty()) throw std::runtime_error("unexpected eof");
    if (s == "'") {
        Expr::List q;
        q.push_back(make_symbol("quote"));
        q.push_back(parse_expr(ts));
        return make_list(q);
    }
    if (s == "(") {
        Expr::List xs;
        while (ts.peek() != ")" && !ts.eof()) xs.push_back(parse_expr(ts));
        if (ts.eof()) throw std::runtime_error("missing )");
        ts.get();
        return make_list(xs);
    }

    if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
        return make_string(s.substr(1, s.size() - 2));
    if (numtok(s)) return make_scalar(std::strtod(s.c_str(), nullptr));
    return make_symbol(s);
}

static Vector broadcast(const Vector& a, std::size_t n) {
    if (a.size() == n) return a;
    if (a.size() == 1) return Vector(a[0], n);
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

static ExprPtr eval(ExprPtr expr, std::shared_ptr<Env> env);
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
        if (is_vec(expr) || is_string(expr) || is_lambda(expr) || is_proc(expr)) return expr;
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
        Vector r = as_vec(args[0]);
        if (args.size() == 1 && unary_from_init) r = binop(Vector(init, 1), r, f);
        else for (std::size_t i = 1; i < args.size(); ++i) r = binop(r, as_vec(args[i]), f);
        return make_vec(r);
    };
}
static Proc make_numeric_unary(const std::function<double(double)>& f) {
    return [f](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("unary primitive expects 1 argument");
        return make_vec(unop(as_vec(args[0]), f));
    };
}

static bool vec_equal(const Vector& a, const Vector& b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i)
        if (a[i] != b[i]) return false;
    return true;
}
static bool expr_equal(const ExprPtr& a, const ExprPtr& b) {
    if (is_symbol(a) && is_symbol(b)) return as_symbol(a) == as_symbol(b);
    if (is_string(a) && is_string(b)) return as_string(a) == as_string(b);
    if (is_vec(a) && is_vec(b)) return vec_equal(as_vec(a), as_vec(b));
    if (is_list(a) && is_list(b)) {
        const auto& xs = std::get<Expr::List>(a->v);
        const auto& ys = std::get<Expr::List>(b->v);
        if (xs.size() != ys.size()) return false;
        for (std::size_t i = 0; i < xs.size(); ++i)
            if (!expr_equal(xs[i], ys[i])) return false;
        return true;
    }
    return false;
}
static Proc fn_env() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env> env) -> ExprPtr {
        if (!args.empty()) throw std::runtime_error("env expects 0 args");
        std::vector<std::string> names;
        for (auto* e = env.get(); e; e = e->parent.get())
            for (const auto& kv : e->table)
                names.push_back(kv.first);
        std::sort(names.begin(), names.end());
        Expr::List keys;
        for (const auto& n : names) keys.push_back(make_symbol(n));
        return make_list(keys);
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
        if (!in) throw std::runtime_error("cannot open file for reading " + as_string(args[0])); 
        std::ostringstream ss; ss << in.rdbuf(); return make_string(ss.str()); 
    };
}
static Proc fn_write() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr { 
        if (args.size() != 2 || !is_string(args[0])) throw std::runtime_error("write expects path and content"); 
        std::ofstream out(as_string(args[0])); 
        if (!out) throw std::runtime_error("cannot open file for writing " + as_string(args[0])); 
        out << to_string_value(args[1]); return args[1]; 
    };
}
static Proc fn_load() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env> env) -> ExprPtr {
        if (args.size() != 1 || !is_string(args[0])) throw std::runtime_error("load expects one string path");
        std::string name = as_string(args[0]);
        std::ifstream* in = new std::ifstream(name);
        if (!in->good()) {
            const char* home = std::getenv("HOME");
            std::string fallback = home + (std::string) "/.pure0/" + name;
            in = new std::ifstream(fallback);
            if (!in->good()) throw std::runtime_error("cannot open file for loading " + name);
        }
        TokenStream ts{*in};
        ExprPtr result = make_scalar(0.0);
        while (!ts.eof()) result = eval(parse_expr(ts), env);
        delete in;
        return result;
    };
}
static Proc fn_list() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr { return make_list(args); };
}
static Proc fn_vec() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        Vector v(args.size());
        for (std::size_t i = 0; i < args.size(); ++i) v[i] = as_scalar(args[i]);
        return make_vec(v);
    };
}
static Proc fn_nth() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 2) throw std::runtime_error("nth expects 2 args");
        double idx = as_scalar(args[1]);
        if (idx < 0.0) throw std::runtime_error("nth expects a non-negative index");
        std::size_t n = static_cast<std::size_t>(idx);
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
        if (is_vec(args[0])) {
            Vector x = as_vec(args[0]);
            if (n >= x.size()) throw std::runtime_error("nth out of range");
            return make_scalar(x[n]);
        }
        throw std::runtime_error("nth expects a list, string, or vector");
    };
}
static Proc fn_len() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr { 
        if (args.size() != 1) throw std::runtime_error("len expects 1 arg"); 
        if (is_list(args[0])) return make_scalar((double)std::get<Expr::List>(args[0]->v).size()); 
        if (is_string(args[0])) return make_scalar((double)as_string(args[0]).size()); 
        return make_scalar((double)as_vec(args[0]).size()); 
    };
}
static Proc fn_append() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 2) throw std::runtime_error("append expects 2 args");
        if (is_list(args[0]) && is_list(args[1])) {
            Expr::List xs = std::get<Expr::List>(args[0]->v);
            const auto& ys = std::get<Expr::List>(args[1]->v);
            xs.insert(xs.end(), ys.begin(), ys.end());
            return make_list(xs);
        }
        if (is_string(args[0]) && is_string(args[1])) {
            return make_string(as_string(args[0]) + as_string(args[1]));
        }
        if (is_vec(args[0]) && is_vec(args[1])) {
            Vector a = as_vec(args[0]), b = as_vec(args[1]);
            Vector r(a.size() + b.size());
            for (std::size_t i = 0; i < a.size(); ++i) r[i] = a[i];
            for (std::size_t i = 0; i < b.size(); ++i) r[i + a.size()] = b[i];
            return make_vec(r);
        }
        throw std::runtime_error("append expects two lists, two strings, or two vectors");
    };
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
static Proc fn_linspace() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 3) throw std::runtime_error("linspace expects 3 args");
        double start = as_scalar(args[0]);
        double end   = as_scalar(args[1]);
        auto   n     = static_cast<std::size_t>(as_scalar(args[2]));
        Vector r = line(n, start, end);
        return make_vec(r);
    };
}
static Proc fn_rand() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("rand expects 1 arg");
        auto n = static_cast<std::size_t>(as_scalar(args[0]));
        Vector r(n);
        for (std::size_t i = 0; i < n; ++i)
            r[i] = (double)std::rand() / (double)RAND_MAX;
        return make_vec(r);
    };
}

static Proc fn_zeros() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("zeros expects 1 arg");
        return make_vec(Vector(0.0, static_cast<std::size_t>(as_scalar(args[0]))));
    };
}
static Proc fn_ones() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("ones expects 1 arg");
        return make_vec(Vector(1.0, static_cast<std::size_t>(as_scalar(args[0]))));
    };
}
static Proc fn_slice() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 3) throw std::runtime_error("slice expects 3 args");
        double di = as_scalar(args[1]);
        double dj = as_scalar(args[2]);
        if (di < 0.0 || dj < 0.0) throw std::runtime_error("slice expects non-negative indices");
        std::size_t i = static_cast<std::size_t>(di);
        std::size_t j = static_cast<std::size_t>(dj);
        if (is_vec(args[0])) {
            Vector v = as_vec(args[0]);
            if (j > v.size()) j = v.size();
            if (i >= j) return make_vec(Vector(0.0, 0));
            Vector r(j - i);
            for (std::size_t k = 0; k < r.size(); ++k) r[k] = v[i + k];
            return make_vec(r);
        }
        if (is_string(args[0])) {
            const auto& s = as_string(args[0]);
            if (j > s.size()) j = s.size();
            if (i >= j) return make_string("");
            return make_string(s.substr(i, j - i));
        }
        if (is_list(args[0])) {
            const auto& xs = std::get<Expr::List>(args[0]->v);
            if (j > xs.size()) j = xs.size();
            if (i >= j) return make_list({});
            return make_list(Expr::List(xs.begin() + i, xs.begin() + j));
        }
        throw std::runtime_error("slice expects a list, string, or vector");
    };
}
static Vector normalize(const Vector& x, double peak = 1.0) {
    double mx = 0.0;
    for (std::size_t i = 0; i < x.size(); ++i) mx = std::max(mx, std::abs(x[i]));
    if (mx <= 0.0) return x;
    return x * (peak / mx);
}
static Proc fn_normalize() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1 && args.size() != 2) throw std::runtime_error("normalize expects 1 or 2 args");
        Vector v = as_vec(args[0]);
        double peak = args.size() == 2 ? as_scalar(args[1]) : 1.0;
        return make_vec(normalize(v, peak));
    };
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
static Proc fn_take() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 2)
            throw std::runtime_error("take expects: vector n");
        Vector x = as_vec(args[0]);
        std::size_t n = (std::size_t)std::max(0.0, as_scalar(args[1]));
        return make_vec(take(x, n));
    };
}

static Proc fn_repeat() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 2)
            throw std::runtime_error("repeat expects: vector n");
        Vector x = as_vec(args[0]);
        std::size_t n = (std::size_t)std::max(0.0, as_scalar(args[1]));
        return make_vec(repeat_to(x, n));
    };
}
static Proc fn_sum() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("sum expects 1 arg");
        return make_scalar(as_vec(args[0]).sum());
    };
}
static Proc fn_prod() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("prod expects 1 arg");
        Vector v = as_vec(args[0]);
        double r = 1.0;
        for (std::size_t i = 0; i < v.size(); ++i) r *= v[i];
        return make_scalar(r);
    };
}
static Proc fn_min() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("min expects 1 arg");
        return make_scalar(as_vec(args[0]).min());
    };
}
static Proc fn_max() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("max expects 1 arg");
        return make_scalar(as_vec(args[0]).max());
    };
}
static Proc fn_mean() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("mean expects 1 arg");
        Vector v = as_vec(args[0]);
        if (v.size() == 0) throw std::runtime_error("mean of empty vector");
        return make_scalar(v.sum() / (double)v.size());
    };
}
static Proc fn_reverse() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("reverse expects 1 arg");
        if (is_vec(args[0])) {
            Vector v = as_vec(args[0]);
            Vector r(v.size());
            for (std::size_t i = 0; i < v.size(); ++i) r[i] = v[v.size() - 1 - i];
            return make_vec(r);
        }
        if (is_string(args[0])) {
            std::string s = as_string(args[0]);
            std::reverse(s.begin(), s.end());
            return make_string(s);
        }
        if (is_list(args[0])) {
            Expr::List xs = std::get<Expr::List>(args[0]->v);
            std::reverse(xs.begin(), xs.end());
            return make_list(xs);
        }
        throw std::runtime_error("reverse expects a list, string, or vector");
    };
}
static Proc fn_sort() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("sort expects 1 arg");
        Vector v = as_vec(args[0]);
        std::vector<double> tmp(std::begin(v), std::end(v));
        std::sort(tmp.begin(), tmp.end());
        Vector r(tmp.size());
        for (std::size_t i = 0; i < tmp.size(); ++i) r[i] = tmp[i];
        return make_vec(r);
    };
}

static Proc fn_cons() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 2 || !is_list(args[1])) throw std::runtime_error("cons expects element and list");
        Expr::List xs = std::get<Expr::List>(args[1]->v);
        xs.insert(xs.begin(), args[0]);
        return make_list(xs);
    };
}
static Proc fn_head() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1 || !is_list(args[0])) throw std::runtime_error("head expects 1 list arg");
        const auto& xs = std::get<Expr::List>(args[0]->v);
        if (xs.empty()) throw std::runtime_error("head of empty list");
        return xs[0];
    };
}
static Proc fn_tail() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1 || !is_list(args[0])) throw std::runtime_error("tail expects 1 list arg");
        const auto& xs = std::get<Expr::List>(args[0]->v);
        if (xs.empty()) throw std::runtime_error("tail of empty list");
        return make_list(Expr::List(xs.begin() + 1, xs.end()));
    };
}
static Proc fn_null() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("null? expects 1 arg");
        if (is_list(args[0])) return make_scalar(std::get<Expr::List>(args[0]->v).empty() ? 1.0 : 0.0);
        if (is_string(args[0])) return make_scalar(as_string(args[0]).empty() ? 1.0 : 0.0);
        if (is_vec(args[0])) return make_scalar(as_vec(args[0]).size() == 0 ? 1.0 : 0.0);
        throw std::runtime_error("null? expects a list, string, or vector");
    };
}

static Proc fn_str() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("str expects 1 arg");
        return make_string(to_string_value(args[0]));
    };
}
static Proc fn_num() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1 || !is_string(args[0])) throw std::runtime_error("num expects 1 string arg");
        const std::string& s = as_string(args[0]);
        char* end = nullptr;
        double v = std::strtod(s.c_str(), &end);
        if (end == s.c_str() || *end != '\0') throw std::runtime_error("num: cannot parse \"" + s + "\"");
        return make_scalar(v);
    };
}
static Proc fn_type() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("type expects 1 arg");
        const auto& x = args[0];
        if (is_vec(x))    return make_symbol("vec");
        if (is_string(x)) return make_symbol("string");
        if (is_list(x))   return make_symbol("list");
        if (is_lambda(x)) return make_symbol("lambda");
        if (is_proc(x))   return make_symbol("proc");
        if (is_symbol(x)) return make_symbol("symbol");
        return make_symbol("unknown");
    };
}
static Proc fn_exec() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1 || !is_string(args[0])) throw std::runtime_error("exec expects 1 string arg");
        FILE* p = popen(as_string(args[0]).c_str(), "r");
        if (!p) throw std::runtime_error("exec: popen failed");
        std::string result;
        char buf[256];
        while (fgets(buf, sizeof(buf), p)) result += buf;
        pclose(p);
        return make_string(result);
    };
}
static Proc fn_getenv() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1 || !is_string(args[0])) throw std::runtime_error("getenv expects 1 string arg");
        const char* v = std::getenv(as_string(args[0]).c_str());
        return make_string(v ? v : "");
    };
}
static Proc fn_time() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (!args.empty()) throw std::runtime_error("time expects 0 args");
        return make_scalar(static_cast<double>(std::time(nullptr)));
    };
}
static Proc fn_error() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1 || !is_string(args[0])) throw std::runtime_error("error expects 1 string arg");
        throw std::runtime_error(as_string(args[0]));
    };
}
static Proc fn_match() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 2 || !is_string(args[0]) || !is_string(args[1]))
            throw std::runtime_error("match expects 2 string args");
        std::regex re(as_string(args[1]));
        return make_scalar(std::regex_search(as_string(args[0]), re) ? 1.0 : 0.0);
    };
}
static Proc fn_split() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 2 || !is_string(args[0]) || !is_string(args[1]))
            throw std::runtime_error("split expects 2 string args");
        const std::string& s   = as_string(args[0]);
        const std::string& sep = as_string(args[1]);
        Expr::List parts;
        if (sep.empty()) {
            for (char c : s) parts.push_back(make_string(std::string(1, c)));
        } else {
            std::size_t start = 0, pos;
            while ((pos = s.find(sep, start)) != std::string::npos) {
                parts.push_back(make_string(s.substr(start, pos - start)));
                start = pos + sep.size();
            }
            parts.push_back(make_string(s.substr(start)));
        }
        return make_list(parts);
    };
}
static Proc fn_join() {
    return [](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 2 || !is_list(args[0]) || !is_string(args[1]))
            throw std::runtime_error("join expects a list and a string separator");
        const auto& xs     = std::get<Expr::List>(args[0]->v);
        const std::string& sep = as_string(args[1]);
        std::string result;
        for (std::size_t i = 0; i < xs.size(); ++i) {
            if (i) result += sep;
            result += to_string_value(xs[i]);
        }
        return make_string(result);
    };
}

static std::shared_ptr<Env> make_environment() {
    auto env = std::make_shared<Env>();
    // basic constants and arithmetic
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
    env->set("log", make_proc(make_numeric_unary([](double x) {
        return std::log(x);
    })));
    env->set("floor", make_proc(make_numeric_unary([](double x) {
        return std::floor(x);
    })));
    env->set("ceil", make_proc(make_numeric_unary([](double x) {
        return std::ceil(x);
    })));
    env->set("=",  make_proc([](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 2) throw std::runtime_error("= expects 2 args");
        return make_scalar(expr_equal(args[0], args[1]) ? 1.0 : 0.0);
    }));
    env->set("<",  make_proc([](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 2) throw std::runtime_error("< expects 2 args");
        return make_scalar(as_scalar(args[0]) < as_scalar(args[1]) ? 1.0 : 0.0);
    }));
    env->set(">",  make_proc([](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 2) throw std::runtime_error("> expects 2 args");
        return make_scalar(as_scalar(args[0]) > as_scalar(args[1]) ? 1.0 : 0.0);
    }));
    env->set("<=",  make_proc([](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 2) throw std::runtime_error("<= expects 2 args");
        return make_scalar(as_scalar(args[0]) <= as_scalar(args[1]) ? 1.0 : 0.0);
    }));
    env->set(">=",  make_proc([](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 2) throw std::runtime_error(">= expects 2 args");
        return make_scalar(as_scalar(args[0]) >= as_scalar(args[1]) ? 1.0 : 0.0);
    }));
    env->set("not", make_proc([](const std::vector<ExprPtr>& args, std::shared_ptr<Env>) -> ExprPtr {
        if (args.size() != 1) throw std::runtime_error("not expects 1 arg");
        return make_scalar(as_scalar(args[0]) == 0.0 ? 1.0 : 0.0);
    }));
    // environment
    env->set("env", make_proc(fn_env()));
    env->set("type",    make_proc(fn_type()));
    // vectors and lists
    env->set("linspace", make_proc(fn_linspace()));
    env->set("rand", make_proc(fn_rand()));
    env->set("zeros", make_proc(fn_zeros()));
    env->set("ones", make_proc(fn_ones()));
    env->set("slice", make_proc(fn_slice()));
    env->set("normalize", make_proc(fn_normalize()));  
    env->set("take", make_proc(fn_take()));   
    env->set("repeat", make_proc(fn_repeat()));
    env->set("sum", make_proc(fn_sum()));
    env->set("prod", make_proc(fn_prod()));
    env->set("min", make_proc(fn_min()));
    env->set("max", make_proc(fn_max()));
    env->set("mean", make_proc(fn_mean()));
    env->set("reverse", make_proc(fn_reverse()));
    env->set("sort", make_proc(fn_sort()));
    env->set("list",   make_proc(fn_list()));
    env->set("vec",    make_proc(fn_vec()));
    env->set("cons",   make_proc(fn_cons()));
    env->set("head",   make_proc(fn_head()));
    env->set("tail",   make_proc(fn_tail()));
    env->set("null?",  make_proc(fn_null()));
    env->set("append", make_proc(fn_append()));
    env->set("cat",    make_proc(fn_append()));
    env->set("nth",    make_proc(fn_nth()));
    env->set("len",    make_proc(fn_len()));
    // I/O
    env->set("print", make_proc(fn_print()));
    env->set("read",  make_proc(fn_read()));
    env->set("write", make_proc(fn_write()));
    env->set("load",  make_proc(fn_load()));
    // system
    env->set("exec",    make_proc(fn_exec()));
    env->set("getenv",  make_proc(fn_getenv()));
    env->set("time",    make_proc(fn_time()));
    env->set("error",   make_proc(fn_error()));
    // strings
    env->set("str",     make_proc(fn_str()));
    env->set("num",     make_proc(fn_num()));
    env->set("match",   make_proc(fn_match()));
    env->set("split",   make_proc(fn_split()));
    env->set("join",    make_proc(fn_join()));

    return env;
}

void repl(std::shared_ptr<Env> env) {
    TokenStream ts{std::cin};
    std::cout << ">> " << std::flush;
    while (!ts.eof()) {
        try {
            ExprPtr result = eval(parse_expr(ts), env);
            std::cout << to_string_value(result) << "\n>> " << std::flush;
        } catch (const std::exception& e) {
            std::cerr << "error: " << e.what() << "\n>> " << std::flush;
        }
    }
    std::cout << "\n";
}
#endif // CORE_H

// eof

