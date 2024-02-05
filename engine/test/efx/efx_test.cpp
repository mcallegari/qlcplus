/*
  Q Light Controller Plus - Unit test
  efx_test.cpp

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
#include <QList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#define protected public
#define private public
#include "mastertimer_stub.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "genericfader.h"
#include "efxfixture.h"
#include "qlcchannel.h"
#include "universe.h"
#include "efx_test.h"
#include "function.h"
#include "qlcfile.h"
#include "fixture.h"
#include "scene.h"
#include "doc.h"
#include "efx.h"
#include "bus.h"
#undef private
#undef protected

#include "../common/resource_paths.h"

void EFX_Test::initTestCase()
{
    m_doc = new Doc(this);

    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_doc->fixtureDefCache()->loadMap(dir));
}

void EFX_Test::cleanupTestCase()
{
    delete m_doc;
}

void EFX_Test::init()
{
}

void EFX_Test::cleanup()
{
    m_doc->clearContents();
}

void EFX_Test::initial()
{
    EFX e(m_doc);
    QCOMPARE(e.type(), Function::EFXType);
    QCOMPARE(e.name(), QString("New EFX"));
    QCOMPARE(e.id(), Function::invalidId());

    QCOMPARE(e.algorithm(), EFX::Circle);
    QCOMPARE(e.width(), 127);
    QCOMPARE(e.height(), 127);
    QCOMPARE(e.rotation(), 0);
    QCOMPARE(e.xOffset(), 127);
    QCOMPARE(e.yOffset(), 127);

    QCOMPARE(e.xFrequency(), 2);
    QCOMPARE(e.yFrequency(), 3);
    QVERIFY(!e.isFrequencyEnabled());

    QCOMPARE(e.yPhase(), 0);
    QCOMPARE(e.xPhase(), 90);
    QVERIFY(!e.isPhaseEnabled());

    QCOMPARE(e.fixtures().size(), 0);
    QCOMPARE(e.propagationMode(), EFX::Parallel);

    QCOMPARE(e.m_fadersMap.count(), 0);
    QCOMPARE(e.m_legacyFadeBus, Bus::invalid());
    QCOMPARE(e.m_legacyHoldBus, Bus::invalid());
}

void EFX_Test::algorithmNames()
{
    QStringList list = EFX::algorithmList();
    QCOMPARE(list.size(), 10);
    QVERIFY(list.contains("Circle"));
    QVERIFY(list.contains("Eight"));
    QVERIFY(list.contains("Line"));
    QVERIFY(list.contains("Line2"));
    QVERIFY(list.contains("Diamond"));
    QVERIFY(list.contains("Square"));
    QVERIFY(list.contains("SquareChoppy"));
    QVERIFY(list.contains("SquareTrue"));
    QVERIFY(list.contains("Leaf"));
    QVERIFY(list.contains("Lissajous"));

    EFX e(m_doc);

    /* All EFX's have Circle as the initial algorithm */
    QCOMPARE(e.algorithm(), EFX::Circle);

    e.setAlgorithm(EFX::Eight);
    QCOMPARE(e.algorithm(), EFX::Eight);

    e.setAlgorithm(EFX::Line);
    QCOMPARE(e.algorithm(), EFX::Line);

    e.setAlgorithm(EFX::Diamond);
    QCOMPARE(e.algorithm(), EFX::Diamond);

    e.setAlgorithm(EFX::Square);
    QCOMPARE(e.algorithm(), EFX::Square);

    e.setAlgorithm(EFX::SquareChoppy);
    QCOMPARE(e.algorithm(), EFX::SquareChoppy);

    e.setAlgorithm(EFX::SquareTrue);
    QCOMPARE(e.algorithm(), EFX::SquareTrue);

    e.setAlgorithm(EFX::Leaf);
    QCOMPARE(e.algorithm(), EFX::Leaf);

    e.setAlgorithm(EFX::Lissajous);
    QCOMPARE(e.algorithm(), EFX::Lissajous);

    /* Invalid algorithm results in Circle as a fallback */
    e.setAlgorithm(EFX::Algorithm(31337));
    QCOMPARE(e.algorithm(), EFX::Circle);
}

void EFX_Test::stringToAlgorithm()
{
    QCOMPARE(EFX::stringToAlgorithm("Circle"), EFX::Circle);
    QCOMPARE(EFX::stringToAlgorithm("Eight"), EFX::Eight);
    QCOMPARE(EFX::stringToAlgorithm("Line"), EFX::Line);
    QCOMPARE(EFX::stringToAlgorithm("Line2"), EFX::Line2);
    QCOMPARE(EFX::stringToAlgorithm("Diamond"), EFX::Diamond);
    QCOMPARE(EFX::stringToAlgorithm("Square"), EFX::Square);
    QCOMPARE(EFX::stringToAlgorithm("SquareChoppy"), EFX::SquareChoppy);
    QCOMPARE(EFX::stringToAlgorithm("SquareTrue"), EFX::SquareTrue);
    QCOMPARE(EFX::stringToAlgorithm("Leaf"), EFX::Leaf);
    QCOMPARE(EFX::stringToAlgorithm("Lissajous"), EFX::Lissajous);
    QCOMPARE(EFX::stringToAlgorithm("Foobar"), EFX::Circle);
}

void EFX_Test::width()
{
    EFX e(m_doc);

    e.setWidth(300);
    QCOMPARE(e.width(), 127);

    e.setWidth(0);
    QCOMPARE(e.width(), 0);

    e.setWidth(128);
    QCOMPARE(e.width(), 127);

    e.setWidth(52);
    QCOMPARE(e.width(), 52);

    e.setWidth(-4);
    QCOMPARE(e.width(), 0);
}

void EFX_Test::height()
{
    EFX e(m_doc);

    e.setHeight(300);
    QCOMPARE(e.height(), 127);

    e.setHeight(0);
    QCOMPARE(e.height(), 0);

    e.setHeight(128);
    QCOMPARE(e.height(), 127);

    e.setHeight(12);
    QCOMPARE(e.height(), 12);

    e.setHeight(-4);
    QCOMPARE(e.height(), 0);
}

void EFX_Test::rotation()
{
    EFX e(m_doc);
    QCOMPARE(e.m_cosR, cos(M_PI/180 * 0));
    QCOMPARE(e.m_sinR, sin(M_PI/180 * 0));

    e.setRotation(400);
    QCOMPARE(e.rotation(), 359);
    QCOMPARE(e.m_cosR, cos(M_PI/180 * 359));
    QCOMPARE(e.m_sinR, sin(M_PI/180 * 359));

    e.setRotation(360);
    QCOMPARE(e.rotation(), 359);
    QCOMPARE(e.m_cosR, cos(M_PI/180 * 359));
    QCOMPARE(e.m_sinR, sin(M_PI/180 * 359));

    e.setRotation(0);
    QCOMPARE(e.rotation(), 0);
    QCOMPARE(e.m_cosR, cos(M_PI/180 * 0));
    QCOMPARE(e.m_sinR, sin(M_PI/180 * 0));

    e.setRotation(12);
    QCOMPARE(e.rotation(), 12);
    QCOMPARE(e.m_cosR, cos(M_PI/180 * 12));
    QCOMPARE(e.m_sinR, sin(M_PI/180 * 12));

    e.setRotation(-4);
    QCOMPARE(e.rotation(), 0);
    QCOMPARE(e.m_cosR, cos(M_PI/180 * 0));
    QCOMPARE(e.m_sinR, sin(M_PI/180 * 0));
}

void EFX_Test::xOffset()
{
    EFX e(m_doc);

    e.setXOffset(300);
    QCOMPARE(e.xOffset(), UCHAR_MAX);

    e.setXOffset(0);
    QCOMPARE(e.xOffset(), 0);

    e.setXOffset(256);
    QCOMPARE(e.xOffset(), UCHAR_MAX);

    e.setXOffset(12);
    QCOMPARE(e.xOffset(), 12);

    e.setXOffset(-4);
    QCOMPARE(e.xOffset(), 0);
}

void EFX_Test::yOffset()
{
    EFX e(m_doc);

    e.setYOffset(300);
    QCOMPARE(e.yOffset(), UCHAR_MAX);

    e.setYOffset(0);
    QCOMPARE(e.yOffset(), 0);

    e.setYOffset(256);
    QCOMPARE(e.yOffset(), UCHAR_MAX);

    e.setYOffset(12);
    QCOMPARE(e.yOffset(), 12);

    e.setYOffset(-4);
    QCOMPARE(e.yOffset(), 0);
}

void EFX_Test::xFrequency()
{
    EFX e(m_doc);

    QVERIFY(!e.isFrequencyEnabled());

    e.setXFrequency(60);
    QCOMPARE(e.xFrequency(), 32);

    e.setXFrequency(33);
    QCOMPARE(e.xFrequency(), 32);

    e.setXFrequency(0);
    QCOMPARE(e.xFrequency(), 0);

    e.setXFrequency(3);
    QCOMPARE(e.xFrequency(), 3);

    e.setXFrequency(-4);
    QCOMPARE(e.xFrequency(), 0);

    e.setAlgorithm(EFX::Lissajous);
    QVERIFY(e.isFrequencyEnabled());

    e.setXFrequency(60);
    QCOMPARE(e.xFrequency(), 32);

    e.setXFrequency(33);
    QCOMPARE(e.xFrequency(), 32);

    e.setXFrequency(0);
    QCOMPARE(e.xFrequency(), 0);

    e.setXFrequency(3);
    QCOMPARE(e.xFrequency(), 3);

    e.setXFrequency(-4);
    QCOMPARE(e.xFrequency(), 0);
}

void EFX_Test::yFrequency()
{
    EFX e(m_doc);

    QVERIFY(!e.isFrequencyEnabled());

    e.setYFrequency(60);
    QCOMPARE(e.yFrequency(), 32);

    e.setYFrequency(33);
    QCOMPARE(e.yFrequency(), 32);

    e.setYFrequency(0);
    QCOMPARE(e.yFrequency(), 0);

    e.setYFrequency(3);
    QCOMPARE(e.yFrequency(), 3);

    e.setYFrequency(-4);
    QCOMPARE(e.yFrequency(), 0);

    e.setAlgorithm(EFX::Lissajous);
    QVERIFY(e.isFrequencyEnabled());

    e.setYFrequency(60);
    QCOMPARE(e.yFrequency(), 32);

    e.setYFrequency(33);
    QCOMPARE(e.yFrequency(), 32);

    e.setYFrequency(0);
    QCOMPARE(e.yFrequency(), 0);

    e.setYFrequency(3);
    QCOMPARE(e.yFrequency(), 3);

    e.setYFrequency(-4);
    QCOMPARE(e.yFrequency(), 0);
}

void EFX_Test::xPhase()
{
    EFX e(m_doc);

    QVERIFY(!e.isPhaseEnabled());

    e.setXPhase(400);
    QCOMPARE(e.xPhase(), 359);

    e.setXPhase(360);
    QCOMPARE(e.xPhase(), 359);

    e.setXPhase(0);
    QCOMPARE(e.xPhase(), 0);

    e.setXPhase(359);
    QCOMPARE(e.xPhase(), 359);

    e.setXPhase(46);
    QCOMPARE(e.xPhase(), 46);

    e.setXPhase(-4);
    QCOMPARE(e.xPhase(), 0);

    e.setAlgorithm(EFX::Lissajous);
    QVERIFY(e.isPhaseEnabled());

    e.setXPhase(400);
    QCOMPARE(e.xPhase(), 359);

    e.setXPhase(360);
    QCOMPARE(e.xPhase(), 359);

    e.setXPhase(0);
    QCOMPARE(e.xPhase(), 0);

    e.setXPhase(359);
    QCOMPARE(e.xPhase(), 359);

    e.setXPhase(46);
    QCOMPARE(e.xPhase(), 46);

    e.setXPhase(-4);
    QCOMPARE(e.xPhase(), 0);
}

void EFX_Test::yPhase()
{
    EFX e(m_doc);

    QVERIFY(!e.isPhaseEnabled());

    e.setYPhase(400);
    QCOMPARE(e.yPhase(), 359);

    e.setYPhase(360);
    QCOMPARE(e.yPhase(), 359);

    e.setYPhase(0);
    QCOMPARE(e.yPhase(), 0);

    e.setYPhase(359);
    QCOMPARE(e.yPhase(), 359);

    e.setYPhase(152);
    QCOMPARE(e.yPhase(), 152);

    e.setYPhase(-4);
    QCOMPARE(e.yPhase(), 0);

    e.setAlgorithm(EFX::Lissajous);
    QVERIFY(e.isPhaseEnabled());

    e.setYPhase(400);
    QCOMPARE(e.yPhase(), 359);

    e.setYPhase(360);
    QCOMPARE(e.yPhase(), 359);

    e.setYPhase(0);
    QCOMPARE(e.yPhase(), 0);

    e.setYPhase(359);
    QCOMPARE(e.yPhase(), 359);

    e.setYPhase(152);
    QCOMPARE(e.yPhase(), 152);

    e.setYPhase(-4);
    QCOMPARE(e.yPhase(), 0);
}

void EFX_Test::fixtures()
{
    EFX* e = new EFX(m_doc);
    QCOMPARE(e->fixtures().size(), 0);

    /* Add first fixture */
    EFXFixture* ef1 = new EFXFixture(e);
    ef1->setHead(GroupHead(12, 0));
    QVERIFY(e->addFixture(ef1));
    QCOMPARE(e->fixtures().size(), 1);

    /* Add second fixture */
    EFXFixture* ef2 = new EFXFixture(e);
    ef2->setHead(GroupHead(34, 0));
    QVERIFY(e->addFixture(ef2));
    QCOMPARE(e->fixtures().size(), 2);

    /* Must not be able to add the same fixture twice */
    //QVERIFY(!e->addFixture(ef1));
    //QVERIFY(!e->addFixture(ef2));
    //QCOMPARE(e->fixtures().size(), 2);

    /* Try to remove a non-member fixture */
    EFXFixture* ef3 = new EFXFixture(e);
    ef3->setHead(GroupHead(56, 0));
    QVERIFY(!e->removeFixture(ef3));
    QCOMPARE(e->fixtures().size(), 2);

    /* Add third fixture */
    e->addFixture(ef3);
    QCOMPARE(e->fixtures().size(), 3);
    QCOMPARE(e->fixtures().at(0), ef1);
    QCOMPARE(e->fixtures().at(1), ef2);
    QCOMPARE(e->fixtures().at(2), ef3);
    QCOMPARE(e->fixture(12, 0), ef1);
    QCOMPARE(e->fixture(34, 0), ef2);
    QCOMPARE(e->fixture(56, 0), ef3);
    /* Test a non existent fixture */
    QVERIFY(e->fixture(11, 22) == NULL);

    /* Add fourth fixture */
    EFXFixture* ef4 = new EFXFixture(e);
    ef4->setHead(GroupHead(78, 0));
    e->addFixture(ef4);
    QCOMPARE(e->fixtures().size(), 4);
    QCOMPARE(e->fixtures().at(0), ef1);
    QCOMPARE(e->fixtures().at(1), ef2);
    QCOMPARE(e->fixtures().at(2), ef3);
    QCOMPARE(e->fixtures().at(3), ef4);

    QVERIFY(e->removeFixture(ef4));
    QCOMPARE(e->fixtures().at(0), ef1);
    QCOMPARE(e->fixtures().at(1), ef2);
    QCOMPARE(e->fixtures().at(2), ef3);
    delete ef4;

    /* Raising the first and lowering the last must not succeed */
    QVERIFY(!e->raiseFixture(ef1));
    QVERIFY(!e->lowerFixture(ef3));
    QCOMPARE(e->fixtures().at(0), ef1);
    QCOMPARE(e->fixtures().at(1), ef2);
    QCOMPARE(e->fixtures().at(2), ef3);

    QVERIFY(e->raiseFixture(ef2));
    QCOMPARE(e->fixtures().at(0), ef2);
    QCOMPARE(e->fixtures().at(1), ef1);
    QCOMPARE(e->fixtures().at(2), ef3);
    QVERIFY(!e->raiseFixture(ef2));

    QVERIFY(e->lowerFixture(ef1));
    QCOMPARE(e->fixtures().at(0), ef2);
    QCOMPARE(e->fixtures().at(1), ef3);
    QCOMPARE(e->fixtures().at(2), ef1);
    QVERIFY(!e->lowerFixture(ef1));

    /* Uninteresting fixture is removed */
    e->slotFixtureRemoved(99);
    QCOMPARE(e->fixtures().size(), 3);
    QCOMPARE(e->fixtures().at(0), ef2);
    QCOMPARE(e->fixtures().at(1), ef3);
    QCOMPARE(e->fixtures().at(2), ef1);

    /* Member fixture is removed */
    e->slotFixtureRemoved(34);
    QCOMPARE(e->fixtures().size(), 2);
    QCOMPARE(e->fixtures().at(0), ef3);
    QCOMPARE(e->fixtures().at(1), ef1);

    /* Non-Member fixture is removed */
    e->slotFixtureRemoved(34);
    QCOMPARE(e->fixtures().size(), 2);
    QCOMPARE(e->fixtures().at(0), ef3);
    QCOMPARE(e->fixtures().at(1), ef1);

    /* Member fixture is removed */
    e->slotFixtureRemoved(12);
    QCOMPARE(e->fixtures().size(), 1);
    QCOMPARE(e->fixtures().at(0), ef3);

    /* Member fixture is removed */
    e->slotFixtureRemoved(56);
    QCOMPARE(e->fixtures().size(), 0);

    EFXFixture* ef5 = new EFXFixture(e);
    ef5->setHead(GroupHead(18, 0));
    QVERIFY(e->addFixture(ef5));

    /* Remove by fixture ID and head number */
    QVERIFY(e->removeFixture(18, 0));
    QCOMPARE(e->fixtures().size(), 0);

    QVERIFY(!e->removeFixture(98, 99));

    delete e;
}

void EFX_Test::propagationMode()
{
    EFX e(m_doc);
    QCOMPARE(e.propagationMode(), EFX::Parallel);

    e.setPropagationMode(EFX::Serial);
    QCOMPARE(e.propagationMode(), EFX::Serial);

    e.setPropagationMode(EFX::Asymmetric);
    QCOMPARE(e.propagationMode(), EFX::Asymmetric);

    QCOMPARE(EFX::propagationModeToString(EFX::Serial), QString("Serial"));
    QCOMPARE(EFX::propagationModeToString(EFX::Parallel), QString("Parallel"));
    QCOMPARE(EFX::propagationModeToString(EFX::Asymmetric), QString("Asymmetric"));
    QCOMPARE(EFX::propagationModeToString(EFX::PropagationMode(7)), QString("Parallel"));

    QCOMPARE(EFX::stringToPropagationMode("Serial"), EFX::Serial);
    QCOMPARE(EFX::stringToPropagationMode("Parallel"), EFX::Parallel);
    QCOMPARE(EFX::stringToPropagationMode("Asymmetric"), EFX::Asymmetric);
    QCOMPARE(EFX::stringToPropagationMode("Foobar"), EFX::Parallel);
}

