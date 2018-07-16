/*
  Q Light Controller Plus - Unit tests
  qlcinputchannel_test.cpp

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

#include "qlcinputchannel_test.h"
#include "qlcinputchannel.h"

void QLCInputChannel_Test::types()
{
    QStringList list(QLCInputChannel::types());
    QVERIFY(list.size() == 7);
    QVERIFY(list.contains(KXMLQLCInputChannelButton));
    QVERIFY(list.contains(KXMLQLCInputChannelSlider));
    QVERIFY(list.contains(KXMLQLCInputChannelKnob));
    QVERIFY(list.contains(KXMLQLCInputChannelEncoder));
    QVERIFY(list.contains(KXMLQLCInputChannelPageUp));
    QVERIFY(list.contains(KXMLQLCInputChannelPageDown));
    QVERIFY(list.contains(KXMLQLCInputChannelPageSet));
}

void QLCInputChannel_Test::type()
{
    QLCInputChannel ch;
    QVERIFY(ch.type() == QLCInputChannel::Button);

    ch.setType(QLCInputChannel::Slider);
    QVERIFY(ch.type() == QLCInputChannel::Slider);

    ch.setType(QLCInputChannel::Button);
    QVERIFY(ch.type() == QLCInputChannel::Button);

    ch.setType(QLCInputChannel::Knob);
    QVERIFY(ch.type() == QLCInputChannel::Knob);

    ch.setType(QLCInputChannel::Encoder);
    QVERIFY(ch.type() == QLCInputChannel::Encoder);

    ch.setType(QLCInputChannel::NextPage);
    QVERIFY(ch.type() == QLCInputChannel::NextPage);

    ch.setType(QLCInputChannel::PrevPage);
    QVERIFY(ch.type() == QLCInputChannel::PrevPage);

    ch.setType(QLCInputChannel::PageSet);
    QVERIFY(ch.type() == QLCInputChannel::PageSet);
}

void QLCInputChannel_Test::typeToString()
{
    QCOMPARE(QLCInputChannel::typeToString(QLCInputChannel::Button),
             QString(KXMLQLCInputChannelButton));
    QCOMPARE(QLCInputChannel::typeToString(QLCInputChannel::Knob),
             QString(KXMLQLCInputChannelKnob));
    QCOMPARE(QLCInputChannel::typeToString(QLCInputChannel::Encoder),
             QString(KXMLQLCInputChannelEncoder));
    QCOMPARE(QLCInputChannel::typeToString(QLCInputChannel::Slider),
             QString(KXMLQLCInputChannelSlider));
    QCOMPARE(QLCInputChannel::typeToString(QLCInputChannel::NextPage),
             QString(KXMLQLCInputChannelPageUp));
    QCOMPARE(QLCInputChannel::typeToString(QLCInputChannel::PrevPage),
             QString(KXMLQLCInputChannelPageDown));
    QCOMPARE(QLCInputChannel::typeToString(QLCInputChannel::PageSet),
             QString(KXMLQLCInputChannelPageSet));
    QCOMPARE(QLCInputChannel::typeToString(QLCInputChannel::Type(42)),
             QString(KXMLQLCInputChannelNone));
}

void QLCInputChannel_Test::stringToType()
{
    QCOMPARE(QLCInputChannel::stringToType(QString(KXMLQLCInputChannelButton)),
             QLCInputChannel::Button);
    QCOMPARE(QLCInputChannel::stringToType(QString(KXMLQLCInputChannelSlider)),
             QLCInputChannel::Slider);
    QCOMPARE(QLCInputChannel::stringToType(QString(KXMLQLCInputChannelKnob)),
             QLCInputChannel::Knob);
    QCOMPARE(QLCInputChannel::stringToType(QString(KXMLQLCInputChannelEncoder)),
             QLCInputChannel::Encoder);
    QCOMPARE(QLCInputChannel::stringToType(QString(KXMLQLCInputChannelPageUp)),
             QLCInputChannel::NextPage);
    QCOMPARE(QLCInputChannel::stringToType(QString(KXMLQLCInputChannelPageDown)),
             QLCInputChannel::PrevPage);
    QCOMPARE(QLCInputChannel::stringToType(QString(KXMLQLCInputChannelPageSet)),
             QLCInputChannel::PageSet);
    QCOMPARE(QLCInputChannel::stringToType(QString("foobar")),
             QLCInputChannel::NoType);
}

void QLCInputChannel_Test::name()
{
    QLCInputChannel ch;
    QVERIFY(ch.name().isEmpty());
    ch.setName("Foobar");
    QVERIFY(ch.name() == "Foobar");
}

void QLCInputChannel_Test::copy()
{
    QLCInputChannel ch;
    ch.setType(QLCInputChannel::Slider);
    ch.setName("Foobar");

    QLCInputChannel *copy = ch.createCopy();
    QVERIFY(copy->type() == QLCInputChannel::Slider);
    QVERIFY(copy->name() == "Foobar");

    QLCInputChannel *another = ch.createCopy();
    QVERIFY(another->type() == QLCInputChannel::Slider);
    QVERIFY(another->name() == "Foobar");
}

void QLCInputChannel_Test::load()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Channel");
    xmlWriter.writeTextElement("Name", "Foobar");
    xmlWriter.writeTextElement("Type", "Slider");
    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QLCInputChannel ch;
    ch.loadXML(xmlReader);
    QVERIFY(ch.name() == "Foobar");
    QVERIFY(ch.type() == QLCInputChannel::Slider);
}

void QLCInputChannel_Test::loadWrongType()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Channel");
    xmlWriter.writeTextElement("Name", "Foobar");
    xmlWriter.writeTextElement("Type", "Xyzzy");
    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QLCInputChannel ch;
    ch.loadXML(xmlReader);
    QVERIFY(ch.name() == "Foobar");
    QVERIFY(ch.type() == QLCInputChannel::NoType);
}

void QLCInputChannel_Test::save()
{
    QLCInputChannel ch;
    ch.setName("Foobar Name");
    ch.setType(QLCInputChannel::Knob);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(ch.saveXML(&xmlWriter, 12) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);

    xmlReader.readNextStartElement();

    QCOMPARE(xmlReader.name().toString(), QString(KXMLQLCInputChannel));

    while (xmlReader.readNextStartElement())
    {
        if (xmlReader.name() == KXMLQLCInputChannelName)
            QCOMPARE(xmlReader.readElementText(), QString("Foobar Name"));
        else if (xmlReader.name() == KXMLQLCInputChannelType)
            QCOMPARE(xmlReader.readElementText(), QString(KXMLQLCInputChannelKnob));
        else
            QFAIL("Unexpected crap in the XML!");
    }
}

QTEST_APPLESS_MAIN(QLCInputChannel_Test)
