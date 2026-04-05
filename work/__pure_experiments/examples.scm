(load "stdlib_ex.scm")

(render "outa.wav"
  (sig 0.2 2 (tone<> amp t ph 110)))

(render "outb.wav"
  (sig 0.2 2 (tone<> amp t ph 440))
  (sig 0.2 2 (tone<> amp t ph 660)))

(render "outc.wav"
  (+ (sig 0.3 15 (tone<> amp t ph 110))
     (delay (sig 0.3 20 (tone<> amp t ph 180)) 5))
  (+ (sig 0.3 15 (tone<> amp t ph 115))
     (delay (sig 0.3 20 (tone<> amp t ph 180)) 5)))

  (render "out4.wav"
    (sig 0.2 2
      (begin
        (def T (tone<> amp t ph))
        (+ (T 440)
           (T 550)
           (T 660)))))