; stft_istft.scm
(load "stdlib.scm")

(def snd   (wavread "../data/cage.wav"))
(def sr    (second snd))
(def ch    (deinterleave (head snd) (third snd) 0))

(def N   2048)
(def hop (/ N 8))

; normal analysis / resynthesis
(def specs (stft ch N hop))
(def recon (istft specs N hop))
(wavwrite recon sr "stft_istft_test.wav")

; time-stretch by overlap-add spacing change
(def stretch2x (istft specs N (* hop 2)))
(wavwrite stretch2x sr "stft_istft_2x.wav")

(def stretch05x (istft specs N (/ hop 2)))
(wavwrite stretch05x sr "stft_istft_halfx.wav")

; plain resampling for comparison
(def resamp2x (resample ch 2))
(wavwrite resamp2x sr "stft_istft_resampled_2x.wav")

(def resamp05x (resample ch 0.5))
(wavwrite resamp05x sr "stft_istft_resampled_halfx.wav")

; ----------------------------------------
; pitch up = stretch longer, then resample down
; pitch down = stretch shorter, then resample up
; ----------------------------------------

; up one octave
(def ps-up2-stretch (istft specs N (* hop 2)))
(def ps-up2         (resample ps-up2-stretch 0.5))
(wavwrite ps-up2 sr "stft_pitch_up_2x.wav")

; down one octave
(def ps-down2-stretch (istft specs N (/ hop 2)))
(def ps-down2         (resample ps-down2-stretch 2.0))
(wavwrite ps-down2 sr "stft_pitch_down_0_5x.wav")

; up a perfect fifth (~1.5x)
(def ps-up15-stretch (istft specs N (floor (* hop 1.5))))
(def ps-up15         (resample ps-up15-stretch (/ 1 1.5)))
(wavwrite ps-up15 sr "stft_pitch_up_1_5x.wav")

; down to 0.8x
(def ps-down08-stretch (istft specs N (floor (* hop 0.8))))
(def ps-down08         (resample ps-down08-stretch (/ 1 0.8)))
(wavwrite ps-down08 sr "stft_pitch_down_0_8x.wav")