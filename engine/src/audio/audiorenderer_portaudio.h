/*
  Q Light Controller Plus
  audiorenderer_portaudio.h

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

#ifndef AUDIORENDERER_COREAUDIO_H
#define AUDIORENDERER_COREAUDIO_H

#include "audiorenderer.h"
#include "audiodecoder.h"

#include <portaudio.h>

class AudioRendererPortAudio : public AudioRenderer
{
    Q_OBJECT
public:
    AudioRendererPortAudio(QObject * parent = 0);
    ~AudioRendererPortAudio();

    /** @reimpl */
    bool initialize(quint32, int, AudioFormat format);

    /** @reimpl */
    qint64 latency();

    static QList<AudioDeviceInfo> getDevicesInfo();

protected:
    /** @reimpl */
    qint64 writeAudio(unsigned char *data, qint64 maxSize);

    /** @reimpl */
    void drain();

    /** @reimpl */
    void reset();

    /** @reimpl */
    void suspend();

    /** @reimpl */
    void resume();

private:
    static int dataCallback ( const void *inputBuffer, void *outputBuffer,
                               unsigned long framesPerBuffer,
                               const PaStreamCallbackTimeInfo* timeInfo,
                               PaStreamCallbackFlags statusFlags,
                               void *userData );

    PaStream *stream;
    QMutex m_mutex;
    QByteArray m_buffer;

    int m_channels;
    int m_frameSize;

    int m_buffersFilled;
    int m_writeBufferIndex;
    int m_readBufferIndex;
};

#endif
