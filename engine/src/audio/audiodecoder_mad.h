/*
  Q Light Controller Plus
  audiodecoder_mad.h

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

/***************************************************************************
 *  Based on qmmp, mq3 and madplay projects                                *
 *                                                                         *
 * Copyright (c) 2000-2001 Brad Hughes <bhughes@trolltech.com>             *
 * Copyright (C) 2000-2004 Robert Leslie <rob@mars.org>                    *
 * Copyright (C) 2009-2012 Ilya Kotov forkotov02@hotmail.ru                *
 *                                                                         *
 ***************************************************************************/

#ifndef AUDIODECODER_MAD_H
#define AUDIODECODER_MAD_H

extern "C"
{
#include <mad.h>
}

#include <QFile>
#include <QStringList>

#include "audiodecoder.h"

class AudioDecoderMAD : public AudioDecoder
{
public:
    AudioDecoderMAD(const QString &path);
    virtual ~AudioDecoderMAD();

    // standard decoder API
    /** @reimpl */
    bool initialize();

    /** @reimpl */
    qint64 totalTime();

    /** @reimpl */
    int bitrate();

    /** @reimpl */
    qint64 read(char *data, qint64 size);

    /** @reimpl */
    void seek(qint64);

    static QStringList getSupportedFormats();

private:
    // helper functions
    qint64 madOutput(char *data, qint64 size);
    bool fillBuffer();
    void deinit();
    bool findHeader();
    bool findXingHeader(struct mad_bitptr, unsigned int);
    uint findID3v2(uchar *data, ulong size);

    QFile *m_input;
    bool m_inited, m_eof;
    qint64 m_totalTime;
    int m_channels, m_skip_frames;
    uint m_bitrate;
    long m_freq, m_len;
    qint64 m_output_bytes, m_output_at;

    // file input buffer
    char *m_input_buf;
    qint64 m_input_bytes;

    // MAD decoder
    struct
    {
        int flags;
        unsigned long frames;
        unsigned long bytes;
        unsigned char toc[100];
        long scale;
    } xing;

    enum
    {
        XING_FRAMES = 0x0001,
        XING_BYTES  = 0x0002,
        XING_TOC    = 0x0004,
        XING_SCALE  = 0x0008
    };

    struct audio_dither
    {
        mad_fixed_t error[3];
        mad_fixed_t random;
    };

    struct mad_stream m_stream;
    struct mad_frame m_frame;
    struct mad_synth m_synth;
    struct audio_dither m_left_dither, m_right_dither;

    //converter functions
    unsigned long prng(unsigned long state);
    void clip(mad_fixed_t *sample);
    long audio_linear_dither(unsigned int bits, mad_fixed_t sample, struct audio_dither *dither);
    long audio_linear_round(unsigned int bits, mad_fixed_t sample);
};


#endif
