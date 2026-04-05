;; stdlib.scm for pure0

(def source load)
(def twopi (* 2 pi))

; functional helpers
(def id (lambda (x) x))
(def const (lambda (x y) x))
(def compose (lambda (f g) (lambda (x) (f (g x)))))
(def inc (+ 1))
(def dec (- 1))
(def square (lambda (x) (* x x)))
(def cube (lambda (x) (* x x x)))

; list helpers
(def first (lambda (xs) (nth xs 0)))
(def second (lambda (xs) (nth xs 1)))
(def third (lambda (xs) (nth xs 2)))

; sound helpers
(def osc-hz
  (lambda (t freq)
    (osc (* freq t))))

(def env<
  (lambda (ph)
    (bpf 0 0  1 1 ph)))

(def env>
  (lambda (ph)
    (bpf 0 1  1 0 ph)))

(def env<>
  (lambda (ph)
    (bpf 0 0  0.1 1  0.9 1  1 0 ph)))

(def tone
  (lambda (amp t freq)
    (* amp (osc-hz t freq))))

(def tone<
  (lambda (amp t ph freq)
    (* (env< ph) (tone amp t freq))))

(def tone>
  (lambda (amp t ph freq)
    (* (env> ph) (tone amp t freq))))

(def tone<>
  (lambda (amp t ph freq)
    (* (env<> ph) (tone amp t freq))))

;; eof  
