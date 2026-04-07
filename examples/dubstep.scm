; ============================================================================
; dubstep-inspired long piece
; 240 bars at 96 bpm = 10 minutes exactly
; ============================================================================

(def sr 48000)
(def bpm 96)

; ----------------------------------------------------------------------------
; one-shot instruments
; ----------------------------------------------------------------------------

(def k1 (kick  sr 0.42 160 42 0.18 0.95))
(def k2 (kick  sr 0.55 120 36 0.24 0.90))

(def s1 (snare sr 0.22 180 0.78 0.11 0.72))
(def s2 (snare sr 0.30 140 0.65 0.16 0.84))

(def h1 (hat   sr 0.05 7000 0.035 0.18))
(def h2 (hat   sr 0.08 9000 0.060 0.15))
(def h3 (hat   sr 0.12 6000 0.090 0.12))

; ----------------------------------------------------------------------------
; drum pattern helpers
; 16 steps = one bar of 4/4 in 16ths
; ----------------------------------------------------------------------------

(def kA  (vec 1 0 0 0   0 0 1 0   1 0 0 0   0 1 0 0))
(def kB  (vec 1 0 0 0   0 1 0 0   1 0 0 1   0 0 0 0))
(def kC  (vec 1 0 1 0   0 0 1 0   1 0 0 0   1 0 0 0))
(def kD  (vec 1 0 0 0   0 0 0 0   1 0 1 0   0 0 0 1))
(def kE  (vec 1 0 0 1   0 0 1 0   1 0 0 0   0 0 1 0))
(def kF  (vec 1 0 0 0   1 0 0 0   1 0 1 0   0 0 0 0))
(def kG  (vec 1 0 0 0   0 0 1 0   1 0 0 1   0 1 0 0))
(def kH  (vec 1 0 1 0   1 0 0 0   1 0 0 0   0 0 1 0))

; classic halftime snare on beat 3, plus ghosts in some versions
(def sA  (vec 0 0 0 0   0 0 0 0   1 0 0 0   0 0 0 0))
(def sB  (vec 0 0 0 0   0 0 0 0   1 0 0 0   0 0 1 0))
(def sC  (vec 0 0 0 0   0 0 1 0   1 0 0 0   0 0 0 0))
(def sD  (vec 0 0 0 0   0 0 0 0   1 0 0 0   1 0 0 0))

(def hA  (euclid 10 16 0))
(def hB  (euclid 11 16 1))
(def hC  (euclid 12 16 2))
(def hD  (euclid 9  16 0))
(def hE  (vec 1 0 1 0   1 0 1 0   1 1 1 0   1 0 1 0))
(def hF  (vec 1 0 1 1   1 0 1 0   1 0 1 1   1 0 1 0))

(def oA  (vec 0 0 0 0   0 0 0 1   0 0 0 0   0 0 0 1))
(def oB  (vec 0 0 0 0   0 0 0 0   0 0 0 1   0 0 0 1))
(def oC  (vec 0 0 0 1   0 0 0 0   0 0 0 1   0 0 0 0))
(def oD  (vec 0 0 0 0   0 0 0 0   0 0 0 0   0 0 0 1))

; ----------------------------------------------------------------------------
; bar constructors
; ----------------------------------------------------------------------------

(def mkbar
  (lambda (kpat spat hpat opat)
    (normalize
      (bmix sr bpm
        0 (pat sr bpm 4 kpat k1)
        0 (pat sr bpm 4 spat s1)
        0 (pat sr bpm 4 hpat h1)
        0 (pat sr bpm 4 opat h2))
      0.88)))

(def mkbar_heavy
  (lambda (kpat spat hpat opat)
    (normalize
      (bmix sr bpm
        0 (pat sr bpm 4 kpat k2)
        0 (pat sr bpm 4 spat s2)
        0 (pat sr bpm 4 hpat h2)
        0 (pat sr bpm 4 opat h3))
      0.90)))

(def mkbar_break
  (lambda (kpat hpat opat)
    (normalize
      (bmix sr bpm
        0 (pat sr bpm 4 kpat k1)
        0 (pat sr bpm 4 hpat h2)
        0 (pat sr bpm 4 opat h3))
      0.82)))

; ----------------------------------------------------------------------------
; concrete bars
; ----------------------------------------------------------------------------

(def barA (mkbar       kA sA hA oA))
(def barB (mkbar       kB sA hB oB))
(def barC (mkbar       kC sB hC oA))
(def barD (mkbar       kD sA hD oD))
(def barE (mkbar_heavy kE sB hE oB))
(def barF (mkbar_heavy kF sC hF oC))
(def barG (mkbar_heavy kG sD hC oA))
(def barH (mkbar_heavy kH sB hB oD))

(def barI (mkbar_break kA hD oD))
(def barJ (mkbar_break kD hA oB))
(def barK (mkbar_break kG hF oC))
(def barL (mkbar_break kH hB oA))

; ----------------------------------------------------------------------------
; drones / bass beds
; each is around 8 bars = 20 seconds at 96 bpm
; ----------------------------------------------------------------------------

