/*
  Q Light Controller Plus
  audiodecoder_sndfile.h

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

#ifndef AUDIODECODER_AUDIOFILE_H
#define AUDIODECODER_AUDIOFILE_H

extern "C"{
#include <sndfile.h>
}

#include <QStringList>

#include "audiodecoder.h"

/** @addtogroup engine_audio Audio
 * @{
 */

class AudioDecoderSndFile : public AudioDecoder
{
public:
    AudioDecoderSndFile(const QString &path);
    virtual ~AudioDecoderSndFile();

    // Standard Decoder API
    /** @reimpl */
    bool initialize();

    /** @reimpl */
    qint64 totalTime();

    /** @reimpl */
    int bitrate();

    /** @reimpl */
    qint64 read(char *audio, qint64 maxSize);

    /** @reimpl */
    void seek(qint64 time);

    static QStringList getSupportedFormats();

private:

    // helper functions
    void deinit();

    SNDFILE *m_sndfile;
    int m_bitrate;
    quint32 m_freq;
    qint64 m_totalTime;
    QString m_path;
};

/** @} */

#endif // DECODER_SNDFILE_H
