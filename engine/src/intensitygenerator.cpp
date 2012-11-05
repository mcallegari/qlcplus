/*
  Q Light Controller
  intensitygenerator.cpp

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

#include <QString>

#include "qlccapability.h"
#include "qlcchannel.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"

#include "intensitygenerator.h"

QList <Scene*> IntensityGenerator::randomScenes(const QList <Fixture*>& fixtures, Doc* doc, quint32 seed)
{
    QList <Scene*> scenes;

    // Initialize random number generator
    srand(seed);

    // Make two passes; create scenes on the first and set values on the second.
    for (int pass = 0; pass < 2; pass++)
    {
        int sequence = 0;
        foreach (Fixture* fxi, fixtures)
        {
            if (fxi->isDimmer() == true)
            {
                for (quint32 ch = 0; ch < fxi->channels(); ch++)
                {
                    if (pass == 0)
                    {
                        // Create a new scene for each dimmer channel
                        scenes << new Scene(doc);
                    }
                    else
                    {
                        for (int i = 0; i < scenes.size(); i++)
                        {
                            Q_ASSERT(scenes[i] != NULL);
                            if (rand() % 2 == 0)
                                ; //scenes[i]->setValue(fxi->id(), ch, 0);
                            else
                                scenes[i]->setValue(fxi->id(), ch, UCHAR_MAX);
                        }
                    }

                    sequence++;
                }
            }
            else
            {
                quint32 ch = intensityChannel(fxi);
                if (ch != QLCChannel::invalid())
                {
                    uchar min = 0;
                    uchar max = UCHAR_MAX;
                    adjustLimits(fxi->channel(ch), &min, &max);

                    if (pass == 0)
                    {
                        // Create a new scene for each non-dimmer fixture
                        scenes << new Scene(doc);
                    }
                    else
                    {
                        for (int i = 0; i < scenes.size(); i++)
                        {
                            Q_ASSERT(scenes[i] != NULL);
                            if (rand() % 2 == 0)
                                ; //scenes[i]->setValue(fxi->id(), ch, min);
                            else
                                scenes[i]->setValue(fxi->id(), ch, max);
                        }
                    }

                    sequence++;
                }
            }
        }
    }

    return scenes;
}

QList <Scene*> IntensityGenerator::sequenceScenes(const QList <Fixture*>& fixtures, Doc* doc)
{
    QList <Scene*> scenes;

    // Make two passes; create scenes on the first and set just the one channel to full.
    // Set the remaining channels to zero on the second pass.
    for (int pass = 0; pass < 2; pass++)
    {
        int sequence = 0;
        foreach (Fixture* fxi, fixtures)
        {
            if (fxi->isDimmer() == true)
            {
                for (quint32 ch = 0; ch < fxi->channels(); ch++)
                {
                    if (pass == 0)
                    {
                        // Create a new scene for each dimmer channel
                        Scene* scene = new Scene(doc);
                        // ...and set that one channel to full
                        scene->setValue(fxi->id(), ch, UCHAR_MAX);
                        scenes << scene;
                    }
                    else
                    {
                        /*
                        for (int i = 0; i < scenes.size(); i++)
                        {
                            Scene* scene = scenes[i];
                            Q_ASSERT(scene != NULL);

                            // Getting a zero value from the scene likely means
                            // there's no such fxi+ch combination at all there.
                            if (scene->value(fxi->id(), ch) == 0)
                                scene->setValue(fxi->id(), ch, 0);
                        }
                        */
                    }

                    sequence++;
                }
            }
            else
            {
                quint32 ch = intensityChannel(fxi);
                if (ch != QLCChannel::invalid())
                {
                    uchar min = 0;
                    uchar max = UCHAR_MAX;
                    adjustLimits(fxi->channel(ch), &min, &max);

                    if (pass == 0)
                    {
                        // Create a new scene for each non-dimmer fixture
                        Scene* scene = new Scene(doc);
                        // ...and set that one channel to $max
                        scene->setValue(fxi->id(), ch, max);
                        scenes << scene;
                    }
                    else
                    {
                        /*
                        for (int i = 0; i < scenes.size(); i++)
                        {
                            Scene* scene = scenes[i];
                            Q_ASSERT(scene != NULL);

                            // If the current fixture's intensity channel has not already
                            // been set to $max, it should be set to $min
                            if (scene->value(fxi->id(), ch) != max)
                                scene->setValue(fxi->id(), ch, min);
                        }
                        */
                    }

                    sequence++;
                }
            }
        }
    }

    return scenes;
}

