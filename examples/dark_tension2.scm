; ============================================================================
; drone -> hats -> bassline -> kick -> melodies -> varied continuation
; all harmonic materials are minor-centered
; ============================================================================

(def sr 48000)
(def bpm 124)

; ----------------------------------------------------------------------------
; instruments
; ----------------------------------------------------------------------------

(def k_soft (kick  sr 0.34 110 34 0.14 0.52))
(def k_deep (kick  sr 0.50  88 28 0.22 0.78))

(def h_tick (hat sr 0.028 9000 0.018 0.06))
(def h_soft (hat sr 0.055 6000 0.040 0.07))
(def h_air  (hat sr 0.090 8500 0.070 0.05))

; ----------------------------------------------------------------------------
; minor pitch materials only
; all collections are minor triads / minor 7 / minor color tones
; ----------------------------------------------------------------------------

(def bass_a (vec 33 36 40 43))   ; A C E G
(def bass_b (vec 31 34 38 41))   ; G Bb D F
(def bass_c (vec 29 33 36 40))   ; F A C E
(def bass_d (vec 28 31 35 38))   ; E G B D
(def bass_e (vec 26 29 33 36))   ; D F A C

(def stab_a (vec 57 60 64 67))   ; A C E G
(def stab_b (vec 55 58 62 65))   ; G Bb D F
(def stab_c (vec 53 57 60 64))   ; F A C E
(def stab_d (vec 52 55 59 62))   ; E G B D
(def stab_e (vec 50 53 57 60))   ; D F A C

(def arp_a (vec 69 72 76 79))    ; A C E G
(def arp_b (vec 67 70 74 77))    ; G Bb D F
(def arp_c (vec 65 69 72 76))    ; F A C E
(def arp_d (vec 64 67 71 74))    ; E G B D
(def arp_e (vec 62 65 69 72))    ; D F A C

; ----------------------------------------------------------------------------
; long drones
; ----------------------------------------------------------------------------

(def drone_a1 (drone sr (mtof 33) 16 0.14 0.03))
(def drone_a2 (drone sr (mtof 40) 16 0.08 0.06))

(def drone_b1 (drone sr (mtof 31) 16 0.15 0.05))
(def drone_b2 (drone sr (mtof 38) 16 0.09 0.08))

(def drone_c1 (drone sr (mtof 29) 16 0.16 0.07))
(def drone_c2 (drone sr (mtof 36) 16 0.10 0.11))

(def drone_d1 (drone sr (mtof 28) 16 0.17 0.10))
(def drone_d2 (drone sr (mtof 35) 16 0.10 0.14))

(def drone_e1 (drone sr (mtof 26) 16 0.18 0.15))
(def drone_e2 (drone sr (mtof 33) 16 0.11 0.19))

; ----------------------------------------------------------------------------
; hats / pulses
; ----------------------------------------------------------------------------

(def hats_1
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 8 (euclid 7 16 1) h_tick)
      0 (pat sr bpm 8 (vec 0 0 0.2 0   0 0 0 0   0 0 0.3 0   0 0 0 0) h_soft))
    0.55))

(def hats_2
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 8 (euclid 8 16 0) h_tick)
      0 (pat sr bpm 8 (vec 0 0 0 1   0 0 0 0   0 0 0 1   0 0 0 0) h_air))
    0.58))

(def hats_3
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 8 (euclid 9 16 2) h_soft)
      0 (pat sr bpm 8 (vec 0 0 0.2 0   0 0 0 0   0 0 0.2 0   0 0 0 1) h_tick))
    0.60))

(def hats_4
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 8 (euclid 11 16 0) h_tick)
      0 (pat sr bpm 8 (vec 0 0 0 0   0 0 0 1   0 0 0 0   0 0 0 1) h_air))
    0.62))

; ----------------------------------------------------------------------------
; basslines
; ----------------------------------------------------------------------------

(def bass_1
  (bassline sr bpm 8
    (vec 1 0 0 0   1 0 0 0   1 0 0 0   0 0 1 0)
    bass_a
    0.75 0.42 1.3 1))

