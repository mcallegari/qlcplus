/*
  Q Light Controller Plus
  scenevalue.h

  Copyright (C) Heikki Junnila
                Massimo Callegari

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

#ifndef SCENEVALUE_H
#define SCENEVALUE_H

#include <QtGlobal>
#include <QMetaType>

#include "fixture.h"

class QXmlStreamReader;

/** @addtogroup engine_functions Functions
 * @{
 */

#define KXMLQLCSceneValue        QString("Value")
#define KXMLQLCSceneValueFixture QString("Fixture")
#define KXMLQLCSceneValueChannel QString("Channel")

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

    /** NON-virtual Destructor
     *
     *  No class derives from this one and we need to keep the memory footprint
     *  as low as possible.
     *
     *  TODO C++11: mark this as final
     */
    ~SceneValue();

    /** A SceneValue is not valid if .fxi == Fixture::invalidId() */
    bool isValid() const;

    SceneValue& operator=(const SceneValue& scv);

    /** Comparator function for qSort() */
    bool operator< (const SceneValue& scv) const;

    /** Comparator function for matching SceneValues */
    bool operator== (const SceneValue& scv) const;

    /** Load this SceneValue's contents from an XML tag */
    bool loadXML(QXmlStreamReader &tag);

    /** Save this SceneValue to an XML document */
    bool saveXML(QXmlStreamWriter *doc) const;

public:
    /** Fixture ID */
    quint32 fxi;
    quint32 channel;
    uchar value;
};

Q_DECLARE_METATYPE(SceneValue)

QDebug operator<<(QDebug debug, const SceneValue &sv);

/** @} */

#endif
