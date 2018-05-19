/*
  Q Light Controller Plus - Unit tests
  qlccapability_test.cpp

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

#include <QtTest>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "qlccapability_test.h"
#include "qlccapability.h"

void QLCCapability_Test::initial()
{
    QLCCapability cap;
    QVERIFY(cap.min() == 0);
    QVERIFY(cap.max() == UCHAR_MAX);
    QVERIFY(cap.name().isEmpty());
}

void QLCCapability_Test::min_data()
{
    QTest::addColumn<uchar> ("value");
    for (uchar i = 0; i < UCHAR_MAX; i++)
        QTest::newRow("foo") << i;
    QTest::newRow("foo") << uchar(UCHAR_MAX);
}

void QLCCapability_Test::min()
{
    QLCCapability cap;
    QVERIFY(cap.min() == 0);

    QFETCH(uchar, value);

    cap.setMin(value);
    QCOMPARE(cap.min(), value);
}

void QLCCapability_Test::max_data()
{
    QTest::addColumn<uchar> ("value");
    for (uchar i = 0; i < UCHAR_MAX; i++)
        QTest::newRow("foo") << i;
    QTest::newRow("foo") << uchar(UCHAR_MAX);
}

void QLCCapability_Test::max()
{
    QLCCapability cap;
    QVERIFY(cap.max() == UCHAR_MAX);

    QFETCH(uchar, value);

    cap.setMax(value);
    QCOMPARE(cap.max(), value);
}

void QLCCapability_Test::middle()
{
    QLCCapability cap;
    QVERIFY(cap.max() == UCHAR_MAX);
    QCOMPARE(cap.middle(), uchar(127));
    cap.setMin(100);
    cap.setMax(200);
    QCOMPARE(cap.middle(), uchar(150));
}

void QLCCapability_Test::name()
{
    QLCCapability cap;
    QVERIFY(cap.name().isEmpty());

    cap.setName("Foobar");
    QVERIFY(cap.name() == "Foobar");
}

void QLCCapability_Test::alias()
{
    QLCCapability cap;
    AliasInfo info1, info2;
    info1.sourceChannel = "Channel 1";
    info1.targetChannel = "Channel 3";
    info1.targetMode = "12 Channel";

    info2.sourceChannel = "Foo";
    info2.targetChannel = "Bar";
    info2.targetMode = "Mode";

    cap.addAlias(info1);
    QVERIFY(cap.aliasList().count() == 1);

    cap.removeAlias(info1);
    QVERIFY(cap.aliasList().count() == 0);

    cap.addAlias(info1);
    cap.addAlias(info2);
    QVERIFY(cap.aliasList().count() == 2);

    cap.removeAlias(info1);
    QVERIFY(cap.aliasList().count() == 1);

    info1.sourceChannel = "John";
    info1.targetChannel = "Doe";
    QList<AliasInfo> aliasList;
    aliasList << info1 << info2;

    cap.replaceAliases(aliasList);
    QVERIFY(cap.aliasList().count() == 2);
    QVERIFY(cap.aliasList().first().sourceChannel == "John");
    QVERIFY(cap.aliasList().first().targetChannel == "Doe");
}

void QLCCapability_Test::overlaps()
{
    QLCCapability cap1;
    QLCCapability cap2;
    QVERIFY(cap1.overlaps(&cap2) == true);
    QVERIFY(cap2.overlaps(&cap1) == true);

    /* cap2 contains cap1 completely */
    cap1.setMin(10);
    cap1.setMax(245);
    QVERIFY(cap1.overlaps(&cap2) == true);
    QVERIFY(cap2.overlaps(&cap1) == true);

    /* cap2's max overlaps cap1 */
    cap2.setMin(0);
    cap2.setMax(10);
    QVERIFY(cap1.overlaps(&cap2) == true);
    QVERIFY(cap2.overlaps(&cap1) == true);

    /* cap2's max overlaps cap1 */
    cap2.setMin(0);
    cap2.setMax(15);
    QVERIFY(cap1.overlaps(&cap2) == true);
    QVERIFY(cap2.overlaps(&cap1) == true);

    /* cap2's min overlaps cap1 */
    cap2.setMin(245);
    cap2.setMax(UCHAR_MAX);
    QVERIFY(cap1.overlaps(&cap2) == true);
    QVERIFY(cap2.overlaps(&cap1) == true);

    /* cap2's min overlaps cap1 */
    cap2.setMin(240);
    cap2.setMax(UCHAR_MAX);
    QVERIFY(cap1.overlaps(&cap2) == true);
    QVERIFY(cap2.overlaps(&cap1) == true);

    /* cap1 contains cap2 completely */
    cap2.setMin(20);
    cap2.setMax(235);
    QVERIFY(cap1.overlaps(&cap2) == true);
    QVERIFY(cap2.overlaps(&cap1) == true);

    cap2.setMin(0);
    cap2.setMax(9);
    QVERIFY(cap1.overlaps(&cap2) == false);
    QVERIFY(cap2.overlaps(&cap1) == false);
}

