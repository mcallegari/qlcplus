#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <climits>

#include "beattracker.h"

// Enable this to get qDebug logs from the tracker
#define BEAT_DEBUG

#ifdef BEAT_DEBUG
#include <QDebug>
#endif

static int nextPowerOfTwo(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

BeatTracker::BeatTracker(int sampleRate,
                         int bufferSize,
                         int channels,
                         int historySize,
                         double sensitivity)
    : m_sampleRate(sampleRate),
    m_frameSize(bufferSize),
    m_fftSize(nextPowerOfTwo(bufferSize)),
    m_channels(std::max(1, channels)),
    m_sensitivity(sensitivity),
    m_fftInput(nullptr),
    m_fftOutput(nullptr),
    m_fftPlan(nullptr),
    m_historySize(historySize),
    m_historyIndex(0),
    m_historyFilled(false),
    m_lastFlux(0.0),
    m_fluxSmoothed(0.0),
    m_fluxSmoothingAlpha(0.7),
    m_minBeatIntervalSec(0.25),
    m_samplesSinceBeat(0),
    m_beatIntervalsSec(),
    m_lastBeatSample(-1),
    m_totalSamplesProcessed(0),
    // --- Silence gate defaults ---
    m_silenceThreshold(0.01),          // ~ -40 dBFS
    m_consecutiveSilentFrames(0),
    m_silenceResetFrames(0),
    // --- Band bins ---
    m_minBin(0),
    m_maxBin(0)
{
    if (bufferSize <= 0 || sampleRate <= 0)
        throw std::runtime_error("BeatTracker: invalid constructor parameters");

    allocateFft();

    // Window and magnitude history
    m_window.resize(m_frameSize);
    m_prevMag.assign(m_fftSize / 2 + 1, 0.0);

    initWindow();

    // Flux history
    m_fluxHistory.assign(m_historySize, 0.0);

    // Default band: wide; we'll usually override in user code
    setBand(40.0, 2000.0);

    // Initialize refractory so first beat is allowed immediately
    int minSamplesBetweenBeats = int(m_minBeatIntervalSec * m_sampleRate);
    m_samplesSinceBeat = minSamplesBetweenBeats;

    // Silence reset: after ~2 seconds of silence, reset BPM memory
    double framesPerSecond = (m_frameSize > 0) ? (double(m_sampleRate) / double(m_frameSize)) : 0.0;
    m_silenceResetFrames = (framesPerSecond > 0.0) ? int(framesPerSecond * 2.0) : 0;

#ifdef BEAT_DEBUG
    qDebug() << "[BeatTracker] ctor sr" << m_sampleRate
             << "frameSize" << m_frameSize
             << "channels" << m_channels
             << "fftSize" << m_fftSize
             << "historySize" << m_historySize
             << "sensitivity" << m_sensitivity
             << "silenceThreshold" << m_silenceThreshold;
#endif
}

BeatTracker::~BeatTracker()
{
    freeFft();
}

void BeatTracker::allocateFft()
{
    freeFft(); // in case it's called from setFormat

    m_fftInput  = (double*)fftw_malloc(sizeof(double) * m_fftSize);
    m_fftOutput = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (m_fftSize / 2 + 1));

    if (!m_fftInput || !m_fftOutput)
        throw std::runtime_error("BeatTracker: FFTW malloc failed");

    m_fftPlan = fftw_plan_dft_r2c_1d(
        m_fftSize,
        m_fftInput,
        m_fftOutput,
        FFTW_MEASURE
        );

    if (!m_fftPlan)
        throw std::runtime_error("BeatTracker: FFTW plan creation failed");
}

void BeatTracker::freeFft()
{
    if (m_fftPlan)
    {
        fftw_destroy_plan(m_fftPlan);
        m_fftPlan = nullptr;
    }
    if (m_fftInput)
    {
        fftw_free(m_fftInput);
        m_fftInput = nullptr;
    }
    if (m_fftOutput)
    {
        fftw_free(m_fftOutput);
        m_fftOutput = nullptr;
    }
}

