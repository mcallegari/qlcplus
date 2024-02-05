/*
  Q Light Controller Plus - Test Unit
  vcxypad_test.cpp

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

#include <QFrame>
#include <QtTest>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#define protected public
#define private public
#include "qlcfixturedefcache.h"
#include "qlcfixturemode.h"
#include "qlcinputsource.h"
#include "virtualconsole.h"
#include "qlcfixturedef.h"
#include "vcxypad_test.h"
#include "mastertimer.h"
#include "vcxypadarea.h"
#include "vcwidget.h"
#include "vcxypad.h"
#include "vcframe.h"
#include "qlcfile.h"
#include "doc.h"
#undef private
#undef protected

#include "../../../engine/test/common/resource_paths.h"

void VCXYPad_Test::initTestCase()
{
    m_doc = new Doc(this);

    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_doc->fixtureDefCache()->loadMap(dir) == true);
}

void VCXYPad_Test::init()
{
    new VirtualConsole(NULL, m_doc);
}

void VCXYPad_Test::cleanup()
{
    delete VirtualConsole::instance();
    m_doc->clearContents();
}

void VCXYPad_Test::initial()
{
    QWidget w;

    VCXYPad pad(&w, m_doc);
    QCOMPARE(pad.objectName(), QString("VCXYPad"));
    QCOMPARE(pad.caption(), QString("XY Pad"));
    QCOMPARE(pad.frameStyle(), QFrame::Panel | QFrame::Sunken);
    QCOMPARE(pad.size(), QSize(230, 230));
    QVERIFY(pad.m_area != NULL);
    QVERIFY(pad.m_area->m_activePixmap.isNull() == false);
    QCOMPARE(pad.m_area->position(), QPointF(0, 0));
    QCOMPARE(pad.m_fixtures.size(), 0);
    QVERIFY(pad.m_vSlider != NULL);
    QVERIFY(pad.m_hSlider != NULL);
}

void VCXYPad_Test::fixtures()
{
    QWidget w;

    VCXYPad pad(&w, m_doc);

    VCXYPadFixture xyf1(m_doc);
    xyf1.setHead(GroupHead(1,0));

    pad.appendFixture(xyf1);
    QCOMPARE(pad.m_fixtures.size(), 1);
    pad.appendFixture(xyf1);
    QCOMPARE(pad.m_fixtures.size(), 1);

    VCXYPadFixture xyf2(m_doc);
    xyf2.setHead(GroupHead(2,5));

    pad.appendFixture(xyf2);
    QCOMPARE(pad.m_fixtures.size(), 2);
    pad.appendFixture(xyf2);
    QCOMPARE(pad.m_fixtures.size(), 2);
    pad.appendFixture(xyf1);
    QCOMPARE(pad.m_fixtures.size(), 2);

    pad.removeFixture(GroupHead(3,0));
    QCOMPARE(pad.m_fixtures.size(), 2);

    pad.removeFixture(GroupHead(1,0));
    QCOMPARE(pad.m_fixtures.size(), 1);
    QCOMPARE(pad.m_fixtures[0].head().fxi, quint32(2));
    QCOMPARE(pad.m_fixtures[0].head().head, 5);

    pad.appendFixture(xyf1);
    QCOMPARE(pad.m_fixtures.size(), 2);

    pad.clearFixtures();
    QCOMPARE(pad.m_fixtures.size(), 0);

    // Invalid fixture
    VCXYPadFixture xyf3(m_doc);
    pad.appendFixture(xyf3);
    QCOMPARE(pad.m_fixtures.size(), 0);
}

void VCXYPad_Test::copy()
{
    QWidget w;

    VCFrame parent(&w, m_doc);
    VCXYPad pad(&parent, m_doc);
    pad.setCaption("Dingdong");
    QPointF pt(50, 30);
    pad.m_area->setPosition(pt);

    VCXYPadFixture xyf1(m_doc);
    xyf1.setHead(GroupHead(1,5));
    pad.appendFixture(xyf1);

    VCXYPadFixture xyf2(m_doc);
    xyf2.setHead(GroupHead(2,7));
    pad.appendFixture(xyf2);

    VCXYPadFixture xyf3(m_doc);
    xyf3.setHead(GroupHead(3,9));
    pad.appendFixture(xyf3);

    VCXYPad* copy = qobject_cast<VCXYPad*> (pad.createCopy(&parent));
    QVERIFY(copy != NULL);
    QCOMPARE(copy->m_fixtures.size(), 3);
    QVERIFY(copy->m_fixtures[0] == xyf1);
    QVERIFY(copy->m_fixtures[1] == xyf2);
    QVERIFY(copy->m_fixtures[2] == xyf3);

    QVERIFY(&copy->m_fixtures[0] != &xyf1);
    QVERIFY(&copy->m_fixtures[1] != &xyf2);
    QVERIFY(&copy->m_fixtures[2] != &xyf3);

    QCOMPARE(copy->m_area->position(), pt);
    QCOMPARE(copy->size(), pad.size());
    QCOMPARE(copy->caption(), QString("Dingdong"));
}

void VCXYPad_Test::loadXML()
{
    QWidget w;

    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("XYPad");

    xmlWriter.writeStartElement("Position");
    xmlWriter.writeAttribute("X", "10");
    xmlWriter.writeAttribute("Y", "20");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Fixture");
    xmlWriter.writeAttribute("ID", "69");
    xmlWriter.writeAttribute("Head", "96");

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
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Fixture");
    xmlWriter.writeAttribute("ID", "50");
    xmlWriter.writeAttribute("Head", "55");

    xmlWriter.writeStartElement("Axis");
    xmlWriter.writeAttribute("ID", "X");
    xmlWriter.writeAttribute("LowLimit", "0.0");
    xmlWriter.writeAttribute("HighLimit", "1.0");
    xmlWriter.writeAttribute("Reverse", "False");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Axis");
    xmlWriter.writeAttribute("ID", "Y");
    xmlWriter.writeAttribute("LowLimit", "0.0");
    xmlWriter.writeAttribute("HighLimit", "1.0");
    xmlWriter.writeAttribute("Reverse", "False");
    xmlWriter.writeEndElement();
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("WindowState");
    xmlWriter.writeAttribute("Width", "42");
    xmlWriter.writeAttribute("Height", "69");
    xmlWriter.writeAttribute("X", "3");
    xmlWriter.writeAttribute("Y", "4");
    xmlWriter.writeAttribute("Visible", "True");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Appearance");
    QFont f(w.font());
    f.setPointSize(f.pointSize() + 3);
    xmlWriter.writeTextElement("Font", f.toString());
    xmlWriter.writeEndElement();
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Foobar");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);

    buffer.seek(0);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    VCXYPad pad(&w, m_doc);
    QVERIFY(pad.loadXML(xmlReader) == true);
    QCOMPARE(pad.m_fixtures.size(), 2);
    QCOMPARE(pad.pos(), QPoint(3, 4));
    QCOMPARE(pad.size(), QSize(42, 69));
    QCOMPARE(pad.m_area->position(), QPointF(10, 20));

    VCXYPadFixture fixture(m_doc);
    fixture.setHead(GroupHead(69, 96));
    QVERIFY(pad.m_fixtures.contains(fixture) == true);
    fixture.setHead(GroupHead(50, 55));
    QVERIFY(pad.m_fixtures.contains(fixture) == true);

    // now add Pan and Tilt tags and check that:
    // - an empty input is OK
    // - positions affect the pad area
    buffer.close();
    QByteArray bData = buffer.data();
    bData.replace("</XYPad>", "<Pan Position=\"23\" /><Tilt Position=\"35\" /></XYPad>");
    buffer.setData(bData);
    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    buffer.seek(0);
    xmlReader.setDevice(&buffer);
    xmlReader.readNextStartElement();

    VCXYPad pad2(&w, m_doc);
    QVERIFY(pad2.loadXML(xmlReader) == true);
    QCOMPARE(pad.m_fixtures.size(), 2);
    QCOMPARE(pad.pos(), QPoint(3, 4));
    QCOMPARE(pad.size(), QSize(42, 69));
    QCOMPARE(pad2.m_area->position(), QPointF(23, 35));

    buffer.close();
    bData = buffer.data();
    bData.replace("<XYPad", "<YXPad");
    buffer.setData(bData);
    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    buffer.seek(0);
    xmlReader.setDevice(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(pad.loadXML(xmlReader) == false);
}

void VCXYPad_Test::saveXML()
{
    QWidget w;

    VCXYPad pad(&w, m_doc);
    pad.show();
    w.show();
    pad.setCaption("MyPad");
    pad.resize(QSize(150, 200));
    pad.move(QPoint(10, 20));
    pad.m_area->setPosition(QPointF(23, 45));
    pad.setInputSource(QSharedPointer<QLCInputSource>(new QLCInputSource(0, 1)), VCXYPad::panInputSourceId);
    pad.setInputSource(QSharedPointer<QLCInputSource>(new QLCInputSource(2, 3)), VCXYPad::tiltInputSourceId);
    pad.setInputSource(QSharedPointer<QLCInputSource>(new QLCInputSource(1, 10)), VCXYPad::widthInputSourceId);
    pad.setInputSource(QSharedPointer<QLCInputSource>(new QLCInputSource(3, 8)), VCXYPad::heightInputSourceId);
    QCOMPARE(pad.m_area->position(), QPointF(23, 45));
    QCOMPARE(pad.m_area->position(), QPointF(23, 45));

    VCXYPadFixture fixture1(m_doc);
    fixture1.setHead(GroupHead(11, 0));
    pad.appendFixture(fixture1);

    VCXYPadFixture fixture2(m_doc);
    fixture2.setHead(GroupHead(22, 0));
    pad.appendFixture(fixture2);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    int fixture = 0, position = 0, wstate = 0, appearance = 0;
    int pan = 0, tilt = 0, width = 0, height = 0;

    QVERIFY(pad.saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QCOMPARE(xmlReader.name().toString(), QString("XYPad"));
    QCOMPARE(xmlReader.attributes().value("Caption").toString(), QString("MyPad"));

    while (xmlReader.readNextStartElement())
    {
        if (xmlReader.name().toString() == "Fixture")
        {
            fixture++;
            QVERIFY(xmlReader.attributes().value("ID") == QString("11") ||
                    xmlReader.attributes().value("ID") == QString("22"));
            QVERIFY(xmlReader.attributes().value("Head") == QString("0"));
            xmlReader.skipCurrentElement();
        }
        else if (xmlReader.name().toString() == "Position")
        {
            position++;
            QFAIL("Legacy tag found in saved XML!");
            xmlReader.skipCurrentElement();
        }
        else if (xmlReader.name().toString() == "Pan")
        {
            pan++;
            QCOMPARE(xmlReader.attributes().value("Position").toString(), QString("23"));
            xmlReader.readNextStartElement();
            QCOMPARE(xmlReader.name().toString(), QString("Input"));
            QCOMPARE(xmlReader.attributes().value("Universe").toString(), QString("0"));
            QCOMPARE(xmlReader.attributes().value("Channel").toString(), QString("1"));
            xmlReader.skipCurrentElement();
            xmlReader.skipCurrentElement();
        }
        else if (xmlReader.name().toString() == "Tilt")
        {
            tilt++;
            QCOMPARE(xmlReader.attributes().value("Position").toString(), QString("45"));
            xmlReader.readNextStartElement();
            QCOMPARE(xmlReader.name().toString(), QString("Input"));
            QCOMPARE(xmlReader.attributes().value("Universe").toString(), QString("2"));
            QCOMPARE(xmlReader.attributes().value("Channel").toString(), QString("3"));
            xmlReader.skipCurrentElement();
            xmlReader.skipCurrentElement();
        }
        else if (xmlReader.name().toString() == "Width")
        {
            width++;
            xmlReader.readNextStartElement();
            QCOMPARE(xmlReader.name().toString(), QString("Input"));
            QCOMPARE(xmlReader.attributes().value("Universe").toString(), QString("1"));
            QCOMPARE(xmlReader.attributes().value("Channel").toString(), QString("10"));
            xmlReader.skipCurrentElement();
            xmlReader.skipCurrentElement();
        }
        else if (xmlReader.name().toString() == "Height")
        {
            height++;
            xmlReader.readNextStartElement();
            QCOMPARE(xmlReader.name().toString(), QString("Input"));
            QCOMPARE(xmlReader.attributes().value("Universe").toString(), QString("3"));
            QCOMPARE(xmlReader.attributes().value("Channel").toString(), QString("8"));
            xmlReader.skipCurrentElement();
            xmlReader.skipCurrentElement();
        }
        else if (xmlReader.name().toString() == "WindowState")
        {
            wstate++;
            xmlReader.skipCurrentElement();
        }
        else if (xmlReader.name().toString() == "Appearance")
        {
            appearance++;
            xmlReader.skipCurrentElement();
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(xmlReader.name().toString()).toUtf8().constData());
        }
    }

    QCOMPARE(fixture, 2);
    QCOMPARE(position, 0);
    QCOMPARE(pan, 1);
    QCOMPARE(tilt, 1);
    QCOMPARE(width, 1);
    QCOMPARE(height, 1);
    QCOMPARE(wstate, 1);
    QCOMPARE(appearance, 1);
}

void VCXYPad_Test::modeChange()
{
    //UniverseArray ua(512);
    QWidget w;

    Fixture* fxi = new Fixture(m_doc);
    QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);
    QLCFixtureMode* mode = def->modes().first();
    QVERIFY(mode != NULL);
    fxi->setFixtureDefinition(def, mode);
    m_doc->addFixture(fxi);

    VCXYPad* pad = new VCXYPad(&w, m_doc);
    pad->show();
    w.show();
    pad->resize(QSize(200, 200));

    VCXYPadFixture xy(m_doc);
    xy.setHead(GroupHead(fxi->id(), 0));
    pad->appendFixture(xy);
    QCOMPARE(pad->fixtures().size(), 1);
    QCOMPARE(pad->fixtures()[0].m_xMSB, QLCChannel::invalid());
    QCOMPARE(pad->fixtures()[0].m_xLSB, QLCChannel::invalid());

    m_doc->setMode(Doc::Operate);
    QVERIFY(pad->fixtures()[0].m_xMSB != QLCChannel::invalid());
    QVERIFY(pad->fixtures()[0].m_yMSB != QLCChannel::invalid());
    QCOMPARE(m_doc->masterTimer()->m_dmxSourceList.size(), 1);
    QCOMPARE(m_doc->masterTimer()->m_dmxSourceList[0], pad);
/*
    // FIXME !!
    pad->m_area->setPosition(QPoint(pad->m_area->width(), pad->m_area->height()));
    pad->writeDMX(m_doc->masterTimer(), &ua);
    QCOMPARE(ua.preGMValues()[0], char(255));
    QCOMPARE(ua.preGMValues()[1], char(255));

    pad->m_area->setPosition(QPoint(pad->m_area->width() / 2, pad->m_area->height() / 4));
    pad->writeDMX(m_doc->masterTimer(), &ua);
    QCOMPARE(ua.preGMValues()[0], char(128));
    QCOMPARE(ua.preGMValues()[1], char(64));
*/
    m_doc->setMode(Doc::Design);
    QCOMPARE(pad->fixtures()[0].m_xMSB, QLCChannel::invalid());
    QCOMPARE(pad->fixtures()[0].m_yMSB, QLCChannel::invalid());
    delete pad;
    QCOMPARE(m_doc->masterTimer()->m_dmxSourceList.size(), 0);
}

QTEST_MAIN(VCXYPad_Test)
