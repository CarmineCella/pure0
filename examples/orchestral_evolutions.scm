
; ============================================================================
; orchestral_evolutions.scm
; hybrid piece for pure0
; combines:
;   - techno / dark-house tools from music.h
;   - orchestral sample sequencing from samplesynth.h
;
; design:
;   - about 10 minutes
;   - intense long-form evolution
;   - alternating sections:
;       * instruments only
;       * drums only
;       * full hybrid texture
;   - orchestral pads + rhythmic bassline + drum patterns
;
; requires:
;   add_music(env)
;   add_samplesynth(env)
;
; instrument names in SOL:
;   Hn   = horn
;   Tbn  = trombone
;   TpC  = trumpet
;   BTb  = bass tuba
;   Vn   = violin
;   Va   = viola
;   Vc   = cello
;   Cb   = contrabass
;   Fl   = flute
;   ClBb = clarinet
;   Ob   = oboe
;   Bn   = bassoon
; ============================================================================

(def sr 48000)
(def bpm 120)

; adjust this path
(def SOL (loaddb "/Users/n4/Projects/Media/Datasets/TinySOL"))

; ----------------------------------------------------------------------------
; electronic instruments
; ----------------------------------------------------------------------------

(def k_main   (kick_sub   sr 0.46  98 28 0.26 0.88))
(def k_click  (kick_click sr 0.22 180 40 0.11 0.30 0.52))

(def h_dark   (hat_dark   sr 0.055 5400 0.060 0.10))
(def h_metal  (hat_metal  sr 0.090 7600 0.080 0.09))
(def h_tick   (hat_metal  sr 0.035 9300 0.030 0.06))

(def bed_a
  (norm
    (bmix sr bpm
      0 (drone_noise sr 49 20 0.11 0.05)
      0 (drone_reson sr 73 20 0.06 1.3 0.06)
      0 (pad_minor sr 45 20 0.08 0.05))
    0.78))

(def bed_b
  (norm
    (bmix sr bpm
      0 (drone_noise sr 43 20 0.12 0.07)
      0 (drone_reson sr 65 20 0.07 1.5 0.08)
      0 (pad_minor sr 41 20 0.09 0.06))
    0.80))

(def bed_c
  (norm
    (bmix sr bpm
      0 (drone_noise sr 38 20 0.13 0.09)
      0 (drone_reson sr 58 20 0.08 1.7 0.10)
      0 (pad_minor sr 38 20 0.10 0.07))
    0.82))

; ----------------------------------------------------------------------------
; electronic patterns
; ----------------------------------------------------------------------------

(def drums_a
  (norm
    (bmix sr bpm
      0 (pat sr bpm 8 (vec 1 0 0 0   1 0 0 0   1 0 0 0   1 0 0 0) k_main)
      0 (pat sr bpm 8 (vec 0.6 0 0 0   0.4 0 0 0   0.6 0 0 0   0.4 0 0 0) k_click)
      0 (pat sr bpm 8 (euclid 10 16 1) h_dark)
      0 (pat sr bpm 8 (vec 0 0 0 1   0 0 0 0   0 0 0 1   0 0 0 0) h_metal))
    0.92))

(def drums_b
  (norm
    (bmix sr bpm
      0 (pat sr bpm 8 (vec 1 0 0 0   1 0 0 0   1 0 0 0   1 0 0 0) k_main)
      0 (pat sr bpm 8 (vec 0.7 0 0 0   0.5 0 0 0   0.7 0 0 0   0.5 0 0 0) k_click)
      0 (pat sr bpm 8 (euclid 11 16 0) h_tick)
      0 (pat sr bpm 8 (vec 0 0 0 0   0 0 0 1   0 0 0 1   0 0 0 1) h_dark))
    0.93))

