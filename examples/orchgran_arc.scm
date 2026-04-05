
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
(def dur 360)

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
    (list 60  0.35)
    (list 120 0.80)
    (list 180 1.70)
    (list 240 1.10)
    (list 300 0.40)
    (list 360 0.18)))

; relative randomization of density
(def rand_density_curve
  (list
    (list 0   0.10)
    (list 120 0.25)
    (list 180 0.35)
    (list 300 0.18)
    (list 360 0.10)))

; long -> short -> long
(def length_curve
  (list
    (list 0   9.0)
    (list 60  7.0)
    (list 120 4.5)
    (list 180 1.2)
    (list 240 2.6)
    (list 300 6.5)
    (list 360 9.0)))

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
    (list 0   3 4)
    (list 60  3 4)
    (list 120 4 4)
    (list 180 4 5)
    (list 240 4 5)
    (list 300 3 4)
    (list 360 3 4)))

(def rand_octave_curve
  (list
    (list 0   0)
    (list 120 1)
    (list 180 1)
    (list 300 0)
    (list 360 0)))

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
    (list 90  (vec 48 51 55 54 57 61 63))
    (list 180 (vec 48 51 54 55 57 61 63 67))
    (list 270 (vec 48 51 55 54 57 61))
    (list 360 (vec 48 51 55 54 57 61))))

; ----------------------------------------------------------------------------
; style groups over time
; keep mostly ordinario, introduce some variety in the dense middle
; ----------------------------------------------------------------------------

(def style_schedule
  (list
    (list 0   (list "ord"))
    (list 120 (list "ord" "ord" "ord"))
    (list 180 (list "ord" "ord" "ord"))
    (list 240 (list "ord" "ord"))
    (list 360 (list "ord"))))

; ----------------------------------------------------------------------------
; dynamic groups over time
; ppp -> fff -> ppp
; ----------------------------------------------------------------------------

(def dynamic_schedule
  (list
    (list 0   (list "ppp"))
    (list 60  (list "ppp" "pp"))
    (list 120 (list "pp" "p" "mp"))
    (list 180 (list "mf" "f" "ff" "fff"))
    (list 240 (list "f" "mf" "mp"))
    (list 300 (list "pp" "ppp"))
    (list 360 (list "ppp"))))

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
