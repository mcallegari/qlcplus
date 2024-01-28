/*
  Q Light Controller Plus
  audiorenderer_portaudio.h

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

#include <portaudio.h>

/** @addtogroup engine_audio Audio
 * @{
 */

class AudioRendererPortAudio : public AudioRenderer
{
    Q_OBJECT
public:
    AudioRendererPortAudio(QString device, QObject * parent = 0);
    ~AudioRendererPortAudio();

    /** @reimpl */
    bool initialize(quint32, int, AudioFormat format);

    /** @reimpl */
    qint64 latency();

    static QList<AudioDeviceInfo> getDevicesInfo();

protected:
    /** @reimpl */
    qint64 writeAudio(unsigned char *data, qint64 maxSize);

    int getPendingDataSize();

    /** @reimpl */
    void drain();

    /** @reimpl */
    void reset();

    /** @reimpl */
    void suspend();

    /** @reimpl */
    void resume();

private:
    static int dataCallback (const void *inputBuffer, void *outputBuffer,
                             unsigned long framesPerBuffer,
                             const PaStreamCallbackTimeInfo* timeInfo,
                             PaStreamCallbackFlags statusFlags,
                             void *userData );

    PaStream *m_paStream;
    QMutex m_paMutex;
    QByteArray m_buffer;
    QString m_device;

    int m_channels;
    int m_frameSize;
};

/** @} */

#endif
