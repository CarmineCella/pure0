; dsp_example.scm — practical DSP examples for pure0
; run with: pure0 dsp_example.scm
;
; Produces WAV files in  you can open in any audio editor.

(load "stdlib.scm")

(def sr 44100)
(def pi2 (* 2 pi))

; ════════════════════════════════════════════════════════════════════════════
; 1. WAVETABLE SYNTHESIS
; ════════════════════════════════════════════════════════════════════════════
; gen builds a wavetable from harmonic amplitudes (gen10 algorithm).
; osc reads the table using linear interpolation.

; pure sine
(def sine-tab   (gen 4096 1))

; sawtooth approximation: harmonics 1/n
(def saw-tab    (gen 4096 1 0.5 0.333 0.25 0.2 0.166 0.142 0.125))

; square approximation: odd harmonics 1/n
(def square-tab (gen 4096 1 0 0.333 0 0.2 0 0.142 0 0.111))

; triangle: odd harmonics with alternating sign, 1/n^2
(def tri-tab    (gen 4096 1 0 -0.111 0 0.04 0 -0.02 0 0.012))

(def secs   (lambda (s) (floor (* s sr))))

; ── example 1a: pure sine, 440 Hz, 1 second ──────────────────────────────────

(def dur1 (secs 1))
(def freq-440 (* (ones dur1) 440))
(def sine-440 (osc sr freq-440 sine-tab))
(wavwrite sine-440 sr "sine_440.wav")
(print "wrote sine_440.wav")

; ── example 1b: sawtooth, 220 Hz, 1 second ───────────────────────────────────

(def freq-220 (* (ones dur1) 220))
(def saw-220  (osc sr freq-220 saw-tab))
(wavwrite saw-220 sr "saw_220.wav")
(print "wrote saw_220.wav")

; ── example 1c: frequency glide (portamento) 200 → 800 Hz over 2 seconds ────

(def dur2   (secs 2))
(def glide  (linspace 200 800 dur2))
(def glided (osc sr glide sine-tab))
(wavwrite glided sr "glide.wav")
(print "wrote glide.wav")

; ── example 1d: chord — three detuned sines mixed ────────────────────────────

(def note (lambda (freq dur)
  (osc sr (* (ones (secs dur)) freq) sine-tab)))

(def chord
  (mix 0 (note 261.63 1.5)   ; C4
       0 (note 329.63 1.5)   ; E4
       0 (note 392.00 1.5))) ; G4

(wavwrite (* chord 0.333) sr "chord_cmaj.wav")
(print "wrote chord_cmaj.wav")


; ════════════════════════════════════════════════════════════════════════════
; 2. AMPLITUDE ENVELOPE
; ════════════════════════════════════════════════════════════════════════════
; Multiply a signal by an amplitude curve to shape its dynamics.

; simple ADSR-like envelope via concatenation of linspaces
(def adsr (lambda (sr a d s r level)
  (def sa (floor (* a sr)))
  (def sd (floor (* d sr)))
  (def ss (floor (* s sr)))
  (def sr_ (floor (* r sr)))
  (cat (cat (linspace 0 1 sa)
            (linspace 1 level sd))
       (cat (* (ones ss) level)
            (linspace level 0 sr_)))))

(def env1 (adsr sr 0.01 0.1 0.8 0.3 0.5))
(def pad-sine (osc sr (* (ones (len env1)) 330) sine-tab))
(wavwrite (* pad-sine env1) sr "adsr_sine.wav")
(print "wrote adsr_sine.wav")


; ════════════════════════════════════════════════════════════════════════════
; 3. SUBTRACTIVE SYNTHESIS — OSC → FILTER
; ════════════════════════════════════════════════════════════════════════════

; sawtooth into a lowpass filter
(def raw-saw (osc sr (* (ones (secs 1)) 110) saw-tab))
(def lp-coef (iirdesign "lowpass" sr 800 1.5 0))
(def b-lp    (nth lp-coef 0))
(def a-lp    (nth lp-coef 1))
(def filtered-saw (iir raw-saw b-lp a-lp))
(wavwrite filtered-saw sr "saw_filtered.wav")
(print "wrote saw_filtered.wav")

; filter sweep: apply lowpass at several cutoffs and mix
(def filt-at (lambda (fc)
  (def coef (iirdesign "lowpass" sr fc 0.707 0))
  (iir raw-saw (nth coef 0) (nth coef 1))))

(def sweep-mix
  (mix 0         (filt-at 200)
       (secs 0.5) (filt-at 600)
       (secs 1.0) (filt-at 2000)
       (secs 1.5) (filt-at 8000)))

(wavwrite sweep-mix sr "filter_sweep.wav")
(print "wrote filter_sweep.wav")


; ════════════════════════════════════════════════════════════════════════════
; 4. CONVOLUTION REVERB
; ════════════════════════════════════════════════════════════════════════════
; Exponentially-decaying noise as a simple room impulse response.

(def ir-len (secs 0.4))
(def noise  (* (- (* (rand ir-len) 2) 1)         ; uniform noise in [-1,1]
               (exp (* (linspace 0 -6 ir-len) 1)))) ; exponential decay

; anechoic hit: impulse at sample 0
(def anechoic (zeros (secs 0.5)))
; actually a short click
(def click (cat (vec 1) (zeros (- (secs 0.5) 1))))

(def reverbed (conv click noise))
(wavwrite reverbed sr "conv_reverb.wav")
(print "wrote conv_reverb.wav")

; apply reverb to a saw note
(def dry-note (osc sr (cat (* (ones (secs 0.3)) 220)
                            (zeros (secs 0.7))) saw-tab))
