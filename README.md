# pure0

**pure0** is a minimal, expressive programming language for **sound synthesis, music creation, and computational creativity**.

It combines:

- a **Scheme-inspired interpreter** written in modern C++
- a **vector-based numerical core**
- a **DSP library** for signal processing
- a **musical layer** for pattern-based composition

The goal of pure0 is to provide a **small, transparent, and hackable environment** where sound, mathematics, and composition coexist naturally.

---

## Philosophy

pure0 is built around a few core ideas:

- **Everything is a signal** → vectors are first-class citizens  
- **Composition = transformation** → signals are shaped, combined, and reinterpreted  
- **Minimal core, extensible layers** → small interpreter + powerful libraries  
- **Clarity over complexity** → no hidden magic, no heavy frameworks  

It sits somewhere between:

- Scheme / Lisp
- SuperCollider / Strudel
- MATLAB / NumPy (for vectors)
- low-level DSP environments

---

## Core Language

The language is a **Scheme-like interpreter** with:

- lexical scoping
- first-class functions (`lambda`)
- partial application
- lists and vectors
- higher-order programming

Example:

```scheme
(def add2 (lambda (x) (+ x 2)))
(add2 3) ; => 5
```

Vectors are native:

```scheme
(vec 1 2 3)
(+ (vec 1 2 3) 10) ; => [11 12 13]
```

Broadcasting and element-wise operations are automatic.

---

## DSP Layer

The DSP library provides low-level building blocks:

- oscillators (`osc`)
- wavetable generation (`gen`)
- FFT / IFFT
- filters (`iir`, `iirdesign`)
- convolution
- delay / comb / allpass
- resampling
- WAV I/O (`wavread`, `wavwrite`)
- signal mixing (`mix`)

Example:

```scheme
(def freq (vec 440 440 440 440))
(def table (gen 1024 (vec 1))) ; sine
(def sig (osc 48000 freq table))
```

---

## Music Layer

On top of DSP, pure0 provides musical primitives:

### Rhythms & Patterns

```scheme
(euclid 5 16)
(pat sr bpm beats pattern sound)
```

### Sequencing

```scheme
(patnotes sr bpm beats pattern notes dur amp)
(bassline sr bpm beats pattern notes dur amp wobble shape)
(arp sr bpm beats notes subdiv dur amp shape mode)
```

### Instruments

```scheme
(kick sr dur ...)
(snare sr dur ...)
(hat sr dur ...)
```

Extended timbral primitives:

```scheme
(drone_noise ...)
(drone_reson ...)
(kick_sub ...)
(kick_click ...)
(hat_dark ...)
(hat_metal ...)
(pad_minor ...)
(stab_minor ...)
(bass_sub ...)
```

### Arrangement

```scheme
(bmix sr bpm beat0 sig0 beat1 sig1 ...)
(stereo left right)
(norm sig 0.95)
```

---

## Example

```scheme
(def sr 48000)
(def bpm 120)

(def drums
  (bmix sr bpm
    0 (pat sr bpm 4 (vec 1 0 0 0 1 0 0 0 1 0 0 0 1 0 0 0)
           (kick sr 0.4))
    0 (pat sr bpm 4 (euclid 11 16)
           (hat sr 0.06))))

(def bass
  (bassline sr bpm 4
    (vec 1 0 1 0 1 0 0 0 1 0 1 0 0 0 1 0)
    (vec 36 38 33 31)
    0.5 0.4))

(def piece
  (norm (bmix sr bpm 0 drums 0 bass) 0.95))

(wavwrite piece sr "out.wav")
```

---

## Architecture

```
core.h     → interpreter, types, evaluation
dsp.h      → signal processing primitives
music.h    → musical abstractions (patterns, instruments)
```

- **core** = language
- **dsp** = physics of sound
- **music** = composition layer

---

## Why pure0?

pure0 is designed for:

- composers exploring **algorithmic music**
- researchers in **computational creativity**
- developers who want a **small, hackable audio language**
- teaching environments for **DSP and music programming**

It emphasizes:

- **transparency** (readable codebase)
- **composability** (everything combines)
- **directness** (no black boxes)

---

## Status

pure0 is experimental and evolving.

It is already capable of:

- full audio synthesis
- structured composition
- long-form generative pieces

Future directions include:

- real-time audio
- live coding workflows
- richer synthesis models
- higher-level musical abstractions

---

## License

(TBD)

---

## Author

Carmine Emanuele Cella
