# BeatTrackerAuto — algorithm documentation

Beat tracker by varghele, developed and benchmarked in Python across several
measured iterations, then ported to self-contained C++ for QLC+
(`beattrackerauto.{h,cpp}`). This document is the standalone description of
how it works, why each stage exists, and how it was validated. Related
upstream discussion: issue #1881.

## Overview

Two components, connected by an onset-strength stream:

```
int16 PCM blocks -> BeatOnsetExtractor -> onset value per 512-sample hop
                        (~86 Hz)              |
                                              v
                                       AutoBpmDetector -> BPM estimate,
                                        (analysis every    confidence,
                                         2 s of audio)     predicted beat grid
```

`BeatTrackerAuto` is the facade that adapts both to QLC+'s `AudioCapture`
contract: interleaved int16 input where `bufferSize` is the **total** sample
count (frames × channels), a boolean per-block "beat occurred" return that
drives `beatDetected()`, and a `bpm()` getter for the UI.

Everything is plain C++ with an internal radix-2 FFT — no Qt, no fftw3.
This is deliberate: fftw3 is an optional engine dependency (`HAS_FFTW3`),
so a tracker relying on it silently disappears from builds without it.
CPU cost is ~1.3% of one core at 44.1 kHz.

## Onset front end (`BeatOnsetExtractor`)

Per 512-sample hop (86.13 Hz at 44.1 kHz):

1. **FFT** over a sliding 4096-sample Hann window (~93 ms). The window
   length in *seconds* is load-bearing: a ~46 ms window resolves every
   micro-transient of sustained instruments and buries the beat in onset
   noise on real music (measured, cost a full benchmark round).
2. **log1p magnitude, positive spectral difference** vs. the previous hop
   (only energy increases count as onset evidence).
3. **Three bands**: < 200 Hz (kick), 200–4000 Hz (snare/vocals),
   ≥ 4 kHz (hats/cymbals).
4. **Per-band saturation** against the band's own recent peak (peak
   follower with ~3 s decay): `v / (v + 0.5·ref)`. A kick and a snare
   onset then spike near-equally, so a backbeat's every-beat periodicity
   survives instead of half-locking at the kick rate.
5. **Rising edge only**: `max(0, sat − previous sat)`. A snare's noise
   tail keeps producing "new energy" for several frames, a kick doesn't —
   this equalizes the spike *shapes* between instruments.
6. **Weighted sum** with weights 1 / 1 / 0.4: hats stay visibly weaker
   than beats so subdivision does not read as double tempo.

The backbeat fix (stages 4–5) was found by measurement: neither scalar nor
per-bin log compression solved kick/snare alternation — the problem is
spectral extent and envelope shape, not loudness.

## Tempo estimator (`AutoBpmDetector`)

Runs every 2 s of **audio time** (not wall clock — deterministic and
testable) over an 8 s window of the onset stream:

1. **Pre-smoothing**: the DC-removed onset signal is convolved with a
   Hann kernel of ~±23 ms absolute width, *scaled with the frame rate* —
   it absorbs human timing jitter, which otherwise smears the ACF peak
   across more lags the finer the hop grid gets.
2. **Unbiased autocorrelation** (each lag normalized by its overlap
   count), computed directly (n ≈ 690, negligible cost).
3. **Harmonic comb score** for every candidate on a fractional BPM grid
   (50–240, 0.25 steps): the ACF sampled at 1×..4× the candidate period
   with weights 1 / 0.7 / 0.45 / 0.3, each harmonic taken as the local
   maximum within ±1 lag (offsets −1, −0.5, 0, +0.5, +1). The fractional
   grid removes lag quantization (at 86 Hz a 190 BPM beat is only ~27
   hops); the local-max sampling lets a slightly detuned harmonic count
   at full height while a genuinely weak off-beat stays weak — that is
   what keeps the octave decision discriminative.
