
; ============================================================================
; sol_rhythmic_patterns_example.scm
; example of rhythmical sample triggering with SOL + samplesynth.h
;
; idea:
; - Vc provides a rhythmic bassline
; - Vn plays repeating upper patterns
; - Fl adds lighter punctuations / answers
; - optional electronic kick / hat for pulse
; ============================================================================

(def sr 48000)
(def bpm 116)

(def SOL (loaddb "/Users/n4/Projects/Media/Datasets/TinySOL"))

; ----------------------------------------------------------------------------
; optional electronic pulse from music.h
; ----------------------------------------------------------------------------

(def k  (kick_sub  sr 0.42 96 30 0.24 0.82))
(def hh (hat_dark  sr 0.05 5200 0.05 0.08))

(def drums
  (norm
    (bmix sr bpm
      0 (pat sr bpm 8
           (vec 1 0 0 0   1 0 0 0   1 0 0 0   1 0 0 0) k)
      0 (pat sr bpm 8
           (euclid 11 16 1) hh))
    0.82))

; ----------------------------------------------------------------------------
; cello rhythmic bassline
; ----------------------------------------------------------------------------

(def vc_bass_a
  (norm
    (sol-patnotes sr bpm 8
      (vec 1 0 0 0   0 0 1 0   1 0 0 0   0 0 1 0)
      SOL "Vc" "ord"
      (vec 36 36 43 41 38 36)
      "mf")
    0.86))

(def vc_bass_b
  (norm
    (sol-patnotes sr bpm 8
      (vec 1 0 1 0   0 0 1 0   1 0 0 0   0 0 0 0)
      SOL "Vc" "ord"
      (vec 36 38 41 43 45 41)
      "mf")
    0.86))

; lower support from contrabass
(def cb_pulse
  (norm
    (sol-patnotes sr bpm 8
      (vec 1 0 0 0   1 0 0 0   0.8 0 0 0   1 0 0 0)
      SOL "Cb" "ord"
      (vec 36 36 31 38)
      "mf")
    0.78))

; ----------------------------------------------------------------------------
; violin upper rhythmic patterns
; ----------------------------------------------------------------------------

(def vn_pattern_a
  (norm
    (sol-patnotes sr bpm 8
      (vec 0 0 1 0   0 0 1 0   0 0 1 0   0 0 1 0)
      SOL "Vn" "ord"
      (vec 72 74 76 79 81 79)
      "mf")
    0.76))

(def vn_pattern_b
  (norm
    (sol-patnotes sr bpm 8
      (vec 0 1 0 0   0 1 0 0   0 0 1 0   0 1 0 0)
      SOL "Vn" "ord"
      (vec 74 76 79 81 83 81)
      "mf")
    0.76))

(def vn_pattern_c
  (norm
    (sol-arp sr bpm 8
      SOL "Vn" "ord"
      (vec 72 76 79 83)
      "mf" 6 2)
    0.72))

; ----------------------------------------------------------------------------
; flute punctuations / replies
; ----------------------------------------------------------------------------

(def fl_pattern_a
  (norm
    (sol-patnotes sr bpm 8
      (vec 0 0 0 1   0 0 0 0   0 0 0 1   0 0 0 0)
      SOL "Fl" "ord"
      (vec 84 88 91 88)
      "mf")
    0.62))

(def fl_pattern_b
  (norm
    (sol-patnotes sr bpm 8
      (vec 0 0 0 0   0 0 1 0   0 0 0 0   0 0 1 0)
      SOL "Fl" "ord"
      (vec 86 88 91 93)
      "mf")
    0.62))

(def fl_pattern_c
  (norm
    (sol-arp sr bpm 8
      SOL "Fl" "ord"
      (vec 84 88 91 96)
      "mf" 8 0)
    0.58))

; ----------------------------------------------------------------------------
; optional clarinet middle layer
; ----------------------------------------------------------------------------

(def cl_mid
  (norm
    (sol-patnotes sr bpm 8
      (vec 1 0 0 0   0 0 0 0   1 0 0 0   0 0 0 0)
      SOL "ClBb" "ord"
      (vec 60 64 67 71)
      "mf")
    0.58))

; ----------------------------------------------------------------------------
; sections
; ----------------------------------------------------------------------------

(def sec1
  (norm
    (bmix sr bpm
      0 cb_pulse
      0 vc_bass_a)
    0.88))

(def sec2
  (norm
    (bmix sr bpm
      0 cb_pulse
      0 vc_bass_a
      0 vn_pattern_a)
    0.90))

(def sec3
  (norm
    (bmix sr bpm
      0 cb_pulse
      0 vc_bass_b
      0 vn_pattern_b
      0 fl_pattern_a)
    0.91))

(def sec4
  (norm
    (bmix sr bpm
      0 cb_pulse
      0 vc_bass_b
      0 vn_pattern_c
      0 fl_pattern_b
      0 cl_mid)
    0.92))

(def sec5
  (norm
    (bmix sr bpm
      0 drums
      0 cb_pulse
      0 vc_bass_a
      0 vn_pattern_a
      0 fl_pattern_c)
    0.93))

(def sec6
  (norm
    (bmix sr bpm
      0 drums
      0 cb_pulse
      0 vc_bass_b
      0 vn_pattern_c
      0 fl_pattern_b
      0 cl_mid)
    0.94))

; ----------------------------------------------------------------------------
; assemble
; ----------------------------------------------------------------------------

(def mono
  (norm
    (bmix sr bpm
      0  sec1
      8  sec2
      16 sec3
      24 sec4
      32 sec5
      40 sec6
      48 sec3
      56 sec5)
    0.95))

(def left
  (norm
    (bmix sr bpm
      0 mono
      24 (sol-patnotes sr bpm 16
           (vec 0 0 1 0   0 0 0 0   0 0 1 0   0 0 0 0)
           SOL "Ob" "ord"
           (vec 79 81 84)
           "mp"))
    0.95))

(def right
  (norm
    (bmix sr bpm
      0 mono
      32 (sol-patnotes sr bpm 16
           (vec 0 0 0 1   0 0 0 0   0 0 0 1   0 0 0 0)
           SOL "Bn" "ord"
           (vec 55 59 62)
           "mp"))
    0.95))

(def piece (stereo left right))
(wavwrite piece sr "sol_rhythmic_patterns_example.wav" 2)
