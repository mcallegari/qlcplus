/*
  Q Light Controller Plus
  audiorenderer_qt6.h

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

#ifndef AUDIORENDERER_QT6_H
#define AUDIORENDERER_QT6_H

#include "audiorenderer.h"

#include <QAudioFormat>
#include <QAudioDevice>
#include <QAudioSink>
#include <QIODevice>

class Doc;

/** @addtogroup engine_audio Audio
 * @{
 */

class AudioRendererQt6 : public AudioRenderer
{
    Q_OBJECT
public:
    AudioRendererQt6(QString device, Doc *doc, QObject * parent = 0);
    ~AudioRendererQt6();

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

    /*********************************************************************
     * Thread functions
     *********************************************************************/
public:
    /** @reimpl */
    void run();

private:
    QAudioSink *m_audioSink;
    QIODevice *m_output;
    QAudioFormat m_format;
    QString m_device;
    QAudioDevice m_deviceInfo;
    QByteArray m_outputBuffer;
};

/** @} */

#endif // AUDIORENDERER_QT6_H
