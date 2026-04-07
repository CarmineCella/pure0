; test_dsp.scm — test suite for dsp.h primitives
(load "stdlib.scm")

(def assert (lambda (name cond)
  (if cond
    (print "PASS" name)
    (error (join (list "FAIL:" name) " ")))))

(def assert-eq (lambda (name got expected)
  (assert name (= got expected))))

(def assert-near (lambda (name got expected eps)
  (assert name (< (abs (- got expected)) eps))))

(def sine-table (gen 512 1))
(def harm-table (gen 512 1 0.5 0.25))

(assert-eq   "gen len"         (len sine-table) 513)
(assert-near "gen guard"       (nth sine-table 512) (nth sine-table 0) 0.0001)
(assert-near "gen sine peak"   (nth sine-table 128) 1.0 0.01)
(assert      "gen amp <=1"     (< (max sine-table) 1.0001))
(assert-eq   "gen multi len"   (len harm-table) 513)

(def sr 44100)
(def dur-samps 4410)
(def freq-vec (* (ones dur-samps) 440))
(def sine-out (osc sr freq-vec sine-table))

(assert-eq   "osc len"     (len sine-out) dur-samps)
(assert-near "osc mean"    (mean sine-out) 0.0 0.01)
(assert      "osc bounded" (< (max sine-out) 1.0001))
(assert      "osc bounded neg" (> (min sine-out) -1.0001))

(def hann     (window 512 0.5  0.5  0.0))
(def hamming  (window 512 0.54 0.46 0.0))
(def blackman (window 512 0.42 0.5  0.08))
(def rect     (window 512 1.0  0.0  0.0))

(assert-eq   "window len"        (len hann) 512)
(assert-near "hann first"        (nth hann 0) 0.0 0.001)
(assert-near "hann mid"          (nth hann 256) 1.0 0.01)
(assert-near "rect val"          (nth rect 100) 1.0 0.0001)
(assert      "blackman bounded"  (< (max blackman) 1.0001))
(assert-near "hamming first"     (nth hamming 0) 0.08 0.001)

(def fft-n 64)
(def test-sig (* (sin (* (linspace 0 (* 2 pi) fft-n) 4)) 1))
(def spectrum (fft test-sig))
(assert-eq   "fft out len"    (len spectrum) 128)
(def recovered (ifft spectrum))
(assert-near "ifft roundtrip"  (nth recovered 10) (nth test-sig 10) 0.0001)
(assert-near "ifft roundtrip2" (nth recovered 50) (nth test-sig 50) 0.0001)

(def spec2  (fft (sin (linspace 0 (* 4 pi) 64))))
(def polar  (car2pol spec2))
(def mags   (deinterleave polar 2 0))
(def phases (deinterleave polar 2 1))
(assert      "car2pol mag>=0"  (> (min mags) -0.0001))
(assert-eq   "car2pol phase len" (len phases) (len mags))
(def car2    (pol2car polar))
(assert-near "pol2car re rt"  (nth car2 0) (nth spec2 0) 0.001)
(assert-near "pol2car im rt"  (nth car2 1) (nth spec2 1) 0.001)

(def ri      (interleave mags phases))
(assert-eq   "interleave len" (len ri) (* 2 (len mags)))
(assert-near "interleave rt0" (nth ri 0) (nth mags 0) 0.0001)
(assert-near "interleave rt1" (nth ri 1) (nth phases 0) 0.0001)

(def impulse (vec 1))
(def sig5    (vec 1 2 3 4 5))
(def ci      (conv sig5 impulse))
(assert-eq   "conv impulse len"  (len ci) 5)
(assert-near "conv impulse val"  (nth ci 2) 3.0 0.001)

(def r3 (vec 1 1 1))
(def cr (conv r3 r3))
(assert-eq   "conv rect len"    (len cr) 5)
(assert-near "conv rect peak"   (nth cr 2) 3.0 0.001)
(assert-near "conv rect edge"   (nth cr 0) 1.0 0.001)

(def b1 (vec 1))
(def a1 (vec 1))
(def fir-id (iir sig5 b1 a1))
(assert-eq   "iir identity len" (len fir-id) 5)
(assert-near "iir identity val" (nth fir-id 3) 4.0 0.001)

(def bma (vec 0.333333 0.333333 0.333333))
(def fma (iir (vec 1 2 3 4 5 6) bma a1))
(assert-near "iir ma val"   (nth fma 4) 4.0 0.01)

(def dc-sig  (* (ones 4096) 1.0))
(def bdc (vec 1 -1))
(def adc (vec 1 -0.995))
(def blocked (iir dc-sig bdc adc))
(assert-near "dc block sum" (mean (slice blocked 2048 4096)) 0.0 0.001)

