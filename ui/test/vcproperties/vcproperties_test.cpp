/*
  Q Light Controller Plus - Unit test
  vcproperties_test.cpp

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
#include <QtTest>

#define private public
#include "vcwidgetproperties.h"
#include "vcproperties.h"
#include "qlcfixturedefcache.h"
#include "vcproperties_test.h"
#include "mastertimer.h"
#include "vcwidget.h"
#include "vcframe.h"
#include "doc.h"
#undef private

void VCProperties_Test::init()
{
    m_doc = new Doc(this);
}

void VCProperties_Test::cleanup()
{
    delete m_doc;
    m_doc = NULL;
}

void VCProperties_Test::initial()
{
    VCProperties p;

    QCOMPARE(p.m_size, QSize(1920, 1080));
    QCOMPARE(p.m_gmChannelMode, GrandMaster::Intensity);
    QCOMPARE(p.m_gmValueMode, GrandMaster::Reduce);
    QCOMPARE(p.m_gmInputUniverse, InputOutputMap::invalidUniverse());
    QCOMPARE(p.m_gmInputChannel, QLCChannel::invalid());
}

void VCProperties_Test::copy()
{
    VCProperties p;
    p.m_size = QSize(1, 2);
    p.m_gmChannelMode = GrandMaster::AllChannels;
    p.m_gmValueMode = GrandMaster::Limit;
    p.m_gmInputUniverse = 5;
    p.m_gmInputChannel = 6;

    VCProperties p2(p);
    QCOMPARE(p2.m_size, p.m_size);
    QCOMPARE(p2.m_gmChannelMode, p.m_gmChannelMode);
    QCOMPARE(p2.m_gmValueMode, p.m_gmValueMode);
    QCOMPARE(p2.m_gmInputUniverse, p.m_gmInputUniverse);
    QCOMPARE(p2.m_gmInputChannel, p.m_gmInputChannel);

    VCProperties p3 = p;
    QCOMPARE(p3.m_size, p.m_size);
    QCOMPARE(p3.m_gmChannelMode, p.m_gmChannelMode);
    QCOMPARE(p3.m_gmValueMode, p.m_gmValueMode);
    QCOMPARE(p3.m_gmInputUniverse, p.m_gmInputUniverse);
    QCOMPARE(p3.m_gmInputChannel, p.m_gmInputChannel);
}

void VCProperties_Test::loadXMLSad()
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("VirtualConsole");

    xmlWriter.writeStartElement("Properties");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Frame");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Foo");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);

    buffer.seek(0);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    VCProperties p;
    QVERIFY(p.loadXML(xmlReader) == false);
}

void VCProperties_Test::loadXMLHappy()
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Properties");

    // Size
    xmlWriter.writeStartElement("Size");
    xmlWriter.writeAttribute("Width", "10");
    xmlWriter.writeAttribute("Height", "20");
    xmlWriter.writeEndElement();

    // Grand Master
    xmlWriter.writeStartElement("GrandMaster");
    xmlWriter.writeAttribute("ChannelMode", "All");
    xmlWriter.writeAttribute("ValueMode", "Limit");

    xmlWriter.writeStartElement("Input");
    xmlWriter.writeAttribute("Universe", "2");
    xmlWriter.writeAttribute("Channel", "15");
    xmlWriter.writeEndElement();
    xmlWriter.writeEndElement();

    // Extra crap
    xmlWriter.writeStartElement("Foo");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);

    buffer.seek(0);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    qDebug() << "Buffer:" << buffer.data();

    VCProperties p;
    QVERIFY(p.loadXML(xmlReader) == true);
    QCOMPARE(p.size(), QSize(10, 20));
    QCOMPARE(p.grandMasterChannelMode(), GrandMaster::AllChannels);
    QCOMPARE(p.grandMasterValueMode(), GrandMaster::Limit);
    QCOMPARE(p.grandMasterInputUniverse(), quint32(2));
    QCOMPARE(p.grandMasterInputChannel(), quint32(15));
}

void VCProperties_Test::loadXMLHappyNoInput()
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Properties");

    // Size
    xmlWriter.writeStartElement("Size");
    xmlWriter.writeAttribute("Width", "30");
    xmlWriter.writeAttribute("Height", "40");
    xmlWriter.writeEndElement();

    // Grand Master
    xmlWriter.writeStartElement("GrandMaster");
    xmlWriter.writeAttribute("ChannelMode", "Intensity");
    xmlWriter.writeAttribute("ValueMode", "Reduce");
    xmlWriter.writeEndElement();

    // Extra crap
    xmlWriter.writeStartElement("Foo");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);

    buffer.seek(0);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    qDebug() << "Buffer:" << buffer.data();

    VCProperties p;
    QVERIFY(p.loadXML(xmlReader) == true);
    QCOMPARE(p.size(), QSize(30, 40));
    QCOMPARE(p.grandMasterChannelMode(), GrandMaster::Intensity);
    QCOMPARE(p.grandMasterValueMode(), GrandMaster::Reduce);
}

void VCProperties_Test::loadXMLInput()
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Input");
    xmlWriter.writeAttribute("Universe", "3");
    xmlWriter.writeAttribute("Channel", "78");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);

    buffer.seek(0);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    quint32 universe = 0;
    quint32 channel = 0;

    QVERIFY(VCProperties::loadXMLInput(xmlReader, &universe, &channel) == true);
    QCOMPARE(universe, quint32(3));
    QCOMPARE(channel, quint32(78));

    buffer.close();
    QByteArray bData = buffer.data();
    bData.replace("<Input", "<Inputt");
    buffer.setData(bData);
    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    buffer.seek(0);
    xmlReader.setDevice(&buffer);
    xmlReader.readNextStartElement();

    universe = channel = 0;

    QVERIFY(VCProperties::loadXMLInput(xmlReader, &universe, &channel) == false);
    QCOMPARE(universe, quint32(0));
    QCOMPARE(channel, quint32(0));

    QBuffer buffer2;
    buffer2.open(QIODevice::ReadWrite | QIODevice::Text);
    xmlWriter.setDevice(&buffer2);

    xmlWriter.writeStartElement("Input");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);

    buffer2.seek(0);
    xmlReader.setDevice(&buffer2);
    xmlReader.readNextStartElement();

    universe = channel = 0;
    QVERIFY(VCProperties::loadXMLInput(xmlReader, &universe, &channel) == false);
    QCOMPARE(universe, InputOutputMap::invalidUniverse());
    QCOMPARE(channel, QLCChannel::invalid());
}

void VCProperties_Test::saveXML()
{
    VCProperties p;
    p.m_size = QSize(33, 44);
    p.m_gmChannelMode = GrandMaster::AllChannels;
    p.m_gmValueMode = GrandMaster::Limit;
    p.m_gmInputUniverse = 3;
    p.m_gmInputChannel = 42;

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(p.saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    VCProperties p2;
    QVERIFY(p2.loadXML(xmlReader) == true);
    QCOMPARE(p2.size(), QSize(33, 44));
    QCOMPARE(p2.grandMasterChannelMode(), GrandMaster::AllChannels);
    QCOMPARE(p2.grandMasterValueMode(), GrandMaster::Limit);
    QCOMPARE(p2.grandMasterInputUniverse(), quint32(3));
    QCOMPARE(p2.grandMasterInputChannel(), quint32(42));
}

QTEST_MAIN(VCProperties_Test)