4. **Temporal belief filter** over the grid: prediction = 15-tap gaussian
   blur (sigma 4 grid steps) plus a 10% uniform jump probability;
   observation = comb scores cubed. The belief argmax is the stable base
   candidate — one noisy analysis cannot flip the estimate, while a real
   tempo change wins within a few analyses via the jump mass.
5. **Octave-raise walk**: from the base candidate, move to a 2×/3× faster
   candidate (searched within ±2 BPM of the multiple) while its comb
   score holds ≥ 0.90 of the current one, with ±0.04 hysteresis toward
   the previously reported estimate so near-threshold decisions don't
   flap between analyses. This disambiguates octaves in both directions
   without a tempo prior. (A log-normal tempo prior and a continuity
   score bonus were both tried and rejected on measurements: the prior
   systematically halves everything ≥ 190 BPM, the bonus locks in early
   wrong estimates.)
6. **Reporting**: `bpm()` returns the median of the last 3 analyses,
   gated on comb confidence ≥ 0.15 (real music scores ~0.2–0.5,
   aperiodic noise stays well under 0.1 — silence and noise never
   report).

## Beat phase and emission

Each analysis correlates the preprocessed onset window with an impulse
train at the found period, weighted with a half-life of one beat so the
most recent onsets dominate, over a 0.25-hop offset grid. The best offset
becomes the beat anchor; anchor + period define a predicted beat grid.
The facade emits a beat when the next predicted beat falls within the
current hop (~11.6 ms resolution), with a half-period refractory that
absorbs small grid shifts when a re-analysis moves the anchor.

Emitting from the predicted grid — rather than reacting to individual
onsets — is what makes the beat output steady on syncopated material.

## Behavior characteristics

- **First estimate** after ~4 s of music (half the analysis window);
  typically correct by 4–8 s.
- **Re-lock after a live tempo change**: ~12–16 s. This is the deliberate
  price of the belief filter + median-of-3 stability (without them,
  single-analysis errors flap the reported BPM).
- **Known limit**: material genuinely dominated by 8th-note energy can
  read an octave up — this is evidence-driven (the subdivision really
  does carry the energy), not a systematic bias.
- **Silence**: onset stream goes to zero, confidence collapses below the
  gate, no beats are emitted and no BPM is reported.
- **Format**: any sample rate (all time constants derive from it) and
  any channel count (interleaved frames are averaged to mono).

## Validation

- The C++ port was verified hop-for-hop against the Python reference:
  identical gated BPM decisions on every hop, onset values matching to
  ~1e-7 (float32 rounding).
- Synthetic audio suite (8 scenarios × 8 tempi, 50–240 BPM: four-on-floor,
  8th hats, kick/snare backbeat, 8th-note bassline, swung hats, ±3% drift,
  2.5 s dropout, +30% tempo step; rendered percussion as int16 PCM):
  **64/64 correct** (4% tolerance). Faithful ports of QLC+'s two existing
  trackers on the same suite: 48/64 (beattracking.cpp) and 54/64
  (beattracker.cpp). An in-repo C++ port of this suite lives in
  `engine/test/beattrackerbench` (run with `--full` for the whole table);
  with its RNG it reproduces 63/64 vs. 54/64 vs. 48/64 — the single flip
  is a 90 BPM 8th-hats clip reading an octave up, i.e. the known limit
  described above.
- Onset-level suite (7 scenarios × 20 tempi incl. noise at SNR 2:1):
  139/140.
- Real songs: 9-song album benchmark with per-section ground truth,
  scored as % of post-warmup time within 4%: 65% vs. 63% and 4% for the
  two existing trackers.
- Issue #1881's reference video (a 100 BPM metronome): reads 100.00 BPM
  with 0.600 s beat intervals; a 140 BPM rock drum-track video reads
  139.75. Both confirmed in a live microphone test.
- Unit test: `engine/test/beattrackerauto` (tempo accuracy at
  90/120/140/174 BPM, beat spacing regularity, silence gating,
  stereo/mono equivalence).

The benchmark harness (including the Python ports of the existing QLC+
trackers used for the head-to-head numbers) is part of varghele's
reference implementation and is available on request.
