; test.scm — full test suite for pure0
; runs top to bottom; any failure throws and aborts with an error message

(def assert (lambda (name cond)
  (if cond
    (print "PASS" name)
    (error (join (list "FAIL:" name) " ")))))

; ── arithmetic ────────────────────────────────────────────────────────────────
(assert "+"          (= (+ 1 2) 3))
(assert "+ multi"    (= (+ 1 2 3 4) 10))
(assert "- unary"    (= (- 5) -5))
(assert "- binary"   (= (- 10 3) 7))
(assert "*"          (= (* 3 4) 12))
(assert "* identity" (= (*) 1))
(assert "/"          (= (/ 10 2) 5))
(assert "pow"        (= (pow 2 10) 1024))

; ── comparisons ───────────────────────────────────────────────────────────────
(assert "= true"     (= (= 3 3) 1))
(assert "= false"    (= (= 3 4) 0))
(assert "< true"     (= (< 1 2) 1))
(assert "< false"    (= (< 2 1) 0))
(assert "> true"     (= (> 5 3) 1))
(assert "> false"    (= (> 3 5) 0))
(assert "not 0"      (= (not 0) 1))
(assert "not nonzero" (= (not 7) 0))

; ── math ──────────────────────────────────────────────────────────────────────
(assert "abs pos"    (= (abs 3) 3))
(assert "abs neg"    (= (abs -5) 5))
(assert "sqrt"       (= (sqrt 9) 3))
(assert "floor"      (= (floor 3.9) 3))
(assert "ceil"       (= (ceil 3.1) 4))
(assert "log/exp"    (= (floor (* (log (exp 2)) 1000)) 2000))
(assert "sin pi"     (< (abs (sin pi)) 0.0001))
(assert "cos 0"      (= (cos 0) 1))

; ── vector construction ───────────────────────────────────────────────────────
(def v (vec 3 1 4 1 5))
(assert "vec len"          (= (len v) 5))
(assert "vec nth"          (= (nth v 2) 4))
(assert "vec empty"        (= (len (vec)) 0))
(assert "zeros"            (= (sum (zeros 6)) 0))
(assert "ones"             (= (sum (ones 4)) 4))
(assert "linspace len"     (= (len (linspace 0 1 11)) 11))
(assert "linspace start"   (= (nth (linspace 0 10 11) 0) 0))
(assert "linspace end"     (= (nth (linspace 0 10 11) 10) 10))
(assert "rand len"         (= (len (rand 8)) 8))
(assert "rand range"       (< (max (rand 100)) 1.00001))

; ── vector ops ────────────────────────────────────────────────────────────────
(assert "sum"      (= (sum v) 14))
(assert "prod"     (= (prod (vec 1 2 3 4 5)) 120))
(assert "min"      (= (min v) 1))
(assert "max"      (= (max v) 5))
(assert "mean"     (= (mean (vec 2 4 6)) 4))
(assert "slice"    (= (sum (slice v 1 3)) 5))
(assert "slice oob" (= (len (slice v 3 99)) 2))
(assert "cat"      (= (len (cat v v)) 10))
(assert "cat sum"  (= (sum (cat (vec 1 2) (vec 3 4))) 10))
(assert "reverse"  (= (nth (reverse (vec 1 2 3)) 0) 3))
(assert "sort"     (= (nth (sort (vec 3 1 2)) 0) 1))
(assert "sort end" (= (nth (sort (vec 3 1 2)) 2) 3))

; ── broadcast arithmetic ──────────────────────────────────────────────────────
(assert "broadcast +"  (= (sum (+ (vec 1 2 3) 10)) 36))
(assert "broadcast *"  (= (nth (* (vec 1 2 3) 2) 2) 6))