void EFX_Test::previewCircle()
{
    EFX e(m_doc);

    QPolygonF poly;
    e.preview(poly);
    QCOMPARE(poly.size(), 128);

    QCOMPARE(poly[0].toPoint(), QPoint(127,254));
    QCOMPARE(poly[1].toPoint(), QPoint(121,254));
    QCOMPARE(poly[2].toPoint(), QPoint(115,253));
    QCOMPARE(poly[3].toPoint(), QPoint(108,253));
    QCOMPARE(poly[4].toPoint(), QPoint(102,252));
    QCOMPARE(poly[5].toPoint(), QPoint(96,250));
    QCOMPARE(poly[6].toPoint(), QPoint(90,249));
    QCOMPARE(poly[7].toPoint(), QPoint(84,247));
    QCOMPARE(poly[8].toPoint(), QPoint(78,244));
    QCOMPARE(poly[9].toPoint(), QPoint(73,242));
    QCOMPARE(poly[10].toPoint(), QPoint(67,239));
    QCOMPARE(poly[11].toPoint(), QPoint(62,236));
    QCOMPARE(poly[12].toPoint(), QPoint(56,233));
    QCOMPARE(poly[13].toPoint(), QPoint(51,229));
    QCOMPARE(poly[14].toPoint(), QPoint(46,225));
    QCOMPARE(poly[15].toPoint(), QPoint(42,221));
    QCOMPARE(poly[16].toPoint(), QPoint(37,217));
    QCOMPARE(poly[17].toPoint(), QPoint(33,212));
    QCOMPARE(poly[18].toPoint(), QPoint(29,208));
    QCOMPARE(poly[19].toPoint(), QPoint(25,203));
    QCOMPARE(poly[20].toPoint(), QPoint(21,198));
    QCOMPARE(poly[21].toPoint(), QPoint(18,192));
    QCOMPARE(poly[22].toPoint(), QPoint(15,187));
    QCOMPARE(poly[23].toPoint(), QPoint(12,181));
    QCOMPARE(poly[24].toPoint(), QPoint(10,176));
    QCOMPARE(poly[25].toPoint(), QPoint(7,170));
    QCOMPARE(poly[26].toPoint(), QPoint(5,164));
    QCOMPARE(poly[27].toPoint(), QPoint(4,158));
    QCOMPARE(poly[28].toPoint(), QPoint(2,152));
    QCOMPARE(poly[29].toPoint(), QPoint(1,146));
    QCOMPARE(poly[30].toPoint(), QPoint(1,139));
    QCOMPARE(poly[31].toPoint(), QPoint(0,133));
    QCOMPARE(poly[32].toPoint(), QPoint(0,127));
    QCOMPARE(poly[33].toPoint(), QPoint(0,121));
    QCOMPARE(poly[34].toPoint(), QPoint(1,115));
    QCOMPARE(poly[35].toPoint(), QPoint(1,108));
    QCOMPARE(poly[36].toPoint(), QPoint(2,102));
    QCOMPARE(poly[37].toPoint(), QPoint(4,96));
    QCOMPARE(poly[38].toPoint(), QPoint(5,90));
    QCOMPARE(poly[39].toPoint(), QPoint(7,84));
    QCOMPARE(poly[40].toPoint(), QPoint(10,78));
    QCOMPARE(poly[41].toPoint(), QPoint(12,73));
    QCOMPARE(poly[42].toPoint(), QPoint(15,67));
    QCOMPARE(poly[43].toPoint(), QPoint(18,62));
    QCOMPARE(poly[44].toPoint(), QPoint(21,56));
    QCOMPARE(poly[45].toPoint(), QPoint(25,51));
    QCOMPARE(poly[46].toPoint(), QPoint(29,46));
    QCOMPARE(poly[47].toPoint(), QPoint(33,42));
    QCOMPARE(poly[48].toPoint(), QPoint(37,37));
    QCOMPARE(poly[49].toPoint(), QPoint(42,33));
    QCOMPARE(poly[50].toPoint(), QPoint(46,29));
    QCOMPARE(poly[51].toPoint(), QPoint(51,25));
    QCOMPARE(poly[52].toPoint(), QPoint(56,21));
    QCOMPARE(poly[53].toPoint(), QPoint(62,18));
    QCOMPARE(poly[54].toPoint(), QPoint(67,15));
    QCOMPARE(poly[55].toPoint(), QPoint(73,12));
    QCOMPARE(poly[56].toPoint(), QPoint(78,10));
    QCOMPARE(poly[57].toPoint(), QPoint(84,7));
    QCOMPARE(poly[58].toPoint(), QPoint(90,5));
    QCOMPARE(poly[59].toPoint(), QPoint(96,4));
    QCOMPARE(poly[60].toPoint(), QPoint(102,2));
    QCOMPARE(poly[61].toPoint(), QPoint(108,1));
    QCOMPARE(poly[62].toPoint(), QPoint(115,1));
    QCOMPARE(poly[63].toPoint(), QPoint(121,0));
    QCOMPARE(poly[64].toPoint(), QPoint(127,0));
    QCOMPARE(poly[65].toPoint(), QPoint(133,0));
    QCOMPARE(poly[66].toPoint(), QPoint(139,1));
    QCOMPARE(poly[67].toPoint(), QPoint(146,1));
    QCOMPARE(poly[68].toPoint(), QPoint(152,2));
    QCOMPARE(poly[69].toPoint(), QPoint(158,4));
    QCOMPARE(poly[70].toPoint(), QPoint(164,5));
    QCOMPARE(poly[71].toPoint(), QPoint(170,7));
    QCOMPARE(poly[72].toPoint(), QPoint(176,10));
    QCOMPARE(poly[73].toPoint(), QPoint(181,12));
    QCOMPARE(poly[74].toPoint(), QPoint(187,15));
    QCOMPARE(poly[75].toPoint(), QPoint(192,18));
    QCOMPARE(poly[76].toPoint(), QPoint(198,21));
    QCOMPARE(poly[77].toPoint(), QPoint(203,25));
    QCOMPARE(poly[78].toPoint(), QPoint(208,29));
    QCOMPARE(poly[79].toPoint(), QPoint(212,33));
    QCOMPARE(poly[80].toPoint(), QPoint(217,37));
    QCOMPARE(poly[81].toPoint(), QPoint(221,42));
    QCOMPARE(poly[82].toPoint(), QPoint(225,46));
    QCOMPARE(poly[83].toPoint(), QPoint(229,51));
    QCOMPARE(poly[84].toPoint(), QPoint(233,56));
    QCOMPARE(poly[85].toPoint(), QPoint(236,62));
    QCOMPARE(poly[86].toPoint(), QPoint(239,67));
    QCOMPARE(poly[87].toPoint(), QPoint(242,73));
    QCOMPARE(poly[88].toPoint(), QPoint(244,78));
    QCOMPARE(poly[89].toPoint(), QPoint(247,84));
    QCOMPARE(poly[90].toPoint(), QPoint(249,90));
    QCOMPARE(poly[91].toPoint(), QPoint(250,96));
    QCOMPARE(poly[92].toPoint(), QPoint(252,102));
    QCOMPARE(poly[93].toPoint(), QPoint(253,108));
    QCOMPARE(poly[94].toPoint(), QPoint(253,115));
    QCOMPARE(poly[95].toPoint(), QPoint(254,121));
    QCOMPARE(poly[96].toPoint(), QPoint(254,127));
    QCOMPARE(poly[97].toPoint(), QPoint(254,133));
    QCOMPARE(poly[98].toPoint(), QPoint(253,139));
    QCOMPARE(poly[99].toPoint(), QPoint(253,146));
    QCOMPARE(poly[100].toPoint(), QPoint(252,152));
    QCOMPARE(poly[101].toPoint(), QPoint(250,158));
    QCOMPARE(poly[102].toPoint(), QPoint(249,164));
    QCOMPARE(poly[103].toPoint(), QPoint(247,170));
    QCOMPARE(poly[104].toPoint(), QPoint(244,176));
    QCOMPARE(poly[105].toPoint(), QPoint(242,181));
    QCOMPARE(poly[106].toPoint(), QPoint(239,187));
    QCOMPARE(poly[107].toPoint(), QPoint(236,192));
    QCOMPARE(poly[108].toPoint(), QPoint(233,198));
    QCOMPARE(poly[109].toPoint(), QPoint(229,203));
    QCOMPARE(poly[110].toPoint(), QPoint(225,208));
    QCOMPARE(poly[111].toPoint(), QPoint(221,212));
    QCOMPARE(poly[112].toPoint(), QPoint(217,217));
    QCOMPARE(poly[113].toPoint(), QPoint(212,221));
    QCOMPARE(poly[114].toPoint(), QPoint(208,225));
    QCOMPARE(poly[115].toPoint(), QPoint(203,229));
    QCOMPARE(poly[116].toPoint(), QPoint(198,233));
    QCOMPARE(poly[117].toPoint(), QPoint(192,236));
    QCOMPARE(poly[118].toPoint(), QPoint(187,239));
    QCOMPARE(poly[119].toPoint(), QPoint(181,242));
    QCOMPARE(poly[120].toPoint(), QPoint(176,244));
    QCOMPARE(poly[121].toPoint(), QPoint(170,247));
    QCOMPARE(poly[122].toPoint(), QPoint(164,249));
    QCOMPARE(poly[123].toPoint(), QPoint(158,250));
    QCOMPARE(poly[124].toPoint(), QPoint(152,252));
    QCOMPARE(poly[125].toPoint(), QPoint(146,253));
    QCOMPARE(poly[126].toPoint(), QPoint(139,253));
    QCOMPARE(poly[127].toPoint(), QPoint(133,254));
}

void EFX_Test::previewEight()
{
    EFX e(m_doc);
    e.setAlgorithm(EFX::Eight);

    QPolygonF poly;
    e.preview(poly);
    QCOMPARE(poly.size(), 128);

    QCOMPARE(poly[0].toPoint(), QPoint(127,254));
    QCOMPARE(poly[1].toPoint(), QPoint(115,254));
    QCOMPARE(poly[2].toPoint(), QPoint(102,253));
    QCOMPARE(poly[3].toPoint(), QPoint(90,253));
    QCOMPARE(poly[4].toPoint(), QPoint(78,252));
    QCOMPARE(poly[5].toPoint(), QPoint(67,250));
    QCOMPARE(poly[6].toPoint(), QPoint(56,249));
    QCOMPARE(poly[7].toPoint(), QPoint(46,247));
    QCOMPARE(poly[8].toPoint(), QPoint(37,244));
    QCOMPARE(poly[9].toPoint(), QPoint(29,242));
    QCOMPARE(poly[10].toPoint(), QPoint(21,239));
    QCOMPARE(poly[11].toPoint(), QPoint(15,236));
    QCOMPARE(poly[12].toPoint(), QPoint(10,233));
    QCOMPARE(poly[13].toPoint(), QPoint(5,229));
    QCOMPARE(poly[14].toPoint(), QPoint(2,225));
    QCOMPARE(poly[15].toPoint(), QPoint(1,221));
    QCOMPARE(poly[16].toPoint(), QPoint(0,217));
    QCOMPARE(poly[17].toPoint(), QPoint(1,212));
    QCOMPARE(poly[18].toPoint(), QPoint(2,208));
    QCOMPARE(poly[19].toPoint(), QPoint(5,203));
    QCOMPARE(poly[20].toPoint(), QPoint(10,198));
    QCOMPARE(poly[21].toPoint(), QPoint(15,192));
    QCOMPARE(poly[22].toPoint(), QPoint(21,187));
    QCOMPARE(poly[23].toPoint(), QPoint(29,181));
    QCOMPARE(poly[24].toPoint(), QPoint(37,176));
    QCOMPARE(poly[25].toPoint(), QPoint(46,170));
    QCOMPARE(poly[26].toPoint(), QPoint(56,164));
    QCOMPARE(poly[27].toPoint(), QPoint(67,158));
    QCOMPARE(poly[28].toPoint(), QPoint(78,152));
    QCOMPARE(poly[29].toPoint(), QPoint(90,146));
    QCOMPARE(poly[30].toPoint(), QPoint(102,139));
    QCOMPARE(poly[31].toPoint(), QPoint(115,133));
    QCOMPARE(poly[32].toPoint(), QPoint(127,127));
    QCOMPARE(poly[33].toPoint(), QPoint(139,121));
    QCOMPARE(poly[34].toPoint(), QPoint(152,115));
    QCOMPARE(poly[35].toPoint(), QPoint(164,108));
    QCOMPARE(poly[36].toPoint(), QPoint(176,102));
    QCOMPARE(poly[37].toPoint(), QPoint(187,96));
    QCOMPARE(poly[38].toPoint(), QPoint(198,90));
    QCOMPARE(poly[39].toPoint(), QPoint(208,84));
    QCOMPARE(poly[40].toPoint(), QPoint(217,78));
    QCOMPARE(poly[41].toPoint(), QPoint(225,73));
    QCOMPARE(poly[42].toPoint(), QPoint(233,67));
    QCOMPARE(poly[43].toPoint(), QPoint(239,62));
    QCOMPARE(poly[44].toPoint(), QPoint(244,56));
    QCOMPARE(poly[45].toPoint(), QPoint(249,51));
    QCOMPARE(poly[46].toPoint(), QPoint(252,46));
    QCOMPARE(poly[47].toPoint(), QPoint(253,42));
    QCOMPARE(poly[48].toPoint(), QPoint(254,37));
    QCOMPARE(poly[49].toPoint(), QPoint(253,33));
    QCOMPARE(poly[50].toPoint(), QPoint(252,29));
    QCOMPARE(poly[51].toPoint(), QPoint(249,25));
    QCOMPARE(poly[52].toPoint(), QPoint(244,21));
    QCOMPARE(poly[53].toPoint(), QPoint(239,18));
    QCOMPARE(poly[54].toPoint(), QPoint(233,15));
    QCOMPARE(poly[55].toPoint(), QPoint(225,12));
    QCOMPARE(poly[56].toPoint(), QPoint(217,10));
    QCOMPARE(poly[57].toPoint(), QPoint(208,7));
    QCOMPARE(poly[58].toPoint(), QPoint(198,5));
    QCOMPARE(poly[59].toPoint(), QPoint(187,4));
    QCOMPARE(poly[60].toPoint(), QPoint(176,2));
    QCOMPARE(poly[61].toPoint(), QPoint(164,1));
    QCOMPARE(poly[62].toPoint(), QPoint(152,1));
    QCOMPARE(poly[63].toPoint(), QPoint(139,0));
    QCOMPARE(poly[64].toPoint(), QPoint(127,0));
    QCOMPARE(poly[65].toPoint(), QPoint(115,0));
    QCOMPARE(poly[66].toPoint(), QPoint(102,1));
    QCOMPARE(poly[67].toPoint(), QPoint(90,1));
    QCOMPARE(poly[68].toPoint(), QPoint(78,2));
    QCOMPARE(poly[69].toPoint(), QPoint(67,4));
    QCOMPARE(poly[70].toPoint(), QPoint(56,5));
    QCOMPARE(poly[71].toPoint(), QPoint(46,7));
    QCOMPARE(poly[72].toPoint(), QPoint(37,10));
    QCOMPARE(poly[73].toPoint(), QPoint(29,12));
    QCOMPARE(poly[74].toPoint(), QPoint(21,15));
    QCOMPARE(poly[75].toPoint(), QPoint(15,18));
    QCOMPARE(poly[76].toPoint(), QPoint(10,21));
    QCOMPARE(poly[77].toPoint(), QPoint(5,25));
    QCOMPARE(poly[78].toPoint(), QPoint(2,29));
    QCOMPARE(poly[79].toPoint(), QPoint(1,33));
    QCOMPARE(poly[80].toPoint(), QPoint(0,37));
    QCOMPARE(poly[81].toPoint(), QPoint(1,42));
    QCOMPARE(poly[82].toPoint(), QPoint(2,46));
    QCOMPARE(poly[83].toPoint(), QPoint(5,51));
    QCOMPARE(poly[84].toPoint(), QPoint(10,56));
    QCOMPARE(poly[85].toPoint(), QPoint(15,62));
    QCOMPARE(poly[86].toPoint(), QPoint(21,67));
    QCOMPARE(poly[87].toPoint(), QPoint(29,73));
    QCOMPARE(poly[88].toPoint(), QPoint(37,78));
    QCOMPARE(poly[89].toPoint(), QPoint(46,84));
    QCOMPARE(poly[90].toPoint(), QPoint(56,90));
    QCOMPARE(poly[91].toPoint(), QPoint(67,96));
    QCOMPARE(poly[92].toPoint(), QPoint(78,102));
    QCOMPARE(poly[93].toPoint(), QPoint(90,108));
    QCOMPARE(poly[94].toPoint(), QPoint(102,115));
    QCOMPARE(poly[95].toPoint(), QPoint(115,121));
    QCOMPARE(poly[96].toPoint(), QPoint(127,127));
    QCOMPARE(poly[97].toPoint(), QPoint(139,133));
    QCOMPARE(poly[98].toPoint(), QPoint(152,139));
    QCOMPARE(poly[99].toPoint(), QPoint(164,146));
    QCOMPARE(poly[100].toPoint(), QPoint(176,152));
    QCOMPARE(poly[101].toPoint(), QPoint(187,158));
    QCOMPARE(poly[102].toPoint(), QPoint(198,164));
    QCOMPARE(poly[103].toPoint(), QPoint(208,170));
    QCOMPARE(poly[104].toPoint(), QPoint(217,176));
    QCOMPARE(poly[105].toPoint(), QPoint(225,181));
    QCOMPARE(poly[106].toPoint(), QPoint(233,187));
    QCOMPARE(poly[107].toPoint(), QPoint(239,192));
    QCOMPARE(poly[108].toPoint(), QPoint(244,198));
    QCOMPARE(poly[109].toPoint(), QPoint(249,203));
    QCOMPARE(poly[110].toPoint(), QPoint(252,208));
    QCOMPARE(poly[111].toPoint(), QPoint(253,212));
    QCOMPARE(poly[112].toPoint(), QPoint(254,217));
    QCOMPARE(poly[113].toPoint(), QPoint(253,221));
    QCOMPARE(poly[114].toPoint(), QPoint(252,225));
    QCOMPARE(poly[115].toPoint(), QPoint(249,229));
    QCOMPARE(poly[116].toPoint(), QPoint(244,233));
    QCOMPARE(poly[117].toPoint(), QPoint(239,236));
    QCOMPARE(poly[118].toPoint(), QPoint(233,239));
    QCOMPARE(poly[119].toPoint(), QPoint(225,242));
    QCOMPARE(poly[120].toPoint(), QPoint(217,244));
    QCOMPARE(poly[121].toPoint(), QPoint(208,247));
    QCOMPARE(poly[122].toPoint(), QPoint(198,249));
    QCOMPARE(poly[123].toPoint(), QPoint(187,250));
    QCOMPARE(poly[124].toPoint(), QPoint(176,252));
    QCOMPARE(poly[125].toPoint(), QPoint(164,253));
    QCOMPARE(poly[126].toPoint(), QPoint(152,253));
    QCOMPARE(poly[127].toPoint(), QPoint(139,254));
}

