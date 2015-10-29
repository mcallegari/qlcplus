/*
  Q Light Controller Plus
  audiodecoder_sndfile.cpp

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

#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

#include "audiodecoder_sndfile.h"

// Decoder class

AudioDecoderSndFile::AudioDecoderSndFile(const QString &path)
        : AudioDecoder()
{
    m_path = path;
    m_bitrate = 0;
    m_totalTime = 0;
    m_sndfile = 0;
    m_freq = 0;
}

AudioDecoderSndFile::~AudioDecoderSndFile()
{
    deinit();
}

bool AudioDecoderSndFile::initialize()
{
    m_bitrate = 0;
    m_totalTime = 0.0;
    SF_INFO snd_info;

    memset (&snd_info, 0, sizeof(snd_info));
    snd_info.format=0;
    m_sndfile = sf_open(m_path.toLocal8Bit(), SFM_READ, &snd_info);
    if (!m_sndfile)
    {
        qWarning("DecoderSndFile: failed to open: %s", qPrintable(m_path));
        return false;
    }

    m_freq = snd_info.samplerate;
    int chan = snd_info.channels;
    m_totalTime = snd_info.frames * 1000 / m_freq;
    m_bitrate =  QFileInfo(m_path).size () * 8.0 / m_totalTime + 0.5;

    if((snd_info.format & SF_FORMAT_SUBMASK) == SF_FORMAT_FLOAT)
        sf_command (m_sndfile, SFC_SET_SCALE_FLOAT_INT_READ, NULL, SF_TRUE);

    AudioFormat pcmFormat = PCM_S16LE;
    switch(snd_info.format & SF_FORMAT_SUBMASK)
    {
        case SF_FORMAT_PCM_S8: pcmFormat = PCM_S8; break;
        case SF_FORMAT_PCM_16: pcmFormat = PCM_S16LE; break;
        case SF_FORMAT_PCM_24: pcmFormat = PCM_S24LE; break;
        case SF_FORMAT_PCM_32: pcmFormat = PCM_S32LE; break;
        default: pcmFormat = PCM_S16LE; break;
    }

    configure(m_freq, chan, pcmFormat);
    qDebug() << "DecoderSndFile: detected format: Sample Rate:" << m_freq <<
            ",Channels: " <<  chan << ", PCM Format: " << snd_info.format;

    return true;
}

void AudioDecoderSndFile::deinit()
{
    m_totalTime = 0;
    m_bitrate = 0;
    m_freq = 0;
    if (m_sndfile)
        sf_close(m_sndfile);
    m_sndfile = 0;
}

qint64 AudioDecoderSndFile::totalTime()
{
    return m_totalTime;
}

int AudioDecoderSndFile::bitrate()
{
    return m_bitrate;
}

qint64 AudioDecoderSndFile::read(char *audio, qint64 maxSize)
{
    return sizeof(short)* sf_read_short  (m_sndfile, (short *)audio, maxSize / sizeof(short));
}

void AudioDecoderSndFile::seek(qint64 pos)
{
    sf_seek(m_sndfile, m_freq * pos/1000, SEEK_SET);
}

QStringList AudioDecoderSndFile::getSupportedFormats()
{
    QStringList caps;
    SF_FORMAT_INFO format_info;
    int k, count ;

    sf_command (0, SFC_GET_SIMPLE_FORMAT_COUNT, &count, sizeof (int)) ;

    for (k = 0 ; k < count ; k++)
    {
        format_info.format = k;
        sf_command (0, SFC_GET_SIMPLE_FORMAT, &format_info, sizeof (format_info));
        qDebug("%08x  %s %s", format_info.format, format_info.name, format_info.extension);
        QString ext = QString(format_info.extension);
        if (ext == "aiff" && !caps.contains("*.aiff"))
            caps << "*.aiff";
        else if (ext == "aifc" && !caps.contains("*.aifc"))
            caps << "*.aifc";
        else if (ext == "flac" && !caps.contains("*.flac"))
            caps << "*.flac";
        else if (ext == "oga" && !caps.contains("*.oga"))
            caps << "*.oga" << "*.ogg";
        else if (ext == "wav" && !caps.contains("*.wav"))
            caps << "*.wav";
    }

    return caps;
}
