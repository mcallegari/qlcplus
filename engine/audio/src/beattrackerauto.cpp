/*
  Q Light Controller Plus
  beattrackerauto.cpp

  Port of the QLCplusShowCreator beat tracker (Python reference,
  commit aa8ca6f). See beattrackerauto.h for the algorithm overview;
  the reference implementation and its measured benchmarks live in
  QLCplusShowCreator: audio/realtime_spectral.py (_push_beat_samples),
  auto/bpm_detector.py (AutoBPMDetector), docs/beat-tracking.md.

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

#include <algorithm>
#include <cmath>
#include <cstring>

#include "beattrackerauto.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/*****************************************************************************
 * Small numeric helpers (match numpy semantics used by the reference)
 *****************************************************************************/

// np.hanning(M): 0.5 - 0.5*cos(2*pi*n/(M-1))
static std::vector<double> hanning(int m)
{
    std::vector<double> w(m);
    if (m == 1)
    {
        w[0] = 1.0;
        return w;
    }
    for (int n = 0; n < m; n++)
        w[n] = 0.5 - 0.5 * std::cos(2.0 * M_PI * n / (m - 1));
    return w;
}

// np.convolve(x, k, mode="same")
static std::vector<double> convolveSame(const std::vector<double> &x,
                                        const std::vector<double> &k)
{
    const int n = int(x.size());
    const int m = int(k.size());
    const int off = (m - 1) / 2;
    std::vector<double> out(n, 0.0);
    for (int i = 0; i < n; i++)
    {
        double acc = 0.0;
        for (int t = 0; t < m; t++)
        {
            int j = i + off - t;
            if (j >= 0 && j < n)
                acc += k[t] * x[j];
        }
        out[i] = acc;
    }
    return out;
}

// np.interp(pos, arange(n), y) for pos already clipped to [0, n-1]
static double lerpAt(const std::vector<double> &y, double pos)
{
    if (pos <= 0.0)
        return y.front();
    const int n = int(y.size());
    if (pos >= n - 1)
        return y.back();
    int i = int(pos);
    double f = pos - i;
    return y[i] + f * (y[i + 1] - y[i]);
}

// first-maximum argmax, numpy-style
static int argmax(const std::vector<double> &v, int lo, int hi)
{
    int best = lo;
    for (int i = lo + 1; i < hi; i++)
        if (v[i] > v[best])
            best = i;
    return best;
}

// In-place iterative radix-2 complex FFT (size must be a power of two).
static void fftRadix2(std::vector<double> &re, std::vector<double> &im)
{
    const int n = int(re.size());
    for (int i = 1, j = 0; i < n; i++)
    {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j)
        {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }
    for (int len = 2; len <= n; len <<= 1)
    {
        double ang = -2.0 * M_PI / len;
        double wr = std::cos(ang), wi = std::sin(ang);
        for (int i = 0; i < n; i += len)
        {
            double cr = 1.0, ci = 0.0;
            for (int k = 0; k < len / 2; k++)
            {
                double ur = re[i + k], ui = im[i + k];
                double vr = re[i + k + len / 2] * cr - im[i + k + len / 2] * ci;
                double vi = re[i + k + len / 2] * ci + im[i + k + len / 2] * cr;
                re[i + k] = ur + vr;
                im[i + k] = ui + vi;
                re[i + k + len / 2] = ur - vr;
                im[i + k + len / 2] = ui - vi;
                double ncr = cr * wr - ci * wi;
                ci = cr * wi + ci * wr;
                cr = ncr;
            }
        }
    }
}

/*****************************************************************************
 * BeatOnsetExtractor
 *****************************************************************************/

BeatOnsetExtractor::BeatOnsetExtractor(int sampleRate)
    : m_sampleRate(sampleRate > 0 ? sampleRate : 44100)
    , m_hop(512)
    , m_fftSize(4096)
{
    std::vector<double> w = hanning(m_fftSize);
    m_window.assign(w.begin(), w.end()); // float32 like the reference
    m_re.resize(m_fftSize);
    m_im.resize(m_fftSize);
    reset();
}

void BeatOnsetExtractor::reset()
{
    m_buffer.assign(m_fftSize, 0.0f);
    m_pending.clear();
    m_prevMagLog.clear();
    m_hasPrev = false;
    for (int i = 0; i < 3; i++)
    {
        m_bandRefs[i] = 0.0;
        m_bandPrev[i] = 0.0;
    }
    // Per-band peak follower with ~3 s decay at the onset frame rate
    m_refDecay = std::exp(-1.0 / (3.0 * frameRateHz()));
}

