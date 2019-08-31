/*
  Q Light Controller Plus
  scenevalue.cpp

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>

#include "scenevalue.h"

SceneValue::SceneValue(quint32 id, quint32 ch, uchar val)
    : fxi(id)
    , channel(ch)
    , value(val)
{
}

SceneValue::SceneValue(const SceneValue& scv)
    : fxi(scv.fxi)
    , channel(scv.channel)
    , value(scv.value)
{
}

SceneValue::~SceneValue()
{
}

bool SceneValue::isValid() const
{
    if (fxi == Fixture::invalidId())
        return false;
    else
        return true;
}

SceneValue &SceneValue::operator=(const SceneValue &scv)
{
    if (this != &scv)
    {
        fxi = scv.fxi;
        channel = scv.channel;
        value = scv.value;
    }

    return *this;
}

bool SceneValue::operator<(const SceneValue& scv) const
{
    if (fxi < scv.fxi)
    {
        return true;
    }
    else if (fxi == scv.fxi)
    {
        if (channel < scv.channel)
            return true;
        else
            return false;
    }
    else
    {
        return false;
    }
}

bool SceneValue::operator==(const SceneValue& scv) const
{
    if (fxi == scv.fxi && channel == scv.channel)
        return true;
    else
        return false;
}

bool SceneValue::loadXML(QXmlStreamReader &tag)
{
    if (tag.name() != KXMLQLCSceneValue)
    {
        qWarning() << Q_FUNC_INFO << "Scene Value node not found";
        return false;
    }

    QXmlStreamAttributes attrs = tag.attributes();
    fxi = attrs.value(KXMLQLCSceneValueFixture).toString().toUInt();
    channel = attrs.value(KXMLQLCSceneValueChannel).toString().toUInt();
    value = uchar(tag.readElementText().toUInt());

    return isValid();
}

bool SceneValue::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    /* Value tag and its attributes */
    doc->writeStartElement(KXMLQLCSceneValue);
    doc->writeAttribute(KXMLQLCSceneValueFixture, QString::number(fxi));
    doc->writeAttribute(KXMLQLCSceneValueChannel, QString::number(channel));

    /* The actual value as node text */
    doc->writeCharacters(QString("%1").arg(value));
    doc->writeEndElement();

    return true;
}

QDebug operator<<(QDebug debug, const SceneValue &sv)
{
    debug.nospace() << "SceneValue(" << sv.fxi << ", " << sv.channel << ", " << sv.value << ")";
    return debug;
}
