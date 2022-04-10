/*
  Q Light Controller Plus - Unit tests
  qlcfixturemode_test.cpp

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

#define protected public
#define private public
#include "qlcfixturemode_test.h"
#include "qlcfixturemode.h"
#include "qlcfixturehead.h"
#include "qlcfixturedef.h"
#undef private
#undef protected

void QLCFixtureMode_Test::initTestCase()
{
    m_fixtureDef = new QLCFixtureDef();
    QVERIFY(m_fixtureDef != NULL);

    m_ch1 = new QLCChannel();
    m_ch1->setName("Channel 1");
    m_fixtureDef->addChannel(m_ch1);

    m_ch2 = new QLCChannel();
    m_ch2->setName("Channel 2");
    m_fixtureDef->addChannel(m_ch2);

    m_ch3 = new QLCChannel();
    m_ch3->setName("Channel 3");
    m_fixtureDef->addChannel(m_ch3);

    m_ch4 = new QLCChannel();
    m_ch4->setName("Channel 4");
    m_fixtureDef->addChannel(m_ch4);
}

void QLCFixtureMode_Test::fixtureDef()
{
    QLCFixtureMode mode(m_fixtureDef);
    QCOMPARE(m_fixtureDef, mode.fixtureDef());
}

void QLCFixtureMode_Test::name()
{
    /* Verify that a name can be set & get for the mode */
    QLCFixtureMode* mode = new QLCFixtureMode(m_fixtureDef);

    QVERIFY(mode->name().isEmpty());
    mode->setName("Normal");
    QVERIFY(mode->name() == "Normal");

    delete mode;
}

void QLCFixtureMode_Test::physical()
{
    /* Verify that a QLCPhysical can be set & get for the mode */
    QLCFixtureMode* mode = new QLCFixtureMode(m_fixtureDef);
    QVERIFY(mode->physical().bulbType().isEmpty());

    QLCPhysical p;
    p.setBulbType("Foobar");
    mode->setPhysical(p);
    QVERIFY(mode->physical().bulbType() == "Foobar");

    delete mode;
}

void QLCFixtureMode_Test::insertChannel()
{
    QLCFixtureMode* mode = new QLCFixtureMode(m_fixtureDef);
    mode->setName("Test");

    QVERIFY(mode->insertChannel(NULL, 0) == false);
    QVERIFY(mode->channels().size() == 0);

    /* Channel that doesn't belong to mode->fixtureDef() */
    QLCChannel* ch = new QLCChannel();
    ch->setName("Rogue");
    mode->insertChannel(ch, 0);
    QVERIFY(mode->channels().size() == 0);
    delete ch;

    /* First channel */
    mode->insertChannel(m_ch1, 0);
    QVERIFY(mode->channel(0) == m_ch1);

    /* Second prepended */
    mode->insertChannel(m_ch2, 0);
    QVERIFY(mode->channel(0) == m_ch2);
    QVERIFY(mode->channel(1) == m_ch1);

    /* Third appended way over the end */
    mode->insertChannel(m_ch3, 10);
    QVERIFY(mode->channel(0) == m_ch2);
    QVERIFY(mode->channel(1) == m_ch1);
    QVERIFY(mode->channel(2) == m_ch3);

    /* Fourth inserted in-between */
    mode->insertChannel(m_ch4, 1);
    QVERIFY(mode->channel(0) == m_ch2);
    QVERIFY(mode->channel(1) == m_ch4);
    QVERIFY(mode->channel(2) == m_ch1);
    QVERIFY(mode->channel(3) == m_ch3);

    delete mode;
}

