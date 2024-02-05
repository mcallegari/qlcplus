/*
  Q Light Controller Plus
  audiorenderer_qt5.h

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

#ifndef AUDIORENDERER_QT5_H
#define AUDIORENDERER_QT5_H

#include "audiorenderer.h"

#include <QAudioOutput>
#include <QIODevice>

class Doc;

/** @addtogroup engine_audio Audio
 * @{
 */

class AudioRendererQt5 : public AudioRenderer
{
    Q_OBJECT
public:
    AudioRendererQt5(QString device, Doc *doc, QObject *parent = 0);
    ~AudioRendererQt5();

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
    QAudioOutput *m_audioOutput;
    QIODevice *m_output;
    QAudioFormat m_format;
    QString m_device;
    QAudioDeviceInfo m_deviceInfo;
};

/** @} */

#endif // AUDIORENDERER_QT5_H
