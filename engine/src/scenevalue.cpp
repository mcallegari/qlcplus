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
    : value(val)
    , fixture_channel(compose(id, ch))
{
}

SceneValue::SceneValue(const SceneValue& scv)
    : value(scv.value)
    , fixture_channel(scv.fixture_channel)
{
}

SceneValue::~SceneValue()
{
}

void SceneValue::assign(quint32 id, quint32 ch, uchar val)
{
    fixture_channel = compose(id,ch);
    value = val;
}

bool SceneValue::isValid() const
{
    return fxi() != Fixture::invalidId();
}

bool SceneValue::operator<(const SceneValue& scv) const
{
    return fixture_channel < scv.fixture_channel;
}

bool SceneValue::operator==(const SceneValue& scv) const
{
    return fixture_channel == scv.fixture_channel;
}

bool SceneValue::loadXML(QXmlStreamReader &tag)
{
    if (tag.name() != KXMLQLCSceneValue)
    {
        qWarning() << Q_FUNC_INFO << "Scene Value node not found";
        return false;
    }

    QXmlStreamAttributes attrs = tag.attributes();
    assign(
        attrs.value(KXMLQLCSceneValueFixture).toUInt(),
        attrs.value(KXMLQLCSceneValueChannel).toUInt(),
        uchar(tag.readElementText().toUInt()));

    return isValid();
}

bool SceneValue::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    /* Value tag and its attributes */
    doc->writeStartElement(KXMLQLCSceneValue);
    doc->writeAttribute(KXMLQLCSceneValueFixture, QString::number(fxi()));
    doc->writeAttribute(KXMLQLCSceneValueChannel, QString::number(channel()));

    /* The actual value as node text */
    doc->writeCharacters(QString("%1").arg(value));
    doc->writeEndElement();

    return true;
}

quint32 SceneValue::fxi() const
{
    quint32 ret = fixture_channel >> 16;
    if (ret == 0xffff)
        return Fixture::invalidId();
    return ret;
}

quint32 SceneValue::channel() const
{
    quint32 ret = fixture_channel & 0xffff;
    if (ret == 0xffff)
        return QLCChannel::invalid();
    return ret;
}

quint32 SceneValue::compose(quint32 id, quint32 ch)
{
    if (id == Fixture::invalidId())
        id = 0xffff;

    if (ch == QLCChannel::invalid())
        ch = 0xffff;

    return id << 16 | ch;
}