void QLCFixtureMode_Test::removeChannel()
{
    QLCFixtureMode* mode = new QLCFixtureMode(m_fixtureDef);

    mode->insertChannel(m_ch1, 0);
    mode->insertChannel(m_ch2, 1);
    mode->insertChannel(m_ch3, 2);
    mode->insertChannel(m_ch4, 3);

    /* Remove one channel in the middle */
    QVERIFY(mode->channels().size() == 4);
    mode->removeChannel(m_ch2);
    QVERIFY(mode->channels().size() == 3);
    QVERIFY(mode->channel(0) == m_ch1);
    QVERIFY(mode->channel(1) == m_ch3);
    QVERIFY(mode->channel(2) == m_ch4);
    QVERIFY(mode->channel(3) == NULL);

    /* Remove the same channel again. Shouldn't change anything. */
    mode->removeChannel(m_ch2);
    QVERIFY(mode->channels().size() == 3);
    QVERIFY(mode->channel(0) == m_ch1);
    QVERIFY(mode->channel(1) == m_ch3);
    QVERIFY(mode->channel(2) == m_ch4);
    QVERIFY(mode->channel(3) == NULL);

    /* Remove last channel. */
    mode->removeChannel(m_ch4);
    QVERIFY(mode->channels().size() == 2);
    QVERIFY(mode->channel(0) == m_ch1);
    QVERIFY(mode->channel(1) == m_ch3);
    QVERIFY(mode->channel(2) == NULL);
    QVERIFY(mode->channel(3) == NULL);

    /* Remove first channel. */
    mode->removeChannel(m_ch1);
    QVERIFY(mode->channels().size() == 1);
    QVERIFY(mode->channel(0) == m_ch3);
    QVERIFY(mode->channel(1) == NULL);
    QVERIFY(mode->channel(2) == NULL);
    QVERIFY(mode->channel(3) == NULL);

    /* Remove last channel. */
    mode->removeChannel(m_ch3);
    QVERIFY(mode->channels().size() == 0);
    QVERIFY(mode->channel(0) == NULL);
    QVERIFY(mode->channel(1) == NULL);
    QVERIFY(mode->channel(2) == NULL);
    QVERIFY(mode->channel(3) == NULL);

    delete mode;
}

void QLCFixtureMode_Test::channelByName()
{
    QLCFixtureMode* mode = new QLCFixtureMode(m_fixtureDef);

    mode->insertChannel(m_ch1, 0);
    mode->insertChannel(m_ch2, 1);
    mode->insertChannel(m_ch3, 2);
    mode->insertChannel(m_ch4, 3);

    QVERIFY(mode->channel("Channel 1") == m_ch1);
    QVERIFY(mode->channel("Channel 2") == m_ch2);
    QVERIFY(mode->channel("Channel 3") == m_ch3);
    QVERIFY(mode->channel("Channel 4") == m_ch4);
    QVERIFY(mode->channel("Foobar") == NULL);
    QVERIFY(mode->channel("") == NULL);

    delete mode;
}

void QLCFixtureMode_Test::channelByIndex()
{
    QLCFixtureMode* mode = new QLCFixtureMode(m_fixtureDef);

    mode->insertChannel(m_ch1, 0);
    mode->insertChannel(m_ch2, 1);
    mode->insertChannel(m_ch3, 2);
    mode->insertChannel(m_ch4, 3);

    QVERIFY(mode->channel(0) == m_ch1);
    QVERIFY(mode->channel(1) == m_ch2);
    QVERIFY(mode->channel(2) == m_ch3);
    QVERIFY(mode->channel(3) == m_ch4);
    QVERIFY(mode->channel(12) == NULL);

    delete mode;
}

void QLCFixtureMode_Test::channels()
{
    QLCFixtureMode* mode = new QLCFixtureMode(m_fixtureDef);
    QVERIFY(mode->channels().size() == 0);

    mode->insertChannel(m_ch1, 0);
    QVERIFY(mode->channels().size() == 1);

    mode->insertChannel(m_ch2, 1);
    QVERIFY(mode->channels().size() == 2);

    mode->insertChannel(m_ch3, 2);
    QVERIFY(mode->channels().size() == 3);

    delete mode;
}

void QLCFixtureMode_Test::channelNumber()
{
    QLCFixtureMode* mode = new QLCFixtureMode(m_fixtureDef);

    mode->insertChannel(m_ch1, 0);
    mode->insertChannel(m_ch2, 1);
    mode->insertChannel(m_ch3, 2);
    mode->insertChannel(m_ch4, 3);

    QVERIFY(mode->channelNumber(m_ch1) == 0);
    QVERIFY(mode->channelNumber(m_ch2) == 1);
    QVERIFY(mode->channelNumber(m_ch3) == 2);
    QVERIFY(mode->channelNumber(m_ch4) == 3);

    QLCChannel* ch = new QLCChannel();
    QVERIFY(mode->channelNumber(ch) == QLCChannel::invalid());
    QVERIFY(mode->channelNumber(NULL) == QLCChannel::invalid());
    delete ch;

    delete mode;
}

