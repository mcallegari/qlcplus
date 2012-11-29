/*
  Q Light Controller Plus
  audiodecoder_sndfile.h

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

#ifndef AUDIODECODER_AUDIOFILE_H
#define AUDIODECODER_AUDIOFILE_H

extern "C"{
#include <sndfile.h>
}

#include <QStringList>

#include "audiodecoder.h"

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


#endif // DECODER_SNDFILE_H
