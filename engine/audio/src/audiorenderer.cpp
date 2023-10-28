/*
  Q Light Controller Plus
  audiorenderer.cpp

  Copyright (c) Massimo Callegari

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
#include <QMutexLocker>

#include "audiorenderer.h"
#include "qlcmacros.h"

AudioRenderer::AudioRenderer (QObject* parent)
    : QThread (parent)
    , m_looped(false)
    , m_fadeStep(0.0)
    , m_userStop(true)
    , m_pause(false)
    , m_intensity(1.0)
    , m_currentIntensity(1.0)
    , m_adec(NULL)
    , audioDataRead(0)
    , pendingAudioBytes(0)
{
}

void AudioRenderer::setDecoder(AudioDecoder *adec)
{
    m_adec = adec;
}

void AudioRenderer::adjustIntensity(qreal fraction)
{
    m_intensity = CLAMP(fraction, 0.0, 1.0);
}

bool AudioRenderer::isLooped()
{
    return m_looped;
}

void AudioRenderer::setLooped(bool looped)
{
    m_looped = looped;
}

/*********************************************************************
 * Fade sequences
 *********************************************************************/

void AudioRenderer::setFadeIn(uint fadeTime)
{
    m_fadeStep = 0;
    m_currentIntensity = 1.0;

    if (fadeTime == 0 || m_adec == NULL)
        return;

    quint32 sampleRate = m_adec->audioParameters().sampleRate();
    int channels = m_adec->audioParameters().channels();
    qreal stepsCount = (qreal)fadeTime * ((qreal)(sampleRate * channels) / 1000);
    m_fadeStep = m_intensity / stepsCount;
    m_currentIntensity = 0;

    qDebug() << Q_FUNC_INFO << "stepsCount:" << stepsCount << ", fadeStep:" << m_fadeStep;
}

void AudioRenderer::setFadeOut(uint fadeTime)
{
    if (fadeTime == 0 || m_adec == NULL)
        return;

    quint32 sampleRate = m_adec->audioParameters().sampleRate();
    int channels = m_adec->audioParameters().channels();
    qreal stepsCount = (qreal)fadeTime * ((qreal)(sampleRate * channels) / 1000);
    m_fadeStep = -(m_intensity / stepsCount);

    qDebug() << Q_FUNC_INFO << "stepsCount:" << stepsCount << ", fadeStep:" << m_fadeStep;
}

void AudioRenderer::stop()
{
    m_userStop = true;
    while (this->isRunning())
        usleep(10000);
    m_intensity = 1.0;
    m_currentIntensity = 1.0;
}

/*********************************************************************
 * Thread functions
 *********************************************************************/

void AudioRenderer::run()
{
    qint64 audioDataWritten;
    m_userStop = false;
    audioDataRead = 0;

    int sampleSize = m_adec->audioParameters().sampleSize();
    if (sampleSize > 2)
        sampleSize = 2;

    while (!m_userStop)
    {
        QMutexLocker locker(&m_mutex);

        if (m_pause == false)
        {
            //qDebug() << "Pending audio bytes: " << pendingAudioBytes;
            if (pendingAudioBytes == 0)
            {
                audioDataRead = m_adec->read((char *)audioData, 8192);
                if (audioDataRead == 0)
                {
                    if (m_looped)
                    {
                        m_adec->seek(0);
                        continue;
                    }
                    else
                    {
                        emit endOfStreamReached();
                        return;
                    }
                }
                if (m_intensity != 1.0 || m_fadeStep != 0)
                {
                    //qDebug() << "Intensity" << m_intensity << ", current" << m_currentIntensity << ", fadeStep" << m_fadeStep;

                    for (int i = 0; i < audioDataRead; i+=sampleSize)
                    {
                        qreal scaleFactor = m_intensity;
                        if (m_fadeStep != 0)
                        {
                            m_currentIntensity += m_fadeStep;
                            scaleFactor = m_currentIntensity;
                            if ((m_fadeStep > 0 && m_currentIntensity >= m_intensity) ||
                                (m_fadeStep < 0 && m_currentIntensity <= 0))
                                    m_fadeStep = 0;
                        }
                        if (sampleSize >= 2)
                        {
                            short sample = ((short)audioData[i+1] << 8) + (short)audioData[i];
                            sample *= scaleFactor;
                            audioData[i+1] = (sample >> 8) & 0x00FF;
                            audioData[i] = sample & 0x00FF;
                        }
                        /*
                        else if (sampleSize == 3)
                        {
                            long sample = ((long)audioData[i+2] << 16) + ((long)audioData[i+1] << 8) + (short)audioData[i];
                            sample *= scaleFactor;
                            audioData[i+2] = (sample >> 16) & 0x000000FF;
                            audioData[i+1] = (sample >> 8) & 0x000000FF;
                            audioData[i] = sample & 0x000000FF;
                        }
                        else if (sampleSize == 4)
                        {
                            long sample = ((long)audioData[i+3] << 24) + ((long)audioData[i+2] << 16) +
                                          ((long)audioData[i+1] << 8) + (short)audioData[i];
                            sample *= scaleFactor;
                            audioData[i+3] = (sample >> 24) & 0x000000FF;
                            audioData[i+2] = (sample >> 16) & 0x000000FF;
                            audioData[i+1] = (sample >> 8) & 0x000000FF;
                            audioData[i] = sample & 0x000000FF;
                        }
                        */
                        else // this can be PCM_S8 or unknown. In any case perform byte per byte scaling
                            audioData[i] = (unsigned char)((char)audioData[i] * scaleFactor);
                    }
                }
                audioDataWritten = writeAudio(audioData, audioDataRead);
                if (audioDataWritten < audioDataRead)
                {
                    pendingAudioBytes = audioDataRead - audioDataWritten;
                    usleep(15000);
                }
                if (m_currentIntensity <= 0)
                    emit endOfStreamReached();
            }
            else
            {
                audioDataWritten = writeAudio(audioData + (audioDataRead - pendingAudioBytes), pendingAudioBytes);
                pendingAudioBytes -= audioDataWritten;
                if (audioDataWritten == 0)
                    usleep(15000);
            }
            //qDebug() << "[Cycle] read:" << audioDataRead << ", written:" << audioDataWritten << ", pending:" << pendingAudioBytes;
        }
        else
        {
            usleep(15000);
        }
    }

    reset();
}