void QLCFixtureMode_Test::heads()
{
    QLCFixtureMode* mode = new QLCFixtureMode(m_fixtureDef);

    QLCFixtureHead head;
    head.addChannel(0);
    head.addChannel(10);
    head.addChannel(20);
    mode->insertHead(-1, head);
    QCOMPARE(mode->heads().size(), 1);
    QVERIFY(mode->m_heads[0].m_channels.contains(quint32(0)) == true);
    QVERIFY(mode->m_heads[0].m_channels.contains(quint32(10)) == true);
    QVERIFY(mode->m_heads[0].m_channels.contains(quint32(20)) == true);

    QLCFixtureHead head2;
    head2.addChannel(1);
    head2.addChannel(2);
    mode->insertHead(10, head2);
    QCOMPARE(mode->heads().size(), 2);
    QVERIFY(mode->m_heads[0].m_channels.contains(quint32(0)) == true);
    QVERIFY(mode->m_heads[0].m_channels.contains(quint32(10)) == true);
    QVERIFY(mode->m_heads[0].m_channels.contains(quint32(20)) == true);
    QVERIFY(mode->m_heads[1].m_channels.contains(quint32(0)) == false);
    QVERIFY(mode->m_heads[1].m_channels.contains(quint32(1)) == true);
    QVERIFY(mode->m_heads[1].m_channels.contains(quint32(2)) == true);

    QLCFixtureHead head3;
    head3.addChannel(3);
    head3.addChannel(4);
    mode->insertHead(1, head3);
    QCOMPARE(mode->heads().size(), 3);
    QVERIFY(mode->m_heads[0].m_channels.contains(quint32(0)) == true);
    QVERIFY(mode->m_heads[0].m_channels.contains(quint32(10)) == true);
    QVERIFY(mode->m_heads[0].m_channels.contains(quint32(20)) == true);
    QVERIFY(mode->m_heads[1].m_channels.contains(quint32(3)) == true);
    QVERIFY(mode->m_heads[1].m_channels.contains(quint32(4)) == true);
    QVERIFY(mode->m_heads[2].m_channels.contains(quint32(1)) == true);
    QVERIFY(mode->m_heads[2].m_channels.contains(quint32(2)) == true);

    QLCFixtureHead head4;
    head4.addChannel(15);
    head4.addChannel(16);
    mode->replaceHead(4, head4);
    QCOMPARE(mode->heads().size(), 3);
    QVERIFY(mode->m_heads[0].m_channels.contains(quint32(0)) == true);
    QVERIFY(mode->m_heads[0].m_channels.contains(quint32(10)) == true);
    QVERIFY(mode->m_heads[0].m_channels.contains(quint32(20)) == true);
    QVERIFY(mode->m_heads[1].m_channels.contains(quint32(3)) == true);
    QVERIFY(mode->m_heads[1].m_channels.contains(quint32(4)) == true);
    QVERIFY(mode->m_heads[2].m_channels.contains(quint32(1)) == true);
    QVERIFY(mode->m_heads[2].m_channels.contains(quint32(2)) == true);

    mode->replaceHead(2, head4);
    QCOMPARE(mode->heads().size(), 3);
    QVERIFY(mode->m_heads[0].m_channels.contains(quint32(0)) == true);
    QVERIFY(mode->m_heads[0].m_channels.contains(quint32(10)) == true);
    QVERIFY(mode->m_heads[0].m_channels.contains(quint32(20)) == true);
    QVERIFY(mode->m_heads[1].m_channels.contains(quint32(3)) == true);
    QVERIFY(mode->m_heads[1].m_channels.contains(quint32(4)) == true);
    QVERIFY(mode->m_heads[2].m_channels.contains(quint32(15)) == true);
    QVERIFY(mode->m_heads[2].m_channels.contains(quint32(16)) == true);

    mode->removeHead(15);
    QCOMPARE(mode->heads().size(), 3);

    mode->removeHead(1);
    QCOMPARE(mode->heads().size(), 2);
    QVERIFY(mode->m_heads[0].m_channels.contains(quint32(0)) == true);
    QVERIFY(mode->m_heads[0].m_channels.contains(quint32(10)) == true);
    QVERIFY(mode->m_heads[0].m_channels.contains(quint32(20)) == true);
    QVERIFY(mode->m_heads[1].m_channels.contains(quint32(15)) == true);
    QVERIFY(mode->m_heads[1].m_channels.contains(quint32(16)) == true);

    mode->removeHead(0);
    QCOMPARE(mode->heads().size(), 1);
    QVERIFY(mode->m_heads[0].m_channels.contains(quint32(15)) == true);
    QVERIFY(mode->m_heads[0].m_channels.contains(quint32(16)) == true);

    mode->removeHead(0);
    QCOMPARE(mode->heads().size(), 0);

    delete mode;
}