void BeatOnsetExtractor::push(const float *samples, int count,
                              std::vector<double> &onsetsOut)
{
    m_pending.insert(m_pending.end(), samples, samples + count);
    size_t consumed = 0;
    while (m_pending.size() - consumed >= size_t(m_hop))
    {
        processHop(m_pending.data() + consumed, onsetsOut);
        consumed += m_hop;
    }
    if (consumed > 0)
        m_pending.erase(m_pending.begin(), m_pending.begin() + consumed);
}

void BeatOnsetExtractor::processHop(const float *chunk,
                                    std::vector<double> &onsetsOut)
{
    // Slide the analysis window by one hop
    std::memmove(m_buffer.data(), m_buffer.data() + m_hop,
                 (m_fftSize - m_hop) * sizeof(float));
    std::memcpy(m_buffer.data() + (m_fftSize - m_hop), chunk,
                m_hop * sizeof(float));

    // Window (in float32, like the reference), FFT in double
    for (int i = 0; i < m_fftSize; i++)
    {
        m_re[i] = double(m_buffer[i] * m_window[i]);
        m_im[i] = 0.0;
    }
    fftRadix2(m_re, m_im);

    const int bins = m_fftSize / 2 + 1;
    std::vector<double> magLog(bins);
    for (int k = 0; k < bins; k++)
        magLog[k] = std::log1p(std::sqrt(m_re[k] * m_re[k] + m_im[k] * m_im[k]));

    if (!m_hasPrev)
    {
        m_prevMagLog = magLog;
        m_hasPrev = true;
        onsetsOut.push_back(0.0);
        return;
    }

    // Positive log-magnitude difference, summed per band:
    // <200 Hz (kick), 200-4000 Hz (snare/vocals), >=4 kHz (hats/cymbals)
    double bandSum[3] = { 0.0, 0.0, 0.0 };
    for (int k = 0; k < bins; k++)
    {
        double diff = magLog[k] - m_prevMagLog[k];
        m_prevMagLog[k] = magLog[k];
        if (diff <= 0.0)
            continue;
        double freq = double(k) * m_sampleRate / m_fftSize;
        int band = (freq < 200.0) ? 0 : (freq < 4000.0 ? 1 : 2);
        bandSum[band] += diff;
    }

    // Per band: saturate against the band's recent peak (a kick and a
    // snare onset then spike near-equally, so a backbeat's every-beat
    // periodicity survives), keep only the rising edge (a snare's noise
    // tail keeps producing "new energy" for several frames, a kick
    // doesn't - shapes equalize), combine with hats down-weighted so
    // subdivision does not read as double tempo.
    static const double bandWeights[3] = { 1.0, 1.0, 0.4 };
    double onset = 0.0;
    for (int i = 0; i < 3; i++)
    {
        double v = bandSum[i];
        double ref = std::max(v, m_bandRefs[i] * m_refDecay);
        m_bandRefs[i] = ref;
        double sat = (ref > 0.0) ? v / (v + 0.5 * ref) : 0.0;
        onset += bandWeights[i] * std::max(0.0, sat - m_bandPrev[i]);
        m_bandPrev[i] = sat;
    }
    onsetsOut.push_back(onset);
}

/*****************************************************************************
 * AutoBpmDetector
 *****************************************************************************/

AutoBpmDetector::AutoBpmDetector(double analysisRateHz, double windowSeconds,
                                 double octaveRaiseThreshold)
    : m_rate(analysisRateHz)
    , m_windowSize(int(windowSeconds * analysisRateHz))
    , m_octaveRaiseThreshold(octaveRaiseThreshold)
    , m_analysisInterval(2.0)
    , m_beliefJumpProb(0.10)
{
    // Candidate tempo grid, 0.25 BPM steps over 50-240
    for (double bpm = 50.0; bpm <= 240.0 + 1e-9; bpm += 0.25)
        m_bpmGrid.push_back(bpm);

    // Pre-ACF smoothing kernel, ~+/-23 ms absolute width: rate-scaled so
    // human timing jitter is absorbed identically at any frame rate
    int half = std::max(1, int(std::lround(0.023 * analysisRateHz)));
    std::vector<double> k = hanning(2 * half + 3);
    m_acfSmoothKernel.assign(k.begin() + 1, k.end() - 1);
    double ksum = 0.0;
    for (size_t i = 0; i < m_acfSmoothKernel.size(); i++)
        ksum += m_acfSmoothKernel[i];
    for (size_t i = 0; i < m_acfSmoothKernel.size(); i++)
        m_acfSmoothKernel[i] /= ksum;

    // Belief transition blur: 15-tap gaussian, sigma 4 grid steps
    double bsum = 0.0;
    for (int i = -7; i <= 7; i++)
    {
        double v = std::exp(-0.5 * (i / 4.0) * (i / 4.0));
        m_beliefBlur.push_back(v);
        bsum += v;
    }
    for (size_t i = 0; i < m_beliefBlur.size(); i++)
        m_beliefBlur[i] /= bsum;

    reset();
}