(def bass_2
  (bassline sr bpm 8
    (vec 1 0 0 0   0 0 1 0   1 0 0 0   0 0 0 0)
    bass_b
    0.75 0.44 1.9 1))

(def bass_3
  (bassline sr bpm 8
    (vec 1 0 0 0   1 0 1 0   0 0 1 0   0 0 0 0)
    bass_c
    0.50 0.46 2.6 2))

(def bass_4
  (bassline sr bpm 8
    (vec 1 0 0 0   0 0 0 0   1 0 0 0   1 0 0 0)
    bass_d
    1.00 0.40 1.2 1))

(def bass_5
  (bassline sr bpm 8
    (vec 1 0 0 0   0 1 0 0   1 0 0 0   0 0 1 0)
    bass_e
    0.50 0.48 2.2 1))

; ----------------------------------------------------------------------------
; kick layers
; ----------------------------------------------------------------------------

(def kicks_1
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 8 (vec 1 0 0 0   0 0 0 0   1 0 0 0   0 0 0 0) k_soft))
    0.68))

(def kicks_2
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 8 (vec 1 0 0 0   0 0 1 0   1 0 0 0   0 0 0 0) k_soft))
    0.74))

(def kicks_3
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 8 (vec 1 0 0 0   1 0 0 0   0 0 1 0   0 0 0 0) k_deep))
    0.80))

(def kicks_4
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 8 (vec 1 0 1 0   0 0 0 0   1 0 0 0   0 0 1 0) k_deep))
    0.84))

; ----------------------------------------------------------------------------
; minor stabs and minor arps only
; ----------------------------------------------------------------------------

(def stabs_1
  (patnotes sr bpm 8
    (vec 1 0 0 0   0 0 0 0   0.6 0 0 0   0 0 0 0)
    stab_a
    1.5 0.10 2))

(def stabs_2
  (patnotes sr bpm 8
    (vec 1 0 0 0   0 0 1 0   0 0 0 0   0.5 0 0 0)
    stab_b
    1.0 0.09 1))

(def stabs_3
  (patnotes sr bpm 8
    (vec 0.8 0 0 0   0 0 0 0   1 0 0 0   0 0 1 0)
    stab_c
    0.75 0.09 2))

(def stabs_4
  (patnotes sr bpm 8
    (vec 1 0 0 0   0 0 0 0   0.5 0 0 0   0 0 0 0)
    stab_d
    2.0 0.08 1))

(def stabs_5
  (patnotes sr bpm 8
    (vec 1 0 0 0   0.5 0 0 0   0 0 1 0   0 0 0 0)
    stab_e
    1.0 0.08 2))

(def arp_1 (arp sr bpm 8 arp_a 8  0.125 0.045 2 2))
(def arp_2 (arp sr bpm 8 arp_b 6  0.166 0.045 0 0))
(def arp_3 (arp sr bpm 8 arp_c 8  0.125 0.040 2 1))
(def arp_4 (arp sr bpm 8 arp_d 12 0.083 0.038 1 2))
(def arp_5 (arp sr bpm 8 arp_e 8  0.125 0.042 0 2))

; ----------------------------------------------------------------------------
; sections, each 8 beats
; ----------------------------------------------------------------------------

; 1. drone only
(def sec_01
  (normalize
    (bmix sr bpm
      0 drone_a1
      0 drone_a2)
    0.86))

; 2. add hats
(def sec_02
  (normalize
    (bmix sr bpm
      0 drone_a1
      0 drone_a2
      0 hats_1)
    0.88))

; 3. hats vary
(def sec_03
  (normalize
    (bmix sr bpm
      0 drone_b1
      0 drone_b2
      0 hats_2)
    0.88))

; 4. add bassline
(def sec_04
  (normalize
    (bmix sr bpm
      0 drone_b1
      0 drone_b2
      0 hats_2
      0 bass_1)
    0.90))

(def sec_05
  (normalize
    (bmix sr bpm
      0 drone_c1
      0 drone_c2
      0 hats_3
      0 bass_2)
    0.90))

