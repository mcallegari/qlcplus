/*
  Q Light Controller Plus
  audiorenderer_alsa.cpp

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

/******************************************************
 *   Based on qmmp project                            *
 *                                                    *
 *   Copyright (C) 2007-2009 by Ilya Kotov            *
 *   forkotov02@hotmail.ru                            *
 ******************************************************/

#include <QString>

#include "audiorenderer_alsa.h"

AudioRendererAlsa::AudioRendererAlsa(QObject * parent)
    : AudioRenderer(parent)
{
    QString dev_name = "default";
    m_use_mmap = false;
    pcm_name = strdup(dev_name.toAscii().data());
    pcm_handle = NULL;
    m_prebuf = NULL;
    m_prebuf_size = 0;
    m_prebuf_fill = 0;
    m_can_pause = false;
}

AudioRendererAlsa::~AudioRendererAlsa()
{
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
        return false;
    }

    uint rate = freq; /* Sample rate */
    uint exact_rate = freq;   /* Sample rate returned by */

    uint buffer_time = 500000;
    uint period_time = 100000;
    bool use_pause =  false;

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
    m_can_pause = snd_pcm_hw_params_can_pause(hwparams) && use_pause;
    qDebug("OutputALSA: can pause: %d", m_can_pause);
    //configure(freq, chan, format); //apply configuration
    //create alsa prebuffer;
    m_prebuf_size = /*QMMP_BUFFER_SIZE + */m_bits_per_frame * m_chunk_size / 8;
    m_prebuf = (uchar *)malloc(m_prebuf_size);

    m_inited = true;
    return true;
}

qint64 AudioRendererAlsa::latency()
{
    //return m_prebuf_fill * 1000 / sampleRate() / channels() / sampleSize();
    return 0;
}

qint64 AudioRendererAlsa::writeAudio(unsigned char *data, qint64 maxSize)
{
    if((maxSize = qMin(maxSize, m_prebuf_size - m_prebuf_fill)) > 0)
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
    if(m >= 0 && m < size)
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
    long m = 0;
    snd_pcm_uframes_t l = snd_pcm_bytes_to_frames(pcm_handle, m_prebuf_fill);
    while (l > 0)
    {
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
    snd_pcm_prepare(pcm_handle);
}

void AudioRendererAlsa::resume()
{
    if (m_can_pause)
        snd_pcm_pause(pcm_handle, 0);
    snd_pcm_prepare(pcm_handle);
}


void AudioRendererAlsa::uninitialize()
{
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
