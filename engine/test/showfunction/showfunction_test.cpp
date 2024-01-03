/*
  Q Light Controller Plus - Unit test
  ShowFunction_Test.cpp

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

#include <QtTest>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "showfunction_test.h"
#include "showfunction.h"

void ShowFunction_Test::defaults()
{
    ShowFunction sf(123);

    // check defaults
    QVERIFY(sf.id() == 123);
    QVERIFY(sf.functionID() == Function::invalidId());
    QVERIFY(sf.startTime() == UINT_MAX);
    QVERIFY(sf.duration() == 0);
    QCOMPARE(sf.color(), QColor::Invalid);
    QCOMPARE(sf.isLocked(), false);
    QVERIFY(sf.intensityOverrideId() == -1);

    QCOMPARE(sf.defaultColor(Function::SceneType), QColor(100, 100, 100));
    QCOMPARE(sf.defaultColor(Function::ChaserType), QColor(85, 107, 128));
    QCOMPARE(sf.defaultColor(Function::AudioType), QColor(96, 128, 83));
    QCOMPARE(sf.defaultColor(Function::RGBMatrixType), QColor(101, 155, 155));
    QCOMPARE(sf.defaultColor(Function::EFXType), QColor(128, 60, 60));
    QCOMPARE(sf.defaultColor(Function::VideoType), QColor(147, 140, 20));

    // set & check base params
    sf.setFunctionID(123);
    sf.setStartTime(445566);
    sf.setDuration(778899);
    sf.setColor(QColor(Qt::red));
    sf.setLocked(true);
    sf.setIntensityOverrideId(468);

    QVERIFY(sf.functionID() == 123);
    QVERIFY(sf.startTime() == 445566);
    QVERIFY(sf.duration() == 778899);
    QCOMPARE(sf.color(), QColor(Qt::red));
    QCOMPARE(sf.isLocked(), true);
    QCOMPARE(sf.intensityOverrideId(), 468);
}

void ShowFunction_Test::load()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("ShowFunction");
    xmlWriter.writeAttribute("ID", "321");
    xmlWriter.writeAttribute("StartTime", "665544");
    xmlWriter.writeAttribute("Duration", "998877");
    xmlWriter.writeAttribute("Color", "#AABBCC");
    xmlWriter.writeAttribute("Locked", "1");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    ShowFunction sf(456);
    QVERIFY(sf.loadXML(xmlReader) == true);

    QVERIFY(sf.functionID() == 321);
    QVERIFY(sf.startTime() == 665544);
    QVERIFY(sf.duration() == 998877);
    QCOMPARE(sf.color(), QColor(0xAA, 0xBB, 0xCC));
    QCOMPARE(sf.isLocked(), true);
}

void ShowFunction_Test::save()
{
    ShowFunction sf(789);
    sf.setFunctionID(123);
    sf.setStartTime(445566);
    sf.setDuration(778899);
    sf.setColor(QColor(Qt::red));
    sf.setLocked(true);
    sf.setIntensityOverrideId(468);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(sf.saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);

    xmlReader.readNextStartElement();
    QVERIFY(xmlReader.name().toString() == "ShowFunction");

    QVERIFY(xmlReader.attributes().value("ID").toString() == "123");
    QVERIFY(xmlReader.attributes().value("StartTime").toString() == "445566");
    QVERIFY(xmlReader.attributes().value("Duration").toString() == "778899");
    QVERIFY(xmlReader.attributes().value("Color").toString() == "#ff0000");
    QVERIFY(xmlReader.attributes().value("Locked").toString() == "1");
}

QTEST_APPLESS_MAIN(ShowFunction_Test)
