/*
  Q Light Controller Plus
  audiodecoder_mad.h

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

#ifndef AUDIODECODER_MAD_H
#define AUDIODECODER_MAD_H

#include <QFile>
#include <QStringList>

#include "audiodecoder.h"

extern "C"
{
    #include <mad.h>
}

/** @addtogroup engine_audio Audio
 * @{
 */

class AudioDecoderMAD : public AudioDecoder
{
    Q_OBJECT
    Q_INTERFACES(AudioDecoder)
    Q_PLUGIN_METADATA(IID QLCPlusAudioPlugin_iid)

public:
    virtual ~AudioDecoderMAD();

    /** @reimpl */
    AudioDecoder *createCopy();

    /** @reimp */
    int priority() const;

    /** @reimp */
    bool initialize(const QString &path);

    /** @reimp */
    qint64 totalTime();

    /** @reimp */
    int bitrate();

    /** @reimp */
    qint64 read(char *data, qint64 size);

    /** @reimp */
    void seek(qint64);

    /** @reimp */
    QStringList supportedFormats();

private:
    // helper functions
    qint64 madOutput(char *data, qint64 size);
    bool fillBuffer();
    void deinit();
    bool findHeader();
    bool findXingHeader(struct mad_bitptr, unsigned int);
    uint findID3v2(uchar *data, ulong size);

    QFile m_input;
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

/** @} */

#endif
