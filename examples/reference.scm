; reference.scm — pure0 language reference
; run with: pure0 reference.scm
;
; pure0 is a minimal language with a math soul.
; Everything is an expression. No statements, no mutation (except def).

(load "stdlib.scm")


; ════════════════════════════════════════════════════════════════════════════
; 1. SCALARS AND ARITHMETIC
; ════════════════════════════════════════════════════════════════════════════
; Scalars are 1-element vectors. Arithmetic is variadic and left-associative.

(print (+ 1 2 3 4))       ; 10
(print (- 100 30 20))     ; 50
(print (* 2 3 4))         ; 24
(print (/ 100 4 5))       ; 5
(print (- 7))             ; -7  — unary minus
(print (pow 2 16))        ; 65536

(print (sqrt 2))          ; 1.41421
(print (log (exp 1)))     ; 1
(print (floor 3.9))       ; 3
(print (ceil  3.1))       ; 4
(print (abs  -99))        ; 99

(print pi)                ; 3.14159
(print (sin pi))          ; ~0
(print (cos 0))           ; 1

; comparisons return 1 (true) or 0 (false)
(print (= 3 3))           ; 1
(print (< 1 2))           ; 1
(print (> 2 1))           ; 1
(print (not 0))           ; 1
(print (not 5))           ; 0


; ════════════════════════════════════════════════════════════════════════════
; 2. VECTORS — the primary numeric type
; ════════════════════════════════════════════════════════════════════════════
; Vectors are fixed-length arrays of doubles with broadcast arithmetic.

(def v (vec 1 2 3 4 5))

; element-wise arithmetic — scalar broadcasts to match length
(print (+ v 10))                 ; [11 12 13 14 15]
(print (* v 2))                  ; [2 4 6 8 10]
(print (* v v))                  ; [1 4 9 16 25]
(print (+ (vec 1 2 3) (vec 10 20 30)))   ; [11 22 33]

; construction
(print (zeros 4))                ; [0 0 0 0]
(print (ones 3))                 ; [1 1 1]
(print (linspace 0 1 5))         ; [0 0.25 0.5 0.75 1]
(print (rand 4))                 ; 4 values in [0,1)

; access and slicing
(print (nth v 0))                ; 1
(print (nth v 4))                ; 5
(print (slice v 1 4))            ; [2 3 4]

; structural
(print (cat (vec 1 2) (vec 3 4)))   ; [1 2 3 4]
(print (reverse v))                 ; [5 4 3 2 1]
(print (sort (vec 3 1 4 1 5 9 2)))
(print (len v))                  ; 5

; reductions
(print (sum v))                  ; 15
(print (prod v))                 ; 120
(print (min v))                  ; 1
(print (max v))                  ; 5
(print (mean v))                 ; 3

; vector of squares via broadcast
(def ns (linspace 1 8 8))
(print (* ns ns))                ; [1 4 9 16 25 36 49 64]


; ════════════════════════════════════════════════════════════════════════════
; 3. LISTS — heterogeneous, symbolic
; ════════════════════════════════════════════════════════════════════════════
; Lists are singly-linked, heterogeneous. They can hold any value.

