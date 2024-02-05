/*
  Q Light Controller Plus
  audiorenderer_waveout.h

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

#ifndef AUDIORENDERER_WAVEOUT_H
#define AUDIORENDERER_WAVEOUT_H

#include "audiorenderer.h"
#include "audiodecoder.h"

#include <QObject>
#include <stdio.h>
#include <windows.h>

/** @addtogroup engine_audio Audio
 * @{
 */

class AudioRendererWaveOut : public AudioRenderer
{
    Q_OBJECT
public:
    AudioRendererWaveOut(QString device, QObject * parent = 0);
    ~AudioRendererWaveOut();

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
    void suspend();

    /** @reimpl */
    void resume();

    /** @reimpl */
    void reset();

private:
    // helper functions
    void status();
    void uninitialize();

private:
    UINT deviceID;
};

/** @} */

#endif // AUDIORENDERER_WAVEOUT_H
