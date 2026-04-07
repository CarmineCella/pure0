; ============================================================================
; dark, tense, minor-centered, subtle rhythms, intense buildup
; ============================================================================

(def sr 48000)
(def bpm 108)

; ----------------------------------------------------------------------------
; instruments
; ----------------------------------------------------------------------------

(def k_soft (kick  sr 0.30 110 36 0.12 0.55))
(def k_deep (kick  sr 0.48  90 28 0.20 0.78))

(def s_dark (snare sr 0.24 120 0.55 0.18 0.42))
(def s_sharp (snare sr 0.18 180 0.72 0.11 0.55))

(def h_soft (hat   sr 0.05 5000 0.040 0.08))
(def h_air  (hat   sr 0.09 8500 0.070 0.05))
(def h_tick (hat   sr 0.03 9500 0.020 0.07))

; ----------------------------------------------------------------------------
; pitch collections
; mostly minor / suspended / dark colors
; ----------------------------------------------------------------------------

(def bass_a  (vec 33 36 40 41))   ; A minor-ish
(def bass_b  (vec 31 34 38 39))   ; G minor-ish
(def bass_c  (vec 29 33 36 38))   ; F dark cluster
(def bass_d  (vec 28 31 35 36))   ; E tense
(def bass_e  (vec 26 29 33 34))   ; D ominous

(def stab_a  (vec 57 60 64 69))
(def stab_b  (vec 55 58 62 67))
(def stab_c  (vec 53 57 60 65))
(def stab_d  (vec 52 55 59 64))
(def stab_e  (vec 50 53 57 62))

(def arp_a   (vec 69 72 76 81))
(def arp_b   (vec 67 70 74 79))
(def arp_c   (vec 65 69 72 77))
(def arp_d   (vec 64 67 71 76))

; ----------------------------------------------------------------------------
; subtle rhythmic bars
; ----------------------------------------------------------------------------

(def bar_pulse_1
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 4 (vec 1 0 0 0   0 0 0 0   1 0 0 0   0 0 0 0) k_soft)
      0 (pat sr bpm 4 (vec 0 0 0 0   0 0 0 0   1 0 0 0   0 0 0 0) s_dark)
      0 (pat sr bpm 4 (vec 0 0 0.3 0   0 0 0 0   0 0 0.2 0   0 0 0 0) h_soft))
    0.78))

(def bar_pulse_2
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 4 (vec 1 0 0 0   0 0 0 0   0 0 1 0   0 0 0 0) k_soft)
      0 (pat sr bpm 4 (vec 0 0 0 0   0 0 0 0   1 0 0 0   0 0 0.4 0) s_dark)
      0 (pat sr bpm 4 (euclid 5 16 1) h_tick))
    0.80))

(def bar_pulse_3
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 4 (vec 1 0 0 0   0 0 0 0   1 0 0 0   0 1 0 0) k_soft)
      0 (pat sr bpm 4 (vec 0 0 0 0   0 0 0 0   1 0 0 0   0 0 0 0) s_dark)
      0 (pat sr bpm 4 (euclid 6 16 0) h_soft))
    0.82))

(def bar_pulse_4
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 4 (vec 1 0 0 0   0 0 1 0   0 0 0 0   0 0 0 0) k_soft)
      0 (pat sr bpm 4 (vec 0 0 0 0   0 0 0 0   1 0 0 0   0 0 0 0) s_dark)
      0 (pat sr bpm 4 (vec 0 0 0 0   0 0 0.3 0   0 0 0 0   0 0 0.2 0) h_air))
    0.78))

(def bar_drive_1
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 4 (vec 1 0 0 0   0 0 1 0   1 0 0 0   0 0 0 0) k_deep)
      0 (pat sr bpm 4 (vec 0 0 0 0   0 0 0 0   1 0 0 0   0 0 1 0) s_sharp)
      0 (pat sr bpm 4 (euclid 8 16 0) h_tick))
    0.86))

(def bar_drive_2
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 4 (vec 1 0 0 0   1 0 0 0   0 0 1 0   0 0 0 0) k_deep)
      0 (pat sr bpm 4 (vec 0 0 0 0   0 0 0.4 0   1 0 0 0   0 0 0 0) s_sharp)
      0 (pat sr bpm 4 (euclid 9 16 1) h_soft))
    0.88))

