/*
  Q Light Controller Plus
  audiodecoder_mad.cpp

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

/***************************************************************************
 *  Based on qmmp, mq3 and madplay projects                                *
 *                                                                         *
 * Copyright (c) 2000-2001 Brad Hughes <bhughes@trolltech.com>             *
 * Copyright (C) 2000-2004 Robert Leslie <rob@mars.org>                    *
 * Copyright (C) 2009-2012 Ilya Kotov forkotov02@hotmail.ru                *
 *                                                                         *
 ***************************************************************************/

#include <QDebug>

#include <math.h>
#include <stdio.h>
#include "audiodecoder_mad.h"

#define XING_MAGIC (('X' << 24) | ('i' << 16) | ('n' << 8) | 'g')
#define INPUT_BUFFER_SIZE (32*1024)
#define USE_DITHERING

AudioDecoderMAD::~AudioDecoderMAD()
{
    deinit();
    if (m_input_buf != NULL)
    {
        qDebug("AudioDecoderMAD: deleting input_buf");
        delete [] m_input_buf;
        m_input_buf = NULL;
    }
}

AudioDecoder *AudioDecoderMAD::createCopy()
{
    AudioDecoderMAD* copy = new AudioDecoderMAD();
    return qobject_cast<AudioDecoder *>(copy);
}

int AudioDecoderMAD::priority() const
{
    return 20;
}

bool AudioDecoderMAD::initialize(const QString &path)
{
    m_inited = false;
    m_totalTime = 0;
    m_channels = 0;
    m_bitrate = 0;
    m_freq = 0;
    m_len = 0;
    m_input_buf = NULL;
    m_input_bytes = 0;
    m_output_bytes = 0;
    m_output_at = 0;
    m_skip_frames = 0;
    m_eof = false;

    m_left_dither.random = 0;
    m_left_dither.error[0] = 0;
    m_left_dither.error[1] = 0;
    m_left_dither.error[2] = 0;

    m_right_dither.random = 0;
    m_right_dither.error[0] = 0;
    m_right_dither.error[1] = 0;
    m_right_dither.error[2] = 0;

    if (path.isEmpty())
        return false;

    m_input.setFileName(path);

    if (m_input.exists() == false)
    {
        qWarning("DecoderMAD: cannot initialize. Source file doesn't exist.");
        return false;
    }

    if (!m_input_buf)
        m_input_buf = new char[INPUT_BUFFER_SIZE];

    if (!m_input.isOpen())
    {
        if (!m_input.open(QIODevice::ReadOnly))
        {
            qWarning("DecoderMAD: %s", qPrintable(m_input.errorString ()));
            return false;
        }
    }

    mad_stream_init(&m_stream);
    mad_frame_init(&m_frame);
    mad_synth_init(&m_synth);

    if (!findHeader())
    {
        qDebug("DecoderMAD: Can't find a valid MPEG header.");
        return false;
    }
    mad_stream_buffer(&m_stream, (unsigned char *) m_input_buf, m_input_bytes);
    m_stream.error = MAD_ERROR_BUFLEN;
    mad_frame_mute (&m_frame);
    m_stream.next_frame = 0;
    m_stream.sync = 0;
    configure(m_freq, m_channels, PCM_S16LE);
    m_inited = true;
    return true;
}


void AudioDecoderMAD::deinit()
{
    if (!m_inited)
        return;

    mad_synth_finish(&m_synth);
    mad_frame_finish(&m_frame);
    mad_stream_finish(&m_stream);

    m_inited = false;
    m_totalTime = 0;
    m_channels = 0;
    m_bitrate = 0;
    m_freq = 0;
    m_len = 0;
    m_input_bytes = 0;
    m_output_bytes = 0;
    m_output_at = 0;
    m_skip_frames = 0;
    m_eof = false;

    if (m_input.isOpen())
        m_input.close();
}

