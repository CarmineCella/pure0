(load "stdlib.scm")

(def sig1-data (wavread "../data/anechoic1.wav"))
(def sig2-data (wavread "../data/Concertgebouw-s.wav"))

(def sig1 (head sig1-data))
(def sr1  (second sig1-data))
(def ch1  (third sig1-data))

(def sig2 (head sig2-data))
(def sr2  (second sig2-data))
(def ch2  (third sig2-data))

(if (not (= sr1 sr2))
    (error "signal1 and signal2 must have the same sample rate")
    0)

(if (not (= ch1 1))
    (if (not (= ch1 2))
        (error "signal1 must be mono or stereo")
        0)
    0)

(if (not (= ch2 1))
    (if (not (= ch2 2))
        (error "signal2 must be mono or stereo")
        0)
    0)

(def s1L (if (= ch1 1) sig1 (deinterleave sig1 ch1 0)))
(def s1R (if (= ch1 1) sig1 (deinterleave sig1 ch1 1)))

(def s2L (if (= ch2 1) sig2 (deinterleave sig2 ch2 0)))
(def s2R (if (= ch2 1) sig2 (deinterleave sig2 ch2 1)))

(if (= ch1 1)
    (if (= ch2 1)
        (begin
          (def wet (normalize (conv s1L s2L)))
          (wavwrite wet sr1 "reverb.wav"))
        (begin
          (def wetL (normalize (conv s1L s2L)))
          (def wetR (normalize (conv s1L s2R)))
          (wavwrite (interleave wetL wetR) sr1 "reverb.wav" 2)))
    (if (= ch2 1)
        (begin
          (def wetL (normalize (conv s1L s2L)))
          (def wetR (normalize (conv s1R s2L)))
          (wavwrite (interleave wetL wetR) sr1 "reverb.wav" 2))
        (begin
          (def wetL (normalize (conv s1L s2L)))
          (def wetR (normalize (conv s1R s2R)))
          (wavwrite (interleave wetL wetR) sr1 "reverb.wav" 2))))
