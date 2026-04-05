
; ============================================================================
; sol_idm_example.scm
; IDM-like example using SOL samples through samplesynth.h
;
; idea:
; - sample-based rhythmic writing with orchestral sources
; - low strings / bassoon / tuba act as broken bass and pulse
; - violin / flute / oboe / clarinet create chopped upper gestures
; - some optional electronic support from music.h can be added later,
;   but this patch is sample-driven
;
; requires:
;   add_samplesynth(env)
;
; replace the SOL path before running.
; ============================================================================

(def sr 48000)
(def bpm 164)

(def SOL (loaddb "/Users/n4/Projects/Media/Datasets/TinySOL"))

; ----------------------------------------------------------------------------
; low pulse / broken bass
; ----------------------------------------------------------------------------

(def cb_pulse_a
  (norm
    (sol-patnotes sr bpm 8
      (vec 1 0 0 0   0 0 1 0   1 0 0 0   0 1 0 0)
      SOL "Cb" "ord"
      (vec 36 38 35 31 33 38)
      "mf")
    0.84))

(def vc_bass_a
  (norm
    (sol-patnotes sr bpm 8
      (vec 1 0 1 0   0 0 1 0   1 0 0 0   1 0 0 0)
      SOL "Vc" "ord"
      (vec 36 43 41 38 36 45)
      "mf")
    0.86))

(def bn_bass_a
  (norm
    (sol-patnotes sr bpm 8
      (vec 1 0 0 0   1 0 0 0   0 0 1 0   0 0 0 0)
      SOL "Bn" "ord"
      (vec 43 45 47 50)
      "mf")
    0.70))

(def btb_hits
  (norm
    (sol-patnotes sr bpm 8
      (vec 1 0 0 0   0 0 0 0   0.8 0 0 0   0 0 0 0)
      SOL "BTb" "ord"
      (vec 35 31 38 43)
      "f")
    0.72))

; ----------------------------------------------------------------------------
; mid / upper rhythmic shards
; ----------------------------------------------------------------------------

(def va_chops_a
  (norm
    (sol-patnotes sr bpm 8
      (vec 0 1 0 0   0 1 0 0   0 0 1 0   0 1 0 0)
      SOL "Va" "ord"
      (vec 60 62 65 67 69)
      "mf")
    0.70))

(def vn_chops_a
  (norm
    (sol-patnotes sr bpm 8
      (vec 0 0 1 0   0 0 1 0   0 0 1 0   0 0 1 0)
      SOL "Vn" "ord"
      (vec 72 74 76 79 81 83)
      "mf")
    0.76))

(def vn_chops_b
  (norm
    (sol-patnotes sr bpm 8
      (vec 0 1 0 1   0 0 1 0   0 1 0 0   0 0 1 0)
      SOL "Vn" "ord"
      (vec 74 76 79 81 83 86)
      "f")
    0.78))

(def ob_stabs_a
  (norm
    (sol-patnotes sr bpm 8
      (vec 0 0 0 1   0 0 0 0   0 0 0 1   0 0 0 0)
      SOL "Ob" "ord"
      (vec 79 81 84 86)
      "mf")
    0.62))

(def cl_reply_a
  (norm
    (sol-patnotes sr bpm 8
      (vec 0 0 0 0   0 0 1 0   0 0 0 0   0 0 1 0)
      SOL "ClBb" "ord"
      (vec 67 71 74 78)
      "mf")
    0.64))

(def fl_spark_a
  (norm
    (sol-arp sr bpm 8
      SOL "Fl" "ord"
      (vec 84 88 91 96)
      "mf" 8 0)
    0.56))

(def fl_spark_b
  (norm
    (sol-arp sr bpm 8
      SOL "Fl" "ord"
      (vec 86 91 93 98)
      "f" 10 2)
    0.58))

(def tpc_spikes
  (norm
    (sol-patnotes sr bpm 8
      (vec 0 0 0 0   0 1 0 0   0 0 0 0   0 1 0 0)
      SOL "TpC" "ord"
      (vec 74 78 81 86)
      "f")
    0.54))