void QLCFixtureMode_Test::copy()
{
    QLCFixtureMode* mode = new QLCFixtureMode(m_fixtureDef);
    mode->setName("Test Mode");

    mode->insertChannel(m_ch1, 0);
    mode->insertChannel(m_ch2, 1);
    mode->insertChannel(m_ch3, 2);
    mode->insertChannel(m_ch4, 3);

    QLCFixtureHead head;
    head.addChannel(0);
    head.addChannel(1);
    head.addChannel(2);
    mode->insertHead(-1, head);

    /* Create a copy of the mode to the same fixtureDef as the original */
    QLCFixtureMode* copy = new QLCFixtureMode(m_fixtureDef, mode);
    QVERIFY(copy != NULL);

    QVERIFY(copy->name() == "Test Mode");
    QVERIFY(copy->channels().size() == 4);
    QVERIFY(copy->channel(0) == m_ch1);
    QVERIFY(copy->channel(1) == m_ch2);
    QVERIFY(copy->channel(2) == m_ch3);
    QVERIFY(copy->channel(3) == m_ch4);
    QCOMPARE(copy->heads().size(), 1);
    QVERIFY(copy->heads().at(0).channels().contains(quint32(0)) == true);
    QVERIFY(copy->heads().at(0).channels().contains(quint32(1)) == true);
    QVERIFY(copy->heads().at(0).channels().contains(quint32(2)) == true);
    delete copy;
    copy = NULL;

    /* Create another fixture def with some channels matching, some not */
    QLCFixtureDef* anotherDef = new QLCFixtureDef();
    QLCChannel* ch1 = new QLCChannel();
    ch1->setName("Channel 1"); // Should match
    anotherDef->addChannel(ch1);

    QLCChannel* ch2 = new QLCChannel();
    ch2->setName("Channel 2, not the same name"); // Shouldn't match
    anotherDef->addChannel(ch2);

    QLCChannel* ch3 = new QLCChannel();
    ch3->setName("Channel 3, still not the same name"); // Shouldn't match
    anotherDef->addChannel(ch3);

    QLCChannel* ch4 = new QLCChannel();
    ch4->setName("Channel 4"); // Should match
    anotherDef->addChannel(ch4);

    QLCChannel* ch5 = new QLCChannel();
    ch5->setName("Channel 5"); // Shouldn't match since original has 4 chans
    anotherDef->addChannel(ch5);

    /* Create a copy of the mode to the other fixtureDef */
    copy = new QLCFixtureMode(anotherDef, mode);

    QVERIFY(copy->name() == "Test Mode");
    QVERIFY(copy->channels().size() == 2);

    QVERIFY(copy->channel(0)->name() == "Channel 1");
    QVERIFY(copy->channel(0) == ch1);

    QVERIFY(copy->channel(1)->name() == "Channel 4");
    QVERIFY(copy->channel(1) == ch4);

    delete copy;
    delete anotherDef;
}

