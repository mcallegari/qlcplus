/*
  Q Light Controller Plus
  audiorenderer_alsa.cpp

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

/******************************************************
 *   Based on qmmp project                            *
 *                                                    *
 *   Copyright (C) 2007-2009 by Ilya Kotov            *
 *   forkotov02@hotmail.ru                            *
 ******************************************************/

#include <QDebug>
#include <QString>
#include <QSettings>

#include "audiorenderer_alsa.h"

AudioRendererAlsa::AudioRendererAlsa(QString device, QObject * parent)
    : AudioRenderer(parent)
{
    QString dev_name = "default";
    if (device.isEmpty())
    {
        QSettings settings;
        QVariant var = settings.value(SETTINGS_AUDIO_OUTPUT_DEVICE);
        if (var.isValid() == true)
            dev_name = var.toString();
    }
    else
        dev_name = device;

    m_inited = false;
    m_use_mmap = false;
    pcm_name = strdup(dev_name.toLatin1().data());
    pcm_handle = NULL;
    m_prebuf = NULL;
    m_prebuf_size = 0;
    m_prebuf_fill = 0;
    m_can_pause = false;
}

AudioRendererAlsa::~AudioRendererAlsa()
{
    qDebug() << Q_FUNC_INFO;
    uninitialize();
    free (pcm_name);
}

