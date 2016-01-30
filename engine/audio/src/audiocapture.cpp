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
#include <QDebug>
#include <qmath.h>

#include "audiocapture.h"

#ifdef HAS_FFTW3
#include "fftw3.h"
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
    , m_fftInputBuffer(NULL)
    , m_fftOutputBuffer(NULL)
{
    int bufferSize = AUDIO_DEFAULT_BUFFER_SIZE;
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

    m_captureSize = bufferSize * m_channels;

    m_audioBuffer = new int16_t[m_captureSize];
    m_fftInputBuffer = new double[m_captureSize];
#ifdef HAS_FFTW3
    m_fftOutputBuffer = fftw_malloc(sizeof(fftw_complex) * m_captureSize);
#endif
}

AudioCapture::~AudioCapture()
{
    // stop() has to be called from the implementation class
    Q_ASSERT(!this->isRunning());

    delete[] m_audioBuffer;
    delete[] m_fftInputBuffer;
#ifdef HAS_FFTW3
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
    double maxMagnitude = 0;
#ifdef HAS_FFTW3
    unsigned int i = 0;
    int subBandWidth = ((m_captureSize * SPECTRUM_MAX_FREQUENCY) / m_sampleRate) / number;

    for (int b = 0; b < number; b++)
    {
        quint64 magnitudeSum = 0;
        for (int s = 0; s < subBandWidth; s++, i++)
        {
            if (i == m_captureSize)
                break;
            magnitudeSum += qSqrt((((fftw_complex*)m_fftOutputBuffer)[i][0] * ((fftw_complex*)m_fftOutputBuffer)[i][0]) +
                                  (((fftw_complex*)m_fftOutputBuffer)[i][1] * ((fftw_complex*)m_fftOutputBuffer)[i][1]));
        }
        double bandMagnitude = (magnitudeSum / subBandWidth);
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
#ifdef HAS_FFTW3
    unsigned int i;
    quint64 pwrSum = 0;

    // 1 ********* Initialize FFTW
    fftw_plan plan_forward;
    plan_forward = fftw_plan_dft_r2c_1d(m_captureSize, m_fftInputBuffer, (fftw_complex*)m_fftOutputBuffer , 0);

    // 2 ********* Apply a window to audio data
    // *********** and convert it to doubles

    for (i = 0; i < m_captureSize; i++)
    {
        if(m_audioBuffer[i] < 0)
            pwrSum += -1 * m_audioBuffer[i];
        else
            pwrSum += m_audioBuffer[i];

#ifdef USE_BLACKMAN
        double a0 = (1-0.16)/2;
        double a1 = 0.5;
        double a2 = 0.16/2;
        m_fftInputBuffer[i] = m_audioBuffer[i]  * (a0 - a1 * qCos((M_2PI * i) / (m_captureSize - 1)) +
                              a2 * qCos((2 * M_2PI * i) / (m_captureSize - 1)));
#endif
#ifdef USE_HANNING
        m_fftInputBuffer[i] = m_audioBuffer[i] * (0.5 * (1.00 - qCos((M_2PI * i) / (m_captureSize - 1))));
#endif
#ifdef USE_NO_WINDOW
        m_fftInputBuffer[i] = (double)m_audioBuffer[i];
#endif
    }

    // 3 ********* Perform FFT
    fftw_execute(plan_forward);
    fftw_destroy_plan(plan_forward);

    // 4 ********* Clear FFT noise
#ifdef CLEAR_FFT_NOISE
    //We delete some values since these will ruin our output
    for (int n = 0; n < 5; n++)
    {
        ((fftw_complex*)m_fftOutputBuffer)[n][0] = 0;
        ((fftw_complex*)m_fftOutputBuffer)[n][1] = 0;
    }
#endif

    // 5 ********* Calculate the average signal power
    m_signalPower = pwrSum / m_captureSize;

    // 6 ********* Calculate vector magnitude
    foreach(int barsNumber, m_fftMagnitudeMap.keys())
    {
        double maxMagnitude = fillBandsData(barsNumber);
        emit dataProcessed(m_fftMagnitudeMap[barsNumber].m_fftMagnitudeBuffer.data(),
                           m_fftMagnitudeMap[barsNumber].m_fftMagnitudeBuffer.size(),
                           maxMagnitude, m_signalPower);
    }
#endif
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