void EFX_Test::previewLine()
{
    EFX e(m_doc);
    e.setAlgorithm(EFX::Line);

    QPolygonF poly;
    e.preview(poly);
    QCOMPARE(poly.size(), 128);

    QCOMPARE(poly[0].toPoint(), QPoint(254,254));
    QCOMPARE(poly[1].toPoint(), QPoint(254,254));
    QCOMPARE(poly[2].toPoint(), QPoint(253,253));
    QCOMPARE(poly[3].toPoint(), QPoint(253,253));
    QCOMPARE(poly[4].toPoint(), QPoint(252,252));
    QCOMPARE(poly[5].toPoint(), QPoint(250,250));
    QCOMPARE(poly[6].toPoint(), QPoint(249,249));
    QCOMPARE(poly[7].toPoint(), QPoint(247,247));
    QCOMPARE(poly[8].toPoint(), QPoint(244,244));
    QCOMPARE(poly[9].toPoint(), QPoint(242,242));
    QCOMPARE(poly[10].toPoint(), QPoint(239,239));
    QCOMPARE(poly[11].toPoint(), QPoint(236,236));
    QCOMPARE(poly[12].toPoint(), QPoint(233,233));
    QCOMPARE(poly[13].toPoint(), QPoint(229,229));
    QCOMPARE(poly[14].toPoint(), QPoint(225,225));
    QCOMPARE(poly[15].toPoint(), QPoint(221,221));
    QCOMPARE(poly[16].toPoint(), QPoint(217,217));
    QCOMPARE(poly[17].toPoint(), QPoint(212,212));
    QCOMPARE(poly[18].toPoint(), QPoint(208,208));
    QCOMPARE(poly[19].toPoint(), QPoint(203,203));
    QCOMPARE(poly[20].toPoint(), QPoint(198,198));
    QCOMPARE(poly[21].toPoint(), QPoint(192,192));
    QCOMPARE(poly[22].toPoint(), QPoint(187,187));
    QCOMPARE(poly[23].toPoint(), QPoint(181,181));
    QCOMPARE(poly[24].toPoint(), QPoint(176,176));
    QCOMPARE(poly[25].toPoint(), QPoint(170,170));
    QCOMPARE(poly[26].toPoint(), QPoint(164,164));
    QCOMPARE(poly[27].toPoint(), QPoint(158,158));
    QCOMPARE(poly[28].toPoint(), QPoint(152,152));
    QCOMPARE(poly[29].toPoint(), QPoint(146,146));
    QCOMPARE(poly[30].toPoint(), QPoint(139,139));
    QCOMPARE(poly[31].toPoint(), QPoint(133,133));
    QCOMPARE(poly[32].toPoint(), QPoint(127,127));
    QCOMPARE(poly[33].toPoint(), QPoint(121,121));
    QCOMPARE(poly[34].toPoint(), QPoint(115,115));
    QCOMPARE(poly[35].toPoint(), QPoint(108,108));
    QCOMPARE(poly[36].toPoint(), QPoint(102,102));
    QCOMPARE(poly[37].toPoint(), QPoint(96,96));
    QCOMPARE(poly[38].toPoint(), QPoint(90,90));
    QCOMPARE(poly[39].toPoint(), QPoint(84,84));
    QCOMPARE(poly[40].toPoint(), QPoint(78,78));
    QCOMPARE(poly[41].toPoint(), QPoint(73,73));
    QCOMPARE(poly[42].toPoint(), QPoint(67,67));
    QCOMPARE(poly[43].toPoint(), QPoint(62,62));
    QCOMPARE(poly[44].toPoint(), QPoint(56,56));
    QCOMPARE(poly[45].toPoint(), QPoint(51,51));
    QCOMPARE(poly[46].toPoint(), QPoint(46,46));
    QCOMPARE(poly[47].toPoint(), QPoint(42,42));
    QCOMPARE(poly[48].toPoint(), QPoint(37,37));
    QCOMPARE(poly[49].toPoint(), QPoint(33,33));
    QCOMPARE(poly[50].toPoint(), QPoint(29,29));
    QCOMPARE(poly[51].toPoint(), QPoint(25,25));
    QCOMPARE(poly[52].toPoint(), QPoint(21,21));
    QCOMPARE(poly[53].toPoint(), QPoint(18,18));
    QCOMPARE(poly[54].toPoint(), QPoint(15,15));
    QCOMPARE(poly[55].toPoint(), QPoint(12,12));
    QCOMPARE(poly[56].toPoint(), QPoint(10,10));
    QCOMPARE(poly[57].toPoint(), QPoint(7,7));
    QCOMPARE(poly[58].toPoint(), QPoint(5,5));
    QCOMPARE(poly[59].toPoint(), QPoint(4,4));
    QCOMPARE(poly[60].toPoint(), QPoint(2,2));
    QCOMPARE(poly[61].toPoint(), QPoint(1,1));
    QCOMPARE(poly[62].toPoint(), QPoint(1,1));
    QCOMPARE(poly[63].toPoint(), QPoint(0,0));
    QCOMPARE(poly[64].toPoint(), QPoint(0,0));
    QCOMPARE(poly[65].toPoint(), QPoint(0,0));
    QCOMPARE(poly[66].toPoint(), QPoint(1,1));
    QCOMPARE(poly[67].toPoint(), QPoint(1,1));
    QCOMPARE(poly[68].toPoint(), QPoint(2,2));
    QCOMPARE(poly[69].toPoint(), QPoint(4,4));
    QCOMPARE(poly[70].toPoint(), QPoint(5,5));
    QCOMPARE(poly[71].toPoint(), QPoint(7,7));
    QCOMPARE(poly[72].toPoint(), QPoint(10,10));
    QCOMPARE(poly[73].toPoint(), QPoint(12,12));
    QCOMPARE(poly[74].toPoint(), QPoint(15,15));
    QCOMPARE(poly[75].toPoint(), QPoint(18,18));
    QCOMPARE(poly[76].toPoint(), QPoint(21,21));
    QCOMPARE(poly[77].toPoint(), QPoint(25,25));
    QCOMPARE(poly[78].toPoint(), QPoint(29,29));
    QCOMPARE(poly[79].toPoint(), QPoint(33,33));
    QCOMPARE(poly[80].toPoint(), QPoint(37,37));
    QCOMPARE(poly[81].toPoint(), QPoint(42,42));
    QCOMPARE(poly[82].toPoint(), QPoint(46,46));
    QCOMPARE(poly[83].toPoint(), QPoint(51,51));
    QCOMPARE(poly[84].toPoint(), QPoint(56,56));
    QCOMPARE(poly[85].toPoint(), QPoint(62,62));
    QCOMPARE(poly[86].toPoint(), QPoint(67,67));
    QCOMPARE(poly[87].toPoint(), QPoint(73,73));
    QCOMPARE(poly[88].toPoint(), QPoint(78,78));
    QCOMPARE(poly[89].toPoint(), QPoint(84,84));
    QCOMPARE(poly[90].toPoint(), QPoint(90,90));
    QCOMPARE(poly[91].toPoint(), QPoint(96,96));
    QCOMPARE(poly[92].toPoint(), QPoint(102,102));
    QCOMPARE(poly[93].toPoint(), QPoint(108,108));
    QCOMPARE(poly[94].toPoint(), QPoint(115,115));
    QCOMPARE(poly[95].toPoint(), QPoint(121,121));
    QCOMPARE(poly[96].toPoint(), QPoint(127,127));
    QCOMPARE(poly[97].toPoint(), QPoint(133,133));
    QCOMPARE(poly[98].toPoint(), QPoint(139,139));
    QCOMPARE(poly[99].toPoint(), QPoint(146,146));
    QCOMPARE(poly[100].toPoint(), QPoint(152,152));
    QCOMPARE(poly[101].toPoint(), QPoint(158,158));
    QCOMPARE(poly[102].toPoint(), QPoint(164,164));
    QCOMPARE(poly[103].toPoint(), QPoint(170,170));
    QCOMPARE(poly[104].toPoint(), QPoint(176,176));
    QCOMPARE(poly[105].toPoint(), QPoint(181,181));
    QCOMPARE(poly[106].toPoint(), QPoint(187,187));
    QCOMPARE(poly[107].toPoint(), QPoint(192,192));
    QCOMPARE(poly[108].toPoint(), QPoint(198,198));
    QCOMPARE(poly[109].toPoint(), QPoint(203,203));
    QCOMPARE(poly[110].toPoint(), QPoint(208,208));
    QCOMPARE(poly[111].toPoint(), QPoint(212,212));
    QCOMPARE(poly[112].toPoint(), QPoint(217,217));
    QCOMPARE(poly[113].toPoint(), QPoint(221,221));
    QCOMPARE(poly[114].toPoint(), QPoint(225,225));
    QCOMPARE(poly[115].toPoint(), QPoint(229,229));
    QCOMPARE(poly[116].toPoint(), QPoint(233,233));
    QCOMPARE(poly[117].toPoint(), QPoint(236,236));
    QCOMPARE(poly[118].toPoint(), QPoint(239,239));
    QCOMPARE(poly[119].toPoint(), QPoint(242,242));
    QCOMPARE(poly[120].toPoint(), QPoint(244,244));
    QCOMPARE(poly[121].toPoint(), QPoint(247,247));
    QCOMPARE(poly[122].toPoint(), QPoint(249,249));
    QCOMPARE(poly[123].toPoint(), QPoint(250,250));
    QCOMPARE(poly[124].toPoint(), QPoint(252,252));
    QCOMPARE(poly[125].toPoint(), QPoint(253,253));
    QCOMPARE(poly[126].toPoint(), QPoint(253,253));
    QCOMPARE(poly[127].toPoint(), QPoint(254,254));
}

void EFX_Test::previewLine2()
{
    EFX e(m_doc);
    e.setAlgorithm(EFX::Line2);

    QPolygonF poly;
    e.preview(poly);
    QCOMPARE(poly.size(), 128);

    QCOMPARE(poly[0].toPoint(), QPoint(0,0));
    QCOMPARE(poly[1].toPoint(), QPoint(2,2));
    QCOMPARE(poly[2].toPoint(), QPoint(4,4));
    QCOMPARE(poly[3].toPoint(), QPoint(6,6));
    QCOMPARE(poly[4].toPoint(), QPoint(8,8));
    QCOMPARE(poly[5].toPoint(), QPoint(10,10));
    QCOMPARE(poly[6].toPoint(), QPoint(12,12));
    QCOMPARE(poly[7].toPoint(), QPoint(14,14));
    QCOMPARE(poly[8].toPoint(), QPoint(16,16));
    QCOMPARE(poly[9].toPoint(), QPoint(18,18));
    QCOMPARE(poly[10].toPoint(), QPoint(20,20));
    QCOMPARE(poly[11].toPoint(), QPoint(22,22));
    QCOMPARE(poly[12].toPoint(), QPoint(24,24));
    QCOMPARE(poly[13].toPoint(), QPoint(26,26));
    QCOMPARE(poly[14].toPoint(), QPoint(28,28));
    QCOMPARE(poly[15].toPoint(), QPoint(30,30));
    QCOMPARE(poly[16].toPoint(), QPoint(32,32));
    QCOMPARE(poly[17].toPoint(), QPoint(34,34));
    QCOMPARE(poly[18].toPoint(), QPoint(36,36));
    QCOMPARE(poly[19].toPoint(), QPoint(38,38));
    QCOMPARE(poly[20].toPoint(), QPoint(40,40));
    QCOMPARE(poly[21].toPoint(), QPoint(42,42));
    QCOMPARE(poly[22].toPoint(), QPoint(44,44));
    QCOMPARE(poly[23].toPoint(), QPoint(46,46));
    QCOMPARE(poly[24].toPoint(), QPoint(48,48));
    QCOMPARE(poly[25].toPoint(), QPoint(50,50));
    QCOMPARE(poly[26].toPoint(), QPoint(52,52));
    QCOMPARE(poly[27].toPoint(), QPoint(54,54));
    QCOMPARE(poly[28].toPoint(), QPoint(56,56));
    QCOMPARE(poly[29].toPoint(), QPoint(58,58));
    QCOMPARE(poly[30].toPoint(), QPoint(60,60));
    QCOMPARE(poly[31].toPoint(), QPoint(62,62));
    QCOMPARE(poly[32].toPoint(), QPoint(64,64));
    QCOMPARE(poly[33].toPoint(), QPoint(65,65));
    QCOMPARE(poly[34].toPoint(), QPoint(67,67));
    QCOMPARE(poly[35].toPoint(), QPoint(69,69));
    QCOMPARE(poly[36].toPoint(), QPoint(71,71));
    QCOMPARE(poly[37].toPoint(), QPoint(73,73));
    QCOMPARE(poly[38].toPoint(), QPoint(75,75));
    QCOMPARE(poly[39].toPoint(), QPoint(77,77));
    QCOMPARE(poly[40].toPoint(), QPoint(79,79));
    QCOMPARE(poly[41].toPoint(), QPoint(81,81));
    QCOMPARE(poly[42].toPoint(), QPoint(83,83));
    QCOMPARE(poly[43].toPoint(), QPoint(85,85));
    QCOMPARE(poly[44].toPoint(), QPoint(87,87));
    QCOMPARE(poly[45].toPoint(), QPoint(89,89));
    QCOMPARE(poly[46].toPoint(), QPoint(91,91));
    QCOMPARE(poly[47].toPoint(), QPoint(93,93));
    QCOMPARE(poly[48].toPoint(), QPoint(95,95));
    QCOMPARE(poly[49].toPoint(), QPoint(97,97));
    QCOMPARE(poly[50].toPoint(), QPoint(99,99));
    QCOMPARE(poly[51].toPoint(), QPoint(101,101));
    QCOMPARE(poly[52].toPoint(), QPoint(103,103));
    QCOMPARE(poly[53].toPoint(), QPoint(105,105));
    QCOMPARE(poly[54].toPoint(), QPoint(107,107));
    QCOMPARE(poly[55].toPoint(), QPoint(109,109));
    QCOMPARE(poly[56].toPoint(), QPoint(111,111));
    QCOMPARE(poly[57].toPoint(), QPoint(113,113));
    QCOMPARE(poly[58].toPoint(), QPoint(115,115));
    QCOMPARE(poly[59].toPoint(), QPoint(117,117));
    QCOMPARE(poly[60].toPoint(), QPoint(119,119));
    QCOMPARE(poly[61].toPoint(), QPoint(121,121));
    QCOMPARE(poly[62].toPoint(), QPoint(123,123));
    QCOMPARE(poly[63].toPoint(), QPoint(125,125));
    QCOMPARE(poly[64].toPoint(), QPoint(127,127));
    QCOMPARE(poly[65].toPoint(), QPoint(129,129));
    QCOMPARE(poly[66].toPoint(), QPoint(131,131));
    QCOMPARE(poly[67].toPoint(), QPoint(133,133));
    QCOMPARE(poly[68].toPoint(), QPoint(135,135));
    QCOMPARE(poly[69].toPoint(), QPoint(137,137));
    QCOMPARE(poly[70].toPoint(), QPoint(139,139));
    QCOMPARE(poly[71].toPoint(), QPoint(141,141));
    QCOMPARE(poly[72].toPoint(), QPoint(143,143));
    QCOMPARE(poly[73].toPoint(), QPoint(145,145));
    QCOMPARE(poly[74].toPoint(), QPoint(147,147));
    QCOMPARE(poly[75].toPoint(), QPoint(149,149));
    QCOMPARE(poly[76].toPoint(), QPoint(151,151));
    QCOMPARE(poly[77].toPoint(), QPoint(153,153));
    QCOMPARE(poly[78].toPoint(), QPoint(155,155));
    QCOMPARE(poly[79].toPoint(), QPoint(157,157));
    QCOMPARE(poly[80].toPoint(), QPoint(159,159));
    QCOMPARE(poly[81].toPoint(), QPoint(161,161));
    QCOMPARE(poly[82].toPoint(), QPoint(163,163));
    QCOMPARE(poly[83].toPoint(), QPoint(165,165));
    QCOMPARE(poly[84].toPoint(), QPoint(167,167));
    QCOMPARE(poly[85].toPoint(), QPoint(169,169));
    QCOMPARE(poly[86].toPoint(), QPoint(171,171));
    QCOMPARE(poly[87].toPoint(), QPoint(173,173));
    QCOMPARE(poly[88].toPoint(), QPoint(175,175));
    QCOMPARE(poly[89].toPoint(), QPoint(177,177));
    QCOMPARE(poly[90].toPoint(), QPoint(179,179));
    QCOMPARE(poly[91].toPoint(), QPoint(181,181));
    QCOMPARE(poly[92].toPoint(), QPoint(183,183));
    QCOMPARE(poly[93].toPoint(), QPoint(185,185));
    QCOMPARE(poly[94].toPoint(), QPoint(187,187));
    QCOMPARE(poly[95].toPoint(), QPoint(189,189));
    QCOMPARE(poly[96].toPoint(), QPoint(190,190));
    QCOMPARE(poly[97].toPoint(), QPoint(192,192));
    QCOMPARE(poly[98].toPoint(), QPoint(194,194));
    QCOMPARE(poly[99].toPoint(), QPoint(196,196));
    QCOMPARE(poly[100].toPoint(), QPoint(198,198));
    QCOMPARE(poly[101].toPoint(), QPoint(200,200));
    QCOMPARE(poly[102].toPoint(), QPoint(202,202));
    QCOMPARE(poly[103].toPoint(), QPoint(204,204));
    QCOMPARE(poly[104].toPoint(), QPoint(206,206));
    QCOMPARE(poly[105].toPoint(), QPoint(208,208));
    QCOMPARE(poly[106].toPoint(), QPoint(210,210));
    QCOMPARE(poly[107].toPoint(), QPoint(212,212));
    QCOMPARE(poly[108].toPoint(), QPoint(214,214));
    QCOMPARE(poly[109].toPoint(), QPoint(216,216));
    QCOMPARE(poly[110].toPoint(), QPoint(218,218));
    QCOMPARE(poly[111].toPoint(), QPoint(220,220));
    QCOMPARE(poly[112].toPoint(), QPoint(222,222));
    QCOMPARE(poly[113].toPoint(), QPoint(224,224));
    QCOMPARE(poly[114].toPoint(), QPoint(226,226));
    QCOMPARE(poly[115].toPoint(), QPoint(228,228));
    QCOMPARE(poly[116].toPoint(), QPoint(230,230));
    QCOMPARE(poly[117].toPoint(), QPoint(232,232));
    QCOMPARE(poly[118].toPoint(), QPoint(234,234));
    QCOMPARE(poly[119].toPoint(), QPoint(236,236));
    QCOMPARE(poly[120].toPoint(), QPoint(238,238));
    QCOMPARE(poly[121].toPoint(), QPoint(240,240));
    QCOMPARE(poly[122].toPoint(), QPoint(242,242));
    QCOMPARE(poly[123].toPoint(), QPoint(244,244));
    QCOMPARE(poly[124].toPoint(), QPoint(246,246));
    QCOMPARE(poly[125].toPoint(), QPoint(248,248));
    QCOMPARE(poly[126].toPoint(), QPoint(250,250));
    QCOMPARE(poly[127].toPoint(), QPoint(252,252));
}

