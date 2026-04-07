(def sr 48000)
(def bpm 120)

; one-shot drum sounds
(def k  (kick  sr 0.42 150 42 0.18 0.95))
(def s  (snare sr 0.20 190 0.78 0.11 0.75))
(def h1 (hat   sr 0.05 7000 0.030 0.22))
(def h2 (hat   sr 0.08 9000 0.050 0.16))

; a warmer drone underneath
(def d1 (drone sr (mtof 33) 16 0.16 0.05))
(def d2 (drone sr (mtof 40) 16 0.12 0.08))

; ---- bar 1 ----
(def bar1
  (bmix sr bpm
    0.0 k
    0.5 h1
    1.0 h1
    1.5 h1
    2.0 s
    2.5 h1
    3.0 h1
    3.5 h2))

; ---- bar 2 ----
(def bar2
  (bmix sr bpm
    0.0 k
    0.5 h1
    1.0 h1
    1.5 h2
    2.0 s
    2.5 h1
    3.0 k
    3.5 h2))

; ---- bar 3 ----
(def bar3
  (bmix sr bpm
    0.0 k
    0.5 h1
    1.0 h2
    1.5 h1
    2.0 s
    2.5 h1
    3.0 h2
    3.5 h1))

; ---- bar 4 ----
(def bar4
  (bmix sr bpm
    0.0 k
    0.5 h2
    1.0 h1
    1.5 h1
    2.0 s
    2.5 h2
    3.0 k
    3.5 h2))

; second phrase with a little more activity
(def bar5
  (bmix sr bpm
    0.0 k
    0.5 h1
    0.75 h1
    1.0 h2
    1.5 h1
    2.0 s
    2.5 h1
    3.0 h2
    3.5 h2))

(def bar6
  (bmix sr bpm
    0.0 k
    0.5 h2
    1.0 h1
    1.5 h2
    2.0 s
    2.5 h1
    3.0 k
    3.25 h1
    3.5 h2))

(def bar7
  (bmix sr bpm
    0.0 k
    0.5 h1
    1.0 h2
    1.5 h1
    2.0 s
    2.5 h2
    3.0 h1
    3.5 h2))

(def bar8
  (bmix sr bpm
    0.0 k
    0.5 h2
    1.0 h2
    1.5 h1
    2.0 s
    2.5 h2
    3.0 k
    3.5 h2))

; place bars one after another
(def drums
  (bmix sr bpm
    0  bar1
    4  bar2
    8  bar3
    12 bar4
    16 bar5
    20 bar6
    24 bar7
    28 bar8))

; long texture under the drums
(def bed
  (bmix sr bpm
    0  d1
    16 d2))

(def piece
  (normalize
    (bmix sr bpm
      0 drums
      0 bed)
    0.95))

(wavwrite piece sr "beat2.wav")