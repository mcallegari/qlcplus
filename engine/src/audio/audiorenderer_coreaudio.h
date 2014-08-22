/*
  Q Light Controller Plus
  audiorenderer_coreaudio.h

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

#ifndef AUDIORENDERER_COREAUDIO_H
#define AUDIORENDERER_COREAUDIO_H

#include "audiorenderer.h"
#include "audiodecoder.h"

#include <CoreAudio/CoreAudio.h>
#include <AudioToolbox/AudioToolbox.h>
#include <AudioToolbox/AudioQueue.h>

/** @addtogroup engine_audio Audio
 * @{
 */

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

/** @} */

#endif
