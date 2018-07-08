/*
  Q Light Controller
  genericfader.h

  Copyright (c) Heikki Junnila

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

#ifndef GENERICFADER
#define GENERICFADER

#include <QList>
#include <QHash>

#include "universe.h"

class FadeChannel;
class Doc;

/** @addtogroup engine Engine
 * @{
 */

class GenericFader
{
public:
    GenericFader(Doc* doc);
    ~GenericFader();

    /**
     * Add a channel that shall be faded from ch.start() to ch.target() within
     * the time specified by ch.fadeTime(). If ch.target() == 0, the channel will
     * be removed automatically from the fader when done.
     *
     * If the fader already contains the same channel, the one whose current
     * value is higher remains in the fader. With LTP channels this might result
     * in the value jumping ina weird way but LTP channels are rarely faded anyway.
     * With HTP channels the lower value has no meaning in the first place.
     *
     * @param ch The channel to fade
     */
    void add(const FadeChannel& ch);

    /** Replace an existing FaderChannel */
    void forceAdd(const FadeChannel& ch);

    /** Remove a channel whose fixture & channel match with $fc's */
    void remove(const FadeChannel& fc);

    /**
     * Remove all channels.
     */
    void removeAll();

    /** Get all channels in a non-modifiable hashmap */
    const QHash <FadeChannel,FadeChannel>& channels() const;

    /**
     * Run the channels forward by one step and write their current values to
     * the given UniverseArray.
     *
     * @param universes The universe array that receives channel data.
     * @param paused Request a pause state, so the fader doesn't have to advance its transition
     */
    void write(QList<Universe *> universes, bool paused = false);

    /**
     * Adjust the intensities of all channels by $fraction
     *
     * @param fraction 0.0 - 1.0
     */
    void adjustIntensity(qreal fraction);

    /**
     * Get the overall intensity adjustment
     *
     * @return 0.0 - 1.0
     */
    qreal intensity() const;

    /**
     * Set the blend mode to be applied in the write method
     *
     * @param mode the blend mode as listed in Universe::BlendMode
     */
    void setBlendMode(Universe::BlendMode mode);

private:
    QHash <FadeChannel,FadeChannel> m_channels;
    qreal m_intensity;
    Universe::BlendMode m_blendMode;
    Doc* m_doc;
};

/** @} */

#endif