void EFX_Test::previewDiamond()
{
    EFX e(m_doc);
    e.setAlgorithm(EFX::Diamond);

    QPolygonF poly;
    e.preview(poly);
    QCOMPARE(poly.size(), 128);

    QCOMPARE(poly[0].toPoint(), QPoint(127,254));
    QCOMPARE(poly[1].toPoint(), QPoint(127,254));
    QCOMPARE(poly[2].toPoint(), QPoint(127,252));
    QCOMPARE(poly[3].toPoint(), QPoint(127,250));
    QCOMPARE(poly[4].toPoint(), QPoint(128,247));
    QCOMPARE(poly[5].toPoint(), QPoint(129,243));
    QCOMPARE(poly[6].toPoint(), QPoint(130,238));
    QCOMPARE(poly[7].toPoint(), QPoint(132,233));
    QCOMPARE(poly[8].toPoint(), QPoint(134,227));
    QCOMPARE(poly[9].toPoint(), QPoint(137,221));
    QCOMPARE(poly[10].toPoint(), QPoint(140,214));
    QCOMPARE(poly[11].toPoint(), QPoint(144,207));
    QCOMPARE(poly[12].toPoint(), QPoint(149,200));
    QCOMPARE(poly[13].toPoint(), QPoint(154,193));
    QCOMPARE(poly[14].toPoint(), QPoint(159,186));
    QCOMPARE(poly[15].toPoint(), QPoint(165,179));
    QCOMPARE(poly[16].toPoint(), QPoint(172,172));
    QCOMPARE(poly[17].toPoint(), QPoint(179,165));
    QCOMPARE(poly[18].toPoint(), QPoint(186,159));
    QCOMPARE(poly[19].toPoint(), QPoint(193,154));
    QCOMPARE(poly[20].toPoint(), QPoint(200,149));
    QCOMPARE(poly[21].toPoint(), QPoint(207,144));
    QCOMPARE(poly[22].toPoint(), QPoint(214,140));
    QCOMPARE(poly[23].toPoint(), QPoint(221,137));
    QCOMPARE(poly[24].toPoint(), QPoint(227,134));
    QCOMPARE(poly[25].toPoint(), QPoint(233,132));
    QCOMPARE(poly[26].toPoint(), QPoint(238,130));
    QCOMPARE(poly[27].toPoint(), QPoint(243,129));
    QCOMPARE(poly[28].toPoint(), QPoint(247,128));
    QCOMPARE(poly[29].toPoint(), QPoint(250,127));
    QCOMPARE(poly[30].toPoint(), QPoint(252,127));
    QCOMPARE(poly[31].toPoint(), QPoint(254,127));
    QCOMPARE(poly[32].toPoint(), QPoint(254,127));
    QCOMPARE(poly[33].toPoint(), QPoint(254,127));
    QCOMPARE(poly[34].toPoint(), QPoint(252,127));
    QCOMPARE(poly[35].toPoint(), QPoint(250,127));
    QCOMPARE(poly[36].toPoint(), QPoint(247,126));
    QCOMPARE(poly[37].toPoint(), QPoint(243,125));
    QCOMPARE(poly[38].toPoint(), QPoint(238,124));
    QCOMPARE(poly[39].toPoint(), QPoint(233,122));
    QCOMPARE(poly[40].toPoint(), QPoint(227,120));
    QCOMPARE(poly[41].toPoint(), QPoint(221,117));
    QCOMPARE(poly[42].toPoint(), QPoint(214,114));
    QCOMPARE(poly[43].toPoint(), QPoint(207,110));
    QCOMPARE(poly[44].toPoint(), QPoint(200,105));
    QCOMPARE(poly[45].toPoint(), QPoint(193,100));
    QCOMPARE(poly[46].toPoint(), QPoint(186,95));
    QCOMPARE(poly[47].toPoint(), QPoint(179,89));
    QCOMPARE(poly[48].toPoint(), QPoint(172,82));
    QCOMPARE(poly[49].toPoint(), QPoint(165,75));
    QCOMPARE(poly[50].toPoint(), QPoint(159,68));
    QCOMPARE(poly[51].toPoint(), QPoint(154,61));
    QCOMPARE(poly[52].toPoint(), QPoint(149,54));
    QCOMPARE(poly[53].toPoint(), QPoint(144,47));
    QCOMPARE(poly[54].toPoint(), QPoint(140,40));
    QCOMPARE(poly[55].toPoint(), QPoint(137,33));
    QCOMPARE(poly[56].toPoint(), QPoint(134,27));
    QCOMPARE(poly[57].toPoint(), QPoint(132,21));
    QCOMPARE(poly[58].toPoint(), QPoint(130,16));
    QCOMPARE(poly[59].toPoint(), QPoint(129,11));
    QCOMPARE(poly[60].toPoint(), QPoint(128,7));
    QCOMPARE(poly[61].toPoint(), QPoint(127,4));
    QCOMPARE(poly[62].toPoint(), QPoint(127,2));
    QCOMPARE(poly[63].toPoint(), QPoint(127,0));
    QCOMPARE(poly[64].toPoint(), QPoint(127,0));
    QCOMPARE(poly[65].toPoint(), QPoint(127,0));
    QCOMPARE(poly[66].toPoint(), QPoint(127,2));
    QCOMPARE(poly[67].toPoint(), QPoint(127,4));
    QCOMPARE(poly[68].toPoint(), QPoint(126,7));
    QCOMPARE(poly[69].toPoint(), QPoint(125,11));
    QCOMPARE(poly[70].toPoint(), QPoint(124,16));
    QCOMPARE(poly[71].toPoint(), QPoint(122,21));
    QCOMPARE(poly[72].toPoint(), QPoint(120,27));
    QCOMPARE(poly[73].toPoint(), QPoint(117,33));
    QCOMPARE(poly[74].toPoint(), QPoint(114,40));
    QCOMPARE(poly[75].toPoint(), QPoint(110,47));
    QCOMPARE(poly[76].toPoint(), QPoint(105,54));
    QCOMPARE(poly[77].toPoint(), QPoint(100,61));
    QCOMPARE(poly[78].toPoint(), QPoint(95,68));
    QCOMPARE(poly[79].toPoint(), QPoint(89,75));
    QCOMPARE(poly[80].toPoint(), QPoint(82,82));
    QCOMPARE(poly[81].toPoint(), QPoint(75,89));
    QCOMPARE(poly[82].toPoint(), QPoint(68,95));
    QCOMPARE(poly[83].toPoint(), QPoint(61,100));
    QCOMPARE(poly[84].toPoint(), QPoint(54,105));
    QCOMPARE(poly[85].toPoint(), QPoint(47,110));
    QCOMPARE(poly[86].toPoint(), QPoint(40,114));
    QCOMPARE(poly[87].toPoint(), QPoint(33,117));
    QCOMPARE(poly[88].toPoint(), QPoint(27,120));
    QCOMPARE(poly[89].toPoint(), QPoint(21,122));
    QCOMPARE(poly[90].toPoint(), QPoint(16,124));
    QCOMPARE(poly[91].toPoint(), QPoint(11,125));
    QCOMPARE(poly[92].toPoint(), QPoint(7,126));
    QCOMPARE(poly[93].toPoint(), QPoint(4,127));
    QCOMPARE(poly[94].toPoint(), QPoint(2,127));
    QCOMPARE(poly[95].toPoint(), QPoint(0,127));
    QCOMPARE(poly[96].toPoint(), QPoint(0,127));
    QCOMPARE(poly[97].toPoint(), QPoint(0,127));
    QCOMPARE(poly[98].toPoint(), QPoint(2,127));
    QCOMPARE(poly[99].toPoint(), QPoint(4,127));
    QCOMPARE(poly[100].toPoint(), QPoint(7,128));
    QCOMPARE(poly[101].toPoint(), QPoint(11,129));
    QCOMPARE(poly[102].toPoint(), QPoint(16,130));
    QCOMPARE(poly[103].toPoint(), QPoint(21,132));
    QCOMPARE(poly[104].toPoint(), QPoint(27,134));
    QCOMPARE(poly[105].toPoint(), QPoint(33,137));
    QCOMPARE(poly[106].toPoint(), QPoint(40,140));
    QCOMPARE(poly[107].toPoint(), QPoint(47,144));
    QCOMPARE(poly[108].toPoint(), QPoint(54,149));
    QCOMPARE(poly[109].toPoint(), QPoint(61,154));
    QCOMPARE(poly[110].toPoint(), QPoint(68,159));
    QCOMPARE(poly[111].toPoint(), QPoint(75,165));
    QCOMPARE(poly[112].toPoint(), QPoint(82,172));
    QCOMPARE(poly[113].toPoint(), QPoint(89,179));
    QCOMPARE(poly[114].toPoint(), QPoint(95,186));
    QCOMPARE(poly[115].toPoint(), QPoint(100,193));
    QCOMPARE(poly[116].toPoint(), QPoint(105,200));
    QCOMPARE(poly[117].toPoint(), QPoint(110,207));
    QCOMPARE(poly[118].toPoint(), QPoint(114,214));
    QCOMPARE(poly[119].toPoint(), QPoint(117,221));
    QCOMPARE(poly[120].toPoint(), QPoint(120,227));
    QCOMPARE(poly[121].toPoint(), QPoint(122,233));
    QCOMPARE(poly[122].toPoint(), QPoint(124,238));
    QCOMPARE(poly[123].toPoint(), QPoint(125,243));
    QCOMPARE(poly[124].toPoint(), QPoint(126,247));
    QCOMPARE(poly[125].toPoint(), QPoint(127,250));
    QCOMPARE(poly[126].toPoint(), QPoint(127,252));
    QCOMPARE(poly[127].toPoint(), QPoint(127,254));
}

void EFX_Test::previewSquare()
{
    EFX e(m_doc);
    e.setAlgorithm(EFX::Square);

    QPolygonF poly;
    e.preview(poly);
    QCOMPARE(poly.size(), 128);

    QCOMPARE(poly[0].toPoint(), QPoint(0,254));
    QCOMPARE(poly[1].toPoint(), QPoint(8,254));
    QCOMPARE(poly[2].toPoint(), QPoint(16,254));
    QCOMPARE(poly[3].toPoint(), QPoint(24,254));
    QCOMPARE(poly[4].toPoint(), QPoint(32,254));
    QCOMPARE(poly[5].toPoint(), QPoint(40,254));
    QCOMPARE(poly[6].toPoint(), QPoint(48,254));
    QCOMPARE(poly[7].toPoint(), QPoint(56,254));
    QCOMPARE(poly[8].toPoint(), QPoint(64,254));
    QCOMPARE(poly[9].toPoint(), QPoint(71,254));
    QCOMPARE(poly[10].toPoint(), QPoint(79,254));
    QCOMPARE(poly[11].toPoint(), QPoint(87,254));
    QCOMPARE(poly[12].toPoint(), QPoint(95,254));
    QCOMPARE(poly[13].toPoint(), QPoint(103,254));
    QCOMPARE(poly[14].toPoint(), QPoint(111,254));
    QCOMPARE(poly[15].toPoint(), QPoint(119,254));
    QCOMPARE(poly[16].toPoint(), QPoint(127,254));
    QCOMPARE(poly[17].toPoint(), QPoint(135,254));
    QCOMPARE(poly[18].toPoint(), QPoint(143,254));
    QCOMPARE(poly[19].toPoint(), QPoint(151,254));
    QCOMPARE(poly[20].toPoint(), QPoint(159,254));
    QCOMPARE(poly[21].toPoint(), QPoint(167,254));
    QCOMPARE(poly[22].toPoint(), QPoint(175,254));
    QCOMPARE(poly[23].toPoint(), QPoint(183,254));
    QCOMPARE(poly[24].toPoint(), QPoint(191,254));
    QCOMPARE(poly[25].toPoint(), QPoint(198,254));
    QCOMPARE(poly[26].toPoint(), QPoint(206,254));
    QCOMPARE(poly[27].toPoint(), QPoint(214,254));
    QCOMPARE(poly[28].toPoint(), QPoint(222,254));
    QCOMPARE(poly[29].toPoint(), QPoint(230,254));
    QCOMPARE(poly[30].toPoint(), QPoint(238,254));
    QCOMPARE(poly[31].toPoint(), QPoint(246,254));
    QCOMPARE(poly[32].toPoint(), QPoint(254,254));
    QCOMPARE(poly[33].toPoint(), QPoint(254,246));
    QCOMPARE(poly[34].toPoint(), QPoint(254,238));
    QCOMPARE(poly[35].toPoint(), QPoint(254,230));
    QCOMPARE(poly[36].toPoint(), QPoint(254,222));
    QCOMPARE(poly[37].toPoint(), QPoint(254,214));
    QCOMPARE(poly[38].toPoint(), QPoint(254,206));
    QCOMPARE(poly[39].toPoint(), QPoint(254,198));
    QCOMPARE(poly[40].toPoint(), QPoint(254,190));
    QCOMPARE(poly[41].toPoint(), QPoint(254,183));
    QCOMPARE(poly[42].toPoint(), QPoint(254,175));
    QCOMPARE(poly[43].toPoint(), QPoint(254,167));
    QCOMPARE(poly[44].toPoint(), QPoint(254,159));
    QCOMPARE(poly[45].toPoint(), QPoint(254,151));
    QCOMPARE(poly[46].toPoint(), QPoint(254,143));
    QCOMPARE(poly[47].toPoint(), QPoint(254,135));
    QCOMPARE(poly[48].toPoint(), QPoint(254,127));
    QCOMPARE(poly[49].toPoint(), QPoint(254,119));
    QCOMPARE(poly[50].toPoint(), QPoint(254,111));
    QCOMPARE(poly[51].toPoint(), QPoint(254,103));
    QCOMPARE(poly[52].toPoint(), QPoint(254,95));
    QCOMPARE(poly[53].toPoint(), QPoint(254,87));
    QCOMPARE(poly[54].toPoint(), QPoint(254,79));
    QCOMPARE(poly[55].toPoint(), QPoint(254,71));
    QCOMPARE(poly[56].toPoint(), QPoint(254,64));
    QCOMPARE(poly[57].toPoint(), QPoint(254,56));
    QCOMPARE(poly[58].toPoint(), QPoint(254,48));
    QCOMPARE(poly[59].toPoint(), QPoint(254,40));
    QCOMPARE(poly[60].toPoint(), QPoint(254,32));
    QCOMPARE(poly[61].toPoint(), QPoint(254,24));
    QCOMPARE(poly[62].toPoint(), QPoint(254,16));
    QCOMPARE(poly[63].toPoint(), QPoint(254,8));
    QCOMPARE(poly[64].toPoint(), QPoint(254,0));
    QCOMPARE(poly[65].toPoint(), QPoint(246,0));
    QCOMPARE(poly[66].toPoint(), QPoint(238,0));
    QCOMPARE(poly[67].toPoint(), QPoint(230,0));
    QCOMPARE(poly[68].toPoint(), QPoint(222,0));
    QCOMPARE(poly[69].toPoint(), QPoint(214,0));
    QCOMPARE(poly[70].toPoint(), QPoint(206,0));
    QCOMPARE(poly[71].toPoint(), QPoint(198,0));
    QCOMPARE(poly[72].toPoint(), QPoint(191,0));
    QCOMPARE(poly[73].toPoint(), QPoint(183,0));
    QCOMPARE(poly[74].toPoint(), QPoint(175,0));
    QCOMPARE(poly[75].toPoint(), QPoint(167,0));
    QCOMPARE(poly[76].toPoint(), QPoint(159,0));
    QCOMPARE(poly[77].toPoint(), QPoint(151,0));
    QCOMPARE(poly[78].toPoint(), QPoint(143,0));
    QCOMPARE(poly[79].toPoint(), QPoint(135,0));
    QCOMPARE(poly[80].toPoint(), QPoint(127,0));
    QCOMPARE(poly[81].toPoint(), QPoint(119,0));
    QCOMPARE(poly[82].toPoint(), QPoint(111,0));
    QCOMPARE(poly[83].toPoint(), QPoint(103,0));
    QCOMPARE(poly[84].toPoint(), QPoint(95,0));
    QCOMPARE(poly[85].toPoint(), QPoint(87,0));
    QCOMPARE(poly[86].toPoint(), QPoint(79,0));
    QCOMPARE(poly[87].toPoint(), QPoint(71,0));
    QCOMPARE(poly[88].toPoint(), QPoint(64,0));
    QCOMPARE(poly[89].toPoint(), QPoint(56,0));
    QCOMPARE(poly[90].toPoint(), QPoint(48,0));
    QCOMPARE(poly[91].toPoint(), QPoint(40,0));
    QCOMPARE(poly[92].toPoint(), QPoint(32,0));
    QCOMPARE(poly[93].toPoint(), QPoint(24,0));
    QCOMPARE(poly[94].toPoint(), QPoint(16,0));
    QCOMPARE(poly[95].toPoint(), QPoint(8,0));
    QCOMPARE(poly[96].toPoint(), QPoint(0,0));
    QCOMPARE(poly[97].toPoint(), QPoint(0,8));
    QCOMPARE(poly[98].toPoint(), QPoint(0,16));
    QCOMPARE(poly[99].toPoint(), QPoint(0,24));
    QCOMPARE(poly[100].toPoint(), QPoint(0,32));
    QCOMPARE(poly[101].toPoint(), QPoint(0,40));
    QCOMPARE(poly[102].toPoint(), QPoint(0,48));
    QCOMPARE(poly[103].toPoint(), QPoint(0,56));
    QCOMPARE(poly[104].toPoint(), QPoint(0,63));
    QCOMPARE(poly[105].toPoint(), QPoint(0,71));
    QCOMPARE(poly[106].toPoint(), QPoint(0,79));
    QCOMPARE(poly[107].toPoint(), QPoint(0,87));
    QCOMPARE(poly[108].toPoint(), QPoint(0,95));
    QCOMPARE(poly[109].toPoint(), QPoint(0,103));
    QCOMPARE(poly[110].toPoint(), QPoint(0,111));
    QCOMPARE(poly[111].toPoint(), QPoint(0,119));
    QCOMPARE(poly[112].toPoint(), QPoint(0,127));
    QCOMPARE(poly[113].toPoint(), QPoint(0,135));
    QCOMPARE(poly[114].toPoint(), QPoint(0,143));
    QCOMPARE(poly[115].toPoint(), QPoint(0,151));
    QCOMPARE(poly[116].toPoint(), QPoint(0,159));
    QCOMPARE(poly[117].toPoint(), QPoint(0,167));
    QCOMPARE(poly[118].toPoint(), QPoint(0,175));
    QCOMPARE(poly[119].toPoint(), QPoint(0,183));
    QCOMPARE(poly[120].toPoint(), QPoint(0,191));
    QCOMPARE(poly[121].toPoint(), QPoint(0,198));
    QCOMPARE(poly[122].toPoint(), QPoint(0,206));
    QCOMPARE(poly[123].toPoint(), QPoint(0,214));
    QCOMPARE(poly[124].toPoint(), QPoint(0,222));
    QCOMPARE(poly[125].toPoint(), QPoint(0,230));
    QCOMPARE(poly[126].toPoint(), QPoint(0,238));
    QCOMPARE(poly[127].toPoint(), QPoint(0,246));
}

