/*
  Q Light Controller Plus
  audiocapture_qt.h

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

#ifndef AUDIOCAPTURE_QT_H
#define AUDIOCAPTURE_QT_H

#include "audiocapture.h"

#include <QAudioInput>

/** @addtogroup engine_audio Audio
 * @{
 */

class AudioCaptureQt : public AudioCapture
{
    Q_OBJECT
public:
    AudioCaptureQt(QObject * parent = 0);
    ~AudioCaptureQt();

    /** @reimpl */
    bool initialize(unsigned int sampleRate, quint8 channels, quint16 bufferSize);

    /** @reimpl */
    qint64 latency();

    /** @reimpl */
    void setVolume(qreal volume);

protected:
    /** @reimpl */
    void suspend();

    /** @reimpl */
    void resume();

    /** @reimpl */
    bool readAudio(int maxSize);

private:
    QAudioInput *m_audioInput;
    QIODevice *m_input;
    QAudioFormat m_format;
    qreal m_volume;
};

/** @} */

#endif // AUDIOCAPTURE_QT_H
