/*
  Q Light Controller - Unit tests
  qlcfixturehead_test.cpp

  Copyright (C) Heikki Junnila

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
#include <QtXml>

#define protected public
#include "qlcfixturehead_test.h"
#include "qlcfixturehead.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#undef protected

void QLCFixtureHead_Test::initTestCase()
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

void QLCFixtureHead_Test::load()
{
    QDomDocument doc;
    QDomElement root = doc.createElement("Head");
    doc.appendChild(root);

    QDomElement ch = doc.createElement("Channel");
    QDomText text = doc.createTextNode("0");
    ch.appendChild(text);
    root.appendChild(ch);

    ch = doc.createElement("Channel");
    text = doc.createTextNode("1");
    ch.appendChild(text);
    root.appendChild(ch);

    ch = doc.createElement("Channel");
    text = doc.createTextNode("15");
    ch.appendChild(text);
    root.appendChild(ch);

    ch = doc.createElement("Foo");
    text = doc.createTextNode("25");
    ch.appendChild(text);
    root.appendChild(ch);

    ch = doc.createElement("Channel");
    text = doc.createTextNode("42");
    ch.appendChild(text);
    root.appendChild(ch);

    QLCFixtureHead head;
    QVERIFY(head.loadXML(ch) == false);
    QVERIFY(head.loadXML(root));
    QCOMPARE(head.channels().size(), 4);
    QVERIFY(head.channels().contains(0));
    QVERIFY(head.channels().contains(1));
    QVERIFY(head.channels().contains(15));
    QVERIFY(head.channels().contains(42));
}

void QLCFixtureHead_Test::save()
{
    QLCFixtureHead head;
    head.addChannel(0);
    head.addChannel(1);
    head.addChannel(2);
    head.addChannel(3);

    QDomDocument doc;
    QDomElement root = doc.createElement("Foo");
    doc.appendChild(root);

    QVERIFY(head.saveXML(&doc, &root));
    QCOMPARE(root.firstChild().toElement().tagName(), QString("Head"));
    int ch = 0;
    QDomNode node = root.firstChild().firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == "Channel")
        {
            QVERIFY(tag.text().toInt() == 0 || tag.text().toInt() == 1 ||
                    tag.text().toInt() == 2 || tag.text().toInt() == 3);
            ch++;
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(tag.tagName()).toUtf8().constData());
        }
        node = node.nextSibling();
    }

    QCOMPARE(ch, 4);
}

void QLCFixtureHead_Test::channels()
{
    QLCFixtureHead head;
    QCOMPARE(head.channels().size(), 0);

    head.addChannel(0);
    QCOMPARE(head.channels().size(), 1);
    QCOMPARE(head.channels().contains(0), true);

    head.addChannel(0);
    QCOMPARE(head.channels().size(), 1);
    QCOMPARE(head.channels().contains(0), true);

    head.addChannel(5000);
    QCOMPARE(head.channels().size(), 2);
    QCOMPARE(head.channels().contains(0), true);
    QCOMPARE(head.channels().contains(5000), true);

    head.removeChannel(1);
    QCOMPARE(head.channels().size(), 2);
    QCOMPARE(head.channels().contains(0), true);
    QCOMPARE(head.channels().contains(5000), true);

    head.removeChannel(0);
    QCOMPARE(head.channels().size(), 1);
    QCOMPARE(head.channels().contains(5000), true);

    head.removeChannel(5000);
    QCOMPARE(head.channels().size(), 0);
}

void QLCFixtureHead_Test::cacheChannelsRgbMaster()
{
    QLCFixtureMode mode(m_fixtureDef);
    QCOMPARE(mode.channels().size(), 0);

    m_ch1->setGroup(QLCChannel::Intensity);
    m_ch1->setColour(QLCChannel::Red);
    mode.insertChannel(m_ch1, 0);

    m_ch2->setGroup(QLCChannel::Intensity);
    m_ch2->setColour(QLCChannel::Green);
    mode.insertChannel(m_ch2, 1);

    m_ch3->setGroup(QLCChannel::Intensity);
    m_ch3->setColour(QLCChannel::Blue);
    mode.insertChannel(m_ch3, 2);

    m_ch4->setGroup(QLCChannel::Intensity);
    m_ch4->setColour(QLCChannel::NoColour);
    mode.insertChannel(m_ch4, 3);

    QLCFixtureHead head;
    head.addChannel(0);
    head.addChannel(1);
    head.addChannel(2);
    head.addChannel(3);
    head.cacheChannels(&mode);

    QCOMPARE(head.panMsbChannel(), QLCChannel::invalid());
    QCOMPARE(head.panLsbChannel(), QLCChannel::invalid());
    QCOMPARE(head.tiltMsbChannel(), QLCChannel::invalid());
    QCOMPARE(head.tiltLsbChannel(), QLCChannel::invalid());
    QCOMPARE(head.rgbChannels(), QVector <quint32> () << 0 << 1 << 2);
    QCOMPARE(head.cmyChannels(), QVector <quint32> ());
    QCOMPARE(head.masterIntensityChannel(), quint32(3));
}

void QLCFixtureHead_Test::cacheChannelsCmyMaster()
{
    QLCFixtureMode mode(m_fixtureDef);
    QCOMPARE(mode.channels().size(), 0);

    m_ch1->setGroup(QLCChannel::Intensity);
    m_ch1->setColour(QLCChannel::Cyan);
    mode.insertChannel(m_ch1, 0);

    m_ch2->setGroup(QLCChannel::Intensity);
    m_ch2->setColour(QLCChannel::Magenta);
    mode.insertChannel(m_ch2, 1);

    m_ch3->setGroup(QLCChannel::Intensity);
    m_ch3->setColour(QLCChannel::NoColour);
    mode.insertChannel(m_ch3, 2);

    m_ch4->setGroup(QLCChannel::Intensity);
    m_ch4->setColour(QLCChannel::Yellow);
    mode.insertChannel(m_ch4, 3);

    QLCFixtureHead head;
    head.addChannel(0);
    head.addChannel(1);
    head.addChannel(2);
    head.addChannel(3);
    head.cacheChannels(&mode);

    QCOMPARE(head.panMsbChannel(), QLCChannel::invalid());
    QCOMPARE(head.panLsbChannel(), QLCChannel::invalid());
    QCOMPARE(head.tiltMsbChannel(), QLCChannel::invalid());
    QCOMPARE(head.tiltLsbChannel(), QLCChannel::invalid());
    QCOMPARE(head.rgbChannels(), QVector <quint32> ());
    QCOMPARE(head.cmyChannels(), QVector <quint32> () << 0 << 1 << 3);
    QCOMPARE(head.masterIntensityChannel(), quint32(2));
}

void QLCFixtureHead_Test::cacheChannelsPanTilt()
{
    QLCFixtureMode mode(m_fixtureDef);
    QCOMPARE(mode.channels().size(), 0);

    m_ch1->setGroup(QLCChannel::Pan);
    m_ch1->setControlByte(QLCChannel::MSB);
    mode.insertChannel(m_ch1, 0);

    m_ch2->setGroup(QLCChannel::Pan);
    m_ch2->setControlByte(QLCChannel::LSB);
    mode.insertChannel(m_ch2, 1);

    m_ch3->setGroup(QLCChannel::Tilt);
    m_ch3->setControlByte(QLCChannel::MSB);
    mode.insertChannel(m_ch3, 2);

    m_ch4->setGroup(QLCChannel::Tilt);
    m_ch4->setControlByte(QLCChannel::LSB);
    mode.insertChannel(m_ch4, 3);

    QLCFixtureHead head;
    head.addChannel(0);
    head.addChannel(1);
    head.addChannel(2);
    head.addChannel(3);
    head.cacheChannels(&mode);

    QCOMPARE(head.panMsbChannel(), quint32(0));
    QCOMPARE(head.panLsbChannel(), quint32(1));
    QCOMPARE(head.tiltMsbChannel(), quint32(2));
    QCOMPARE(head.tiltLsbChannel(), quint32(3));
    QCOMPARE(head.rgbChannels(), QVector <quint32> ());
    QCOMPARE(head.cmyChannels(), QVector <quint32> ());
    QCOMPARE(head.masterIntensityChannel(), QLCChannel::invalid());

    head.cacheChannels((QLCFixtureMode*) 0xDEADBEEF);
    QCOMPARE(head.panMsbChannel(), quint32(0));
    QCOMPARE(head.panLsbChannel(), quint32(1));
    QCOMPARE(head.tiltMsbChannel(), quint32(2));
    QCOMPARE(head.tiltLsbChannel(), quint32(3));
    QCOMPARE(head.rgbChannels(), QVector <quint32> ());
    QCOMPARE(head.cmyChannels(), QVector <quint32> ());
    QCOMPARE(head.masterIntensityChannel(), QLCChannel::invalid());
}

void QLCFixtureHead_Test::doublePanTilt()
{
    // Test that the first found Pan/Tilt channel is used
    // - for the case the user forgets to set the latter to LSB
    // - in most cases the Pan MSB channel comes first
    QLCFixtureMode mode(m_fixtureDef);
    QCOMPARE(mode.channels().size(), 0);

    m_ch1->setGroup(QLCChannel::Pan);
    m_ch1->setControlByte(QLCChannel::MSB);
    mode.insertChannel(m_ch1, 0);

    m_ch2->setGroup(QLCChannel::Pan);
    m_ch2->setControlByte(QLCChannel::MSB);
    mode.insertChannel(m_ch2, 1);

    m_ch3->setGroup(QLCChannel::Tilt);
    m_ch3->setControlByte(QLCChannel::MSB);
    mode.insertChannel(m_ch3, 2);

    m_ch4->setGroup(QLCChannel::Tilt);
    m_ch4->setControlByte(QLCChannel::MSB);
    mode.insertChannel(m_ch4, 3);

    QLCFixtureHead head;
    head.addChannel(0);
    head.addChannel(1);
    head.addChannel(2);
    head.addChannel(3);
    head.cacheChannels(&mode);

    QCOMPARE(head.panMsbChannel(), quint32(0));
    QCOMPARE(head.panLsbChannel(), QLCChannel::invalid());
    QCOMPARE(head.tiltMsbChannel(), quint32(2));
    QCOMPARE(head.tiltLsbChannel(), QLCChannel::invalid());
}

void QLCFixtureHead_Test::cacheChannelsColor()
{
    QLCFixtureMode mode(m_fixtureDef);
    QCOMPARE(mode.channels().size(), 0);

    m_ch1->setGroup(QLCChannel::Pan);
    m_ch1->setControlByte(QLCChannel::MSB);
    mode.insertChannel(m_ch1, 0);

    m_ch2->setGroup(QLCChannel::Colour);
    m_ch2->setControlByte(QLCChannel::MSB);
    mode.insertChannel(m_ch2, 1);

    m_ch3->setGroup(QLCChannel::Tilt);
    m_ch3->setControlByte(QLCChannel::MSB);
    mode.insertChannel(m_ch3, 2);

    m_ch4->setGroup(QLCChannel::Colour);
    m_ch4->setControlByte(QLCChannel::MSB);
    mode.insertChannel(m_ch4, 3);

    QLCFixtureHead head;
    head.addChannel(0);
    head.addChannel(1);
    head.addChannel(2);
    head.addChannel(3);
    head.cacheChannels(&mode);

    QCOMPARE(quint32(head.colorWheels().count()), quint32(2));
    QCOMPARE(head.colorWheels().at(0), quint32(1));
    QCOMPARE(head.colorWheels().at(1), quint32(3));
}

void QLCFixtureHead_Test::dimmerHead()
{
    QLCDimmerHead dh(5);
    QCOMPARE(dh.masterIntensityChannel(), quint32(5));
    QCOMPARE(dh.m_channelsCached, true);
}

void QLCFixtureHead_Test::cleanupTestCase()
{
    QVERIFY(m_fixtureDef != NULL);
    delete m_fixtureDef;
}

QTEST_APPLESS_MAIN(QLCFixtureHead_Test)