void EFX_Test::previewSquareChoppy()
{
    EFX e(m_doc);
    e.setAlgorithm(EFX::SquareChoppy);

    QPolygonF poly;
    e.preview(poly);
    QCOMPARE(poly.size(), 128);

    QCOMPARE(poly[0].toPoint(), QPoint(254,127));
    QCOMPARE(poly[1].toPoint(), QPoint(254,127));
    QCOMPARE(poly[2].toPoint(), QPoint(254,127));
    QCOMPARE(poly[3].toPoint(), QPoint(254,127));
    QCOMPARE(poly[4].toPoint(), QPoint(254,127));
    QCOMPARE(poly[5].toPoint(), QPoint(254,127));
    QCOMPARE(poly[6].toPoint(), QPoint(254,127));
    QCOMPARE(poly[7].toPoint(), QPoint(254,127));
    QCOMPARE(poly[8].toPoint(), QPoint(254,127));
    QCOMPARE(poly[9].toPoint(), QPoint(254,127));
    QCOMPARE(poly[10].toPoint(), QPoint(254,127));
    QCOMPARE(poly[11].toPoint(), QPoint(254,254));
    QCOMPARE(poly[12].toPoint(), QPoint(254,254));
    QCOMPARE(poly[13].toPoint(), QPoint(254,254));
    QCOMPARE(poly[14].toPoint(), QPoint(254,254));
    QCOMPARE(poly[15].toPoint(), QPoint(254,254));
    QCOMPARE(poly[16].toPoint(), QPoint(254,254));
    QCOMPARE(poly[17].toPoint(), QPoint(254,254));
    QCOMPARE(poly[18].toPoint(), QPoint(254,254));
    QCOMPARE(poly[19].toPoint(), QPoint(254,254));
    QCOMPARE(poly[20].toPoint(), QPoint(254,254));
    QCOMPARE(poly[21].toPoint(), QPoint(254,254));
    QCOMPARE(poly[22].toPoint(), QPoint(127,254));
    QCOMPARE(poly[23].toPoint(), QPoint(127,254));
    QCOMPARE(poly[24].toPoint(), QPoint(127,254));
    QCOMPARE(poly[25].toPoint(), QPoint(127,254));
    QCOMPARE(poly[26].toPoint(), QPoint(127,254));
    QCOMPARE(poly[27].toPoint(), QPoint(127,254));
    QCOMPARE(poly[28].toPoint(), QPoint(127,254));
    QCOMPARE(poly[29].toPoint(), QPoint(127,254));
    QCOMPARE(poly[30].toPoint(), QPoint(127,254));
    QCOMPARE(poly[31].toPoint(), QPoint(127,254));
    QCOMPARE(poly[32].toPoint(), QPoint(127,254));
    QCOMPARE(poly[33].toPoint(), QPoint(127,254));
    QCOMPARE(poly[34].toPoint(), QPoint(127,254));
    QCOMPARE(poly[35].toPoint(), QPoint(127,254));
    QCOMPARE(poly[36].toPoint(), QPoint(127,254));
    QCOMPARE(poly[37].toPoint(), QPoint(127,254));
    QCOMPARE(poly[38].toPoint(), QPoint(127,254));
    QCOMPARE(poly[39].toPoint(), QPoint(127,254));
    QCOMPARE(poly[40].toPoint(), QPoint(127,254));
    QCOMPARE(poly[41].toPoint(), QPoint(127,254));
    QCOMPARE(poly[42].toPoint(), QPoint(127,254));
    QCOMPARE(poly[43].toPoint(), QPoint(0,254));
    QCOMPARE(poly[44].toPoint(), QPoint(0,254));
    QCOMPARE(poly[45].toPoint(), QPoint(0,254));
    QCOMPARE(poly[46].toPoint(), QPoint(0,254));
    QCOMPARE(poly[47].toPoint(), QPoint(0,254));
    QCOMPARE(poly[48].toPoint(), QPoint(0,254));
    QCOMPARE(poly[49].toPoint(), QPoint(0,254));
    QCOMPARE(poly[50].toPoint(), QPoint(0,254));
    QCOMPARE(poly[51].toPoint(), QPoint(0,254));
    QCOMPARE(poly[52].toPoint(), QPoint(0,254));
    QCOMPARE(poly[53].toPoint(), QPoint(0,254));
    QCOMPARE(poly[54].toPoint(), QPoint(0,127));
    QCOMPARE(poly[55].toPoint(), QPoint(0,127));
    QCOMPARE(poly[56].toPoint(), QPoint(0,127));
    QCOMPARE(poly[57].toPoint(), QPoint(0,127));
    QCOMPARE(poly[58].toPoint(), QPoint(0,127));
    QCOMPARE(poly[59].toPoint(), QPoint(0,127));
    QCOMPARE(poly[60].toPoint(), QPoint(0,127));
    QCOMPARE(poly[61].toPoint(), QPoint(0,127));
    QCOMPARE(poly[62].toPoint(), QPoint(0,127));
    QCOMPARE(poly[63].toPoint(), QPoint(0,127));
    QCOMPARE(poly[64].toPoint(), QPoint(0,127));
    QCOMPARE(poly[65].toPoint(), QPoint(0,127));
    QCOMPARE(poly[66].toPoint(), QPoint(0,127));
    QCOMPARE(poly[67].toPoint(), QPoint(0,127));
    QCOMPARE(poly[68].toPoint(), QPoint(0,127));
    QCOMPARE(poly[69].toPoint(), QPoint(0,127));
    QCOMPARE(poly[70].toPoint(), QPoint(0,127));
    QCOMPARE(poly[71].toPoint(), QPoint(0,127));
    QCOMPARE(poly[72].toPoint(), QPoint(0,127));
    QCOMPARE(poly[73].toPoint(), QPoint(0,127));
    QCOMPARE(poly[74].toPoint(), QPoint(0,127));
    QCOMPARE(poly[75].toPoint(), QPoint(0,0));
    QCOMPARE(poly[76].toPoint(), QPoint(0,0));
    QCOMPARE(poly[77].toPoint(), QPoint(0,0));
    QCOMPARE(poly[78].toPoint(), QPoint(0,0));
    QCOMPARE(poly[79].toPoint(), QPoint(0,0));
    QCOMPARE(poly[80].toPoint(), QPoint(0,0));
    QCOMPARE(poly[81].toPoint(), QPoint(0,0));
    QCOMPARE(poly[82].toPoint(), QPoint(0,0));
    QCOMPARE(poly[83].toPoint(), QPoint(0,0));
    QCOMPARE(poly[84].toPoint(), QPoint(0,0));
    QCOMPARE(poly[85].toPoint(), QPoint(0,0));
    QCOMPARE(poly[86].toPoint(), QPoint(127,0));
    QCOMPARE(poly[87].toPoint(), QPoint(127,0));
    QCOMPARE(poly[88].toPoint(), QPoint(127,0));
    QCOMPARE(poly[89].toPoint(), QPoint(127,0));
    QCOMPARE(poly[90].toPoint(), QPoint(127,0));
    QCOMPARE(poly[91].toPoint(), QPoint(127,0));
    QCOMPARE(poly[92].toPoint(), QPoint(127,0));
    QCOMPARE(poly[93].toPoint(), QPoint(127,0));
    QCOMPARE(poly[94].toPoint(), QPoint(127,0));
    QCOMPARE(poly[95].toPoint(), QPoint(127,0));
    QCOMPARE(poly[96].toPoint(), QPoint(127,0));
    QCOMPARE(poly[97].toPoint(), QPoint(127,0));
    QCOMPARE(poly[98].toPoint(), QPoint(127,0));
    QCOMPARE(poly[99].toPoint(), QPoint(127,0));
    QCOMPARE(poly[100].toPoint(), QPoint(127,0));
    QCOMPARE(poly[101].toPoint(), QPoint(127,0));
    QCOMPARE(poly[102].toPoint(), QPoint(127,0));
    QCOMPARE(poly[103].toPoint(), QPoint(127,0));
    QCOMPARE(poly[104].toPoint(), QPoint(127,0));
    QCOMPARE(poly[105].toPoint(), QPoint(127,0));
    QCOMPARE(poly[106].toPoint(), QPoint(127,0));
    QCOMPARE(poly[107].toPoint(), QPoint(254,0));
    QCOMPARE(poly[108].toPoint(), QPoint(254,0));
    QCOMPARE(poly[109].toPoint(), QPoint(254,0));
    QCOMPARE(poly[110].toPoint(), QPoint(254,0));
    QCOMPARE(poly[111].toPoint(), QPoint(254,0));
    QCOMPARE(poly[112].toPoint(), QPoint(254,0));
    QCOMPARE(poly[113].toPoint(), QPoint(254,0));
    QCOMPARE(poly[114].toPoint(), QPoint(254,0));
    QCOMPARE(poly[115].toPoint(), QPoint(254,0));
    QCOMPARE(poly[116].toPoint(), QPoint(254,0));
    QCOMPARE(poly[117].toPoint(), QPoint(254,0));
    QCOMPARE(poly[118].toPoint(), QPoint(254,127));
    QCOMPARE(poly[119].toPoint(), QPoint(254,127));
    QCOMPARE(poly[120].toPoint(), QPoint(254,127));
    QCOMPARE(poly[121].toPoint(), QPoint(254,127));
    QCOMPARE(poly[122].toPoint(), QPoint(254,127));
    QCOMPARE(poly[123].toPoint(), QPoint(254,127));
    QCOMPARE(poly[124].toPoint(), QPoint(254,127));
    QCOMPARE(poly[125].toPoint(), QPoint(254,127));
    QCOMPARE(poly[126].toPoint(), QPoint(254,127));
    QCOMPARE(poly[127].toPoint(), QPoint(254,127));
}

void EFX_Test::previewSquareTrue()
{
    EFX e(m_doc);
    e.setAlgorithm(EFX::SquareTrue);

    QPolygonF poly;
    e.preview(poly);
    QCOMPARE(poly.size(), 128);

    QCOMPARE(poly[0].toPoint(), QPoint(254,254));
    QCOMPARE(poly[1].toPoint(), QPoint(254,254));
    QCOMPARE(poly[2].toPoint(), QPoint(254,254));
    QCOMPARE(poly[3].toPoint(), QPoint(254,254));
    QCOMPARE(poly[4].toPoint(), QPoint(254,254));
    QCOMPARE(poly[5].toPoint(), QPoint(254,254));
    QCOMPARE(poly[6].toPoint(), QPoint(254,254));
    QCOMPARE(poly[7].toPoint(), QPoint(254,254));
    QCOMPARE(poly[8].toPoint(), QPoint(254,254));
    QCOMPARE(poly[9].toPoint(), QPoint(254,254));
    QCOMPARE(poly[10].toPoint(), QPoint(254,254));
    QCOMPARE(poly[11].toPoint(), QPoint(254,254));
    QCOMPARE(poly[12].toPoint(), QPoint(254,254));
    QCOMPARE(poly[13].toPoint(), QPoint(254,254));
    QCOMPARE(poly[14].toPoint(), QPoint(254,254));
    QCOMPARE(poly[15].toPoint(), QPoint(254,254));
    QCOMPARE(poly[16].toPoint(), QPoint(254,254));
    QCOMPARE(poly[17].toPoint(), QPoint(254,254));
    QCOMPARE(poly[18].toPoint(), QPoint(254,254));
    QCOMPARE(poly[19].toPoint(), QPoint(254,254));
    QCOMPARE(poly[20].toPoint(), QPoint(254,254));
    QCOMPARE(poly[21].toPoint(), QPoint(254,254));
    QCOMPARE(poly[22].toPoint(), QPoint(254,254));
    QCOMPARE(poly[23].toPoint(), QPoint(254,254));
    QCOMPARE(poly[24].toPoint(), QPoint(254,254));
    QCOMPARE(poly[25].toPoint(), QPoint(254,254));
    QCOMPARE(poly[26].toPoint(), QPoint(254,254));
    QCOMPARE(poly[27].toPoint(), QPoint(254,254));
    QCOMPARE(poly[28].toPoint(), QPoint(254,254));
    QCOMPARE(poly[29].toPoint(), QPoint(254,254));
    QCOMPARE(poly[30].toPoint(), QPoint(254,254));
    QCOMPARE(poly[31].toPoint(), QPoint(254,254));
    QCOMPARE(poly[32].toPoint(), QPoint(254,0));
    QCOMPARE(poly[33].toPoint(), QPoint(254,0));
    QCOMPARE(poly[34].toPoint(), QPoint(254,0));
    QCOMPARE(poly[35].toPoint(), QPoint(254,0));
    QCOMPARE(poly[36].toPoint(), QPoint(254,0));
    QCOMPARE(poly[37].toPoint(), QPoint(254,0));
    QCOMPARE(poly[38].toPoint(), QPoint(254,0));
    QCOMPARE(poly[39].toPoint(), QPoint(254,0));
    QCOMPARE(poly[40].toPoint(), QPoint(254,0));
    QCOMPARE(poly[41].toPoint(), QPoint(254,0));
    QCOMPARE(poly[42].toPoint(), QPoint(254,0));
    QCOMPARE(poly[43].toPoint(), QPoint(254,0));
    QCOMPARE(poly[44].toPoint(), QPoint(254,0));
    QCOMPARE(poly[45].toPoint(), QPoint(254,0));
    QCOMPARE(poly[46].toPoint(), QPoint(254,0));
    QCOMPARE(poly[47].toPoint(), QPoint(254,0));
    QCOMPARE(poly[48].toPoint(), QPoint(254,0));
    QCOMPARE(poly[49].toPoint(), QPoint(254,0));
    QCOMPARE(poly[50].toPoint(), QPoint(254,0));
    QCOMPARE(poly[51].toPoint(), QPoint(254,0));
    QCOMPARE(poly[52].toPoint(), QPoint(254,0));
    QCOMPARE(poly[53].toPoint(), QPoint(254,0));
    QCOMPARE(poly[54].toPoint(), QPoint(254,0));
    QCOMPARE(poly[55].toPoint(), QPoint(254,0));
    QCOMPARE(poly[56].toPoint(), QPoint(254,0));
    QCOMPARE(poly[57].toPoint(), QPoint(254,0));
    QCOMPARE(poly[58].toPoint(), QPoint(254,0));
    QCOMPARE(poly[59].toPoint(), QPoint(254,0));
    QCOMPARE(poly[60].toPoint(), QPoint(254,0));
    QCOMPARE(poly[61].toPoint(), QPoint(254,0));
    QCOMPARE(poly[62].toPoint(), QPoint(254,0));
    QCOMPARE(poly[63].toPoint(), QPoint(254,0));
    QCOMPARE(poly[64].toPoint(), QPoint(254,0));
    QCOMPARE(poly[65].toPoint(), QPoint(0,0));
    QCOMPARE(poly[66].toPoint(), QPoint(0,0));
    QCOMPARE(poly[67].toPoint(), QPoint(0,0));
    QCOMPARE(poly[68].toPoint(), QPoint(0,0));
    QCOMPARE(poly[69].toPoint(), QPoint(0,0));
    QCOMPARE(poly[70].toPoint(), QPoint(0,0));
    QCOMPARE(poly[71].toPoint(), QPoint(0,0));
    QCOMPARE(poly[72].toPoint(), QPoint(0,0));
    QCOMPARE(poly[73].toPoint(), QPoint(0,0));
    QCOMPARE(poly[74].toPoint(), QPoint(0,0));
    QCOMPARE(poly[75].toPoint(), QPoint(0,0));
    QCOMPARE(poly[76].toPoint(), QPoint(0,0));
    QCOMPARE(poly[77].toPoint(), QPoint(0,0));
    QCOMPARE(poly[78].toPoint(), QPoint(0,0));
    QCOMPARE(poly[79].toPoint(), QPoint(0,0));
    QCOMPARE(poly[80].toPoint(), QPoint(0,0));
    QCOMPARE(poly[81].toPoint(), QPoint(0,0));
    QCOMPARE(poly[82].toPoint(), QPoint(0,0));
    QCOMPARE(poly[83].toPoint(), QPoint(0,0));
    QCOMPARE(poly[84].toPoint(), QPoint(0,0));
    QCOMPARE(poly[85].toPoint(), QPoint(0,0));
    QCOMPARE(poly[86].toPoint(), QPoint(0,0));
    QCOMPARE(poly[87].toPoint(), QPoint(0,0));
    QCOMPARE(poly[88].toPoint(), QPoint(0,0));
    QCOMPARE(poly[89].toPoint(), QPoint(0,0));
    QCOMPARE(poly[90].toPoint(), QPoint(0,0));
    QCOMPARE(poly[91].toPoint(), QPoint(0,0));
    QCOMPARE(poly[92].toPoint(), QPoint(0,0));
    QCOMPARE(poly[93].toPoint(), QPoint(0,0));
    QCOMPARE(poly[94].toPoint(), QPoint(0,0));
    QCOMPARE(poly[95].toPoint(), QPoint(0,0));
    QCOMPARE(poly[96].toPoint(), QPoint(0,0));
    QCOMPARE(poly[97].toPoint(), QPoint(0,254));
    QCOMPARE(poly[98].toPoint(), QPoint(0,254));
    QCOMPARE(poly[99].toPoint(), QPoint(0,254));
    QCOMPARE(poly[100].toPoint(), QPoint(0,254));
    QCOMPARE(poly[101].toPoint(), QPoint(0,254));
    QCOMPARE(poly[102].toPoint(), QPoint(0,254));
    QCOMPARE(poly[103].toPoint(), QPoint(0,254));
    QCOMPARE(poly[104].toPoint(), QPoint(0,254));
    QCOMPARE(poly[105].toPoint(), QPoint(0,254));
    QCOMPARE(poly[106].toPoint(), QPoint(0,254));
    QCOMPARE(poly[107].toPoint(), QPoint(0,254));
    QCOMPARE(poly[108].toPoint(), QPoint(0,254));
    QCOMPARE(poly[109].toPoint(), QPoint(0,254));
    QCOMPARE(poly[110].toPoint(), QPoint(0,254));
    QCOMPARE(poly[111].toPoint(), QPoint(0,254));
    QCOMPARE(poly[112].toPoint(), QPoint(0,254));
    QCOMPARE(poly[113].toPoint(), QPoint(0,254));
    QCOMPARE(poly[114].toPoint(), QPoint(0,254));
    QCOMPARE(poly[115].toPoint(), QPoint(0,254));
    QCOMPARE(poly[116].toPoint(), QPoint(0,254));
    QCOMPARE(poly[117].toPoint(), QPoint(0,254));
    QCOMPARE(poly[118].toPoint(), QPoint(0,254));
    QCOMPARE(poly[119].toPoint(), QPoint(0,254));
    QCOMPARE(poly[120].toPoint(), QPoint(0,254));
    QCOMPARE(poly[121].toPoint(), QPoint(0,254));
    QCOMPARE(poly[122].toPoint(), QPoint(0,254));
    QCOMPARE(poly[123].toPoint(), QPoint(0,254));
    QCOMPARE(poly[124].toPoint(), QPoint(0,254));
    QCOMPARE(poly[125].toPoint(), QPoint(0,254));
    QCOMPARE(poly[126].toPoint(), QPoint(0,254));
    QCOMPARE(poly[127].toPoint(), QPoint(0,254));
}

