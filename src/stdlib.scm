; stdlib.scm — pure0 standard library
; load with: (load "stdlib.scm")

; ── type predicates ───────────────────────────────────────────────────────────

(def list?   (lambda (x) (= (match (str (type x)) "^list$")   1)))
(def vec?    (lambda (x) (= (match (str (type x)) "^vec$")    1)))
(def string? (lambda (x) (= (match (str (type x)) "^string$") 1)))
(def lambda? (lambda (x) (= (match (str (type x)) "^lambda$") 1)))
(def proc?   (lambda (x) (= (match (str (type x)) "^proc$")   1)))
(def fn?     (lambda (x) (if (lambda? x) 1 (proc? x))))

; ── function utilities ────────────────────────────────────────────────────────

(def identity (lambda (x) x))
(def const    (lambda (x) (lambda (_) x)))
(def compose  (lambda (f g) (lambda (x) (f (g x)))))
(def flip     (lambda (f)   (lambda (a b) (f b a))))
(def iterate  (lambda (f n x)
  (if (= n 0) x (iterate f (- n 1) (f x)))))

; ── list construction ─────────────────────────────────────────────────────────

(def iota (lambda (n)
  (def go (lambda (i acc)
    (if (= i 0) acc (go (- i 1) (cons (- i 1) acc)))))
  (go n '())))

(def range (lambda (start end)
  (def go (lambda (i acc)
    (if (< i start) acc (go (- i 1) (cons i acc)))))
  (go (- end 1) '())))

(def replicate (lambda (n x)
  (def go (lambda (i acc)
    (if (= i 0) acc (go (- i 1) (cons x acc)))))
  (go n '())))

; ── list access ───────────────────────────────────────────────────────────────

(def second (lambda (xs) (head (tail xs))))
(def third  (lambda (xs) (head (tail (tail xs)))))

(def last (lambda (xs)
  (if (null? (tail xs)) (head xs) (last (tail xs)))))

(def init (lambda (xs)
  (if (null? (tail xs)) '()
                        (cons (head xs) (init (tail xs))))))

; ── list transformation ───────────────────────────────────────────────────────

(def map (lambda (f xs)
  (if (null? xs) '()
                 (cons (f (head xs)) (map f (tail xs))))))

(def filter (lambda (pred xs)
  (if (null? xs) '()
    (if (pred (head xs))
      (cons (head xs) (filter pred (tail xs)))
      (filter pred (tail xs))))))

(def reduce (lambda (f init xs)
  (if (null? xs) init
                 (reduce f (f init (head xs)) (tail xs)))))

(def for-each (lambda (f xs)
  (if (null? xs) 0
    (begin (f (head xs)) (for-each f (tail xs))))))

(def flat-map (lambda (f xs)
  (reduce append '() (map f xs))))

; ── list predicates ───────────────────────────────────────────────────────────

(def any (lambda (pred xs)
  (if (null? xs) 0
    (if (pred (head xs)) 1 (any pred (tail xs))))))

(def all (lambda (pred xs)
  (if (null? xs) 1
    (if (pred (head xs)) (all pred (tail xs)) 0))))

(def equal? (lambda (a b) (= a b)))

(def member? (lambda (x xs)
  (any (lambda (y) (equal? x y)) xs)))

(def count (lambda (pred xs)
  (reduce (lambda (acc x) (if (pred x) (+ acc 1) acc)) 0 xs)))

; ── list restructuring ────────────────────────────────────────────────────────

(def reverse-list (lambda (xs) (reverse xs)))

(def list-take (lambda (n xs)
  (if (= n 0) '()
    (if (null? xs) '()
                   (cons (head xs) (list-take (- n 1) (tail xs)))))))

(def list-drop (lambda (n xs)
  (if (= n 0) xs
    (if (null? xs) '() (list-drop (- n 1) (tail xs))))))

(def take-while (lambda (pred xs)
  (if (null? xs) '()
    (if (pred (head xs))
      (cons (head xs) (take-while pred (tail xs)))
      '()))))

(def drop-while (lambda (pred xs)
  (if (null? xs) '()
    (if (pred (head xs)) (drop-while pred (tail xs)) xs))))

(def zip (lambda (xs ys)
  (if (null? xs) '()
    (if (null? ys) '()
      (cons (list (head xs) (head ys))
            (zip (tail xs) (tail ys)))))))

(def zip-with (lambda (f xs ys)
  (if (null? xs) '()
    (if (null? ys) '()
      (cons (f (head xs) (head ys))
            (zip-with f (tail xs) (tail ys)))))))

(def flatten (lambda (xs)
  (if (null? xs) '()
    (if (list? (head xs))
      (append (flatten (head xs)) (flatten (tail xs)))
      (cons (head xs) (flatten (tail xs)))))))

(def partition (lambda (pred xs)
  (list (filter pred xs)
        (filter (lambda (x) (not (pred x))) xs))))

; ── list search ───────────────────────────────────────────────────────────────

(def find (lambda (pred xs)
  (if (null? xs) '()
    (if (pred (head xs)) (head xs) (find pred (tail xs))))))

(def index-of (lambda (pred xs)
  (def go (lambda (xs i)
    (if (null? xs) -1
      (if (pred (head xs)) i (go (tail xs) (+ i 1))))))
  (go xs 0)))

; ── association lists  (list of (key value) pairs) ───────────────────────────

(def assoc (lambda (key alist)
  (find (lambda (pair) (= (head pair) key)) alist)))

(def assoc-get (lambda (key alist default)
  (def pair (assoc key alist))
  (if (null? pair) default (second pair))))

; ── math ─────────────────────────────────────────────────────────────────────

(def square    (lambda (x) (* x x)))
(def cube      (lambda (x) (* x x x)))
(def clamp     (lambda (lo hi x) (if (< x lo) lo (if (> x hi) hi x))))
(def lerp      (lambda (a b t) (+ a (* (- b a) t))))
(def norm      (lambda (v) (sqrt (sum (* v v)))))
(def vnormalize (lambda (v) (/ v (norm v))))
(def dot       (lambda (a b) (sum (* a b))))
(def sign      (lambda (x) (if (< x 0) -1 (if (> x 0) 1 0))))

(def cumsum (lambda (v)
  (def n (len v))
  (def go (lambda (i acc result)
    (if (= i n) result
      (begin
        (def acc (+ acc (nth v i)))
        (go (+ i 1) acc (append result (vec acc)))))))
  (go 0 0 (vec))))

; ── string utilities ──────────────────────────────────────────────────────────

(def starts-with? (lambda (s prefix)
  (= (match s (join (list "^" prefix) "")) 1)))

(def ends-with? (lambda (s suffix)
  (= (match s (join (list suffix "$") "")) 1)))

; ── i/o helpers ───────────────────────────────────────────────────────────────

(def print-list (lambda (xs) (for-each print xs)))