(def dA1 (drone sr (mtof 34) 20 0.18 0.05))
(def dA2 (drone sr (mtof 29) 20 0.16 0.08))

(def dB1 (drone sr (mtof 36) 20 0.20 0.10))
(def dB2 (drone sr (mtof 41) 20 0.15 0.14))

(def dC1 (drone sr (mtof 31) 20 0.17 0.07))
(def dC2 (drone sr (mtof 38) 20 0.14 0.12))

(def dD1 (drone sr (mtof 33) 20 0.19 0.18))
(def dD2 (drone sr (mtof 45) 20 0.13 0.22))

(def dE1 (drone sr (mtof 26) 20 0.20 0.04))
(def dE2 (drone sr (mtof 38) 20 0.12 0.09))

(def dF1 (drone sr (mtof 29) 20 0.18 0.16))
(def dF2 (drone sr (mtof 36) 20 0.13 0.20))

(def dG1 (drone sr (mtof 41) 20 0.16 0.06))
(def dG2 (drone sr (mtof 33) 20 0.15 0.11))

(def dH1 (drone sr (mtof 43) 20 0.17 0.15))
(def dH2 (drone sr (mtof 31) 20 0.15 0.19))

; side textures for stereo differences
(def dl1 (drone sr (mtof 50) 10 0.07 0.30))
(def dl2 (drone sr (mtof 57) 10 0.06 0.26))
(def dl3 (drone sr (mtof 45) 10 0.05 0.34))

(def dr1 (drone sr (mtof 53) 10 0.07 0.27))
(def dr2 (drone sr (mtof 60) 10 0.05 0.21))
(def dr3 (drone sr (mtof 48) 10 0.06 0.31))

; ----------------------------------------------------------------------------
; 16-bar section builder
; ----------------------------------------------------------------------------

(def section16
  (lambda (b1 b2 b3 b4 d1 d2)
    (normalize
      (bmix sr bpm
        0  b1
        4  b2
        8  b3
        12 b4

        16 b1
        20 b2
        24 b4
        28 b3

        32 b3
        36 b4
        40 b2
        44 b1

        48 b4
        52 b2
        56 b1
        60 b3

        0  d1
        32 d2)
      0.92)))

; ----------------------------------------------------------------------------
; 15 sections × 16 bars = 240 bars = 10 minutes
; ----------------------------------------------------------------------------

(def sec01 (section16 barI barA barB barJ dA1 dA2))
(def sec02 (section16 barA barB barC barD dB1 dB2))
(def sec03 (section16 barB barC barE barD dC1 dC2))
(def sec04 (section16 barJ barD barK barA dD1 dD2))
(def sec05 (section16 barC barE barF barB dE1 dE2))
(def sec06 (section16 barK barF barG barD dF1 dF2))
(def sec07 (section16 barD barB barH barC dG1 dG2))
(def sec08 (section16 barL barI barA barJ dH1 dH2))
(def sec09 (section16 barE barF barG barH dA1 dD2))
(def sec10 (section16 barB barD barC barF dB1 dE2))
(def sec11 (section16 barK barG barH barL dF1 dH2))
(def sec12 (section16 barA barC barB barD dC1 dG2))
(def sec13 (section16 barF barE barH barG dD1 dA2))
(def sec14 (section16 barJ barK barL barI dE1 dB2))
(def sec15 (section16 barH barG barF barE dH1 dC2))

; ----------------------------------------------------------------------------
; main mono body, 15 sections placed one after another
; each section = 64 beats = 16 bars
; ----------------------------------------------------------------------------

(def main
  (normalize
    (bmix sr bpm
      0   sec01
      64  sec02
      128 sec03
      192 sec04
      256 sec05
      320 sec06
      384 sec07
      448 sec08
      512 sec09
      576 sec10
      640 sec11
      704 sec12
      768 sec13
      832 sec14
      896 sec15)
    0.93))

; ----------------------------------------------------------------------------
; stereo decorations
; same body in both channels, but different side layers and extra hats
; ----------------------------------------------------------------------------

(def side_l
  (normalize
    (bmix sr bpm
      64  (pat sr bpm 16 (euclid 29 64 0) h2)
      192 (pat sr bpm 16 (euclid 23 64 3) h3)
      320 dl1
      384 (pat sr bpm 16 (euclid 31 64 1) h2)
      512 dl2
      640 (pat sr bpm 16 (euclid 27 64 2) h3)
      768 dl3
      896 (pat sr bpm 16 (euclid 25 64 0) h2))
    0.55))

(def side_r
  (normalize
    (bmix sr bpm
      0   dr1
      128 (pat sr bpm 16 (euclid 21 64 2) h3)
      256 dr2
      448 (pat sr bpm 16 (euclid 33 64 0) h2)
      576 dr3
      704 (pat sr bpm 16 (euclid 19 64 1) h3)
      832 dr1)
    0.55))

(def left
  (normalize
    (bmix sr bpm
      0 main
      0 side_l)
    0.95))

(def right
  (normalize
    (bmix sr bpm
      0 main
      0 side_r)
    0.95))

(def piece (interleave left right))
(wavwrite piece sr "dubstep_long_10min.wav" 2)