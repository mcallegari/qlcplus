/*
  Q Light Controller Plus - Unit tests
  qlcchannel_test.cpp

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

#include "qlcchannel_test.h"
#include "qlccapability.h"
#include "qlcchannel.h"

void QLCChannel_Test::groupList()
{
    QStringList list(QLCChannel::groupList());

    QVERIFY(list.size() == 12);
    QVERIFY(list.contains(QLCChannel::groupToString(QLCChannel::Beam)));
    QVERIFY(list.contains(QLCChannel::groupToString(QLCChannel::Colour)));
    QVERIFY(list.contains(QLCChannel::groupToString(QLCChannel::Effect)));
    QVERIFY(list.contains(QLCChannel::groupToString(QLCChannel::Gobo)));
    QVERIFY(list.contains(QLCChannel::groupToString(QLCChannel::Intensity)));
    QVERIFY(list.contains(QLCChannel::groupToString(QLCChannel::Maintenance)));
    QVERIFY(list.contains(QLCChannel::groupToString(QLCChannel::NoGroup)));
    QVERIFY(list.contains(QLCChannel::groupToString(QLCChannel::Pan)));
    QVERIFY(list.contains(QLCChannel::groupToString(QLCChannel::Prism)));
    QVERIFY(list.contains(QLCChannel::groupToString(QLCChannel::Shutter)));
    QVERIFY(list.contains(QLCChannel::groupToString(QLCChannel::Speed)));
    QVERIFY(list.contains(QLCChannel::groupToString(QLCChannel::Tilt)));
}

void QLCChannel_Test::name()
{
    /* Verify that a name can be set & get for the channel */
    QLCChannel* channel = new QLCChannel();
    QVERIFY(channel->name().isEmpty());

    channel->setName("Channel");
    QVERIFY(channel->name() == "Channel");

    delete channel;
}

void QLCChannel_Test::group()
{
    QLCChannel* channel = new QLCChannel();
    QVERIFY(channel->group() == QLCChannel::Intensity);

    channel->setGroup(QLCChannel::Beam);
    QVERIFY(channel->group() == QLCChannel::Beam);

    channel->setGroup(QLCChannel::Group(31337));
    QVERIFY(channel->group() == QLCChannel::Group(31337));

    delete channel;
}

void QLCChannel_Test::defaultValue()
{
    QLCChannel* channel = new QLCChannel();
    QVERIFY(channel->defaultValue() == 0);

    channel->setDefaultValue(137);
    QVERIFY(channel->defaultValue() == 137);
}

void QLCChannel_Test::controlByte()
{
    QCOMPARE(int(QLCChannel::MSB), 0);
    QCOMPARE(int(QLCChannel::LSB), 1);

    QLCChannel* channel = new QLCChannel();
    QVERIFY(channel->controlByte() == QLCChannel::MSB);

    channel->setControlByte(QLCChannel::LSB);
    QVERIFY(channel->controlByte() == QLCChannel::LSB);

    delete channel;
}

void QLCChannel_Test::colourList()
{
    QStringList list(QLCChannel::colourList());

    QCOMPARE(list.size(), 11);
    //QVERIFY(list.contains(QLCChannel::colourToString(QLCChannel::NoColour)));
    QVERIFY(list.contains(QLCChannel::colourToString(QLCChannel::Red)));
    QVERIFY(list.contains(QLCChannel::colourToString(QLCChannel::Green)));
    QVERIFY(list.contains(QLCChannel::colourToString(QLCChannel::Blue)));
    QVERIFY(list.contains(QLCChannel::colourToString(QLCChannel::Cyan)));
    QVERIFY(list.contains(QLCChannel::colourToString(QLCChannel::Magenta)));
    QVERIFY(list.contains(QLCChannel::colourToString(QLCChannel::Yellow)));
    QVERIFY(list.contains(QLCChannel::colourToString(QLCChannel::Amber)));
    QVERIFY(list.contains(QLCChannel::colourToString(QLCChannel::White)));
    QVERIFY(list.contains(QLCChannel::colourToString(QLCChannel::UV)));
    QVERIFY(list.contains(QLCChannel::colourToString(QLCChannel::Lime)));
    QVERIFY(list.contains(QLCChannel::colourToString(QLCChannel::Indigo)));
}