void AutoBpmDetector::reset()
{
    m_fluxBuffer.assign(m_windowSize, 0.0f);
    m_writePos = 0;
    m_count = 0;
    m_lastAnalysisTime = 0.0;
    m_hasBpm = false;
    m_currentBpm = 0.0;
    m_confidence = 0.0;
    m_recentBpms.clear();
    m_belief.clear();
    m_hasBelief = false;
    m_beatAnchorFrame = 0.0;
    m_beatPeriodFrames = 0.0;
    m_hasPhase = false;
}

void AutoBpmDetector::pushOnset(double value)
{
    m_fluxBuffer[m_writePos] = float(value);
    m_writePos = (m_writePos + 1) % m_windowSize;
    m_count++;

    // Re-analyze every 2 s of AUDIO time (deterministic, unlike the
    // wall-clock pacing of the Python reference; equivalent under the
    // reference's own fake-clock test rig).
    double now = double(m_count - 1) / m_rate;
    if (now - m_lastAnalysisTime >= m_analysisInterval)
    {
        m_lastAnalysisTime = now;
        analyze();
    }
}

double AutoBpmDetector::bpm() const
{
    // Gate calibrated for this onset train: real music scores ~0.2-0.5,
    // aperiodic noise stays well under 0.1
    if (m_confidence < 0.15 || !m_hasBpm)
        return 0.0;
    if (m_recentBpms.empty())
        return m_currentBpm;
    std::vector<double> s(m_recentBpms);
    std::sort(s.begin(), s.end());
    size_t n = s.size();
    return (n % 2 == 1) ? s[n / 2] : 0.5 * (s[n / 2 - 1] + s[n / 2]);
}

double AutoBpmDetector::nextBeatFrame() const
{
    if (m_confidence < 0.15 || !m_hasPhase || m_beatPeriodFrames <= 0.0)
        return -1.0;
    double framesNow = double(m_count - 1);
    double k = std::ceil((framesNow - m_beatAnchorFrame) / m_beatPeriodFrames);
    return m_beatAnchorFrame + k * m_beatPeriodFrames;
}