(def bar_drive_3
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 4 (vec 1 0 1 0   0 0 0 0   1 0 0 0   0 0 1 0) k_deep)
      0 (pat sr bpm 4 (vec 0 0 0 0   0 0 0 0   1 0 0 0   0 1 0 0) s_dark)
      0 (pat sr bpm 4 (euclid 10 16 2) h_tick))
    0.90))

(def bar_break
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 4 (vec 1 0 0 0   0 0 0 0   0 0 0 0   0 0 0 1) k_soft)
      0 (pat sr bpm 4 (vec 0 0 0.2 0   0 0 0 0   0 0 0.2 0   0 0 0 0) h_air))
    0.70))

; ----------------------------------------------------------------------------
; low-end and harmony
; ----------------------------------------------------------------------------

(def bass_phrase_1
  (bassline sr bpm 8
    (vec 1 0 0 0   1 0 0 0   1 0 0 0   0 0 1 0)
    bass_a
    0.75 0.48 1.4 1))

(def bass_phrase_2
  (bassline sr bpm 8
    (vec 1 0 0 0   0 0 1 0   1 0 0 0   0 0 0 0)
    bass_b
    0.75 0.44 1.8 1))

(def bass_phrase_3
  (bassline sr bpm 8
    (vec 1 0 0 0   1 0 1 0   0 0 1 0   0 0 0 0)
    bass_c
    0.50 0.46 2.2 2))

(def bass_phrase_4
  (bassline sr bpm 8
    (vec 1 0 0 0   0 0 0 0   1 0 0 0   1 0 0 0)
    bass_d
    1.00 0.42 1.2 1))

(def bass_phrase_5
  (bassline sr bpm 8
    (vec 1 0 0 0   0 1 0 0   1 0 0 0   0 0 1 0)
    bass_e
    0.50 0.50 2.8 1))

(def stabs_1
  (patnotes sr bpm 8
    (vec 1 0 0 0   0 0 0 0   0.7 0 0 0   0 0 0 0)
    stab_a
    1.5 0.12 2))

(def stabs_2
  (patnotes sr bpm 8
    (vec 1 0 0 0   0 0 1 0   0 0 0 0   0.6 0 0 0)
    stab_b
    1.0 0.10 1))

(def stabs_3
  (patnotes sr bpm 8
    (vec 0.8 0 0 0   0 0 0 0   1 0 0 0   0 0 1 0)
    stab_c
    0.75 0.11 2))

(def stabs_4
  (patnotes sr bpm 8
    (vec 1 0 0 0   0 0 0 0   0.5 0 0 0   0 0 0 0)
    stab_d
    2.0 0.10 1))

(def stabs_5
  (patnotes sr bpm 8
    (vec 1 0 0 0   0.5 0 0 0   0 0 1 0   0 0 0 0)
    stab_e
    1.0 0.10 2))

(def arp_line_1
  (arp sr bpm 8 arp_a 8 0.125 0.05 2 2))

(def arp_line_2
  (arp sr bpm 8 arp_b 6 0.166 0.05 0 0))

(def arp_line_3
  (arp sr bpm 8 arp_c 8 0.125 0.04 2 1))

(def arp_line_4
  (arp sr bpm 8 arp_d 12 0.083 0.04 1 2))

; ----------------------------------------------------------------------------
; drones
; ----------------------------------------------------------------------------

(def drone_a1 (drone sr (mtof 33) 12 0.12 0.04))
(def drone_a2 (drone sr (mtof 40) 12 0.08 0.07))

(def drone_b1 (drone sr (mtof 31) 12 0.13 0.06))
(def drone_b2 (drone sr (mtof 38) 12 0.07 0.10))

(def drone_c1 (drone sr (mtof 29) 12 0.14 0.09))
(def drone_c2 (drone sr (mtof 36) 12 0.08 0.13))

(def drone_d1 (drone sr (mtof 28) 12 0.15 0.12))
(def drone_d2 (drone sr (mtof 35) 12 0.09 0.16))

(def drone_e1 (drone sr (mtof 26) 12 0.16 0.18))
(def drone_e2 (drone sr (mtof 33) 12 0.10 0.22))

; ----------------------------------------------------------------------------
; sections, each 8 beats
; ----------------------------------------------------------------------------

