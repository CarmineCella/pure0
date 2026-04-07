; ============================================================================
; dark_house.mu
; original dark creative house piece
; ============================================================================

(def sr 48000)
(def bpm 124)

; ----------------------------------------------------------------------------
; drums / timbres
; ----------------------------------------------------------------------------

(def kick_main  (kick_sub   sr 0.48 100 30 0.26 0.86))
(def kick_top   (kick_click sr 0.22 180 42 0.10 0.35 0.55))

(def hat_closed (hat_dark   sr 0.055 5200 0.060 0.11))
(def hat_open   (hat_metal  sr 0.110 7600 0.080 0.10))
(def hat_tick   (hat_metal  sr 0.040 9200 0.030 0.07))

; ----------------------------------------------------------------------------
; harmonic world: minor only
; ----------------------------------------------------------------------------

(def bass_a (vec 33 36 40 43))   ; A C E G
(def bass_b (vec 31 34 38 41))   ; G Bb D F
(def bass_c (vec 29 33 36 40))   ; F A C E
(def bass_d (vec 28 31 35 38))   ; E G B D

(def arp_a  (vec 69 72 76 79))   ; A C E G
(def arp_b  (vec 67 70 74 77))   ; G Bb D F
(def arp_c  (vec 65 69 72 76))   ; F A C E
(def arp_d  (vec 64 67 71 74))   ; E G B D

; ----------------------------------------------------------------------------
; dark beds
; ----------------------------------------------------------------------------

(def bed_a
  (normalize
    (bmix sr bpm
      0 (drone_noise sr 55 16 0.14 0.05)
      0 (drone_reson sr 73 16 0.08 1.2 0.04)
      0 (pad_minor sr 45 16 0.11 0.04))
    0.82))

(def bed_b
  (normalize
    (bmix sr bpm
      0 (drone_noise sr 49 16 0.15 0.07)
      0 (drone_reson sr 65 16 0.09 1.4 0.05)
      0 (pad_minor sr 43 16 0.12 0.05))
    0.84))

(def bed_c
  (normalize
    (bmix sr bpm
      0 (drone_noise sr 43 16 0.16 0.08)
      0 (drone_reson sr 58 16 0.10 1.5 0.06)
      0 (pad_minor sr 41 16 0.12 0.06))
    0.86))

(def bed_d
  (normalize
    (bmix sr bpm
      0 (drone_noise sr 41 16 0.17 0.09)
      0 (drone_reson sr 55 16 0.11 1.7 0.07)
      0 (pad_minor sr 40 16 0.13 0.07))
    0.88))

; ----------------------------------------------------------------------------
; house drum bars
; one bar = 4 beats = 16 steps
; ----------------------------------------------------------------------------

(def drums_a
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 4 (vec 1 0 0 0   1 0 0 0   1 0 0 0   1 0 0 0) kick_main)
      0 (pat sr bpm 4 (vec 0.6 0 0 0   0.5 0 0 0   0.6 0 0 0   0.5 0 0 0) kick_top)
      0 (pat sr bpm 4 (vec 0 0 1 0   0 0 1 0   0 0 1 0   0 0 1 0) hat_closed)
      0 (pat sr bpm 4 (vec 0 0 0 0   0 0 0 1   0 0 0 0   0 0 0 1) hat_open))
    0.92))

(def drums_b
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 4 (vec 1 0 0 0   1 0 0 0   1 0 0 0   1 0 0 0) kick_main)
      0 (pat sr bpm 4 (vec 0.7 0 0 0   0.4 0 0 0   0.7 0 0 0   0.4 0 0 0) kick_top)
      0 (pat sr bpm 4 (euclid 10 16 1) hat_closed)
      0 (pat sr bpm 4 (vec 0 0 0 0   0 0 0 1   0 0 0 1   0 0 0 1) hat_tick))
    0.92))

(def drums_c
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 4 (vec 1 0 0 0   1 0 0 0   1 0 0 0   1 0 0 0) kick_main)
      0 (pat sr bpm 4 (vec 0.5 0 0 0   0.5 0 0 0   0.8 0 0 0   0.5 0 0 0) kick_top)
      0 (pat sr bpm 4 (euclid 11 16 0) hat_closed)
      0 (pat sr bpm 4 (vec 0 0 0 1   0 0 0 0   0 0 0 1   0 0 0 0) hat_open))
    0.92))

(def drums_d
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 4 (vec 1 0 0 0   1 0 0 0   1 0 0 0   1 0 0 0) kick_main)
      0 (pat sr bpm 4 (vec 0.6 0 0 0   0.6 0 0 0   0.6 0 0 0   0.8 0 0 0) kick_top)
      0 (pat sr bpm 4 (euclid 12 16 2) hat_tick)
      0 (pat sr bpm 4 (vec 0 0 0 0   0 0 0 1   0 0 0 0   0 0 0 1) hat_open))
    0.93))

; ----------------------------------------------------------------------------
; bass movement
; ----------------------------------------------------------------------------

(def bassline_a
  (bassline sr bpm 8
    (vec 1 0 0 0   1 0 1 0   1 0 0 0   0 0 1 0)
    bass_a
    0.50 0.46 1.2 1))

(def bassline_b
  (bassline sr bpm 8
    (vec 1 0 0 0   0 0 1 0   1 0 0 0   1 0 1 0)
    bass_b
    0.50 0.48 1.8 1))

(def bassline_c
  (bassline sr bpm 8
    (vec 1 0 1 0   1 0 0 0   1 0 1 0   0 0 0 0)
    bass_c
    0.50 0.50 2.2 2))