(def lp-coeff (iirdesign "lowpass" sr 1000 0.707 0))
(def b-lp (nth lp-coeff 0))
(def a-lp (nth lp-coeff 1))
(assert-eq   "filtdesign b len"  (len b-lp) 3)
(assert-eq   "filtdesign a len"  (len a-lp) 3)
(assert-near "filtdesign a0"     (nth a-lp 0) 1.0 0.0001)
(def dc-gain (/ (sum b-lp) (sum a-lp)))
(assert-near "filtdesign lp DC"  dc-gain 1.0 0.01)

(def hp-coeff (iirdesign "highpass" sr 1000 0.707 0))
(def b-hp (nth hp-coeff 0))
(def a-hp (nth hp-coeff 1))
(def hp-dc (/ (sum b-hp) (sum a-hp)))
(assert-near "filtdesign hp DC"  hp-dc 0.0 0.01)

(def del-in  (vec 1 2 3 4 5 6 7 8))
(def del-out (delay del-in 3))
(assert-near "delay first"  (nth del-out 0) 0.0 0.001)
(assert-near "delay third"  (nth del-out 2) 0.0 0.001)
(assert-near "delay fourth" (nth del-out 3) 1.0 0.01)
(assert-near "delay fifth"  (nth del-out 4) 2.0 0.01)

(def comb0 (comb sig5 2 0))
(assert-near "comb g=0" (nth comb0 3) 4.0 0.001)

(def comb1 (comb sig5 1 0.5))
(assert-near "comb d=1 first"  (nth comb1 0) 1.0 0.001)
(assert-near "comb d=1 second" (nth comb1 1) 2.5 0.001)

(def ap-in  (sin (linspace 0 (* 4 pi) 128)))
(def ap-out (allpass ap-in 4 0.5))
(def e-in   (sum (* ap-in ap-in)))
(def e-out  (sum (* ap-out ap-out)))
(assert      "allpass energy" (< (abs (- e-out e-in)) (* e-in 0.25)))

(def rs-in  (sin (linspace 0 (* 4 pi) 64)))
(def rs-up  (resample rs-in 2.0))
(assert      "resample up len"  (> (len rs-up) 100))
(def rs-dn  (resample rs-in 0.5))
(assert      "resample dn len"  (< (len rs-dn) 50))

(def m1 (vec 1 1 1))
(def m2 (vec 2 2 2))
(def mx0 (mix 0 m1 0 m2))
(assert-eq   "mix len offset 0" (len mx0) 3)
(assert-near "mix val"          (nth mx0 0) 3.0 0.001)
(def mx1 (mix 0 m1 3 m2))
(assert-eq   "mix len offset"   (len mx1) 6)
(assert-near "mix first half"   (nth mx1 0) 1.0 0.001)
(assert-near "mix second half"  (nth mx1 3) 2.0 0.001)

; deinterleave / interleave on stereo-like data
(def stereo-like (interleave (vec 1 2 3) (vec 10 20 30)))
(assert-eq   "deinterleave left len" (len (deinterleave stereo-like 2 0)) 3)
(assert-near "deinterleave left v1" (nth (deinterleave stereo-like 2 0) 1) 2.0 0.001)
(assert-near "deinterleave right v1" (nth (deinterleave stereo-like 2 1) 2) 30.0 0.001)

; oscbank
(def amp1 (* (ones 128) 0.5))
(def amp2 (* (ones 128) 0.25))
(def frq1 (* (ones 128) 220))
(def frq2 (* (ones 128) 330))
(def ob (oscbank sr (list amp1 amp2) (list frq1 frq2) sine-table))
(assert-eq "oscbank len" (len ob) 128)

; stft / istft
(def stft-in (* (sin (linspace 0 (* 8 pi) 2048)) 0.5))
(def specs (stft stft-in 256 64))
(assert "stft nonempty" (> (len specs) 0))
(def ist (istft specs 256 64))
(assert "istft len > 0" (> (len ist) 0))
(assert-near "istft sample" (nth ist 500) (nth stft-in 500) 0.1)

(def wav-sig (sin (linspace 0 (* 4 pi) 512)))
(wavwrite wav-sig sr "/tmp/pure0_test.wav")
(def wv (wavread "/tmp/pure0_test.wav"))
(def wv-sig (nth wv 0))
(def wv-sr  (nth wv 1))
(def wv-nch (nth wv 2))

(assert-eq   "wavread sr"    wv-sr  sr)
(assert-eq   "wavread nch"   wv-nch 1)
(assert-eq   "wavread len"   (len wv-sig) 512)
(assert-near "wavread val"   (nth wv-sig 64) (nth wav-sig 64) 0.001)
(assert-near "wavread peak"  (max (abs wv-sig)) 1.0 0.001)

(print "")
(print "all DSP tests passed")
