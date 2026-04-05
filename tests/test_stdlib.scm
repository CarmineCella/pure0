; test_stdlib.scm — test suite for stdlib.scm
; run with: pure0 test_stdlib.scm

(load "stdlib.scm")

(def assert (lambda (name cond)
  (if cond
    (print "PASS" name)
    (error (join (list "FAIL:" name) " ")))))

(def assert-eq (lambda (name got expected)
  (assert name (= got expected))))

(def assert-near (lambda (name got expected eps)
  (assert name (< (abs (- got expected)) eps))))


; ── type predicates ───────────────────────────────────────────────────────────

(assert     "list? true"    (list? '(1 2 3)))
(assert     "list? empty"   (list? '()))
(assert     "vec? true"     (vec? (vec 1 2 3)))
(assert     "vec? scalar"   (vec? 42))
(assert     "string? true"  (string? "hi"))
(assert     "lambda? true"  (lambda? (lambda (x) x)))
(assert     "proc? true"    (proc? +))
(assert     "fn? lambda"    (fn? (lambda (x) x)))
(assert     "fn? proc"      (fn? map))
(assert-eq  "not list?"     (list? 42) 0)
(assert-eq  "not vec?"      (vec? "hi") 0)
(assert-eq  "not string?"   (string? (vec 1)) 0)


; ── function utilities ────────────────────────────────────────────────────────

(def dbl (lambda (x) (* x 2)))
(def inc (lambda (x) (+ x 1)))

(assert-eq  "identity"      (identity 42) 42)
(assert-eq  "const"         ((const 7) 99) 7)
(assert-eq  "compose"       ((compose dbl inc) 5) 12)   ; dbl(inc(5)) = 12
(assert-eq  "flip"          ((flip -) 3 10) 7)           ; -(10,3) = 7
(assert-eq  "iterate"       (iterate dbl 4 1) 16)        ; 1→2→4→8→16


; ── list construction ─────────────────────────────────────────────────────────

(assert-eq  "iota len"      (len (iota 5)) 5)
(assert-eq  "iota first"    (nth (iota 5) 0) 0)
(assert-eq  "iota last"     (nth (iota 5) 4) 4)
(assert-eq  "iota zero"     (len (iota 0)) 0)

(assert-eq  "range len"     (len (range 2 7)) 5)
(assert-eq  "range first"   (nth (range 2 7) 0) 2)
(assert-eq  "range last"    (nth (range 2 7) 4) 6)

(assert-eq  "replicate len" (len (replicate 4 99)) 4)
(assert-eq  "replicate val" (nth (replicate 3 7) 2) 7)


; ── list access ───────────────────────────────────────────────────────────────

(def l5 (list 10 20 30 40 50))

(assert-eq  "second"        (second l5) 20)
(assert-eq  "third"         (third l5) 30)
(assert-eq  "last"          (last l5) 50)
(assert-eq  "init len"      (len (init l5)) 4)
(assert-eq  "init last"     (last (init l5)) 40)


; ── map ───────────────────────────────────────────────────────────────────────

(assert-eq  "map len"       (len (map square (iota 5))) 5)
(assert-eq  "map val"       (nth (map square (range 1 6)) 2) 9)   ; square(3)=9
(assert-eq  "map empty"     (len (map square '())) 0)
(assert     "map str"       (= (match (str (map dbl (list 1 2 3))) "2") 1))


; ── filter ───────────────────────────────────────────────────────────────────

(def pos? (lambda (x) (> x 0)))
(def neg? (lambda (x) (< x 0)))
(def data (list -3 -1 0 2 4 5))

(assert-eq  "filter len"    (len (filter pos? data)) 3)
(assert-eq  "filter first"  (nth (filter pos? data) 0) 2)
(assert-eq  "filter empty"  (len (filter pos? '())) 0)


; ── reduce ────────────────────────────────────────────────────────────────────

(assert-eq  "reduce sum"    (reduce + 0 (iota 5)) 10)
(assert-eq  "reduce prod"   (reduce * 1 (range 1 6)) 120)
(assert-eq  "reduce empty"  (reduce + 0 '()) 0)
(assert-eq  "reduce build"  (len (reduce (lambda (acc x) (cons x acc)) '() (iota 4))) 4)


; ── for-each ─────────────────────────────────────────────────────────────────

; side-effect only — test it runs without error
; (def fe-result 0)
; (for-each (lambda (x) (def fe-result (+ fe-result x))) (list 1 2 3))
; (assert-eq  "for-each"      fe-result 6)


; ── flat-map ──────────────────────────────────────────────────────────────────

(def pair-up (lambda (x) (list x (- x))))
(assert-eq  "flat-map len"  (len (flat-map pair-up (list 1 2 3))) 6)
(assert-eq  "flat-map val"  (nth (flat-map pair-up (list 1 2 3)) 1) -1)


; ── any / all / member? / count ───────────────────────────────────────────────

(assert-eq  "any true"      (any pos? (list -1 0 2)) 1)
(assert-eq  "any false"     (any pos? (list -1 -2)) 0)
(assert-eq  "any empty"     (any pos? '()) 0)

(assert-eq  "all true"      (all pos? (list 1 2 3)) 1)
(assert-eq  "all false"     (all pos? (list 1 -1 3)) 0)
(assert-eq  "all empty"     (all pos? '()) 1)

(assert-eq  "member? num yes"    (member? 3 (list 1 2 3 4)) 1)
(assert-eq  "member? num no"     (member? 5 (list 1 2 3)) 0)
(assert-eq  "member? sym yes"    (member? 'b '(a b c)) 1)
(assert-eq  "member? sym no"     (member? 'd '(a b c)) 0)
(assert-eq  "equal? nums"        (equal? 3 3) 1)
(assert-eq  "equal? nums diff"   (equal? 3 4) 0)
(assert-eq  "equal? syms"        (equal? 'foo 'foo) 1)
(assert-eq  "equal? syms diff"   (equal? 'foo 'bar) 0)

(assert-eq  "count"         (count pos? (list -1 2 -3 4 5)) 3)
(assert-eq  "count none"    (count pos? (list -1 -2)) 0)


; ── reverse-list ──────────────────────────────────────────────────────────────

(def rev3 (reverse-list (list 1 2 3)))
(assert-eq  "reverse-list first"  (nth rev3 0) 3)
(assert-eq  "reverse-list last"   (nth rev3 2) 1)
(assert-eq  "reverse-list empty"  (len (reverse-list '())) 0)


; ── take / drop ───────────────────────────────────────────────────────────────

(assert-eq  "take len"      (len (take 3 (iota 10))) 3)
(assert-eq  "take val"      (nth (take 3 (iota 10)) 2) 2)
(assert-eq  "take all"      (len (take 100 (iota 5))) 5)
(assert-eq  "take zero"     (len (take 0 (iota 5))) 0)

(assert-eq  "drop len"      (len (drop 3 (iota 10))) 7)
(assert-eq  "drop first"    (nth (drop 3 (iota 10)) 0) 3)
(assert-eq  "drop all"      (len (drop 100 (iota 5))) 0)


; ── take-while / drop-while ───────────────────────────────────────────────────

(def lt5 (lambda (x) (< x 5)))

(assert-eq  "take-while len"    (len (take-while lt5 (iota 10))) 5)
(assert-eq  "take-while last"   (last (take-while lt5 (iota 10))) 4)
(assert-eq  "take-while none"   (len (take-while neg? (iota 5))) 0)

(assert-eq  "drop-while first"  (nth (drop-while lt5 (iota 10)) 0) 5)
(assert-eq  "drop-while len"    (len (drop-while lt5 (iota 10))) 5)
(assert-eq  "drop-while all"    (len (drop-while neg? (iota 5))) 5)


; ── zip / zip-with ────────────────────────────────────────────────────────────

(def za (list 1 2 3))
(def zb (list 10 20 30))
(def zipped (zip za zb))

(assert-eq  "zip len"           (len zipped) 3)
(assert-eq  "zip pair first"    (nth (head zipped) 0) 1)
(assert-eq  "zip pair second"   (nth (head zipped) 1) 10)
(assert-eq  "zip shorter"       (len (zip (list 1 2) (list 10 20 30))) 2)

(def summed (zip-with + za zb))
(assert-eq  "zip-with len"      (len summed) 3)
(assert-eq  "zip-with val"      (nth summed 1) 22)


; ── flatten ───────────────────────────────────────────────────────────────────

(assert-eq  "flatten len"   (len (flatten '(1 (2 3) (4 (5 6))))) 6)
(assert-eq  "flatten first" (nth (flatten '(1 (2 3))) 0) 1)
(assert-eq  "flatten deep"  (nth (flatten '((1 (2)) (3))) 1) 2)
(assert-eq  "flatten flat"  (len (flatten (list 1 2 3))) 3)


; ── partition ────────────────────────────────────────────────────────────────

(def parts (partition pos? (list -2 1 -3 4 5)))
(assert-eq  "partition yes len"  (len (nth parts 0)) 3)
(assert-eq  "partition no len"   (len (nth parts 1)) 2)
(assert-eq  "partition yes val"  (nth (nth parts 0) 0) 1)
(assert-eq  "partition no val"   (nth (nth parts 1) 0) -2)


; ── find / index-of ───────────────────────────────────────────────────────────

(assert-eq  "find hit"      (find pos? (list -1 -2 3 4)) 3)
(assert      "find miss"    (null? (find pos? (list -1 -2))))
(assert-eq  "index-of hit"  (index-of pos? (list -1 -2 3 4)) 2)
(assert-eq  "index-of miss" (index-of pos? (list -1 -2)) -1)


; ── assoc / assoc-get ────────────────────────────────────────────────────────

(def db (list (list 'name "Alice") (list 'age 30) (list 'score 99)))

(assert     "assoc found"       (not (null? (assoc 'age db))))
(assert     "assoc miss"        (null? (assoc 'missing db)))
(assert-eq  "assoc-get hit"     (assoc-get 'score db 0) 99)
(assert-eq  "assoc-get miss"    (assoc-get 'missing db 42) 42)
(assert-eq  "assoc-get age"     (assoc-get 'age db -1) 30)


; ── math ─────────────────────────────────────────────────────────────────────

(assert-eq   "square"       (square 7) 49)
(assert-eq   "cube"         (cube 3) 27)
(assert-eq   "clamp lo"     (clamp 0 1 -0.5) 0)
(assert-eq   "clamp hi"     (clamp 0 1 1.5) 1)
(assert-eq   "clamp mid"    (clamp 0 1 0.5) 0.5)
(assert-eq   "lerp 0"       (lerp 0 100 0) 0)
(assert-eq   "lerp 1"       (lerp 0 100 1) 100)
(assert-eq   "lerp mid"     (lerp 0 100 0.3) 30)
(assert-eq   "norm 3-4-5"   (norm (vec 3 4)) 5)
(assert-near "normalize x"  (nth (normalize (vec 3 4)) 0) 0.6 0.0001)
(assert-near "normalize y"  (nth (normalize (vec 3 4)) 1) 0.8 0.0001)
(assert-eq   "dot"          (dot (vec 1 2 3) (vec 4 5 6)) 32)
(assert-eq   "sign neg"     (sign -5) -1)
(assert-eq   "sign pos"     (sign 3) 1)
(assert-eq   "sign zero"    (sign 0) 0)
(assert-eq   "cumsum last"  (nth (cumsum (vec 1 2 3 4 5)) 4) 15)
(assert-eq   "cumsum mid"   (nth (cumsum (vec 1 2 3 4 5)) 2) 6)
(assert-eq   "cumsum first" (nth (cumsum (vec 1 2 3 4 5)) 0) 1)

; ── string utilities ──────────────────────────────────────────────────────────

(assert     "starts-with? yes"  (starts-with? "hello world" "hello"))
(assert-eq  "starts-with? no"   (starts-with? "hello world" "world") 0)
(assert     "ends-with? yes"    (ends-with? "hello world" "world"))
(assert-eq  "ends-with? no"     (ends-with? "hello world" "hello") 0)


; ─────────────────────────────────────────────────────────────────────────────
(print "")
(print "all stdlib tests passed")