void QLCChannel_Test::colour()
{
    QCOMPARE(int(QLCChannel::NoColour), 0);
    QCOMPARE(int(QLCChannel::Red), 0xFF0000);
    QCOMPARE(int(QLCChannel::Green), 0x00FF00);
    QCOMPARE(int(QLCChannel::Blue), 0x0000FF);
    QCOMPARE(int(QLCChannel::Cyan), 0x00FFFF);
    QCOMPARE(int(QLCChannel::Magenta), 0xFF00FF);
    QCOMPARE(int(QLCChannel::Yellow), 0xFFFF00);
    QCOMPARE(int(QLCChannel::Amber), 0xFF7E00);
    QCOMPARE(int(QLCChannel::White), 0xFFFFFF);
    QCOMPARE(int(QLCChannel::UV), 0x9400D3);
    QCOMPARE(int(QLCChannel::Lime), 0xADFF2F);
    QCOMPARE(int(QLCChannel::Indigo), 0x4B0082);

    QLCChannel* channel = new QLCChannel();
    QCOMPARE(channel->colour(), QLCChannel::NoColour);

    channel->setColour(QLCChannel::Red);
    QCOMPARE(channel->colour(), QLCChannel::Red);

    channel->setColour(QLCChannel::Green);
    QCOMPARE(channel->colour(), QLCChannel::Green);

    channel->setColour(QLCChannel::Blue);
    QCOMPARE(channel->colour(), QLCChannel::Blue);

    channel->setColour(QLCChannel::Cyan);
    QCOMPARE(channel->colour(), QLCChannel::Cyan);

    channel->setColour(QLCChannel::Magenta);
    QCOMPARE(channel->colour(), QLCChannel::Magenta);

    channel->setColour(QLCChannel::Yellow);
    QCOMPARE(channel->colour(), QLCChannel::Yellow);

    channel->setColour(QLCChannel::Amber);
    QCOMPARE(channel->colour(), QLCChannel::Amber);

    channel->setColour(QLCChannel::White);
    QCOMPARE(channel->colour(), QLCChannel::White);

    channel->setColour(QLCChannel::UV);
    QCOMPARE(channel->colour(), QLCChannel::UV);

    channel->setColour(QLCChannel::Lime);
    QCOMPARE(channel->colour(), QLCChannel::Lime);

    channel->setColour(QLCChannel::Indigo);
    QCOMPARE(channel->colour(), QLCChannel::Indigo);

    channel->setColour(QLCChannel::NoColour);
    QCOMPARE(channel->colour(), QLCChannel::NoColour);
}

void QLCChannel_Test::searchCapabilityByValue()
{
    QLCChannel* channel = new QLCChannel();
    QVERIFY(channel->capabilities().size() == 0);

    QLCCapability* cap1 = new QLCCapability(0, 9, "0-9");
    QVERIFY(channel->addCapability(cap1) == true);
    QVERIFY(channel->capabilities().size() == 1);

    QLCCapability* cap2 = new QLCCapability(10, 19, "10-19");
    QVERIFY(channel->addCapability(cap2) == true);
    QVERIFY(channel->capabilities().size() == 2);

    QLCCapability* cap3 = new QLCCapability(20, 29, "20-29");
    QVERIFY(channel->addCapability(cap3) == true);
    QVERIFY(channel->capabilities().size() == 3);

    QVERIFY(channel->searchCapability(0) == cap1);
    QVERIFY(channel->searchCapability(1) == cap1);
    QVERIFY(channel->searchCapability(2) == cap1);
    QVERIFY(channel->searchCapability(3) == cap1);
    QVERIFY(channel->searchCapability(4) == cap1);
    QVERIFY(channel->searchCapability(5) == cap1);
    QVERIFY(channel->searchCapability(6) == cap1);
    QVERIFY(channel->searchCapability(7) == cap1);
    QVERIFY(channel->searchCapability(8) == cap1);
    QVERIFY(channel->searchCapability(9) == cap1);

    QVERIFY(channel->searchCapability(10) == cap2);
    QVERIFY(channel->searchCapability(11) == cap2);
    QVERIFY(channel->searchCapability(12) == cap2);
    QVERIFY(channel->searchCapability(13) == cap2);
    QVERIFY(channel->searchCapability(14) == cap2);
    QVERIFY(channel->searchCapability(15) == cap2);
    QVERIFY(channel->searchCapability(16) == cap2);
    QVERIFY(channel->searchCapability(17) == cap2);
    QVERIFY(channel->searchCapability(18) == cap2);
    QVERIFY(channel->searchCapability(19) == cap2);

    QVERIFY(channel->searchCapability(30) == NULL);

    delete channel;
}

