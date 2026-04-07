; test_core.scm — tests for pure0 core.h
; This file exercises only core primitives. No stdlib required.

(def assert
  (lambda (name cond)
    (if cond
        (print "PASS" name)
        (error (append "FAIL: " name)))))

(def assert-eq
  (lambda (name got expected)
    (assert name (= got expected))))

(def assert-near
  (lambda (name got expected eps)
    (assert name (< (abs (- got expected)) eps))))

; --------------------------------------------------
; types and predicates
; --------------------------------------------------

(assert-eq "type scalar" (type 1) 'vec)
(assert-eq "type vec"    (type (vec 1 2 3)) 'vec)
(assert-eq "type string" (type "abc") 'string)
(assert-eq "type list"   (type '(1 2 3)) 'list)
(assert-eq "type symbol" (type 'abc) 'symbol)
(assert-eq "type lambda" (type (lambda (x) x)) 'lambda)
(assert-eq "type proc"   (type +) 'proc)

(assert-eq "not false" (not 0) 1)
(assert-eq "not true"  (not 1) 0)

; --------------------------------------------------
; equality and comparisons
; --------------------------------------------------

(assert-eq "= scalar" (= 3 3) 1)
(assert-eq "= scalar false" (= 3 4) 0)
(assert-eq "= vec true" (= (vec 1 2 3) (vec 1 2 3)) 1)
(assert-eq "= vec false" (= (vec 1 2 3) (vec 1 2 4)) 0)
(assert-eq "= list true" (= '(1 2 3) '(1 2 3)) 1)
(assert-eq "= list false" (= '(1 2 3) '(1 3 2)) 0)
(assert-eq "= string true" (= "abc" "abc") 1)
(assert-eq "= string false" (= "abc" "abd") 0)
(assert-eq "= symbol true" (= 'abc 'abc) 1)
(assert-eq "= symbol false" (= 'abc 'abd) 0)
(assert-eq "= mixed false" (= "1" 1) 0)

(assert-eq "< true"  (< 1 2) 1)
(assert-eq "< false" (< 2 1) 0)
(assert-eq "> true"  (> 2 1) 1)
(assert-eq "> false" (> 1 2) 0)
(assert-eq "<= true eq" (<= 2 2) 1)
(assert-eq "<= true lt" (<= 1 2) 1)
(assert-eq "<= false"   (<= 3 2) 0)
(assert-eq ">= true eq" (>= 2 2) 1)
(assert-eq ">= true gt" (>= 3 2) 1)
(assert-eq ">= false"   (>= 1 2) 0)

; --------------------------------------------------
; arithmetic / math
; --------------------------------------------------

(assert-eq "+ reduce" (+ 1 2 3 4) 10)
(assert-eq "- unary"  (- 5) -5)
(assert-eq "- binary" (- 7 2) 5)
(assert-eq "* reduce" (* 2 3 4) 24)
(assert-near "/ binary" (/ 7 2) 3.5 0.0001)

(assert-near "sin pi/2" (sin (/ pi 2)) 1 0.0001)
(assert-near "cos 0"    (cos 0) 1 0.0001)
(assert-near "sqrt 2"   (sqrt 2) 1.41421356 0.0001)
(assert-near "log exp"  (log (exp 3)) 3 0.0001)
(assert-eq "floor" (floor 3.9) 3)
(assert-eq "ceil"  (ceil 3.1) 4)
(assert-near "abs" (abs -7) 7 0.0001)

; vector broadcasting
(assert-eq "vec + scalar" (+ (vec 1 2 3) 10) (vec 11 12 13))
(assert-eq "vec * scalar" (* (vec 1 2 3) 2)  (vec 2 4 6))
(assert-eq "vec + vec"    (+ (vec 1 2 3) (vec 4 5 6)) (vec 5 7 9))
(assert-eq "broadcast short vec" (+ (vec 1 2 3) (vec 10)) (vec 11 12 13))

; --------------------------------------------------
; special forms / functions
; --------------------------------------------------

(assert-eq "quote symbol" 'abc 'abc)
(assert-eq "quote list"   '(1 2 3) '(1 2 3))

(def id (lambda (x) x))
(assert-eq "lambda call" (id 42) 42)

(def add3 (lambda (a b c) (+ a b c)))
(assert-eq "partial lambda 1" ((add3 1) 2 3) 6)
(assert-eq "partial lambda 2" (((add3 1) 2) 3) 6)
(assert-eq "overapply lambda" (add3 1 2 3) 6)

(assert-eq "begin" (begin 1 2 3) 3)
(assert-eq "if true"  (if 1 10 20) 10)
(assert-eq "if false" (if 0 10 20) 20)

(assert-eq "eval quoted form" (eval '(+ 2 3)) 5)
(assert-eq "apply +" (apply + '(1 2 3 4)) 10)

; --------------------------------------------------
; sequence constructors and queries
; --------------------------------------------------

(assert-eq "list constructor" (list 1 2 3) '(1 2 3))
(assert-eq "vec constructor"  (vec 1 2 3) (vec 1 2 3))

(assert-eq "len list"   (len '(1 2 3)) 3)
(assert-eq "len vec"    (len (vec 1 2 3 4)) 4)
(assert-eq "len string" (len "abcd") 4)

(assert-eq "nth list"   (nth '(10 20 30) 1) 20)
(assert-eq "nth vec"    (nth (vec 10 20 30) 2) 30)
(assert-eq "nth string" (nth "abcd" 2) "c")

(assert-eq "head list" (head '(10 20 30)) 10)
(assert-eq "tail list" (tail '(10 20 30)) '(20 30))
(assert-eq "cons list" (cons 5 '(10 20)) '(5 10 20))

(assert-eq "null? empty list" (null? '()) 1)
(assert-eq "null? nonempty list" (null? '(1)) 0)
(assert-eq "null? empty string" (null? "") 1)
(assert-eq "null? nonempty string" (null? "x") 0)
(assert-eq "null? empty vec" (null? (zeros 0)) 1)
(assert-eq "null? nonempty vec" (null? (vec 1)) 0)

; --------------------------------------------------
; append / reverse / slice
; --------------------------------------------------

(assert-eq "append list"   (append '(1 2) '(3 4)) '(1 2 3 4))
(assert-eq "append string" (append "ab" "cd") "abcd")
(assert-eq "append vec"    (append (vec 1 2) (vec 3 4)) (vec 1 2 3 4))
(assert-eq "cat alias"     (cat (vec 1 2) (vec 3 4)) (vec 1 2 3 4))

(assert-eq "reverse list"   (reverse '(1 2 3)) '(3 2 1))
(assert-eq "reverse string" (reverse "abcd") "dcba")
(assert-eq "reverse vec"    (reverse (vec 1 2 3 4)) (vec 4 3 2 1))

(assert-eq "slice list"   (slice '(1 2 3 4 5) 1 4) '(2 3 4))
(assert-eq "slice string" (slice "abcdef" 2 5) "cde")
(assert-eq "slice vec"    (slice (vec 1 2 3 4 5) 1 4) (vec 2 3 4))
(assert-eq "slice empty list" (slice '(1 2 3) 2 2) '())
(assert-eq "slice empty string" (slice "abc" 2 2) "")
(assert-eq "slice empty vec" (slice (vec 1 2 3) 2 2) (zeros 0))

; --------------------------------------------------
; vector constructors / transforms
; --------------------------------------------------

(assert-eq "zeros" (zeros 4) (vec 0 0 0 0))
(assert-eq "ones"  (ones 3)  (vec 1 1 1))
(assert-eq "linspace len" (len (linspace 0 1 5)) 5)
(assert-near "linspace first" (nth (linspace 0 1 5) 0) 0 0.0001)
(assert-near "linspace last"  (nth (linspace 0 1 5) 4) 1 0.0001)

(assert-eq "take truncate" (take (vec 1 2 3 4) 2) (vec 1 2))
(assert-eq "take pad"      (take (vec 1 2) 4) (vec 1 2 0 0))
(assert-eq "repeat extend" (repeat (vec 1 2 3) 8) (vec 1 2 3 1 2 3 1 2))
(assert-eq "repeat empty out" (repeat (zeros 0) 4) (vec 0 0 0 0))

(assert-near "sum vec"  (sum (vec 1 2 3 4)) 10 0.0001)
(assert-near "prod vec" (prod (vec 1 2 3 4)) 24 0.0001)
(assert-near "min vec"  (min (vec 3 1 4 2)) 1 0.0001)
(assert-near "max vec"  (max (vec 3 1 4 2)) 4 0.0001)
(assert-near "mean vec" (mean (vec 2 4 6 8)) 5 0.0001)

(assert-eq "sort vec" (sort (vec 4 1 3 2)) (vec 1 2 3 4))
(assert-near "normalize peak" (max (normalize (vec 1 2 4))) 1 0.0001)
(assert-near "normalize peak arg" (max (normalize (vec 1 2 4) 0.5)) 0.5 0.0001)

; --------------------------------------------------
; strings and system-ish helpers
; --------------------------------------------------

(assert-eq "str scalar" (str 12) "[12]")
(assert-eq "str list"   (str '(1 2)) "([1] [2])")

(assert-eq "str symbol" (str 'abc) "abc")
(assert-eq "str string" (str "abc") "abc")
(assert-near "num" (num "3.25") 3.25 0.0001)
(assert-eq "split string" (split "a,b,c" ",") '("a" "b" "c"))
(assert-eq "split empty sep" (split "abc" "") '("a" "b" "c"))
(assert-eq "join" (join '("aa" "bb" "cc") "-") "aa-bb-cc")
(assert-eq "match true" (match "abcdef" "bcd") 1)
(assert-eq "match false" (match "abcdef" "xyz") 0)

; --------------------------------------------------
; env / aliases / extra sanity
; --------------------------------------------------

(assert "env contains +" (match (str (env)) "\\+") )
(assert "env contains append" (match (str (env)) "append"))
(assert-eq "append alias cat vec" (cat (vec 9) (vec 8)) (vec 9 8))

(print "")
(print "all core tests passed")
