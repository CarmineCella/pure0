; filters.scm
(load "dsp.scm")

(def wavinfo (wavread "../data/cage.wav"))
(def sr      (second wavinfo))
(def w       (deinterleave (head wavinfo) (third wavinfo) 0))

(print "lowpass\n")

(def cutoff-lp 200)
(def q-lp      0.707)
(def w-lp      (lowpass w sr cutoff-lp q-lp))
(wavwrite w-lp sr "lp.wav")

(print "bandpass (highpass + lowpass cascade)\n")

(def cutoff-hp 2000)
(def cutoff-bp 2500)
(def q-bp      0.707)

(def w-hp (highpass w sr cutoff-hp q-bp))
(def w-bp (lowpass w-hp sr cutoff-bp q-bp))
(wavwrite w-bp sr "bp.wav")
