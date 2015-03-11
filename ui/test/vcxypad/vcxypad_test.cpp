/*
  Q Light Controller
  vcxypad_test.cpp

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

#include <QFrame>
#include <QtTest>
#include <QtXml>

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
    QVERIFY(m_doc->fixtureDefCache()->load(dir) == true);
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
    QSize size(80, 80);
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

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("XYPad");
    xmldoc.appendChild(root);

    QDomElement pos = xmldoc.createElement("Position");
    pos.setAttribute("X", "10");
    pos.setAttribute("Y", "20");
    root.appendChild(pos);

    QDomElement fxi = xmldoc.createElement("Fixture");
    fxi.setAttribute("ID", "69");
    fxi.setAttribute("Head", "96");
    root.appendChild(fxi);

    QDomElement x = xmldoc.createElement("Axis");
    x.setAttribute("ID", "X");
    x.setAttribute("LowLimit", "0.1");
    x.setAttribute("HighLimit", "0.5");
    x.setAttribute("Reverse", "True");
    fxi.appendChild(x);

    QDomElement y = xmldoc.createElement("Axis");
    y.setAttribute("ID", "Y");
    y.setAttribute("LowLimit", "0.2");
    y.setAttribute("HighLimit", "0.6");
    y.setAttribute("Reverse", "True");
    fxi.appendChild(y);

    QDomElement fxi2 = xmldoc.createElement("Fixture");
    fxi2.setAttribute("ID", "50");
    fxi2.setAttribute("Head", "55");
    root.appendChild(fxi2);

    QDomElement x2 = xmldoc.createElement("Axis");
    x2.setAttribute("ID", "X");
    x2.setAttribute("LowLimit", "0.0");
    x2.setAttribute("HighLimit", "1.0");
    x2.setAttribute("Reverse", "False");
    fxi2.appendChild(x2);

    QDomElement y2 = xmldoc.createElement("Axis");
    y2.setAttribute("ID", "Y");
    y2.setAttribute("LowLimit", "0.0");
    y2.setAttribute("HighLimit", "1.0");
    y2.setAttribute("Reverse", "False");
    fxi2.appendChild(y2);

    QDomElement wstate = xmldoc.createElement("WindowState");
    wstate.setAttribute("Width", "42");
    wstate.setAttribute("Height", "69");
    wstate.setAttribute("X", "3");
    wstate.setAttribute("Y", "4");
    wstate.setAttribute("Visible", "True");
    root.appendChild(wstate);

    QDomElement appearance = xmldoc.createElement("Appearance");
    QFont f(w.font());
    f.setPointSize(f.pointSize() + 3);
    QDomElement font = xmldoc.createElement("Font");
    QDomText fontText = xmldoc.createTextNode(f.toString());
    font.appendChild(fontText);
    appearance.appendChild(font);
    root.appendChild(appearance);

    QDomElement foobar = xmldoc.createElement("Foobar");
    root.appendChild(foobar);

    VCXYPad pad(&w, m_doc);
    QVERIFY(pad.loadXML(&root) == true);
    QCOMPARE(pad.m_fixtures.size(), 2);
    QCOMPARE(pad.pos(), QPoint(3, 4));
    QCOMPARE(pad.size(), QSize(42, 69));
    QCOMPARE(pad.m_area->position(), QPointF(10, 20));

    VCXYPadFixture fixture(m_doc);
    fixture.setHead(GroupHead(69, 96));
    QVERIFY(pad.m_fixtures.contains(fixture) == true);
    fixture.setHead(GroupHead(50, 55));
    QVERIFY(pad.m_fixtures.contains(fixture) == true);

    root.setTagName("YXPad");
    QVERIFY(pad.loadXML(&root) == false);
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
    pad.setInputSource(new QLCInputSource(0, 1), VCXYPad::panInputSourceId);
    pad.setInputSource(new QLCInputSource(2, 3), VCXYPad::tiltInputSourceId);
    QCOMPARE(pad.m_area->position(), QPointF(23, 45));
    QCOMPARE(pad.m_area->position(), QPointF(23, 45));

    VCXYPadFixture fixture1(m_doc);
    fixture1.setHead(GroupHead(11, 0));
    pad.appendFixture(fixture1);

    VCXYPadFixture fixture2(m_doc);
    fixture2.setHead(GroupHead(22, 0));
    pad.appendFixture(fixture2);

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("Root");
    xmldoc.appendChild(root);

    int fixture = 0, position = 0, wstate = 0, appearance = 0, pan = 0, tilt = 0;

    QVERIFY(pad.saveXML(&xmldoc, &root) == true);
    QDomNode node = root.firstChild();
    QCOMPARE(node.toElement().tagName(), QString("XYPad"));
    QCOMPARE(node.toElement().attribute("Caption"), QString("MyPad"));
    node = node.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == "Fixture")
        {
            fixture++;
            QVERIFY(tag.attribute("ID") == QString("11") ||
                    tag.attribute("ID") == QString("22"));
            QVERIFY(tag.attribute("Head") == QString("0"));
            QCOMPARE(tag.childNodes().count(), 2);
        }
        else if (tag.tagName() == "Position")
        {
            position++;
            QFAIL("Legacy tag found in saved XML!");
        }
        else if (tag.tagName() == "Pan")
        {
            pan++;
            QCOMPARE(tag.attribute("Position"), QString("23"));
            QCOMPARE(tag.firstChild().toElement().attribute("Universe"), QString("0"));
            QCOMPARE(tag.firstChild().toElement().attribute("Channel"), QString("1"));
        }
        else if (tag.tagName() == "Tilt")
        {
            tilt++;
            QCOMPARE(tag.attribute("Position"), QString("45"));
            QCOMPARE(tag.firstChild().toElement().attribute("Universe"), QString("2"));
            QCOMPARE(tag.firstChild().toElement().attribute("Channel"), QString("3"));
        }
        else if (tag.tagName() == "WindowState")
        {
            wstate++;
        }
        else if (tag.tagName() == "Appearance")
        {
            appearance++;
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(tag.tagName()).toUtf8().constData());
        }

        node = node.nextSibling();
    }

    QCOMPARE(fixture, 2);
    QCOMPARE(position, 0);
    QCOMPARE(pan, 1);
    QCOMPARE(tilt, 1);
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

    VCXYPad pad(&w, m_doc);
    pad.show();
    w.show();
    pad.resize(QSize(200, 200));

    VCXYPadFixture xy(m_doc);
    xy.setHead(GroupHead(fxi->id(), 0));
    pad.appendFixture(xy);
    QCOMPARE(pad.fixtures().size(), 1);
    QCOMPARE(pad.fixtures()[0].m_xMSB, QLCChannel::invalid());
    QCOMPARE(pad.fixtures()[0].m_xLSB, QLCChannel::invalid());

    m_doc->setMode(Doc::Operate);
    QVERIFY(pad.fixtures()[0].m_xMSB != QLCChannel::invalid());
    QVERIFY(pad.fixtures()[0].m_yMSB != QLCChannel::invalid());
    QCOMPARE(m_doc->masterTimer()->m_dmxSourceList.size(), 1);
    QCOMPARE(m_doc->masterTimer()->m_dmxSourceList[0], &pad);
/*
    // FIXME !!
    pad.m_area->setPosition(QPoint(pad.m_area->width(), pad.m_area->height()));
    pad.writeDMX(m_doc->masterTimer(), &ua);
    QCOMPARE(ua.preGMValues()[0], char(255));
    QCOMPARE(ua.preGMValues()[1], char(255));

    pad.m_area->setPosition(QPoint(pad.m_area->width() / 2, pad.m_area->height() / 4));
    pad.writeDMX(m_doc->masterTimer(), &ua);
    QCOMPARE(ua.preGMValues()[0], char(128));
    QCOMPARE(ua.preGMValues()[1], char(64));
*/
    m_doc->setMode(Doc::Design);
    QCOMPARE(pad.fixtures()[0].m_xMSB, QLCChannel::invalid());
    QCOMPARE(pad.fixtures()[0].m_yMSB, QLCChannel::invalid());
    QCOMPARE(m_doc->masterTimer()->m_dmxSourceList.size(), 0);
}

QTEST_MAIN(VCXYPad_Test)
