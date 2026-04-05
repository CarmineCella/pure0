(def sr 48000)
(def bpm 120)

(def kpat (euclid 4 16))
(def spat (vec 0 0 0 0 1 0 0 0 0 0 0 0 1 0 0 0))
(def hpat (euclid 11 16 1))

(def ktrig (steps sr bpm 4 kpat))
(def strig (steps sr bpm 4 spat))
(def htrig (steps sr bpm 4 hpat))

(def k (kick sr 0.4))
(def s (snare sr 0.22))
(def h (hat sr 0.08))

(def bar
  (mix
    0 (gate (loop k (len ktrig)) ktrig)
    0 (gate (loop s (len strig)) strig)
    0 (gate (loop h (len htrig)) htrig)))

(def d (drone sr (mtof 45) 8 0.25 0.07))

(def piece (norm (bmix sr bpm 0 bar 0 d) 0.95))
(wavwrite piece sr "out.wav")