void QLCFixtureMode_Test::intensityChannels()
{
    QLCFixtureDef def;

    QLCChannel * masterCh = new QLCChannel();
    masterCh->setGroup(QLCChannel::Intensity);
    masterCh->setControlByte(QLCChannel::MSB);
    masterCh->setColour(QLCChannel::NoColour);
    def.addChannel(masterCh);

    QLCChannel * h1Ch = new QLCChannel();
    h1Ch->setGroup(QLCChannel::Intensity);
    h1Ch->setControlByte(QLCChannel::MSB);
    h1Ch->setColour(QLCChannel::NoColour);
    def.addChannel(h1Ch);

    QLCChannel * h2Ch = new QLCChannel();
    h2Ch->setGroup(QLCChannel::Intensity);
    h2Ch->setControlByte(QLCChannel::MSB);
    h2Ch->setColour(QLCChannel::NoColour);
    def.addChannel(h2Ch);

    QLCFixtureMode mode(&def);
    mode.insertChannel(masterCh, 0);
    mode.insertChannel(h1Ch, 1);
    mode.insertChannel(h2Ch, 2);

    QLCFixtureHead h1;
    h1.addChannel(1);
    mode.insertHead(0, h1);

    QLCFixtureHead h2;
    h2.addChannel(2);
    mode.insertHead(1, h2);

    mode.cacheHeads();

    QCOMPARE(mode.masterIntensityChannel(), 0U);
    QCOMPARE(mode.heads()[0].channelNumber(QLCChannel::Intensity, QLCChannel::MSB), 1U);
    QCOMPARE(mode.heads()[1].channelNumber(QLCChannel::Intensity, QLCChannel::MSB), 2U);
}

void QLCFixtureMode_Test::load()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Mode");
    xmlWriter.writeAttribute("Name", "Mode1");

    xmlWriter.writeStartElement("Channel");
    xmlWriter.writeAttribute("Number", "0");
    xmlWriter.writeCharacters("Channel 1");
    xmlWriter.writeEndElement();

    /* Shouldn't appear in the mode since Channel 1 is already added */
    xmlWriter.writeStartElement("Channel");
    xmlWriter.writeAttribute("Number", "1");
    xmlWriter.writeCharacters("Channel 1");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Channel");
    xmlWriter.writeAttribute("Number", "1");
    xmlWriter.writeCharacters("Channel 3");
    xmlWriter.writeEndElement();

    /* Head1 */
    xmlWriter.writeEmptyElement("Head");

    /* Head2 */
    xmlWriter.writeEmptyElement("Head");

    /* Physical */
    xmlWriter.writeStartElement("Physical");

    /* Bulb */
    xmlWriter.writeStartElement("Bulb");
    xmlWriter.writeAttribute("Type", "LED");
    xmlWriter.writeAttribute("Lumens", "18000");
    xmlWriter.writeAttribute("ColourTemperature", "6500");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndElement();

    /* Unrecognized tag on top level */
    xmlWriter.writeEmptyElement("Foo");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    QLCFixtureMode mode(m_fixtureDef);
    QVERIFY(mode.physical().bulbType() != "LED");
    QVERIFY(mode.physical().bulbLumens() != 18000);
    QVERIFY(mode.physical().bulbColourTemperature() != 6500);

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(mode.loadXML(xmlReader) == true);
    QVERIFY(mode.physical().bulbType() == "LED");
    QVERIFY(mode.physical().bulbLumens() == 18000);
    QVERIFY(mode.physical().bulbColourTemperature() == 6500);

    QVERIFY(mode.channels().size() == 2);
    QVERIFY(mode.channels()[0] == m_ch1);
    QVERIFY(mode.channels()[1] == m_ch3);

    QCOMPARE(mode.heads().size(), 2);
}

void QLCFixtureMode_Test::loadWrongRoot()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("ode");
    xmlWriter.writeAttribute("Name", "Mode1");

    xmlWriter.writeStartElement("Channel");
    xmlWriter.writeAttribute("Number", "0");
    xmlWriter.writeCharacters("Channel 1");
    xmlWriter.writeEndElement();

    /* Shouldn't appear in the mode since Channel 1 is already added */
    xmlWriter.writeStartElement("Channel");
    xmlWriter.writeAttribute("Number", "1");
    xmlWriter.writeCharacters("Channel 1");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Channel");
    xmlWriter.writeAttribute("Number", "1");
    xmlWriter.writeCharacters("Channel 3");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QLCFixtureMode mode(m_fixtureDef);
    QVERIFY(mode.loadXML(xmlReader) == false);
    QVERIFY(mode.channels().size() == 0);
}

