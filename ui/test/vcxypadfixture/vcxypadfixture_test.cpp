/*
  Q Light Controller Plus - Unit test
  vcxypadfixture_test.cpp

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

#define private public
#include "vcxypadfixture.h"
#undef private

#include "vcxypadfixture_test.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#define protected public
#include "genericfader.h"
#include "universe.h"
#undef protected
#include "qlcfile.h"
#include "doc.h"

#include "../../../engine/test/common/resource_paths.h"

void VCXYPadFixture_Test::initTestCase()
{
    m_doc = new Doc(this);
    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_doc->fixtureDefCache()->loadMap(dir) == true);
}

void VCXYPadFixture_Test::init()
{
}

void VCXYPadFixture_Test::cleanup()
{
    m_doc->clearContents();
}

void VCXYPadFixture_Test::initial()
{
    VCXYPadFixture fxi(m_doc);
    QVERIFY(!fxi.m_head.isValid());
    QCOMPARE(fxi.m_doc, m_doc);

    QCOMPARE(fxi.m_xMin, qreal(0));
    QCOMPARE(fxi.m_xMax, qreal(1));
    QCOMPARE(fxi.m_xReverse, false);

    QCOMPARE(fxi.m_yMin, qreal(0));
    QCOMPARE(fxi.m_yMax, qreal(1));
    QCOMPARE(fxi.m_yReverse, false);

    QCOMPARE(fxi.m_xLSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_xMSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_yLSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_yMSB, QLCChannel::invalid());
}

void VCXYPadFixture_Test::params()
{
    VCXYPadFixture fxi(m_doc);
    fxi.setX(0.2, 0.35, false);
    fxi.setDisplayMode(VCXYPadFixture::Percentage);
    QCOMPARE(fxi.xMin(), 0.2);
    QCOMPARE(fxi.xMax(), 0.35);
    QCOMPARE(fxi.xReverse(), false);
    QCOMPARE(fxi.xBrief(), QString("%1% - %2%").arg(100 * 0.2).arg(100 * 0.35));

    fxi.setDisplayMode(VCXYPadFixture::DMX);
    QCOMPARE(fxi.xBrief(), QString("%1 - %2").arg(qRound(255 * 0.2)).arg(qRound(255 * 0.35)));

    fxi.setX(0.2, 0.35, true);
    fxi.setDisplayMode(VCXYPadFixture::Percentage);
    QCOMPARE(fxi.xReverse(), true);
    QCOMPARE(fxi.xBrief(), QString("%1: %2% - %3%").arg(tr("Reversed")).arg(100 * 0.35).arg(100 * 0.2));

    fxi.setDisplayMode(VCXYPadFixture::DMX);
    QCOMPARE(fxi.xBrief(), QString("%1: %2 - %3").arg(tr("Reversed")).arg(qRound(255 * 0.35)).arg(qRound(255 * 0.2)));

    fxi.setY(0.1, 0.8, false);
    fxi.setDisplayMode(VCXYPadFixture::Percentage);
    QCOMPARE(fxi.yMin(), 0.1);
    QCOMPARE(fxi.yMax(), 0.8);
    QCOMPARE(fxi.yReverse(), false);
    QCOMPARE(fxi.yBrief(), QString("%1% - %2%").arg(100 * 0.1).arg(100 * 0.8));

    fxi.setDisplayMode(VCXYPadFixture::DMX);
    QCOMPARE(fxi.yBrief(), QString("%1 - %2").arg(qRound(255 * 0.1)).arg(qRound(255 * 0.8)));

    fxi.setY(0.1, 0.8, true);
    fxi.setDisplayMode(VCXYPadFixture::Percentage);
    QCOMPARE(fxi.yReverse(), true);
    QCOMPARE(fxi.yBrief(), QString("%1: %2% - %3%").arg(tr("Reversed")).arg(100 * 0.8).arg(100 * 0.1));

    fxi.setDisplayMode(VCXYPadFixture::DMX);
    QCOMPARE(fxi.yBrief(), QString("%1: %2 - %3").arg(tr("Reversed")).arg(qRound(255 * 0.8)).arg(qRound(255 * 0.1)));
}

void VCXYPadFixture_Test::paramsDegrees()
{
    Fixture *fxi = new Fixture(m_doc);
    QLCFixtureDef *def = m_doc->fixtureDefCache()->fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);
    QLCFixtureMode *mode = def->modes().first();
    QVERIFY(mode != NULL);
    fxi->setFixtureDefinition(def, mode);
    m_doc->addFixture(fxi);

    VCXYPadFixture xy(m_doc);
    xy.setHead(GroupHead(fxi->id(), 0));
    xy.setX(0, 1, false);
    xy.setY(0, 1, false);

    QCOMPARE(xy.xBrief(), QString("%1° - %2°").arg(0).arg(180));
    QCOMPARE(xy.yBrief(), QString("%1° - %2°").arg(0).arg(75));

    xy.setX(0.5, 1, false);
    xy.setY(0.2, 1, false);

    QCOMPARE(xy.xBrief(), QString("%1° - %2°").arg(90).arg(180));
    QCOMPARE(xy.yBrief(), QString("%1° - %2°").arg(15).arg(75));
}

void VCXYPadFixture_Test::fromVariantBelowZero()
{
    QStringList list;
    list << QString("12");
    list << QString("7");
    list << QString("-0.1");
    list << QString("-0.2");
    list << QString("0");
    list << QString("-0.3");
    list << QString("-0.4");
    list << QString("0");
    list << QString("1");
    list << QString("1");

    VCXYPadFixture fxi(m_doc, list);
    QCOMPARE(fxi.m_head.fxi, quint32(12));
    QCOMPARE(fxi.m_head.head, 7);

    QCOMPARE(fxi.m_xMin, qreal(0));
    QCOMPARE(fxi.m_xMax, qreal(0));
    QCOMPARE(fxi.m_xReverse, false);

    QCOMPARE(fxi.m_yMin, qreal(0));
    QCOMPARE(fxi.m_yMax, qreal(0));
    QCOMPARE(fxi.m_yReverse, false);

    QCOMPARE(fxi.m_xLSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_xMSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_yLSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_yMSB, QLCChannel::invalid());
}

void VCXYPadFixture_Test::fromVariantAboveOne()
{
    QStringList list;
    list << QString("12");
    list << QString("0");
    list << QString("1.1");
    list << QString("1.2");
    list << QString("2");
    list << QString("1.3");
    list << QString("1.4");
    list << QString("3");
    list << QString("1");
    list << QString("1");

    VCXYPadFixture fxi(m_doc, list);
    QCOMPARE(fxi.m_head.fxi, quint32(12));
    QCOMPARE(fxi.m_head.head, 0);

    QCOMPARE(fxi.m_xMin, qreal(1));
    QCOMPARE(fxi.m_xMax, qreal(1));
    QCOMPARE(fxi.m_xReverse, true);

    QCOMPARE(fxi.m_yMin, qreal(1));
    QCOMPARE(fxi.m_yMax, qreal(1));
    QCOMPARE(fxi.m_yReverse, true);

    QCOMPARE(fxi.m_xLSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_xMSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_yLSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_yMSB, QLCChannel::invalid());
}

void VCXYPadFixture_Test::fromVariantWithinRange()
{
    QStringList list;
    list << QString("12");
    list << QString("0");
    list << QString("0.3");
    list << QString("0.4");
    list << QString("1");
    list << QString("0.5");
    list << QString("0.6");
    list << QString("1");
    list << QString("1");
    list << QString("1");

    VCXYPadFixture fxi(m_doc, list);
    QCOMPARE(fxi.m_head.fxi, quint32(12));
    QCOMPARE(fxi.m_head.head, 0);

    QCOMPARE(fxi.m_xMin, qreal(0.3));
    QCOMPARE(fxi.m_xMax, qreal(0.4));
    QCOMPARE(fxi.m_xReverse, true);

    QCOMPARE(fxi.m_yMin, qreal(0.5));
    QCOMPARE(fxi.m_yMax, qreal(0.6));
    QCOMPARE(fxi.m_yReverse, true);

    QCOMPARE(fxi.m_xLSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_xMSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_yLSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_yMSB, QLCChannel::invalid());
}

void VCXYPadFixture_Test::fromVariantWrongSize()
{
    QStringList list;
    list << QString("12");
    list << QString("0");
    list << QString("0.3");
    list << QString("0.4");
    list << QString("1");
    list << QString("0.5");
    list << QString("0.6");
    list << QString("1");
    list.takeLast();

    VCXYPadFixture fxi(m_doc, list);
    QVERIFY(!fxi.m_head.isValid());

    QCOMPARE(fxi.m_xMin, qreal(0));
    QCOMPARE(fxi.m_xMax, qreal(1));
    QCOMPARE(fxi.m_xReverse, false);

    QCOMPARE(fxi.m_yMin, qreal(0));
    QCOMPARE(fxi.m_yMax, qreal(1));
    QCOMPARE(fxi.m_yReverse, false);

    QCOMPARE(fxi.m_xLSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_xMSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_yLSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_yMSB, QLCChannel::invalid());
}

void VCXYPadFixture_Test::fromVariantWrongVariant()
{
    VCXYPadFixture fxi(m_doc, QVariant(42));
    QVERIFY(!fxi.m_head.isValid());

    QCOMPARE(fxi.m_xMin, qreal(0));
    QCOMPARE(fxi.m_xMax, qreal(1));
    QCOMPARE(fxi.m_xReverse, false);

    QCOMPARE(fxi.m_yMin, qreal(0));
    QCOMPARE(fxi.m_yMax, qreal(1));
    QCOMPARE(fxi.m_yReverse, false);

    QCOMPARE(fxi.m_xLSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_xMSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_yLSB, QLCChannel::invalid());
    QCOMPARE(fxi.m_yMSB, QLCChannel::invalid());
}

void VCXYPadFixture_Test::toVariant()
{
    VCXYPadFixture fxi(m_doc);
    fxi.setHead(GroupHead(3000, 178));
    fxi.setX(0.1, 0.2, true);
    fxi.setY(0.3, 0.4, false);

    QVariant var(fxi);
    QVERIFY(var.canConvert<QStringList>() == true);
    QStringList list = var.toStringList();
    QCOMPARE(list.size(), 10);
    QCOMPARE(list.takeFirst(), QString("3000"));
    QCOMPARE(list.takeFirst(), QString("178"));
    QCOMPARE(list.takeFirst(), QString("0.1"));
    QCOMPARE(list.takeFirst(), QString("0.2"));
    QCOMPARE(list.takeFirst(), QString("1"));
    QCOMPARE(list.takeFirst(), QString("0.3"));
    QCOMPARE(list.takeFirst(), QString("0.4"));
    QCOMPARE(list.takeFirst(), QString("0"));
    QCOMPARE(list.takeFirst(), QString("1"));
    QCOMPARE(list.takeFirst(), QString("1"));
}

void VCXYPadFixture_Test::copy()
{
    VCXYPadFixture fxi(m_doc);
    fxi.setHead(GroupHead(3000, 178));
    fxi.setX(0.1, 0.2, true);
    fxi.setY(0.3, 0.4, false);

    VCXYPadFixture fxi2 = fxi;
    QCOMPARE(fxi2.m_doc, fxi.m_doc);
    QCOMPARE(fxi2.head(), fxi.head());

    QCOMPARE(fxi2.xMin(), fxi.xMin());
    QCOMPARE(fxi2.xMax(), fxi.xMax());
    QCOMPARE(fxi2.xReverse(), fxi.xReverse());

    QCOMPARE(fxi2.yMin(), fxi.yMin());
    QCOMPARE(fxi2.yMax(), fxi.yMax());
    QCOMPARE(fxi2.yReverse(), fxi.yReverse());
}

void VCXYPadFixture_Test::compare()
{
    VCXYPadFixture fxi(m_doc);
    fxi.setHead(GroupHead(42, 13));

    VCXYPadFixture fxi2(m_doc);
    fxi2.setHead(GroupHead(24, 31));

    QVERIFY((fxi == fxi2) == false);

    fxi2.setHead(GroupHead(42, 13));
    QVERIFY(fxi == fxi2);
}

void VCXYPadFixture_Test::name()
{
    Fixture *fxi = new Fixture(m_doc);
    fxi->setName("Test fixture");
    fxi->setChannels(1);
    m_doc->addFixture(fxi);

    VCXYPadFixture xy(m_doc);
    QCOMPARE(xy.name(), QString());

    xy.setHead(GroupHead(fxi->id(), 0));
    QCOMPARE(xy.name(), QString("Test fixture"));

    xy.setHead(GroupHead(fxi->id() + 1, 0));
    QCOMPARE(xy.name(), QString());

    m_doc->deleteFixture(fxi->id());
}

void VCXYPadFixture_Test::loadXMLWrongRoot()
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Fixteru");
    xmlWriter.writeAttribute("ID", "69");
    xmlWriter.writeAttribute("Head", "0");
    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);

    buffer.seek(0);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    VCXYPadFixture fxi(m_doc);
    QVERIFY(fxi.loadXML(xmlReader) == false);
}

void VCXYPadFixture_Test::loadXMLHappy()
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Fixture");
    xmlWriter.writeAttribute("ID", "69");
    xmlWriter.writeAttribute("Head", "0");

    xmlWriter.writeStartElement("Axis");
    xmlWriter.writeAttribute("ID", "X");
    xmlWriter.writeAttribute("LowLimit", "0.1");
    xmlWriter.writeAttribute("HighLimit", "0.5");
    xmlWriter.writeAttribute("Reverse", "True");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Axis");
    xmlWriter.writeAttribute("ID", "Y");
    xmlWriter.writeAttribute("LowLimit", "0.2");
    xmlWriter.writeAttribute("HighLimit", "0.6");
    xmlWriter.writeAttribute("Reverse", "True");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Axis"); // Foo axis
    xmlWriter.writeAttribute("ID", "Z");
    xmlWriter.writeAttribute("LowLimit", "0.2");
    xmlWriter.writeAttribute("HighLimit", "0.6");
    xmlWriter.writeAttribute("Reverse", "True");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Foo");
    xmlWriter.writeAttribute("ID", "Z");
    xmlWriter.writeAttribute("LowLimit", "0.2");
    xmlWriter.writeAttribute("HighLimit", "0.6");
    xmlWriter.writeAttribute("Reverse", "True");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);

    buffer.seek(0);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    VCXYPadFixture fxi(m_doc);
    QVERIFY(fxi.loadXML(xmlReader) == true);
    QCOMPARE(fxi.head().fxi, quint32(69));
    QCOMPARE(fxi.head().head, 0);

    QCOMPARE(fxi.xMin(), qreal(0.1));
    QCOMPARE(fxi.xMax(), qreal(0.5));
    QCOMPARE(fxi.xReverse(), true);

    QCOMPARE(fxi.yMin(), qreal(0.2));
    QCOMPARE(fxi.yMax(), qreal(0.6));
    QCOMPARE(fxi.yReverse(), true);
}

void VCXYPadFixture_Test::loadXMLSad()
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Fixture");
    xmlWriter.writeAttribute("ID", "69");
    xmlWriter.writeAttribute("Head", "0");

    xmlWriter.writeStartElement("Axis");
    xmlWriter.writeAttribute("ID", "X");
    xmlWriter.writeAttribute("LowLimit", "0.1");
    xmlWriter.writeAttribute("HighLimit", "0.5");
    xmlWriter.writeAttribute("Reverse", "False");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Axis");
    xmlWriter.writeAttribute("ID", "Y");
    xmlWriter.writeAttribute("LowLimit", "0.2");
    xmlWriter.writeAttribute("HighLimit", "0.6");
    xmlWriter.writeAttribute("Reverse", "False");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Axis"); // Foo axis
    xmlWriter.writeAttribute("ID", "Z");
    xmlWriter.writeAttribute("LowLimit", "0.2");
    xmlWriter.writeAttribute("HighLimit", "0.6");
    xmlWriter.writeAttribute("Reverse", "False");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Foo");
    xmlWriter.writeAttribute("ID", "Z");
    xmlWriter.writeAttribute("LowLimit", "0.2");
    xmlWriter.writeAttribute("HighLimit", "0.6");
    xmlWriter.writeAttribute("Reverse", "False");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);

    buffer.seek(0);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    VCXYPadFixture fxi(m_doc);
    QVERIFY(fxi.loadXML(xmlReader) == true);
    QCOMPARE(fxi.head().fxi, quint32(69));
    QCOMPARE(fxi.head().head, 0);

    QCOMPARE(fxi.xMin(), qreal(0.1));
    QCOMPARE(fxi.xMax(), qreal(0.5));
    QCOMPARE(fxi.xReverse(), false);

    QCOMPARE(fxi.yMin(), qreal(0.2));
    QCOMPARE(fxi.yMax(), qreal(0.6));
    QCOMPARE(fxi.yReverse(), false);
}

void VCXYPadFixture_Test::saveXMLHappy()
{
    VCXYPadFixture fxi(m_doc);
    fxi.setHead(GroupHead(54,32));
    fxi.setX(0.1, 0.2, true);
    fxi.setY(0.3, 0.4, true);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(fxi.saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QCOMPARE(xmlReader.name().toString(), QString("Fixture"));
    QCOMPARE(xmlReader.attributes().value("ID").toString(), QString("54"));
    QCOMPARE(xmlReader.attributes().value("Head").toString(), QString("32"));

    bool x = false, y = false;

    while (xmlReader.readNextStartElement())
    {
        if (xmlReader.name().toString() == "Axis")
        {
            QXmlStreamAttributes attrs = xmlReader.attributes();
            QString id = attrs.value("ID").toString();
            QString low = attrs.value("LowLimit").toString();
            QString high = attrs.value("HighLimit").toString();
            QString rev = attrs.value("Reverse").toString();
            if (id == "X")
            {
                x = true;
                QCOMPARE(low, QString("0.1"));
                QCOMPARE(high, QString("0.2"));
                QCOMPARE(rev, QString("True"));
            }
            else if (id == "Y")
            {
                y = true;
                QCOMPARE(low, QString("0.3"));
                QCOMPARE(high, QString("0.4"));
                QCOMPARE(rev, QString("True"));
            }
            else
            {
                QFAIL(QString("Unexpected axis: %1").arg(id).toUtf8().constData());
            }
            xmlReader.skipCurrentElement();
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(xmlReader.name().toString()).toUtf8().constData());
        }
    }

    QVERIFY(x == true);
    QVERIFY(y == true);
}

void VCXYPadFixture_Test::saveXMLSad()
{
    VCXYPadFixture fxi(m_doc);
    fxi.setHead(GroupHead(54,32));
    fxi.setX(0.1, 0.2, false);
    fxi.setY(0.3, 0.4, false);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(fxi.saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QCOMPARE(xmlReader.name().toString(), QString("Fixture"));
    QCOMPARE(xmlReader.attributes().value("ID").toString(), QString("54"));
    QCOMPARE(xmlReader.attributes().value("Head").toString(), QString("32"));

    bool x = false, y = false;

    while (xmlReader.readNextStartElement())
    {
        if (xmlReader.name().toString() == "Axis")
        {
            QXmlStreamAttributes attrs = xmlReader.attributes();
            QString id = attrs.value("ID").toString();
            QString low = attrs.value("LowLimit").toString();
            QString high = attrs.value("HighLimit").toString();
            QString rev = attrs.value("Reverse").toString();

            if (id == "X")
            {
                x = true;
                QCOMPARE(low, QString("0.1"));
                QCOMPARE(high, QString("0.2"));
                QCOMPARE(rev, QString("False"));
            }
            else if (id == "Y")
            {
                y = true;
                QCOMPARE(low, QString("0.3"));
                QCOMPARE(high, QString("0.4"));
                QCOMPARE(rev, QString("False"));
            }
            else
            {
                QFAIL(QString("Unexpected axis: %1").arg(id).toUtf8().constData());
            }
            xmlReader.skipCurrentElement();
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(xmlReader.name().toString()).toUtf8().constData());
        }
    }

    QVERIFY(x == true);
    QVERIFY(y == true);
}

void VCXYPadFixture_Test::armNoFixture()
{
    VCXYPadFixture xy(m_doc);
    xy.arm();
    QCOMPARE(xy.m_xMSB, QLCChannel::invalid());
    QCOMPARE(xy.m_xLSB, QLCChannel::invalid());
    QCOMPARE(xy.m_yMSB, QLCChannel::invalid());
    QCOMPARE(xy.m_yLSB, QLCChannel::invalid());
}

void VCXYPadFixture_Test::armDimmer()
{
    Fixture *fxi = new Fixture(m_doc);
    fxi->setChannels(6);
    m_doc->addFixture(fxi);

    VCXYPadFixture xy(m_doc);
    xy.setHead(GroupHead(fxi->id(), 0));
    xy.arm();
    QCOMPARE(xy.m_xMSB, QLCChannel::invalid());
    QCOMPARE(xy.m_xLSB, QLCChannel::invalid());
    QCOMPARE(xy.m_yMSB, QLCChannel::invalid());
    QCOMPARE(xy.m_yLSB, QLCChannel::invalid());

    m_doc->deleteFixture(fxi->id());
}

void VCXYPadFixture_Test::arm8bit()
{
    Fixture *fxi = new Fixture(m_doc);
    QLCFixtureDef *def = m_doc->fixtureDefCache()->fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);
    QLCFixtureMode *mode = def->modes().first();
    QVERIFY(mode != NULL);
    fxi->setFixtureDefinition(def, mode);
    fxi->setAddress(50);
    m_doc->addFixture(fxi);

    VCXYPadFixture xy(m_doc);
    xy.setHead(GroupHead(fxi->id(), 0));
    xy.arm();
    QCOMPARE(xy.m_fixtureAddress, quint32(50));
    QCOMPARE(xy.m_xMSB, quint32(0));
    QCOMPARE(xy.m_xLSB, QLCChannel::invalid());
    QCOMPARE(xy.m_yMSB, quint32(1));
    QCOMPARE(xy.m_yLSB, QLCChannel::invalid());

    m_doc->deleteFixture(fxi->id());
}

void VCXYPadFixture_Test::arm16bit()
{
    Fixture *fxi = new Fixture(m_doc);
    QLCFixtureDef *def = m_doc->fixtureDefCache()->fixtureDef("Varytec", "Easy Move LED XS Spot");
    QVERIFY(def != NULL);
    QLCFixtureMode *mode = def->modes().first();
    QVERIFY(mode != NULL);
    fxi->setFixtureDefinition(def, mode);
    m_doc->addFixture(fxi);

    VCXYPadFixture xy(m_doc);
    xy.setHead(GroupHead(fxi->id(), 0));
    xy.arm();
    QCOMPARE(xy.m_xMSB, quint32(0));
    QCOMPARE(xy.m_xLSB, quint32(1));
    QCOMPARE(xy.m_yMSB, quint32(2));
    QCOMPARE(xy.m_yLSB, quint32(3));

    m_doc->deleteFixture(fxi->id());
}

void VCXYPadFixture_Test::disarm()
{
    VCXYPadFixture xy(m_doc);
    xy.m_xMSB = 0;
    xy.m_xLSB = 1;
    xy.m_yMSB = 2;
    xy.m_yLSB = 3;
    xy.disarm();
    QCOMPARE(xy.m_xMSB, QLCChannel::invalid());
    QCOMPARE(xy.m_xLSB, QLCChannel::invalid());
    QCOMPARE(xy.m_yMSB, QLCChannel::invalid());
    QCOMPARE(xy.m_yLSB, QLCChannel::invalid());
}

void VCXYPadFixture_Test::writeDimmer()
{
    VCXYPadFixture xy(m_doc);
    QList<Universe*> ua = m_doc->inputOutputMap()->universes();
    QSharedPointer<GenericFader> fader = ua[0]->requestFader();

    xy.writeDMX(1, 1, fader, ua[0]);
    ua[0]->processFaders();
    QCOMPARE(ua[0]->preGMValues()[0], char(0));

    xy.m_xMSB = 0;
    xy.m_yMSB = QLCChannel::invalid();
    xy.writeDMX(1, 1, fader, ua[0]);
    ua[0]->processFaders();
    QCOMPARE(ua[0]->preGMValues()[0], char(0));

    xy.m_xMSB = QLCChannel::invalid();
    xy.m_yMSB = 0;
    xy.writeDMX(1, 1, fader, ua[0]);
    ua[0]->processFaders();
    QCOMPARE(ua[0]->preGMValues()[0], char(0));
}

void VCXYPadFixture_Test::write8bitNoReverse()
{
    Fixture *fxi = new Fixture(m_doc);
    QLCFixtureDef *def = m_doc->fixtureDefCache()->fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);
    QLCFixtureMode *mode = def->modes().first();
    QVERIFY(mode != NULL);
    fxi->setFixtureDefinition(def, mode);
    m_doc->addFixture(fxi);

    VCXYPadFixture xy(m_doc);
    xy.setHead(GroupHead(fxi->id(), 0));
    xy.setX(0, 1, false);
    xy.setY(0, 1, false);
    xy.arm();

    QList<Universe*> ua = m_doc->inputOutputMap()->universes();
    QSharedPointer<GenericFader> fader = ua[0]->requestFader();

    for (qreal i = 0; i <= 1.01; i += (qreal(1) / qreal(USHRT_MAX)))
    {
        xy.writeDMX(i, 1.0 - i, fader, ua[0]);

        ushort x = floor((qreal(USHRT_MAX) * i) + 0.5);
        ushort y = floor((qreal(USHRT_MAX) * (1.0 - i)) + 0.5);

        ua[0]->processFaders();
        QCOMPARE(ua[0]->preGMValues()[0], char(x >> 8));
        QCOMPARE(ua[0]->preGMValues()[1], char(y >> 8));
        QCOMPARE(ua[0]->preGMValues()[2], char(0));
        QCOMPARE(ua[0]->preGMValues()[3], char(0));
        QCOMPARE(ua[0]->preGMValues()[4], char(0));
        QCOMPARE(ua[0]->preGMValues()[5], char(0));
    }
}

void VCXYPadFixture_Test::write8bitReverse()
{
    Fixture *fxi = new Fixture(m_doc);
    QLCFixtureDef *def = m_doc->fixtureDefCache()->fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);
    QLCFixtureMode *mode = def->modes().first();
    QVERIFY(mode != NULL);
    fxi->setFixtureDefinition(def, mode);
    m_doc->addFixture(fxi);

    VCXYPadFixture xy(m_doc);
    xy.setHead(GroupHead(fxi->id(), 0));
    xy.setX(0, 1, true);
    xy.setY(0, 1, true);
    xy.arm();

    QList<Universe*> ua = m_doc->inputOutputMap()->universes();
    QSharedPointer<GenericFader> fader = ua[0]->requestFader();

    for (qreal i = 0; i <= 1.01; i += (qreal(1) / qreal(USHRT_MAX)))
    {
        xy.writeDMX(i, 1.0 - i, fader, ua[0]);

        ushort x = floor((qreal(USHRT_MAX) * (1.0 - i)) + 0.5);
        ushort y = floor((qreal(USHRT_MAX) * i) + 0.5);

        ua[0]->processFaders();
        QCOMPARE(ua[0]->preGMValues()[0], char(x >> 8));
        QCOMPARE(ua[0]->preGMValues()[1], char(y >> 8));
        QCOMPARE(ua[0]->preGMValues()[2], char(0));
        QCOMPARE(ua[0]->preGMValues()[3], char(0));
        QCOMPARE(ua[0]->preGMValues()[4], char(0));
        QCOMPARE(ua[0]->preGMValues()[5], char(0));
    }
}

void VCXYPadFixture_Test::write16bitNoReverse()
{
    Fixture *fxi = new Fixture(m_doc);
    QLCFixtureDef *def = m_doc->fixtureDefCache()->fixtureDef("Varytec", "Easy Move LED XS Spot");
    QVERIFY(def != NULL);
    QLCFixtureMode *mode = def->modes().first();
    QVERIFY(mode != NULL);
    fxi->setFixtureDefinition(def, mode);
    m_doc->addFixture(fxi);

    VCXYPadFixture xy(m_doc);
    xy.setHead(GroupHead(fxi->id(), 0));
    xy.setX(0, 1, false);
    xy.setY(0, 1, false);
    xy.arm();

    QList<Universe*> ua = m_doc->inputOutputMap()->universes();
    QSharedPointer<GenericFader> fader = ua[0]->requestFader();

    for (qreal i = 0; i <= 1.01; i += (qreal(1) / qreal(USHRT_MAX)))
    {
        xy.writeDMX(i, 1.0 - i, fader, ua[0]);

        ushort x = floor((qreal(USHRT_MAX) * i) + 0.5);
        ushort y = floor((qreal(USHRT_MAX) * (1.0 - i)) + 0.5);

        ua[0]->processFaders();
        QCOMPARE(ua[0]->preGMValues()[0], char(x >> 8));
        QCOMPARE(ua[0]->preGMValues()[1], char(x & 0xFF));
        QCOMPARE(ua[0]->preGMValues()[2], char(y >> 8));
        QCOMPARE(ua[0]->preGMValues()[3], char(y & 0xFF));
    }
}

void VCXYPadFixture_Test::write16bitReverse()
{
    Fixture *fxi = new Fixture(m_doc);
    QLCFixtureDef *def = m_doc->fixtureDefCache()->fixtureDef("Varytec", "Easy Move LED XS Spot");
    QVERIFY(def != NULL);
    QLCFixtureMode *mode = def->modes().first();
    QVERIFY(mode != NULL);
    fxi->setFixtureDefinition(def, mode);
    m_doc->addFixture(fxi);

    VCXYPadFixture xy(m_doc);
    xy.setHead(GroupHead(fxi->id(), 0));
    xy.setX(0.1, 0.9, true);
    xy.setY(0.2, 0.8, true);
    xy.arm();

#ifdef Q_PROCESSOR_X86_64
    QList<Universe*> ua = m_doc->inputOutputMap()->universes();
    QSharedPointer<GenericFader> fader = ua[0]->requestFader();

    for (qreal i = 0; i <= 1.01; i += (qreal(1) / qreal(USHRT_MAX)))
    {
        xy.writeDMX(i, 1.0 - i, fader, ua[0]);

        qreal xmul = i;
        qreal ymul = 1.0 - i;

        xmul = ((xy.xMax() - xy.xMin()) * xmul) + xy.xMin();
        ymul = ((xy.yMax() - xy.yMin()) * ymul) + xy.yMin();

        xmul = 1 - xmul;
        ymul = 1 - ymul;

        ushort x = floor((qreal(USHRT_MAX) * xmul) + 0.5);
        ushort y = floor((qreal(USHRT_MAX) * ymul) + 0.5);

        ua[0]->processFaders();
        QCOMPARE(ua[0]->preGMValues()[0], char(x >> 8));
        QCOMPARE(ua[0]->preGMValues()[1], char(x & 0xFF));
        QCOMPARE(ua[0]->preGMValues()[2], char(y >> 8));
        QCOMPARE(ua[0]->preGMValues()[3], char(y & 0xFF));
    }
#endif
}

void VCXYPadFixture_Test::writeRange()
{
    QFETCH(qreal, rangeMin);
    QFETCH(qreal, rangeMax);
    QFETCH(bool, reverse);
    QFETCH(int, valueAt0);
    QFETCH(int, valueAt1);

    // For testing pourpose we will test only on the X axis
    // keeping the Y axis at its full range
    Fixture *fxi = new Fixture(m_doc);

    // Select fixture
    QLCFixtureDef *def = m_doc->fixtureDefCache()->fixtureDef("American DJ", "Inno Pocket Spot");
    QVERIFY(def != NULL);
    QLCFixtureMode *mode = def->modes().at(1);
    QVERIFY(mode != NULL);

    fxi->setFixtureDefinition(def, mode);

    QList<Universe*> ua = m_doc->inputOutputMap()->universes();
    QSharedPointer<GenericFader> fader = ua[0]->requestFader();

    m_doc->addFixture(fxi);
    VCXYPadFixture xy(m_doc);
    xy.setHead(GroupHead(fxi->id(), 0));
    xy.setX(rangeMin, rangeMax, reverse);
    xy.setY(0, 1, false);
    xy.arm();

    // Handle on the left
    qreal xmul = 0.0;
    qreal ymul = 0.0;

    xy.writeDMX(xmul, ymul, fader, ua[0]);
    ua[0]->processFaders();
    QCOMPARE((int)ua[0]->preGMValue(0), valueAt0);

    // handle on the right
    xmul = 1;
    xy.writeDMX(xmul, ymul, fader, ua[0]);
    ua[0]->processFaders();
    QCOMPARE((int)ua[0]->preGMValue(0), valueAt1);
}

void VCXYPadFixture_Test::writeRange_data()
{
    QTest::addColumn<qreal>("rangeMin");
    QTest::addColumn<qreal>("rangeMax");
    QTest::addColumn<bool>("reverse");
    QTest::addColumn<int>("valueAt0");
    QTest::addColumn<int>("valueAt1");

    // normal
    QTest::newRow("0-100% / DMX: 0-255") << 0.0 << 1.0 << false << 0 << 255;
    QTest::newRow("40-60% / DMX: 102-153") << 0.4 << 0.6 << false << 102 << 153;
    QTest::newRow("0-20% / DMX: 0-51 ") << 0.0 << 0.2 << false << 0 << 51;
    QTest::newRow("80-100% / DMX: 204-255") << 0.8 << 1.0 << false << 204 << 255;

    // reversed
    QTest::newRow("0-100% / DMX: 0-255 reversed") << 0.0 << 1.0 << true << 255 << 0;
    QTest::newRow("40-60% / DMX: 102-153 reversed") << 0.4 << 0.6 << true << 153 << 102;
    QTest::newRow("0-20% / DMX: 0-51 reversed") << 0.0 << 0.2 << true << 51 << 0;
    QTest::newRow("80-100% / DMX: 204-255 reversed") << 0.8 << 1.0 << true << 255 << 204;
}

void VCXYPadFixture_Test::readRange()
{
    QFETCH(qreal, rangeMin);
    QFETCH(qreal, rangeMax);
    QFETCH(bool, reverse);
    QFETCH(int, valueAt0);
    QFETCH(int, valueAt1);

    // For testing pourpose we will test only on the X axis
    // keeping the Y axis at its full range
    Fixture *fxi = new Fixture(m_doc);

    // Select fixture
    QLCFixtureDef *def = m_doc->fixtureDefCache()->fixtureDef("American DJ", "Inno Pocket Spot");
    QVERIFY(def != NULL);
    QLCFixtureMode *mode = def->modes().at(1);
    QVERIFY(mode != NULL);

    fxi->setFixtureDefinition(def, mode);

    QList<Universe*> ua = m_doc->inputOutputMap()->universes();

    m_doc->addFixture(fxi);
    VCXYPadFixture xy(m_doc);
    xy.setHead(GroupHead(fxi->id(), 0));
    xy.setX(rangeMin, rangeMax, reverse);
    xy.setY(0.0, 1.0, false);
    xy.arm();

    qreal const rangeWidth = rangeMax - rangeMin;
    qreal const rangeStep = 1.0/rangeWidth;

    // Handle on the left
    qreal xmul = 0.0;
    qreal ymul = 0.0;

    ua[0]->write(0, valueAt0);
    ua[0]->processFaders();
    QByteArray uniData = QByteArray(ua[0]->postGMValues()->data(), ua[0]->usedChannels());
    xy.readDMX(uniData, xmul, ymul);
    // the value was scaled to interval rangeMax-rangeMin,
    // so the resolution is less than 1/that range
    // here the value should be near zero
    QVERIFY(xmul < rangeStep);

    // handle on the right
    ua[0]->write(0, valueAt1);
    ua[0]->processFaders();
    uniData = QByteArray(ua[0]->postGMValues()->data(), ua[0]->usedChannels());
    xy.readDMX(uniData, xmul, ymul);
    // again, the resolution depends on the range.
    // here the value should be near one
    QVERIFY(xmul > (rangeWidth - rangeStep));
}

void VCXYPadFixture_Test::readRange_data()
{
    QTest::addColumn<qreal>("rangeMin");
    QTest::addColumn<qreal>("rangeMax");
    QTest::addColumn<bool>("reverse");
    QTest::addColumn<int>("valueAt0");
    QTest::addColumn<int>("valueAt1");

    // normal
    QTest::newRow("0-100% / DMX: 0-255") << 0.0 << 1.0 << false << 0 << 255;
    QTest::newRow("40-60% / DMX: 102-153") << 0.4 << 0.6 << false << 102 << 153;
    QTest::newRow("0-20% / DMX: 0-51 ") << 0.0 << 0.2 << false << 0 << 51;
    QTest::newRow("80-100% / DMX: 204-255") << 0.8 << 1.0 << false << 204 << 255;

    // reversed
    QTest::newRow("0-100% / DMX: 0-255 reversed") << 0.0 << 1.0 << true << 255 << 0;
    QTest::newRow("40-60% / DMX: 102-153 reversed") << 0.4 << 0.6 << true << 153 << 102;
    QTest::newRow("0-20% / DMX: 0-51 reversed") << 0.0 << 0.2 << true << 51 << 0;
    QTest::newRow("80-100% / DMX: 204-255 reversed") << 0.8 << 1.0 << true << 255 << 204;
}

void VCXYPadFixture_Test::cleanupTestCase()
{
    delete m_doc;
    m_doc = NULL;
}

QTEST_MAIN(VCXYPadFixture_Test)
