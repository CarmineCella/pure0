; reconstruction.scm
(load "stdlib.scm")

(def dur     1.2)
(def samps   (floor (* dur 44100)))
(def nwin    4096)
(def offset  128)
(def tab1    (gen nwin 1))

(print "analysing......")

(def wavinfo (wavread "../data/gong_c_sharp.wav"))
(def raw     (head wavinfo))
(def sr      (second wavinfo))
(def nch     (third wavinfo))

(def input
  (if (= nch 1)
      raw
      (deinterleave raw nch 0)))

(def samps   (floor (* dur sr)))

(def spec      (fft input))
(def polar     (car2pol spec))
(def mag0      (deinterleave polar 2 0))
(def nfft      (len mag0))
(def stop      (min (vec nwin nfft)))
(def mag       (slice mag0 offset stop))
(def fftfreqs0 (linspace 0 sr nfft))
(def fftfreqs  (slice fftfreqs0 offset (+ offset (len mag))))

(def build-envs
  (lambda (i amps freqs)
    (if (>= i (len mag))
        (list (reverse amps) (reverse freqs))
        (begin
          (def v    (nth mag i))
          (def vsc  (* v (/ 2 nfft)))
          (def enva (* (ones samps) vsc))
          (def envf (* (ones samps) (nth fftfreqs i)))
          (build-envs (+ i 1) (cons enva amps) (cons envf freqs))))))

(def envs  (build-envs 0 '() '()))
(def amps  (head envs))
(def freqs (second envs))

(print "done\nsynthesising...")

(def out       (oscbank sr amps freqs tab1))
(def fade      (linspace 0.8 0.0 samps))
(def out-faded (* out fade))

(print "done\n")
(wavwrite out-faded sr "reconstructed.wav")