void BeatTracker::initWindow()
{
    // Hann window
    if (m_frameSize <= 1)
        return;

    m_window.resize(m_frameSize);
    for (int n = 0; n < m_frameSize; ++n)
    {
        m_window[n] = 0.5 * (1.0 - std::cos(2.0 * M_PI * n / (m_frameSize - 1)));
    }
}

void BeatTracker::setBand(double minHz, double maxHz)
{
    if (minHz < 0.0) minHz = 0.0;
    if (maxHz > m_sampleRate / 2.0) maxHz = m_sampleRate / 2.0;
    if (minHz >= maxHz)
    {
        minHz = 0.0;
        maxHz = m_sampleRate / 2.0;
    }

    auto hzToBin = [this](double hz) -> int {
        return static_cast<int>(std::floor(hz * m_fftSize / m_sampleRate));
    };

    m_minBin = hzToBin(minHz);
    m_maxBin = hzToBin(maxHz);

    if (m_minBin < 0) m_minBin = 0;
    if (m_maxBin > m_fftSize / 2) m_maxBin = m_fftSize / 2;
    if (m_minBin > m_maxBin) m_minBin = m_maxBin;

#ifdef BEAT_DEBUG
    qDebug() << "[BeatTracker] setBand" << minHz << "Hz ->" << maxHz << "Hz"
             << "-> bins" << m_minBin << "-" << m_maxBin;
#endif
}

// runtime reconfiguration
void BeatTracker::setFormat(int sampleRate,
                            int bufferSize,
                            int channels)
{
    if (sampleRate <= 0 || bufferSize <= 0)
        return;

    channels = std::max(1, channels);

    // If nothing changed, skip
    if (sampleRate == m_sampleRate &&
        bufferSize == m_frameSize &&
        channels == m_channels)
    {
        return;
    }

    m_sampleRate = sampleRate;
    m_frameSize  = bufferSize;
    m_channels   = channels;
    m_fftSize    = nextPowerOfTwo(m_frameSize);

    allocateFft();

    m_window.clear();
    initWindow();

    m_prevMag.assign(m_fftSize / 2 + 1, 0.0);

    m_fluxHistory.assign(m_historySize, 0.0);
    m_historyIndex  = 0;
    m_historyFilled = false;
    m_lastFlux      = 0.0;
    m_fluxSmoothed  = 0.0;

    // Reset BPM state
    m_beatIntervalsSec.clear();
    m_lastBeatSample = -1;
    m_totalSamplesProcessed = 0;

    // Reset refractory based on new sample rate
    int minSamplesBetweenBeats = int(m_minBeatIntervalSec * m_sampleRate);
    m_samplesSinceBeat = minSamplesBetweenBeats;

    // Recompute silence reset frames
    double framesPerSecond = (m_frameSize > 0) ? (double(m_sampleRate) / double(m_frameSize)) : 0.0;
    m_silenceResetFrames = (framesPerSecond > 0.0) ? int(framesPerSecond * 2.0) : 0;
    m_consecutiveSilentFrames = 0;

    // Keep same nominal band in Hz, but recompute bins
    setBand(40.0, 2000.0);

#ifdef BEAT_DEBUG
    qDebug() << "[BeatTracker] setFormat sr" << m_sampleRate
             << "frameSize" << m_frameSize
             << "channels" << m_channels
             << "fftSize" << m_fftSize
             << "silenceResetFrames" << m_silenceResetFrames;
#endif
}

// spectral flux with log compression + low-frequency weighting
double BeatTracker::computeSpectralFlux()
{
    const int binCount = m_fftSize / 2 + 1;
    double flux = 0.0;

    for (int k = m_minBin; k <= m_maxBin && k < binCount; ++k)
    {
        double re = m_fftOutput[k][0];
        double im = m_fftOutput[k][1];
        double magLin = std::sqrt(re * re + im * im);

        // log compression reduces huge differences between hits
        double mag = std::log1p(magLin); // log(1 + |X|)

        double diff = mag - m_prevMag[k];
        if (diff > 0.0)
        {
            // Weighting: emphasize low bins within [m_minBin, m_maxBin]
            double t = 0.0;
            if (m_maxBin > m_minBin)
                t = double(k - m_minBin) / double(m_maxBin - m_minBin);

            // from 1.5 at low end to 1.0 at high end
            double weight = 1.5 - 0.5 * t;

            flux += diff * weight;
        }

        m_prevMag[k] = mag;
    }

    return flux;
}