(def drums_c
  (norm
    (bmix sr bpm
      0 (pat sr bpm 8 (vec 1 0 0 0   1 0 0 0   1 0 0 0   1 0 0 0) k_main)
      0 (pat sr bpm 8 (vec 0.5 0 0 0   0.5 0 0 0   0.8 0 0 0   0.5 0 0 0) k_click)
      0 (pat sr bpm 8 (euclid 12 16 2) h_tick)
      0 (pat sr bpm 8 (vec 0 0 0 1   0 0 0 0   0 0 0 1   0 0 0 1) h_metal))
    0.94))

(def bassline_a
  (bassline sr bpm 8
    (vec 1 0 0 0   1 0 1 0   1 0 0 0   0 0 1 0)
    (vec 33 36 40 43)
    0.50 0.44 1.4 1))

(def bassline_b
  (bassline sr bpm 8
    (vec 1 0 0 0   0 0 1 0   1 0 0 0   1 0 1 0)
    (vec 31 34 38 41)
    0.50 0.46 1.9 1))

(def bassline_c
  (bassline sr bpm 8
    (vec 1 0 1 0   1 0 0 0   1 0 1 0   0 0 0 0)
    (vec 35 38 42 45)
    0.50 0.50 2.3 2))

(def bass_sub_a (bass_sub sr (mtof 33) 1.5 0.18 1.1))
(def bass_sub_b (bass_sub sr (mtof 31) 1.5 0.17 1.2))
(def bass_sub_c (bass_sub sr (mtof 35) 1.5 0.16 1.3))

(def stabs_a
  (norm
    (bmix sr bpm
      0 (pat sr bpm 8 (vec 1 0 0 0   0.5 0 0 0   0 0 1 0   0 0 0 0) (stab_minor sr 57 1.0 0.12 2))
      0 (pat sr bpm 8 (vec 0 0 0 0   1 0 0 0   0 0 0 0   0.5 0 0 0) (stab_minor sr 53 1.5 0.10 1)))
    0.70))

(def stabs_b
  (norm
    (bmix sr bpm
      0 (pat sr bpm 8 (vec 1 0 0 0   0 0 0 0   0.7 0 0 0   0 0 1 0) (stab_minor sr 52 2.0 0.10 1))
      0 (pat sr bpm 8 (vec 0 0 1 0   0 0 0 0   0.6 0 0 0   0 0 0 0) (stab_minor sr 60 1.0 0.08 2)))
    0.70))

(def arp_a (arp sr bpm 8 (vec 69 72 76 79) 8  0.125 0.045 2 2))
(def arp_b (arp sr bpm 8 (vec 67 70 74 77) 6  0.166 0.040 0 0))
(def arp_c (arp sr bpm 8 (vec 71 74 78 81) 12 0.083 0.038 1 2))

; ----------------------------------------------------------------------------
; orchestral sample layers
; all use patterning, not only sustained notes
; ----------------------------------------------------------------------------

(def orch_brass_low
  (norm
    (bmix sr bpm
      0 (sol-patnotes sr bpm 8
           (vec 1 0 0 0   0 0 0 0   0.8 0 0 0   0 0 0 0)
           SOL "BTb" "ord" (vec 35 38 42 43) "mf")
      0 (sol-patnotes sr bpm 8
           (vec 1 0 0 0   0.7 0 0 0   1 0 0 0   0.7 0 0 0)
           SOL "Tbn" "ord" (vec 47 50 54 55) "mf")
      0 (sol-patnotes sr bpm 8
           (vec 0 0 1 0   0 0 0 0   0 0 1 0   0 0 0 0)
           SOL "Hn" "ord" (vec 55 57 59 62) "mf"))
    0.84))

