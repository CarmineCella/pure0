; ============================================================================
; idm_demo.mu
; fast, changing, layered, stereo
; ============================================================================

(def sr 48000)
(def bpm 164)

; ----------------------------------------------------------------------------
; instruments
; ----------------------------------------------------------------------------

(def k1 (kick  sr 0.30 170 45 0.14 0.95))
(def k2 (kick  sr 0.42 120 34 0.20 0.88))

(def s1 (snare sr 0.18 210 0.82 0.09 0.72))
(def s2 (snare sr 0.26 150 0.68 0.14 0.82))

(def h1 (hat   sr 0.035 8500 0.020 0.16))
(def h2 (hat   sr 0.060 7000 0.045 0.13))
(def h3 (hat   sr 0.090 6000 0.070 0.10))

; ----------------------------------------------------------------------------
; drum bars
; each bar = 4 beats = 16 steps
; ----------------------------------------------------------------------------

(def dbar_a
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 4 (vec 1 0 0 0   0 0 1 0   1 0 0 0   0 1 0 0) k1)
      0 (pat sr bpm 4 (vec 0 0 0 0   0 0 0 0   1 0 0 0   0 0 1 0) s1)
      0 (pat sr bpm 4 (euclid 11 16 1) h1)
      0 (pat sr bpm 4 (vec 0 0 0 1   0 0 0 0   0 0 0 1   0 0 0 0) h2))
    0.92))

(def dbar_b
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 4 (vec 1 0 1 0   0 0 1 0   1 0 0 0   1 0 0 0) k1)
      0 (pat sr bpm 4 (vec 0 0 0 0   0 1 0 0   1 0 0 0   0 0 0 0) s1)
      0 (pat sr bpm 4 (euclid 9 16 0) h2)
      0 (pat sr bpm 4 (vec 0 0 0 0   0 0 0 1   0 0 0 0   0 0 0 1) h3))
    0.92))

(def dbar_c
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 4 (vec 1 0 0 0   1 0 0 0   1 0 1 0   0 0 0 0) k2)
      0 (pat sr bpm 4 (vec 0 0 0 0   0 0 0 0   1 0 0 0   1 0 0 0) s2)
      0 (pat sr bpm 4 (euclid 12 16 2) h1)
      0 (pat sr bpm 4 (vec 0 0 1 0   0 0 0 0   0 0 0 1   0 0 1 0) h2))
    0.92))

(def dbar_d
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 4 (vec 1 0 0 1   0 0 1 0   1 0 0 1   0 0 0 0) k1)
      0 (pat sr bpm 4 (vec 0 0 0 0   0 0 1 0   1 0 0 0   0 0 0 0) s2)
      0 (pat sr bpm 4 (euclid 13 16 0) h1)
      0 (pat sr bpm 4 (vec 0 0 0 1   0 0 1 0   0 0 0 0   0 0 0 1) h3))
    0.92))

(def dbar_e
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 4 (vec 1 0 0 0   0 1 0 0   1 0 0 1   0 1 0 0) k2)
      0 (pat sr bpm 4 (vec 0 0 0 0   0 0 0 0   1 0 0 0   0 1 0 0) s1)
      0 (pat sr bpm 4 (euclid 10 16 3) h2)
      0 (pat sr bpm 4 (vec 0 0 0 0   0 0 0 1   0 0 1 0   0 0 0 1) h3))
    0.92))

(def dbar_f
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 4 (vec 1 0 1 0   1 0 0 0   1 0 0 0   0 0 1 0) k1)
      0 (pat sr bpm 4 (vec 0 0 0 0   0 0 0 0   1 0 0 0   0 0 0 0) s1)
      0 (pat sr bpm 4 (euclid 8 16 0) h1)
      0 (pat sr bpm 4 (vec 0 0 0 1   0 0 0 1   0 0 0 1   0 0 0 1) h2))
    0.92))

(def dbar_break
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 4 (vec 1 0 0 0   0 0 0 0   1 0 0 0   0 0 0 1) k1)
      0 (pat sr bpm 4 (euclid 7 16 1) h3))
    0.88))

; ----------------------------------------------------------------------------
; harmonic materials
; ----------------------------------------------------------------------------

(def notes_a (vec 38 41 45 48))
(def notes_b (vec 36 43 46 50))
(def notes_c (vec 33 40 45 47))
(def notes_d (vec 31 38 43 46))
(def notes_e (vec 43 45 50 53))
(def notes_f (vec 40 47 52 55))

(def lead_a (vec 74 77 81 84))
(def lead_b (vec 72 76 79 83 86))
(def lead_c (vec 69 74 76 81))
(def lead_d (vec 71 74 78 81 85))

; ----------------------------------------------------------------------------
; phrase builders
; ----------------------------------------------------------------------------

(def bass_phrase_a
  (bassline sr bpm 8
    (vec 1 0 1 0   1 0 0 0   1 0 1 0   0 0 1 0)
    notes_a
    0.50 0.55 2.2 1))

(def bass_phrase_b
  (bassline sr bpm 8
    (vec 1 0 0 0   1 0 1 0   1 0 0 0   1 0 1 0)
    notes_b
    0.50 0.58 3.4 1))

(def bass_phrase_c
  (bassline sr bpm 8
    (vec 1 0 1 0   0 0 1 0   1 0 1 0   1 0 0 0)
    notes_c
    0.375 0.56 4.6 2))