; ----------------------------------------------------------------------------
; hybrid pattern layers using banks and sample-seq
; sample-seq cycles through chosen entries from the db
; ----------------------------------------------------------------------------

(def vn_bank
  (bank
    (db-pick SOL "Vn" "ord" "C5" "mf")
    (db-pick SOL "Vn" "ord" "E5" "mf")
    (db-pick SOL "Vn" "ord" "G5" "mf")
    (db-pick SOL "Vn" "ord" "A5" "mf")))

(def fl_bank
  (bank
    (db-pick SOL "Fl" "ord" "C6" "mf")
    (db-pick SOL "Fl" "ord" "E6" "mf")
    (db-pick SOL "Fl" "ord" "G6" "mf")
    (db-pick SOL "Fl" "ord" "A6" "mf")))

(def seq_vn
  (norm
    (sample-seq sr bpm 8
      (vec 0 0 1 0   0 1 0 0   0 0 1 0   0 1 0 0)
      vn_bank
      (vec 0 1 2 3 2 1))
    0.72))

(def seq_fl
  (norm
    (sample-seq sr bpm 8
      (vec 0 0 0 1   0 0 0 0   0 0 0 1   0 0 0 1)
      fl_bank
      (vec 0 2 3 1))
    0.52))

; ----------------------------------------------------------------------------
; sections
; ----------------------------------------------------------------------------

(def sec1
  (norm
    (bmix sr bpm
      0 cb_pulse_a
      0 vc_bass_a)
    0.88))

(def sec2
  (norm
    (bmix sr bpm
      0 cb_pulse_a
      0 vc_bass_a
      0 va_chops_a
      0 ob_stabs_a)
    0.90))

(def sec3
  (norm
    (bmix sr bpm
      0 cb_pulse_a
      0 vc_bass_a
      0 bn_bass_a
      0 vn_chops_a
      0 cl_reply_a)
    0.91))

(def sec4
  (norm
    (bmix sr bpm
      0 cb_pulse_a
      0 vc_bass_a
      0 btb_hits
      0 vn_chops_b
      0 fl_spark_a)
    0.92))

(def sec5
  (norm
    (bmix sr bpm
      0 cb_pulse_a
      0 vc_bass_a
      0 bn_bass_a
      0 seq_vn
      0 seq_fl
      0 ob_stabs_a)
    0.92))

(def sec6
  (norm
    (bmix sr bpm
      0 cb_pulse_a
      0 vc_bass_a
      0 btb_hits
      0 va_chops_a
      0 vn_chops_b
      0 fl_spark_b
      0 tpc_spikes)
    0.94))

(def sec7
  (norm
    (bmix sr bpm
      0 cb_pulse_a
      0 vc_bass_a
      0 bn_bass_a
      0 seq_vn
      0 vn_chops_a
      0 cl_reply_a
      0 fl_spark_a)
    0.93))

(def sec8
  (norm
    (bmix sr bpm
      0 cb_pulse_a
      0 vc_bass_a
      0 btb_hits
      0 seq_vn
      0 seq_fl
      0 vn_chops_b
      0 tpc_spikes
      0 fl_spark_b)
    0.95))

; ----------------------------------------------------------------------------
; assemble a compact IDM-like form
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
      48 sec7
      56 sec8

      64 sec3
      72 sec5
      80 sec2
      88 sec7
      96 sec4
      104 sec8
      112 sec6
      120 sec5)
    0.95))

(def left
  (norm
    (bmix sr bpm
      0 mono
      40 (sol-patnotes sr bpm 16
           (vec 0 0 1 0   0 0 0 0   0 0 1 0   0 0 0 0)
           SOL "Ob" "ord"
           (vec 79 84 86)
           "mp"))
    0.95))

(def right
  (norm
    (bmix sr bpm
      0 mono
      56 (sol-patnotes sr bpm 16
           (vec 0 0 0 1   0 0 0 0   0 0 0 1   0 0 0 1)
           SOL "Fl" "ord"
           (vec 88 91 96)
           "mp"))
    0.95))

(def piece (stereo left right))
(wavwrite piece sr "sol_idm_example.wav" 2)