(def orch_strings_pad
  (norm
    (bmix sr bpm
      0 (sol-patnotes sr bpm 8
           (vec 1 0 0 0   0.8 0 0 0   1 0 0 0   0.8 0 0 0)
           SOL "Cb" "ord" (vec 35 38 42 47) "mf")
      0 (sol-patnotes sr bpm 8
           (vec 1 0 0 0   1 0 0 0   0.8 0 0 0   1 0 0 0)
           SOL "Vc" "ord" (vec 47 50 54 57) "mf")
      0 (sol-patnotes sr bpm 8
           (vec 1 0 0 0   0 0 0 0   1 0 0 0   0 0 0 0)
           SOL "Va" "ord" (vec 54 57 59 62) "mf")
      0 (sol-patnotes sr bpm 8
           (vec 0 0 1 0   0 0 0 0   0 0 1 0   0 0 0 0)
           SOL "Vn" "ord" (vec 62 66 69 74) "mf"))
    0.86))

(def orch_winds_motifs
  (norm
    (bmix sr bpm
      0 (sol-arp sr bpm 8 SOL "Ob"   "ord" (vec 74 78 81 86) "mf" 4 2)
      0 (sol-arp sr bpm 8 SOL "Fl"   "ord" (vec 78 81 86 90) "mf" 6 0)
      0 (sol-patnotes sr bpm 8
           (vec 0 0 1 0   0 0 1 0   0 0 1 0   0 0 1 0)
           SOL "ClBb" "ord" (vec 69 74 78 81) "mf")
      0 (sol-patnotes sr bpm 8
           (vec 1 0 0 0   0 0 0 0   0.7 0 0 0   0 0 0 0)
           SOL "Bn" "ord" (vec 47 50 54 57) "mf"))
    0.84))

(def orch_brass_high
  (norm
    (bmix sr bpm
      0 (sol-patnotes sr bpm 8
           (vec 1 0 0 0   0.8 0 0 0   1 0 0 0   0.8 0 0 0)
           SOL "Hn" "ord" (vec 59 62 66 69) "f")
      0 (sol-patnotes sr bpm 8
           (vec 1 0 0 0   1 0 0 0   1 0 0 0   0.7 0 0 0)
           SOL "TpC" "ord" (vec 66 69 74 78) "f")
      0 (sol-patnotes sr bpm 8
           (vec 0 0 1 0   0 0 1 0   0 0 1 0   0 0 1 0)
           SOL "Tbn" "ord" (vec 54 57 62 66) "f"))
    0.88))

(def orch_full_low
  (norm
    (bmix sr bpm
      0 orch_brass_low
      0 orch_strings_pad
      0 (sol-patnotes sr bpm 8
           (vec 1 0 0 0   0 0 0 0   1 0 0 0   0 0 0 0)
           SOL "Bn" "ord" (vec 42 45 47 50) "mf"))
    0.90))

(def orch_full_high
  (norm
    (bmix sr bpm
      0 orch_strings_pad
      0 orch_winds_motifs
      0 orch_brass_high)
    0.92))

; ----------------------------------------------------------------------------
; hybrid sections
; ----------------------------------------------------------------------------

(def sec_instr_1
  (norm
    (bmix sr bpm
      0 bed_a
      0 orch_brass_low)
    0.90))

(def sec_instr_2
  (norm
    (bmix sr bpm
      0 bed_b
      0 orch_full_low)
    0.91))

(def sec_instr_3
  (norm
    (bmix sr bpm
      0 bed_c
      0 orch_full_high)
    0.92))

(def sec_drums_1
  (norm
    (bmix sr bpm
      0 drums_a
      0 bassline_a
      0 (pat sr bpm 8 (vec 1 0 0 0   0 0 0 0   1 0 0 0   0 0 0 0) bass_sub_a))
    0.92))

(def sec_drums_2
  (norm
    (bmix sr bpm
      0 drums_b
      0 bassline_b
      0 stabs_a
      0 (pat sr bpm 8 (vec 1 0 0 0   0 0 1 0   1 0 0 0   0 0 0 0) bass_sub_b))
    0.93))

(def sec_drums_3
  (norm
    (bmix sr bpm
      0 drums_c
      0 bassline_c
      0 stabs_b
      0 arp_a
      0 (pat sr bpm 8 (vec 1 0 1 0   0 0 0 0   1 0 0 0   0 0 1 0) bass_sub_c))
    0.94))