void EFX_Test::previewLeaf()
{
    EFX e(m_doc);
    e.setAlgorithm(EFX::Leaf);

    QPolygonF poly;
    e.preview(poly);
    QCOMPARE(poly.size(), 128);

    QCOMPARE(poly[0].toPoint(), QPoint(127,254));
    QCOMPARE(poly[1].toPoint(), QPoint(127,254));
    QCOMPARE(poly[2].toPoint(), QPoint(127,253));
    QCOMPARE(poly[3].toPoint(), QPoint(127,253));
    QCOMPARE(poly[4].toPoint(), QPoint(127,252));
    QCOMPARE(poly[5].toPoint(), QPoint(127,250));
    QCOMPARE(poly[6].toPoint(), QPoint(127,249));
    QCOMPARE(poly[7].toPoint(), QPoint(126,247));
    QCOMPARE(poly[8].toPoint(), QPoint(126,244));
    QCOMPARE(poly[9].toPoint(), QPoint(125,242));
    QCOMPARE(poly[10].toPoint(), QPoint(124,239));
    QCOMPARE(poly[11].toPoint(), QPoint(122,236));
    QCOMPARE(poly[12].toPoint(), QPoint(120,233));
    QCOMPARE(poly[13].toPoint(), QPoint(117,229));
    QCOMPARE(poly[14].toPoint(), QPoint(114,225));
    QCOMPARE(poly[15].toPoint(), QPoint(110,221));
    QCOMPARE(poly[16].toPoint(), QPoint(105,217));
    QCOMPARE(poly[17].toPoint(), QPoint(99,212));
    QCOMPARE(poly[18].toPoint(), QPoint(92,208));
    QCOMPARE(poly[19].toPoint(), QPoint(85,203));
    QCOMPARE(poly[20].toPoint(), QPoint(77,198));
    QCOMPARE(poly[21].toPoint(), QPoint(68,192));
    QCOMPARE(poly[22].toPoint(), QPoint(59,187));
    QCOMPARE(poly[23].toPoint(), QPoint(50,181));
    QCOMPARE(poly[24].toPoint(), QPoint(42,176));
    QCOMPARE(poly[25].toPoint(), QPoint(33,170));
    QCOMPARE(poly[26].toPoint(), QPoint(25,164));
    QCOMPARE(poly[27].toPoint(), QPoint(18,158));
    QCOMPARE(poly[28].toPoint(), QPoint(12,152));
    QCOMPARE(poly[29].toPoint(), QPoint(7,146));
    QCOMPARE(poly[30].toPoint(), QPoint(3,139));
    QCOMPARE(poly[31].toPoint(), QPoint(1,133));
    QCOMPARE(poly[32].toPoint(), QPoint(0,127));
    QCOMPARE(poly[33].toPoint(), QPoint(1,121));
    QCOMPARE(poly[34].toPoint(), QPoint(3,115));
    QCOMPARE(poly[35].toPoint(), QPoint(7,108));
    QCOMPARE(poly[36].toPoint(), QPoint(12,102));
    QCOMPARE(poly[37].toPoint(), QPoint(18,96));
    QCOMPARE(poly[38].toPoint(), QPoint(25,90));
    QCOMPARE(poly[39].toPoint(), QPoint(33,84));
    QCOMPARE(poly[40].toPoint(), QPoint(42,78));
    QCOMPARE(poly[41].toPoint(), QPoint(50,73));
    QCOMPARE(poly[42].toPoint(), QPoint(59,67));
    QCOMPARE(poly[43].toPoint(), QPoint(68,62));
    QCOMPARE(poly[44].toPoint(), QPoint(77,56));
    QCOMPARE(poly[45].toPoint(), QPoint(85,51));
    QCOMPARE(poly[46].toPoint(), QPoint(92,46));
    QCOMPARE(poly[47].toPoint(), QPoint(99,42));
    QCOMPARE(poly[48].toPoint(), QPoint(105,37));
    QCOMPARE(poly[49].toPoint(), QPoint(110,33));
    QCOMPARE(poly[50].toPoint(), QPoint(114,29));
    QCOMPARE(poly[51].toPoint(), QPoint(117,25));
    QCOMPARE(poly[52].toPoint(), QPoint(120,21));
    QCOMPARE(poly[53].toPoint(), QPoint(122,18));
    QCOMPARE(poly[54].toPoint(), QPoint(124,15));
    QCOMPARE(poly[55].toPoint(), QPoint(125,12));
    QCOMPARE(poly[56].toPoint(), QPoint(126,10));
    QCOMPARE(poly[57].toPoint(), QPoint(126,7));
    QCOMPARE(poly[58].toPoint(), QPoint(127,5));
    QCOMPARE(poly[59].toPoint(), QPoint(127,4));
    QCOMPARE(poly[60].toPoint(), QPoint(127,2));
    QCOMPARE(poly[61].toPoint(), QPoint(127,1));
    QCOMPARE(poly[62].toPoint(), QPoint(127,1));
    QCOMPARE(poly[63].toPoint(), QPoint(127,0));
    QCOMPARE(poly[64].toPoint(), QPoint(127,0));
    QCOMPARE(poly[65].toPoint(), QPoint(127,0));
    QCOMPARE(poly[66].toPoint(), QPoint(127,1));
    QCOMPARE(poly[67].toPoint(), QPoint(127,1));
    QCOMPARE(poly[68].toPoint(), QPoint(127,2));
    QCOMPARE(poly[69].toPoint(), QPoint(127,4));
    QCOMPARE(poly[70].toPoint(), QPoint(127,5));
    QCOMPARE(poly[71].toPoint(), QPoint(128,7));
    QCOMPARE(poly[72].toPoint(), QPoint(128,10));
    QCOMPARE(poly[73].toPoint(), QPoint(129,12));
    QCOMPARE(poly[74].toPoint(), QPoint(130,15));
    QCOMPARE(poly[75].toPoint(), QPoint(132,18));
    QCOMPARE(poly[76].toPoint(), QPoint(134,21));
    QCOMPARE(poly[77].toPoint(), QPoint(137,25));
    QCOMPARE(poly[78].toPoint(), QPoint(140,29));
    QCOMPARE(poly[79].toPoint(), QPoint(144,33));
    QCOMPARE(poly[80].toPoint(), QPoint(149,37));
    QCOMPARE(poly[81].toPoint(), QPoint(155,42));
    QCOMPARE(poly[82].toPoint(), QPoint(162,46));
    QCOMPARE(poly[83].toPoint(), QPoint(169,51));
    QCOMPARE(poly[84].toPoint(), QPoint(177,56));
    QCOMPARE(poly[85].toPoint(), QPoint(186,62));
    QCOMPARE(poly[86].toPoint(), QPoint(195,67));
    QCOMPARE(poly[87].toPoint(), QPoint(204,73));
    QCOMPARE(poly[88].toPoint(), QPoint(212,78));
    QCOMPARE(poly[89].toPoint(), QPoint(221,84));
    QCOMPARE(poly[90].toPoint(), QPoint(229,90));
    QCOMPARE(poly[91].toPoint(), QPoint(236,96));
    QCOMPARE(poly[92].toPoint(), QPoint(242,102));
    QCOMPARE(poly[93].toPoint(), QPoint(247,108));
    QCOMPARE(poly[94].toPoint(), QPoint(251,115));
    QCOMPARE(poly[95].toPoint(), QPoint(253,121));
    QCOMPARE(poly[96].toPoint(), QPoint(254,127));
    QCOMPARE(poly[97].toPoint(), QPoint(253,133));
    QCOMPARE(poly[98].toPoint(), QPoint(251,139));
    QCOMPARE(poly[99].toPoint(), QPoint(247,146));
    QCOMPARE(poly[100].toPoint(), QPoint(242,152));
    QCOMPARE(poly[101].toPoint(), QPoint(236,158));
    QCOMPARE(poly[102].toPoint(), QPoint(229,164));
    QCOMPARE(poly[103].toPoint(), QPoint(221,170));
    QCOMPARE(poly[104].toPoint(), QPoint(212,176));
    QCOMPARE(poly[105].toPoint(), QPoint(204,181));
    QCOMPARE(poly[106].toPoint(), QPoint(195,187));
    QCOMPARE(poly[107].toPoint(), QPoint(186,192));
    QCOMPARE(poly[108].toPoint(), QPoint(177,198));
    QCOMPARE(poly[109].toPoint(), QPoint(169,203));
    QCOMPARE(poly[110].toPoint(), QPoint(162,208));
    QCOMPARE(poly[111].toPoint(), QPoint(155,212));
    QCOMPARE(poly[112].toPoint(), QPoint(149,217));
    QCOMPARE(poly[113].toPoint(), QPoint(144,221));
    QCOMPARE(poly[114].toPoint(), QPoint(140,225));
    QCOMPARE(poly[115].toPoint(), QPoint(137,229));
    QCOMPARE(poly[116].toPoint(), QPoint(134,233));
    QCOMPARE(poly[117].toPoint(), QPoint(132,236));
    QCOMPARE(poly[118].toPoint(), QPoint(130,239));
    QCOMPARE(poly[119].toPoint(), QPoint(129,242));
    QCOMPARE(poly[120].toPoint(), QPoint(128,244));
    QCOMPARE(poly[121].toPoint(), QPoint(128,247));
    QCOMPARE(poly[122].toPoint(), QPoint(127,249));
    QCOMPARE(poly[123].toPoint(), QPoint(127,250));
    QCOMPARE(poly[124].toPoint(), QPoint(127,252));
    QCOMPARE(poly[125].toPoint(), QPoint(127,253));
    QCOMPARE(poly[126].toPoint(), QPoint(127,253));
    QCOMPARE(poly[127].toPoint(), QPoint(127,254));
}

void EFX_Test::previewLissajous()
{
    EFX e(m_doc);
    e.setAlgorithm(EFX::Lissajous);

    QPolygonF poly;
    e.preview(poly);
    QCOMPARE(poly.size(), 128);

    QCOMPARE(poly[0].toPoint(), QPoint(127,254));
    QCOMPARE(poly[1].toPoint(), QPoint(139,253));
    QCOMPARE(poly[2].toPoint(), QPoint(152,249));
    QCOMPARE(poly[3].toPoint(), QPoint(164,242));
    QCOMPARE(poly[4].toPoint(), QPoint(176,233));
    QCOMPARE(poly[5].toPoint(), QPoint(187,221));
    QCOMPARE(poly[6].toPoint(), QPoint(198,208));
    QCOMPARE(poly[7].toPoint(), QPoint(208,192));
    QCOMPARE(poly[8].toPoint(), QPoint(217,176));
    QCOMPARE(poly[9].toPoint(), QPoint(225,158));
    QCOMPARE(poly[10].toPoint(), QPoint(233,139));
    QCOMPARE(poly[11].toPoint(), QPoint(239,121));
    QCOMPARE(poly[12].toPoint(), QPoint(244,102));
    QCOMPARE(poly[13].toPoint(), QPoint(249,84));
    QCOMPARE(poly[14].toPoint(), QPoint(252,67));
    QCOMPARE(poly[15].toPoint(), QPoint(253,51));
    QCOMPARE(poly[16].toPoint(), QPoint(254,37));
    QCOMPARE(poly[17].toPoint(), QPoint(253,25));
    QCOMPARE(poly[18].toPoint(), QPoint(252,15));
    QCOMPARE(poly[19].toPoint(), QPoint(249,7));
    QCOMPARE(poly[20].toPoint(), QPoint(244,2));
    QCOMPARE(poly[21].toPoint(), QPoint(239,0));
    QCOMPARE(poly[22].toPoint(), QPoint(233,1));
    QCOMPARE(poly[23].toPoint(), QPoint(225,4));
    QCOMPARE(poly[24].toPoint(), QPoint(217,10));
    QCOMPARE(poly[25].toPoint(), QPoint(208,18));
    QCOMPARE(poly[26].toPoint(), QPoint(198,29));
    QCOMPARE(poly[27].toPoint(), QPoint(187,42));
    QCOMPARE(poly[28].toPoint(), QPoint(176,56));
    QCOMPARE(poly[29].toPoint(), QPoint(164,73));
    QCOMPARE(poly[30].toPoint(), QPoint(152,90));
    QCOMPARE(poly[31].toPoint(), QPoint(139,108));
    QCOMPARE(poly[32].toPoint(), QPoint(127,127));
    QCOMPARE(poly[33].toPoint(), QPoint(115,146));
    QCOMPARE(poly[34].toPoint(), QPoint(102,164));
    QCOMPARE(poly[35].toPoint(), QPoint(90,181));
    QCOMPARE(poly[36].toPoint(), QPoint(78,198));
    QCOMPARE(poly[37].toPoint(), QPoint(67,212));
    QCOMPARE(poly[38].toPoint(), QPoint(56,225));
    QCOMPARE(poly[39].toPoint(), QPoint(46,236));
    QCOMPARE(poly[40].toPoint(), QPoint(37,244));
    QCOMPARE(poly[41].toPoint(), QPoint(29,250));
    QCOMPARE(poly[42].toPoint(), QPoint(21,253));
    QCOMPARE(poly[43].toPoint(), QPoint(15,254));
    QCOMPARE(poly[44].toPoint(), QPoint(10,252));
    QCOMPARE(poly[45].toPoint(), QPoint(5,247));
    QCOMPARE(poly[46].toPoint(), QPoint(2,239));
    QCOMPARE(poly[47].toPoint(), QPoint(1,229));
    QCOMPARE(poly[48].toPoint(), QPoint(0,217));
    QCOMPARE(poly[49].toPoint(), QPoint(1,203));
    QCOMPARE(poly[50].toPoint(), QPoint(2,187));
    QCOMPARE(poly[51].toPoint(), QPoint(5,170));
    QCOMPARE(poly[52].toPoint(), QPoint(10,152));
    QCOMPARE(poly[53].toPoint(), QPoint(15,133));
    QCOMPARE(poly[54].toPoint(), QPoint(21,115));
    QCOMPARE(poly[55].toPoint(), QPoint(29,96));
    QCOMPARE(poly[56].toPoint(), QPoint(37,78));
    QCOMPARE(poly[57].toPoint(), QPoint(46,62));
    QCOMPARE(poly[58].toPoint(), QPoint(56,46));
    QCOMPARE(poly[59].toPoint(), QPoint(67,33));
    QCOMPARE(poly[60].toPoint(), QPoint(78,21));
    QCOMPARE(poly[61].toPoint(), QPoint(90,12));
    QCOMPARE(poly[62].toPoint(), QPoint(102,5));
    QCOMPARE(poly[63].toPoint(), QPoint(115,1));
    QCOMPARE(poly[64].toPoint(), QPoint(127,0));
    QCOMPARE(poly[65].toPoint(), QPoint(139,1));
    QCOMPARE(poly[66].toPoint(), QPoint(152,5));
    QCOMPARE(poly[67].toPoint(), QPoint(164,12));
    QCOMPARE(poly[68].toPoint(), QPoint(176,21));
    QCOMPARE(poly[69].toPoint(), QPoint(187,33));
    QCOMPARE(poly[70].toPoint(), QPoint(198,46));
    QCOMPARE(poly[71].toPoint(), QPoint(208,62));
    QCOMPARE(poly[72].toPoint(), QPoint(217,78));
    QCOMPARE(poly[73].toPoint(), QPoint(225,96));
    QCOMPARE(poly[74].toPoint(), QPoint(233,115));
    QCOMPARE(poly[75].toPoint(), QPoint(239,133));
    QCOMPARE(poly[76].toPoint(), QPoint(244,152));
    QCOMPARE(poly[77].toPoint(), QPoint(249,170));
    QCOMPARE(poly[78].toPoint(), QPoint(252,187));
    QCOMPARE(poly[79].toPoint(), QPoint(253,203));
    QCOMPARE(poly[80].toPoint(), QPoint(254,217));
    QCOMPARE(poly[81].toPoint(), QPoint(253,229));
    QCOMPARE(poly[82].toPoint(), QPoint(252,239));
    QCOMPARE(poly[83].toPoint(), QPoint(249,247));
    QCOMPARE(poly[84].toPoint(), QPoint(244,252));
    QCOMPARE(poly[85].toPoint(), QPoint(239,254));
    QCOMPARE(poly[86].toPoint(), QPoint(233,253));
    QCOMPARE(poly[87].toPoint(), QPoint(225,250));
    QCOMPARE(poly[88].toPoint(), QPoint(217,244));
    QCOMPARE(poly[89].toPoint(), QPoint(208,236));
    QCOMPARE(poly[90].toPoint(), QPoint(198,225));
    QCOMPARE(poly[91].toPoint(), QPoint(187,212));
    QCOMPARE(poly[92].toPoint(), QPoint(176,198));
    QCOMPARE(poly[93].toPoint(), QPoint(164,181));
    QCOMPARE(poly[94].toPoint(), QPoint(152,164));
    QCOMPARE(poly[95].toPoint(), QPoint(139,146));
    QCOMPARE(poly[96].toPoint(), QPoint(127,127));
    QCOMPARE(poly[97].toPoint(), QPoint(115,108));
    QCOMPARE(poly[98].toPoint(), QPoint(102,90));
    QCOMPARE(poly[99].toPoint(), QPoint(90,73));
    QCOMPARE(poly[100].toPoint(), QPoint(78,56));
    QCOMPARE(poly[101].toPoint(), QPoint(67,42));
    QCOMPARE(poly[102].toPoint(), QPoint(56,29));
    QCOMPARE(poly[103].toPoint(), QPoint(46,18));
    QCOMPARE(poly[104].toPoint(), QPoint(37,10));
    QCOMPARE(poly[105].toPoint(), QPoint(29,4));
    QCOMPARE(poly[106].toPoint(), QPoint(21,1));
    QCOMPARE(poly[107].toPoint(), QPoint(15,0));
    QCOMPARE(poly[108].toPoint(), QPoint(10,2));
    QCOMPARE(poly[109].toPoint(), QPoint(5,7));
    QCOMPARE(poly[110].toPoint(), QPoint(2,15));
    QCOMPARE(poly[111].toPoint(), QPoint(1,25));
    QCOMPARE(poly[112].toPoint(), QPoint(0,37));
    QCOMPARE(poly[113].toPoint(), QPoint(1,51));
    QCOMPARE(poly[114].toPoint(), QPoint(2,67));
    QCOMPARE(poly[115].toPoint(), QPoint(5,84));
    QCOMPARE(poly[116].toPoint(), QPoint(10,102));
    QCOMPARE(poly[117].toPoint(), QPoint(15,121));
    QCOMPARE(poly[118].toPoint(), QPoint(21,139));
    QCOMPARE(poly[119].toPoint(), QPoint(29,158));
    QCOMPARE(poly[120].toPoint(), QPoint(37,176));
    QCOMPARE(poly[121].toPoint(), QPoint(46,192));
    QCOMPARE(poly[122].toPoint(), QPoint(56,208));
    QCOMPARE(poly[123].toPoint(), QPoint(67,221));
    QCOMPARE(poly[124].toPoint(), QPoint(78,233));
    QCOMPARE(poly[125].toPoint(), QPoint(90,242));
    QCOMPARE(poly[126].toPoint(), QPoint(102,249));
    QCOMPARE(poly[127].toPoint(), QPoint(115,253));
}

// Due to rounding errors, reverse direction might come out
// +/- one point. For now it's acceptable, but should be fixed
// some day.
static bool CloseEnough(QPointF const & a, QPointF const & b)
{
    QPointF diff = a - b;
    //qDebug() << "Diff:" << diff.x() << diff.y();
    return -1 <= diff.x() && diff.x() <= 1 &&
           -1 <= diff.y() && diff.y() <= 1;
}

void EFX_Test::previewCircleBackwards()
{
    EFX e(m_doc);

    QPolygonF polyF;
    e.preview(polyF);
    QCOMPARE(polyF.size(), 128);

    e.setDirection(Function::Backward);

    QPolygonF polyB;
    e.preview(polyB);
    QCOMPARE(polyB.size(), 128);

    QVERIFY(CloseEnough(polyB[0], polyF[0]));

    for (int i = 1; i < 128; ++i)
    {
         QVERIFY(CloseEnough(polyB[128 - i], polyF[i]));
    }
}

void EFX_Test::previewEightBackwards()
{
    EFX e(m_doc);

    e.setAlgorithm(EFX::Eight);

    QPolygonF polyF;
    e.preview(polyF);
    QCOMPARE(polyF.size(), 128);

    e.setDirection(Function::Backward);

    QPolygonF polyB;
    e.preview(polyB);
    QCOMPARE(polyB.size(), 128);

    QVERIFY(CloseEnough(polyB[0], polyF[0]));

    for (int i = 1; i < 128; ++i)
    {
         QVERIFY(CloseEnough(polyB[128 - i], polyF[i]));
    }
}

void EFX_Test::previewLineBackwards()
{
    EFX e(m_doc);

    e.setAlgorithm(EFX::Line);

    QPolygonF polyF;
    e.preview(polyF);
    QCOMPARE(polyF.size(), 128);

    e.setDirection(Function::Backward);

    QPolygonF polyB;
    e.preview(polyB);
    QCOMPARE(polyB.size(), 128);

    for (int i = 0; i < 64; ++i)
    {
         QVERIFY(CloseEnough(polyB[i + 64], polyF[i]));
    }

    for (int i = 64; i < 128; ++i)
    {
         QVERIFY(CloseEnough(polyB[i - 64], polyF[i]));
    }
}

void EFX_Test::previewLine2Backwards()
{
    EFX e(m_doc);

    e.setAlgorithm(EFX::Line2);

    QPolygonF polyF;
    e.preview(polyF);
    QCOMPARE(polyF.size(), 128);

    e.setDirection(Function::Backward);

    QPolygonF polyB;
    e.preview(polyB);
    QCOMPARE(polyB.size(), 128);

    QVERIFY(CloseEnough(polyB[0], polyF[0]));

    for (int i = 1; i < 128; ++i)
    {
         QVERIFY(CloseEnough(polyB[128 - i], polyF[i]));
    }
}

void EFX_Test::previewDiamondBackwards()
{
    EFX e(m_doc);

    e.setAlgorithm(EFX::Diamond);

    QPolygonF polyF;
    e.preview(polyF);
    QCOMPARE(polyF.size(), 128);

    e.setDirection(Function::Backward);

    QPolygonF polyB;
    e.preview(polyB);
    QCOMPARE(polyB.size(), 128);

    QVERIFY(CloseEnough(polyB[0], polyF[0]));

    for (int i = 1; i < 128; ++i)
    {
         QVERIFY(CloseEnough(polyB[128 - i], polyF[i]));
    }
}

void EFX_Test::previewSquareBackwards()
{
    EFX e(m_doc);

    e.setAlgorithm(EFX::Square);

    QPolygonF polyF;
    e.preview(polyF);
    QCOMPARE(polyF.size(), 128);

    e.setDirection(Function::Backward);

    QPolygonF polyB;
    e.preview(polyB);
    QCOMPARE(polyB.size(), 128);

    QVERIFY(CloseEnough(polyB[0], polyF[0]));

    for (int i = 1; i < 128; ++i)
    {
         QVERIFY(CloseEnough(polyB[128 - i], polyF[i]));
    }
}

