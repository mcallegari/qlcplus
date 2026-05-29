/*
  Q Light Controller Plus
  audiocapture.cpp

  Copyright (c) Massimo Callegari
  based on libbeat code by Maximilian Güntner

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

#include <QSettings>
#include <QDateTime>
#include <QDebug>
#include <qmath.h>

#include "audiocapture.h"
#include "beattracker.h"
#include "spectrumgrid.h"

#define USE_HANNING
#define CLEAR_FFT_NOISE

#define M_2PI       6.28318530718           /* 2*pi */

AudioCapture::AudioCapture (QObject* parent)
    : QThread (parent)
    , m_userStop(true)
    , m_pause(false)
    , m_captureSize(0)
    , m_sampleRate(0)
    , m_channels(0)
    , m_audioBuffer(NULL)
    , m_audioMixdown(NULL)
    , m_signalPower(0)
    , m_smoothedSignalPower(0.0)
    , m_fftInputBuffer(NULL)
    , m_fftOutputBuffer(NULL)
{
    m_bufferSize = AUDIO_DEFAULT_BUFFER_SIZE;
    m_sampleRate = AUDIO_DEFAULT_SAMPLE_RATE;
    m_channels = AUDIO_DEFAULT_CHANNELS;

    QSettings settings;
    QVariant var = settings.value(SETTINGS_AUDIO_INPUT_SRATE);

    if (var.isValid() == true)
        m_sampleRate = var.toInt();

    var = settings.value(SETTINGS_AUDIO_INPUT_CHANNELS);

    if (var.isValid() == true)
        m_channels = var.toInt();

    qDebug() << "[AudioCapture] initialize" << m_sampleRate << m_channels;

    m_captureSize = m_bufferSize * m_channels;

    m_audioBuffer = new int16_t[m_captureSize];
    m_audioMixdown = new int16_t[m_bufferSize];
    m_fftInputBuffer = new double[m_bufferSize];
#ifdef HAS_FFTW3
    m_fftOutputBuffer = fftw_malloc(sizeof(fftw_complex) * m_bufferSize);

    // Init FFTW
    m_plan_forward = fftw_plan_dft_r2c_1d(m_bufferSize, m_fftInputBuffer,
                                          reinterpret_cast<fftw_complex*>(m_fftOutputBuffer), 0);
#endif
    m_beatTracker = new BeatTracker(m_sampleRate, m_bufferSize, m_channels, 86, 1.3);
    m_beatTracker->setBand(40.0, 400.0);      // bit wider band for now
    m_beatTracker->setFluxSmoothing(0.6);     // less smoothing
    m_beatTracker->setMinBeatInterval(0.20);  // ~300 BPM max
}

AudioCapture::~AudioCapture()
{
    // stop() has to be called from the implementation class
    Q_ASSERT(!this->isRunning());

    delete[] m_audioBuffer;
    delete[] m_audioMixdown;
    delete[] m_fftInputBuffer;
#ifdef HAS_FFTW3
    fftw_destroy_plan(m_plan_forward);

    if (m_fftOutputBuffer)
        fftw_free(m_fftOutputBuffer);
#endif
}

int AudioCapture::defaultBarsNumber() const
{
    return FREQ_SUBBANDS_DEFAULT_NUMBER;
}

void AudioCapture::registerBandsNumber(int number)
{
    registerBands(number, SpectrumGridMode::LogUniform);
}

void AudioCapture::unregisterBandsNumber(int number)
{
    unregisterBands(number, SpectrumGridMode::LogUniform);
}

void AudioCapture::registerBands(int number, SpectrumGridMode mode, double lowBandGamma)
{
    qDebug() << "[AudioCapture] registering" << number << "bands, grid" << int(mode)
             << "gamma" << lowBandGamma;

    QMutexLocker locker(&m_mutex);

    const quint32 key = spectrumBandsRegistryKey(number, mode, lowBandGamma);
    bool firstBand = m_fftMagnitudeMap.isEmpty();
    if (number > 0 && number <= FREQ_SUBBANDS_MAX_NUMBER)
    {
        if (m_fftMagnitudeMap.contains(key) == false)
        {
            BandsData newBands;
            newBands.m_registerCounter = 1;
            newBands.m_fftMagnitudeBuffer = QVector<double>(number);
            m_fftMagnitudeMap[key] = newBands;
        }
        else
            m_fftMagnitudeMap[key].m_registerCounter++;

        if (firstBand)
        {
            locker.unlock();
            start();
        }
    }
}

void AudioCapture::unregisterBands(int number, SpectrumGridMode mode, double lowBandGamma)
{
    qDebug() << "[AudioCapture] unregistering" << number << "bands, grid" << int(mode)
             << "gamma" << lowBandGamma;

    QMutexLocker locker(&m_mutex);

    const quint32 key = spectrumBandsRegistryKey(number, mode, lowBandGamma);
    if (m_fftMagnitudeMap.contains(key))
    {
        m_fftMagnitudeMap[key].m_registerCounter--;
        if (m_fftMagnitudeMap[key].m_registerCounter == 0)
            m_fftMagnitudeMap.remove(key);

        if (m_fftMagnitudeMap.isEmpty())
        {
            locker.unlock();
            stop();
        }
    }
}