; quote: elements are NOT evaluated (symbols stay symbols, numbers become scalars)
(def colors '(red green blue))

; list: elements ARE evaluated
(def nums (list (+ 1 0) (+ 1 1) (+ 1 2)))   ; (1 2 3)

; primitives
(print (head nums))              ; 1
(print (tail nums))              ; (2 3)
(print (null? '()))              ; 1  — empty list check
(print (null? nums))             ; 0

; cons prepends one element
(print (cons 0 nums))            ; (0 1 2 3)
(print (cons 'x '(y z)))         ; (x y z)

; append joins two lists
(print (append '(a b) '(c d)))   ; (a b c d)
(print (append nums (list 4 5))) ; (1 2 3 4 5)

; construction helpers (from stdlib)
(print (iota 5))                 ; (0 1 2 3 4)
(print (range 3 7))              ; (3 4 5 6)
(print (replicate 4 0))          ; (0 0 0 0)

; access helpers (from stdlib)
(def l5 (list 10 20 30 40 50))
(print (second l5))              ; 20
(print (third  l5))              ; 30
(print (last   l5))              ; 50
(print (init   l5))              ; (10 20 30 40)   — all but last

; len and nth work on lists too
(print (len colors))             ; 3
(print (nth colors 0))           ; red

; stdlib helpers for association lists and metadata-like records
(def rec '((instrument "Ob") (articulation "ord") (pitch "A#3") (dynamic "ff")))
(print (assoc 'pitch rec))       ; (pitch "A#3")
(print (second (assoc 'pitch rec))) ; "A#3"

(def recs '(((instrument "Ob") (pitch "A#3"))
            ((instrument "Cl") (pitch "B3"))
            ((instrument "Ob") (pitch "C4"))))
(print (field-values recs 'instrument)) ; ("Ob" "Cl" "Ob")
(print (sort-uniq (field-values recs 'instrument))) ; unique values, order-preserving


; lists are code — eval executes them
(print '(+ 1 2))                 ; (+ 1 2) — unevaluated
(print (eval '(+ 1 2)))          ; 3


; ════════════════════════════════════════════════════════════════════════════
; 4. STRINGS
; ════════════════════════════════════════════════════════════════════════════

(def greeting "hello, world")

(print (len greeting))           ; 12
(print (nth greeting 0))         ; h  — single char as string

; conversion
(print (str (vec 1 2 3)))        ; [1 2 3]
(print (str '(a b c)))           ; (a b c)
(print (num "2.718"))            ; 2.718

; regex matching — returns 1 or 0
(print (match "pure0 is cool" "cool"))   ; 1
(print (match "pure0 is cool" "^is"))    ; 0
(print (match "abc123" "[0-9]+"))        ; 1

; split and join
(def parts (split "one,two,three" ","))
(print parts)                            ; (one two three)
(print (join parts " | "))               ; one | two | three
(print (join (list "x=" (str 42)) ""))   ; x=42

; stdlib string predicates
(print (starts-with? "hello world" "hello"))   ; 1
(print (ends-with?   "hello world" "world"))   ; 1
(print (starts-with? "hello world" "world"))   ; 0


; ════════════════════════════════════════════════════════════════════════════
; 5. TYPES
; ════════════════════════════════════════════════════════════════════════════
; type returns a symbol: vec | string | list | lambda | proc

(print (type 42))                ; vec  (scalars are 1-element vectors)
(print (type (vec 1 2)))         ; vec
(print (type "hi"))              ; string
(print (type '(a b)))            ; list
(print (type +))                 ; proc
(print (type (lambda (x) x)))    ; lambda

; stdlib type predicates — return 1/0
(print (vec?    v))              ; 1
(print (list?   colors))         ; 1
(print (string? greeting))       ; 1
(print (proc?   +))              ; 1
(print (lambda? map))            ; 1  — map is a lambda in stdlib
(print (fn?     reduce))         ; 1  — true for both lambda and proc


; ════════════════════════════════════════════════════════════════════════════
; 6. FUNCTIONS AND CLOSURES
; ════════════════════════════════════════════════════════════════════════════

; basic lambda — stdlib already defines square, cube, etc.
(print (square 9))               ; 81
(print (cube 3))                 ; 27

; multi-body lambda: implicit begin, last expression is the return value
(def describe (lambda (x)
  (print "  value:"  x)
  (print "  square:" (square x))
  (* x x)))
(describe 5)

; closures capture their environment at definition time
(def make-adder (lambda (n) (lambda (x) (+ x n))))
(def add10  (make-adder 10))
(def add100 (make-adder 100))
(print (add10 5))                ; 15
(print (add100 5))               ; 105

; closure over local state — each call gets its own counter
(def make-counter (lambda ()
  (def count 0)
  (lambda ()
    (def count (+ count 1))
    count)))
(def c1 (make-counter))
(def c2 (make-counter))
(print (c1))   ; 1
(print (c1))   ; 2
(print (c2))   ; 1  — independent state


; ════════════════════════════════════════════════════════════════════════════
; 7. PARTIAL APPLICATION (built-in currying)
; ════════════════════════════════════════════════════════════════════════════
; Calling a lambda with fewer args than params returns a partial application.

(def add (lambda (a b) (+ a b)))
(def inc (add 1))                ; b is still open
(print (inc 41))                 ; 42
(print (inc 99))                 ; 100

; clamp from stdlib — partially apply to fix the bounds
(def clamp01  (clamp 0 1))
(def clampAbs (clamp -1 1))
(print (clamp01  -0.5))          ; 0
(print (clamp01   0.7))          ; 0.7
(print (clamp01   1.3))          ; 1
(print (clampAbs -3.0))          ; -1

; partial application composes naturally with higher-order functions
(def add5 (add 5))
(print (map add5 (iota 5)))      ; (5 6 7 8 9)


; ════════════════════════════════════════════════════════════════════════════
; 8. FUNCTION UTILITIES (from stdlib)
; ════════════════════════════════════════════════════════════════════════════

; identity — useful as a no-op callback
(print (identity 42))            ; 42
(print (map identity '(a b c)))  ; (a b c)

; const — returns a function that always returns x
(def always-zero (const 0))
(print (always-zero 99))         ; 0
(print (map (const 1) (iota 5))) ; (1 1 1 1 1)

; compose — f(g(x))
(def dbl         (lambda (x) (* x 2)))
(def sqrt-of-abs (compose sqrt abs))
(def dbl-then-inc (compose inc dbl))
(print (sqrt-of-abs -16))        ; 4
(print (dbl-then-inc 5))         ; 11  — inc(dbl(5)) = inc(10) = 11

; flip — swap argument order of a binary function
(def rsub (flip -))              ; rsub a b = b - a
(print (rsub 3 10))              ; 7  — (- 10 3)
(print (map (flip /) (list 2 4 5)))  ; map (100 / x) by using partial

; iterate — apply f n times
(print (iterate dbl 6 1))        ; 64  — 2^6
(print (iterate square 3 2))     ; 256 — ((2^2)^2)^2


; ════════════════════════════════════════════════════════════════════════════
; 9. RECURSION
; ════════════════════════════════════════════════════════════════════════════

; tail-recursive factorial — pure0 has TCO, no stack growth
(def fact (lambda (n acc)
  (if (= n 0) acc (fact (- n 1) (* acc n)))))
(print (fact 10 1))              ; 3628800

; fibonacci
(def fib (lambda (n)
  (if (< n 2) n (+ (fib (- n 1)) (fib (- n 2))))))
(print (fib 10))                 ; 55

; recursive list sum using head/tail/null?
(def list-sum (lambda (xs)
  (if (null? xs) 0 (+ (head xs) (list-sum (tail xs))))))
(print (list-sum (list 10 20 30 40)))   ; 100

; deep recursion via tail call (counts to 1000 without stack overflow)
(def count-down (lambda (n)
  (if (= n 0) "done" (count-down (- n 1)))))
(print (count-down 10000))       ; done


; ════════════════════════════════════════════════════════════════════════════
; 10. LIST TRANSFORMS (from stdlib)
; ════════════════════════════════════════════════════════════════════════════

(def digits (iota 8))            ; (0 1 2 3 4 5 6 7)

; map — apply f to each element
(print (map square digits))      ; (0 1 4 9 16 25 36 49)
(print (map str (list 1 2 3)))   ; ([1] [2] [3])

; filter — keep elements where pred returns nonzero
(def even? (lambda (x) (= (- x (* 2 (floor (/ x 2)))) 0)))
(def odd?  (lambda (x) (not (even? x))))
(print (filter even? digits))    ; (0 2 4 6)
(print (filter odd?  digits))    ; (1 3 5 7)

; reduce — fold a list into a single value
(print (reduce + 0 digits))                           ; 28
(print (reduce * 1 (range 1 6)))                      ; 120
(print (reduce (lambda (a b) (if (> a b) a b)) 0 (list 3 1 4 1 5 9)))  ; 9
(print (reduce (lambda (acc x) (cons x acc)) '() (list 1 2 3)))        ; (3 2 1)

; for-each — side effects only, returns 0
(for-each (lambda (x) (print " " x)) (list 10 20 30))

; flat-map — map then flatten one level
(def neighbours (lambda (x) (list (- x 1) x (+ x 1))))
(print (flat-map neighbours (list 10 20)))   ; (9 10 11 19 20 21)


; ════════════════════════════════════════════════════════════════════════════
; 11. LIST PREDICATES (from stdlib)
; ════════════════════════════════════════════════════════════════════════════

(def pos? (lambda (x) (> x 0)))
(def mixed (list -3 -1 0 2 4 5))

(print (any    pos? mixed))      ; 1 — at least one positive
(print (all    pos? mixed))      ; 0 — not all positive
(print (all    pos? (list 1 2))) ; 1

(print (member? 4   mixed))      ; 1
(print (member? 99  mixed))      ; 0

(print (count  pos? mixed))      ; 3 — number of positives


; ════════════════════════════════════════════════════════════════════════════
; 12. LIST RESTRUCTURING (from stdlib)
; ════════════════════════════════════════════════════════════════════════════

(def xs (iota 8))                ; (0 1 2 3 4 5 6 7)

; take and drop
(print (take 3 xs))              ; (0 1 2)
(print (drop 5 xs))              ; (5 6 7)

; take-while and drop-while
(def lt4 (lambda (x) (< x 4)))
(print (take-while lt4 xs))      ; (0 1 2 3)
(print (drop-while lt4 xs))      ; (4 5 6 7)

; reverse-list (separate from vector reverse)
(print (reverse-list '(a b c d)))  ; (d c b a)

; zip and zip-with
(def letters '(a b c))
(def values  (list 10 20 30))
(print (zip letters values))                   ; ((a 10) (b 20) (c 30))
(print (zip-with + (iota 4) (range 10 14)))    ; (10 12 14 16)

; flatten — recursively flattens nested lists
(print (flatten '(1 (2 3) (4 (5 6)))))         ; (1 2 3 4 5 6)
(print (flatten (map (lambda (x) (list x (square x))) (range 1 5))))
                                               ; (1 1 2 4 3 9 4 16)

; partition — split into two lists by predicate
(def halves (partition pos? mixed))
(print (nth halves 0))           ; (2 4 5)  — positives
(print (nth halves 1))           ; (-3 -1 0) — rest


; ════════════════════════════════════════════════════════════════════════════
; 13. LIST SEARCH (from stdlib)
; ════════════════════════════════════════════════════════════════════════════

(def data (list 3 1 4 1 5 9 2 6))

; find — first element matching predicate, or '() if none
(print (find (lambda (x) (> x 4)) data))    ; 5
(print (null? (find (lambda (x) (> x 99)) data)))  ; 1 — not found

; index-of — index of first match, or -1
(print (index-of (lambda (x) (> x 4)) data))   ; 4  (value 5 is at index 4)
(print (index-of (lambda (x) (> x 99)) data))  ; -1

; assoc / assoc-get — association list (list of (key value) pairs)
(def config (list
  (list 'host    "localhost")
  (list 'port    8080)
  (list 'debug   1)))

(print (assoc 'port config))               ; (port 8080)
(print (assoc-get 'host  config ""))       ; localhost
(print (assoc-get 'port  config 0))        ; 8080
(print (assoc-get 'missing config 42))     ; 42 — default


; ════════════════════════════════════════════════════════════════════════════
; 14. MATH (from stdlib + built-ins)
; ════════════════════════════════════════════════════════════════════════════

; norm, normalize, dot
(print (norm (vec 3 4)))             ; 5
(print (norm (vec 1 1 1 1)))         ; 2
(print (normalize (vec 3 4)))        ; [0.6 0.8]
(print (dot (vec 1 2 3) (vec 4 5 6))) ; 32

; cumsum — running total of a vector
(print (cumsum (vec 1 2 3 4 5)))     ; [1 3 6 10 15]

; lerp — linear interpolation between a and b at position t in [0,1]
(print (lerp 0 100 0))               ; 0
(print (lerp 0 100 0.5))             ; 50
(print (lerp 0 100 1))               ; 100
(print (lerp (vec 0 0) (vec 100 200) 0.25))  ; [25 50]  — works on vectors too

; sign
(print (map sign (list -5 0 3)))     ; (-1 0 1)

; clamp with partial application
(def unit (clamp 0 1))
(print (map unit (list -1 0.3 0.7 1.5)))   ; (0 0.3 0.7 1)

; gaussian curve
(def gauss (lambda (x) (exp (- (* x x)))))
(print (gauss (linspace -2 2 5)))    ; bell shape

; function composition for math pipelines
(def normalize-and-abs (compose (lambda (v) (abs v)) normalize))
(print (normalize-and-abs (vec -3 4)))  ; [0.6 0.8]


; ════════════════════════════════════════════════════════════════════════════
; 15. SYSTEM
; ════════════════════════════════════════════════════════════════════════════

(print (time))                           ; unix timestamp
(print (getenv "HOME"))                  ; home directory
(print (match (exec "echo hi") "hi"))    ; 1

; user-defined error signalling
(def safe-sqrt (lambda (x)
  (if (< x 0) (error "sqrt of negative") (sqrt x))))
(print (safe-sqrt 9))            ; 3
; (safe-sqrt -1)                 ; would throw: sqrt of negative


; ════════════════════════════════════════════════════════════════════════════
; 16. FILE I/O AND LOAD
; ════════════════════════════════════════════════════════════════════════════

; write any value, read back as string
(write "/tmp/pure0_ref.txt" (str (linspace 0 1 5)))
(print (read "/tmp/pure0_ref.txt"))      ; [0 0.25 0.5 0.75 1]

; write and load pure0 source
(write "/tmp/pure0_extra.p0"
  "(def tau (* 2 pi)) (def e (exp 1))")
(load "/tmp/pure0_extra.p0")
(print tau)                      ; 6.28318
(print e)                        ; 2.71828


; ════════════════════════════════════════════════════════════════════════════
; 17. ENVIRONMENT INSPECTION
; ════════════════════════════════════════════════════════════════════════════

(def all-names (env))
(print (len all-names))          ; total bound names (builtins + stdlib + locals)
(print (take 5 all-names))       ; first 5 alphabetically
(print (member? 'map all-names))  ; 1 — map is bound (uses equal? internally)
