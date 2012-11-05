/*
  Q Light Controller
  scenevalue.h

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

#ifndef SCENEVALUE_H
#define SCENEVALUE_H

#include <QtGlobal>

#include "fixture.h"

class QDomDocument;
class QDomElement;

#define KXMLQLCSceneValue "Value"
#define KXMLQLCSceneValueFixture "Fixture"
#define KXMLQLCSceneValueChannel "Channel"

/**
 * SceneValue is a helper class used to store individual channel TARGET values
 * for Scene functions. Each channel that is taking part in a scene is
 * represented with a SceneValue.
 *
 * A SceneValue consists of a fixture, channel and value. Fixture tells, which
 * fixture a particular value corresponds to, channel contains the particular
 * channel number from the fixture and, value tells the exact target value for
 * that channel. Channel numbers are not absolute DMX channels because the
 * fixture address can be changed at any time. Instead, channel number tells
 * the relative channel number, respective to fixture address.
 *
 * For example:
 * Let's say we have a SceneValue with channel = 5, value = 127 and, the
 * fixture assigned to the SceneValue is at DMX address 100. Thus, the
 * SceneValue represents value 127 for absolute DMX channel 105. If the address
 * of the fixture is changed, the SceneValue doesn't need to be touched at all.
 */
class SceneValue
{
public:
    /** Normal constructor */
    SceneValue(quint32 fxi_id = Fixture::invalidId(),
               quint32 channel = QLCChannel::invalid(),
               uchar value = 0);

    /** Copy constructor */
    SceneValue(const SceneValue& scv);

    /** Destructor */
    virtual ~SceneValue();

    /** A SceneValue is not valid if .fxi == Fixture::invalidId() */
    bool isValid() const;

    /** Comparator function for qSort() */
    bool operator< (const SceneValue& scv) const;

    /** Comparator function for matching SceneValues */
    bool operator== (const SceneValue& scv) const;

    /** Load this SceneValue's contents from an XML tag */
    bool loadXML(const QDomElement& tag);

    /** Save this SceneValue to an XML document */
    bool saveXML(QDomDocument* doc, QDomElement* scene_root) const;

public:
    quint32 fxi;
    quint32 channel;
    uchar value;
};

#endif
