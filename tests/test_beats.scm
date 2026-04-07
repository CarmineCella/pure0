; test_beats.scm — tests for beats.h primitives
(load "stdlib.scm")

(def assert
  (lambda (name cond)
    (if cond
        (print "PASS" name)
        (error (append "FAIL: " name)))))

(def assert-eq
  (lambda (name got expected)
    (assert name (= got expected))))

(def assert-near
  (lambda (name got expected eps)
    (assert name (< (abs (- got expected)) eps))))

(def sr 44100)
(def bpm 120)

; -----------------------------------------
; pitch conversions
; -----------------------------------------

(assert-near "mtof 69" (nth (mtof (vec 69)) 0) 440 0.0001)
(assert-near "ftom 440" (nth (ftom (vec 440)) 0) 69 0.0001)
(assert-near "ftom/mtof rt" (nth (ftom (mtof (vec 60 69 72))) 1) 69 0.001)

; -----------------------------------------
; sequence helpers
; -----------------------------------------

(assert-eq "seq len" (len (seq 8 1 2 3)) 8)
(assert-eq "seq vals" (seq 8 1 2 3) (vec 1 2 3 1 2 3 1 2))

(assert-eq "hold len" (len (hold (vec 10 20 30) 2)) 6)
(assert-eq "hold vals" (hold (vec 10 20 30) 2) (vec 10 10 20 20 30 30))

; -----------------------------------------
; impulses / noise / pulse / euclid / steps
; -----------------------------------------

(assert-eq "impulse len" (len (impulse 8 3)) 8)
(assert-near "impulse pos" (nth (impulse 8 3) 3) 1 0.0001)
(assert-near "impulse rest" (sum (impulse 8 3)) 1 0.0001)

(def nz (noise 64))
(assert-eq "noise len" (len nz) 64)
(assert "noise bounded hi" (<= (max nz) 1.0001))
(assert "noise bounded lo" (>= (min nz) -1.0001))

(def pul (pulse 16 4 1 0.5))
(assert-eq "pulse len" (len pul) 16)
(assert-near "pulse first" (nth pul 0) 0.5 0.0001)
(assert-near "pulse step" (nth pul 4) 0.5 0.0001)

(def e33 (euclid 3 8))
(assert-eq "euclid len" (len e33) 8)
(assert-near "euclid count" (sum e33) 3 0.0001)

(def st (steps sr bpm 4 (vec 1 0 1 0)))
(assert-eq "steps len" (len st) (beatsamps sr bpm 4))
(assert-near "steps first hit" (nth st 0) 1 0.0001)

; -----------------------------------------
; beat samples
; -----------------------------------------

(assert-near "beatsamps quarter at 120 bpm" (beatsamps sr bpm 1) 22050 0.0001)
(assert-near "beatsamps two beats at 120 bpm" (beatsamps sr bpm 2) 44100 0.0001)

; -----------------------------------------
; tone / drones / percussion / harmonic textures
; -----------------------------------------

(def tn (tone sr 440 0.1 0.5))
(assert-eq "tone len" (len tn) 4410)
(assert "tone bounded" (<= (max (abs tn)) 0.5001))

(def dr (drone sr 110 0.2 0.4 0.5))
(assert-eq "drone len" (len dr) 8820)
(assert "drone bounded" (<= (max (abs dr)) 0.4001))

(def drn (drone_noise sr 110 0.2 0.4 0.3))
(assert-eq "drone_noise len" (len drn) 8820)
(assert "drone_noise bounded" (<= (max (abs drn)) 1.0001))

(def drr (drone_reson sr 110 0.2 0.4 0.4 0.3))
(assert-eq "drone_reson len" (len drr) 8820)
(assert "drone_reson bounded" (<= (max (abs drr)) 1.0001))

(def kk (kick sr 0.15 120 40 0.08 0.8))
(assert-eq "kick len" (len kk) 6615)
(assert "kick bounded" (<= (max (abs kk)) 1.0001))

(def kkc (kick_click sr 0.15 120 40 0.08 0.5 0.8))
(assert-eq "kick_click len" (len kkc) 6615)
(assert "kick_click bounded" (<= (max (abs kkc)) 1.0001))

