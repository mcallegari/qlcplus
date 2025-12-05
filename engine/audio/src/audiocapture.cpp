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
#ifdef NEW_TRACKER
  #include "beattracker.h"
#else
  #include "beattracking.h"
#endif

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
 #ifdef NEW_TRACKER
    m_beatTracker = new BeatTracker(m_sampleRate, m_bufferSize, m_channels, 86, 1.3);
    m_beatTracker->setBand(40.0, 400.0);      // bit wider band for now
    m_beatTracker->setFluxSmoothing(0.6);     // less smoothing
    m_beatTracker->setMinBeatInterval(0.20);  // ~300 BPM max
 #else
    m_beatTracker = new BeatTracking(2);
 #endif
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

int AudioCapture::defaultBarsNumber()
{
    return FREQ_SUBBANDS_DEFAULT_NUMBER;
}

void AudioCapture::registerBandsNumber(int number)
{
    qDebug() << "[AudioCapture] registering" << number << "bands";

    QMutexLocker locker(&m_mutex);

    bool firstBand = m_fftMagnitudeMap.isEmpty();
    if (number > 0 && number <= FREQ_SUBBANDS_MAX_NUMBER)
    {
        if (m_fftMagnitudeMap.contains(number) == false)
        {
            BandsData newBands;
            newBands.m_registerCounter = 1;
            newBands.m_fftMagnitudeBuffer = QVector<double>(number);
            m_fftMagnitudeMap[number] = newBands;
        }
        else
            m_fftMagnitudeMap[number].m_registerCounter++;

        if (firstBand)
        {
            locker.unlock();
            start();
        }
    }
}

void AudioCapture::unregisterBandsNumber(int number)
{
    qDebug() << "[AudioCapture] unregistering" << number << "bands";

    QMutexLocker locker(&m_mutex);

    if (m_fftMagnitudeMap.contains(number))
    {
        m_fftMagnitudeMap[number].m_registerCounter--;
        if (m_fftMagnitudeMap[number].m_registerCounter == 0)
            m_fftMagnitudeMap.remove(number);

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

double AudioCapture::fillBandsData(int number)
{
    // m_fftOutputBuffer contains the real and imaginary data of a spectrum
    // representing all the frequencies from 0 to m_sampleRate Hz.
    // I will just consider 0 to 5000Hz and will calculate average magnitude
    // for the number of desired bands.
    double maxMagnitude = 0.;
#ifdef HAS_FFTW3
    unsigned int i = 1; // skip DC bin
    int subBandWidth = ((m_bufferSize * SPECTRUM_MAX_FREQUENCY) / m_sampleRate) / number;

    for (int b = 0; b < number; b++)
    {
        double magnitudeSum = 0.;
        for (int s = 0; s < subBandWidth; s++, i++)
        {
            if (i == m_bufferSize)
                break;
            magnitudeSum += qSqrt((((fftw_complex*)m_fftOutputBuffer)[i][0] * ((fftw_complex*)m_fftOutputBuffer)[i][0]) +
                                  (((fftw_complex*)m_fftOutputBuffer)[i][1] * ((fftw_complex*)m_fftOutputBuffer)[i][1]));
        }
        double bandMagnitude = (magnitudeSum / (subBandWidth * M_2PI));
        m_fftMagnitudeMap[number].m_fftMagnitudeBuffer[b] = bandMagnitude;
        if (maxMagnitude < bandMagnitude)
            maxMagnitude = bandMagnitude;
    }
#else
    Q_UNUSED(number)
#endif
    return maxMagnitude;
}

void AudioCapture::processData()
{
    unsigned int i, j;
    double pwrSum = 0.;
    double maxMagnitude = 0.;

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
        quint32 power = 0;
        for (int barsNumber : m_fftMagnitudeMap.keys())
        {
            // Ensure the buffer exists and is zeroed
            auto &buf = m_fftMagnitudeMap[barsNumber].m_fftMagnitudeBuffer;
            if (buf.size() != barsNumber)
                buf = QVector<double>(barsNumber);
            else
                buf.fill(0.0);
            emit dataProcessed(buf.data(), buf.size(), maxMagnitude, power);
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
    for (int n = 0; n < 5; n++)
    {
        ((fftw_complex*)m_fftOutputBuffer)[n][0] = 0;
        ((fftw_complex*)m_fftOutputBuffer)[n][1] = 0;
    }
#endif
#endif

    // 5) Fill per-band magnitudes and compute power
    for (int barsNumber : m_fftMagnitudeMap.keys())
    {
        maxMagnitude = fillBandsData(barsNumber); // fills & returns max per-band
        pwrSum = 0.;
        for (int n = 0; n < barsNumber; n++)
            pwrSum += m_fftMagnitudeMap[barsNumber].m_fftMagnitudeBuffer[n];

        m_signalPower = 32768 * pwrSum * qSqrt(M_2PI) / (double)barsNumber;
        emit dataProcessed(m_fftMagnitudeMap[barsNumber].m_fftMagnitudeBuffer.data(),
                           m_fftMagnitudeMap[barsNumber].m_fftMagnitudeBuffer.size(),
                           maxMagnitude, m_signalPower);
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
#ifdef FULL_BEATTRACKING
                if (m_beatTracker->processAudio(m_audioBuffer, m_captureSize))
                    emit beatDetected();
#endif
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