; 5. add kick
(def sec_06
  (normalize
    (bmix sr bpm
      0 drone_c1
      0 drone_c2
      0 hats_3
      0 bass_2
      0 kicks_1)
    0.92))

(def sec_07
  (normalize
    (bmix sr bpm
      0 drone_d1
      0 drone_d2
      0 hats_4
      0 bass_3
      0 kicks_2)
    0.93))

; 6. add melody / arp
(def sec_08
  (normalize
    (bmix sr bpm
      0 drone_d1
      0 drone_d2
      0 hats_4
      0 bass_3
      0 kicks_2
      0 stabs_1
      0 arp_1)
    0.94))

(def sec_09
  (normalize
    (bmix sr bpm
      0 drone_e1
      0 drone_e2
      0 hats_3
      0 bass_4
      0 kicks_3
      0 stabs_2
      0 arp_2)
    0.95))

(def sec_10
  (normalize
    (bmix sr bpm
      0 drone_e1
      0 drone_e2
      0 hats_2
      0 bass_5
      0 kicks_4
      0 stabs_3
      0 arp_3)
    0.95))

; further variations
(def sec_11
  (normalize
    (bmix sr bpm
      0 drone_d1
      0 hats_1
      0 bass_3
      0 kicks_3
      0 stabs_4
      0 arp_4)
    0.94))

(def sec_12
  (normalize
    (bmix sr bpm
      0 drone_c1
      0 hats_4
      0 bass_2
      0 kicks_2
      0 stabs_5
      0 arp_5)
    0.94))

(def sec_13
  (normalize
    (bmix sr bpm
      0 drone_b1
      0 hats_2
      0 bass_1
      0 kicks_1
      0 stabs_2
      0 arp_3)
    0.93))

(def sec_14
  (normalize
    (bmix sr bpm
      0 drone_d2
      0 hats_3
      0 bass_4
      0 kicks_4
      0 stabs_1
      0 arp_2)
    0.95))

(def sec_15
  (normalize
    (bmix sr bpm
      0 drone_e2
      0 hats_4
      0 bass_5
      0 kicks_3
      0 stabs_3
      0 arp_4)
    0.95))

(def sec_16
  (normalize
    (bmix sr bpm
      0 drone_a1
      0 hats_1
      0 bass_2
      0 kicks_2
      0 stabs_5
      0 arp_1)
    0.93))

; ----------------------------------------------------------------------------
; long form
; ----------------------------------------------------------------------------

(def body
  (normalize
    (bmix sr bpm
      0   sec_01
      8   sec_02
      16  sec_03
      24  sec_04
      32  sec_05
      40  sec_06
      48  sec_07
      56  sec_08
      64  sec_09
      72  sec_10
      80  sec_11
      88  sec_12
      96  sec_13
      104 sec_14
      112 sec_15
      120 sec_16

      128 sec_05
      136 sec_06
      144 sec_08
      152 sec_09
      160 sec_10
      168 sec_12
      176 sec_14
      184 sec_15
      192 sec_11
      200 sec_13
      208 sec_16
      216 sec_09
      224 sec_10
      232 sec_15)
    0.95))

; ----------------------------------------------------------------------------
; stereo unease
; ----------------------------------------------------------------------------

(def left_side
  (normalize
    (bmix sr bpm
      32  (drone sr (mtof 57) 10 0.03 0.20)
      88  (arp sr bpm 8 (vec 81 76 72 69) 10 0.09 0.028 2 1)
      160 (patnotes sr bpm 8 (euclid 7 16 1) (vec 64 67 71 74) 0.50 0.04 0))
    0.34))

(def right_side
  (normalize
    (bmix sr bpm
      48  (drone sr (mtof 60) 10 0.03 0.17)
      104 (arp sr bpm 8 (vec 79 74 71 67) 12 0.08 0.028 1 2)
      184 (pat sr bpm 8 (euclid 15 32 0) h_air))
    0.34))

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
(wavwrite piece sr "dark_tension2.wav" 2)