(def kks (kick_sub sr 0.15 120 40 0.08 0.8))
(assert-eq "kick_sub len" (len kks) 6615)
(assert "kick_sub bounded" (<= (max (abs kks)) 1.0001))

(def sn (snare sr 0.15 180 0.5 0.08 0.8))
(assert "snare nonempty" (> (len sn) 0))
(assert "snare bounded" (<= (max (abs sn)) 1.0001))

(def ht (hat sr 0.08 6000 0.03 0.8))
(assert-eq "hat len" (len ht) 3528)
(assert "hat bounded" (<= (max (abs ht)) 1.0001))

(def htd (hat_dark sr 0.08 4000 0.03 0.8))
(assert-eq "hat_dark len" (len htd) 3528)
(assert "hat_dark bounded" (<= (max (abs htd)) 1.0001))

(def htm (hat_metal sr 0.08 3000 0.03 0.8))
(assert-eq "hat_metal len" (len htm) 3528)
(assert "hat_metal bounded" (<= (max (abs htm)) 1.0001))

(def pd (pad_minor sr 57 0.3 0.6 0.4))
(assert-eq "pad_minor len" (len pd) 13230)
(assert "pad_minor bounded" (<= (max (abs pd)) 1.0001))

(def sb (stab_minor sr 57 0.15 0.7 1))
(assert-eq "stab_minor len" (len sb) 6615)
(assert "stab_minor bounded" (<= (max (abs sb)) 1.0001))

(def bs (bass_sub sr 55 0.2 0.8 1.5))
(assert-eq "bass_sub len" (len bs) 8820)
(assert "bass_sub bounded" (<= (max (abs bs)) 1.0001))

; -----------------------------------------
; beat-domain mixing and patterning
; -----------------------------------------

(def ev (vec 1 0 0))
(def pa (pat sr bpm 4 (vec 1 0 0 1) ev))
(assert-eq "pat len" (len pa) (beatsamps sr bpm 4))
(assert-near "pat first" (nth pa 0) 1 0.0001)
(assert-near "pat last trigger exists" (> (sum pa) 1) 1 0.0001)

(def bmx (bmix sr bpm
               0   (vec 1 1 1)
               0.5 (vec 2 2)))
(assert "bmix nonempty" (> (len bmx) 3))
(assert-near "bmix first" (nth bmx 0) 1 0.0001)

(def pn (patnotes sr bpm 4 (vec 1 0 1 0) (vec 60 64) 0.5 0.6))
(assert-eq "patnotes len" (len pn) (beatsamps sr bpm 4))
(assert "patnotes bounded" (<= (max (abs pn)) 1.0001))

(def bl (bassline sr bpm 4 (vec 1 0 1 0) (vec 36 43) 0.5 0.7))
(assert-eq "bassline len" (len bl) (beatsamps sr bpm 4))
(assert "bassline bounded" (<= (max (abs bl)) 1.0001))

(def ar0 (arp sr bpm 4 (vec 60 64 67) 2 0.25 0.5))
(assert-eq "arp len" (len ar0) (beatsamps sr bpm 4))
(assert "arp bounded" (<= (max (abs ar0)) 1.0001))

(def ar1 (arp sr bpm 4 (vec 60 64 67) 2 0.25 0.5 0 1))
(assert-eq "arp mode down len" (len ar1) (beatsamps sr bpm 4))
(assert "arp mode down bounded" (<= (max (abs ar1)) 1.0001))

(def ar2 (arp sr bpm 4 (vec 60 64 67) 2 0.25 0.5 0 2))
(assert-eq "arp mode updown len" (len ar2) (beatsamps sr bpm 4))
(assert "arp mode updown bounded" (<= (max (abs ar2)) 1.0001))

; -----------------------------------------
; gate
; -----------------------------------------

(assert-eq "gate mask" (gate (vec 1 2 3 4) (vec 1 0 1 0)) (vec 1 0 3 0))

(print "")
(print "all beats tests passed")
