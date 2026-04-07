; dsp_example.scm — practical DSP examples for pure0
(load "stdlib.scm")

(def sr 44100)

(def sine-tab   (gen 4096 1))
(def saw-tab    (gen 4096 1 0.5 0.333 0.25 0.2 0.166 0.142 0.125))
(def square-tab (gen 4096 1 0 0.333 0 0.2 0 0.142 0 0.111))
(def tri-tab    (gen 4096 1 0 -0.111 0 0.04 0 -0.02 0 0.012))

(def secs   (lambda (s) (floor (* s sr))))

(def dur1 (secs 1))
(def freq-440 (* (ones dur1) 440))
(def sine-440 (osc sr freq-440 sine-tab))
(wavwrite sine-440 sr "sine_440.wav")
(print "wrote sine_440.wav")

(def freq-220 (* (ones dur1) 220))
(def saw-220  (osc sr freq-220 saw-tab))
(wavwrite saw-220 sr "saw_220.wav")
(print "wrote saw_220.wav")

(def dur2   (secs 2))
(def glide  (linspace 200 800 dur2))
(def glided (osc sr glide sine-tab))
(wavwrite glided sr "glide.wav")
(print "wrote glide.wav")

(def note (lambda (freq dur)
  (osc sr (* (ones (secs dur)) freq) sine-tab)))

(def chord
  (mix 0 (note 261.63 1.5)
       0 (note 329.63 1.5)
       0 (note 392.00 1.5)))

(wavwrite (* chord 0.333) sr "chord_cmaj.wav")
(print "wrote chord_cmaj.wav")

(def adsr (lambda (sr a d s r level)
  (def sa (floor (* a sr)))
  (def sd (floor (* d sr)))
  (def ss (floor (* s sr)))
  (def sr_ (floor (* r sr)))
  (append (append (linspace 0 1 sa)
                  (linspace 1 level sd))
          (append (* (ones ss) level)
                  (linspace level 0 sr_)))))

(def env1 (adsr sr 0.01 0.1 0.8 0.3 0.5))
(def pad-sine (osc sr (* (ones (len env1)) 330) sine-tab))
(wavwrite (* pad-sine env1) sr "adsr_sine.wav")
(print "wrote adsr_sine.wav")

(def raw-saw (osc sr (* (ones (secs 1)) 110) saw-tab))
(def lp-coef (iirdesign "lowpass" sr 800 1.5 0))
(def b-lp    (nth lp-coef 0))
(def a-lp    (nth lp-coef 1))
(def filtered-saw (iir raw-saw b-lp a-lp))
(wavwrite filtered-saw sr "saw_filtered.wav")
(print "wrote saw_filtered.wav")

(def filt-at (lambda (fc)
  (def coef (iirdesign "lowpass" sr fc 0.707 0))
  (iir raw-saw (nth coef 0) (nth coef 1))))

(def sweep-mix
  (mix 0          (filt-at 200)
       (secs 0.5) (filt-at 600)
       (secs 1.0) (filt-at 2000)
       (secs 1.5) (filt-at 8000)))

(wavwrite sweep-mix sr "filter_sweep.wav")
(print "wrote filter_sweep.wav")

(def ir-len (secs 0.4))
(def noise  (* (- (* (rand ir-len) 2) 1)
               (exp (linspace 0 -6 ir-len))))

(def click (append (vec 1) (zeros (- (secs 0.5) 1))))

(def reverbed (conv click noise))
(wavwrite reverbed sr "conv_reverb.wav")
(print "wrote conv_reverb.wav")

(def dry-note (osc sr (append (* (ones (secs 0.3)) 220)
                              (zeros (secs 0.7))) saw-tab))
(def wet-note (conv dry-note noise))
(def wet-trim (slice wet-note 0 (secs 1.5)))
(wavwrite wet-trim sr "saw_reverb.wav")
(print "wrote saw_reverb.wav")

(def dry     (osc sr (append (* (ones (secs 0.3)) 440) (zeros (secs 0.5))) sine-tab))
(def echoed  (mix 0 dry  (floor (* 0.15 sr)) (* dry 0.6)))
(wavwrite echoed sr "slapback.wav")
(print "wrote slapback.wav")

(def combed (comb raw-saw 100 0.7))
(wavwrite combed sr "comb_filter.wav")
(print "wrote comb_filter.wav")

(def ap1 (allpass raw-saw 347 0.5))
(def ap2 (allpass ap1     113 0.5))
(def ap3 (allpass ap2      37 0.5))
(wavwrite ap3 sr "allpass_chain.wav")
(print "wrote allpass_chain.wav")

(def excite (append (vec 1) (zeros 2000)))

(def mode1 (reson excite sr 440 0.5))
(def mode2 (reson excite sr 880 0.3))
(def mode3 (reson excite sr 1320 0.2))

(def modal-len (max (vec (len mode1) (len mode2) (len mode3))))
(def pad-to (lambda (sig n)
  (append sig (zeros (- n (len sig))))))

(def bell
  (+ (pad-to mode1 (floor modal-len))
     (* (pad-to mode2 (floor modal-len)) 0.5)
     (* (pad-to mode3 (floor modal-len)) 0.3)))

(wavwrite bell sr "bell.wav")
(print "wrote bell.wav")

(def frame-size 512)
(def sp-signal (osc sr (* (ones frame-size) 440) saw-tab))
(def sp-win    (window frame-size 0.5 0.5 0.0))
(def sp-spec   (fft (* sp-signal sp-win)))

(def sp-total  (len sp-spec))
(def sp-half   (floor (/ sp-total 2)))
(def sp-lp     (append (slice sp-spec 0 sp-half) (zeros sp-half)))

(def sp-out    (ifft sp-lp))
(wavwrite (* (slice sp-out 0 frame-size) sp-win) sr "spectral_lp.wav")
(print "wrote spectral_lp.wav")

(def sp-rt     (ifft sp-spec))
(wavwrite (slice sp-rt 0 frame-size) sr "spectral_roundtrip.wav")
(print "wrote spectral_roundtrip.wav")

; deinterleave/interleave examples
(def test-stereo (interleave sine-440 saw-220))
(wavwrite test-stereo sr "stereo_test.wav" 2)
(print "wrote stereo_test.wav")
(def left-back  (deinterleave test-stereo 2 0))
(def right-back (deinterleave test-stereo 2 1))
(wavwrite left-back sr "stereo_test_left.wav")
(wavwrite right-back sr "stereo_test_right.wav")
(print "wrote stereo_test_left.wav and stereo_test_right.wav")

(print "")
(print "done.")
