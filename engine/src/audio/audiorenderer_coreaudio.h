/*
  Q Light Controller Plus
  audiorenderer_coreaudio.h

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

#include <CoreAudio/CoreAudio.h>
#include <AudioToolbox/AudioToolbox.h>
#include <AudioToolbox/AudioQueue.h>

#define AUDIO_BUFFER_SIZE   8192
#define AUDIO_BUFFERS_NUM   4

class AudioRendererCoreAudio : public AudioRenderer
{
    Q_OBJECT
public:
    AudioRendererCoreAudio(QObject * parent = 0);
    ~AudioRendererCoreAudio();

    /** @reimpl */
    bool initialize(quint32, int, AudioFormat format);

    /** @reimpl */
    qint64 latency();

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
    static void inCallback (void *inUserData, AudioQueueRef queue, AudioQueueBufferRef buf_ref);

    AudioQueueRef m_queue;
    AudioQueueBufferRef m_buffer[AUDIO_BUFFERS_NUM];

    int m_buffersFilled;
    int m_bufferIndex;
};

#endif