QList <Scene*> IntensityGenerator::fullZeroScenes(const QList <Fixture*>& fixtures, Doc* doc)
{
    QList <Scene*> scenes;

    scenes << new Scene(doc);
    scenes << new Scene(doc);

    foreach (Fixture* fxi, fixtures)
    {
        if (fxi->isDimmer() == true)
        {
            for (quint32 ch = 0; ch < fxi->channels(); ch++)
            {
                scenes[0]->setValue(fxi->id(), ch, UCHAR_MAX);
                //scenes[1]->setValue(fxi->id(), ch, 0);
            }
        }
        else
        {
            quint32 ch = intensityChannel(fxi);
            if (ch != QLCChannel::invalid())
            {
                uchar min = 0;
                uchar max = UCHAR_MAX;
                adjustLimits(fxi->channel(ch), &min, &max);

                scenes[0]->setValue(fxi->id(), ch, max);
                //scenes[1]->setValue(fxi->id(), ch, min);
            }
        }
    }

    return scenes;
}

QList <Scene*> IntensityGenerator::evenOddScenes(const QList <Fixture*>& fixtures, Doc* doc)
{
    QList <Scene*> scenes;

    scenes << new Scene(doc);
    scenes << new Scene(doc);

    int sequence = 0;

    foreach (Fixture* fxi, fixtures)
    {
        if (fxi->isDimmer() == true)
        {
            // All channels in a dimmer control individual intensities
            for (quint32 ch = 0; ch < fxi->channels(); ch++)
            {
                if (sequence % 2)
                {
                    // Even
                    scenes[0]->setValue(fxi->id(), ch, UCHAR_MAX);
                    //scenes[1]->setValue(fxi->id(), ch, 0);
                }
                else
                {
                    // Odd
                    //scenes[0]->setValue(fxi->id(), ch, 0);
                    scenes[1]->setValue(fxi->id(), ch, UCHAR_MAX);
                }

                sequence++;
            }
        }
        else
        {
            // Assume a non-dimmer fixture has max one channel for general intensity
            quint32 ch = intensityChannel(fxi);
            if (ch != QLCChannel::invalid())
            {
                uchar min = 0;
                uchar max = UCHAR_MAX;
                adjustLimits(fxi->channel(ch), &min, &max);

                if (sequence % 2)
                {
                    // Even
                    //scenes[1]->setValue(fxi->id(), ch, min);
                    scenes[0]->setValue(fxi->id(), ch, max);
                }
                else
                {
                    // Odd
                    scenes[1]->setValue(fxi->id(), ch, max);
                    //scenes[0]->setValue(fxi->id(), ch, min);
                }
            }

            sequence++;
        }
    }


    return scenes;
}

quint32 IntensityGenerator::intensityChannel(const Fixture* fixture)
{
    if (fixture == NULL)
        return QLCChannel::invalid();

    for (quint32 ch = 0; ch < fixture->channels(); ch++)
    {
        const QLCChannel* channel = fixture->channel(ch);
        Q_ASSERT(channel != NULL);

        if (channel->group() == QLCChannel::Intensity)
        {
            if (channel->colour() == QLCChannel::NoColour)
                return ch;
        }
    }

    return QLCChannel::invalid();
}

void IntensityGenerator::adjustLimits(const QLCChannel* channel, uchar* min, uchar* max)
{
    Q_ASSERT(channel != NULL);
    Q_ASSERT(min != NULL);
    Q_ASSERT(max != NULL);

    // The intensity channel contains also something else than just a
    // dimmer if it has more than one capability -> a minmax search is needed.
    // Try to be smart and guess which capability provides dimmer
    // intensity. If a suitable capability is not found, values are not
    // modified.
    if (channel->capabilities().size() > 1)
    {
        const QLCCapability* cap = NULL;

        // Search for (I|i)ntensity or (D|d)immer capability
        /** @todo This does not support localized channels */
        cap = channel->searchCapability("ntensity", false);
        if (cap == NULL)
            cap = channel->searchCapability("immer", false);

        if (cap != NULL)
        {
            *min = cap->min();
            *max = cap->max();
        }
        else
        {
            *min = 0;
            *max = UCHAR_MAX;
        }
    }
}