(def sec_intro_1
  (normalize
    (bmix sr bpm
      0 bar_break
      4 bar_pulse_1
      0 bass_phrase_1
      0 stabs_4
      0 drone_a1)
    0.90))

(def sec_intro_2
  (normalize
    (bmix sr bpm
      0 bar_pulse_2
      4 bar_pulse_3
      0 bass_phrase_2
      0 stabs_1
      0 drone_a2)
    0.90))

(def sec_mid_1
  (normalize
    (bmix sr bpm
      0 bar_pulse_4
      4 bar_drive_1
      0 bass_phrase_3
      0 stabs_2
      0 arp_line_1
      0 drone_b1)
    0.92))

(def sec_mid_2
  (normalize
    (bmix sr bpm
      0 bar_drive_2
      4 bar_pulse_3
      0 bass_phrase_4
      0 stabs_3
      0 drone_b2)
    0.92))

(def sec_rise_1
  (normalize
    (bmix sr bpm
      0 bar_drive_1
      4 bar_drive_2
      0 bass_phrase_3
      0 stabs_2
      0 arp_line_2
      0 drone_c1)
    0.93))

(def sec_rise_2
  (normalize
    (bmix sr bpm
      0 bar_drive_3
      4 bar_drive_1
      0 bass_phrase_5
      0 stabs_5
      0 arp_line_3
      0 drone_c2)
    0.94))

(def sec_drop_1
  (normalize
    (bmix sr bpm
      0 bar_drive_3
      4 bar_drive_2
      0 bass_phrase_5
      0 stabs_1
      0 arp_line_4
      0 drone_d1)
    0.95))

(def sec_drop_2
  (normalize
    (bmix sr bpm
      0 bar_drive_2
      4 bar_break
      0 bass_phrase_4
      0 stabs_3
      0 drone_d2)
    0.94))

(def sec_dark_1
  (normalize
    (bmix sr bpm
      0 bar_pulse_1
      4 bar_break
      0 bass_phrase_2
      0 stabs_4
      0 arp_line_2
      0 drone_e1)
    0.90))

(def sec_dark_2
  (normalize
    (bmix sr bpm
      0 bar_pulse_4
      4 bar_pulse_2
      0 bass_phrase_1
      0 stabs_5
      0 drone_e2)
    0.91))

; ----------------------------------------------------------------------------
; large form
; 30 sections × 8 beats = 240 beats
; at 108 bpm ≈ 2 min 13 s
; duplicate / extend below if you want even longer
; ----------------------------------------------------------------------------

(def body
  (normalize
    (bmix sr bpm
      0   sec_intro_1
      8   sec_intro_2
      16  sec_dark_1
      24  sec_mid_1
      32  sec_mid_2
      40  sec_rise_1
      48  sec_dark_2
      56  sec_rise_2
      64  sec_drop_1
      72  sec_drop_2

      80  sec_intro_2
      88  sec_dark_1
      96  sec_mid_1
      104 sec_rise_1
      112 sec_rise_2
      120 sec_drop_1
      128 sec_dark_2
      136 sec_mid_2
      144 sec_drop_2
      152 sec_drop_1

      160 sec_dark_1
      168 sec_intro_1
      176 sec_mid_2
      184 sec_rise_2
      192 sec_drop_1
      200 sec_drop_2
      208 sec_dark_2
      216 sec_mid_1
      224 sec_rise_1
      232 sec_drop_1)
    0.95))

; ----------------------------------------------------------------------------
; stereo side layers for width and unease
; ----------------------------------------------------------------------------

(def left_side
  (normalize
    (bmix sr bpm
      24  (drone sr (mtof 57) 10 0.03 0.24)
      72  (arp sr bpm 8 (vec 81 76 72 69) 10 0.10 0.03 2 1)
      128 (patnotes sr bpm 8 (euclid 7 16 1) (vec 64 67 71 74) 0.50 0.05 0)
      184 (drone sr (mtof 52) 10 0.04 0.29))
    0.40))

(def right_side
  (normalize
    (bmix sr bpm
      40  (drone sr (mtof 60) 10 0.03 0.21)
      88  (arp sr bpm 8 (vec 79 74 71 67) 12 0.08 0.03 1 2)
      152 (pat sr bpm 8 (euclid 15 32 0) h_air)
      208 (drone sr (mtof 55) 10 0.04 0.27))
    0.40))

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
(wavwrite piece sr "dark_tension.wav" 2)