void QLCCapability_Test::copy()
{
    QLCCapability cap1;
    QVERIFY(cap1.min() == 0);
    QVERIFY(cap1.max() == UCHAR_MAX);
    QVERIFY(cap1.name().isEmpty());

    cap1.setMin(5);
    cap1.setMax(15);
    cap1.setName("Foobar");

    QLCCapability *cap2 = cap1.createCopy();
    QVERIFY(cap2->min() == 5);
    QVERIFY(cap2->max() == 15);
    QVERIFY(cap2->name() == "Foobar");
}

void QLCCapability_Test::load()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Capability");
    xmlWriter.writeAttribute("Min", "13");
    xmlWriter.writeAttribute("Max", "19");
    xmlWriter.writeCharacters("Test1");
    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QLCCapability cap;
    QVERIFY(cap.loadXML(xmlReader) == true);
    QVERIFY(cap.name() == "Test1");
    QVERIFY(cap.min() == 13);
    QVERIFY(cap.max() == 19);
}

void QLCCapability_Test::loadWrongRoot()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("apability");
    xmlWriter.writeAttribute("Min", "13");
    xmlWriter.writeAttribute("Max", "19");
    xmlWriter.writeCharacters("Test1");
    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QLCCapability cap;
    QVERIFY(cap.loadXML(xmlReader) == false);
    QVERIFY(cap.name().isEmpty());
    QVERIFY(cap.min() == 0);
    QVERIFY(cap.max() == UCHAR_MAX);
}

void QLCCapability_Test::loadNoMin()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Capability");
    xmlWriter.writeAttribute("Max", "19");
    xmlWriter.writeCharacters("Test1");
    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QLCCapability cap;
    QVERIFY(cap.loadXML(xmlReader) == false);
    QVERIFY(cap.name().isEmpty());
    QVERIFY(cap.min() == 0);
    QVERIFY(cap.max() == UCHAR_MAX);
}

void QLCCapability_Test::loadNoMax()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Capability");
    xmlWriter.writeAttribute("Min", "13");
    xmlWriter.writeCharacters("Test1");
    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QLCCapability cap;
    QVERIFY(cap.loadXML(xmlReader) == false);
    QVERIFY(cap.name().isEmpty());
    QVERIFY(cap.min() == 0);
    QVERIFY(cap.max() == UCHAR_MAX);
}

void QLCCapability_Test::loadMinGreaterThanMax()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Capability");
    xmlWriter.writeAttribute("Min", "20");
    xmlWriter.writeAttribute("Max", "19");
    xmlWriter.writeCharacters("Test1");
    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QLCCapability cap;
    QVERIFY(cap.loadXML(xmlReader) == false);
    QVERIFY(cap.name().isEmpty());
    QVERIFY(cap.min() == 0);
    QVERIFY(cap.max() == UCHAR_MAX);
}

void QLCCapability_Test::save()
{
    QLCCapability cap;
    cap.setName("Testing");
    cap.setMin(5);
    cap.setMax(87);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(cap.saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);

    xmlReader.readNextStartElement();

    QVERIFY(xmlReader.name().toString() == "Capability");
    QVERIFY(xmlReader.attributes().value("Min").toString() == "5");
    QVERIFY(xmlReader.attributes().value("Max").toString() == "87");
    QVERIFY(xmlReader.readElementText() == "Testing");
}

void QLCCapability_Test::savePreset()
{
    QLCCapability cap;
    cap.setName("PresetTest");
    cap.setMin(0);
    cap.setMax(127);
    cap.setPreset(QLCCapability::StrobeFreqRange);
    cap.setResource(0, 1);
    cap.setResource(1, 30);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(cap.saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);

    xmlReader.readNextStartElement();

    QVERIFY(xmlReader.name().toString() == "Capability");
    QVERIFY(xmlReader.attributes().value("Min").toString() == "0");
    QVERIFY(xmlReader.attributes().value("Max").toString() == "127");
    QVERIFY(xmlReader.attributes().value("Preset").toString() == "StrobeFreqRange");
    QVERIFY(xmlReader.attributes().value("Res1").toString() == "1");
    QVERIFY(xmlReader.attributes().value("Res2").toString() == "30");
    QVERIFY(xmlReader.readElementText() == "PresetTest");
}

void QLCCapability_Test::saveAlias()
{
    QLCCapability cap;
    cap.setName("PresetTest");
    cap.setMin(10);
    cap.setMax(20);

    AliasInfo alias;
    alias.sourceChannel = "Channel 1";
    alias.targetChannel = "Channel 3";
    alias.targetMode = "12 Channel";
    cap.addAlias(alias);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(cap.saveXML(&xmlWriter) == true);

    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QLCCapability capRead;
    QVERIFY(capRead.loadXML(xmlReader) == true);

    QVERIFY(capRead.aliasList().count() == 1);
    QVERIFY(cap.aliasList().first().sourceChannel == "Channel 1");
    QVERIFY(cap.aliasList().first().targetChannel == "Channel 3");
    QVERIFY(cap.aliasList().first().targetMode == "12 Channel");
}

QTEST_APPLESS_MAIN(QLCCapability_Test)