(def sec_full_1
  (norm
    (bmix sr bpm
      0 drums_a
      0 bassline_a
      0 stabs_a
      0 orch_full_low
      0 bed_a)
    0.94))

(def sec_full_2
  (norm
    (bmix sr bpm
      0 drums_b
      0 bassline_b
      0 stabs_b
      0 arp_b
      0 orch_full_low
      0 orch_winds_motifs
      0 bed_b)
    0.95))

(def sec_full_3
  (norm
    (bmix sr bpm
      0 drums_c
      0 bassline_c
      0 arp_c
      0 orch_full_high
      0 bed_c)
    0.95))

(def sec_pad_bass_1
  (norm
    (bmix sr bpm
      0 orch_strings_pad
      0 bed_b
      0 bassline_a
      0 (pat sr bpm 8 (vec 1 0 0 0   1 0 0 0   0 0 1 0   1 0 0 0) bass_sub_a))
    0.92))

(def sec_pad_bass_2
  (norm
    (bmix sr bpm
      0 orch_strings_pad
      0 orch_winds_motifs
      0 bed_c
      0 bassline_b
      0 stabs_a)
    0.93))

(def sec_climax
  (norm
    (bmix sr bpm
      0 drums_c
      0 bassline_c
      0 arp_c
      0 stabs_b
      0 orch_brass_high
      0 orch_strings_pad
      0 orch_winds_motifs
      0 bed_c)
    0.97))

; ----------------------------------------------------------------------------
; large form
; each section = 8 beats = 4 seconds at 120 bpm
; 150 sections = 600 s = 10 minutes
; ----------------------------------------------------------------------------

(def piece_body
  (norm
    (bmix sr bpm
      ; block 1
      0    sec_instr_1
      8    sec_drums_1
      16   sec_instr_2
      24   sec_full_1
      32   sec_drums_2
      40   sec_pad_bass_1
      48   sec_instr_3
      56   sec_full_2
      64   sec_drums_3
      72   sec_full_3

      ; block 2
      80   sec_instr_2
      88   sec_drums_1
      96   sec_pad_bass_1
      104  sec_full_1
      112  sec_instr_3
      120  sec_drums_2
      128  sec_pad_bass_2
      136  sec_full_2
      144  sec_drums_3
      152  sec_full_3

      ; block 3
      160  sec_instr_1
      168  sec_drums_2
      176  sec_instr_2
      184  sec_full_1
      192  sec_pad_bass_2
      200  sec_drums_3
      208  sec_instr_3
      216  sec_full_2
      224  sec_pad_bass_1
      232  sec_full_3

      ; block 4
      240  sec_instr_2
      248  sec_drums_1
      256  sec_instr_3
      264  sec_full_2
      272  sec_drums_3
      280  sec_pad_bass_2
      288  sec_instr_1
      296  sec_full_1
      304  sec_drums_2
      312  sec_full_3

      ; block 5
      320  sec_instr_3
      328  sec_pad_bass_1
      336  sec_drums_3
      344  sec_full_2
      352  sec_instr_2
      360  sec_drums_1
      368  sec_pad_bass_2
      376  sec_full_1
      384  sec_instr_1
      392  sec_full_3

      ; block 6
      400  sec_drums_2
      408  sec_instr_3
      416  sec_pad_bass_1
      424  sec_full_2
      432  sec_drums_3
      440  sec_instr_2
      448  sec_full_1
      456  sec_pad_bass_2
      464  sec_drums_1
      472  sec_full_3

      ; block 7
      480  sec_instr_1
      488  sec_drums_2
      496  sec_instr_2
      504  sec_full_1
      512  sec_instr_3
      520  sec_drums_3
      528  sec_pad_bass_1
      536  sec_full_2
      544  sec_drums_1
      552  sec_full_3

      ; block 8
      560  sec_instr_2
      568  sec_pad_bass_2
      576  sec_drums_3
      584  sec_full_2
      592  sec_instr_3
      600  sec_drums_2
      608  sec_full_1
      616  sec_pad_bass_1
      624  sec_drums_1
      632  sec_full_3

      ; block 9
      640  sec_instr_1
      648  sec_drums_1
      656  sec_instr_2
      664  sec_full_1
      672  sec_pad_bass_2
      680  sec_drums_2
      688  sec_instr_3
      696  sec_full_2
      704  sec_drums_3
      712  sec_full_3

      ; block 10
      720  sec_instr_3
      728  sec_pad_bass_1
      736  sec_drums_3
      744  sec_full_2
      752  sec_instr_2
      760  sec_drums_2
      768  sec_full_1
      776  sec_pad_bass_2
      784  sec_drums_1
      792  sec_climax

      ; final stretch
      800  sec_full_3
      808  sec_climax
      816  sec_drums_3
      824  sec_full_2
      832  sec_instr_3
      840  sec_climax
      848  sec_full_3
      856  sec_pad_bass_2
      864  sec_drums_2
      872  sec_climax

      880  sec_full_3
      888  sec_climax
      896  sec_instr_2
      904  sec_full_2
      912  sec_drums_3
      920  sec_climax
      928  sec_full_3
      936  sec_instr_1
      944  sec_pad_bass_1
      952  sec_full_2

      960  sec_climax
      968  sec_full_3
      976  sec_drums_1
      984  sec_instr_3
      992  sec_full_2
      1000 sec_climax
      1008 sec_full_3
      1016 sec_pad_bass_2
      1024 sec_drums_2
      1032 sec_climax

      1040 sec_full_3
      1048 sec_climax
      1056 sec_instr_2
      1064 sec_full_2
      1072 sec_drums_3
      1080 sec_climax
      1088 sec_full_3
      1096 sec_pad_bass_1
      1104 sec_drums_1
      1112 sec_climax

      1120 sec_full_3
      1128 sec_climax
      1136 sec_instr_1
      1144 sec_full_2
      1152 sec_drums_2
      1160 sec_climax
      1168 sec_full_3
      1176 sec_instr_3
      1184 sec_pad_bass_2
      1192 sec_full_1)
    0.96))