void QLCChannel_Test::searchCapabilityByName()
{
    QLCChannel* channel = new QLCChannel();
    QVERIFY(channel->capabilities().size() == 0);

    QLCCapability* cap1 = new QLCCapability(0, 9, "0-9");
    QVERIFY(channel->addCapability(cap1) == true);

    QLCCapability* cap2 = new QLCCapability(10, 19, "10-19");
    QVERIFY(channel->addCapability(cap2) == true);

    QLCCapability* cap3 = new QLCCapability(20, 29, "20-29");
    QVERIFY(channel->addCapability(cap3) == true);

    QVERIFY(channel->searchCapability("0-9") == cap1);
    QVERIFY(channel->searchCapability("10-19") == cap2);
    QVERIFY(channel->searchCapability("20-29") == cap3);
    QVERIFY(channel->searchCapability("foo") == NULL);

    delete channel;
}

void QLCChannel_Test::addCapability()
{
    QLCChannel* channel = new QLCChannel();
    QVERIFY(channel->capabilities().size() == 0);

    QLCCapability* cap1 = new QLCCapability(15, 19, "15-19");
    QVERIFY(channel->addCapability(cap1) == true);
    QVERIFY(channel->capabilities().size() == 1);
    QVERIFY(channel->capabilities()[0] == cap1);

    QLCCapability* cap2 = new QLCCapability(0, 9, "0-9");
    QVERIFY(channel->addCapability(cap2) == true);
    QVERIFY(channel->capabilities().size() == 2);
    QVERIFY(channel->capabilities()[0] == cap1);
    QVERIFY(channel->capabilities()[1] == cap2);

    /* Completely overlapping with cap2 */
    QLCCapability* cap3 = new QLCCapability(5, 6, "5-6");
    QVERIFY(channel->addCapability(cap3) == false);
    delete cap3;
    cap3 = NULL;

    /* Partially overlapping from low-end with cap1 */
    QLCCapability* cap4 = new QLCCapability(19, 25, "19-25");
    QVERIFY(channel->addCapability(cap4) == false);
    delete cap4;
    cap4 = NULL;

    /* Partially overlapping from high end with cap1 */
    QLCCapability* cap5 = new QLCCapability(10, 15, "10-15");
    QVERIFY(channel->addCapability(cap5) == false);
    delete cap5;
    cap5 = NULL;

    /* Partially overlapping with two ranges at both ends (cap1 & cap2) */
    QLCCapability* cap6 = new QLCCapability(8, 16, "8-16");
    QVERIFY(channel->addCapability(cap6) == false);
    delete cap6;
    cap6 = NULL;

    /* Completely containing cap1 */
    QLCCapability* cap7 = new QLCCapability(14, 20, "14-20");
    QVERIFY(channel->addCapability(cap7) == false);
    delete cap7;
    cap7 = NULL;

    /* Non-overlapping, between cap1 & cap2*/
    QLCCapability* cap8 = new QLCCapability(10, 14, "10-14");
    QVERIFY(channel->addCapability(cap8) == true);
    /* Don't delete cap8 because it's now a member of the channel and gets
       deleted from the channel's destructor. */

    delete channel;
}

void QLCChannel_Test::removeCapability()
{
    QLCChannel* channel = new QLCChannel();
    QVERIFY(channel->capabilities().size() == 0);

    QLCCapability* cap1 = new QLCCapability(10, 20, "10-20");
    QVERIFY(channel->addCapability(cap1) == true);
    QVERIFY(channel->capabilities().size() == 1);

    QLCCapability* cap2 = new QLCCapability(0, 9, "0-9");
    QVERIFY(channel->addCapability(cap2) == true);
    QVERIFY(channel->capabilities().size() == 2);

    QVERIFY(channel->removeCapability(cap2) == true);
    QVERIFY(channel->capabilities().size() == 1);
    /* cap2 is deleted by QLCChannel::removeCapability() */

    QVERIFY(channel->removeCapability(cap2) == false);
    QVERIFY(channel->capabilities().size() == 1);

    QVERIFY(channel->removeCapability(cap1) == true);
    QVERIFY(channel->capabilities().size() == 0);
    /* cap1 is deleted by QLCChannel::removeCapability() */

    delete channel;
}