void AutoBpmDetector::analyze()
{
    if (m_count < m_windowSize / 2)
        return; // not enough data

    // Chronological onset signal (oldest first)
    std::vector<double> signal;
    if (m_count >= m_windowSize)
    {
        signal.resize(m_windowSize);
        for (int i = 0; i < m_windowSize; i++)
            signal[i] = m_fluxBuffer[(m_writePos + i) % m_windowSize];
    }
    else
    {
        signal.assign(m_fluxBuffer.begin(), m_fluxBuffer.begin() + int(m_count));
    }
    const int n = int(signal.size());
    if (n < 100)
        return;

    // Remove DC, widen onset spikes (rate-scaled kernel) so a beat
    // period between lag bins still forms one coherent ACF peak
    double mean = 0.0;
    for (int i = 0; i < n; i++)
        mean += signal[i];
    mean /= n;
    for (int i = 0; i < n; i++)
        signal[i] -= mean;
    signal = convolveSame(signal, m_acfSmoothKernel);

    // Unbiased autocorrelation (each lag averaged over its overlap count)
    std::vector<double> acf(n);
    for (int lag = 0; lag < n; lag++)
    {
        double acc = 0.0;
        for (int i = lag; i < n; i++)
            acc += signal[i] * signal[i - lag];
        acf[lag] = acc / (n - lag);
    }
    if (acf[0] <= 0.0)
        return;
    for (int i = n - 1; i >= 0; i--)
        acf[i] /= acf[0];

    // Comb score per candidate tempo: ACF at 1x..4x the (fractional)
    // period, each harmonic sampled as the local max within +/-1 lag
    static const double harmonicWeights[4] = { 1.0, 0.7, 0.45, 0.3 };
    static const double offsets[5] = { -1.0, -0.5, 0.0, 0.5, 1.0 };
    const int gridSize = int(m_bpmGrid.size());
    const double maxUsable = double(n - 1);
    std::vector<double> scores(gridSize, 0.0);
    std::vector<double> weightUsed(gridSize, 0.0);
    for (int g = 0; g < gridSize; g++)
    {
        double period = m_rate * 60.0 / m_bpmGrid[g];
        for (int k = 1; k <= 4; k++)
        {
            double h = k * period;
            if (h > maxUsable)
                continue;
            double peak = -1e300;
            for (int o = 0; o < 5; o++)
            {
                double pos = h + offsets[o];
                if (pos < 0.0)
                    pos = 0.0;
                if (pos > maxUsable)
                    pos = maxUsable;
                peak = std::max(peak, lerpAt(acf, pos));
            }
            scores[g] += harmonicWeights[k - 1] * peak;
            weightUsed[g] += harmonicWeights[k - 1];
        }
    }
    bool anyUsable = false;
    for (int g = 0; g < gridSize; g++)
    {
        if (weightUsed[g] > 0.0)
        {
            scores[g] /= weightUsed[g];
            anyUsable = true;
        }
    }
    if (!anyUsable)
        return;

    // Temporal belief filter: predict (blur + uniform jump), update with
    // sharpened scores; the argmax is the stable base candidate
    std::vector<double> obs(gridSize);
    for (int g = 0; g < gridSize; g++)
        obs[g] = scores[g] * scores[g] * scores[g] + 1e-9;
    if (!m_hasBelief)
    {
        double s = 0.0;
        for (int g = 0; g < gridSize; g++)
            s += obs[g];
        m_belief.resize(gridSize);
        for (int g = 0; g < gridSize; g++)
            m_belief[g] = obs[g] / s;
        m_hasBelief = true;
    }
    else
    {
        std::vector<double> pred = convolveSame(m_belief, m_beliefBlur);
        for (int g = 0; g < gridSize; g++)
            pred[g] = (1.0 - m_beliefJumpProb) * pred[g]
                      + m_beliefJumpProb / gridSize;
        double s = 0.0;
        for (int g = 0; g < gridSize; g++)
        {
            m_belief[g] = pred[g] * obs[g];
            s += m_belief[g];
        }
        for (int g = 0; g < gridSize; g++)
            m_belief[g] /= s;
    }

    int best = raiseOctave(argmax(m_belief, 0, gridSize), scores);

    // Confidence is the harmonic score itself: ~1 for a cleanly periodic
    // onset train, near 0 for aperiodic noise
    m_confidence = std::min(1.0, std::max(0.0, scores[best]));
    m_currentBpm = std::min(240.0, std::max(50.0, m_bpmGrid[best]));
    m_hasBpm = true;
    m_recentBpms.push_back(m_currentBpm);
    if (m_recentBpms.size() > 3)
        m_recentBpms.erase(m_recentBpms.begin());

    estimatePhase(signal);
}

int AutoBpmDetector::raiseOctave(int idx, const std::vector<double> &scores) const
{
    // Walk up to 2x/3x-faster candidates while their comb evidence holds
    // (>= threshold x current), searching +/-2 BPM around each multiple.
    // Hysteresis: a candidate within ~0.06 octaves of the previously
    // reported estimate needs 0.04 less evidence, so near-threshold
    // octave decisions don't flap between analyses.
    const double step = m_bpmGrid[1] - m_bpmGrid[0];
    const int window = int(std::lround(2.0 / step));
    const int gridSize = int(m_bpmGrid.size());

    // m_currentBpm still holds the previous analysis' estimate here
    const bool havePrev = m_hasBpm;
    const double prevBpm = m_currentBpm;

    while (true)
    {
        double bpm = m_bpmGrid[idx];
        int bestCand = -1;
        static const double mults[2] = { 2.0, 3.0 };
        for (int mi = 0; mi < 2; mi++)
        {
            double target = bpm * mults[mi];
            if (target > m_bpmGrid[gridSize - 1] + 1e-9)
                continue;
            int j = int(std::lround((target - m_bpmGrid[0]) / step));
            int lo = std::max(0, j - window);
            int hi = std::min(gridSize, j + window + 1);
            int k = argmax(scores, lo, hi);
            double threshold = m_octaveRaiseThreshold;
            bool candNearPrev = havePrev
                && std::fabs(std::log2(m_bpmGrid[k] / prevBpm)) < 0.06;
            bool baseNearPrev = havePrev
                && std::fabs(std::log2(bpm / prevBpm)) < 0.06;
            if (candNearPrev)
                threshold -= 0.04;
            else if (baseNearPrev)
                threshold += 0.04;
            if (scores[k] >= threshold * scores[idx])
            {
                if (bestCand < 0 || scores[k] > scores[bestCand])
                    bestCand = k;
            }
        }
        if (bestCand < 0)
            return idx;
        idx = bestCand;
    }
}

