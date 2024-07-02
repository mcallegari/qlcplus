/*
  Q Light Controller Plus
  fixtureutils.h

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

#ifndef FIXTUREUTILS_H
#define FIXTUREUTILS_H

#include <QColor>
#include <QPointF>
#include <QVector3D>

class Doc;
class Fixture;
class QLCChannel;
class QLCCapability;
class QLCFixtureMode;
class MonitorProperties;

class FixtureUtils
{
public:
    FixtureUtils();

    /** Returns a unique item ID composed by the provided parameters */
    static quint32 fixtureItemID(quint32 fid, quint16 headIndex, quint16 linkedIndex);
    /** Returns a Fixture ID from the given itemID composite ID */
    static quint32 itemFixtureID(quint32 itemID);
    /** Returns the head index from the given itemID composite ID */
    static quint16 itemHeadIndex(quint32 itemID);
    /** Returns the linked index from the given itemID composite ID */
    static quint16 itemLinkedIndex(quint32 itemID);

    static QPointF item2DPosition(MonitorProperties *monProps, int pointOfView, QVector3D pos);
    static float item2DRotation(int pointOfView, QVector3D rot);
    static QSizeF item2DDimension(QLCFixtureMode *fxMode, int pointOfView);
    static void alignItem(QVector3D refPos, QVector3D &origPos, int pointOfView, int alignment);

    static QVector3D item3DPosition(MonitorProperties *monProps, QPointF point, float thirdVal);

    /** Returns the first available space (in mm) for a rectangle
     * of the given width and height.
     * This method works with the monitor properties and the fixtures list */
    static QPointF available2DPosition(Doc *doc, int pointOfView, QRectF fxRect);

    /** Perform a linear blending of $b over $a with the given $mix amount */
    static QColor blendColors(QColor a, QColor b, float mix);

    /** Return the color of the head with $headIndex of $fixture.
     *  This considers: RGB / CMY / WAUVLI channels, dimmers and gel color */
    static QColor headColor(Fixture *fixture, bool hasDimmer, int headIndex = 0);

    static QColor applyColorFilter(QColor source, QColor filter);

    /** Calculate the pan/tilt speed depending on the $ch preset */
    static void positionTimings(const QLCChannel *ch, uchar value, int &panDuration, int &tiltDuration);

    /** Calculate the gobo wheel speed depending on $cap preset */
    static bool goboTiming(const QLCCapability *cap, uchar value, int &speed);

    /** Calculate the rise/fall periods for a shutter channel $ch, considering presets */
    static int shutterTimings(const QLCChannel *ch, uchar value, int &highTime, int &lowTime);
};

#endif // FIXTUREUTILS_H
