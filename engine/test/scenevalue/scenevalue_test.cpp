/*
  Q Light Controller Plus - Unit test
  scenevalue_test.cpp

  Copyright (c) Heikki Junnila
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

#include <QtTest>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "scenevalue_test.h"
#include "scenevalue.h"

void SceneValue_Test::initial()
{
    SceneValue scv;
    QVERIFY(scv.isValid() == false);
    QVERIFY(scv.fxi == Fixture::invalidId());
    QVERIFY(scv.channel == QLCChannel::invalid());
    QVERIFY(scv.value == 0);

    SceneValue scv2(31337, 5150, 42);
    QVERIFY(scv2.isValid() == true);
    QVERIFY(scv2.fxi == 31337);
    QVERIFY(scv2.channel == 5150);
    QVERIFY(scv2.value == 42);
}

void SceneValue_Test::lessThan()
{
    SceneValue scv1;
    SceneValue scv2;

    /* Both have invalid ID and channel; neither is less than the other */
    QVERIFY((scv1 < scv2) == false);
    QVERIFY((scv2 < scv1) == false);

    /* Both save same ID, invalid channel; same here */
    scv1.fxi = 0;
    scv2.fxi = 0;
    QVERIFY((scv1 < scv2) == false);
    QVERIFY((scv2 < scv1) == false);

    /* Same ID, different channel */
    scv1.channel = 0;
    scv2.channel = 1;
    QVERIFY((scv1 < scv2) == true);
    QVERIFY((scv2 < scv1) == false);

    /* Different ID, different channel */
    scv1.fxi = 1;
    scv2.fxi = 0;
    QVERIFY((scv1 < scv2) == false);
    QVERIFY((scv2 < scv1) == true);

    /* Different ID, same channel */
    scv1.channel = 1;
    scv2.channel = 1;
    QVERIFY((scv1 < scv2) == false);
    QVERIFY((scv2 < scv1) == true);
}

void SceneValue_Test::loadSuccess()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Value");
    xmlWriter.writeAttribute("Fixture", "5");
    xmlWriter.writeAttribute("Channel", "60");
    xmlWriter.writeCharacters("100");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    SceneValue scv;
    QVERIFY(scv.loadXML(xmlReader) == true);
    QVERIFY(scv.fxi == 5);
    QVERIFY(scv.channel == 60);
    QVERIFY(scv.value == 100);
}

void SceneValue_Test::loadWrongRoot()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Valu");
    xmlWriter.writeAttribute("Fixture", "5");
    xmlWriter.writeAttribute("Channel", "60");
    xmlWriter.writeCharacters("100");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    SceneValue scv;
    QVERIFY(scv.loadXML(xmlReader) == false);
}

void SceneValue_Test::loadWrongFixture()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Value");
    xmlWriter.writeAttribute("Fixture", QString::number(Fixture::invalidId()));
    xmlWriter.writeAttribute("Channel", "60");
    xmlWriter.writeCharacters("100");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    SceneValue scv;
    QVERIFY(scv.loadXML(xmlReader) == false);
}

void SceneValue_Test::loadWrongValue()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Value");
    xmlWriter.writeAttribute("Fixture", "5");
    xmlWriter.writeAttribute("Channel", "60");
    xmlWriter.writeCharacters("257");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    SceneValue scv;
    QVERIFY(scv.loadXML(xmlReader) == true);
    QVERIFY(scv.value == 1);
}

void SceneValue_Test::save()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    SceneValue scv(4, 8, 16);
    QVERIFY(scv.saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(xmlReader.name().toString() == "Value");
    QVERIFY(xmlReader.attributes().value("Fixture").toString().toInt() == 4);
    QVERIFY(xmlReader.attributes().value("Channel").toString().toInt() == 8);
    QVERIFY(xmlReader.readElementText().toInt() == 16);
}

QTEST_APPLESS_MAIN(SceneValue_Test)