void QLCChannel_Test::sortCapabilities()
{
    QLCChannel* channel = new QLCChannel();
    QVERIFY(channel->capabilities().size() == 0);

    QLCCapability* cap1 = new QLCCapability(10, 19, "10-19");
    QVERIFY(channel->addCapability(cap1) == true);

    QLCCapability* cap2 = new QLCCapability(50, 59, "50-59");
    QVERIFY(channel->addCapability(cap2) == true);

    QLCCapability* cap3 = new QLCCapability(40, 49, "40-49");
    QVERIFY(channel->addCapability(cap3) == true);

    QLCCapability* cap4 = new QLCCapability(0, 9, "0-9");
    QVERIFY(channel->addCapability(cap4) == true);

    QLCCapability* cap5 = new QLCCapability(200, 209, "200-209");
    QVERIFY(channel->addCapability(cap5) == true);

    QLCCapability* cap6 = new QLCCapability(30, 39, "30-39");
    QVERIFY(channel->addCapability(cap6) == true);

    QLCCapability* cap7 = new QLCCapability(26, 29, "26-29");
    QVERIFY(channel->addCapability(cap7) == true);

    QLCCapability* cap8 = new QLCCapability(20, 25, "20-25");
    QVERIFY(channel->addCapability(cap8) == true);

    QList <QLCCapability*> orig(channel->capabilities());
    QVERIFY(orig.at(0) == cap1);
    QVERIFY(orig.at(1) == cap2);
    QVERIFY(orig.at(2) == cap3);
    QVERIFY(orig.at(3) == cap4);
    QVERIFY(orig.at(4) == cap5);
    QVERIFY(orig.at(5) == cap6);
    QVERIFY(orig.at(6) == cap7);
    QVERIFY(orig.at(7) == cap8);

    channel->sortCapabilities();

    QList <QLCCapability*> sorted(channel->capabilities());
    QVERIFY(sorted.at(0) == cap4);
    QVERIFY(sorted.at(1) == cap1);
    QVERIFY(sorted.at(2) == cap8);
    QVERIFY(sorted.at(3) == cap7);
    QVERIFY(sorted.at(4) == cap6);
    QVERIFY(sorted.at(5) == cap3);
    QVERIFY(sorted.at(6) == cap2);
    QVERIFY(sorted.at(7) == cap5);

    delete channel;
}

