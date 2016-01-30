/*
  Q Light Controller Plus
  audiocapture_alsa.h

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

#ifndef AUDIOCAPTURE_ALSA_H
#define AUDIOCAPTURE_ALSA_H

#include "audiocapture.h"

extern "C"
{
#include <alsa/asoundlib.h>
}

/** @addtogroup engine_audio Audio
 * @{
 */

class AudioCaptureAlsa : public AudioCapture
{
    Q_OBJECT
public:
    AudioCaptureAlsa(QObject * parent = 0);
    ~AudioCaptureAlsa();

    /** @reimpl */
    qint64 latency();

protected:
    /** @reimpl */
    bool initialize();

    /** @reimpl */
    virtual void uninitialize();

    /** @reimpl */
    void suspend();

    /** @reimpl */
    void resume();

    /** @reimpl */
    bool readAudio(int maxSize);

private:
    snd_pcm_t *m_captureHandle;
    char *pcm_name;
};

/** @} */

#endif // AUDIOCAPTURE_ALSA_H