void AutoBpmDetector::estimatePhase(const std::vector<double> &signal)
{
    // Correlate the preprocessed onset window with an impulse train at
    // the current period, weighted with a half-life of one beat so the
    // most recent onsets dominate; the best fractional offset becomes
    // the anchor for the predicted beat grid.
    if (!m_hasBpm)
        return;
    double period = m_rate * 60.0 / m_currentBpm;
    const int n = int(signal.size());
    int nBeats = std::min(int(n / period), 16);
    if (nBeats < 2)
        return;

    const int nOffsets = int(std::ceil(period / 0.25 - 1e-12));
    std::vector<double> scores(nOffsets, 0.0);
    double decay = 1.0;
    for (int k = 0; k < nBeats; k++)
    {
        for (int o = 0; o < nOffsets; o++)
        {
            double pos = (n - 1) - o * 0.25 - k * period;
            if (pos >= 0.0)
                scores[o] += decay * lerpAt(signal, pos);
        }
        decay *= 0.5;
    }
    double best = 0.25 * argmax(scores, 0, nOffsets);
    m_beatAnchorFrame = double(m_count - 1) - best;
    m_beatPeriodFrames = period;
    m_hasPhase = true;
}

/*****************************************************************************
 * BeatTrackerAuto
 *****************************************************************************/

BeatTrackerAuto::BeatTrackerAuto(int sampleRate, int channels)
    : m_sampleRate(sampleRate > 0 ? sampleRate : 44100)
    , m_channels(std::max(1, channels))
    , m_extractor(m_sampleRate)
    , m_detector(m_extractor.frameRateHz())
    , m_lastEmitFrame(-1.0)
{
}

void BeatTrackerAuto::setFormat(int sampleRate, int channels)
{
    if (sampleRate <= 0)
        return;
    m_sampleRate = sampleRate;
    m_channels = std::max(1, channels);
    m_extractor = BeatOnsetExtractor(m_sampleRate);
    m_detector = AutoBpmDetector(m_extractor.frameRateHz());
    m_lastEmitFrame = -1.0;
}

void BeatTrackerAuto::reset()
{
    m_extractor.reset();
    m_detector.reset();
    m_lastEmitFrame = -1.0;
}

bool BeatTrackerAuto::processAudio(const int16_t *buffer, int bufferSize)
{
    if (!buffer || bufferSize <= 0)
        return false;

    // bufferSize is the TOTAL sample count (frames * channels)
    const int frames = bufferSize / m_channels;
    if (frames <= 0)
        return false;

    m_mono.resize(frames);
    for (int i = 0; i < frames; i++)
    {
        int32_t acc = 0;
        const int16_t *fp = buffer + i * m_channels;
        for (int c = 0; c < m_channels; c++)
            acc += fp[c];
        m_mono[i] = float(double(acc) / (32768.0 * m_channels));
    }

    m_onsets.clear();
    m_extractor.push(m_mono.data(), frames, m_onsets);

    // Advance the detector hop by hop and emit on predicted-beat
    // crossings at hop resolution (~11.6 ms at 44.1 kHz)
    bool beat = false;
    for (size_t i = 0; i < m_onsets.size(); i++)
    {
        m_detector.pushOnset(m_onsets[i]);
        double next = m_detector.nextBeatFrame();
        if (next < 0.0)
            continue;
        double framesNow = double(m_detector.frameCount() - 1);
        double period = m_detector.beatPeriodFrames();
        // The predicted beat lands before the next hop; the half-period
        // refractory absorbs small grid shifts when a re-analysis moves
        // the anchor
        if (next - framesNow < 1.0
            && (m_lastEmitFrame < 0.0
                || framesNow - m_lastEmitFrame >= 0.5 * period))
        {
            beat = true;
            m_lastEmitFrame = framesNow;
        }
    }
    return beat;
}