void EFX_Test::previewSquareChoppyBackwards()
{
    EFX e(m_doc);

    e.setAlgorithm(EFX::SquareChoppy);

    QPolygonF polyF;
    e.preview(polyF);
    QCOMPARE(polyF.size(), 128);

    e.setDirection(Function::Backward);

    QPolygonF polyB;
    e.preview(polyB);
    QCOMPARE(polyB.size(), 128);

    QVERIFY(CloseEnough(polyB[0], polyF[0]));

    for (int i = 1; i < 128; ++i)
    {
         QVERIFY(CloseEnough(polyB[128 - i], polyF[i]));
    }
}

void EFX_Test::previewLeafBackwards()
{
    EFX e(m_doc);

    e.setAlgorithm(EFX::Leaf);

    QPolygonF polyF;
    e.preview(polyF);
    QCOMPARE(polyF.size(), 128);

    e.setDirection(Function::Backward);

    QPolygonF polyB;
    e.preview(polyB);
    QCOMPARE(polyB.size(), 128);

    QVERIFY(CloseEnough(polyB[0], polyF[0]));

    for (int i = 1; i < 128; ++i)
    {
         QVERIFY(CloseEnough(polyB[128 - i], polyF[i]));
    }
}

void EFX_Test::previewLissajousBackwards()
{
    EFX e(m_doc);

    e.setAlgorithm(EFX::Lissajous);

    QPolygonF polyF;
    e.preview(polyF);
    QCOMPARE(polyF.size(), 128);

    e.setDirection(Function::Backward);

    QPolygonF polyB;
    e.preview(polyB);
    QCOMPARE(polyB.size(), 128);

    QVERIFY(CloseEnough(polyB[0], polyF[0]));

    for (int i = 1; i < 128; ++i)
    {
         QVERIFY(CloseEnough(polyB[128 - i], polyF[i]));
    }
}

void EFX_Test::widthHeightOffset()
{
    EFX e(m_doc);
    int i = 0;
    int max = 0;

    QPolygonF poly;
    e.preview(poly);
    QCOMPARE(poly.size(), 128);

    /* Check that width affects the pattern */
    e.setWidth(50);
    poly.clear();
    e.preview(poly);
    QVERIFY(poly.size() == 128);

    /* Width of 50 means actually 50px left of center (127-50) and
       50px right of center (127+50). +1 because the bound coordinates
       are OUTSIDE the actual points. */
    QVERIFY(poly.boundingRect().width() == 50 + 50);
    QVERIFY(poly.boundingRect().height() == 127 + 127);

    /* Check that height affects the pattern */
    e.setHeight(87);
    poly.clear();
    e.preview(poly);
    QVERIFY(poly.size() == 128);

    /* Height of 87 means actually 87px down of center (127-87) and
       87px up of center (127+87). And +1 because the bound coordinates
       are OUTSIDE the actual points. */
    QVERIFY(poly.boundingRect().height() == 87 + 87);
    QVERIFY(poly.boundingRect().width() == 100);

    /* X Offset is at center */
    max = 0;
    poly.clear();
    e.preview(poly);
    QVERIFY(poly.size() == 128);
    for (i = 0; i < 128; i++)
        if (poly[i].x() > max)
            max = poly[i].x();
    QVERIFY(max == 177);

    /* X offset + 20 */
    max = 0;
    e.setXOffset(127 + 20);
    poly.clear();
    e.preview(poly);
    QVERIFY(poly.size() == 128);
    for (i = 0; i < 128; i++)
        if (poly[i].x() > max)
            max = poly[i].x();
    QVERIFY(max == 197);

    /* Y Offset is at center */
    max = 0;
    poly.clear();
    e.preview(poly);
    QVERIFY(poly.size() == 128);
    for (i = 0; i < 128; i++)
        if (poly[i].y() > max)
            max = poly[i].y();
    QVERIFY(max == 214);

    /* Y offset - 25 */
    max = 0;
    e.setYOffset(127 - 25);
    poly.clear();
    e.preview(poly);
    QVERIFY(poly.size() == 128);
    for (i = 0; i < 128; i++)
        if (poly[i].y() > max)
            max = poly[i].y();
    QVERIFY(max == 189);
}

void EFX_Test::rotateAndScale()
{
    EFX efx(m_doc);

    /* The X & Y params used here represent the first point in a circle
       algorithm as calculated by EFX::circlePoint(), based on iterator
       value 0, before calling rotateAndScale(). */
    float x, y;

    /* Rotation */
    x = -0.125333;
    y = 0.992115;
    efx.setWidth(127);
    efx.setHeight(127);
    efx.setXOffset(127);
    efx.setYOffset(127);
    efx.setRotation(0);
    efx.rotateAndScale(&x, &y);
    QVERIFY(floor(x + 0.5) == 111);
    QVERIFY(floor(y + 0.5) == 253);

    x = -0.125333;
    y = 0.992115;
    efx.setWidth(127);
    efx.setHeight(127);
    efx.setXOffset(127);
    efx.setYOffset(127);
    efx.setRotation(90);
    efx.rotateAndScale(&x, &y);
    QVERIFY(floor(x + 0.5) == 253);
    QVERIFY(floor(y + 0.5) == 143);

    x = -0.125333;
    y = 0.992115;
    efx.setWidth(127);
    efx.setHeight(127);
    efx.setXOffset(127);
    efx.setYOffset(127);
    efx.setRotation(180);
    efx.rotateAndScale(&x, &y);
    QVERIFY(floor(x + 0.5) == 143);
    QVERIFY(floor(y + 0.5) == 1);

    x = -0.125333;
    y = 0.992115;
    efx.setWidth(127);
    efx.setHeight(127);
    efx.setXOffset(127);
    efx.setYOffset(127);
    efx.setRotation(270);
    efx.rotateAndScale(&x, &y);
    QVERIFY(floor(x + 0.5) == 1);
    QVERIFY(floor(y + 0.5) == 111);

    x = -0.125333;
    y = 0.992115;
    efx.setWidth(127);
    efx.setHeight(127);
    efx.setXOffset(127);
    efx.setYOffset(127);
    efx.setRotation(45);
    efx.rotateAndScale(&x, &y);
    QVERIFY(floor(x + 0.5) == 205);
    QVERIFY(floor(y + 0.5) == 227);

    /* Offset */
    x = -0.125333;
    y = 0.992115;
    efx.setWidth(127);
    efx.setHeight(127);
    efx.setXOffset(128);
    efx.setYOffset(128);
    efx.setRotation(0);
    efx.rotateAndScale(&x, &y);
    QVERIFY(floor(x + 0.5) == 112);
    QVERIFY(floor(y + 0.5) == 254);

    x = -0.125333;
    y = 0.992115;
    efx.setWidth(127);
    efx.setHeight(127);
    efx.setXOffset(0);
    efx.setYOffset(0);
    efx.setRotation(0);
    efx.rotateAndScale(&x, &y);
    QVERIFY(floor(x + 0.5) == -16);
    QVERIFY(floor(y + 0.5) == 126);

    x = -0.125333;
    y = 0.992115;
    efx.setWidth(127);
    efx.setHeight(127);
    efx.setXOffset(0);
    efx.setYOffset(127);
    efx.setRotation(0);
    efx.rotateAndScale(&x, &y);
    QVERIFY(floor(x + 0.5) == -16);
    QVERIFY(floor(y + 0.5) == 253);

    x = -0.125333;
    y = 0.992115;
    efx.setWidth(127);
    efx.setHeight(127);
    efx.setXOffset(127);
    efx.setYOffset(0);
    efx.setRotation(0);
    efx.rotateAndScale(&x, &y);
    QVERIFY(floor(x + 0.5) == 111);
    QVERIFY(floor(y + 0.5) == 126);

    x = -0.125333;
    y = 0.992115;
    efx.setWidth(127);
    efx.setHeight(127);
    efx.setXOffset(1);
    efx.setYOffset(UCHAR_MAX);
    efx.setRotation(0);
    efx.rotateAndScale(&x, &y);
    QVERIFY(floor(x + 0.5) == -15);
    QVERIFY(floor(y + 0.5) == 381);

    /* Width & height (rotate also by 90 degrees to get more tangible
       results, because x&y point to the bottom-center, where width & height
       params have very little effect without offset or rotation). */
    x = -0.125333;
    y = 0.992115;
    efx.setWidth(64);
    efx.setHeight(64);
    efx.setXOffset(127);
    efx.setYOffset(127);
    efx.setRotation(90);
    efx.rotateAndScale(&x, &y);
    QCOMPARE(floor(x + 0.5), double(190));
    QCOMPARE(floor(y + 0.5), double(135));

    x = -0.125333;
    y = 0.992115;
    efx.setWidth(10);
    efx.setHeight(127);
    efx.setXOffset(127);
    efx.setYOffset(127);
    efx.setRotation(90);
    efx.rotateAndScale(&x, &y);
    QCOMPARE(floor(x + 0.5), double(253));
    QCOMPARE(floor(y + 0.5), double(128));

    x = -0.125333;
    y = 0.992115;
    efx.setWidth(64);
    efx.setHeight(127);
    efx.setXOffset(127);
    efx.setYOffset(127);
    efx.setRotation(90);
    efx.rotateAndScale(&x, &y);
    QCOMPARE(floor(x + 0.5), double(253));
    QCOMPARE(floor(y + 0.5), double(135));

    x = -0.125333;
    y = 0.992115;
    efx.setWidth(64);
    efx.setHeight(0);
    efx.setXOffset(127);
    efx.setYOffset(127);
    efx.setRotation(90);
    efx.rotateAndScale(&x, &y);
    QCOMPARE(floor(x + 0.5), double(127));
    QCOMPARE(floor(y + 0.5), double(135));

    x = -0.125333;
    y = 0.992115;
    efx.setWidth(127);
    efx.setHeight(0);
    efx.setXOffset(127);
    efx.setYOffset(127);
    efx.setRotation(90);
    efx.rotateAndScale(&x, &y);
    QCOMPARE(floor(x + 0.5), double(127));
    QCOMPARE(floor(y + 0.5), double(143));
}

void EFX_Test::copyFrom()
{
    EFX e1(m_doc);
    e1.setName("First");
    e1.setDirection(EFX::Backward);
    e1.setRunOrder(EFX::SingleShot);
    e1.setAlgorithm(EFX::Lissajous);
    e1.setWidth(13);
    e1.setHeight(42);
    e1.setRotation(78);
    e1.setXOffset(34);
    e1.setYOffset(27);
    e1.setXFrequency(5);
    e1.setYFrequency(4);
    e1.setXPhase(163);
    e1.setYPhase(94);
    e1.setPropagationMode(EFX::Serial);
    e1.setFadeInSpeed(42);
    e1.setFadeOutSpeed(69);
    e1.setDuration(1337);

    EFXFixture* ef1 = new EFXFixture(&e1);
    ef1->setHead(GroupHead(12, 3));
    e1.addFixture(ef1);
    EFXFixture* ef2 = new EFXFixture(&e1);
    ef2->setHead(GroupHead(34, 7));
    e1.addFixture(ef2);

    /* Verify that EFX contents are copied */
    EFX e2(m_doc);
    QSignalSpy spy(&e2, SIGNAL(changed(quint32)));
    QVERIFY(e2.copyFrom(&e1) == true);
    QCOMPARE(spy.size(), 1);
    QVERIFY(e2.name() == e1.name());
    QVERIFY(e2.direction() == EFX::Backward);
    QVERIFY(e2.runOrder() == EFX::SingleShot);
    QVERIFY(e2.algorithm() == EFX::Lissajous);
    QVERIFY(e2.width() == 13);
    QVERIFY(e2.height() == 42);
    QVERIFY(e2.rotation() == 78);
    QVERIFY(e2.xOffset() == 34);
    QVERIFY(e2.yOffset() == 27);
    QVERIFY(e2.xFrequency() == 5);
    QVERIFY(e2.yFrequency() == 4);
    QVERIFY(e2.xPhase() == 163);
    QVERIFY(e2.yPhase() == 94);
    QVERIFY(e2.propagationMode() == EFX::Serial);
    QCOMPARE(e2.fadeInSpeed(), uint(42));
    QCOMPARE(e2.fadeOutSpeed(), uint(69));
    QCOMPARE(e2.duration(), uint(1337));
    QVERIFY(e2.fixtures().size() == 2);
    QVERIFY(e2.fixtures().at(0)->head().fxi == 12);
    QVERIFY(e2.fixtures().at(0)->head().head == 3);
    QVERIFY(e2.fixtures().at(1)->head().fxi == 34);
    QVERIFY(e2.fixtures().at(1)->head().head == 7);
    QVERIFY(e2.fixtures().at(0) != ef1);
    QVERIFY(e2.fixtures().at(1) != ef2);

    /* Verify that an EFX gets a copy only from another EFX */
    Scene s(m_doc);
    QVERIFY(e2.copyFrom(&s) == false);

    /* Make a third EFX */
    EFX e3(m_doc);
    e3.setName("Third");
    e3.setDirection(EFX::Forward);
    e3.setRunOrder(EFX::Loop);
    e3.setAlgorithm(EFX::Eight);
    e3.setWidth(31);
    e3.setHeight(24);
    e3.setRotation(87);
    e3.setXOffset(43);
    e3.setYOffset(72);
    e3.setXFrequency(3);
    e3.setYFrequency(2);
    e3.setXPhase(136);
    e3.setYPhase(49);
    e3.setPropagationMode(EFX::Parallel);
    e3.setFadeInSpeed(69);
    e3.setFadeOutSpeed(1337);
    e3.setDuration(42);
    EFXFixture* ef3 = new EFXFixture(&e3);
    ef3->setHead(GroupHead(56, 8));
    e3.addFixture(ef3);
    EFXFixture* ef4 = new EFXFixture(&e3);
    ef4->setHead(GroupHead(78, 9));
    e3.addFixture(ef4);

    /* Verify that copying to the same EFX a second time succeeds */
    QVERIFY(e2.copyFrom(&e3) == true);
    QVERIFY(e2.name() == e3.name());
    QVERIFY(e2.direction() == EFX::Forward);
    QVERIFY(e2.runOrder() == EFX::Loop);
    QVERIFY(e2.algorithm() == EFX::Eight);
    QVERIFY(e2.width() == 31);
    QVERIFY(e2.height() == 24);
    QVERIFY(e2.rotation() == 87);
    QVERIFY(e2.xOffset() == 43);
    QVERIFY(e2.yOffset() == 72);
    QVERIFY(e2.xFrequency() == 3);
    QVERIFY(e2.yFrequency() == 2);
    QVERIFY(e2.xPhase() == 136);
    QVERIFY(e2.yPhase() == 49);
    QVERIFY(e2.propagationMode() == EFX::Parallel);
    QCOMPARE(e2.fadeInSpeed(), uint(69));
    QCOMPARE(e2.fadeOutSpeed(), uint(1337));
    QCOMPARE(e2.duration(), uint(42));
    QVERIFY(e2.fixtures().size() == 2);
    QVERIFY(e2.fixtures().at(0)->head().fxi == 56);
    QVERIFY(e2.fixtures().at(0)->head().head == 8);
    QVERIFY(e2.fixtures().at(1)->head().fxi == 78);
    QVERIFY(e2.fixtures().at(1)->head().head == 9);
    QVERIFY(e2.fixtures().at(0) != ef1);
    QVERIFY(e2.fixtures().at(0) != ef3);
    QVERIFY(e2.fixtures().at(1) != ef2);
    QVERIFY(e2.fixtures().at(1) != ef4);
}

void EFX_Test::createCopy()
{
    Doc doc(this);

    EFX* e1 = new EFX(m_doc);
    e1->setName("First");
    e1->setDirection(EFX::Forward);
    e1->setRunOrder(EFX::PingPong);
    e1->setAlgorithm(EFX::Line);
    e1->setWidth(13);
    e1->setHeight(42);
    e1->setRotation(78);
    e1->setXOffset(34);
    e1->setYOffset(27);
    e1->setXFrequency(5);
    e1->setYFrequency(4);
    e1->setXPhase(163);
    e1->setYPhase(94);
    e1->setPropagationMode(EFX::Serial);
    e1->setFadeInSpeed(42);
    e1->setFadeOutSpeed(69);
    e1->setDuration(1337);
    EFXFixture* ef1 = new EFXFixture(e1);
    ef1->setHead(GroupHead(12, 3));
    e1->addFixture(ef1);
    EFXFixture* ef2 = new EFXFixture(e1);
    ef2->setHead(GroupHead(34, 7));
    e1->addFixture(ef2);

    doc.addFunction(e1);
    QVERIFY(e1->id() != Function::invalidId());

    Function* f = e1->createCopy(&doc);
    QVERIFY(f != NULL);
    QVERIFY(f != e1);
    QVERIFY(f->id() != e1->id());

    EFX* copy = qobject_cast<EFX*> (f);
    QVERIFY(copy != NULL);
    QVERIFY(copy->name() == e1->name());
    QVERIFY(copy->direction() == EFX::Forward);
    QVERIFY(copy->runOrder() == EFX::PingPong);
    QVERIFY(copy->algorithm() == EFX::Line);
    QVERIFY(copy->width() == 13);
    QVERIFY(copy->height() == 42);
    QVERIFY(copy->rotation() == 78);
    QVERIFY(copy->xOffset() == 34);
    QVERIFY(copy->yOffset() == 27);
    QVERIFY(copy->xFrequency() == 5);
    QVERIFY(copy->yFrequency() == 4);
    QVERIFY(copy->xPhase() == 163);
    QVERIFY(copy->yPhase() == 94);
    QVERIFY(copy->propagationMode() == EFX::Serial);
    QCOMPARE(copy->fadeInSpeed(), uint(42));
    QCOMPARE(copy->fadeOutSpeed(), uint(69));
    QCOMPARE(copy->duration(), uint(1337));
    QVERIFY(copy->fixtures().size() == 2);
    QVERIFY(copy->fixtures().at(0)->head().fxi == 12);
    QVERIFY(copy->fixtures().at(0)->head().head == 3);
    QVERIFY(copy->fixtures().at(1)->head().fxi == 34);
    QVERIFY(copy->fixtures().at(1)->head().head == 7);
    QVERIFY(copy->fixtures().at(0) != ef1);
    QVERIFY(copy->fixtures().at(1) != ef2);
}

void EFX_Test::loadXAxis()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Axis");
    xmlWriter.writeAttribute("Name", "X");

    xmlWriter.writeTextElement("Offset", "1");
    xmlWriter.writeTextElement("Frequency", "2");
    xmlWriter.writeTextElement("Phase", "3");
    // Unknown tag
    xmlWriter.writeTextElement("Foo", "Bar");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    EFX e(m_doc);
    QVERIFY(e.loadXMLAxis(xmlReader) == true);
}

void EFX_Test::loadYAxis()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Axis");
    xmlWriter.writeAttribute("Name", "Y");

    xmlWriter.writeTextElement("Offset", "1");
    xmlWriter.writeTextElement("Frequency", "2");
    xmlWriter.writeTextElement("Phase", "3");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    EFX e(m_doc);
    QVERIFY(e.loadXMLAxis(xmlReader) == true);
}

void EFX_Test::loadYAxisWrongRoot()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("sixA");
    xmlWriter.writeAttribute("Name", "Y");

    xmlWriter.writeTextElement("Offset", "1");
    xmlWriter.writeTextElement("Frequency", "2");
    xmlWriter.writeTextElement("Phase", "3");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    EFX e(m_doc);
    QVERIFY(e.loadXMLAxis(xmlReader) == false);
}