double BeatTracker::computeAdaptiveThreshold() const
{
    int count = m_historyFilled ? m_historySize : m_historyIndex;
    if (count <= 0)
        return std::numeric_limits<double>::infinity();

    double sum = 0.0;
    for (int i = 0; i < count; ++i)
        sum += m_fluxHistory[i];

    double mean = sum / count;
    return mean * m_sensitivity;
}

double BeatTracker::getCurrentBpm() const
{
    if (m_beatIntervalsSec.empty())
        return 0.0;

    double sum = 0.0;
    for (double dt : m_beatIntervalsSec)
        sum += dt;

    if (sum <= 0.0)
        return 0.0;

    double meanInterval = sum / double(m_beatIntervalsSec.size());
    return 60.0 / meanInterval;
}

// process interleaved multi-channel audio
bool BeatTracker::processAudio(int16_t *buffer, int bufferSize)
{
    if (!buffer || bufferSize <= 0 || m_channels <= 0)
        return false;

    // frames = how many sample-frames we actually have in this buffer
    int framesAvailable = bufferSize / m_channels;
    if (framesAvailable <= 0)
        return false;

    int frames = std::min(framesAvailable, m_frameSize);

    // Sample index at the start of this frame block
    int frameStartSample = m_totalSamplesProcessed;

#ifdef BEAT_DEBUG
    static bool s_formatLogged = false;
    if (!s_formatLogged)
    {
        qDebug() << "[BeatTracker] first processAudio call:"
                 << "bufferSize(samples)" << bufferSize
                 << "framesAvailable" << framesAvailable
                 << "framesUsed" << frames
                 << "channels" << m_channels;
        s_formatLogged = true;
    }
#endif

    // 1. Mix to mono and apply window
    double maxAbsSample = 0.0;
    for (int i = 0; i < frames; ++i)
    {
        int32_t sum = 0;
        const int16_t *framePtr = buffer + i * m_channels;
        for (int c = 0; c < m_channels; ++c)
            sum += framePtr[c];

        // Average and normalize to [-1, 1]
        double s = static_cast<double>(sum) / (32768.0 * m_channels);
        if (std::fabs(s) > maxAbsSample)
            maxAbsSample = std::fabs(s);

        if (i < (int)m_window.size())
            s *= m_window[i];

        m_fftInput[i] = s;
    }

    // --- SILENCE GATE CHECK ---
    bool isSilent = (maxAbsSample < m_silenceThreshold);

    if (isSilent)
    {
        // No need to run FFT: treat as "no onset" and slowly decay flux
        m_fluxSmoothed = m_fluxSmoothingAlpha * m_fluxSmoothed;
        double fluxForHistory = m_fluxSmoothed;

        // Update history (keeps threshold reasonable over long silences)
        m_fluxHistory[m_historyIndex] = fluxForHistory;
        m_historyIndex++;
        if (m_historyIndex >= m_historySize)
        {
            m_historyIndex = 0;
            m_historyFilled = true;
        }

        // Keep lastFlux in sync with smoothed value
        m_lastFlux = fluxForHistory;

        // Refractory update: we still advance time
        int minSamplesBetweenBeats = int(m_minBeatIntervalSec * m_sampleRate);
        m_samplesSinceBeat += frames;
        if (m_samplesSinceBeat > minSamplesBetweenBeats)
            m_samplesSinceBeat = minSamplesBetweenBeats;

        // BPM state: after sustained silence, clear tempo memory
        m_consecutiveSilentFrames++;
        if (m_silenceResetFrames > 0 &&
            m_consecutiveSilentFrames >= m_silenceResetFrames)
        {
            m_beatIntervalsSec.clear();
            m_lastBeatSample = -1;
        }

        // Advance global sample counter
        m_totalSamplesProcessed += frames;

#ifdef BEAT_DEBUG
        static int s_frameCounterSilent = 0;
        s_frameCounterSilent++;
        const int logEverySilent = 40;
        if (s_frameCounterSilent % logEverySilent == 0)
        {
            qDebug().nospace()
            << "[BeatTracker] silent frame maxAbs=" << maxAbsSample
            << " fluxSmoothed=" << fluxForHistory
            << " thr=" << computeAdaptiveThreshold()
            << " consecutiveSilent=" << m_consecutiveSilentFrames;
        }
#endif

        return false;
    }
    else
    {
        // We have significant signal
        m_consecutiveSilentFrames = 0;
    }

    // Zero-pad remaining samples up to fftSize
    for (int i = frames; i < m_fftSize; ++i)
        m_fftInput[i] = 0.0;

    // 2. FFT
    fftw_execute(m_fftPlan);

    // 3. Spectral flux (raw)
    double flux = computeSpectralFlux();

    // 3b. Smooth flux (simple 1-pole low-pass)
    m_fluxSmoothed = m_fluxSmoothingAlpha * m_fluxSmoothed
                     + (1.0 - m_fluxSmoothingAlpha) * flux;
    double fluxForHistory = m_fluxSmoothed;

    // 4. Update history (for adaptive threshold)
    m_fluxHistory[m_historyIndex] = fluxForHistory;
    m_historyIndex++;
    if (m_historyIndex >= m_historySize)
    {
        m_historyIndex = 0;
        m_historyFilled = true;
    }

    // 5. Adaptive threshold
    double threshold = computeAdaptiveThreshold();

    // 6. Peak pick + refractory period
    bool isBeatCandidate = (fluxForHistory > threshold && fluxForHistory > m_lastFlux);
    m_lastFlux = fluxForHistory;

    int minSamplesBetweenBeats = int(m_minBeatIntervalSec * m_sampleRate);
    bool canTrigger = (m_samplesSinceBeat >= minSamplesBetweenBeats);

    bool isBeat = isBeatCandidate && canTrigger;

    // --- BPM estimation: if we detected a beat, update intervals ---
    if (isBeat)
    {
        // Approximate beat position at the center of the frame
        int beatSample = frameStartSample + frames / 2;

        if (m_lastBeatSample >= 0)
        {
            int diffSamples = beatSample - m_lastBeatSample;
            double dt = double(diffSamples) / double(m_sampleRate);

            // Filter out crazy intervals (e.g. <30 BPM or >240 BPM)
            if (dt > 0.25 && dt < 2.0)
            {
                m_beatIntervalsSec.push_back(dt);
                if (m_beatIntervalsSec.size() > 16) // keep last 16 intervals
                    m_beatIntervalsSec.erase(m_beatIntervalsSec.begin());
            }
        }

        m_lastBeatSample = beatSample;
    }

    // Update samplesSinceBeat for next call
    m_samplesSinceBeat += frames;

    // Clamp to avoid overflow and keep it meaningful
    if (m_samplesSinceBeat > minSamplesBetweenBeats)
        m_samplesSinceBeat = minSamplesBetweenBeats;

    if (isBeat)
        m_samplesSinceBeat = 0;

    // Update total processed samples (for future beat positions)
    m_totalSamplesProcessed += frames;

#ifdef BEAT_DEBUG
    static int s_frameCounter = 0;
    s_frameCounter++;

    // Log every N frames so we don't spam too much
    const int logEvery = 20;
    if (s_frameCounter % logEvery == 0 || isBeat)
    {
        double bpm = getCurrentBpm();
        qDebug().nospace()
            << "[BeatTracker] frame#" << s_frameCounter
            << " maxAbs=" << maxAbsSample
            << " flux=" << flux
            << " fluxSmoothed=" << fluxForHistory
            << " thr=" << threshold
            << " candidate=" << isBeatCandidate
            << " canTrig=" << canTrigger
            << " isBeat=" << isBeat
            << " bpm=" << bpm;
    }
#endif

    return isBeat;
}
