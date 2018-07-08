/*
  Q Light Controller Plus
  audiocapture_wavein.h

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

#ifndef AUDIOCAPTURE_WAVEIN_H
#define AUDIOCAPTURE_WAVEIN_H

#include "audiocapture.h"

#include <QMutex>

/** @addtogroup engine_audio Audio
 * @{
 */

#define HEADERS_NUMBER   2

class AudioCaptureWaveIn : public AudioCapture
{
    Q_OBJECT
public:
    AudioCaptureWaveIn(QObject * parent = 0);
    ~AudioCaptureWaveIn();

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
    int m_currentBufferIndex;
    char *m_internalBuffers[HEADERS_NUMBER];
};

/** @} */

#endif // AUDIOCAPTURE_WAVEIN_H