void QLCChannel_Test::copy()
{
    QLCChannel* channel = new QLCChannel();
    QVERIFY(channel->capabilities().size() == 0);

    channel->setName("Foobar");
    channel->setGroup(QLCChannel::Tilt);
    channel->setControlByte(QLCChannel::ControlByte(3));
    channel->setColour(QLCChannel::Yellow);

    QLCCapability* cap1 = new QLCCapability(10, 19, "10-19");
    QVERIFY(channel->addCapability(cap1) == true);

    QLCCapability* cap2 = new QLCCapability(50, 59, "50-59");
    QVERIFY(channel->addCapability(cap2) == true);

    QLCCapability* cap3 = new QLCCapability(40, 49, "40-49");
    QVERIFY(channel->addCapability(cap3) == true);

    QLCCapability* cap4 = new QLCCapability(0, 9, "0-9");
    QVERIFY(channel->addCapability(cap4) == true);

    QLCCapability* cap5 = new QLCCapability(200, 209, "200-209");
    QVERIFY(channel->addCapability(cap5) == true);

    QLCCapability* cap6 = new QLCCapability(30, 39, "30-39");
    QVERIFY(channel->addCapability(cap6) == true);

    QLCCapability* cap7 = new QLCCapability(26, 29, "26-29");
    QVERIFY(channel->addCapability(cap7) == true);

    QLCCapability* cap8 = new QLCCapability(20, 25, "20-25");
    QVERIFY(channel->addCapability(cap8) == true);

    /* Create a copy of the original channel */
    QLCChannel* copy = channel->createCopy();

    QVERIFY(copy->name() == "Foobar");
    QVERIFY(copy->group() == QLCChannel::Tilt);
    QVERIFY(copy->controlByte() == QLCChannel::ControlByte(3));
    QVERIFY(copy->colour() == QLCChannel::Yellow);

    /* Verify that the capabilities in the copied channel are also
       copies i.e. their pointers are not the same as the originals. */
    QList <QLCCapability*> caps(copy->capabilities());
    QVERIFY(caps.size() == 8);
    QVERIFY(caps.at(0) != cap1);
    QVERIFY(caps.at(0)->name() == cap1->name());
    QVERIFY(caps.at(0)->min() == cap1->min());
    QVERIFY(caps.at(0)->max() == cap1->max());

    QVERIFY(caps.at(1) != cap2);
    QVERIFY(caps.at(1)->name() == cap2->name());
    QVERIFY(caps.at(1)->min() == cap2->min());
    QVERIFY(caps.at(1)->max() == cap2->max());

    QVERIFY(caps.at(2) != cap3);
    QVERIFY(caps.at(2)->name() == cap3->name());
    QVERIFY(caps.at(2)->min() == cap3->min());
    QVERIFY(caps.at(2)->max() == cap3->max());

    QVERIFY(caps.at(3) != cap4);
    QVERIFY(caps.at(3)->name() == cap4->name());
    QVERIFY(caps.at(3)->min() == cap4->min());
    QVERIFY(caps.at(3)->max() == cap4->max());

    QVERIFY(caps.at(4) != cap5);
    QVERIFY(caps.at(4)->name() == cap5->name());
    QVERIFY(caps.at(4)->min() == cap5->min());
    QVERIFY(caps.at(4)->max() == cap5->max());

    QVERIFY(caps.at(5) != cap6);
    QVERIFY(caps.at(5)->name() == cap6->name());
    QVERIFY(caps.at(5)->min() == cap6->min());
    QVERIFY(caps.at(5)->max() == cap6->max());

    QVERIFY(caps.at(6) != cap7);
    QVERIFY(caps.at(6)->name() == cap7->name());
    QVERIFY(caps.at(6)->min() == cap7->min());
    QVERIFY(caps.at(6)->max() == cap7->max());

    QVERIFY(caps.at(7) != cap8);
    QVERIFY(caps.at(7)->name() == cap8->name());
    QVERIFY(caps.at(7)->min() == cap8->min());
    QVERIFY(caps.at(7)->max() == cap8->max());
}

void QLCChannel_Test::load()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Channel");
    xmlWriter.writeAttribute("Name", "Channel1");

    xmlWriter.writeStartElement("Group");
    xmlWriter.writeAttribute("Byte", "1");
    xmlWriter.writeCharacters("Tilt");
    xmlWriter.writeEndElement();

    xmlWriter.writeTextElement("Colour", QLCChannel::colourToString(QLCChannel::Cyan));

    xmlWriter.writeStartElement("Capability");
    xmlWriter.writeAttribute("Min", "0");
    xmlWriter.writeAttribute("Max", "10");
    xmlWriter.writeCharacters("Cap1");
    xmlWriter.writeEndElement();

    /* Overlaps with cap1, shouldn't appear in the channel */
    xmlWriter.writeStartElement("Capability");
    xmlWriter.writeAttribute("Min", "5");
    xmlWriter.writeAttribute("Max", "15");
    xmlWriter.writeCharacters("Cap2");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Capability");
    xmlWriter.writeAttribute("Min", "11");
    xmlWriter.writeAttribute("Max", "20");
    xmlWriter.writeCharacters("Cap3");
    xmlWriter.writeEndElement();

    /* Invalid capability tag, shouldn't appear in the channel, since it
       is not recognized by the channel. */
    xmlWriter.writeStartElement("apability");
    xmlWriter.writeAttribute("Min", "21");
    xmlWriter.writeAttribute("Max", "30");
    xmlWriter.writeCharacters("Cap4");
    xmlWriter.writeEndElement();

    /* Missing minimum value, shouldn't appear in the channel, because
       loadXML() fails. */
    xmlWriter.writeStartElement("Capability");
    xmlWriter.writeAttribute("Max", "30");
    xmlWriter.writeCharacters("Cap5");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QLCChannel ch;
    QVERIFY(ch.loadXML(xmlReader) == true);
    qDebug() << int(ch.colour());
    QVERIFY(ch.name() == "Channel1");
    QVERIFY(ch.group() == QLCChannel::Tilt);
    QVERIFY(ch.controlByte() == QLCChannel::LSB);
    QVERIFY(ch.colour() == QLCChannel::Cyan);
    QVERIFY(ch.capabilities().size() == 2);
    QVERIFY(ch.capabilities()[0]->name() == "Cap1");
    QVERIFY(ch.capabilities()[1]->name() == "Cap3");
}

