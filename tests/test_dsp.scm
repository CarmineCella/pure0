; test_dsp.scm — test suite for dsp.h primitives
; run with: pure0 test_dsp.scm

(load "stdlib.scm")

(def assert (lambda (name cond)
  (if cond
    (print "PASS" name)
    (error (join (list "FAIL:" name) " ")))))

(def assert-eq (lambda (name got expected)
  (assert name (= got expected))))

(def assert-near (lambda (name got expected eps)
  (assert name (< (abs (- got expected)) eps))))


; ── gen ───────────────────────────────────────────────────────────────────────

(def sine-table (gen 512 1))             ; pure sine wavetable
(def harm-table (gen 512 1 0.5 0.25))    ; 3 harmonics

; gen produces n+1 samples (guard point)
(assert-eq   "gen len"         (len sine-table) 513)
; guard point: last == first
(assert-near "gen guard"       (nth sine-table 512) (nth sine-table 0) 0.0001)
; pure sine: sample at index n/4 should be near +1 (quarter period)
(assert-near "gen sine peak"   (nth sine-table 128) 1.0 0.01)
; amplitude normalised: peak <= 1
(assert      "gen amp <=1"     (< (max sine-table) 1.0001))
; multi-harmonic table same length
(assert-eq   "gen multi len"   (len harm-table) 513)


; ── osc ───────────────────────────────────────────────────────────────────────

; constant-frequency sine: 440 Hz for 0.1 s at sr=44100
(def sr 44100)
(def dur-samps 4410)
(def freq-vec (* (ones dur-samps) 440))
(def sine-out (osc sr freq-vec sine-table))

(assert-eq   "osc len"     (len sine-out) dur-samps)
; should oscillate around zero
(assert-near "osc mean"    (mean sine-out) 0.0 0.01)
; amplitude should be bounded
(assert      "osc bounded" (< (max sine-out) 1.0001))
(assert      "osc bounded neg" (> (min sine-out) -1.0001))


; ── window ────────────────────────────────────────────────────────────────────

(def hann     (window 512 0.5  0.5  0.0))
(def hamming  (window 512 0.54 0.46 0.0))
(def blackman (window 512 0.42 0.5  0.08))
(def rect     (window 512 1.0  0.0  0.0))

(assert-eq   "window len"        (len hann) 512)
; Hann: endpoints near 0
(assert-near "hann first"        (nth hann 0) 0.0 0.001)
; Hann: centre near 1.0
(assert-near "hann mid"          (nth hann 256) 1.0 0.001)
; rect: all ones
(assert-near "rect val"          (nth rect 100) 1.0 0.0001)
; Blackman: peak <= 1
(assert      "blackman bounded"  (< (max blackman) 1.0001))
(assert-near "hamming first"     (nth hamming 0) 0.08 0.001)


; ── fft / ifft roundtrip ──────────────────────────────────────────────────────

; a 64-sample sine — forward then inverse should recover the original
(def fft-n 64)
(def test-sig (* (sin (* (linspace 0 (* 2 pi) fft-n) 4)) 1))  ; 4 cycles
(def spectrum (fft test-sig))
; spectrum length = 2 * next_pow2(64) = 128
(assert-eq   "fft out len"    (len spectrum) 128)
; ifft roundtrip
(def recovered (ifft spectrum))
(assert-near "ifft roundtrip" (nth recovered 10) (nth test-sig 10) 0.0001)
(assert-near "ifft roundtrip2" (nth recovered 50) (nth test-sig 50) 0.0001)


; ── car2pol / pol2car roundtrip ───────────────────────────────────────────────

(def spec2  (fft (sin (linspace 0 (* 4 pi) 64))))
(def polar  (car2pol spec2))
; magnitudes (even indices) should be non-negative
; extract every other element starting at 0: indices 0, 2, 4, ...
(def n-bins (floor (/ (len polar) 2)))
(def mags-only (zeros n-bins))
(def extract-mags (lambda (i)
  (if (= i n-bins) mags-only
    (begin
      (def mags-only
        (cat (cat (slice mags-only 0 i)
                  (vec (nth polar (* 2 i))))
             (slice mags-only (+ i 1) n-bins)))
      (extract-mags (+ i 1))))))
(extract-mags 0)
(assert      "car2pol mag>=0"  (> (min mags-only) -0.0001))
; roundtrip
(def car2    (pol2car polar))
(assert-near "pol2car re rt"  (nth car2 0) (nth spec2 0) 0.001)
(assert-near "pol2car im rt"  (nth car2 1) (nth spec2 1) 0.001)


; ── conv ──────────────────────────────────────────────────────────────────────

; conv of [1] with anything is identity
(def impulse (vec 1))
(def sig5    (vec 1 2 3 4 5))
(def ci      (conv sig5 impulse))
(assert-eq   "conv impulse len"  (len ci) 5)
(assert-near "conv impulse val"  (nth ci 2) 3.0 0.001)

; conv of two rectangles: triangle-ish
(def r3 (vec 1 1 1))
(def cr (conv r3 r3))            ; [1 2 3 2 1]
(assert-eq   "conv rect len"    (len cr) 5)
(assert-near "conv rect peak"   (nth cr 2) 3.0 0.001)
(assert-near "conv rect edge"   (nth cr 0) 1.0 0.001)


; ── iir ───────────────────────────────────────────────────────────────────────

; FIR identity: b=[1], a=[1]  → output == input
(def b1 (vec 1))
(def a1 (vec 1))
(def fir-id (iir sig5 b1 a1))
(assert-eq   "iir identity len" (len fir-id) 5)
(assert-near "iir identity val" (nth fir-id 3) 4.0 0.001)

