(def sr 48000)
(def bpm 140)

(def drums
  (bmix sr bpm
    0 (pat sr bpm 8 (vec 1 0 0 0  0 0 1 0  1 0 0 0  0 1 0 0) (kick sr 0.4))
    0 (pat sr bpm 8 (vec 0 0 0 0  1 0 0 0  0 0 0 0  1 0 0 0) (snare sr 0.22))
    0 (pat sr bpm 8 (euclid 11 16 1) (hat sr 0.06))))

(def bass
  (bassline sr bpm 8
    (vec 1 0 1 0  1 0 0 0  1 0 1 0  0 0 1 0)
    (vec 38 41 36 34)
    0.5 0.55 2.8 1))

(def lead
  (arp sr bpm 8
    (vec 62 65 69 72)
    4 0.18 0.18 2 2))

(def stab
  (patnotes sr bpm 8
    (vec 1 0 0 0  0.7 0 0 0  1 0 0 0  0.7 0 0 0)
    (vec 50 57 53 60)
    0.75 0.22 1))

(def left  (normalize (bmix sr bpm 0 drums 0 bass 0 stab 0 lead) 0.95))
(def right (normalize (bmix sr bpm 0 drums 0 bass 0 (drone sr (mtof 45) 8 0.08 0.18) 0 lead) 0.95))

(wavwrite (interleave left right) sr "test_music_new.wav" 2)