/*
  Q Light Controller
  intensitygenerator.h

  Copyright (C) Heikki Junnila

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

#ifndef INTENSITYGENERATOR_H
#define INTENSITYGENERATOR_H

#include <QList>

class QLCChannel;
class Fixture;
class Scene;
class Doc;

class IntensityGenerator
{
public:
    /**
     * Create a list of scenes for the given fixtures, under $doc. The number of scenes
     * will be equal to the amount of non-colour intensity channels in $fixtures total.
     * Whether a channel is on or off in which scene(s) will be randomized.
     *
     * @param fixture A list of fixtures to create scenes for
     * @param doc The Doc* who should own the scenes
     * @param seed Random seed
     * @return A list containing randomized scenes
     */
    static QList <Scene*> randomScenes(const QList <Fixture*>& fixtures, Doc* doc,
                                       quint32 seed);

    /**
     * Generate scenes for each non-colour intensity channel within fixtures and put
     * the scenes under doc. Each scene will have exactly one channel set to full
     * and all other channels set to zero.
     *
     * @param fixture A list of fixtures to create scenes for
     * @param doc The Doc* who should own the scenes
     * @return A list containing scenes for each intensity channel
     */
    static QList <Scene*> sequenceScenes(const QList <Fixture*>& fixtures, Doc* doc);

    /**
     * Generate full & zero scenes for a list of fixtures and put the scenes under doc.
     * The first scene of the list has each non-colour intensity channel set to full.
     * The second scene has each non-colour intensity channel set to zero.
     *
     * @param fixture A list of fixtures to create scenes for
     * @param doc The Doc* who should own the scenes
     * @return A list containing two scenes; full and zero
     */
    static QList <Scene*> fullZeroScenes(const QList <Fixture*>& fixtures, Doc* doc);

    /**
     * Generate even & odd scenes for a list of fixtures and put the scenes under doc.
     * The first scene of the list has each odd channel (sequentially speaking) set to
     * full, while each even channel is set to zero. The second scene has each odd
     * channel set to zero and each even channel set to full, respectively.
     *
     * @param fixture A list of fixtures to create scenes for
     * @param doc The Doc* who should own the scenes
     * @return A list containing two scenes; odd and even
     */
    static QList <Scene*> evenOddScenes(const QList <Fixture*>& fixtures, Doc* doc);

    /**
     * Find an intensity channel from an intelligent fixture. Only channels whose
     * colour() == QLCChannel::NoColour will be searched for.
     *
     * @param fixture The fixture to search from
     * @return A relative channel number or QLCChannel::invalidChannel() if not found.
     */
    static quint32 intensityChannel(const Fixture* fixture);

    /**
     * Search for the minimum and maximum limits for a "dimmer" feature from
     * the given channel. Some fixtures (especially the cheap ones) try to put as
     * many features to one channel as possible so many times only a part of an
     * intensity channel's value space is dedicated to dimmer control. Therefore,
     * we need to search for the actual dimmer control values from the channel.
     *
     * @param channel[in] The channel to search from
     * @param min[out] The adjusted minimum value for dimmer control
     * @param max[out] The adjusted maximum value for dimmer control
     */
    static void adjustLimits(const QLCChannel* channel, uchar* min, uchar* max);
};

#endif
