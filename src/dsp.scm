; dsp.scm — small convenience wrappers built on top of dsp.h
(load "stdlib.scm")

(def lowpass
  (lambda (sig sr cutoff q)
    (def coeffs (iirdesign "lowpass" sr cutoff q 0))
    (iir sig (nth coeffs 0) (nth coeffs 1))))

(def highpass
  (lambda (sig sr cutoff q)
    (def coeffs (iirdesign "highpass" sr cutoff q 0))
    (iir sig (nth coeffs 0) (nth coeffs 1))))
