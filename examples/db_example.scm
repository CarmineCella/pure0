

(def SOL (loaddb "/Users/n4/Projects/Media/Datasets/TinySOL"))

(print (catalog-stats SOL))
(print (catalog-instruments SOL))
(def ob (catalog-pick SOL "Ob" "ord" "A#3" "ff"))
(def sig (catalog-load ob 48000))

(wavwrite sig 48000 "test_ob.wav")