void AudioCapture::stop()
{
    qDebug() << "[AudioCapture] stop capture";
    while (this->isRunning())
    {
        m_userStop = true;
        usleep(10000);
    }
}

double AudioCapture::fillBandsData(quint32 registryKey)
{
    const int number = spectrumBandsCountFromKey(registryKey);
    const SpectrumGridMode gridMode = spectrumGridModeFromKey(registryKey);
    const double lowBandGamma = spectrumLowBandGammaFromKey(registryKey);

    double maxMagnitude = 0.;
#ifdef HAS_FFTW3
    fftw_complex *fft = reinterpret_cast<fftw_complex*>(m_fftOutputBuffer);
    const int maxBin = int(m_bufferSize / 2); // r2c valid bins: 0..N/2
    const double nyquist = double(m_sampleRate) / 2.0;
    const double minFreq = qMax(1.0, double(SPECTRUM_MIN_FREQUENCY));
    const double maxFreq = qMin(double(SPECTRUM_MAX_FREQUENCY), nyquist);

    if (number <= 0 || maxBin <= 1 || maxFreq <= minFreq)
    {
        if (number > 0 && m_fftMagnitudeMap.contains(registryKey))
            m_fftMagnitudeMap[registryKey].m_fftMagnitudeBuffer.fill(0.0);
        return 0.0;
    }

    const QVector<double> edges = computeSpectrumBandEdges(number, minFreq, maxFreq, gridMode, lowBandGamma);
    if (edges.size() != number + 1)
    {
        m_fftMagnitudeMap[registryKey].m_fftMagnitudeBuffer.fill(0.0);
        return 0.0;
    }

    // Partition FFT bins without overlap. Narrow Hz bands used to map to the same
    // bin (endBin <= startBin forced +1), producing identical levels ("shelves").
    QVector<int> binBoundaries(number + 1);
    const double binHz = double(m_sampleRate) / double(m_bufferSize);
    binBoundaries[0] = qBound(1, int(qFloor(edges[0] / binHz)), maxBin);
    for (int b = 1; b < number; ++b)
    {
        const int next = qBound(1, int(qFloor(edges[b] / binHz)), maxBin);
        binBoundaries[b] = qMax(next, binBoundaries[b - 1]);
    }
    binBoundaries[number] = maxBin + 1;

    auto &buf = m_fftMagnitudeMap[registryKey].m_fftMagnitudeBuffer;
    for (int b = 0; b < number; b++)
    {
        const int startBin = binBoundaries[b];
        int endBin = binBoundaries[b + 1];
        if (b == number - 1)
            endBin = maxBin + 1;

        if (endBin <= startBin)
        {
            buf[b] = 0.0;
            continue;
        }

        double magnitudeSum = 0.0;
        for (int i = startBin; i < endBin; i++)
            magnitudeSum += qSqrt((fft[i][0] * fft[i][0]) + (fft[i][1] * fft[i][1]));

        const int bandWidth = endBin - startBin;
        const double bandMagnitude = magnitudeSum / (double(bandWidth) * M_2PI);
        buf[b] = bandMagnitude;
        if (maxMagnitude < bandMagnitude)
            maxMagnitude = bandMagnitude;
    }
#else
    Q_UNUSED(registryKey)
#endif
    return maxMagnitude;
}

