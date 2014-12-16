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

#include <QDebug>
#include <qmath.h>

#include "audiocapture.h"

#include "fftw3.h"

#define USE_HANNING
#define CLEAR_FFT_NOISE

#define M_2PI       6.28318530718           /* 2*pi */

AudioCapture::AudioCapture (QObject* parent)
    : QThread (parent)
    , m_userStop(true)
    , m_pause(false)
    , m_isInitialized(false)
    , m_captureSize(0)
    , m_sampleRate(0)
    , m_channels(0)
    , m_audioBuffer(NULL)
    , m_fftInputBuffer(NULL)
    , m_fftOutputBuffer(NULL)
{
}

AudioCapture::~AudioCapture()
{
    if (m_audioBuffer)
        delete[] m_audioBuffer;
    if (m_fftInputBuffer)
        delete[] m_fftInputBuffer;
    if (m_fftOutputBuffer)
        fftw_free(m_fftOutputBuffer);
}

int AudioCapture::defaultBarsNumber()
{
    return FREQ_SUBBANDS_DEFAULT_NUMBER;
}

void AudioCapture::registerBandsNumber(int number)
{
    if (number > 0 && number < FREQ_SUBBANDS_MAX_NUMBER)
    {
        qDebug() << "[AudioCapture] registering" << number << "bands";
        if (m_fftMagnitudeMap.contains(number) == false)
        {
            BandsData newBands;
            newBands.m_registerCounter = 1;
            newBands.m_fftMagnitudeBuffer = QVector<double>(number);
            m_fftMagnitudeMap[number] = newBands;
        }
        else
            m_fftMagnitudeMap[number].m_registerCounter++;
    }
}

void AudioCapture::unregisterBandsNumber(int number)
{
    if (m_fftMagnitudeMap.contains(number))
    {
        qDebug() << "[AudioCapture] unregistering" << number << "bands";
        m_fftMagnitudeMap[number].m_registerCounter--;
        if (m_fftMagnitudeMap[number].m_registerCounter == 0)
            m_fftMagnitudeMap.remove(number);
    }
}

bool AudioCapture::isInitialized()
{
    return m_isInitialized;
}

bool AudioCapture::initialize(unsigned int sampleRate, quint8 channels, quint16 bufferSize)
{
    Q_UNUSED(sampleRate)

    m_captureSize = bufferSize * channels;
    m_sampleRate = sampleRate;
    m_channels = channels;

    m_audioBuffer = new int16_t[m_captureSize];
    m_fftInputBuffer = new double[m_captureSize];
    m_fftOutputBuffer = fftw_malloc(sizeof(fftw_complex) * m_captureSize);

    m_isInitialized = true;

    return true;
}

void AudioCapture::stop()
{
    if (m_fftMagnitudeMap.isEmpty())
    {
        qDebug() << "[AudioCapture] No clients need us anymore. Shutting down...";
        m_userStop = true;
        while (this->isRunning())
            usleep(10000);
    }
}

double AudioCapture::fillBandsData(int number)
{
    // m_fftOutputBuffer contains the real and imaginary data of a spectrum
    // representing all the frequencies from 0 to m_sampleRate Hz.
    // I will just consider 0 to 5000Hz and will calculate average magnitude
    // for the number of desired bands.
    unsigned int i = 0;
    int subBandWidth = ((m_captureSize * SPECTRUM_MAX_FREQUENCY) / m_sampleRate) / number;
    double maxMagnitude = 0;

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
    return maxMagnitude;
}

void AudioCapture::processData()
{
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
}

void AudioCapture::run()
{
    m_userStop = false;

    while (!m_userStop)
    {
        m_mutex.lock();
        if (m_pause == false && m_captureSize != 0)
        {
            if (readAudio(m_captureSize) == true)
            {
                processData();
            }
            else
            {
                //qDebug() << "Error reading data from audio source";
                usleep(5000);
            }

        }
        else
            usleep(15000);
        m_mutex.unlock();
    }
}
