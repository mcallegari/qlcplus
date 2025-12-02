#pragma once

#include <cstdint>
#include <vector>
#include <fftw3.h>

class BeatTracker
{
public:
    // Constructor: initial format
    BeatTracker(int sampleRate,
                int bufferSize,          // frames per callback
                int channels = 1,
                int historySize = 86,    // ~2s history at ~40–50 fps
                double sensitivity = 1.3);

    ~BeatTracker();

    // Change sample rate / channels / frame size at runtime.
    // Call this from a non-audio thread or while audio is stopped.
    void setFormat(int sampleRate,
                   int bufferSize,
                   int channels);

    // Process interleaved audio buffer.
    // bufferSize = total number of samples (frames * channels)
    // Returns true if beat onset detected in this block.
    bool processAudio(int16_t *buffer, int bufferSize);

    // Optional: set band where we measure flux (Hz)
    // For rock, try: setBand(40.0, 250.0);
    void setBand(double minHz, double maxHz);

    // Optional tuning if you want to tweak at runtime:
    void setSensitivity(double sensitivity) { m_sensitivity = sensitivity; }
    void setFluxSmoothing(double alpha)     { m_fluxSmoothingAlpha = alpha; }
    void setMinBeatInterval(double seconds) { m_minBeatIntervalSec = seconds; }

    // Silence gate threshold on normalized sample amplitude [0..1].
    // Typical values: 0.005–0.02 (~ -46..-34 dBFS).
    void setSilenceThreshold(double t)      { m_silenceThreshold = t; }

    // Estimated BPM from the last few beats.
    // Returns 0.0 if not enough data yet.
    double getCurrentBpm() const;

private:
    int m_sampleRate;
    int m_frameSize;      // frames per block
    int m_fftSize;
    int m_channels;

    double m_sensitivity;

    // FFTW
    double *m_fftInput;
    fftw_complex *m_fftOutput;
    fftw_plan m_fftPlan;

    // Window + magnitude history
    std::vector<double> m_window;
    std::vector<double> m_prevMag;

    // Spectral flux history (adaptive threshold)
    std::vector<double> m_fluxHistory;
    int m_historySize;
    int m_historyIndex;
    bool m_historyFilled;

    double m_lastFlux;

    // Smoothed flux
    double m_fluxSmoothed;
    double m_fluxSmoothingAlpha; // 0=no smoothing, ->1 more smoothing

    // Refractory period
    double m_minBeatIntervalSec; // seconds
    int    m_samplesSinceBeat;   // in samples

    // BPM estimation
    std::vector<double> m_beatIntervalsSec; // last few intervals in seconds
    int                 m_lastBeatSample;   // sample index of last beat
    int                 m_totalSamplesProcessed; // running sample position

    // Silence gate
    double m_silenceThreshold;      // amplitude threshold for silence [0..1]
    int    m_consecutiveSilentFrames;
    int    m_silenceResetFrames;    // after this many silent frames, reset BPM

    // Frequency band in bins
    int m_minBin;
    int m_maxBin;

    // Internal helpers
    void allocateFft();
    void freeFft();
    void initWindow();
    double computeSpectralFlux();
    double computeAdaptiveThreshold() const;
};
