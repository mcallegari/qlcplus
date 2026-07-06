/*
  Q Light Controller Plus
  beattrackerauto.h

  Port of the QLCplusShowCreator beat tracker (Python reference,
  commit aa8ca6f): multi-band saturated rising-edge onset front end
  (audio/realtime_spectral.py, _push_beat_samples) feeding a
  comb-scored autocorrelation tempo estimator with a temporal belief
  filter, octave-raise walk and beat-phase output
  (auto/bpm_detector.py, AutoBPMDetector).

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#ifndef BEATTRACKERAUTO_H
#define BEATTRACKERAUTO_H

#include <cstdint>
#include <vector>

/** @addtogroup engine_audio Audio
 * @{
 */

/**
 * Onset front end: one band-combined onset value per 512 input samples
 * (86.13 Hz at 44100). 4096-sample Hann-windowed FFT (~93 ms - the window
 * span in SECONDS is load-bearing: a ~46 ms window resolves every
 * micro-transient of sustained instruments and buries the beat in onset
 * noise on real music), log1p magnitude, positive spectral difference,
 * three bands (<200 Hz kick / 200-4000 Hz snare-vocals / >=4 kHz hats).
 * Each band is saturated against its own recent peak (~3 s decay
 * follower) and reduced to its rising edge, then combined with weights
 * 1 / 1 / 0.4. Self-contained radix-2 FFT; no external dependencies.
 */
class BeatOnsetExtractor
{
public:
    explicit BeatOnsetExtractor(int sampleRate);

    void reset();

    /** Onset values produced per second of audio (sampleRate / 512). */
    double frameRateHz() const { return double(m_sampleRate) / double(m_hop); }

    /** Feed mono samples in [-1, 1]; appends one onset value per
     *  elapsed 512-sample hop to @a onsetsOut (possibly none). */
    void push(const float *samples, int count, std::vector<double> &onsetsOut);

private:
    void processHop(const float *chunk, std::vector<double> &onsetsOut);

private:
    int m_sampleRate;
    int m_hop;      // 512
    int m_fftSize;  // 4096

    std::vector<float> m_window;    // Hann, float32 to match the reference
    std::vector<float> m_buffer;    // sliding fftSize window
    std::vector<float> m_pending;   // not-yet-hop-aligned input

    std::vector<double> m_prevMagLog;
    bool m_hasPrev;

    double m_refDecay;              // exp(-1 / (3 s * frame rate))
    double m_bandRefs[3];
    double m_bandPrev[3];

    // FFT workspace (radix-2, complex in-place)
    std::vector<double> m_re, m_im;
};

/**
 * Tempo + phase estimator over the onset train. Every 2 s of audio it
 * runs, on an 8 s window: unbiased autocorrelation pre-smoothed with a
 * rate-scaled (~+/-23 ms) Hann kernel; a harmonic comb score for every
 * candidate on a fractional 50-240 BPM grid (0.25 steps, ACF sampled at
 * 1x..4x the period as the local max within +/-1 lag, weights
 * 1/0.7/0.45/0.3); a forward-filtered belief over the grid (observation
 * = scores^3, transition = gaussian blur + 10% uniform jump) picks the
 * base candidate; an octave-raise walk prefers the fastest 2x/3x
 * candidate holding >= 0.90 of the current score, with +/-0.04
 * hysteresis toward the previous estimate. Reported BPM is the median
 * of the last 3 analyses, gated on comb confidence >= 0.15.
 */
class AutoBpmDetector
{
public:
    explicit AutoBpmDetector(double analysisRateHz,
                             double windowSeconds = 8.0,
                             double octaveRaiseThreshold = 0.90);

    void reset();

    /** Feed one onset value (one hop of audio). */
    void pushOnset(double value);

    /** Median of the last 3 analyses, or 0.0 while confidence < 0.15. */
    double bpm() const;

    /** Comb score of the current estimate, ~0..1. */
    double confidence() const { return m_confidence; }

    /** Total onset frames received so far. */
    long long frameCount() const { return m_count; }

    /** Absolute (fractional) frame index of the next predicted beat,
     *  >= frameCount()-1, or a negative value when there is no
     *  confident estimate. */
    double nextBeatFrame() const;

    /** Current beat period in frames, or 0 when unknown. */
    double beatPeriodFrames() const { return m_beatPeriodFrames; }

private:
    void analyze();
    int raiseOctave(int idx, const std::vector<double> &scores) const;
    void estimatePhase(const std::vector<double> &signal);

private:
    double m_rate;
    int m_windowSize;
    double m_octaveRaiseThreshold;

    std::vector<float> m_fluxBuffer;   // ring buffer, float32 like the reference
    int m_writePos;
    long long m_count;
    double m_lastAnalysisTime;         // audio seconds
    double m_analysisInterval;

    bool m_hasBpm;
    double m_currentBpm;
    double m_confidence;
    std::vector<double> m_recentBpms;  // last <= 3 analyses

    std::vector<double> m_bpmGrid;     // 50..240 step 0.25
    std::vector<double> m_acfSmoothKernel;
    std::vector<double> m_beliefBlur;  // 15-tap gaussian, sigma 4 grid steps
    double m_beliefJumpProb;
    std::vector<double> m_belief;
    bool m_hasBelief;

    double m_beatAnchorFrame;          // absolute frame of last located beat
    double m_beatPeriodFrames;
    bool m_hasPhase;
};

/**
 * QLC+ facade: implements the AudioCapture beat interface (interleaved
 * int16 blocks, bufferSize = TOTAL samples = frames * channels).
 * processAudio() returns true when a predicted beat falls inside the
 * block; bpm() exposes the estimator's tempo so the UI can display it
 * instead of re-deriving BPM from wall-clock signal spacing.
 */
class BeatTrackerAuto
{
public:
    BeatTrackerAuto(int sampleRate, int channels);

    /** Reconfigure for a new capture format; resets all state. */
    void setFormat(int sampleRate, int channels);

    void reset();

    /** Process an interleaved int16 block; bufferSize is the total
     *  number of samples (frames * channels). Returns true if a beat
     *  occurred within this block. */
    bool processAudio(const int16_t *buffer, int bufferSize);

    /** Current tempo estimate, 0.0 while unavailable/low-confidence. */
    double bpm() const { return m_detector.bpm(); }

    double confidence() const { return m_detector.confidence(); }

private:
    int m_sampleRate;
    int m_channels;
    BeatOnsetExtractor m_extractor;
    AutoBpmDetector m_detector;
    std::vector<float> m_mono;
    std::vector<double> m_onsets;
    double m_lastEmitFrame;
};

/** @} */

#endif // BEATTRACKERAUTO_H
