/*
  Q Light Controller Plus
  audiocapture_alsa.cpp

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
#include <QSettings>

#include "audiocapture_alsa.h"

AudioCaptureAlsa::AudioCaptureAlsa(QObject * parent)
    : AudioCapture(parent)
    , m_captureHandle(NULL)
{
}

AudioCaptureAlsa::~AudioCaptureAlsa()
{
    stop();
    Q_ASSERT(m_captureHandle == NULL);
}

bool AudioCaptureAlsa::initialize()
{
    snd_pcm_hw_params_t *hw_params = NULL;
    QString dev_name = "default";
    int err;

    QSettings settings;
    QVariant var = settings.value(SETTINGS_AUDIO_INPUT_DEVICE);
    if (var.isValid() == true)
        dev_name = var.toString();

    pcm_name = strdup(dev_name.toLatin1().data());

    qDebug() << "AudioCaptureAlsa: initializing device " << pcm_name;

    Q_ASSERT(m_captureHandle == NULL);

    if ((err = snd_pcm_open (&m_captureHandle, pcm_name, SND_PCM_STREAM_CAPTURE, 0)) < 0)
        qWarning("cannot open audio device (%s)\n", snd_strerror (err));
    else if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0)
        qWarning("cannot allocate hardware parameter structure (%s)\n", snd_strerror (err));
    else if ((err = snd_pcm_hw_params_any (m_captureHandle, hw_params)) < 0)
        qWarning("cannot initialize hardware parameter structure (%s)\n", snd_strerror (err));
    else if ((err = snd_pcm_hw_params_set_access (m_captureHandle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
        qWarning("cannot set access type (%s)\n", snd_strerror (err));
    else if ((err = snd_pcm_hw_params_set_format (m_captureHandle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0)
        qWarning("cannot set sample format (%s)\n", snd_strerror (err));
    else if ((err = snd_pcm_hw_params_set_rate_near (m_captureHandle, hw_params, &m_sampleRate, 0)) < 0)
        qWarning("cannot set sample rate (%s)\n", snd_strerror (err));
    else if ((err = snd_pcm_hw_params_set_channels (m_captureHandle, hw_params, m_channels)) < 0)
        qWarning("cannot set channel count to %d (%s)\n", m_channels, snd_strerror (err));
    else if ((err = snd_pcm_hw_params (m_captureHandle, hw_params)) < 0)
        qWarning("cannot set parameters (%s)\n", snd_strerror (err));
    else if ((err = snd_pcm_prepare (m_captureHandle)) < 0)
        qWarning("cannot prepare audio interface for use (%s)\n", snd_strerror (err));

    if (hw_params)
        snd_pcm_hw_params_free (hw_params);

    if (err < 0)
    {
        if (m_captureHandle != NULL)
        {
            snd_pcm_close(m_captureHandle);
            m_captureHandle = NULL;
        }
        return false;
    }

    return true;
}

void AudioCaptureAlsa::uninitialize()
{
    Q_ASSERT(m_captureHandle != NULL);
    snd_pcm_close(m_captureHandle);
    m_captureHandle = NULL;
}

qint64 AudioCaptureAlsa::latency()
{
    return 0; // TODO
}

void AudioCaptureAlsa::suspend()
{
}

void AudioCaptureAlsa::resume()
{
}

bool AudioCaptureAlsa::readAudio(int maxSize)
{
    Q_ASSERT(m_captureHandle != NULL);

    int read;
    if ((read = snd_pcm_readi (m_captureHandle, m_audioBuffer, maxSize)) != maxSize)
    {
        qWarning() << "[ALSA readAudio] read from audio interface failed (" << snd_strerror(read) << ")";
        return false;
    }

    //qDebug() << "Audio sample #0:" << m_audioBuffer[0] << ", #max:" << m_audioBuffer[m_captureSize - 1];
    qDebug() << "[ALSA readAudio] " << maxSize << "bytes read";

    return true;
}