(def bassline_d
  (bassline sr bpm 8
    (vec 1 0 0 0   1 0 0 0   0 0 1 0   1 0 0 0)
    bass_d
    0.75 0.44 1.4 1))

(def sub_a
  (bass_sub sr (mtof 33) 1.5 0.18 1.0))

(def sub_b
  (bass_sub sr (mtof 31) 1.5 0.16 1.1))

(def sub_c
  (bass_sub sr (mtof 29) 1.5 0.16 1.2))

(def sub_d
  (bass_sub sr (mtof 28) 1.5 0.15 1.3))

; ----------------------------------------------------------------------------
; stabs and arps
; ----------------------------------------------------------------------------

(def stabs_a
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 8 (vec 1 0 0 0   0 0 0 0   0.6 0 0 0   0 0 0 0) (stab_minor sr 57 1.5 0.14 2))
      0 (pat sr bpm 8 (vec 0 0 0 0   1 0 0 0   0 0 0 0   0.5 0 0 0) (stab_minor sr 55 1.0 0.11 1)))
    0.74))

(def stabs_b
  (normalize
    (bmix sr bpm
      0 (pat sr bpm 8 (vec 1 0 0 0   0.5 0 0 0   0 0 1 0   0 0 0 0) (stab_minor sr 53 1.0 0.12 2))
      0 (pat sr bpm 8 (vec 0 0 0 0   0 0 0 0   1 0 0 0   0 0 0 0) (stab_minor sr 52 2.0 0.10 1)))
    0.74))

(def arp_line_a (arp sr bpm 8 arp_a 8  0.125 0.05 2 2))
(def arp_line_b (arp sr bpm 8 arp_b 6  0.166 0.045 0 0))
(def arp_line_c (arp sr bpm 8 arp_c 8  0.125 0.045 2 1))
(def arp_line_d (arp sr bpm 8 arp_d 12 0.083 0.040 1 2))

; ----------------------------------------------------------------------------
; sections, each 8 beats
; ----------------------------------------------------------------------------

(def sec_01
  (normalize
    (bmix sr bpm
      0 bed_a)
    0.88))

(def sec_02
  (normalize
    (bmix sr bpm
      0 bed_a
      0 drums_a)
    0.90))

(def sec_03
  (normalize
    (bmix sr bpm
      0 bed_b
      0 drums_b)
    0.91))

(def sec_04
  (normalize
    (bmix sr bpm
      0 bed_b
      0 drums_b
      0 bassline_a
      0 (pat sr bpm 8 (vec 1 0 0 0   0 0 0 0   1 0 0 0   0 0 0 0) sub_a))
    0.92))

(def sec_05
  (normalize
    (bmix sr bpm
      0 bed_c
      0 drums_c
      0 bassline_b
      0 (pat sr bpm 8 (vec 1 0 0 0   0 0 1 0   1 0 0 0   0 0 0 0) sub_b))
    0.93))

(def sec_06
  (normalize
    (bmix sr bpm
      0 bed_c
      0 drums_c
      0 bassline_b
      0 stabs_a)
    0.93))

(def sec_07
  (normalize
    (bmix sr bpm
      0 bed_d
      0 drums_d
      0 bassline_c
      0 stabs_a
      0 arp_line_a)
    0.94))

(def sec_08
  (normalize
    (bmix sr bpm
      0 bed_d
      0 drums_d
      0 bassline_c
      0 stabs_b
      0 arp_line_b)
    0.94))

(def sec_09
  (normalize
    (bmix sr bpm
      0 bed_c
      0 drums_b
      0 bassline_d
      0 stabs_b
      0 arp_line_c
      0 (pat sr bpm 8 (vec 1 0 1 0   0 0 0 0   1 0 0 0   0 0 1 0) sub_c))
    0.95))

(def sec_10
  (normalize
    (bmix sr bpm
      0 bed_b
      0 drums_a
      0 bassline_a
      0 stabs_a
      0 arp_line_d
      0 (pat sr bpm 8 (vec 1 0 0 0   1 0 0 0   0 0 1 0   1 0 0 0) sub_d))
    0.95))

(def sec_11
  (normalize
    (bmix sr bpm
      0 bed_a
      0 drums_c
      0 bassline_b
      0 stabs_b)
    0.93))

(def sec_12
  (normalize
    (bmix sr bpm
      0 bed_d
      0 drums_d
      0 bassline_d
      0 stabs_a
      0 arp_line_a)
    0.95))

; ----------------------------------------------------------------------------
; form
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

      96  sec_04
      104 sec_05
      112 sec_07
      120 sec_08
      128 sec_10
      136 sec_11
      144 sec_12
      152 sec_09)
    0.95))

; ----------------------------------------------------------------------------
; stereo side color
; ----------------------------------------------------------------------------

(def left_side
  (normalize
    (bmix sr bpm
      24  (drone_reson sr 92 10 0.03 1.6 0.18)
      64  (arp sr bpm 8 (vec 81 76 72 69) 10 0.08 0.026 2 1)
      120 (pat sr bpm 8 (euclid 15 32 0) hat_open))
    0.30))

(def right_side
  (normalize
    (bmix sr bpm
      40  (drone_reson sr 98 10 0.03 1.8 0.21)
      88  (arp sr bpm 8 (vec 79 74 71 67) 12 0.07 0.026 1 2)
      136 (pat sr bpm 8 (euclid 13 32 1) hat_tick))
    0.30))

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
(wavwrite piece sr "dark_house.wav" 2)