void QLCFixtureMode_Test::loadNoName()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Mode");

    xmlWriter.writeStartElement("Channel");
    xmlWriter.writeAttribute("Number", "0");
    xmlWriter.writeCharacters("Channel 1");
    xmlWriter.writeEndElement();

    /* Shouldn't appear in the mode since Channel 1 is already added */
    xmlWriter.writeStartElement("Channel");
    xmlWriter.writeAttribute("Number", "1");
    xmlWriter.writeCharacters("Channel 1");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Channel");
    xmlWriter.writeAttribute("Number", "1");
    xmlWriter.writeCharacters("Channel 3");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QLCFixtureMode mode(m_fixtureDef);
    QVERIFY(mode.loadXML(xmlReader) == false);
    QVERIFY(mode.channels().size() == 0);
}

void QLCFixtureMode_Test::save()
{
    QVERIFY(m_fixtureDef != NULL);
    QCOMPARE(m_fixtureDef->channels().size(), 4);

    QString name("Foobar");
    QLCFixtureMode mode(m_fixtureDef);
    mode.setName(name);
    QVERIFY(mode.insertChannel(m_ch1, 0) == true);
    QVERIFY(mode.insertChannel(m_ch4, 1) == true);
    QVERIFY(mode.insertChannel(m_ch2, 2) == true);
    QVERIFY(mode.insertChannel(m_ch3, 3) == true);

    QLCFixtureHead head;
    mode.insertHead(-1, head);
    mode.insertHead(-1, head);
    mode.insertHead(-1, head);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    bool physical = false;
    int heads = 0;
    QMap <int,QString> channels;

    QVERIFY(mode.saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);

    xmlReader.readNextStartElement();

    QCOMPARE(xmlReader.name().toString(), QString(KXMLQLCFixtureMode));
    QCOMPARE(xmlReader.attributes().value(KXMLQLCFixtureModeName).toString(), name);

    while (xmlReader.readNextStartElement())
    {
        if (xmlReader.name() == KXMLQLCChannel)
        {
            int num = xmlReader.attributes().value(KXMLQLCFixtureModeChannelNumber).toString().toInt();
            channels[num] = xmlReader.readElementText();
        }
        else if (xmlReader.name() == KXMLQLCFixtureHead)
        {
            heads++;
            xmlReader.skipCurrentElement();
        }
    }

    QVERIFY(physical == false);
    QCOMPARE(channels.size(), 4);
    QCOMPARE(channels[0], m_ch1->name());
    QCOMPARE(channels[1], m_ch4->name());
    QCOMPARE(channels[2], m_ch2->name());
    QCOMPARE(channels[3], m_ch3->name());
    QCOMPARE(heads, 3);
}

void QLCFixtureMode_Test::savePhysicalOverride()
{
    QVERIFY(m_fixtureDef != NULL);
    QCOMPARE(m_fixtureDef->channels().size(), 4);

    QString name("Foobar");
    QLCFixtureMode mode(m_fixtureDef);
    mode.setName(name);

    QVERIFY(mode.useGlobalPhysical() == true);

    QLCPhysical phy;
    phy.setWeight(10);
    phy.setWidth(11);
    phy.setHeight(12);
    phy.setDepth(13);

    mode.setPhysical(phy);

    QVERIFY(mode.useGlobalPhysical() == false);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(mode.saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);

    xmlReader.readNextStartElement();

    QCOMPARE(xmlReader.name().toString(), QString(KXMLQLCFixtureMode));

    bool physical = false;

    while (xmlReader.readNextStartElement())
    {
        if (xmlReader.name() == KXMLQLCPhysical)
        {
            // Only check that physical node is there. Its contents are
            // tested by another test case (QLCPhysical_Test)
            physical = true;
            xmlReader.skipCurrentElement();
        }
    }

    QVERIFY(physical == true);
}

void QLCFixtureMode_Test::cleanupTestCase()
{
    QVERIFY(m_fixtureDef != NULL);
    delete m_fixtureDef;
}

QTEST_APPLESS_MAIN(QLCFixtureMode_Test)
