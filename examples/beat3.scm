(def sr 48000)
(def bpm 118)

; one-shot events
(def k  (kick  sr 0.40 150 42 0.18 0.95))
(def s  (snare sr 0.22 180 0.78 0.11 0.70))
(def h1 (hat   sr 0.05 7000 0.035 0.18))
(def h2 (hat   sr 0.08 9000 0.060 0.14))

; patterns over one 4/4 bar of 16 steps
(def kpat (vec 1 0 0 0  1 0 0 0  1 0 1 0  0 0 0 0))
(def spat (vec 0 0 0 0  1 0 0 0  0 0 0 0  1 0 0 0))
(def hpat (euclid 11 16 1))
(def ohat (vec 0 0 0 0  0 0 0 1  0 0 0 0  0 0 0 1))

; build bars as actual event patterns
(def drums_l_bar
  (norm
    (bmix sr bpm
      0 (pat sr bpm 4 kpat k)
      0 (pat sr bpm 4 spat s)
      0 (pat sr bpm 4 hpat h1)
      0 (pat sr bpm 4 ohat h2))
    0.9))

; a slightly different right bar
(def drums_r_bar
  (norm
    (bmix sr bpm
      0 (pat sr bpm 4 (euclid 5 16 1) k)
      0 (pat sr bpm 4 spat s)
      0 (pat sr bpm 4 (euclid 9 16 0) h1)
      0 (pat sr bpm 4 ohat h2))
    0.9))

; make a longer form by placing bars one after another
(def drums_l
  (bmix sr bpm
    0  drums_l_bar
    4  drums_l_bar
    8  drums_l_bar
    12 drums_l_bar
    16 drums_l_bar
    20 drums_l_bar
    24 drums_l_bar
    28 drums_l_bar))

(def drums_r
  (bmix sr bpm
    0  drums_r_bar
    4  drums_r_bar
    8  drums_r_bar
    12 drums_r_bar
    16 drums_r_bar
    20 drums_r_bar
    24 drums_r_bar
    28 drums_r_bar))

; drones, slightly different on the two sides
(def drone_l
  (drone sr (mtof 38) 16 0.18 0.05))

(def drone_r
  (drone sr (mtof 45) 16 0.15 0.08))

; second half: move the harmony a bit
(def drone_l2
  (drone sr (mtof 33) 16 0.17 0.04))

(def drone_r2
  (drone sr (mtof 40) 16 0.14 0.07))

(def left
  (norm
    (bmix sr bpm
      0  drums_l
      0  drone_l
      16 drone_l2)
    0.95))

(def right
  (norm
    (bmix sr bpm
      0  drums_r
      0  drone_r
      16 drone_r2)
    0.95))

(def piece (stereo left right))
(wavwrite piece sr "out_stereo.wav" 2)