void AudioCapture::processData()
{
    unsigned int i, j;
    const double frameSec = (m_sampleRate > 0) ? (double(m_bufferSize) / double(m_sampleRate)) : 0.0;
    static constexpr double kAttackTauSec = 0.040;  // fast rise
    static constexpr double kReleaseTauSec = 0.200; // slower fall
    const double attackAlpha = (frameSec > 0.0) ? (1.0 - qExp(-frameSec / kAttackTauSec)) : 1.0;
    const double releaseAlpha = (frameSec > 0.0) ? (1.0 - qExp(-frameSec / kReleaseTauSec)) : 1.0;

    auto smoothPower = [&](double rawPower) -> quint32
    {
        rawPower = qBound(0.0, rawPower, 32767.0);
        const double alpha = (rawPower > m_smoothedSignalPower) ? attackAlpha : releaseAlpha;
        m_smoothedSignalPower += alpha * (rawPower - m_smoothedSignalPower);
        m_smoothedSignalPower = qBound(0.0, m_smoothedSignalPower, 32767.0);
        return quint32(qRound(m_smoothedSignalPower));
    };

    // 2) Mix down to mono (int16 -> int16)
    for (i = 0; i < m_bufferSize; i++)
    {
        m_audioMixdown[i] = 0;
        for (j = 0; j < m_channels; j++)
            m_audioMixdown[i] += m_audioBuffer[i*m_channels + j] / m_channels;
    }

    // 2a) DC removal + RMS (silence gate)
    // Compute mean (DC)
    long long acc = 0;
    for (i = 0; i < m_bufferSize; ++i)
        acc += m_audioMixdown[i];
    const double mean = double(acc) / double(m_bufferSize);

    // Remove DC, compute RMS in one pass (normalize to [-1,1])
    double sumSq = 0.0;
    for (i = 0; i < m_bufferSize; ++i)
    {
        const double x = (double(m_audioMixdown[i]) - mean) / 32768.0;
        sumSq += x * x;
        m_fftInputBuffer[i] = x; // will be windowed right below
    }
    const double rms = qSqrt(sumSq / double(m_bufferSize));

    // If the frame is effectively silent, emit zeros and bail early.
    // Threshold is tunable; ~0.002 ≈ -54 dBFS works well for typical PC inputs.
    static constexpr double kSilenceRms = 0.002;
    if (rms < kSilenceRms)
    {
        double maxMagnitude = 0.0;
        quint32 power = smoothPower(0.0);
        m_signalPower = power;
        for (quint32 key : m_fftMagnitudeMap.keys())
        {
            const int barsNumber = spectrumBandsCountFromKey(key);
            auto &buf = m_fftMagnitudeMap[key].m_fftMagnitudeBuffer;
            if (buf.size() != barsNumber)
                buf = QVector<double>(barsNumber);
            else
                buf.fill(0.0);
            emit dataProcessed(buf.data(), buf.size(), maxMagnitude, power,
                               int(spectrumGridModeFromKey(key)),
                               spectrumLowBandGammaFromKey(key));
        }
        return;
    }

    // 2b) Apply window to doubles already placed in m_fftInputBuffer
#ifdef USE_BLACKMAN
    const double a0 = (1-0.16)/2, a1 = 0.5, a2 = 0.16/2;
    for (i = 0; i < bufferSize; i++)
        m_fftInputBuffer[i] = m_fftInputBuffer[i] *
                              (a0 - a1 * qCos((M_2PI * i) / (bufferSize - 1)) +
                               a2 * qCos((2 * M_2PI * i) / (bufferSize - 1)));
#endif
#ifdef USE_HANNING
    for (i = 0; i < m_bufferSize; i++)
        m_fftInputBuffer[i] = m_fftInputBuffer[i] *
                              (0.5 * (1.0 - qCos((M_2PI * i) / (m_bufferSize - 1))));
#endif
#ifdef USE_NO_WINDOW
    // already filled: keep as-is
#endif

#ifdef HAS_FFTW3
    // 3) FFT
    fftw_execute(m_plan_forward);

    // 4) Clear low-bin FFT noise
#ifdef CLEAR_FFT_NOISE
    // Clear only the very-low-frequency floor (incl. DC),
    // otherwise log-spaced low bands get entirely muted.
    const double noiseFloorHz = 20.0;
    const int maxBin = int(m_bufferSize / 2);
    const int noiseBins = qBound(1, int(qRound((noiseFloorHz * m_bufferSize) / double(m_sampleRate))), maxBin);
    for (int n = 0; n < noiseBins; n++)
    {
        ((fftw_complex*)m_fftOutputBuffer)[n][0] = 0;
        ((fftw_complex*)m_fftOutputBuffer)[n][1] = 0;
    }
#endif
#endif

    // 5) Fill per-band magnitudes and compute power
    double pwrSum = 0.;
    double maxMagnitude = 0.;
    for (quint32 key : m_fftMagnitudeMap.keys())
    {
        const int barsNumber = spectrumBandsCountFromKey(key);
        maxMagnitude = fillBandsData(key);
        pwrSum = 0.;
        for (int n = 0; n < barsNumber; n++)
            pwrSum += m_fftMagnitudeMap[key].m_fftMagnitudeBuffer[n];

        const double rawPower = 32768.0 * pwrSum * qSqrt(M_2PI) / double(barsNumber);
        m_signalPower = smoothPower(rawPower);
        emit dataProcessed(m_fftMagnitudeMap[key].m_fftMagnitudeBuffer.data(),
                           m_fftMagnitudeMap[key].m_fftMagnitudeBuffer.size(),
                           maxMagnitude, m_signalPower,
                           int(spectrumGridModeFromKey(key)),
                           spectrumLowBandGammaFromKey(key));
    }
}

void AudioCapture::run()
{
    qDebug() << "[AudioCapture] start capture";

    m_userStop = false;

    if (!initialize())
    {
        qWarning() << "[AudioCapture] Could not initialize audio capture, abandon";
        return;
    }

    while (!m_userStop)
    {
        if (m_pause == false && m_captureSize != 0)
        {
            if (readAudio(m_captureSize) == true)
            {
                QMutexLocker locker(&m_mutex);
                processData();

                if (m_beatTracker->processAudio(m_audioBuffer, m_captureSize))
                    emit beatDetected();
            }
            else
            {
                //qDebug() << "Error reading data from audio source";
                QThread::msleep(5);
            }
        }
        else
        {
            QThread::msleep(15);
        }

        QThread::yieldCurrentThread();
    }

    uninitialize();
}