bool AudioDecoderMAD::findXingHeader(struct mad_bitptr ptr, unsigned int bitlen)
{
    if (bitlen < 64 || mad_bit_read(&ptr, 32) != XING_MAGIC)
        goto fail;

    xing.flags = mad_bit_read(&ptr, 32);
    bitlen -= 64;

    if (xing.flags & XING_FRAMES)
    {
        if (bitlen < 32)
            goto fail;

        xing.frames = mad_bit_read(&ptr, 32);
        bitlen -= 32;
    }

    if (xing.flags & XING_BYTES)
    {
        if (bitlen < 32)
            goto fail;

        xing.bytes = mad_bit_read(&ptr, 32);
        bitlen -= 32;
    }

    if (xing.flags & XING_TOC)
    {
        int i;

        if (bitlen < 800)
            goto fail;

        for (i = 0; i < 100; ++i)
            xing.toc[i] = mad_bit_read(&ptr, 8);

        bitlen -= 800;
    }

    if (xing.flags & XING_SCALE)
    {
        if (bitlen < 32)
            goto fail;

        xing.scale = mad_bit_read(&ptr, 32);
        bitlen -= 32;
    }

    return true;

fail:
    xing.flags = 0;
    xing.frames = 0;
    xing.bytes = 0;
    xing.scale = 0;
    return false;
}

bool AudioDecoderMAD::findHeader()
{
    bool result = false;
    int count = 0;
    bool has_xing = false;
    bool is_vbr = false;
    mad_timer_t duration = mad_timer_zero;
    struct mad_header header;
    mad_header_init(&header);

    forever
    {
        if (m_stream.error == MAD_ERROR_BUFLEN || !m_stream.buffer)
        {
            size_t remaining = 0;

            if (m_stream.next_frame)
            {
                remaining = m_stream.bufend - m_stream.next_frame;
                memmove (m_input_buf, m_stream.next_frame, remaining);
            }

            m_input_bytes = m_input.read(m_input_buf + remaining, INPUT_BUFFER_SIZE - remaining);

            if (m_input_bytes <= 0)
            {
                qDebug() << "End of file reached";

                if (is_vbr)
                    break;

                return false;
            }

            mad_stream_buffer(&m_stream, (unsigned char *) m_input_buf + remaining, m_input_bytes);
            m_stream.error = MAD_ERROR_NONE;
        }

        if (mad_header_decode(&header, &m_stream) < 0)
        {
            if (m_stream.error == MAD_ERROR_LOSTSYNC)
            {
                uint tagSize = findID3v2((uchar *)m_stream.this_frame,
                                         (ulong) (m_stream.bufend - m_stream.this_frame));
                if (tagSize > 0)
                {
                    mad_stream_skip(&m_stream, tagSize);
                    qDebug() << "Skipping ID3 tag bytes" << tagSize;
                }
                continue;
            }
            else if (m_stream.error == MAD_ERROR_BUFLEN || MAD_RECOVERABLE(m_stream.error))
            {
                continue;
            }
            else
            {
                qDebug ("DecoderMAD: Can't decode header: %s", mad_stream_errorstr(&m_stream));
                break;
            }
        }

        result = true;
        count++;

        //qDebug() << "Detecting header" << count << m_stream.error << m_stream.sync << header.bitrate << header.layer;

        // try to detect xing header
        if (count == 1)
        {
            m_frame.header = header;
            if (mad_frame_decode(&m_frame, &m_stream) != -1 &&
                findXingHeader(m_stream.anc_ptr, m_stream.anc_bitlen))
            {
                is_vbr = true;

                qDebug ("DecoderMAD: Xing header detected");

                if (xing.flags & XING_FRAMES)
                {
                    has_xing = true;
                    count = xing.frames;
                    break;
                }
            }
        }

        //try to detect VBR
        if (!is_vbr && !(count > 15))
        {
            if (m_bitrate && header.bitrate != m_bitrate)
            {
                qDebug ("DecoderMAD: VBR detected");
                is_vbr = true;
            }
            else
                m_bitrate = header.bitrate;
        }
        else if (!is_vbr)
        {
            qDebug ("DecoderMAD: Fixed rate detected");
            break;
        }
        mad_timer_add (&duration, header.duration);
    }

    if (!result)
    {
        qDebug() << "BAD RESULT";
        return false;
    }

    if (!is_vbr && !m_input.isSequential())
    {
        double time = (m_input.size() * 8.0) / (header.bitrate);
        double timefrac = (double)time - ((long)(time));
        mad_timer_set(&duration, (long)time, (long)(timefrac * 100), 100);
    }
    else if (has_xing)
    {
        mad_timer_multiply(&header.duration, count);
        duration = header.duration;
    }

    m_totalTime = mad_timer_count(duration, MAD_UNITS_MILLISECONDS);
    qDebug ("DecoderMAD: Total time: %ld", long(m_totalTime));
    m_freq = header.samplerate;
    m_channels = MAD_NCHANNELS(&header);
    m_bitrate = header.bitrate / 1000;
    mad_header_finish(&header);
    m_input.seek(0);
    m_input_bytes = 0;
    return true;
}