(def bass_phrase_d
  (bassline sr bpm 8
    (vec 1 0 0 0   1 0 0 0   1 0 1 0   0 1 0 0)
    notes_d
    0.75 0.52 1.8 1))

(def chord_stabs_a
  (patnotes sr bpm 8
    (vec 1 0 0 0   0.7 0 0 0   1 0 0 0   0.5 0 0 0)
    notes_e
    0.75 0.18 1))

(def chord_stabs_b
  (patnotes sr bpm 8
    (vec 1 0 0 0   0 0 1 0   0.6 0 0 0   0 0 1 0)
    notes_f
    0.50 0.16 2))

(def lead_run_a
  (arp sr bpm 8 lead_a 8 0.125 0.12 2 0))

(def lead_run_b
  (arp sr bpm 8 lead_b 6 0.166 0.10 1 2))

(def lead_run_c
  (arp sr bpm 8 lead_c 4 0.250 0.14 0 1))

(def lead_run_d
  (arp sr bpm 8 lead_d 8 0.125 0.11 2 2))

; ----------------------------------------------------------------------------
; drones
; ----------------------------------------------------------------------------

(def drone_a (drone sr (mtof 33) 12 0.10 0.06))
(def drone_b (drone sr (mtof 38) 12 0.08 0.11))
(def drone_c (drone sr (mtof 45) 12 0.07 0.17))
(def drone_d (drone sr (mtof 52) 12 0.06 0.23))

; ----------------------------------------------------------------------------
; 8-beat sections
; ----------------------------------------------------------------------------

(def sec1
  (normalize
    (bmix sr bpm
      0 dbar_a
      4 dbar_b
      0 bass_phrase_a
      0 chord_stabs_a
      0 drone_a)
    0.93))

(def sec2
  (normalize
    (bmix sr bpm
      0 dbar_c
      4 dbar_d
      0 bass_phrase_b
      0 lead_run_a
      0 drone_b)
    0.93))

(def sec3
  (normalize
    (bmix sr bpm
      0 dbar_e
      4 dbar_break
      0 bass_phrase_c
      0 chord_stabs_b
      0 lead_run_b
      0 drone_c)
    0.93))

(def sec4
  (normalize
    (bmix sr bpm
      0 dbar_f
      4 dbar_b
      0 bass_phrase_d
      0 lead_run_c
      0 drone_d)
    0.93))

(def sec5
  (normalize
    (bmix sr bpm
      0 dbar_break
      4 dbar_c
      0 bass_phrase_b
      0 chord_stabs_a
      0 lead_run_d
      0 drone_a)
    0.93))

(def sec6
  (normalize
    (bmix sr bpm
      0 dbar_d
      4 dbar_e
      0 bass_phrase_c
      0 chord_stabs_b
      0 drone_b)
    0.93))

(def sec7
  (normalize
    (bmix sr bpm
      0 dbar_a
      4 dbar_f
      0 bass_phrase_a
      0 lead_run_b
      0 drone_c)
    0.93))

(def sec8
  (normalize
    (bmix sr bpm
      0 dbar_c
      4 dbar_break
      0 bass_phrase_d
      0 lead_run_a
      0 chord_stabs_b
      0 drone_d)
    0.93))

; ----------------------------------------------------------------------------
; assemble larger form
; each section is 8 beats long
; ----------------------------------------------------------------------------

(def body
  (normalize
    (bmix sr bpm
      0  sec1
      8  sec2
      16 sec3
      24 sec4
      32 sec5
      40 sec6
      48 sec7
      56 sec8

      64 sec3
      72 sec1
      80 sec6
      88 sec4
      96 sec7
      104 sec5
      112 sec2
      120 sec8

      128 sec5
      136 sec6
      144 sec1
      152 sec7
      160 sec4
      168 sec2
      176 sec8
      184 sec3)
    0.94))

; ----------------------------------------------------------------------------
; stereo-specific side layers
; ----------------------------------------------------------------------------

(def left_side
  (normalize
    (bmix sr bpm
      16  (arp sr bpm 8 (vec 86 81 77 74) 10 0.10 0.07 2 1)
      48  (patnotes sr bpm 8 (euclid 9 16 1) (vec 62 65 69 72) 0.25 0.08 0)
      80  (drone sr (mtof 57) 10 0.04 0.31)
      112 (arp sr bpm 8 (vec 79 83 86 91) 12 0.08 0.06 0 0)
      152 (pat sr bpm 8 (euclid 23 32 2) h2)
      176 (drone sr (mtof 64) 8 0.03 0.38))
    0.50))

(def right_side
  (normalize
    (bmix sr bpm
      8   (arp sr bpm 8 (vec 74 77 81 84 88) 7 0.14 0.06 1 2)
      40  (patnotes sr bpm 8 (euclid 7 16 0) (vec 67 71 74 79) 0.375 0.09 2)
      72  (drone sr (mtof 60) 10 0.04 0.28)
      104 (arp sr bpm 8 (vec 72 76 79 83) 9 0.11 0.07 2 2)
      144 (pat sr bpm 8 (euclid 19 32 0) h3)
      168 (drone sr (mtof 69) 8 0.03 0.41))
    0.50))

(def left
  (normalize
    (bmix sr bpm
      0 body
      0 left_side)
    0.95))

(def right
  (normalize
    (bmix sr bpm
      0 body
      0 right_side)
    0.95))

(def piece (interleave left right))
(wavwrite piece sr "idm_demo.wav" 2)