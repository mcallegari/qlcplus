/*
  Q Light Controller
  genericfader.h

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef GENERICFADER
#define GENERICFADER

#include <QList>
#include <QHash>

class UniverseArray;
class FadeChannel;
class Doc;

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
     */
    void write(UniverseArray* universes);

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

private:
    QHash <FadeChannel,FadeChannel> m_channels;
    qreal m_intensity;
    Doc* m_doc;
};

#endif