qint64 AudioDecoderMAD::totalTime()
{
    if (!m_inited)
        return 0;
    return m_totalTime;
}

int AudioDecoderMAD::bitrate()
{
    return int(m_bitrate);
}

qint64 AudioDecoderMAD::read(char *data, qint64 size)
{
    forever
    {
        if (((m_stream.error == MAD_ERROR_BUFLEN) || !m_stream.buffer) && !m_eof)
        {
            m_eof = !fillBuffer();
        }
        if (mad_frame_decode(&m_frame, &m_stream) < 0)
        {
            switch((int) m_stream.error)
            {
            case MAD_ERROR_LOSTSYNC:
            {
                //skip ID3v2 tag
                uint tagSize = findID3v2((uchar *)m_stream.this_frame,
                                         (ulong) (m_stream.bufend - m_stream.this_frame));
                if (tagSize > 0)
                {
                    mad_stream_skip(&m_stream, tagSize);
                    qDebug("DecoderMAD: %d bytes skipped", tagSize);
                }
                continue;
            }
            case MAD_ERROR_BUFLEN:
                if (m_eof)
                    return 0;
                continue;
            default:
                if (!MAD_RECOVERABLE(m_stream.error))
                    return 0;
                else
                    continue;
            }
        }
        if (m_skip_frames)
        {
            m_skip_frames--;
            continue;
        }
        mad_synth_frame(&m_synth, &m_frame);
        return madOutput(data, size);
    }
}
void AudioDecoderMAD::seek(qint64 pos)
{
    if (m_totalTime > 0)
    {
        qint64 seek_pos = qint64(pos * m_input.size() / m_totalTime);
        m_input.seek(seek_pos);
        mad_frame_mute(&m_frame);
        mad_synth_mute(&m_synth);
        m_stream.error = MAD_ERROR_BUFLEN;
        m_stream.sync = 0;
        m_input_bytes = 0;
        m_stream.next_frame = 0;
        m_skip_frames = 2;
        m_eof = 0;
    }
}

QStringList AudioDecoderMAD::supportedFormats()
{
    QStringList caps;
    caps << "*.mp3";
    return caps;
}

bool AudioDecoderMAD::fillBuffer()
{
    if (m_stream.next_frame)
    {
        m_input_bytes = &m_input_buf[m_input_bytes] - (char *) m_stream.next_frame;
        memmove(m_input_buf, m_stream.next_frame, m_input_bytes);
    }
    int len = m_input.read((char *) m_input_buf + m_input_bytes, INPUT_BUFFER_SIZE - m_input_bytes);
    if (!len)
    {
        qDebug("DecoderMAD: end of file");
        return false;
    }
    else if (len < 0)
    {
        qWarning("DecoderMAD: error");
        return false;
    }
    m_input_bytes += len;
    mad_stream_buffer(&m_stream, (unsigned char *) m_input_buf, m_input_bytes);
    return true;
}