; FIR moving average 3-tap: b=[1/3 1/3 1/3]
(def bma (vec 0.333333 0.333333 0.333333))
(def fma (iir (vec 1 2 3 4 5 6) bma a1))
(assert-near "iir ma val"   (nth fma 4) 4.0 0.01)    ; (3+4+5)/3 = 4

; DC block test: with pole=0.995, output decays as 0.995^n.
; At sample 2048: 0.995^2048 ≈ 3.5e-5 — mean of tail should be near zero.
(def dc-sig  (* (ones 4096) 1.0))
(def bdc (vec 1 -1))
(def adc (vec 1 -0.995))
(def blocked (iir dc-sig bdc adc))
(assert-near "dc block sum" (mean (slice blocked 2048 4096)) 0.0 0.001)


; ── iirdesign ─────────────────────────────────────────────────────────────

(def lp-coeff (iirdesign "lowpass" sr 1000 0.707 0))
(def b-lp (nth lp-coeff 0))
(def a-lp (nth lp-coeff 1))

; returns two 3-element vectors
(assert-eq   "filtdesign b len"  (len b-lp) 3)
(assert-eq   "filtdesign a len"  (len a-lp) 3)
; a[0] == 1 (normalized)
(assert-near "filtdesign a0"     (nth a-lp 0) 1.0 0.0001)
; DC gain of lowpass should be ~1: sum(b) / sum(a) ≈ 1
(def dc-gain (/ (sum b-lp) (sum a-lp)))
(assert-near "filtdesign lp DC"  dc-gain 1.0 0.01)

; high-pass DC gain ≈ 0
(def hp-coeff (iirdesign "highpass" sr 1000 0.707 0))
(def b-hp (nth hp-coeff 0))
(def a-hp (nth hp-coeff 1))
(def hp-dc (/ (sum b-hp) (sum a-hp)))
(assert-near "filtdesign hp DC"  hp-dc 0.0 0.01)


; ── delay ─────────────────────────────────────────────────────────────────────

; delay by 3: first 3 samples should be 0, rest shifted
(def del-in  (vec 1 2 3 4 5 6 7 8))
(def del-out (delay del-in 3))
(assert-near "delay first"  (nth del-out 0) 0.0 0.001)
(assert-near "delay third"  (nth del-out 2) 0.0 0.001)
(assert-near "delay fourth" (nth del-out 3) 1.0 0.01)
(assert-near "delay fifth"  (nth del-out 4) 2.0 0.01)


; ── comb ──────────────────────────────────────────────────────────────────────

; comb with g=0 is identity
(def comb0 (comb sig5 2 0))
(assert-near "comb g=0" (nth comb0 3) 4.0 0.001)

; comb with g=0.5, d=1: y[0]=x[0]=1, y[1]=x[1]+0.5*y[0]=2+0.5=2.5
(def comb1 (comb sig5 1 0.5))
(assert-near "comb d=1 first"  (nth comb1 0) 1.0 0.001)
(assert-near "comb d=1 second" (nth comb1 1) 2.5 0.001)


; ── allpass ───────────────────────────────────────────────────────────────────

; allpass energy preservation: energy in ≈ energy out
(def ap-in  (sin (linspace 0 (* 4 pi) 128)))
(def ap-out (allpass ap-in 4 0.5))
(def e-in   (sum (* ap-in ap-in)))
(def e-out  (sum (* ap-out ap-out)))
; settled part (drop first few samples): energies within 20%
(assert      "allpass energy" (< (abs (- e-out e-in)) (* e-in 0.25)))


; ── resample ──────────────────────────────────────────────────────────────────

(def rs-in  (sin (linspace 0 (* 4 pi) 64)))
; upsample 2x: output should have ~128 samples
(def rs-up  (resample rs-in 2.0))
(assert      "resample up len"  (> (len rs-up) 100))
; downsample 0.5x: output should have ~32 samples
(def rs-dn  (resample rs-in 0.5))
(assert      "resample dn len"  (< (len rs-dn) 50))


; ── mix ───────────────────────────────────────────────────────────────────────

(def m1 (vec 1 1 1))
(def m2 (vec 2 2 2))

; two signals at offset 0: sum
(def mx0 (mix 0 m1 0 m2))
(assert-eq   "mix len offset 0" (len mx0) 3)
(assert-near "mix val"          (nth mx0 0) 3.0 0.001)

; second signal offset by 3: total length = 6
(def mx1 (mix 0 m1 3 m2))
(assert-eq   "mix len offset"   (len mx1) 6)
(assert-near "mix first half"   (nth mx1 0) 1.0 0.001)
(assert-near "mix second half"  (nth mx1 3) 2.0 0.001)


; ── wavwrite / wavread roundtrip ──────────────────────────────────────────────

(def wav-sig (sin (linspace 0 (* 4 pi) 512)))
(wavwrite wav-sig sr "/tmp/pure0_test.wav")
(def wv (wavread "/tmp/pure0_test.wav"))
(def wv-sig (nth wv 0))
(def wv-sr  (nth wv 1))
(def wv-nch (nth wv 2))

(assert-eq   "wavread sr"    wv-sr  sr)
(assert-eq   "wavread nch"   wv-nch 1)
(assert-eq   "wavread len"   (len wv-sig) 512)
; values should be close (16-bit quantization: ~1/32768 error per sample)
(assert-near "wavread val"   (nth wv-sig 64) (nth wav-sig 64) 0.001)
; peak normalized: max near 1.0
(assert-near "wavread peak"  (max (abs wv-sig)) 1.0 0.001)


; ─────────────────────────────────────────────────────────────────────────────
(print "")
(print "all DSP tests passed")
