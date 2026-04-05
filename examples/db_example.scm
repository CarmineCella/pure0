

(def SOL (loaddb "/Users/n4/Projects/Media/Datasets/TinySOL"))

(print (db-stats SOL))
(print (db-instruments SOL))
(def ob (db-pick SOL "Ob" "ord" "A#3" "ff"))
(def sig (db-load ob 48000))

(wavwrite sig 48000 "test_ob.wav")