; ── lists ─────────────────────────────────────────────────────────────────────
(def lst (list 10 20 30 40))
(assert "list len"    (= (len lst) 4))
(assert "list nth 0"  (= (nth lst 0) 10))
(assert "list nth 2"  (= (nth lst 2) 30))
(assert "quote len"   (= (len '(a b c d e)) 5))

; ── strings ───────────────────────────────────────────────────────────────────
(assert "len string"  (= (len "hello") 5))
(assert "nth string"  (= (match (nth "hello" 1) "e") 1))
(assert "str vec"     (= (match (str (vec 1 2 3)) "\\[") 1))
(assert "str list"    (= (match (str '(x y)) "\\(") 1))
(assert "str number"  (= (match (str 42) "42") 1))
(assert "num"         (= (num "3.14") 3.14))
(assert "num int"     (= (num "100") 100))

; ── type ──────────────────────────────────────────────────────────────────────
(assert "type vec"    (= (match (str (type (vec 1 2))) "vec") 1))
(assert "type list"   (= (match (str (type '(a))) "list") 1))
(assert "type string" (= (match (str (type "hi")) "string") 1))
(assert "type lambda" (= (match (str (type (lambda (x) x))) "lambda") 1))
(assert "type proc"   (= (match (str (type +)) "proc") 1))

; ── match / split / join ──────────────────────────────────────────────────────
(assert "match hit"     (= (match "hello world" "world") 1))
(assert "match miss"    (= (match "hello" "xyz") 0))
(assert "match regex"   (= (match "abc123" "[0-9]+") 1))
(assert "split len"     (= (len (split "a,b,c" ",")) 3))
(assert "split first"   (= (match (nth (split "x:y:z" ":") 0) "x") 1))
(assert "split last"    (= (match (nth (split "x:y:z" ":") 2) "z") 1))
(assert "join"          (= (match (join (list "a" "b" "c") "-") "a-b-c") 1))
(assert "join empty sep" (= (match (join (list "foo" "bar") "") "foobar") 1))

; ── system ────────────────────────────────────────────────────────────────────
(assert "time"         (> (time) 1000000000))
(assert "getenv PATH"  (> (len (getenv "PATH")) 0))
(assert "exec echo"    (= (match (exec "echo hello") "hello") 1))
(assert "exec pwd"     (= (match (exec "pwd") "/") 1))

; ── error ─────────────────────────────────────────────────────────────────────
(assert "error not triggered" (if 1 1 (error "should not reach this")))

; ── control flow ──────────────────────────────────────────────────────────────
(assert "if true"    (= (if 1 42 0) 42))
(assert "if false"   (= (if 0 0 99) 99))
(assert "begin"      (= (begin (def _x 1) (def _x 2) _x) 2))

; ── lambda & higher-order ─────────────────────────────────────────────────────
(def double (lambda (x) (* x 2)))
(assert "lambda call"   (= (double 7) 14))
(assert "apply +"       (= (apply + (list 1 2 3 4)) 10))
(assert "apply *"       (= (apply * (list 2 3 4)) 24))
(def add (lambda (a b) (+ a b)))
(assert "partial"       (= ((add 10) 5) 15))

; ── eval & quote ──────────────────────────────────────────────────────────────
(assert "eval"          (= (eval '(+ 1 2)) 3))
(assert "quote symbol"  (= (match (str 'hello) "hello") 1))

; ── env ───────────────────────────────────────────────────────────────────────
(assert "env is list"   (= (match (str (type (env))) "list") 1))
(assert "env has pi"    (= (match (str (env)) "pi") 1))
(assert "env sorted"    (= (match (str (env)) "abs.*ceil") 1))  ; alphabetical: abs before ceil

; ── file i/o ──────────────────────────────────────────────────────────────────
(write "/tmp/pure0_rw.txt" "pure0 file io test")
(assert "read/write"    (= (match (read "/tmp/pure0_rw.txt") "pure0") 1))

(write "/tmp/pure0_load.p0" "(def _loaded_val 12345)")
(load "/tmp/pure0_load.p0")
(assert "load"          (= _loaded_val 12345))

; ── done ──────────────────────────────────────────────────────────────────────
(print "")
(print "all tests passed")
