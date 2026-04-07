; db_example.scm — examples for the reduced sample database layer
(load "stdlib.scm")
(load "sampsynth.scm")

(def db (db-load "/Users/n4/Projects/Media/Datasets/StaticSOL"))

(print "number of entries:")
(print (len db))
(print "")

(print "all instruments:")
(print (db-instruments db))
(print "")

(print "all playing styles:")
(print (db-playingstyles db))
(print "")

(print "all dynamics:")
(print (db-dynamics db))
(print "")

(print "all pitches:")
(print (db-pitches db))
(print "")

(print "all oboe entries:")
(def q-ob (db-query db "Ob"))
(print (len q-ob))
(print "")

(print "all oboe ordinario entries:")
(def q-ob-ord (db-query db "Ob.*ord"))
(print (len q-ob-ord))
(print "")

(print "all ff entries:")
(def q-ff (db-query db "ff"))
(print (len q-ff))
(print "")

(print "all pizzicato violin entries:")
(def q-vn-pizz (db-query db "Vn.*pizz"))
(print (len q-vn-pizz))
(print "")

(print "all oboe entries at A#3:")
(def q-ob-asharp3 (db-query db "Ob.*A#3"))
(print q-ob-asharp3)
(print "")

(print "all clarinet OR bassoon staccato entries:")
(def q-reed-stacc (db-query db "(Cl|Bn).*stacc"))
(print (len q-reed-stacc))
(print "")

(print "all entries matching instrument + style + dynamic:")
(def q-combo (db-query db "Ob.*ord.*ff"))
(print q-combo)
(print "")

(print "instruments in the ff subset:")
(print (db-instruments-query db "ff"))
(print "")

(print "playing styles available for oboe:")
(print (db-playingstyles-query db "Ob"))
(print "")

(print "dynamics available for violin pizzicato:")
(print (db-dynamics-query db "Vn.*pizz"))
(print "")

(print "pitches available for clarinet staccato:")
(print (db-pitches-query db "Cl.*stacc"))
(print "")

(if (> (len q-ob-ord) 0)
    (begin
        (def xs (db-query db "Ob.*A4.*ff"))
        (def sig (db-pick (head xs) 44100))
        (print "loaded one oboe ordinario sample, length:")
        (print (len sig))
        (wavwrite sig 44100 "db_pick_example.wav"))
    (print "no oboe ordinario sample found"))
