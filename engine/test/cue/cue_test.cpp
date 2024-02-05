/*
  Q Light Controller Plus - Unit test
  cue_test.cpp

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

#include "cue_test.h"
#include "cue.h"

void Cue_Test::initial()
{
    Cue cue;
    QCOMPARE(cue.name(), QString());
    QCOMPARE(cue.values().size(), 0);
    QCOMPARE(cue.fadeInSpeed(), uint(0));
    QCOMPARE(cue.fadeOutSpeed(), uint(0));
    QCOMPARE(cue.duration(), uint(0));

    cue = Cue("Foo");
    QCOMPARE(cue.name(), QString("Foo"));
    QCOMPARE(cue.values().size(), 0);

    QHash <uint,uchar> values;
    values[0] = 14;
    values[932] = 5;
    values[5] = 255;
    cue = Cue(values);
    QCOMPARE(cue.name(), QString());
    QCOMPARE(cue.values().size(), 3);
    QCOMPARE(cue.values()[0], uchar(14));
    QCOMPARE(cue.values()[932], uchar(5));
    QCOMPARE(cue.values()[5], uchar(255));
}

void Cue_Test::name()
{
    Cue cue;
    cue.setName("Foobar");
    QCOMPARE(cue.name(), QString("Foobar"));
}

void Cue_Test::value()
{
    Cue cue;
    QCOMPARE(cue.value(0), uchar(0));
    QCOMPARE(cue.value(UINT_MAX), uchar(0));
    QCOMPARE(cue.value(12345), uchar(0));

    cue.setValue(0, 15);
    QCOMPARE(cue.values().size(), 1);
    QCOMPARE(cue.value(0), uchar(15));

    cue.setValue(0, 15);
    QCOMPARE(cue.values().size(), 1);
    QCOMPARE(cue.value(0), uchar(15));

    cue.setValue(UINT_MAX, 42);
    QCOMPARE(cue.values().size(), 2);
    QCOMPARE(cue.value(0), uchar(15));
    QCOMPARE(cue.value(UINT_MAX), uchar(42));

    cue.unsetValue(0);
    QCOMPARE(cue.values().size(), 1);
    QCOMPARE(cue.value(UINT_MAX), uchar(42));

    cue.unsetValue(0);
    QCOMPARE(cue.values().size(), 1);
    QCOMPARE(cue.value(UINT_MAX), uchar(42));
}

void Cue_Test::copy()
{
    Cue cue1("Foo");
    cue1.setValue(0, 1);
    cue1.setValue(1, 2);
    cue1.setValue(2, 3);
    cue1.setFadeInSpeed(10);
    cue1.setFadeOutSpeed(20);
    cue1.setDuration(30);

    Cue cue2 = cue1;
    QCOMPARE(cue2.name(), QString("Foo"));
    QCOMPARE(cue2.values().size(), 3);
    QCOMPARE(cue2.value(0), uchar(1));
    QCOMPARE(cue2.value(1), uchar(2));
    QCOMPARE(cue2.value(2), uchar(3));
    QCOMPARE(cue2.fadeInSpeed(), uint(10));
    QCOMPARE(cue2.fadeOutSpeed(), uint(20));
    QCOMPARE(cue2.duration(), uint(30));
}

void Cue_Test::save()
{
    Cue cue("Foo");
    cue.setValue(0, 15);
    cue.setValue(31337, 255);
    cue.setValue(42, 127);
    cue.setFadeInSpeed(10);
    cue.setFadeOutSpeed(20);
    cue.setDuration(30);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);
    xmlWriter.writeStartElement("Bar");

    QVERIFY(cue.saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();
    QCOMPARE(xmlReader.name().toString(), QString("Bar"));
    xmlReader.readNextStartElement();
    QCOMPARE(xmlReader.name().toString(), QString("Cue"));
    QCOMPARE(xmlReader.attributes().value("Name").toString(), QString("Foo"));

    int value = 0, speed = 0;

    while (xmlReader.readNextStartElement())
    {
        if (xmlReader.name().toString() == "Value")
        {
            value++;
            QString ch = xmlReader.attributes().value("Channel").toString();
            QVERIFY(ch.isEmpty() == false);
            QString text = xmlReader.readElementText();
            if (ch.toUInt() == 0)
                QCOMPARE(text.toInt(), 15);
            else if (ch.toUInt() == 42)
                QCOMPARE(text.toInt(), 127);
            else if (ch.toUInt() == 31337)
                QCOMPARE(text.toInt(), 255);
            else
                QFAIL(QString("Unexpected channel in value tag: %1").arg(ch).toUtf8().constData());
        }
        else if (xmlReader.name().toString() == "Speed")
        {
            speed++;
            QCOMPARE(xmlReader.attributes().value("FadeIn").toString().toInt(), 10);
            QCOMPARE(xmlReader.attributes().value("FadeOut").toString().toInt(), 20);
            QCOMPARE(xmlReader.attributes().value("Duration").toString().toInt(), 30);
            xmlReader.skipCurrentElement();
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(xmlReader.name().toString()).toUtf8().constData());
            xmlReader.skipCurrentElement();
        }
    }

    QCOMPARE(value, 3);
    QCOMPARE(speed, 1);
}

void Cue_Test::load()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Cue");
    xmlWriter.writeAttribute("Name", "Baz");

    xmlWriter.writeStartElement("Value");
    xmlWriter.writeAttribute("Channel", "1");
    xmlWriter.writeCharacters("127");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Value");
    xmlWriter.writeAttribute("Channel", "42");
    xmlWriter.writeCharacters("255");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Value");
    xmlWriter.writeAttribute("Channel", "69");
    xmlWriter.writeCharacters("0");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Speed");
    xmlWriter.writeAttribute("FadeIn", "100");
    xmlWriter.writeAttribute("FadeOut", "200");
    xmlWriter.writeAttribute("Duration", "300");
    xmlWriter.writeEndElement();

    // Extra garbage
    xmlWriter.writeStartElement("Foo");
    xmlWriter.writeAttribute("Channel", "69");
    xmlWriter.writeCharacters("0");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);

    Cue cue;
    QVERIFY(cue.loadXML(xmlReader) == false);
    xmlReader.readNextStartElement();
    QVERIFY(cue.loadXML(xmlReader) == true);
    QCOMPARE(cue.name(), QString("Baz"));
    QCOMPARE(cue.values().size(), 3);
    QCOMPARE(cue.value(0), uchar(0));
    QCOMPARE(cue.value(1), uchar(127));
    QCOMPARE(cue.value(42), uchar(255));
    QCOMPARE(cue.value(69), uchar(0));
    QCOMPARE(cue.fadeInSpeed(), uint(100));
    QCOMPARE(cue.fadeOutSpeed(), uint(200));
    QCOMPARE(cue.duration(), uint(300));
}

QTEST_APPLESS_MAIN(Cue_Test)
