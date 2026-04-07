; inst-gran_example.scm — processo graduale con bpf

(load "stdlib.scm")
(load "sampsynth.scm")

(def sr 44100)
(def dur 40.0)

(def full-db (db-load "/Users/n4/Projects/Media/Datasets/StaticSOL"))
(def db (db-query full-db "(Fl|Ob|ClBb|Bn|Vn|Va|Vc|Cb|Hn|Tbn|TpC|BTb).*ord"))

; ------------------------------------------------------------
; orchestra (gruppi sincronizzati)
; ------------------------------------------------------------

(def orchestra
  (list
    (list "Ob" "Fl")
    (list "ClBb" "Bn")
    (list "Vn" "Va")
    (list "Vc" "Cb")
    (list "Hn" "Tbn")
    (list "TpC" "BTb")))

; ------------------------------------------------------------
; fase continua in secondi per costruire le curve con bpf
; ------------------------------------------------------------

(def phase (linspace 0 dur 1024))

; ------------------------------------------------------------
; curve continue
;
; inizio:
;   ottava 3, ppp, do minore + fa# minore
;   note ~1.0 s
;   densità ~10 note/sec
;
; fino a 30 s:
;   trasformazione graduale verso
;   ottava 5, fff, si minore + la# aggiunto
;   note ~0.1 s
;   densità ~20 note/sec
; ------------------------------------------------------------

(def density
  (bpf
    0    10
    8    11
    16   13
    24   16
    30   20
    40   20
    phase))

(def rand-density
  (bpf
    0    0.08
    10   0.10
    20   0.15
    30   0.18
    40   0.18
    phase))

(def length
  (bpf
    0    1.0
    8    0.8
    16   0.5
    24   0.25
    30   0.1
    40   0.1
    phase))

(def rand-length
  (bpf
    0    0.05
    10   0.08
    20   0.12
    30   0.18
    40   0.18
    phase))

(def rand-octave
  (bpf
    0    0
    12   0
    20   0.3
    30   0
    40   0
    phase))

; ------------------------------------------------------------
; schedule discreti
; ------------------------------------------------------------

; ottava: da 3 a 5 in fasi graduali
(def octave-range
  (list
    (vec 3 3)
    (vec 3 3)
    (vec 3 4)
    (vec 4 4)
    (vec 5 5)
    (vec 5 5)))

; armonie espresse come classi di altezza:
; do minore + fa# minore  = C Eb G + F# A C#
; si minore + la# aggiunto = B D F# + A#
(def chords
  (list
    (vec 0 3 7 6 9 1)       ; C min + F# min
    (vec 0 3 6 7 9 1)       ; leggero addensamento / fusione
    (vec 11 2 6 9 10)       ; zona intermedia verso B
    (vec 11 2 6 10)         ; B min + A#
    (vec 11 2 6 10)         ; stabilizzazione
    (vec 11 2 6 10)))       ; finale

; stile: ordinario sempre
(def styles
  (list
    (list "ord")
    (list "ord")
    (list "ord")
    (list "ord")
    (list "ord")
    (list "ord")))

; dinamica graduale da ppp a fff
(def dynamics
  (list
    (list "ppp")
    (list "pp")
    (list "mp")
    (list "mf" "f")
    (list "fff")
    (list "fff")))

; ------------------------------------------------------------
; granulazione
; ------------------------------------------------------------

(def result
  (inst-gran sr dur orchestra
             density rand-density
             length rand-length
             octave-range rand-octave
             chords styles dynamics
             db))

(def render (inst-gran-render result))
(def score  (inst-gran-score result))

(print "render length:")
(print (len render))
(print "")

(print "events:")
(print (len score))
(print "")

(wavwrite render sr "inst_gran.wav")
(write "score.scm" score)