void QLCChannel_Test::loadWrongRoot()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Chanel");
    xmlWriter.writeAttribute("Name", "Channel1");

    xmlWriter.writeStartElement("Group");
    xmlWriter.writeAttribute("Byte", "1");
    xmlWriter.writeCharacters("Tilt");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Capability");
    xmlWriter.writeAttribute("Min", "0");
    xmlWriter.writeAttribute("Max", "10");
    xmlWriter.writeCharacters("Cap1");
    xmlWriter.writeEndElement();

    /* Overlaps with cap1, shouldn't appear in the channel */
    xmlWriter.writeStartElement("Capability");
    xmlWriter.writeAttribute("Min", "5");
    xmlWriter.writeAttribute("Max", "15");
    xmlWriter.writeCharacters("Cap2");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Capability");
    xmlWriter.writeAttribute("Min", "11");
    xmlWriter.writeAttribute("Max", "20");
    xmlWriter.writeCharacters("Cap3");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QLCChannel ch;
    QVERIFY(ch.loadXML(xmlReader) == false);
    QVERIFY(ch.name().isEmpty());
    QVERIFY(ch.group() == QLCChannel::Intensity);
    QVERIFY(ch.controlByte() == QLCChannel::MSB);
    QVERIFY(ch.capabilities().size() == 0);
}

void QLCChannel_Test::save()
{
    QLCChannel* channel = new QLCChannel();

    channel->setName("Foobar");
    channel->setGroup(QLCChannel::Shutter);
    channel->setControlByte(QLCChannel::LSB);

    QLCCapability* cap1 = new QLCCapability(0, 9, "One");
    QVERIFY(channel->addCapability(cap1) == true);

    QLCCapability* cap2 = new QLCCapability(10, 19, "Two");
    QVERIFY(channel->addCapability(cap2) == true);

    QLCCapability* cap3 = new QLCCapability(20, 29, "Three");
    QVERIFY(channel->addCapability(cap3) == true);

    QLCCapability* cap4 = new QLCCapability(30, 39, "Four");
    QVERIFY(channel->addCapability(cap4) == true);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(channel->saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);

    xmlReader.readNextStartElement();
    QVERIFY(xmlReader.name().toString() == "Channel");
    QVERIFY(xmlReader.attributes().value("Name").toString() == "Foobar");

    bool group = false;
    bool capOne = false, capTwo = false, capThree = false, capFour = false;

    while (xmlReader.readNextStartElement())
    {
        if (xmlReader.name().toString() == "Group")
        {
            group = true;
            QVERIFY(xmlReader.attributes().value("Byte").toString() == "1");
            QVERIFY(xmlReader.readElementText() == "Shutter");
        }
        else if (xmlReader.name().toString() == "Capability")
        {
            QString capName = xmlReader.readElementText();
            if (capName == "One" && capOne == false)
                capOne = true;
            else if (capName == "Two" && capTwo == false)
                capTwo = true;
            else if (capName == "Three" && capThree == false)
                capThree = true;
            else if (capName == "Four" && capFour == false)
                capFour = true;
            else
                QFAIL("Same capability saved multiple times");
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(xmlReader.name().toString())
                  .toLatin1());
            xmlReader.skipCurrentElement();
        }
    }

    QVERIFY(group == true);
    QVERIFY(capOne == true);
    QVERIFY(capTwo == true);
    QVERIFY(capThree == true);
    QVERIFY(capFour == true);

    delete channel;
}

QTEST_APPLESS_MAIN(QLCChannel_Test)
