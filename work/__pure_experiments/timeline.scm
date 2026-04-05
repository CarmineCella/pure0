(render
  (+ (sig 0.3 15 (* amp (bpf 0 0  0.1 1  0.9 1  1 0 ph) (osc (* 110 t))))
     (delay (sig 0.3 20 (* amp (bpf 0 0  0.1 1  0.9 1  1 0 ph) (osc (* 180 t)))) 5))
  (+ (sig 0.3 15 (* amp (bpf 0 0  0.1 1  0.9 1  1 0 ph) (osc (* 115 t))))
     (delay (sig 0.3 20 (* amp (bpf 0 0  0.1 1  0.9 1  1 0 ph) (osc (* 180 t)))) 5)))