bool AudioRendererAlsa::initialize(quint32 freq, int chan, AudioFormat format)
{
    m_inited = false;

    if (pcm_handle)
        return false;

    if (snd_pcm_open(&pcm_handle, pcm_name, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK) < 0)
    {
        qWarning ("OutputALSA: Error opening PCM device %s", pcm_name);
        // if it fails, fallback to the default device
        pcm_name = strdup("default");
        if (snd_pcm_open(&pcm_handle, pcm_name, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK) < 0)
        {
            qWarning ("OutputALSA: Error opening default PCM device. Giving up.");
            return false;
        }
    }

    uint rate = freq; /* Sample rate */
    uint exact_rate = freq;   /* Sample rate returned by */

    uint buffer_time = 500000;
    uint period_time = 100000;

    snd_pcm_hw_params_t *hwparams = 0;
    snd_pcm_sw_params_t *swparams = 0;
    int err; //alsa error code

    //hw params
    snd_pcm_hw_params_alloca(&hwparams);
    if ((err = snd_pcm_hw_params_any(pcm_handle, hwparams)) < 0)
    {
        qWarning("OutputALSA: Can not read configuration for PCM device: %s", snd_strerror(err));
        return false;
    }
    if (m_use_mmap)
    {
        if ((err = snd_pcm_hw_params_set_access(pcm_handle, hwparams, SND_PCM_ACCESS_MMAP_INTERLEAVED)) < 0)
        {
            qWarning("OutputALSA: Error setting mmap access: %s", snd_strerror(err));
            m_use_mmap = false;
        }
    }
    if (!m_use_mmap)
    {
        if ((err = snd_pcm_hw_params_set_access(pcm_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
        {
            qWarning("OutputALSA: Error setting access: %s", snd_strerror(err));
            return false;
        }
    }

    snd_pcm_format_t alsa_format = SND_PCM_FORMAT_UNKNOWN;
    switch (format)
    {
    case PCM_S8:
        alsa_format = SND_PCM_FORMAT_S8;
        break;
    case PCM_S16LE:
        alsa_format = SND_PCM_FORMAT_S16_LE;
        break;
    case PCM_S24LE:
        alsa_format = SND_PCM_FORMAT_S24_LE;
        break;
    case PCM_S32LE:
        alsa_format = SND_PCM_FORMAT_S32_LE;
        break;
    default:
        qWarning("OutputALSA: unsupported format detected");
        return false;
    }
    if ((err = snd_pcm_hw_params_set_format(pcm_handle, hwparams, alsa_format)) < 0)
    {
        qDebug("OutputALSA: Error setting format: %s", snd_strerror(err));
        return false;
    }
    exact_rate = rate;

    if ((err = snd_pcm_hw_params_set_rate_near(pcm_handle, hwparams, &exact_rate, 0)) < 0)
    {
        qWarning("OutputALSA: Error setting rate: %s", snd_strerror(err));
        return false;
    }
    if (rate != exact_rate)
    {
        qWarning("OutputALSA: The rate %d Hz is not supported by your hardware.\n==> Using %d Hz instead.", rate, exact_rate);
    }
    uint c = chan;
    if ((err = snd_pcm_hw_params_set_channels_near(pcm_handle, hwparams, &c)) < 0)
    {
        qWarning("OutputALSA: Error setting channels: %s", snd_strerror(err));
        return false;
    }
    if ((err = snd_pcm_hw_params_set_period_time_near(pcm_handle, hwparams, &period_time ,0)) < 0)
    {
        qWarning("OutputALSA: Error setting period time: %s", snd_strerror(err));
        return false;
    }
    if ((err = snd_pcm_hw_params_set_buffer_time_near(pcm_handle, hwparams, &buffer_time ,0)) < 0)
    {
        qWarning("OutputALSA: Error setting buffer time: %s", snd_strerror(err));
        return false;
    }
    if ((err = snd_pcm_hw_params(pcm_handle, hwparams)) < 0)
    {
        qWarning("OutputALSA: Error setting HW params: %s", snd_strerror(err));
        return false;
    }
    //read some alsa parameters
    snd_pcm_uframes_t buffer_size = 0;
    snd_pcm_uframes_t period_size = 0;
    if ((err = snd_pcm_hw_params_get_buffer_size(hwparams, &buffer_size)) < 0)
    {
        qWarning("OutputALSA: Error reading buffer size: %s", snd_strerror(err));
        return false;
    }
    if ((err = snd_pcm_hw_params_get_period_size(hwparams, &period_size, 0)) < 0)
    {
        qWarning("OutputALSA: Error reading period size: %s", snd_strerror(err));
        return false;
    }
    //swparams
    snd_pcm_sw_params_alloca(&swparams);
    snd_pcm_sw_params_current(pcm_handle, swparams);
    if ((err = snd_pcm_sw_params_set_start_threshold(pcm_handle, swparams,
               buffer_size - period_size)) < 0)
        qWarning("OutputALSA: Error setting threshold: %s", snd_strerror(err));
    if ((err = snd_pcm_sw_params(pcm_handle, swparams)) < 0)
    {
        qWarning("OutputALSA: Error setting SW params: %s", snd_strerror(err));
        return false;
    }
    //setup needed values
    m_bits_per_frame = snd_pcm_format_physical_width(alsa_format) * chan;
    m_chunk_size = period_size;
    m_can_pause = snd_pcm_hw_params_can_pause(hwparams);

    qDebug("OutputALSA: can pause: %d", m_can_pause);

    //create alsa prebuffer;
    m_prebuf_size = m_bits_per_frame * m_chunk_size / 8;
    m_prebuf = (uchar *)malloc(m_prebuf_size);

    m_inited = true;
    return true;
}

qint64 AudioRendererAlsa::latency()
{
    //return m_prebuf_fill * 1000 / sampleRate() / channels() / sampleSize();
    return 0;
}

QList<AudioDeviceInfo> AudioRendererAlsa::getDevicesInfo()
{
    QList<AudioDeviceInfo> devList;
    int cardIdx = -1;

    while (snd_card_next(&cardIdx) == 0 && cardIdx >= 0)
    {
        snd_ctl_t *cardHandle;
        snd_ctl_card_info_t *cardInfo;
        char str[64];
        int devIdx = -1;
        int err;

        // Open this card's control interface. We specify only the card number -- not
        // any device nor sub-device too
        sprintf(str, "hw:%i", cardIdx);
        if ((err = snd_ctl_open(&cardHandle, str, 0)) < 0)
        {
            qWarning("Can't open card %d: %s\n", cardIdx, snd_strerror(err));
            continue;
        }

        // We need to get a snd_ctl_card_info_t. Just alloc it on the stack
        snd_ctl_card_info_alloca(&cardInfo);

        // Tell ALSA to fill in our snd_ctl_card_info_t with info about this card
        if ((err = snd_ctl_card_info(cardHandle, cardInfo)) < 0)
        {
            printf("Can't get info for card %i: %s\n", cardIdx, snd_strerror(err));
            continue;
        }

        qDebug() << "[getDevicesInfo] Card" << cardIdx << "=" << snd_ctl_card_info_get_name(cardInfo);

        while (snd_ctl_pcm_next_device(cardHandle, &devIdx) == 0 && devIdx >= 0)
        {
            snd_pcm_info_t *pcmInfo;
            int tmpCaps = 0;

            snd_pcm_info_alloca(&pcmInfo);

            snprintf(str, sizeof (str), "plughw:%d,%d", cardIdx, devIdx);

            /* Obtain info about this particular device */
            snd_pcm_info_set_device(pcmInfo, devIdx);
            snd_pcm_info_set_subdevice(pcmInfo, 0);
            snd_pcm_info_set_stream(pcmInfo, SND_PCM_STREAM_CAPTURE);
            if (snd_ctl_pcm_info(cardHandle, pcmInfo) >= 0)
                tmpCaps |= AUDIO_CAP_INPUT;

            snd_pcm_info_set_stream(pcmInfo, SND_PCM_STREAM_PLAYBACK);
            if (snd_ctl_pcm_info(cardHandle, pcmInfo) >= 0)
                tmpCaps |= AUDIO_CAP_OUTPUT;

            if (tmpCaps != 0)
            {
                AudioDeviceInfo info;
                info.deviceName = QString(snd_ctl_card_info_get_name(cardInfo)) + " - " +
                                  QString(snd_pcm_info_get_name(pcmInfo));
                info.privateName = QString(str);
                info.capabilities = tmpCaps;
                devList.append(info);
            }
        }

        // Close the card's control interface after we're done with it
        snd_ctl_close(cardHandle);
    }

    // ALSA allocates some mem to load its config file when we call some of the
    // above functions. Now that we're done getting the info, let's tell ALSA
    // to unload the info and free up that mem
    snd_config_update_free_global();

    return devList;
}

qint64 AudioRendererAlsa::writeAudio(unsigned char *data, qint64 maxSize)
{
    if (pcm_handle == NULL || m_prebuf == NULL)
        return 0;

    if ((maxSize = qMin(maxSize, m_prebuf_size - m_prebuf_fill)) > 0)
    {
        memmove(m_prebuf + m_prebuf_fill, data, maxSize);
        m_prebuf_fill += maxSize;
    }

    snd_pcm_uframes_t l = snd_pcm_bytes_to_frames(pcm_handle, m_prebuf_fill);

    while (l >= m_chunk_size)
    {
        snd_pcm_wait(pcm_handle, 10);
        long m;
        if ((m = alsa_write(m_prebuf, m_chunk_size)) >= 0)
        {
            l -= m;
            m = snd_pcm_frames_to_bytes(pcm_handle, m); // convert frames to bytes
            m_prebuf_fill -= m;
            memmove(m_prebuf, m_prebuf + m, m_prebuf_fill); //move data to begin
        }
        else
            return -1;
    }
    return maxSize;
}

long AudioRendererAlsa::alsa_write(unsigned char *data, long size)
{
    long m = snd_pcm_avail_update(pcm_handle);
    if (m >= 0 && m < size)
    {
        snd_pcm_wait(pcm_handle, 500);
        return 0;
    }

    if (m_use_mmap)
        m = snd_pcm_mmap_writei (pcm_handle, data, size);
    else
        m = snd_pcm_writei (pcm_handle, data, size);

    if (m == -EAGAIN)
    {
        snd_pcm_wait(pcm_handle, 500);
        return 0;
    }
    else if (m >= 0)
    {
        if (m < size)
        {
            snd_pcm_wait(pcm_handle, 500);
        }
        return m;
    }
    else if (m == -EPIPE)
    {
        qDebug ("OutputALSA: buffer underrun!");
        if ((m = snd_pcm_prepare(pcm_handle)) < 0)
        {
            qDebug ("OutputALSA: Can't recover after underrun: %s",
                    snd_strerror(m));
            /* TODO: reopen the device */
            return -1;
        }
        return 0;
    }
#ifdef ESTRPIPE
    else if (m == -ESTRPIPE)
    {
        qDebug ("OutputALSA: Suspend, trying to resume");
        while ((m = snd_pcm_resume(pcm_handle))
                == -EAGAIN)
            sleep (1);
        if (m < 0)
        {
            qDebug ("OutputALSA: Failed, restarting");
            if ((m = snd_pcm_prepare(pcm_handle)) < 0)
            {
                qDebug ("OutputALSA: Failed to restart device: %s.",
                        snd_strerror(m));
                return -1;
            }
        }
        return 0;
    }
#endif
    qDebug ("OutputALSA: error: %s", snd_strerror(m));
    return snd_pcm_prepare (pcm_handle);
}

void AudioRendererAlsa::drain()
{
    snd_pcm_uframes_t l = snd_pcm_bytes_to_frames(pcm_handle, m_prebuf_fill);

    while (l > 0)
    {
        long m = 0;
        if ((m = alsa_write(m_prebuf, l)) >= 0)
        {
            l -= m;
            m = snd_pcm_frames_to_bytes(pcm_handle, m); // convert frames to bytes
            m_prebuf_fill -= m;
            memmove(m_prebuf, m_prebuf + m, m_prebuf_fill);
        }
        else
            break;
    }
    snd_pcm_nonblock(pcm_handle, 0);
    snd_pcm_drain(pcm_handle);
    snd_pcm_nonblock(pcm_handle, 1);
}

void AudioRendererAlsa::reset()
{
    m_prebuf_fill = 0;
    snd_pcm_drop(pcm_handle);
    snd_pcm_prepare(pcm_handle);
}

void AudioRendererAlsa::suspend()
{
    if (m_can_pause)
        snd_pcm_pause(pcm_handle, 1);
}

void AudioRendererAlsa::resume()
{
    if (m_can_pause)
        snd_pcm_pause(pcm_handle, 0);
}

void AudioRendererAlsa::uninitialize()
{
    qDebug() << Q_FUNC_INFO;

    if (!m_inited)
        return;

    m_inited = false;
    if (pcm_handle)
    {
        snd_pcm_drop(pcm_handle);
        qDebug("OutputALSA: closing pcm_handle");
        snd_pcm_close(pcm_handle);
        pcm_handle = 0;
    }
    if (m_prebuf)
        free(m_prebuf);
    m_prebuf = 0;
}