uint AudioDecoderMAD::findID3v2(uchar *data, ulong size) //retuns ID3v2 tag size
{
    if (size < 10)
        return 0;

    if (((data[0] == 'I' && data[1] == 'D' && data[2] == '3') || //ID3v2 tag
         (data[0] == '3' && data[1] == 'D' && data[2] == 'I')) && //ID3v2 footer
            data[3] < 0xff && data[4] < 0xff && data[6] < 0x80 &&
            data[7] < 0x80 && data[8] < 0x80 && data[9] < 0x80)
    {
        quint32 id3v2_size = (data[6] << 21) + (data[7] << 14) + (data[8] << 7) + data[9];
        return id3v2_size;
    }
    return 0;
}

unsigned long AudioDecoderMAD::prng(unsigned long state) // 32-bit pseudo-random number generator
{
    return (state * 0x0019660dL + 0x3c6ef35fL) & 0xffffffffL;
}

// gather signal statistics while clipping
void AudioDecoderMAD::clip(mad_fixed_t *sample)
{
    enum
    {
        MIN = -MAD_F_ONE,
        MAX =  MAD_F_ONE - 1
    };

    if (*sample > MAX)
        *sample = MAX;
    else if (*sample < MIN)
        *sample = MIN;
}

long AudioDecoderMAD::audio_linear_dither(unsigned int bits, mad_fixed_t sample,
                                     struct audio_dither *dither)
{
    unsigned int scalebits;
    mad_fixed_t output, mask, random;

    /* noise shape */
    sample += dither->error[0] - dither->error[1] + dither->error[2];

    dither->error[2] = dither->error[1];
    dither->error[1] = dither->error[0] / 2;

    /* bias */
    output = sample + (1L << (MAD_F_FRACBITS + 1 - bits - 1));

    scalebits = MAD_F_FRACBITS + 1 - bits;
    mask = (1L << scalebits) - 1;

    /* dither */
    random  = prng(dither->random);
    output += (random & mask) - (dither->random & mask);

    dither->random = random;

    /* clip */
    clip(&output);

    /* quantize */
    output &= ~mask;

    /* error feedback */
    dither->error[0] = sample - output;

    /* scale */
    return output >> scalebits;
}

//generic linear sample quantize routine
long AudioDecoderMAD::audio_linear_round(unsigned int bits, mad_fixed_t sample)
{
    /* round */
    sample += (1L << (MAD_F_FRACBITS - bits));

    /* clip */
    clip(&sample);

    /* quantize and scale */
    return sample >> (MAD_F_FRACBITS + 1 - bits);
}

qint64 AudioDecoderMAD::madOutput(char *data, qint64 size)
{
    unsigned int samples, channels;
    mad_fixed_t const *left, *right;

    samples = m_synth.pcm.length;
    channels = m_synth.pcm.channels;
    left = m_synth.pcm.samples[0];
    right = m_synth.pcm.samples[1];
    m_bitrate = m_frame.header.bitrate / 1000;
    m_output_at = 0;
    m_output_bytes = 0;

    if (samples * channels * 2 > size)
    {
        qWarning() << "DecoderMad: input buffer is too small. Required: " << (samples * channels * 2) << ", available: " << size;
        samples = size / channels / 2;
    }

    while (samples--)
    {
        signed int sample;
#ifdef USE_DITHERING
        sample = audio_linear_dither(16, *left++,  &m_left_dither);
#else
        sample = audio_linear_round(16, *left++);
#endif
        *(data + m_output_at++) = ((sample >> 0) & 0xff);
        *(data + m_output_at++) = ((sample >> 8) & 0xff);
        m_output_bytes += 2;

        if (channels == 2)
        {
#ifdef USE_DITHERING
            sample = audio_linear_dither(16, *right++, &m_right_dither);
#else
            sample = audio_linear_round(16, *right++);
#endif
            *(data + m_output_at++) = ((sample >> 0) & 0xff);
            *(data + m_output_at++) = ((sample >> 8) & 0xff);
            m_output_bytes += 2;
        }
    }
    return m_output_bytes;
}