(def wet-note (conv dry-note noise))
(def wet-trim (slice wet-note 0 (secs 1.5)))
(wavwrite wet-trim sr "saw_reverb.wav")
(print "wrote saw_reverb.wav")


; ════════════════════════════════════════════════════════════════════════════
; 5. DELAY EFFECTS
; ════════════════════════════════════════════════════════════════════════════

; slapback echo: mix dry + delayed copy
(def dry     (osc sr (cat (* (ones (secs 0.3)) 440) (zeros (secs 0.5))) sine-tab))
(def echoed  (mix 0 dry  (floor (* 0.15 sr)) (* dry 0.6)))
(wavwrite echoed sr "slapback.wav")
(print "wrote slapback.wav")

; comb filter (metallic resonance): feedback at 100 samples
(def combed (comb raw-saw 100 0.7))
(wavwrite combed sr "comb_filter.wav")
(print "wrote comb_filter.wav")

; Schroeder allpass chain (diffusor)
(def ap1 (allpass raw-saw 347 0.5))
(def ap2 (allpass ap1     113 0.5))
(def ap3 (allpass ap2      37 0.5))
(wavwrite ap3 sr "allpass_chain.wav")
(print "wrote allpass_chain.wav")


; ════════════════════════════════════════════════════════════════════════════
; 6. RESONATOR (modal synthesis)
; ════════════════════════════════════════════════════════════════════════════
; Excite a second-order resonator with a short impulse.
; tau controls decay time, freq sets the resonant frequency.

(def excite (cat (vec 1) (zeros 2000)))

(def mode1 (reson excite sr 440 0.5))
(def mode2 (reson excite sr 880 0.3))
(def mode3 (reson excite sr 1320 0.2))

; mix the three modes (pad shorter ones to the same length)
(def modal-len (max (vec (len mode1) (len mode2) (len mode3))))
(def pad-to (lambda (sig n)
  (cat sig (zeros (- n (len sig))))))

(def bell
  (+ (pad-to mode1 (floor modal-len))
     (* (pad-to mode2 (floor modal-len)) 0.5)
     (* (pad-to mode3 (floor modal-len)) 0.3)))

(wavwrite bell sr "bell.wav")
(print "wrote bell.wav")


; ════════════════════════════════════════════════════════════════════════════
; 7. SPECTRAL PROCESSING (FFT domain)
; ════════════════════════════════════════════════════════════════════════════

; Frequency-domain lowpass: zero the upper half of the spectrum,
; then IFFT back. This is an ideal brickwall lowpass at Nyquist/2.

(def frame-size 512)
(def sp-signal (osc sr (* (ones frame-size) 440) saw-tab))

; Hann window before FFT to reduce spectral leakage
(def sp-win    (window frame-size 0.5 0.5 0.0))
(def sp-spec   (fft (* sp-signal sp-win)))       ; interleaved complex

; zero upper half: keep bins 0..N/2, silence N/2+1..N
(def sp-total  (len sp-spec))
(def sp-half   (floor (/ sp-total 2)))
(def sp-lp     (cat (slice sp-spec 0 sp-half) (zeros sp-half)))

(def sp-out    (ifft sp-lp))
(wavwrite (* (slice sp-out 0 frame-size) sp-win) sr "spectral_lp.wav")
(print "wrote spectral_lp.wav")

; Spectral round-trip sanity: FFT → IFFT should recover the windowed signal
(def sp-rt     (ifft sp-spec))
(wavwrite (slice sp-rt 0 frame-size) sr "spectral_roundtrip.wav")
(print "wrote spectral_roundtrip.wav")


; ════════════════════════════════════════════════════════════════════════════
; 8. RESAMPLE (pitch shifting without time change)
; ════════════════════════════════════════════════════════════════════════════

(def src     (osc sr (* (ones (secs 0.5)) 330) sine-tab))
; pitch up one octave: resample at 2x then take first half
(def up2x    (resample src 2.0))
(def pitched (slice up2x 0 (len src)))
(wavwrite pitched sr "pitched_up.wav")
(print "wrote pitched_up.wav")

; pitch down one octave
(def dn2x     (resample src 0.5))
; now it's half as long — pad with silence to original length
(def pitched-dn (cat dn2x (zeros (- (len src) (len dn2x)))))
(wavwrite pitched-dn sr "pitched_down.wav")
(print "wrote pitched_down.wav")


; ════════════════════════════════════════════════════════════════════════════
; 9. STEREO — interleaved L/R
; ════════════════════════════════════════════════════════════════════════════
; wavwrite with nch=2 expects interleaved samples [L0 R0 L1 R1 ...]

(def left  (osc sr (* (ones dur1) 330) sine-tab))
(def right (osc sr (* (ones dur1) 440) sine-tab))

; interleave manually
(def stereo-len (* (len left) 2))
(def interleave-lr (lambda (l r)
  (def n (len l))
  (def buf (zeros (* n 2)))
  (def go (lambda (i buf)
    (if (= i n) buf
      (go (+ i 1)
          (cat (cat (slice buf 0 (* 2 i))
                    (vec (nth l i) (nth r i)))
               (slice buf (+ (* 2 i) 2) (len buf)))))))
  (go 0 buf)))

; Note: for large signals the recursive interleave above is slow.
; Using cat-based approach for the demo:
(def stereo (interleave-lr (* left 0.7) (* right 0.7)))
(wavwrite stereo sr "stereo_ping.wav" 2)
(print "wrote stereo_ping.wav")


; ════════════════════════════════════════════════════════════════════════════
; done
; ════════════════════════════════════════════════════════════════════════════

(print "")
(print "all examples written to wav files ")
