/*
  Q Light Controller Plus
  audiocapture_portaudio.h

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

#ifndef AUDIOCAPTURE_PORTAUDIO_H
#define AUDIOCAPTURE_PORTAUDIO_H

#include "audiocapture.h"

/** @addtogroup engine_audio Audio
 * @{
 */

class AudioCapturePortAudio : public AudioCapture
{
    Q_OBJECT
public:
    AudioCapturePortAudio(QObject * parent = 0);
    ~AudioCapturePortAudio();

    /** @reimpl */
    bool initialize(unsigned int sampleRate, quint8 channels, quint16 bufferSize);

    /** @reimpl */
    qint64 latency();

protected:
    /** @reimpl */
    void suspend();

    /** @reimpl */
    void resume();

    /** @reimpl */
    bool readAudio(int maxSize);
};

/** @} */

#endif // AUDIOCAPTURE_PORTAUDIO_H
