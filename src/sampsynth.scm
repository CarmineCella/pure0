; sampsynth.scm — light wrappers for the reduced sampsynth layer
(load "stdlib.scm")

(def db-instruments-query
  (lambda (db regex)
    (db-instruments (db-query db regex))))

(def db-playingstyles-query
  (lambda (db regex)
    (db-playingstyles (db-query db regex))))

(def db-dynamics-query
  (lambda (db regex)
    (db-dynamics (db-query db regex))))

(def db-pitches-query
  (lambda (db regex)
    (db-pitches (db-query db regex))))
