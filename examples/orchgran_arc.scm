
; ============================================================================
; inst-gran_arc.scm
; example use of orchestral granulator
;
; form:
;   1) start: low register (oct. 3-4), long grains, very sparse, ppp,
;      harmony mixing C minor and F# minor
;   2) middle: density increases, grains shorten, register rises toward oct. 5,
;      dynamics move toward fff
;   3) end: returns to the initial situation
;
;
; ============================================================================

(def sr 48000)

; total duration in seconds (~6 minutes here; change if you want)
(def dur 40)

(def SOL (loaddb "/Users/n4/Projects/Media/Datasets/TinySOL"))

; ----------------------------------------------------------------------------
; orchestra
; nested groups mean synchronized instruments
; ----------------------------------------------------------------------------

(def orchestra
  (list
    (list "Ob" "Fl")
    (list "ClBb" "Bn")
    (list "Vn" "Va")
    (list "Vc" "Cb")
    (list "Hn" "Tbn")
    (list "TpC" "BTb")))

; ----------------------------------------------------------------------------
; parameter curves
; scalar curves use entries: (time value)
; octave curve uses entries: (time low high)
; ----------------------------------------------------------------------------

; sparse -> dense -> sparse
(def density_curve
  (list
    (list 0   0.18)
    (list 6  0.35)
    (list 12 0.80)
    (list 18 1.70)
    (list 24 4.10)
    (list 30 8.40)
    (list 36 10.18)))

; relative randomization of density
(def rand_density_curve
  (list
    (list 0   0.10)
    (list 12 0.25)
    (list 18 0.35)
    (list 30 0.18)
    (list 36 0.10)))

; long -> short -> long
(def length_curve
  (list
    (list 0   4.0)
    (list 6  3 0)
    (list 12 1.5)
    (list 18 1)
    (list 24 .6)
    (list 30 .5)
    (list 36 .2)))

(def rand_length_curve
  (list
    (list 0   0.15)
    (list 120 0.25)
    (list 180 0.40)
    (list 300 0.20)
    (list 360 0.15)))

; register 3-4 -> toward 5 -> back to 3-4
(def octave_range_curve
  (list
    (list 0 3 3)
    (list 6  3 4)
    (list 12 4 4)
    (list 18 5 6)
    (list 24 6 6)))

(def rand_octave_curve
  (list
    (list 0   0)
    (list 12 1)
    (list 18 1)
    (list 30 0)
    (list 36 0)))
 
; ----------------------------------------------------------------------------
; harmonic schedule
; first: mixed C minor + F# minor
; middle: same aggregate but brighter placement due to octave curve
; return: same opening collection
;
; The granulator chooses pitch classes from the active chord and places them in
; the evolving octave range.
; ----------------------------------------------------------------------------

(def chord_schedule
  (list
    ; C minor + F# minor aggregate:
    ; C Eb G  +  F# A C#
    (list 0   (vec 48 51 55 54 57 61))
    (list 9  (vec 48 51 55 54 57 61 63))
    (list 18  (vec 48 51 54 55 57 61 63 67))
    (list 27  (vec 48 51 55 54 57 61))
    (list 36  (vec 48 51 55 54 57 61))))

; ----------------------------------------------------------------------------
; style groups over time
; keep mostly ordinario, introduce some variety in the dense middle
; ----------------------------------------------------------------------------

(def style_schedule
  (list
    (list 0   (list "ord"))))

; ----------------------------------------------------------------------------
; dynamic groups over time
; ppp -> fff -> ppp
; ----------------------------------------------------------------------------

(def dynamic_schedule
  (list
    (list 0   (list "ppp"))
    (list 18 (list "mf" "f" "ff" "fff"))
    (list 36 (list "ppp"))))

; ----------------------------------------------------------------------------
; render
; ----------------------------------------------------------------------------

(def result
  (inst-gran sr dur orchestra
            density_curve rand_density_curve
            length_curve rand_length_curve
            octave_range_curve rand_octave_curve
            chord_schedule style_schedule dynamic_schedule
            SOL))

(def sig    (inst-gran-signal result))
(def events (inst-gran-events result))

(wavwrite sig sr "inst-gran_arc.wav")
(inst-gran-saveevents events "inst-gran_arc_events.scm")
