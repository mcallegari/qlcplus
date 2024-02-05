/*
  Q Light Controller Plus
  audiorenderer_null.h

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

#ifndef AUDIORENDERER_NULL_H
#define AUDIORENDERER_NULL_H

#include "audiorenderer.h"
#include "audiodecoder.h"

/** @addtogroup engine_audio Audio
 * @{
 */

class AudioRendererNull : public AudioRenderer
{
    Q_OBJECT
public:
    AudioRendererNull(QObject * parent = 0);

    ~AudioRendererNull() { }

    /** @reimpl */
    bool initialize(quint32, int, AudioFormat) { return true; }

    /** @reimpl */
    qint64 latency() { return 0; } // not bad for a null device huh ? ;)

protected:
    /** @reimpl */
    qint64 writeAudio(unsigned char *, qint64 maxSize) { return maxSize; }

    /** @reimpl */
    void drain() { }

    /** @reimpl */
    void reset() { }

    /** @reimpl */
    void suspend() { }

    /** @reimpl */
    void resume() { }

};

/** @} */

#endif
