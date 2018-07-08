/*
  Q Light Controller Plus
  audiorenderer_alsa.h

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

#ifndef AUDIORENDERER_ALSA_H
#define AUDIORENDERER_ALSA_H

#include "audiorenderer.h"
#include "audiodecoder.h"

extern "C"
{
#include <alsa/asoundlib.h>
}

/** @addtogroup engine_audio Audio
 * @{
 */

class AudioRendererAlsa : public AudioRenderer
{
    Q_OBJECT
public:
    AudioRendererAlsa(QString device, QObject * parent = 0);
    ~AudioRendererAlsa();

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
    // helper functions
    long alsa_write(unsigned char *data, long size);

    void uninitialize();

    bool m_inited;
    bool m_use_mmap;

    // ALSA specific
    snd_pcm_t *pcm_handle;
    char *pcm_name;
    snd_pcm_uframes_t m_chunk_size;
    size_t m_bits_per_frame;
    //prebuffer
    uchar *m_prebuf;
    qint64 m_prebuf_size;
    qint64 m_prebuf_fill;
    bool m_can_pause;
};

/** @} */

#endif // AUDIORENDERER_ALSA_H
