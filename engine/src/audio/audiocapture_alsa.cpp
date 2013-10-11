/*
  Q Light Controller Plus
  audiocapture_alsa.cpp

  Copyright (c) Massimo Callegari

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QDebug>
#include <QSettings>

#include "audiocapture_alsa.h"

AudioCaptureAlsa::AudioCaptureAlsa(QObject * parent)
    : AudioCapture(parent)
{
    m_captureHandle = NULL;
}

AudioCaptureAlsa::~AudioCaptureAlsa()
{
    if (m_captureHandle)
        snd_pcm_close (m_captureHandle);
}

bool AudioCaptureAlsa::initialize(unsigned int sampleRate, quint8 channels, quint16 bufferSize)
{
    snd_pcm_hw_params_t *hw_params;
    QString dev_name = "default";
    int err;

    QSettings settings;
    QVariant var = settings.value(SETTINGS_AUDIO_INPUT_DEVICE);
    if (var.isValid() == true)
        dev_name = var.toString();

    pcm_name = strdup(dev_name.toAscii().data());

    if ((err = snd_pcm_open (&m_captureHandle, pcm_name, SND_PCM_STREAM_CAPTURE, 0)) < 0)
    {
        qWarning("cannot open audio device (%s)\n", snd_strerror (err));
        return false;
    }
    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0)
    {
        qWarning("cannot allocate hardware parameter structure (%s)\n", snd_strerror (err));
        return false;
    }
    if ((err = snd_pcm_hw_params_any (m_captureHandle, hw_params)) < 0)
    {
        qWarning("cannot initialize hardware parameter structure (%s)\n", snd_strerror (err));
        return false;
    }
    if ((err = snd_pcm_hw_params_set_access (m_captureHandle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
    {
        qWarning("cannot set access type (%s)\n", snd_strerror (err));
        return false;
    }
    if ((err = snd_pcm_hw_params_set_format (m_captureHandle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0)
    {
        qWarning("cannot set sample format (%s)\n", snd_strerror (err));
        return false;
    }
    if ((err = snd_pcm_hw_params_set_rate_near (m_captureHandle, hw_params, &sampleRate, 0)) < 0)
    {
        qWarning("cannot set sample rate (%s)\n", snd_strerror (err));
        return false;
    }
    if ((err = snd_pcm_hw_params_set_channels (m_captureHandle, hw_params, channels)) < 0)
    {
        qWarning("cannot set channel count (%s)\n", snd_strerror (err));
        return false;
    }
    if ((err = snd_pcm_hw_params (m_captureHandle, hw_params)) < 0)
    {
        qWarning("cannot set parameters (%s)\n", snd_strerror (err));
        return false;
    }
    snd_pcm_hw_params_free (hw_params);

    if ((err = snd_pcm_prepare (m_captureHandle)) < 0)
    {
        qWarning("cannot prepare audio interface for use (%s)\n", snd_strerror (err));
        return false;
    }

    return AudioCapture::initialize(sampleRate, channels, bufferSize);
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
    int read;
    if ((read = snd_pcm_readi (m_captureHandle, m_audioBuffer, maxSize)) != maxSize)
    {
        qWarning("read from audio interface failed (%s)\n", snd_strerror (read));
        return false;
    }

    //qDebug() << "Audio sample #0:" << m_audioBuffer[0] << ", #max:" << m_audioBuffer[m_captureSize - 1];
    qDebug() << "[ALSA readAudio] " << maxSize << "bytes read";

    return true;
}