void EFX_Test::loadAxisNoXY()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Axis");
    xmlWriter.writeAttribute("Name", "Not X nor Y");

    xmlWriter.writeTextElement("Offset", "1");
    xmlWriter.writeTextElement("Frequency", "5");
    xmlWriter.writeTextElement("Phase", "333");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    EFX e(m_doc);
    QVERIFY(e.loadXMLAxis(xmlReader) == false);
    QVERIFY(e.xOffset() != 1);
    QVERIFY(e.xFrequency() != 5);
    QVERIFY(e.xPhase() != 333);
}

void EFX_Test::loadSuccessLegacy()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Function");
    xmlWriter.writeAttribute("Type", "EFX");
    xmlWriter.writeAttribute("Name", "Test EFX");

    xmlWriter.writeTextElement("PropagationMode", "Serial");

    xmlWriter.writeStartElement("Bus");
    xmlWriter.writeAttribute("Role", "Fade");
    xmlWriter.writeCharacters("12");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Bus");
    xmlWriter.writeAttribute("Role", "Hold");
    xmlWriter.writeCharacters("13");
    xmlWriter.writeEndElement();

    xmlWriter.writeTextElement("Direction", "Forward");
    xmlWriter.writeTextElement("RunOrder", "Loop");
    xmlWriter.writeTextElement("Algorithm", "Diamond");
    xmlWriter.writeTextElement("Width", "100");
    xmlWriter.writeTextElement("Height", "90");
    xmlWriter.writeTextElement("Rotation", "310");

    /* X Axis */
    xmlWriter.writeStartElement("Axis");
    xmlWriter.writeAttribute("Name", "X");
    xmlWriter.writeTextElement("Offset", "10");
    xmlWriter.writeTextElement("Frequency", "2");
    xmlWriter.writeTextElement("Phase", "270");
    xmlWriter.writeEndElement();

    /* Y Axis */
    xmlWriter.writeStartElement("Axis");
    xmlWriter.writeAttribute("Name", "Y");
    xmlWriter.writeTextElement("Offset", "20");
    xmlWriter.writeTextElement("Frequency", "3");
    xmlWriter.writeTextElement("Phase", "80");
    xmlWriter.writeEndElement();

    /* Fixture 1 */
    xmlWriter.writeStartElement("Fixture");
    xmlWriter.writeTextElement("ID", "33");
    xmlWriter.writeTextElement("Direction", "Forward");
    xmlWriter.writeEndElement();

    /* Fixture 2 */
    xmlWriter.writeStartElement("Fixture");
    xmlWriter.writeTextElement("ID", "11");
    xmlWriter.writeTextElement("Direction", "Backward");
    xmlWriter.writeEndElement();

    /* Fixture 3 */
    xmlWriter.writeStartElement("Fixture");
    xmlWriter.writeTextElement("ID", "45");
    xmlWriter.writeTextElement("Direction", "Backward");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    EFX e(m_doc);
    QVERIFY(e.loadXML(xmlReader) == true);

    QVERIFY(e.m_legacyFadeBus == 12);
    QVERIFY(e.m_legacyHoldBus == 13);
    QVERIFY(e.direction() == EFX::Forward);
    QVERIFY(e.runOrder() == EFX::Loop);

    QVERIFY(e.algorithm() == EFX::Diamond);
    QVERIFY(e.width() == 100);
    QVERIFY(e.height() == 90);
    QVERIFY(e.rotation() == 310);

    QVERIFY(e.xOffset() == 10);
    QVERIFY(e.xFrequency() == 2);
    QVERIFY(e.xPhase() == 270);
    QVERIFY(e.yOffset() == 20);
    QVERIFY(e.yFrequency() == 3);
    QVERIFY(e.yPhase() == 80);

    QVERIFY(e.propagationMode() == EFX::Serial);
    QCOMPARE(e.fixtures().size(), 3);
    QCOMPARE(e.fixtures().at(0)->head().fxi, quint32(33));
    QCOMPARE(e.fixtures().at(0)->direction(), EFX::Forward);
    QCOMPARE(e.fixtures().at(1)->head().fxi, quint32(11));
    QCOMPARE(e.fixtures().at(1)->direction(), EFX::Backward);
    QCOMPARE(e.fixtures().at(2)->head().fxi, quint32(45));
    QCOMPARE(e.fixtures().at(2)->direction(), EFX::Backward);

    Bus::instance()->setValue(12, 50);
    Bus::instance()->setValue(13, 100);
    e.postLoad();

    QVERIFY(e.fadeInSpeed() == uint(1000));
    QVERIFY(e.fadeOutSpeed() == uint(1000));
    QVERIFY(e.duration() == uint(2000));
}

void EFX_Test::loadSuccess()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Function");
    xmlWriter.writeAttribute("Type", "EFX");
    xmlWriter.writeAttribute("Name", "Test EFX");

    xmlWriter.writeTextElement("PropagationMode", "Serial");

    xmlWriter.writeStartElement("Speed");
    xmlWriter.writeAttribute("FadeIn", "1300");
    xmlWriter.writeAttribute("FadeOut", "1400");
    xmlWriter.writeAttribute("Duration", "1500");
    xmlWriter.writeEndElement();

    xmlWriter.writeTextElement("Direction", "Forward");
    xmlWriter.writeTextElement("RunOrder", "Loop");
    xmlWriter.writeTextElement("Algorithm", "Diamond");
    xmlWriter.writeTextElement("Width", "100");
    xmlWriter.writeTextElement("Height", "90");
    xmlWriter.writeTextElement("Rotation", "310");

    /* X Axis */
    xmlWriter.writeStartElement("Axis");
    xmlWriter.writeAttribute("Name", "X");
    xmlWriter.writeTextElement("Offset", "10");
    xmlWriter.writeTextElement("Frequency", "2");
    xmlWriter.writeTextElement("Phase", "270");
    xmlWriter.writeEndElement();

    /* Y Axis */
    xmlWriter.writeStartElement("Axis");
    xmlWriter.writeAttribute("Name", "Y");
    xmlWriter.writeTextElement("Offset", "20");
    xmlWriter.writeTextElement("Frequency", "3");
    xmlWriter.writeTextElement("Phase", "80");
    xmlWriter.writeEndElement();

    /* Fixture 1 */
    xmlWriter.writeStartElement("Fixture");
    xmlWriter.writeTextElement("ID", "33");
    xmlWriter.writeTextElement("Direction", "Forward");
    xmlWriter.writeEndElement();

    /* Fixture 2 */
    xmlWriter.writeStartElement("Fixture");
    xmlWriter.writeTextElement("ID", "11");
    xmlWriter.writeTextElement("Direction", "Backward");
    xmlWriter.writeEndElement();

    /* Fixture 3 */
    xmlWriter.writeStartElement("Fixture");
    xmlWriter.writeTextElement("ID", "45");
    xmlWriter.writeTextElement("Direction", "Backward");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    EFX e(m_doc);
    QVERIFY(e.loadXML(xmlReader) == true);
    QVERIFY(e.direction() == EFX::Forward);
    QVERIFY(e.runOrder() == EFX::Loop);

    QVERIFY(e.algorithm() == EFX::Diamond);
    QVERIFY(e.width() == 100);
    QVERIFY(e.height() == 90);
    QVERIFY(e.rotation() == 310);

    QVERIFY(e.xOffset() == 10);
    QVERIFY(e.xFrequency() == 2);
    QVERIFY(e.xPhase() == 270);
    QVERIFY(e.yOffset() == 20);
    QVERIFY(e.yFrequency() == 3);
    QVERIFY(e.yPhase() == 80);

    QVERIFY(e.propagationMode() == EFX::Serial);
    QVERIFY(e.fixtures().size() == 3);
    QVERIFY(e.fixtures().at(0)->head().fxi == 33);
    QVERIFY(e.fixtures().at(0)->direction() == EFX::Forward);
    QVERIFY(e.fixtures().at(1)->head().fxi == 11);
    QVERIFY(e.fixtures().at(1)->direction() == EFX::Backward);
    QVERIFY(e.fixtures().at(2)->head().fxi == 45);
    QVERIFY(e.fixtures().at(2)->direction() == EFX::Backward);

    QVERIFY(e.fadeInSpeed() == uint(1300));
    QVERIFY(e.fadeOutSpeed() == uint(1400));
    QVERIFY(e.duration() == uint(1500));
}

void EFX_Test::loadWrongType()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Function");
    xmlWriter.writeAttribute("Type", "Chaser");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    EFX e(m_doc);
    QVERIFY(e.loadXML(xmlReader) == false);
}

void EFX_Test::loadWrongRoot()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("EFX");
    xmlWriter.writeAttribute("Type", "EFX");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    EFX e(m_doc);
    QVERIFY(e.loadXML(xmlReader) == false);
}

void EFX_Test::loadDuplicateFixture()
{
    QSKIP("Duplicate fixtures are allowed because can animate different parameters (RGB, dimmer, etc.)");

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Function");
    xmlWriter.writeAttribute("Type", "EFX");
    xmlWriter.writeAttribute("Name", "Test EFX");

    /* Fixture 1 */
    xmlWriter.writeStartElement("Fixture");
    xmlWriter.writeTextElement("ID", "33");
    xmlWriter.writeTextElement("Direction", "Forward");
    xmlWriter.writeEndElement();

    /* Fixture 2 */
    xmlWriter.writeStartElement("Fixture");
    xmlWriter.writeTextElement("ID", "33");
    xmlWriter.writeTextElement("Direction", "Backward");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    EFX e(m_doc);
    QVERIFY(e.loadXML(xmlReader) == true);
    QVERIFY(e.fixtures().size() == 1);
    QVERIFY(e.fixtures().at(0)->direction() == EFX::Forward);
}

void EFX_Test::save()
{
    EFX e1(m_doc);
    e1.setName("First");
    e1.setDirection(EFX::Backward);
    e1.setRunOrder(EFX::SingleShot);
    e1.setFadeInSpeed(42);
    e1.setFadeOutSpeed(69);
    e1.setDuration(1337);
    e1.setAlgorithm(EFX::Lissajous);
    e1.setWidth(13);
    e1.setHeight(42);
    e1.setRotation(78);
    e1.setStartOffset(91);
    e1.setIsRelative(false);
    e1.setXOffset(34);
    e1.setYOffset(27);
    e1.setXFrequency(5);
    e1.setYFrequency(4);
    e1.setXPhase(163);
    e1.setYPhase(94);
    e1.setPropagationMode(EFX::Serial);

    EFXFixture* ef1 = new EFXFixture(&e1);
    ef1->setHead(GroupHead(12, 3));
    e1.addFixture(ef1);
    EFXFixture* ef2 = new EFXFixture(&e1);
    ef2->setHead(GroupHead(34, 5));
    ef2->setDirection(EFX::Backward);
    ef2->setStartOffset(27);
    e1.addFixture(ef2);
    EFXFixture* ef3 = new EFXFixture(&e1);
    ef3->setHead(GroupHead(56,7));
    e1.addFixture(ef3);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("TestRoot");

    QVERIFY(e1.saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();
    QVERIFY(xmlReader.name().toString() == "TestRoot");
    xmlReader.readNextStartElement();
    QVERIFY(xmlReader.name().toString() == "Function");

    QVERIFY(xmlReader.attributes().value("Type").toString() == "EFX");
    QVERIFY(xmlReader.attributes().value("Name").toString() == "First");

    bool dir = false, off = false, run = false, algo = false, w = false,
         h = false, rot = false, isRelative = false, xoff = false, yoff = false,
         xfreq = false, yfreq = false, xpha = false, ypha = false,
         prop = false, speed = false;
    int fixtureid = 0, fixturehead = 0, fixturedirection = 0, fixtureStartOffset = 0;
    QList <QString> fixtures;

    while (xmlReader.readNextStartElement())
    {
        if (xmlReader.name().toString() == "Speed")
        {
            QCOMPARE(xmlReader.attributes().value("FadeIn").toString().toUInt(), uint(42));
            QCOMPARE(xmlReader.attributes().value("FadeOut").toString().toUInt(), uint(69));
            QCOMPARE(xmlReader.attributes().value("Duration").toString().toUInt(), uint(1337));
            speed = true;
            xmlReader.skipCurrentElement();
        }
        else if (xmlReader.name().toString() == "Direction")
        {
            QVERIFY(xmlReader.readElementText() == "Backward");
            dir = true;
        }
        else if (xmlReader.name().toString() == "StartOffset")
        {
            QVERIFY(xmlReader.readElementText() == "91");
            off = true;
        }
        else if (xmlReader.name().toString() == "RunOrder")
        {
            QVERIFY(xmlReader.readElementText() == "SingleShot");
            run = true;
        }
        else if (xmlReader.name().toString() == "Bus")
        {
            QFAIL("EFX should not save a Bus tag anymore!");
            xmlReader.skipCurrentElement();
        }
        else if (xmlReader.name().toString() == "Algorithm")
        {
            QVERIFY(xmlReader.readElementText() == "Lissajous");
            algo = true;
        }
        else if (xmlReader.name().toString() == "Width")
        {
            QVERIFY(xmlReader.readElementText() == "13");
            w = true;
        }
        else if (xmlReader.name().toString() == "Height")
        {
            QVERIFY(xmlReader.readElementText() == "42");
            h = true;
        }
        else if (xmlReader.name().toString() == "Rotation")
        {
            QVERIFY(xmlReader.readElementText() == "78");
            rot = true;
        }
        else if (xmlReader.name().toString() == "IsRelative")
        {
            QVERIFY(xmlReader.readElementText() == "0");
            isRelative = true;
        }
        else if (xmlReader.name().toString() == "PropagationMode")
        {
            QVERIFY(xmlReader.readElementText() == "Serial");
            prop = true;
        }
        else if (xmlReader.name().toString() == "Axis")
        {
            bool axis = true;
            if (xmlReader.attributes().value("Name").toString() == "X")
                axis = true;
            else if (xmlReader.attributes().value("Name").toString() == "Y")
                axis = false;
            else
                QFAIL("Invalid axis!");

            while (xmlReader.readNextStartElement())
            {
                if (xmlReader.name().toString() == "Offset")
                {
                    if (axis == true)
                    {
                        QVERIFY(xmlReader.readElementText() == "34");
                        xoff = true;
                    }
                    else
                    {
                        QVERIFY(xmlReader.readElementText() == "27");
                        yoff = true;
                    }
                }
                else if (xmlReader.name().toString() == "Frequency")
                {
                    if (axis == true)
                    {
                        QVERIFY(xmlReader.readElementText() == "5");
                        xfreq = true;
                    }
                    else
                    {
                        QVERIFY(xmlReader.readElementText() == "4");
                        yfreq = true;
                    }
                }
                else if (xmlReader.name().toString() == "Phase")
                {
                    if (axis == true)
                    {
                        QVERIFY(xmlReader.readElementText() == "163");
                        xpha = true;
                    }
                    else
                    {
                        QVERIFY(xmlReader.readElementText() == "94");
                        ypha = true;
                    }
                }
                else
                {
                    QFAIL("Unexpected axis tag!");
                }
            }
        }
        else if (xmlReader.name().toString() == "Fixture")
        {
            int expectHead = 0;
            bool expectBackward = false;
            int expectedMode = 0;
            int expectStartOffset = 0;

            while (xmlReader.readNextStartElement())
            {
                if (xmlReader.name().toString() == "ID")
                {
                    QString text = xmlReader.readElementText();
                    if (fixtures.contains(text) == true)
                        QFAIL("Same fixture multiple times!");
                    else
                        fixtures.append(text);

                    if (text == "34")
                    {
                        expectHead = 5;
                        expectBackward = true;
                        expectStartOffset = 27;
                    }
                    else if (text == "12")
                    {
                        expectHead = 3;
                        expectBackward = false;
                        expectStartOffset = 0;
                    }
                    else
                    {
                        expectHead = 7;
                        expectBackward = false;
                        expectStartOffset = 0;
                    }

                    fixtureid++;
                }
                else if (xmlReader.name().toString() == "Head")
                {
                    QCOMPARE(xmlReader.readElementText().toInt(), expectHead);
                    fixturehead++;
                }
                else if (xmlReader.name().toString() == "Direction")
                {
                    QString text = xmlReader.readElementText();
                    if (expectBackward == false && text == "Backward")
                    {
                        QFAIL("Not expecting reversal!");
                    }

                    fixturedirection++;
                }
                else if (xmlReader.name().toString() == "StartOffset")
                {
                    QCOMPARE(xmlReader.readElementText().toInt(), expectStartOffset);
                    fixtureStartOffset++;
                }
                else if (xmlReader.name().toString() == "Mode")
                {
                    QCOMPARE(xmlReader.readElementText().toInt(), expectedMode);
                }
                else
                {
                    QFAIL("Unexpected fixture tag!");
                }
            }
        }
        else
        {
            QFAIL("Unexpected EFX tag!");
        }
    }

    QCOMPARE(fixtures.size(), 3);
    QCOMPARE(fixtureid, 3);
    QCOMPARE(fixturehead, 3);
    QCOMPARE(fixturedirection, 3);
    QCOMPARE(fixtureStartOffset, 3);
    QVERIFY(dir == true);
    QVERIFY(off == true);
    QVERIFY(run == true);
    QVERIFY(speed == true);
    QVERIFY(algo == true);
    QVERIFY(w == true);
    QVERIFY(h == true);
    QVERIFY(rot == true);
    QVERIFY(isRelative == true);
    QVERIFY(xoff == true);
    QVERIFY(yoff == true);
    QVERIFY(xfreq == true);
    QVERIFY(yfreq == true);
    QVERIFY(xpha == true);
    QVERIFY(ypha == true);
    QVERIFY(prop == true);
}

void EFX_Test::preRunPostRun()
{
    QList<Universe*> ua;
    ua.append(new Universe(0, new GrandMaster()));
    MasterTimerStub timer(m_doc, ua);

    EFX* e = new EFX(m_doc);
    e->setName("Test EFX");
    QVERIFY(e->m_fadersMap.count() == 0);

    QSignalSpy spy(e, SIGNAL(running(quint32)));
    e->preRun(&timer);
    QVERIFY(e->m_fadersMap.count() == 0);
    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy[0].size(), 1);
    QCOMPARE(spy[0][0].toUInt(), e->id());
    e->postRun(&timer, ua);
    QVERIFY(e->m_fadersMap.count() == 0);
}

void EFX_Test::adjustIntensity()
{
    /* Basically any fixture with 16bit pan & tilt channels will do, but
       then the exact channel numbers and mode name has to be changed
       below. */
    QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Martin", "MAC250+");
    QVERIFY(def != NULL);

    QLCFixtureMode* mode = def->mode("Mode 4");
    QVERIFY(mode != NULL);

    Fixture* fxi1 = new Fixture(m_doc);
    fxi1->setFixtureDefinition(def, mode);
    fxi1->setName("Test Scanner");
    fxi1->setAddress(0);
    fxi1->setUniverse(0);
    m_doc->addFixture(fxi1);

    Fixture* fxi2 = new Fixture(m_doc);
    fxi2->setFixtureDefinition(def, mode);
    fxi2->setName("Test Scanner");
    fxi2->setAddress(0);
    fxi2->setUniverse(1);
    m_doc->addFixture(fxi2);

    EFX* e = new EFX(m_doc);
    e->setName("Test EFX");

    EFXFixture* ef1 = new EFXFixture(e);
    ef1->setHead(GroupHead(fxi1->id(), 0));
    e->addFixture(ef1);

    EFXFixture* ef2 = new EFXFixture(e);
    ef2->setHead(GroupHead(fxi2->id(), 0));
    e->addFixture(ef2);

    e->adjustAttribute(0.2);
    QCOMPARE(e->getAttributeValue(Function::Intensity), 0.2);

    e->preRun(m_doc->masterTimer());

    e->adjustAttribute(0.5);
    QCOMPARE(e->getAttributeValue(Function::Intensity), 0.5);

    QList<Universe*> ua;
    ua.append(new Universe(0, new GrandMaster()));
    e->postRun(m_doc->masterTimer(), ua);
}

QTEST_APPLESS_MAIN(EFX_Test)