; ----------------------------------------------------------------------------
; stereo side detail
; ----------------------------------------------------------------------------

(def left_side
  (norm
    (bmix sr bpm
      96  (sol-arp sr bpm 16 SOL "Ob" "ord" (vec 74 78 81 86) "mf" 6 2)
      352 (sol-patnotes sr bpm 16
             (vec 0 0 1 0   0 0 0 0   0 0 1 0   0 0 0 0)
             SOL "Fl" "ord" (vec 78 81 86 90) "mf")
      736 (arp sr bpm 16 (vec 81 78 74 69) 10 0.08 0.025 2 1)
      1000 (sol-arp sr bpm 16 SOL "Vn" "ord" (vec 74 78 81 86) "f" 8 2))
    0.28))

(def right_side
  (norm
    (bmix sr bpm
      160 (sol-arp sr bpm 16 SOL "ClBb" "ord" (vec 69 74 78 81) "mf" 6 0)
      448 (sol-patnotes sr bpm 16
             (vec 0 0 0 0   0 0 1 0   0 0 0 0   0 0 1 0)
             SOL "Ob" "ord" (vec 74 78 81 86) "mf")
      816 (arp sr bpm 16 (vec 79 74 71 67) 12 0.07 0.024 1 2)
      1080 (sol-arp sr bpm 16 SOL "TpC" "ord" (vec 66 69 74 78) "f" 6 2))
    0.28))

(def left
  (norm
    (bmix sr bpm
      0 piece_body
      0 left_side)
    0.95))

(def right
  (norm
    (bmix sr bpm
      0 piece_body
      0 right_side)
    0.95))

(def piece (stereo left right))
(wavwrite piece sr "orchestral_evolutions.wav" 2)
