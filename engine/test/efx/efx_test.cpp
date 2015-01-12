/*
  Q Light Controller - Unit test
  efx_test.cpp

  Copyright (c) Heikki Junnila

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
#include <QList>

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
    QVERIFY(m_doc->fixtureDefCache()->load(dir));
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
    QCOMPARE(e.type(), Function::EFX);
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

    QCOMPARE(e.m_fader, (GenericFader*)(NULL));
    QCOMPARE(e.m_legacyFadeBus, Bus::invalid());
    QCOMPARE(e.m_legacyHoldBus, Bus::invalid());
}

void EFX_Test::algorithmNames()
{
    QStringList list = EFX::algorithmList();
    QCOMPARE(list.size(), 9);
    QVERIFY(list.contains("Circle"));
    QVERIFY(list.contains("Eight"));
    QVERIFY(list.contains("Line"));
    QVERIFY(list.contains("Line2"));
    QVERIFY(list.contains("Diamond"));
    QVERIFY(list.contains("Square"));
    QVERIFY(list.contains("SquareChoppy"));
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
    ef1->setHead(GroupHead(12,0));
    QVERIFY(e->addFixture(ef1));
    QCOMPARE(e->fixtures().size(), 1);

    /* Add second fixture */
    EFXFixture* ef2 = new EFXFixture(e);
    ef2->setHead(GroupHead(34,0));
    QVERIFY(e->addFixture(ef2));
    QCOMPARE(e->fixtures().size(), 2);

    /* Must not be able to add the same fixture twice */
    QVERIFY(!e->addFixture(ef1));
    QVERIFY(!e->addFixture(ef2));
    QCOMPARE(e->fixtures().size(), 2);

    /* Try to remove a non-member fixture */
    EFXFixture* ef3 = new EFXFixture(e);
    ef3->setHead(GroupHead(56,0));
    QVERIFY(!e->removeFixture(ef3));
    QCOMPARE(e->fixtures().size(), 2);

    /* Add third fixture */
    e->addFixture(ef3);
    QCOMPARE(e->fixtures().size(), 3);
    QCOMPARE(e->fixtures().at(0), ef1);
    QCOMPARE(e->fixtures().at(1), ef2);
    QCOMPARE(e->fixtures().at(2), ef3);

    /* Add fourth fixture */
    EFXFixture* ef4 = new EFXFixture(e);
    ef4->setHead(GroupHead(78,0));
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

    QVector <QPoint> poly;
    e.preview(poly);
    QCOMPARE(poly.size(), 128);

    QCOMPARE(poly[0], QPoint(127,254));
    QCOMPARE(poly[1], QPoint(120,253));
    QCOMPARE(poly[2], QPoint(114,253));
    QCOMPARE(poly[3], QPoint(108,252));
    QCOMPARE(poly[4], QPoint(102,251));
    QCOMPARE(poly[5], QPoint(96,250));
    QCOMPARE(poly[6], QPoint(90,248));
    QCOMPARE(poly[7], QPoint(84,246));
    QCOMPARE(poly[8], QPoint(78,244));
    QCOMPARE(poly[9], QPoint(72,241));
    QCOMPARE(poly[10], QPoint(67,239));
    QCOMPARE(poly[11], QPoint(61,235));
    QCOMPARE(poly[12], QPoint(56,232));
    QCOMPARE(poly[13], QPoint(51,229));
    QCOMPARE(poly[14], QPoint(46,225));
    QCOMPARE(poly[15], QPoint(41,221));
    QCOMPARE(poly[16], QPoint(37,216));
    QCOMPARE(poly[17], QPoint(32,212));
    QCOMPARE(poly[18], QPoint(28,207));
    QCOMPARE(poly[19], QPoint(24,202));
    QCOMPARE(poly[20], QPoint(21,197));
    QCOMPARE(poly[21], QPoint(18,192));
    QCOMPARE(poly[22], QPoint(14,186));
    QCOMPARE(poly[23], QPoint(12,181));
    QCOMPARE(poly[24], QPoint(9,175));
    QCOMPARE(poly[25], QPoint(7,169));
    QCOMPARE(poly[26], QPoint(5,163));
    QCOMPARE(poly[27], QPoint(3,157));
    QCOMPARE(poly[28], QPoint(2,151));
    QCOMPARE(poly[29], QPoint(1,145));
    QCOMPARE(poly[30], QPoint(0,139));
    QCOMPARE(poly[31], QPoint(0,133));
    QCOMPARE(poly[32], QPoint(0,126));
    QCOMPARE(poly[33], QPoint(0,120));
    QCOMPARE(poly[34], QPoint(0,114));
    QCOMPARE(poly[35], QPoint(1,108));
    QCOMPARE(poly[36], QPoint(2,102));
    QCOMPARE(poly[37], QPoint(3,96));
    QCOMPARE(poly[38], QPoint(5,90));
    QCOMPARE(poly[39], QPoint(7,84));
    QCOMPARE(poly[40], QPoint(9,78));
    QCOMPARE(poly[41], QPoint(12,72));
    QCOMPARE(poly[42], QPoint(14,67));
    QCOMPARE(poly[43], QPoint(18,61));
    QCOMPARE(poly[44], QPoint(21,56));
    QCOMPARE(poly[45], QPoint(24,51));
    QCOMPARE(poly[46], QPoint(28,46));
    QCOMPARE(poly[47], QPoint(32,41));
    QCOMPARE(poly[48], QPoint(37,37));
    QCOMPARE(poly[49], QPoint(41,32));
    QCOMPARE(poly[50], QPoint(46,28));
    QCOMPARE(poly[51], QPoint(51,24));
    QCOMPARE(poly[52], QPoint(56,21));
    QCOMPARE(poly[53], QPoint(61,18));
    QCOMPARE(poly[54], QPoint(67,14));
    QCOMPARE(poly[55], QPoint(72,12));
    QCOMPARE(poly[56], QPoint(78,9));
    QCOMPARE(poly[57], QPoint(84,7));
    QCOMPARE(poly[58], QPoint(90,5));
    QCOMPARE(poly[59], QPoint(96,3));
    QCOMPARE(poly[60], QPoint(102,2));
    QCOMPARE(poly[61], QPoint(108,1));
    QCOMPARE(poly[62], QPoint(114,0));
    QCOMPARE(poly[63], QPoint(120,0));
    QCOMPARE(poly[64], QPoint(126,0));
    QCOMPARE(poly[65], QPoint(133,0));
    QCOMPARE(poly[66], QPoint(139,0));
    QCOMPARE(poly[67], QPoint(145,1));
    QCOMPARE(poly[68], QPoint(151,2));
    QCOMPARE(poly[69], QPoint(157,3));
    QCOMPARE(poly[70], QPoint(163,5));
    QCOMPARE(poly[71], QPoint(169,7));
    QCOMPARE(poly[72], QPoint(175,9));
    QCOMPARE(poly[73], QPoint(181,12));
    QCOMPARE(poly[74], QPoint(186,14));
    QCOMPARE(poly[75], QPoint(192,18));
    QCOMPARE(poly[76], QPoint(197,21));
    QCOMPARE(poly[77], QPoint(202,24));
    QCOMPARE(poly[78], QPoint(207,28));
    QCOMPARE(poly[79], QPoint(212,32));
    QCOMPARE(poly[80], QPoint(216,37));
    QCOMPARE(poly[81], QPoint(221,41));
    QCOMPARE(poly[82], QPoint(225,46));
    QCOMPARE(poly[83], QPoint(229,51));
    QCOMPARE(poly[84], QPoint(232,56));
    QCOMPARE(poly[85], QPoint(235,61));
    QCOMPARE(poly[86], QPoint(239,67));
    QCOMPARE(poly[87], QPoint(241,72));
    QCOMPARE(poly[88], QPoint(244,78));
    QCOMPARE(poly[89], QPoint(246,84));
    QCOMPARE(poly[90], QPoint(248,90));
    QCOMPARE(poly[91], QPoint(250,96));
    QCOMPARE(poly[92], QPoint(251,102));
    QCOMPARE(poly[93], QPoint(252,108));
    QCOMPARE(poly[94], QPoint(253,114));
    QCOMPARE(poly[95], QPoint(253,120));
    QCOMPARE(poly[96], QPoint(254,126));
    QCOMPARE(poly[97], QPoint(253,133));
    QCOMPARE(poly[98], QPoint(253,139));
    QCOMPARE(poly[99], QPoint(252,145));
    QCOMPARE(poly[100], QPoint(251,151));
    QCOMPARE(poly[101], QPoint(250,157));
    QCOMPARE(poly[102], QPoint(248,163));
    QCOMPARE(poly[103], QPoint(246,169));
    QCOMPARE(poly[104], QPoint(244,175));
    QCOMPARE(poly[105], QPoint(241,181));
    QCOMPARE(poly[106], QPoint(239,186));
    QCOMPARE(poly[107], QPoint(235,192));
    QCOMPARE(poly[108], QPoint(232,197));
    QCOMPARE(poly[109], QPoint(229,202));
    QCOMPARE(poly[110], QPoint(225,207));
    QCOMPARE(poly[111], QPoint(221,212));
    QCOMPARE(poly[112], QPoint(216,216));
    QCOMPARE(poly[113], QPoint(212,221));
    QCOMPARE(poly[114], QPoint(207,225));
    QCOMPARE(poly[115], QPoint(202,229));
    QCOMPARE(poly[116], QPoint(197,232));
    QCOMPARE(poly[117], QPoint(192,235));
    QCOMPARE(poly[118], QPoint(186,239));
    QCOMPARE(poly[119], QPoint(181,241));
    QCOMPARE(poly[120], QPoint(175,244));
    QCOMPARE(poly[121], QPoint(169,246));
    QCOMPARE(poly[122], QPoint(163,248));
    QCOMPARE(poly[123], QPoint(157,250));
    QCOMPARE(poly[124], QPoint(151,251));
    QCOMPARE(poly[125], QPoint(145,252));
    QCOMPARE(poly[126], QPoint(139,253));
    QCOMPARE(poly[127], QPoint(133,253));
}

void EFX_Test::previewEight()
{
    EFX e(m_doc);
    e.setAlgorithm(EFX::Eight);

    QVector <QPoint> poly;
    e.preview(poly);
    QCOMPARE(poly.size(), 128);

    QCOMPARE(poly[0], QPoint(127,254));
    QCOMPARE(poly[1], QPoint(114,253));
    QCOMPARE(poly[2], QPoint(102,253));
    QCOMPARE(poly[3], QPoint(90,252));
    QCOMPARE(poly[4], QPoint(78,251));
    QCOMPARE(poly[5], QPoint(67,250));
    QCOMPARE(poly[6], QPoint(56,248));
    QCOMPARE(poly[7], QPoint(46,246));
    QCOMPARE(poly[8], QPoint(37,244));
    QCOMPARE(poly[9], QPoint(28,241));
    QCOMPARE(poly[10], QPoint(21,239));
    QCOMPARE(poly[11], QPoint(14,235));
    QCOMPARE(poly[12], QPoint(9,232));
    QCOMPARE(poly[13], QPoint(5,229));
    QCOMPARE(poly[14], QPoint(2,225));
    QCOMPARE(poly[15], QPoint(0,221));
    QCOMPARE(poly[16], QPoint(0,216));
    QCOMPARE(poly[17], QPoint(0,212));
    QCOMPARE(poly[18], QPoint(2,207));
    QCOMPARE(poly[19], QPoint(5,202));
    QCOMPARE(poly[20], QPoint(9,197));
    QCOMPARE(poly[21], QPoint(14,192));
    QCOMPARE(poly[22], QPoint(21,186));
    QCOMPARE(poly[23], QPoint(28,181));
    QCOMPARE(poly[24], QPoint(37,175));
    QCOMPARE(poly[25], QPoint(46,169));
    QCOMPARE(poly[26], QPoint(56,163));
    QCOMPARE(poly[27], QPoint(67,157));
    QCOMPARE(poly[28], QPoint(78,151));
    QCOMPARE(poly[29], QPoint(90,145));
    QCOMPARE(poly[30], QPoint(102,139));
    QCOMPARE(poly[31], QPoint(114,133));
    QCOMPARE(poly[32], QPoint(127,126));
    QCOMPARE(poly[33], QPoint(139,120));
    QCOMPARE(poly[34], QPoint(151,114));
    QCOMPARE(poly[35], QPoint(163,108));
    QCOMPARE(poly[36], QPoint(175,102));
    QCOMPARE(poly[37], QPoint(186,96));
    QCOMPARE(poly[38], QPoint(197,90));
    QCOMPARE(poly[39], QPoint(207,84));
    QCOMPARE(poly[40], QPoint(216,78));
    QCOMPARE(poly[41], QPoint(225,72));
    QCOMPARE(poly[42], QPoint(232,67));
    QCOMPARE(poly[43], QPoint(239,61));
    QCOMPARE(poly[44], QPoint(244,56));
    QCOMPARE(poly[45], QPoint(248,51));
    QCOMPARE(poly[46], QPoint(251,46));
    QCOMPARE(poly[47], QPoint(253,41));
    QCOMPARE(poly[48], QPoint(254,37));
    QCOMPARE(poly[49], QPoint(253,32));
    QCOMPARE(poly[50], QPoint(251,28));
    QCOMPARE(poly[51], QPoint(248,24));
    QCOMPARE(poly[52], QPoint(244,21));
    QCOMPARE(poly[53], QPoint(239,18));
    QCOMPARE(poly[54], QPoint(232,14));
    QCOMPARE(poly[55], QPoint(225,12));
    QCOMPARE(poly[56], QPoint(216,9));
    QCOMPARE(poly[57], QPoint(207,7));
    QCOMPARE(poly[58], QPoint(197,5));
    QCOMPARE(poly[59], QPoint(186,3));
    QCOMPARE(poly[60], QPoint(175,2));
    QCOMPARE(poly[61], QPoint(163,1));
    QCOMPARE(poly[62], QPoint(151,0));
    QCOMPARE(poly[63], QPoint(139,0));
    QCOMPARE(poly[64], QPoint(127,0));
    QCOMPARE(poly[65], QPoint(114,0));
    QCOMPARE(poly[66], QPoint(102,0));
    QCOMPARE(poly[67], QPoint(90,1));
    QCOMPARE(poly[68], QPoint(78,2));
    QCOMPARE(poly[69], QPoint(67,3));
    QCOMPARE(poly[70], QPoint(56,5));
    QCOMPARE(poly[71], QPoint(46,7));
    QCOMPARE(poly[72], QPoint(37,9));
    QCOMPARE(poly[73], QPoint(28,12));
    QCOMPARE(poly[74], QPoint(21,14));
    QCOMPARE(poly[75], QPoint(14,18));
    QCOMPARE(poly[76], QPoint(9,21));
    QCOMPARE(poly[77], QPoint(5,24));
    QCOMPARE(poly[78], QPoint(2,28));
    QCOMPARE(poly[79], QPoint(0,32));
    QCOMPARE(poly[80], QPoint(0,37));
    QCOMPARE(poly[81], QPoint(0,41));
    QCOMPARE(poly[82], QPoint(2,46));
    QCOMPARE(poly[83], QPoint(5,51));
    QCOMPARE(poly[84], QPoint(9,56));
    QCOMPARE(poly[85], QPoint(14,61));
    QCOMPARE(poly[86], QPoint(21,67));
    QCOMPARE(poly[87], QPoint(28,72));
    QCOMPARE(poly[88], QPoint(37,78));
    QCOMPARE(poly[89], QPoint(46,84));
    QCOMPARE(poly[90], QPoint(56,90));
    QCOMPARE(poly[91], QPoint(67,96));
    QCOMPARE(poly[92], QPoint(78,102));
    QCOMPARE(poly[93], QPoint(90,108));
    QCOMPARE(poly[94], QPoint(102,114));
    QCOMPARE(poly[95], QPoint(114,120));
    QCOMPARE(poly[96], QPoint(126,126));
    QCOMPARE(poly[97], QPoint(139,133));
    QCOMPARE(poly[98], QPoint(151,139));
    QCOMPARE(poly[99], QPoint(163,145));
    QCOMPARE(poly[100], QPoint(175,151));
    QCOMPARE(poly[101], QPoint(186,157));
    QCOMPARE(poly[102], QPoint(197,163));
    QCOMPARE(poly[103], QPoint(207,169));
    QCOMPARE(poly[104], QPoint(216,175));
    QCOMPARE(poly[105], QPoint(225,181));
    QCOMPARE(poly[106], QPoint(232,186));
    QCOMPARE(poly[107], QPoint(239,192));
    QCOMPARE(poly[108], QPoint(244,197));
    QCOMPARE(poly[109], QPoint(248,202));
    QCOMPARE(poly[110], QPoint(251,207));
    QCOMPARE(poly[111], QPoint(253,212));
    QCOMPARE(poly[112], QPoint(254,216));
    QCOMPARE(poly[113], QPoint(253,221));
    QCOMPARE(poly[114], QPoint(251,225));
    QCOMPARE(poly[115], QPoint(248,229));
    QCOMPARE(poly[116], QPoint(244,232));
    QCOMPARE(poly[117], QPoint(239,235));
    QCOMPARE(poly[118], QPoint(232,239));
    QCOMPARE(poly[119], QPoint(225,241));
    QCOMPARE(poly[120], QPoint(216,244));
    QCOMPARE(poly[121], QPoint(207,246));
    QCOMPARE(poly[122], QPoint(197,248));
    QCOMPARE(poly[123], QPoint(186,250));
    QCOMPARE(poly[124], QPoint(175,251));
    QCOMPARE(poly[125], QPoint(163,252));
    QCOMPARE(poly[126], QPoint(151,253));
    QCOMPARE(poly[127], QPoint(139,253));
}

void EFX_Test::previewLine()
{
    EFX e(m_doc);
    e.setAlgorithm(EFX::Line);

    QVector <QPoint> poly;
    e.preview(poly);
    QCOMPARE(poly.size(), 128);

    QCOMPARE(poly[0], QPoint(254,254));
    QCOMPARE(poly[1], QPoint(253,253));
    QCOMPARE(poly[2], QPoint(253,253));
    QCOMPARE(poly[3], QPoint(252,252));
    QCOMPARE(poly[4], QPoint(251,251));
    QCOMPARE(poly[5], QPoint(250,250));
    QCOMPARE(poly[6], QPoint(248,248));
    QCOMPARE(poly[7], QPoint(246,246));
    QCOMPARE(poly[8], QPoint(244,244));
    QCOMPARE(poly[9], QPoint(241,241));
    QCOMPARE(poly[10], QPoint(239,239));
    QCOMPARE(poly[11], QPoint(235,235));
    QCOMPARE(poly[12], QPoint(232,232));
    QCOMPARE(poly[13], QPoint(229,229));
    QCOMPARE(poly[14], QPoint(225,225));
    QCOMPARE(poly[15], QPoint(221,221));
    QCOMPARE(poly[16], QPoint(216,216));
    QCOMPARE(poly[17], QPoint(212,212));
    QCOMPARE(poly[18], QPoint(207,207));
    QCOMPARE(poly[19], QPoint(202,202));
    QCOMPARE(poly[20], QPoint(197,197));
    QCOMPARE(poly[21], QPoint(192,192));
    QCOMPARE(poly[22], QPoint(186,186));
    QCOMPARE(poly[23], QPoint(181,181));
    QCOMPARE(poly[24], QPoint(175,175));
    QCOMPARE(poly[25], QPoint(169,169));
    QCOMPARE(poly[26], QPoint(163,163));
    QCOMPARE(poly[27], QPoint(157,157));
    QCOMPARE(poly[28], QPoint(151,151));
    QCOMPARE(poly[29], QPoint(145,145));
    QCOMPARE(poly[30], QPoint(139,139));
    QCOMPARE(poly[31], QPoint(133,133));
    QCOMPARE(poly[32], QPoint(126,126));
    QCOMPARE(poly[33], QPoint(120,120));
    QCOMPARE(poly[34], QPoint(114,114));
    QCOMPARE(poly[35], QPoint(108,108));
    QCOMPARE(poly[36], QPoint(102,102));
    QCOMPARE(poly[37], QPoint(96,96));
    QCOMPARE(poly[38], QPoint(90,90));
    QCOMPARE(poly[39], QPoint(84,84));
    QCOMPARE(poly[40], QPoint(78,78));
    QCOMPARE(poly[41], QPoint(72,72));
    QCOMPARE(poly[42], QPoint(67,67));
    QCOMPARE(poly[43], QPoint(61,61));
    QCOMPARE(poly[44], QPoint(56,56));
    QCOMPARE(poly[45], QPoint(51,51));
    QCOMPARE(poly[46], QPoint(46,46));
    QCOMPARE(poly[47], QPoint(41,41));
    QCOMPARE(poly[48], QPoint(37,37));
    QCOMPARE(poly[49], QPoint(32,32));
    QCOMPARE(poly[50], QPoint(28,28));
    QCOMPARE(poly[51], QPoint(24,24));
    QCOMPARE(poly[52], QPoint(21,21));
    QCOMPARE(poly[53], QPoint(18,18));
    QCOMPARE(poly[54], QPoint(14,14));
    QCOMPARE(poly[55], QPoint(12,12));
    QCOMPARE(poly[56], QPoint(9,9));
    QCOMPARE(poly[57], QPoint(7,7));
    QCOMPARE(poly[58], QPoint(5,5));
    QCOMPARE(poly[59], QPoint(3,3));
    QCOMPARE(poly[60], QPoint(2,2));
    QCOMPARE(poly[61], QPoint(1,1));
    QCOMPARE(poly[62], QPoint(0,0));
    QCOMPARE(poly[63], QPoint(0,0));
    QCOMPARE(poly[64], QPoint(0,0));
    QCOMPARE(poly[65], QPoint(0,0));
    QCOMPARE(poly[66], QPoint(0,0));
    QCOMPARE(poly[67], QPoint(1,1));
    QCOMPARE(poly[68], QPoint(2,2));
    QCOMPARE(poly[69], QPoint(3,3));
    QCOMPARE(poly[70], QPoint(5,5));
    QCOMPARE(poly[71], QPoint(7,7));
    QCOMPARE(poly[72], QPoint(9,9));
    QCOMPARE(poly[73], QPoint(12,12));
    QCOMPARE(poly[74], QPoint(14,14));
    QCOMPARE(poly[75], QPoint(18,18));
    QCOMPARE(poly[76], QPoint(21,21));
    QCOMPARE(poly[77], QPoint(24,24));
    QCOMPARE(poly[78], QPoint(28,28));
    QCOMPARE(poly[79], QPoint(32,32));
    QCOMPARE(poly[80], QPoint(37,37));
    QCOMPARE(poly[81], QPoint(41,41));
    QCOMPARE(poly[82], QPoint(46,46));
    QCOMPARE(poly[83], QPoint(51,51));
    QCOMPARE(poly[84], QPoint(56,56));
    QCOMPARE(poly[85], QPoint(61,61));
    QCOMPARE(poly[86], QPoint(67,67));
    QCOMPARE(poly[87], QPoint(72,72));
    QCOMPARE(poly[88], QPoint(78,78));
    QCOMPARE(poly[89], QPoint(84,84));
    QCOMPARE(poly[90], QPoint(90,90));
    QCOMPARE(poly[91], QPoint(96,96));
    QCOMPARE(poly[92], QPoint(102,102));
    QCOMPARE(poly[93], QPoint(108,108));
    QCOMPARE(poly[94], QPoint(114,114));
    QCOMPARE(poly[95], QPoint(120,120));
    QCOMPARE(poly[96], QPoint(126,126));
    QCOMPARE(poly[97], QPoint(133,133));
    QCOMPARE(poly[98], QPoint(139,139));
    QCOMPARE(poly[99], QPoint(145,145));
    QCOMPARE(poly[100], QPoint(151,151));
    QCOMPARE(poly[101], QPoint(157,157));
    QCOMPARE(poly[102], QPoint(163,163));
    QCOMPARE(poly[103], QPoint(169,169));
    QCOMPARE(poly[104], QPoint(175,175));
    QCOMPARE(poly[105], QPoint(181,181));
    QCOMPARE(poly[106], QPoint(186,186));
    QCOMPARE(poly[107], QPoint(192,192));
    QCOMPARE(poly[108], QPoint(197,197));
    QCOMPARE(poly[109], QPoint(202,202));
    QCOMPARE(poly[110], QPoint(207,207));
    QCOMPARE(poly[111], QPoint(212,212));
    QCOMPARE(poly[112], QPoint(216,216));
    QCOMPARE(poly[113], QPoint(221,221));
    QCOMPARE(poly[114], QPoint(225,225));
    QCOMPARE(poly[115], QPoint(229,229));
    QCOMPARE(poly[116], QPoint(232,232));
    QCOMPARE(poly[117], QPoint(235,235));
    QCOMPARE(poly[118], QPoint(239,239));
    QCOMPARE(poly[119], QPoint(241,241));
    QCOMPARE(poly[120], QPoint(244,244));
    QCOMPARE(poly[121], QPoint(246,246));
    QCOMPARE(poly[122], QPoint(248,248));
    QCOMPARE(poly[123], QPoint(250,250));
    QCOMPARE(poly[124], QPoint(251,251));
    QCOMPARE(poly[125], QPoint(252,252));
    QCOMPARE(poly[126], QPoint(253,253));
    QCOMPARE(poly[127], QPoint(253,253));
}

void EFX_Test::previewLine2()
{
    EFX e(m_doc);
    e.setAlgorithm(EFX::Line2);

    QVector <QPoint> poly;
    e.preview(poly);
    QCOMPARE(poly.size(), 128);

    QCOMPARE(poly[0], QPoint(0,0));
    QCOMPARE(poly[1], QPoint(1,1));
    QCOMPARE(poly[2], QPoint(3,3));
    QCOMPARE(poly[3], QPoint(5,5));
    QCOMPARE(poly[4], QPoint(7,7));
    QCOMPARE(poly[5], QPoint(9,9));
    QCOMPARE(poly[6], QPoint(11,11));
    QCOMPARE(poly[7], QPoint(13,13));
    QCOMPARE(poly[8], QPoint(15,15));
    QCOMPARE(poly[9], QPoint(17,17));
    QCOMPARE(poly[10], QPoint(19,19));
    QCOMPARE(poly[11], QPoint(21,21));
    QCOMPARE(poly[12], QPoint(23,23));
    QCOMPARE(poly[13], QPoint(25,25));
    QCOMPARE(poly[14], QPoint(27,27));
    QCOMPARE(poly[15], QPoint(29,29));
    QCOMPARE(poly[16], QPoint(31,31));
    QCOMPARE(poly[17], QPoint(33,33));
    QCOMPARE(poly[18], QPoint(35,35));
    QCOMPARE(poly[19], QPoint(37,37));
    QCOMPARE(poly[20], QPoint(39,39));
    QCOMPARE(poly[21], QPoint(41,41));
    QCOMPARE(poly[22], QPoint(43,43));
    QCOMPARE(poly[23], QPoint(45,45));
    QCOMPARE(poly[24], QPoint(47,47));
    QCOMPARE(poly[25], QPoint(49,49));
    QCOMPARE(poly[26], QPoint(51,51));
    QCOMPARE(poly[27], QPoint(53,53));
    QCOMPARE(poly[28], QPoint(55,55));
    QCOMPARE(poly[29], QPoint(57,57));
    QCOMPARE(poly[30], QPoint(59,59));
    QCOMPARE(poly[31], QPoint(61,61));
    QCOMPARE(poly[32], QPoint(63,63));
    QCOMPARE(poly[33], QPoint(65,65));
    QCOMPARE(poly[34], QPoint(67,67));
    QCOMPARE(poly[35], QPoint(69,69));
    QCOMPARE(poly[36], QPoint(71,71));
    QCOMPARE(poly[37], QPoint(73,73));
    QCOMPARE(poly[38], QPoint(75,75));
    QCOMPARE(poly[39], QPoint(77,77));
    QCOMPARE(poly[40], QPoint(79,79));
    QCOMPARE(poly[41], QPoint(81,81));
    QCOMPARE(poly[42], QPoint(83,83));
    QCOMPARE(poly[43], QPoint(85,85));
    QCOMPARE(poly[44], QPoint(87,87));
    QCOMPARE(poly[45], QPoint(89,89));
    QCOMPARE(poly[46], QPoint(91,91));
    QCOMPARE(poly[47], QPoint(93,93));
    QCOMPARE(poly[48], QPoint(95,95));
    QCOMPARE(poly[49], QPoint(97,97));
    QCOMPARE(poly[50], QPoint(99,99));
    QCOMPARE(poly[51], QPoint(101,101));
    QCOMPARE(poly[52], QPoint(103,103));
    QCOMPARE(poly[53], QPoint(105,105));
    QCOMPARE(poly[54], QPoint(107,107));
    QCOMPARE(poly[55], QPoint(109,109));
    QCOMPARE(poly[56], QPoint(111,111));
    QCOMPARE(poly[57], QPoint(113,113));
    QCOMPARE(poly[58], QPoint(115,115));
    QCOMPARE(poly[59], QPoint(117,117));
    QCOMPARE(poly[60], QPoint(119,119));
    QCOMPARE(poly[61], QPoint(121,121));
    QCOMPARE(poly[62], QPoint(123,123));
    QCOMPARE(poly[63], QPoint(125,125));
    QCOMPARE(poly[64], QPoint(126,126));
    QCOMPARE(poly[65], QPoint(128,128));
    QCOMPARE(poly[66], QPoint(130,130));
    QCOMPARE(poly[67], QPoint(132,132));
    QCOMPARE(poly[68], QPoint(134,134));
    QCOMPARE(poly[69], QPoint(136,136));
    QCOMPARE(poly[70], QPoint(138,138));
    QCOMPARE(poly[71], QPoint(140,140));
    QCOMPARE(poly[72], QPoint(142,142));
    QCOMPARE(poly[73], QPoint(144,144));
    QCOMPARE(poly[74], QPoint(146,146));
    QCOMPARE(poly[75], QPoint(148,148));
    QCOMPARE(poly[76], QPoint(150,150));
    QCOMPARE(poly[77], QPoint(152,152));
    QCOMPARE(poly[78], QPoint(154,154));
    QCOMPARE(poly[79], QPoint(156,156));
    QCOMPARE(poly[80], QPoint(158,158));
    QCOMPARE(poly[81], QPoint(160,160));
    QCOMPARE(poly[82], QPoint(162,162));
    QCOMPARE(poly[83], QPoint(164,164));
    QCOMPARE(poly[84], QPoint(166,166));
    QCOMPARE(poly[85], QPoint(168,168));
    QCOMPARE(poly[86], QPoint(170,170));
    QCOMPARE(poly[87], QPoint(172,172));
    QCOMPARE(poly[88], QPoint(174,174));
    QCOMPARE(poly[89], QPoint(176,176));
    QCOMPARE(poly[90], QPoint(178,178));
    QCOMPARE(poly[91], QPoint(180,180));
    QCOMPARE(poly[92], QPoint(182,182));
    QCOMPARE(poly[93], QPoint(184,184));
    QCOMPARE(poly[94], QPoint(186,186));
    QCOMPARE(poly[95], QPoint(188,188));
    QCOMPARE(poly[96], QPoint(190,190));
    QCOMPARE(poly[97], QPoint(192,192));
    QCOMPARE(poly[98], QPoint(194,194));
    QCOMPARE(poly[99], QPoint(196,196));
    QCOMPARE(poly[100], QPoint(198,198));
    QCOMPARE(poly[101], QPoint(200,200));
    QCOMPARE(poly[102], QPoint(202,202));
    QCOMPARE(poly[103], QPoint(204,204));
    QCOMPARE(poly[104], QPoint(206,206));
    QCOMPARE(poly[105], QPoint(208,208));
    QCOMPARE(poly[106], QPoint(210,210));
    QCOMPARE(poly[107], QPoint(212,212));
    QCOMPARE(poly[108], QPoint(214,214));
    QCOMPARE(poly[109], QPoint(216,216));
    QCOMPARE(poly[110], QPoint(218,218));
    QCOMPARE(poly[111], QPoint(220,220));
    QCOMPARE(poly[112], QPoint(222,222));
    QCOMPARE(poly[113], QPoint(224,224));
    QCOMPARE(poly[114], QPoint(226,226));
    QCOMPARE(poly[115], QPoint(228,228));
    QCOMPARE(poly[116], QPoint(230,230));
    QCOMPARE(poly[117], QPoint(232,232));
    QCOMPARE(poly[118], QPoint(234,234));
    QCOMPARE(poly[119], QPoint(236,236));
    QCOMPARE(poly[120], QPoint(238,238));
    QCOMPARE(poly[121], QPoint(240,240));
    QCOMPARE(poly[122], QPoint(242,242));
    QCOMPARE(poly[123], QPoint(244,244));
    QCOMPARE(poly[124], QPoint(246,246));
    QCOMPARE(poly[125], QPoint(248,248));
    QCOMPARE(poly[126], QPoint(250,250));
    QCOMPARE(poly[127], QPoint(252,252));
}

void EFX_Test::previewDiamond()
{
    EFX e(m_doc);
    e.setAlgorithm(EFX::Diamond);

    QVector <QPoint> poly;
    e.preview(poly);
    QCOMPARE(poly.size(), 128);

    QCOMPARE(poly[0], QPoint(127,254));
    QCOMPARE(poly[1], QPoint(127,253));
    QCOMPARE(poly[2], QPoint(127,252));
    QCOMPARE(poly[3], QPoint(127,249));
    QCOMPARE(poly[4], QPoint(127,246));
    QCOMPARE(poly[5], QPoint(128,242));
    QCOMPARE(poly[6], QPoint(130,238));
    QCOMPARE(poly[7], QPoint(131,233));
    QCOMPARE(poly[8], QPoint(134,227));
    QCOMPARE(poly[9], QPoint(136,220));
    QCOMPARE(poly[10], QPoint(140,214));
    QCOMPARE(poly[11], QPoint(144,207));
    QCOMPARE(poly[12], QPoint(148,200));
    QCOMPARE(poly[13], QPoint(153,192));
    QCOMPARE(poly[14], QPoint(159,185));
    QCOMPARE(poly[15], QPoint(165,178));
    QCOMPARE(poly[16], QPoint(171,171));
    QCOMPARE(poly[17], QPoint(178,165));
    QCOMPARE(poly[18], QPoint(185,159));
    QCOMPARE(poly[19], QPoint(192,153));
    QCOMPARE(poly[20], QPoint(200,148));
    QCOMPARE(poly[21], QPoint(207,144));
    QCOMPARE(poly[22], QPoint(214,140));
    QCOMPARE(poly[23], QPoint(220,136));
    QCOMPARE(poly[24], QPoint(227,134));
    QCOMPARE(poly[25], QPoint(233,131));
    QCOMPARE(poly[26], QPoint(238,130));
    QCOMPARE(poly[27], QPoint(242,128));
    QCOMPARE(poly[28], QPoint(246,127));
    QCOMPARE(poly[29], QPoint(249,127));
    QCOMPARE(poly[30], QPoint(252,127));
    QCOMPARE(poly[31], QPoint(253,127));
    QCOMPARE(poly[32], QPoint(254,127));
    QCOMPARE(poly[33], QPoint(253,126));
    QCOMPARE(poly[34], QPoint(252,126));
    QCOMPARE(poly[35], QPoint(249,126));
    QCOMPARE(poly[36], QPoint(246,126));
    QCOMPARE(poly[37], QPoint(242,125));
    QCOMPARE(poly[38], QPoint(238,123));
    QCOMPARE(poly[39], QPoint(233,122));
    QCOMPARE(poly[40], QPoint(227,119));
    QCOMPARE(poly[41], QPoint(220,117));
    QCOMPARE(poly[42], QPoint(214,113));
    QCOMPARE(poly[43], QPoint(207,109));
    QCOMPARE(poly[44], QPoint(200,105));
    QCOMPARE(poly[45], QPoint(192,100));
    QCOMPARE(poly[46], QPoint(185,94));
    QCOMPARE(poly[47], QPoint(178,88));
    QCOMPARE(poly[48], QPoint(171,82));
    QCOMPARE(poly[49], QPoint(165,75));
    QCOMPARE(poly[50], QPoint(159,68));
    QCOMPARE(poly[51], QPoint(153,61));
    QCOMPARE(poly[52], QPoint(148,53));
    QCOMPARE(poly[53], QPoint(144,46));
    QCOMPARE(poly[54], QPoint(140,39));
    QCOMPARE(poly[55], QPoint(136,33));
    QCOMPARE(poly[56], QPoint(134,26));
    QCOMPARE(poly[57], QPoint(131,20));
    QCOMPARE(poly[58], QPoint(130,15));
    QCOMPARE(poly[59], QPoint(128,11));
    QCOMPARE(poly[60], QPoint(127,7));
    QCOMPARE(poly[61], QPoint(127,4));
    QCOMPARE(poly[62], QPoint(127,1));
    QCOMPARE(poly[63], QPoint(127,0));
    QCOMPARE(poly[64], QPoint(127,0));
    QCOMPARE(poly[65], QPoint(126,0));
    QCOMPARE(poly[66], QPoint(126,1));
    QCOMPARE(poly[67], QPoint(126,4));
    QCOMPARE(poly[68], QPoint(126,7));
    QCOMPARE(poly[69], QPoint(125,11));
    QCOMPARE(poly[70], QPoint(123,15));
    QCOMPARE(poly[71], QPoint(122,20));
    QCOMPARE(poly[72], QPoint(119,26));
    QCOMPARE(poly[73], QPoint(117,33));
    QCOMPARE(poly[74], QPoint(113,39));
    QCOMPARE(poly[75], QPoint(109,46));
    QCOMPARE(poly[76], QPoint(105,53));
    QCOMPARE(poly[77], QPoint(100,61));
    QCOMPARE(poly[78], QPoint(94,68));
    QCOMPARE(poly[79], QPoint(88,75));
    QCOMPARE(poly[80], QPoint(82,82));
    QCOMPARE(poly[81], QPoint(75,88));
    QCOMPARE(poly[82], QPoint(68,94));
    QCOMPARE(poly[83], QPoint(61,100));
    QCOMPARE(poly[84], QPoint(53,105));
    QCOMPARE(poly[85], QPoint(46,109));
    QCOMPARE(poly[86], QPoint(39,113));
    QCOMPARE(poly[87], QPoint(33,117));
    QCOMPARE(poly[88], QPoint(26,119));
    QCOMPARE(poly[89], QPoint(20,122));
    QCOMPARE(poly[90], QPoint(15,123));
    QCOMPARE(poly[91], QPoint(11,125));
    QCOMPARE(poly[92], QPoint(7,126));
    QCOMPARE(poly[93], QPoint(4,126));
    QCOMPARE(poly[94], QPoint(1,126));
    QCOMPARE(poly[95], QPoint(0,126));
    QCOMPARE(poly[96], QPoint(0,127));
    QCOMPARE(poly[97], QPoint(0,127));
    QCOMPARE(poly[98], QPoint(1,127));
    QCOMPARE(poly[99], QPoint(4,127));
    QCOMPARE(poly[100], QPoint(7,127));
    QCOMPARE(poly[101], QPoint(11,128));
    QCOMPARE(poly[102], QPoint(15,130));
    QCOMPARE(poly[103], QPoint(20,131));
    QCOMPARE(poly[104], QPoint(26,134));
    QCOMPARE(poly[105], QPoint(33,136));
    QCOMPARE(poly[106], QPoint(39,140));
    QCOMPARE(poly[107], QPoint(46,144));
    QCOMPARE(poly[108], QPoint(53,148));
    QCOMPARE(poly[109], QPoint(61,153));
    QCOMPARE(poly[110], QPoint(68,159));
    QCOMPARE(poly[111], QPoint(75,165));
    QCOMPARE(poly[112], QPoint(82,171));
    QCOMPARE(poly[113], QPoint(88,178));
    QCOMPARE(poly[114], QPoint(94,185));
    QCOMPARE(poly[115], QPoint(100,192));
    QCOMPARE(poly[116], QPoint(105,200));
    QCOMPARE(poly[117], QPoint(109,207));
    QCOMPARE(poly[118], QPoint(113,214));
    QCOMPARE(poly[119], QPoint(117,220));
    QCOMPARE(poly[120], QPoint(119,227));
    QCOMPARE(poly[121], QPoint(122,233));
    QCOMPARE(poly[122], QPoint(123,238));
    QCOMPARE(poly[123], QPoint(125,242));
    QCOMPARE(poly[124], QPoint(126,246));
    QCOMPARE(poly[125], QPoint(126,249));
    QCOMPARE(poly[126], QPoint(126,252));
    QCOMPARE(poly[127], QPoint(126,253));
}

void EFX_Test::previewSquare()
{
    EFX e(m_doc);
    e.setAlgorithm(EFX::Square);

    QVector <QPoint> poly;
    e.preview(poly);
    QCOMPARE(poly.size(), 128);

    QCOMPARE(poly[0], QPoint(0,254));
    QCOMPARE(poly[1], QPoint(7,254));
    QCOMPARE(poly[2], QPoint(15,254));
    QCOMPARE(poly[3], QPoint(23,254));
    QCOMPARE(poly[4], QPoint(31,254));
    QCOMPARE(poly[5], QPoint(39,254));
    QCOMPARE(poly[6], QPoint(47,254));
    QCOMPARE(poly[7], QPoint(55,254));
    QCOMPARE(poly[8], QPoint(63,254));
    QCOMPARE(poly[9], QPoint(71,254));
    QCOMPARE(poly[10], QPoint(79,254));
    QCOMPARE(poly[11], QPoint(87,254));
    QCOMPARE(poly[12], QPoint(95,254));
    QCOMPARE(poly[13], QPoint(103,254));
    QCOMPARE(poly[14], QPoint(111,254));
    QCOMPARE(poly[15], QPoint(119,254));
    QCOMPARE(poly[16], QPoint(127,254));
    QCOMPARE(poly[17], QPoint(134,254));
    QCOMPARE(poly[18], QPoint(142,254));
    QCOMPARE(poly[19], QPoint(150,254));
    QCOMPARE(poly[20], QPoint(158,254));
    QCOMPARE(poly[21], QPoint(166,254));
    QCOMPARE(poly[22], QPoint(174,254));
    QCOMPARE(poly[23], QPoint(182,254));
    QCOMPARE(poly[24], QPoint(190,254));
    QCOMPARE(poly[25], QPoint(198,254));
    QCOMPARE(poly[26], QPoint(206,254));
    QCOMPARE(poly[27], QPoint(214,254));
    QCOMPARE(poly[28], QPoint(222,254));
    QCOMPARE(poly[29], QPoint(230,254));
    QCOMPARE(poly[30], QPoint(238,254));
    QCOMPARE(poly[31], QPoint(246,254));
    QCOMPARE(poly[32], QPoint(254,253));
    QCOMPARE(poly[33], QPoint(254,246));
    QCOMPARE(poly[34], QPoint(254,238));
    QCOMPARE(poly[35], QPoint(254,230));
    QCOMPARE(poly[36], QPoint(254,222));
    QCOMPARE(poly[37], QPoint(254,214));
    QCOMPARE(poly[38], QPoint(254,206));
    QCOMPARE(poly[39], QPoint(254,198));
    QCOMPARE(poly[40], QPoint(254,190));
    QCOMPARE(poly[41], QPoint(254,182));
    QCOMPARE(poly[42], QPoint(254,174));
    QCOMPARE(poly[43], QPoint(254,166));
    QCOMPARE(poly[44], QPoint(254,158));
    QCOMPARE(poly[45], QPoint(254,150));
    QCOMPARE(poly[46], QPoint(254,142));
    QCOMPARE(poly[47], QPoint(254,134));
    QCOMPARE(poly[48], QPoint(254,126));
    QCOMPARE(poly[49], QPoint(254,119));
    QCOMPARE(poly[50], QPoint(254,111));
    QCOMPARE(poly[51], QPoint(254,103));
    QCOMPARE(poly[52], QPoint(254,95));
    QCOMPARE(poly[53], QPoint(254,87));
    QCOMPARE(poly[54], QPoint(254,79));
    QCOMPARE(poly[55], QPoint(254,71));
    QCOMPARE(poly[56], QPoint(254,63));
    QCOMPARE(poly[57], QPoint(254,55));
    QCOMPARE(poly[58], QPoint(254,47));
    QCOMPARE(poly[59], QPoint(254,39));
    QCOMPARE(poly[60], QPoint(254,31));
    QCOMPARE(poly[61], QPoint(254,23));
    QCOMPARE(poly[62], QPoint(254,15));
    QCOMPARE(poly[63], QPoint(254,7));
    QCOMPARE(poly[64], QPoint(254,0));
    QCOMPARE(poly[65], QPoint(246,0));
    QCOMPARE(poly[66], QPoint(238,0));
    QCOMPARE(poly[67], QPoint(230,0));
    QCOMPARE(poly[68], QPoint(222,0));
    QCOMPARE(poly[69], QPoint(214,0));
    QCOMPARE(poly[70], QPoint(206,0));
    QCOMPARE(poly[71], QPoint(198,0));
    QCOMPARE(poly[72], QPoint(190,0));
    QCOMPARE(poly[73], QPoint(182,0));
    QCOMPARE(poly[74], QPoint(174,0));
    QCOMPARE(poly[75], QPoint(166,0));
    QCOMPARE(poly[76], QPoint(158,0));
    QCOMPARE(poly[77], QPoint(150,0));
    QCOMPARE(poly[78], QPoint(142,0));
    QCOMPARE(poly[79], QPoint(134,0));
    QCOMPARE(poly[80], QPoint(127,0));
    QCOMPARE(poly[81], QPoint(119,0));
    QCOMPARE(poly[82], QPoint(111,0));
    QCOMPARE(poly[83], QPoint(103,0));
    QCOMPARE(poly[84], QPoint(95,0));
    QCOMPARE(poly[85], QPoint(87,0));
    QCOMPARE(poly[86], QPoint(79,0));
    QCOMPARE(poly[87], QPoint(71,0));
    QCOMPARE(poly[88], QPoint(63,0));
    QCOMPARE(poly[89], QPoint(55,0));
    QCOMPARE(poly[90], QPoint(47,0));
    QCOMPARE(poly[91], QPoint(39,0));
    QCOMPARE(poly[92], QPoint(31,0));
    QCOMPARE(poly[93], QPoint(23,0));
    QCOMPARE(poly[94], QPoint(15,0));
    QCOMPARE(poly[95], QPoint(7,0));
    QCOMPARE(poly[96], QPoint(0,0));
    QCOMPARE(poly[97], QPoint(0,7));
    QCOMPARE(poly[98], QPoint(0,15));
    QCOMPARE(poly[99], QPoint(0,23));
    QCOMPARE(poly[100], QPoint(0,31));
    QCOMPARE(poly[101], QPoint(0,39));
    QCOMPARE(poly[102], QPoint(0,47));
    QCOMPARE(poly[103], QPoint(0,55));
    QCOMPARE(poly[104], QPoint(0,63));
    QCOMPARE(poly[105], QPoint(0,71));
    QCOMPARE(poly[106], QPoint(0,79));
    QCOMPARE(poly[107], QPoint(0,87));
    QCOMPARE(poly[108], QPoint(0,95));
    QCOMPARE(poly[109], QPoint(0,103));
    QCOMPARE(poly[110], QPoint(0,111));
    QCOMPARE(poly[111], QPoint(0,119));
    QCOMPARE(poly[112], QPoint(0,126));
    QCOMPARE(poly[113], QPoint(0,134));
    QCOMPARE(poly[114], QPoint(0,142));
    QCOMPARE(poly[115], QPoint(0,150));
    QCOMPARE(poly[116], QPoint(0,158));
    QCOMPARE(poly[117], QPoint(0,166));
    QCOMPARE(poly[118], QPoint(0,174));
    QCOMPARE(poly[119], QPoint(0,182));
    QCOMPARE(poly[120], QPoint(0,190));
    QCOMPARE(poly[121], QPoint(0,198));
    QCOMPARE(poly[122], QPoint(0,206));
    QCOMPARE(poly[123], QPoint(0,214));
    QCOMPARE(poly[124], QPoint(0,222));
    QCOMPARE(poly[125], QPoint(0,230));
    QCOMPARE(poly[126], QPoint(0,238));
    QCOMPARE(poly[127], QPoint(0,246));
}

void EFX_Test::previewSquareChoppy()
{
    EFX e(m_doc);
    e.setAlgorithm(EFX::SquareChoppy);

    QVector <QPoint> poly;
    e.preview(poly);
    QCOMPARE(poly.size(), 128);

    QCOMPARE(poly[0], QPoint(254,127));
    QCOMPARE(poly[1], QPoint(254,127));
    QCOMPARE(poly[2], QPoint(254,127));
    QCOMPARE(poly[3], QPoint(254,127));
    QCOMPARE(poly[4], QPoint(254,127));
    QCOMPARE(poly[5], QPoint(254,127));
    QCOMPARE(poly[6], QPoint(254,127));
    QCOMPARE(poly[7], QPoint(254,127));
    QCOMPARE(poly[8], QPoint(254,127));
    QCOMPARE(poly[9], QPoint(254,127));
    QCOMPARE(poly[10], QPoint(254,127));
    QCOMPARE(poly[11], QPoint(254,254));
    QCOMPARE(poly[12], QPoint(254,254));
    QCOMPARE(poly[13], QPoint(254,254));
    QCOMPARE(poly[14], QPoint(254,254));
    QCOMPARE(poly[15], QPoint(254,254));
    QCOMPARE(poly[16], QPoint(254,254));
    QCOMPARE(poly[17], QPoint(254,254));
    QCOMPARE(poly[18], QPoint(254,254));
    QCOMPARE(poly[19], QPoint(254,254));
    QCOMPARE(poly[20], QPoint(254,254));
    QCOMPARE(poly[21], QPoint(254,254));
    QCOMPARE(poly[22], QPoint(127,254));
    QCOMPARE(poly[23], QPoint(127,254));
    QCOMPARE(poly[24], QPoint(127,254));
    QCOMPARE(poly[25], QPoint(127,254));
    QCOMPARE(poly[26], QPoint(127,254));
    QCOMPARE(poly[27], QPoint(127,254));
    QCOMPARE(poly[28], QPoint(127,254));
    QCOMPARE(poly[29], QPoint(127,254));
    QCOMPARE(poly[30], QPoint(127,254));
    QCOMPARE(poly[31], QPoint(127,254));
    QCOMPARE(poly[32], QPoint(127,254));
    QCOMPARE(poly[33], QPoint(127,254));
    QCOMPARE(poly[34], QPoint(127,254));
    QCOMPARE(poly[35], QPoint(127,254));
    QCOMPARE(poly[36], QPoint(127,254));
    QCOMPARE(poly[37], QPoint(127,254));
    QCOMPARE(poly[38], QPoint(127,254));
    QCOMPARE(poly[39], QPoint(127,254));
    QCOMPARE(poly[40], QPoint(127,254));
    QCOMPARE(poly[41], QPoint(127,254));
    QCOMPARE(poly[42], QPoint(127,254));
    QCOMPARE(poly[43], QPoint(0,254));
    QCOMPARE(poly[44], QPoint(0,254));
    QCOMPARE(poly[45], QPoint(0,254));
    QCOMPARE(poly[46], QPoint(0,254));
    QCOMPARE(poly[47], QPoint(0,254));
    QCOMPARE(poly[48], QPoint(0,254));
    QCOMPARE(poly[49], QPoint(0,254));
    QCOMPARE(poly[50], QPoint(0,254));
    QCOMPARE(poly[51], QPoint(0,254));
    QCOMPARE(poly[52], QPoint(0,254));
    QCOMPARE(poly[53], QPoint(0,254));
    QCOMPARE(poly[54], QPoint(0,127));
    QCOMPARE(poly[55], QPoint(0,127));
    QCOMPARE(poly[56], QPoint(0,127));
    QCOMPARE(poly[57], QPoint(0,127));
    QCOMPARE(poly[58], QPoint(0,127));
    QCOMPARE(poly[59], QPoint(0,127));
    QCOMPARE(poly[60], QPoint(0,127));
    QCOMPARE(poly[61], QPoint(0,127));
    QCOMPARE(poly[62], QPoint(0,127));
    QCOMPARE(poly[63], QPoint(0,127));
    QCOMPARE(poly[64], QPoint(0,127));
    QCOMPARE(poly[65], QPoint(0,127));
    QCOMPARE(poly[66], QPoint(0,127));
    QCOMPARE(poly[67], QPoint(0,127));
    QCOMPARE(poly[68], QPoint(0,127));
    QCOMPARE(poly[69], QPoint(0,127));
    QCOMPARE(poly[70], QPoint(0,127));
    QCOMPARE(poly[71], QPoint(0,127));
    QCOMPARE(poly[72], QPoint(0,127));
    QCOMPARE(poly[73], QPoint(0,127));
    QCOMPARE(poly[74], QPoint(0,127));
    QCOMPARE(poly[75], QPoint(0,0));
    QCOMPARE(poly[76], QPoint(0,0));
    QCOMPARE(poly[77], QPoint(0,0));
    QCOMPARE(poly[78], QPoint(0,0));
    QCOMPARE(poly[79], QPoint(0,0));
    QCOMPARE(poly[80], QPoint(0,0));
    QCOMPARE(poly[81], QPoint(0,0));
    QCOMPARE(poly[82], QPoint(0,0));
    QCOMPARE(poly[83], QPoint(0,0));
    QCOMPARE(poly[84], QPoint(0,0));
    QCOMPARE(poly[85], QPoint(0,0));
    QCOMPARE(poly[86], QPoint(127,0));
    QCOMPARE(poly[87], QPoint(127,0));
    QCOMPARE(poly[88], QPoint(127,0));
    QCOMPARE(poly[89], QPoint(127,0));
    QCOMPARE(poly[90], QPoint(127,0));
    QCOMPARE(poly[91], QPoint(127,0));
    QCOMPARE(poly[92], QPoint(127,0));
    QCOMPARE(poly[93], QPoint(127,0));
    QCOMPARE(poly[94], QPoint(127,0));
    QCOMPARE(poly[95], QPoint(127,0));
    QCOMPARE(poly[96], QPoint(127,0));
    QCOMPARE(poly[97], QPoint(127,0));
    QCOMPARE(poly[98], QPoint(127,0));
    QCOMPARE(poly[99], QPoint(127,0));
    QCOMPARE(poly[100], QPoint(127,0));
    QCOMPARE(poly[101], QPoint(127,0));
    QCOMPARE(poly[102], QPoint(127,0));
    QCOMPARE(poly[103], QPoint(127,0));
    QCOMPARE(poly[104], QPoint(127,0));
    QCOMPARE(poly[105], QPoint(127,0));
    QCOMPARE(poly[106], QPoint(127,0));
    QCOMPARE(poly[107], QPoint(254,0));
    QCOMPARE(poly[108], QPoint(254,0));
    QCOMPARE(poly[109], QPoint(254,0));
    QCOMPARE(poly[110], QPoint(254,0));
    QCOMPARE(poly[111], QPoint(254,0));
    QCOMPARE(poly[112], QPoint(254,0));
    QCOMPARE(poly[113], QPoint(254,0));
    QCOMPARE(poly[114], QPoint(254,0));
    QCOMPARE(poly[115], QPoint(254,0));
    QCOMPARE(poly[116], QPoint(254,0));
    QCOMPARE(poly[117], QPoint(254,0));
    QCOMPARE(poly[118], QPoint(254,127));
    QCOMPARE(poly[119], QPoint(254,127));
    QCOMPARE(poly[120], QPoint(254,127));
    QCOMPARE(poly[121], QPoint(254,127));
    QCOMPARE(poly[122], QPoint(254,127));
    QCOMPARE(poly[123], QPoint(254,127));
    QCOMPARE(poly[124], QPoint(254,127));
    QCOMPARE(poly[125], QPoint(254,127));
    QCOMPARE(poly[126], QPoint(254,127));
    QCOMPARE(poly[127], QPoint(254,127));
}

void EFX_Test::previewLeaf()
{
    EFX e(m_doc);
    e.setAlgorithm(EFX::Leaf);

    QVector <QPoint> poly;
    e.preview(poly);
    QCOMPARE(poly.size(), 128);

    QCOMPARE(poly[0], QPoint(127,254));
    QCOMPARE(poly[1], QPoint(126,253));
    QCOMPARE(poly[2], QPoint(126,253));
    QCOMPARE(poly[3], QPoint(126,252));
    QCOMPARE(poly[4], QPoint(126,251));
    QCOMPARE(poly[5], QPoint(126,250));
    QCOMPARE(poly[6], QPoint(126,248));
    QCOMPARE(poly[7], QPoint(126,246));
    QCOMPARE(poly[8], QPoint(125,244));
    QCOMPARE(poly[9], QPoint(125,241));
    QCOMPARE(poly[10], QPoint(124,239));
    QCOMPARE(poly[11], QPoint(122,235));
    QCOMPARE(poly[12], QPoint(120,232));
    QCOMPARE(poly[13], QPoint(117,229));
    QCOMPARE(poly[14], QPoint(113,225));
    QCOMPARE(poly[15], QPoint(109,221));
    QCOMPARE(poly[16], QPoint(104,216));
    QCOMPARE(poly[17], QPoint(98,212));
    QCOMPARE(poly[18], QPoint(91,207));
    QCOMPARE(poly[19], QPoint(84,202));
    QCOMPARE(poly[20], QPoint(76,197));
    QCOMPARE(poly[21], QPoint(68,192));
    QCOMPARE(poly[22], QPoint(59,186));
    QCOMPARE(poly[23], QPoint(50,181));
    QCOMPARE(poly[24], QPoint(41,175));
    QCOMPARE(poly[25], QPoint(33,169));
    QCOMPARE(poly[26], QPoint(25,163));
    QCOMPARE(poly[27], QPoint(17,157));
    QCOMPARE(poly[28], QPoint(11,151));
    QCOMPARE(poly[29], QPoint(6,145));
    QCOMPARE(poly[30], QPoint(3,139));
    QCOMPARE(poly[31], QPoint(0,133));
    QCOMPARE(poly[32], QPoint(0,126));
    QCOMPARE(poly[33], QPoint(0,120));
    QCOMPARE(poly[34], QPoint(3,114));
    QCOMPARE(poly[35], QPoint(6,108));
    QCOMPARE(poly[36], QPoint(11,102));
    QCOMPARE(poly[37], QPoint(17,96));
    QCOMPARE(poly[38], QPoint(25,90));
    QCOMPARE(poly[39], QPoint(33,84));
    QCOMPARE(poly[40], QPoint(41,78));
    QCOMPARE(poly[41], QPoint(50,72));
    QCOMPARE(poly[42], QPoint(59,67));
    QCOMPARE(poly[43], QPoint(68,61));
    QCOMPARE(poly[44], QPoint(76,56));
    QCOMPARE(poly[45], QPoint(84,51));
    QCOMPARE(poly[46], QPoint(91,46));
    QCOMPARE(poly[47], QPoint(98,41));
    QCOMPARE(poly[48], QPoint(104,37));
    QCOMPARE(poly[49], QPoint(109,32));
    QCOMPARE(poly[50], QPoint(113,28));
    QCOMPARE(poly[51], QPoint(117,24));
    QCOMPARE(poly[52], QPoint(120,21));
    QCOMPARE(poly[53], QPoint(122,18));
    QCOMPARE(poly[54], QPoint(124,14));
    QCOMPARE(poly[55], QPoint(125,12));
    QCOMPARE(poly[56], QPoint(125,9));
    QCOMPARE(poly[57], QPoint(126,7));
    QCOMPARE(poly[58], QPoint(126,5));
    QCOMPARE(poly[59], QPoint(126,3));
    QCOMPARE(poly[60], QPoint(126,2));
    QCOMPARE(poly[61], QPoint(126,1));
    QCOMPARE(poly[62], QPoint(126,0));
    QCOMPARE(poly[63], QPoint(126,0));
    QCOMPARE(poly[64], QPoint(127,0));
    QCOMPARE(poly[65], QPoint(127,0));
    QCOMPARE(poly[66], QPoint(127,0));
    QCOMPARE(poly[67], QPoint(127,1));
    QCOMPARE(poly[68], QPoint(127,2));
    QCOMPARE(poly[69], QPoint(127,3));
    QCOMPARE(poly[70], QPoint(127,5));
    QCOMPARE(poly[71], QPoint(127,7));
    QCOMPARE(poly[72], QPoint(128,9));
    QCOMPARE(poly[73], QPoint(128,12));
    QCOMPARE(poly[74], QPoint(129,14));
    QCOMPARE(poly[75], QPoint(131,18));
    QCOMPARE(poly[76], QPoint(133,21));
    QCOMPARE(poly[77], QPoint(136,24));
    QCOMPARE(poly[78], QPoint(140,28));
    QCOMPARE(poly[79], QPoint(144,32));
    QCOMPARE(poly[80], QPoint(149,37));
    QCOMPARE(poly[81], QPoint(155,41));
    QCOMPARE(poly[82], QPoint(162,46));
    QCOMPARE(poly[83], QPoint(169,51));
    QCOMPARE(poly[84], QPoint(177,56));
    QCOMPARE(poly[85], QPoint(185,61));
    QCOMPARE(poly[86], QPoint(194,67));
    QCOMPARE(poly[87], QPoint(203,72));
    QCOMPARE(poly[88], QPoint(212,78));
    QCOMPARE(poly[89], QPoint(220,84));
    QCOMPARE(poly[90], QPoint(228,90));
    QCOMPARE(poly[91], QPoint(236,96));
    QCOMPARE(poly[92], QPoint(242,102));
    QCOMPARE(poly[93], QPoint(247,108));
    QCOMPARE(poly[94], QPoint(250,114));
    QCOMPARE(poly[95], QPoint(253,120));
    QCOMPARE(poly[96], QPoint(254,126));
    QCOMPARE(poly[97], QPoint(253,133));
    QCOMPARE(poly[98], QPoint(250,139));
    QCOMPARE(poly[99], QPoint(247,145));
    QCOMPARE(poly[100], QPoint(242,151));
    QCOMPARE(poly[101], QPoint(236,157));
    QCOMPARE(poly[102], QPoint(228,163));
    QCOMPARE(poly[103], QPoint(220,169));
    QCOMPARE(poly[104], QPoint(212,175));
    QCOMPARE(poly[105], QPoint(203,181));
    QCOMPARE(poly[106], QPoint(194,186));
    QCOMPARE(poly[107], QPoint(185,192));
    QCOMPARE(poly[108], QPoint(177,197));
    QCOMPARE(poly[109], QPoint(169,202));
    QCOMPARE(poly[110], QPoint(162,207));
    QCOMPARE(poly[111], QPoint(155,212));
    QCOMPARE(poly[112], QPoint(149,216));
    QCOMPARE(poly[113], QPoint(144,221));
    QCOMPARE(poly[114], QPoint(140,225));
    QCOMPARE(poly[115], QPoint(136,229));
    QCOMPARE(poly[116], QPoint(133,232));
    QCOMPARE(poly[117], QPoint(131,235));
    QCOMPARE(poly[118], QPoint(129,239));
    QCOMPARE(poly[119], QPoint(128,241));
    QCOMPARE(poly[120], QPoint(128,244));
    QCOMPARE(poly[121], QPoint(127,246));
    QCOMPARE(poly[122], QPoint(127,248));
    QCOMPARE(poly[123], QPoint(127,250));
    QCOMPARE(poly[124], QPoint(127,251));
    QCOMPARE(poly[125], QPoint(127,252));
    QCOMPARE(poly[126], QPoint(127,253));
    QCOMPARE(poly[127], QPoint(127,253));
}

void EFX_Test::previewLissajous()
{
    EFX e(m_doc);
    e.setAlgorithm(EFX::Lissajous);

    QVector <QPoint> poly;
    e.preview(poly);
    QCOMPARE(poly.size(), 128);

    QCOMPARE(poly[0], QPoint(127,254));
    QCOMPARE(poly[1], QPoint(139,252));
    QCOMPARE(poly[2], QPoint(151,248));
    QCOMPARE(poly[3], QPoint(163,241));
    QCOMPARE(poly[4], QPoint(175,232));
    QCOMPARE(poly[5], QPoint(186,221));
    QCOMPARE(poly[6], QPoint(197,207));
    QCOMPARE(poly[7], QPoint(207,192));
    QCOMPARE(poly[8], QPoint(216,175));
    QCOMPARE(poly[9], QPoint(225,157));
    QCOMPARE(poly[10], QPoint(232,139));
    QCOMPARE(poly[11], QPoint(239,120));
    QCOMPARE(poly[12], QPoint(244,102));
    QCOMPARE(poly[13], QPoint(248,84));
    QCOMPARE(poly[14], QPoint(251,67));
    QCOMPARE(poly[15], QPoint(253,51));
    QCOMPARE(poly[16], QPoint(254,37));
    QCOMPARE(poly[17], QPoint(253,24));
    QCOMPARE(poly[18], QPoint(251,14));
    QCOMPARE(poly[19], QPoint(248,7));
    QCOMPARE(poly[20], QPoint(244,2));
    QCOMPARE(poly[21], QPoint(239,0));
    QCOMPARE(poly[22], QPoint(232,0));
    QCOMPARE(poly[23], QPoint(225,3));
    QCOMPARE(poly[24], QPoint(216,9));
    QCOMPARE(poly[25], QPoint(207,18));
    QCOMPARE(poly[26], QPoint(197,28));
    QCOMPARE(poly[27], QPoint(186,41));
    QCOMPARE(poly[28], QPoint(175,56));
    QCOMPARE(poly[29], QPoint(163,72));
    QCOMPARE(poly[30], QPoint(151,90));
    QCOMPARE(poly[31], QPoint(139,108));
    QCOMPARE(poly[32], QPoint(126,127));
    QCOMPARE(poly[33], QPoint(114,145));
    QCOMPARE(poly[34], QPoint(102,163));
    QCOMPARE(poly[35], QPoint(90,181));
    QCOMPARE(poly[36], QPoint(78,197));
    QCOMPARE(poly[37], QPoint(67,212));
    QCOMPARE(poly[38], QPoint(56,225));
    QCOMPARE(poly[39], QPoint(46,235));
    QCOMPARE(poly[40], QPoint(37,244));
    QCOMPARE(poly[41], QPoint(28,250));
    QCOMPARE(poly[42], QPoint(21,253));
    QCOMPARE(poly[43], QPoint(14,253));
    QCOMPARE(poly[44], QPoint(9,251));
    QCOMPARE(poly[45], QPoint(5,246));
    QCOMPARE(poly[46], QPoint(2,239));
    QCOMPARE(poly[47], QPoint(0,229));
    QCOMPARE(poly[48], QPoint(0,216));
    QCOMPARE(poly[49], QPoint(0,202));
    QCOMPARE(poly[50], QPoint(2,186));
    QCOMPARE(poly[51], QPoint(5,169));
    QCOMPARE(poly[52], QPoint(9,151));
    QCOMPARE(poly[53], QPoint(14,133));
    QCOMPARE(poly[54], QPoint(21,114));
    QCOMPARE(poly[55], QPoint(28,96));
    QCOMPARE(poly[56], QPoint(37,78));
    QCOMPARE(poly[57], QPoint(46,61));
    QCOMPARE(poly[58], QPoint(56,46));
    QCOMPARE(poly[59], QPoint(67,32));
    QCOMPARE(poly[60], QPoint(78,21));
    QCOMPARE(poly[61], QPoint(90,12));
    QCOMPARE(poly[62], QPoint(102,5));
    QCOMPARE(poly[63], QPoint(114,1));
    QCOMPARE(poly[64], QPoint(126,0));
    QCOMPARE(poly[65], QPoint(139,1));
    QCOMPARE(poly[66], QPoint(151,5));
    QCOMPARE(poly[67], QPoint(163,12));
    QCOMPARE(poly[68], QPoint(175,21));
    QCOMPARE(poly[69], QPoint(186,32));
    QCOMPARE(poly[70], QPoint(197,46));
    QCOMPARE(poly[71], QPoint(207,61));
    QCOMPARE(poly[72], QPoint(216,78));
    QCOMPARE(poly[73], QPoint(225,96));
    QCOMPARE(poly[74], QPoint(232,114));
    QCOMPARE(poly[75], QPoint(239,133));
    QCOMPARE(poly[76], QPoint(244,151));
    QCOMPARE(poly[77], QPoint(248,169));
    QCOMPARE(poly[78], QPoint(251,186));
    QCOMPARE(poly[79], QPoint(253,202));
    QCOMPARE(poly[80], QPoint(254,216));
    QCOMPARE(poly[81], QPoint(253,229));
    QCOMPARE(poly[82], QPoint(251,239));
    QCOMPARE(poly[83], QPoint(248,246));
    QCOMPARE(poly[84], QPoint(244,251));
    QCOMPARE(poly[85], QPoint(239,253));
    QCOMPARE(poly[86], QPoint(232,253));
    QCOMPARE(poly[87], QPoint(225,250));
    QCOMPARE(poly[88], QPoint(216,244));
    QCOMPARE(poly[89], QPoint(207,235));
    QCOMPARE(poly[90], QPoint(197,225));
    QCOMPARE(poly[91], QPoint(186,212));
    QCOMPARE(poly[92], QPoint(175,197));
    QCOMPARE(poly[93], QPoint(163,181));
    QCOMPARE(poly[94], QPoint(151,163));
    QCOMPARE(poly[95], QPoint(139,145));
    QCOMPARE(poly[96], QPoint(127,127));
    QCOMPARE(poly[97], QPoint(114,108));
    QCOMPARE(poly[98], QPoint(102,90));
    QCOMPARE(poly[99], QPoint(90,72));
    QCOMPARE(poly[100], QPoint(78,56));
    QCOMPARE(poly[101], QPoint(67,41));
    QCOMPARE(poly[102], QPoint(56,28));
    QCOMPARE(poly[103], QPoint(46,18));
    QCOMPARE(poly[104], QPoint(37,9));
    QCOMPARE(poly[105], QPoint(28,3));
    QCOMPARE(poly[106], QPoint(21,0));
    QCOMPARE(poly[107], QPoint(14,0));
    QCOMPARE(poly[108], QPoint(9,2));
    QCOMPARE(poly[109], QPoint(5,7));
    QCOMPARE(poly[110], QPoint(2,14));
    QCOMPARE(poly[111], QPoint(0,24));
    QCOMPARE(poly[112], QPoint(0,37));
    QCOMPARE(poly[113], QPoint(0,51));
    QCOMPARE(poly[114], QPoint(2,67));
    QCOMPARE(poly[115], QPoint(5,84));
    QCOMPARE(poly[116], QPoint(9,102));
    QCOMPARE(poly[117], QPoint(14,120));
    QCOMPARE(poly[118], QPoint(21,139));
    QCOMPARE(poly[119], QPoint(28,157));
    QCOMPARE(poly[120], QPoint(37,175));
    QCOMPARE(poly[121], QPoint(46,192));
    QCOMPARE(poly[122], QPoint(56,207));
    QCOMPARE(poly[123], QPoint(67,221));
    QCOMPARE(poly[124], QPoint(78,232));
    QCOMPARE(poly[125], QPoint(90,241));
    QCOMPARE(poly[126], QPoint(102,248));
    QCOMPARE(poly[127], QPoint(114,252));
}

// Due to rounding errors, reverse direction might come out 
// +/- one point. For now it's acceptable, but should be fixed
// some day.
static bool CloseEnough(QPoint const & a, QPoint const & b)
{
    QPoint diff = a - b;
    return -1 <= diff.rx() && diff.rx() <= 1 &&
           -1 <= diff.ry() && diff.ry() <= 1;
}

void EFX_Test::previewCircleBackwards()
{
    EFX e(m_doc);

    QVector <QPoint> polyF;
    e.preview(polyF);
    QCOMPARE(polyF.size(), 128);

    e.setDirection(Function::Backward);

    QVector <QPoint> polyB;
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

    QVector <QPoint> polyF;
    e.preview(polyF);
    QCOMPARE(polyF.size(), 128);

    e.setDirection(Function::Backward);

    QVector <QPoint> polyB;
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

    QVector <QPoint> polyF;
    e.preview(polyF);
    QCOMPARE(polyF.size(), 128);

    e.setDirection(Function::Backward);

    QVector <QPoint> polyB;
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

    QVector <QPoint> polyF;
    e.preview(polyF);
    QCOMPARE(polyF.size(), 128);

    e.setDirection(Function::Backward);

    QVector <QPoint> polyB;
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

    QVector <QPoint> polyF;
    e.preview(polyF);
    QCOMPARE(polyF.size(), 128);

    e.setDirection(Function::Backward);

    QVector <QPoint> polyB;
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

    QVector <QPoint> polyF;
    e.preview(polyF);
    QCOMPARE(polyF.size(), 128);

    e.setDirection(Function::Backward);

    QVector <QPoint> polyB;
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

    QVector <QPoint> polyF;
    e.preview(polyF);
    QCOMPARE(polyF.size(), 128);

    e.setDirection(Function::Backward);

    QVector <QPoint> polyB;
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

    QVector <QPoint> polyF;
    e.preview(polyF);
    QCOMPARE(polyF.size(), 128);

    e.setDirection(Function::Backward);

    QVector <QPoint> polyB;
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

    QVector <QPoint> polyF;
    e.preview(polyF);
    QCOMPARE(polyF.size(), 128);

    e.setDirection(Function::Backward);

    QVector <QPoint> polyB;
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

    QVector <QPoint> poly;
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
    QVERIFY(QPolygon(poly).boundingRect().width() == 50 + 50 + 1);
    QVERIFY(QPolygon(poly).boundingRect().height() == UCHAR_MAX);

    /* Check that height affects the pattern */
    e.setHeight(87);
    poly.clear();
    e.preview(poly);
    QVERIFY(poly.size() == 128);

    /* Height of 87 means actually 87px down of center (127-87) and
       87px up of center (127+87). And +1 because the bound coordinates
       are OUTSIDE the actual points. */
    QVERIFY(QPolygon(poly).boundingRect().height() == 87 + 87 + 1);
    QVERIFY(QPolygon(poly).boundingRect().width() == 100 + 1);

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
    qreal x, y;

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
    QCOMPARE(floor(x + 0.5), qreal(190));
    QCOMPARE(floor(y + 0.5), qreal(135));

    x = -0.125333;
    y = 0.992115;
    efx.setWidth(10);
    efx.setHeight(127);
    efx.setXOffset(127);
    efx.setYOffset(127);
    efx.setRotation(90);
    efx.rotateAndScale(&x, &y);
    QCOMPARE(floor(x + 0.5), qreal(253));
    QCOMPARE(floor(y + 0.5), qreal(128));

    x = -0.125333;
    y = 0.992115;
    efx.setWidth(64);
    efx.setHeight(127);
    efx.setXOffset(127);
    efx.setYOffset(127);
    efx.setRotation(90);
    efx.rotateAndScale(&x, &y);
    QCOMPARE(floor(x + 0.5), qreal(253));
    QCOMPARE(floor(y + 0.5), qreal(135));

    x = -0.125333;
    y = 0.992115;
    efx.setWidth(64);
    efx.setHeight(0);
    efx.setXOffset(127);
    efx.setYOffset(127);
    efx.setRotation(90);
    efx.rotateAndScale(&x, &y);
    QCOMPARE(floor(x + 0.5), qreal(127));
    QCOMPARE(floor(y + 0.5), qreal(135));

    x = -0.125333;
    y = 0.992115;
    efx.setWidth(127);
    efx.setHeight(0);
    efx.setXOffset(127);
    efx.setYOffset(127);
    efx.setRotation(90);
    efx.rotateAndScale(&x, &y);
    QCOMPARE(floor(x + 0.5), qreal(127));
    QCOMPARE(floor(y + 0.5), qreal(143));
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
    QDomDocument doc;

    QDomElement ax = doc.createElement("Axis");
    ax.setAttribute("Name", "X");

    QDomElement off = doc.createElement("Offset");
    QDomText offText = doc.createTextNode("1");
    off.appendChild(offText);
    ax.appendChild(off);

    QDomElement freq = doc.createElement("Frequency");
    QDomText freqText = doc.createTextNode("2");
    freq.appendChild(freqText);
    ax.appendChild(freq);

    QDomElement pha = doc.createElement("Phase");
    QDomText phaText = doc.createTextNode("3");
    pha.appendChild(phaText);
    ax.appendChild(pha);

    // Unknown tag
    QDomElement foo = doc.createElement("Foo");
    QDomText fooText = doc.createTextNode("Bar");
    foo.appendChild(fooText);
    ax.appendChild(foo);

    EFX e(m_doc);
    QVERIFY(e.loadXMLAxis(ax) == true);
}

void EFX_Test::loadYAxis()
{
    QDomDocument doc;

    QDomElement ax = doc.createElement("Axis");
    ax.setAttribute("Name", "Y");

    QDomElement off = doc.createElement("Offset");
    QDomText offText = doc.createTextNode("1");
    off.appendChild(offText);
    ax.appendChild(off);

    QDomElement freq = doc.createElement("Frequency");
    QDomText freqText = doc.createTextNode("2");
    freq.appendChild(freqText);
    ax.appendChild(freq);

    QDomElement pha = doc.createElement("Phase");
    QDomText phaText = doc.createTextNode("3");
    pha.appendChild(phaText);
    ax.appendChild(pha);

    EFX e(m_doc);
    QVERIFY(e.loadXMLAxis(ax) == true);
}

void EFX_Test::loadYAxisWrongRoot()
{
    QDomDocument doc;

    QDomElement ax = doc.createElement("sixA");
    ax.setAttribute("Name", "Y");

    QDomElement off = doc.createElement("Offset");
    QDomText offText = doc.createTextNode("1");
    off.appendChild(offText);
    ax.appendChild(off);

    QDomElement freq = doc.createElement("Frequency");
    QDomText freqText = doc.createTextNode("2");
    freq.appendChild(freqText);
    ax.appendChild(freq);

    QDomElement pha = doc.createElement("Phase");
    QDomText phaText = doc.createTextNode("3");
    pha.appendChild(phaText);
    ax.appendChild(pha);

    EFX e(m_doc);
    QVERIFY(e.loadXMLAxis(ax) == false);
}

void EFX_Test::loadAxisNoXY()
{
    QDomDocument doc;

    QDomElement ax = doc.createElement("Axis");
    ax.setAttribute("Name", "Not X nor Y");

    QDomElement off = doc.createElement("Offset");
    QDomText offText = doc.createTextNode("1");
    off.appendChild(offText);
    ax.appendChild(off);

    QDomElement freq = doc.createElement("Frequency");
    QDomText freqText = doc.createTextNode("5");
    freq.appendChild(freqText);
    ax.appendChild(freq);

    QDomElement pha = doc.createElement("Phase");
    QDomText phaText = doc.createTextNode("333");
    pha.appendChild(phaText);
    ax.appendChild(pha);

    EFX e(m_doc);
    QVERIFY(e.loadXMLAxis(ax) == false);
    QVERIFY(e.xOffset() != 1);
    QVERIFY(e.xFrequency() != 5);
    QVERIFY(e.xPhase() != 333);
}

void EFX_Test::loadSuccessLegacy()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Function");
    root.setAttribute("Type", "EFX");
    root.setAttribute("Name", "Test EFX");

    QDomElement prop = doc.createElement("PropagationMode");
    QDomText propText = doc.createTextNode("Serial");
    prop.appendChild(propText);
    root.appendChild(prop);

    QDomElement bus = doc.createElement("Bus");
    bus.setAttribute("Role", "Fade");
    QDomText busText = doc.createTextNode("12");
    bus.appendChild(busText);
    root.appendChild(bus);

    QDomElement hbus = doc.createElement("Bus");
    hbus.setAttribute("Role", "Hold");
    QDomText hbusText = doc.createTextNode("13");
    hbus.appendChild(hbusText);
    root.appendChild(hbus);

    QDomElement dir = doc.createElement("Direction");
    QDomText dirText = doc.createTextNode("Forward");
    dir.appendChild(dirText);
    root.appendChild(dir);

    QDomElement run = doc.createElement("RunOrder");
    QDomText runText = doc.createTextNode("Loop");
    run.appendChild(runText);
    root.appendChild(run);

    QDomElement algo = doc.createElement("Algorithm");
    QDomText algoText = doc.createTextNode("Diamond");
    algo.appendChild(algoText);
    root.appendChild(algo);

    QDomElement w = doc.createElement("Width");
    QDomText wText = doc.createTextNode("100");
    w.appendChild(wText);
    root.appendChild(w);

    QDomElement h = doc.createElement("Height");
    QDomText hText = doc.createTextNode("90");
    h.appendChild(hText);
    root.appendChild(h);

    QDomElement rot = doc.createElement("Rotation");
    QDomText rotText = doc.createTextNode("310");
    rot.appendChild(rotText);
    root.appendChild(rot);

    /* X Axis */
    QDomElement xax = doc.createElement("Axis");
    xax.setAttribute("Name", "X");
    root.appendChild(xax);

    QDomElement xoff = doc.createElement("Offset");
    QDomText xoffText = doc.createTextNode("10");
    xoff.appendChild(xoffText);
    xax.appendChild(xoff);

    QDomElement xfreq = doc.createElement("Frequency");
    QDomText xfreqText = doc.createTextNode("2");
    xfreq.appendChild(xfreqText);
    xax.appendChild(xfreq);

    QDomElement xpha = doc.createElement("Phase");
    QDomText xphaText = doc.createTextNode("270");
    xpha.appendChild(xphaText);
    xax.appendChild(xpha);

    /* Y Axis */
    QDomElement yax = doc.createElement("Axis");
    yax.setAttribute("Name", "Y");
    root.appendChild(yax);

    QDomElement yoff = doc.createElement("Offset");
    QDomText yoffText = doc.createTextNode("20");
    yoff.appendChild(yoffText);
    yax.appendChild(yoff);

    QDomElement yfreq = doc.createElement("Frequency");
    QDomText yfreqText = doc.createTextNode("3");
    yfreq.appendChild(yfreqText);
    yax.appendChild(yfreq);

    QDomElement ypha = doc.createElement("Phase");
    QDomText yphaText = doc.createTextNode("80");
    ypha.appendChild(yphaText);
    yax.appendChild(ypha);

    /* Fixture 1 */
    QDomElement ef1 = doc.createElement("Fixture");
    root.appendChild(ef1);

    QDomElement ef1ID = doc.createElement("ID");
    QDomText ef1IDText = doc.createTextNode("33");
    ef1ID.appendChild(ef1IDText);
    ef1.appendChild(ef1ID);

    QDomElement ef1dir = doc.createElement("Direction");
    QDomText ef1dirText = doc.createTextNode("Forward");
    ef1dir.appendChild(ef1dirText);
    ef1.appendChild(ef1dir);

    /* Fixture 2 */
    QDomElement ef2 = doc.createElement("Fixture");
    root.appendChild(ef2);

    QDomElement ef2ID = doc.createElement("ID");
    QDomText ef2IDText = doc.createTextNode("11");
    ef2ID.appendChild(ef2IDText);
    ef2.appendChild(ef2ID);

    QDomElement ef2dir = doc.createElement("Direction");
    QDomText ef2dirText = doc.createTextNode("Backward");
    ef2dir.appendChild(ef2dirText);
    ef2.appendChild(ef2dir);

    /* Fixture 3 */
    QDomElement ef3 = doc.createElement("Fixture");
    root.appendChild(ef3);

    QDomElement ef3ID = doc.createElement("ID");
    QDomText ef3IDText = doc.createTextNode("45");
    ef3ID.appendChild(ef3IDText);
    ef3.appendChild(ef3ID);

    QDomElement ef3dir = doc.createElement("Direction");
    QDomText ef3dirText = doc.createTextNode("Backward");
    ef3dir.appendChild(ef3dirText);
    ef3.appendChild(ef3dir);

    EFX e(m_doc);
    QVERIFY(e.loadXML(root) == true);

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
    QDomDocument doc;

    QDomElement root = doc.createElement("Function");
    root.setAttribute("Type", "EFX");
    root.setAttribute("Name", "Test EFX");

    QDomElement prop = doc.createElement("PropagationMode");
    QDomText propText = doc.createTextNode("Serial");
    prop.appendChild(propText);
    root.appendChild(prop);

    QDomElement speed = doc.createElement("Speed");
    speed.setAttribute("FadeIn", "1300");
    speed.setAttribute("FadeOut", "1400");
    speed.setAttribute("Duration", "1500");
    root.appendChild(speed);

    QDomElement dir = doc.createElement("Direction");
    QDomText dirText = doc.createTextNode("Forward");
    dir.appendChild(dirText);
    root.appendChild(dir);

    QDomElement run = doc.createElement("RunOrder");
    QDomText runText = doc.createTextNode("Loop");
    run.appendChild(runText);
    root.appendChild(run);

    QDomElement algo = doc.createElement("Algorithm");
    QDomText algoText = doc.createTextNode("Diamond");
    algo.appendChild(algoText);
    root.appendChild(algo);

    QDomElement w = doc.createElement("Width");
    QDomText wText = doc.createTextNode("100");
    w.appendChild(wText);
    root.appendChild(w);

    QDomElement h = doc.createElement("Height");
    QDomText hText = doc.createTextNode("90");
    h.appendChild(hText);
    root.appendChild(h);

    QDomElement rot = doc.createElement("Rotation");
    QDomText rotText = doc.createTextNode("310");
    rot.appendChild(rotText);
    root.appendChild(rot);

    /* X Axis */
    QDomElement xax = doc.createElement("Axis");
    xax.setAttribute("Name", "X");
    root.appendChild(xax);

    QDomElement xoff = doc.createElement("Offset");
    QDomText xoffText = doc.createTextNode("10");
    xoff.appendChild(xoffText);
    xax.appendChild(xoff);

    QDomElement xfreq = doc.createElement("Frequency");
    QDomText xfreqText = doc.createTextNode("2");
    xfreq.appendChild(xfreqText);
    xax.appendChild(xfreq);

    QDomElement xpha = doc.createElement("Phase");
    QDomText xphaText = doc.createTextNode("270");
    xpha.appendChild(xphaText);
    xax.appendChild(xpha);

    /* Y Axis */
    QDomElement yax = doc.createElement("Axis");
    yax.setAttribute("Name", "Y");
    root.appendChild(yax);

    QDomElement yoff = doc.createElement("Offset");
    QDomText yoffText = doc.createTextNode("20");
    yoff.appendChild(yoffText);
    yax.appendChild(yoff);

    QDomElement yfreq = doc.createElement("Frequency");
    QDomText yfreqText = doc.createTextNode("3");
    yfreq.appendChild(yfreqText);
    yax.appendChild(yfreq);

    QDomElement ypha = doc.createElement("Phase");
    QDomText yphaText = doc.createTextNode("80");
    ypha.appendChild(yphaText);
    yax.appendChild(ypha);

    /* Fixture 1 */
    QDomElement ef1 = doc.createElement("Fixture");
    root.appendChild(ef1);

    QDomElement ef1ID = doc.createElement("ID");
    QDomText ef1IDText = doc.createTextNode("33");
    ef1ID.appendChild(ef1IDText);
    ef1.appendChild(ef1ID);

    QDomElement ef1dir = doc.createElement("Direction");
    QDomText ef1dirText = doc.createTextNode("Forward");
    ef1dir.appendChild(ef1dirText);
    ef1.appendChild(ef1dir);

    /* Fixture 2 */
    QDomElement ef2 = doc.createElement("Fixture");
    root.appendChild(ef2);

    QDomElement ef2ID = doc.createElement("ID");
    QDomText ef2IDText = doc.createTextNode("11");
    ef2ID.appendChild(ef2IDText);
    ef2.appendChild(ef2ID);

    QDomElement ef2dir = doc.createElement("Direction");
    QDomText ef2dirText = doc.createTextNode("Backward");
    ef2dir.appendChild(ef2dirText);
    ef2.appendChild(ef2dir);

    /* Fixture 3 */
    QDomElement ef3 = doc.createElement("Fixture");
    root.appendChild(ef3);

    QDomElement ef3ID = doc.createElement("ID");
    QDomText ef3IDText = doc.createTextNode("45");
    ef3ID.appendChild(ef3IDText);
    ef3.appendChild(ef3ID);

    QDomElement ef3dir = doc.createElement("Direction");
    QDomText ef3dirText = doc.createTextNode("Backward");
    ef3dir.appendChild(ef3dirText);
    ef3.appendChild(ef3dir);

    EFX e(m_doc);
    QVERIFY(e.loadXML(root) == true);
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
    QDomDocument doc;

    QDomElement root = doc.createElement("Function");
    root.setAttribute("Type", "Chaser");

    EFX e(m_doc);
    QVERIFY(e.loadXML(root) == false);
}

void EFX_Test::loadWrongRoot()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("EFX");
    root.setAttribute("Type", "EFX");

    EFX e(m_doc);
    QVERIFY(e.loadXML(root) == false);
}

void EFX_Test::loadDuplicateFixture()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Function");
    root.setAttribute("Type", "EFX");

    /* Fixture 1 */
    QDomElement ef1 = doc.createElement("Fixture");
    root.appendChild(ef1);

    QDomElement ef1ID = doc.createElement("ID");
    QDomText ef1IDText = doc.createTextNode("33");
    ef1ID.appendChild(ef1IDText);
    ef1.appendChild(ef1ID);

    QDomElement ef1dir = doc.createElement("Direction");
    QDomText ef1dirText = doc.createTextNode("Forward");
    ef1dir.appendChild(ef1dirText);
    ef1.appendChild(ef1dir);

    /* Fixture 2 */
    QDomElement ef2 = doc.createElement("Fixture");
    root.appendChild(ef2);

    QDomElement ef2ID = doc.createElement("ID");
    QDomText ef2IDText = doc.createTextNode("33");
    ef2ID.appendChild(ef2IDText);
    ef2.appendChild(ef2ID);

    QDomElement ef2dir = doc.createElement("Direction");
    QDomText ef2dirText = doc.createTextNode("Backward");
    ef2dir.appendChild(ef2dirText);
    ef2.appendChild(ef2dir);

    EFX e(m_doc);
    QVERIFY(e.loadXML(root) == true);
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
    ef1->setFadeIntensity(128);
    e1.addFixture(ef1);
    EFXFixture* ef2 = new EFXFixture(&e1);
    ef2->setHead(GroupHead(34, 5));
    ef2->setDirection(EFX::Backward);
    ef2->setStartOffset(27);
    ef2->setFadeIntensity(64);
    e1.addFixture(ef2);
    EFXFixture* ef3 = new EFXFixture(&e1);
    ef3->setHead(GroupHead(56,7));
    e1.addFixture(ef3);

    QDomDocument doc;
    QDomElement root = doc.createElement("TestRoot");

    QVERIFY(e1.saveXML(&doc, &root) == true);
    QVERIFY(root.firstChild().toElement().tagName() == "Function");
    QVERIFY(root.firstChild().toElement().attribute("Type") == "EFX");
    QVERIFY(root.firstChild().toElement().attribute("Name") == "First");

    bool dir = false, off = false, run = false, algo = false, w = false,
         h = false, rot = false, isRelative = false, xoff = false, yoff = false,
         xfreq = false, yfreq = false, xpha = false, ypha = false,
         prop = false, intensity = false, speed = false;
    int fixtureid = 0, fixturehead = 0, fixturedirection = 0, fixtureStartOffset = 0;
    QList <QString> fixtures;

    QDomNode node = root.firstChild().firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == "Speed")
        {
            QCOMPARE(tag.attribute("FadeIn").toUInt(), uint(42));
            QCOMPARE(tag.attribute("FadeOut").toUInt(), uint(69));
            QCOMPARE(tag.attribute("Duration").toUInt(), uint(1337));
            speed = true;
        }
        else if (tag.tagName() == "Direction")
        {
            QVERIFY(tag.text() == "Backward");
            dir = true;
        }
        else if (tag.tagName() == "StartOffset")
        {
            QVERIFY(tag.text() == "91");
            off = true;
        }
        else if (tag.tagName() == "RunOrder")
        {
            QVERIFY(tag.text() == "SingleShot");
            run = true;
        }
        else if (tag.tagName() == "Bus")
        {
            QFAIL("EFX should not save a Bus tag anymore!");
        }
        else if (tag.tagName() == "Algorithm")
        {
            QVERIFY(tag.text() == "Lissajous");
            algo = true;
        }
        else if (tag.tagName() == "Width")
        {
            QVERIFY(tag.text() == "13");
            w = true;
        }
        else if (tag.tagName() == "Height")
        {
            QVERIFY(tag.text() == "42");
            h = true;
        }
        else if (tag.tagName() == "Rotation")
        {
            QVERIFY(tag.text() == "78");
            rot = true;
        }
        else if (tag.tagName() == "IsRelative")
        {
            QVERIFY(tag.text() == "0");
            isRelative = true;
        }
        else if (tag.tagName() == "PropagationMode")
        {
            QVERIFY(tag.text() == "Serial");
            prop = true;
        }
        else if (tag.tagName() == "Axis")
        {
            bool axis = true;
            if (tag.attribute("Name") == "X")
                axis = true;
            else if (tag.attribute("Name") == "Y")
                axis = false;
            else
                QFAIL("Invalid axis!");

            QDomNode subnode = tag.firstChild();
            while (subnode.isNull() == false)
            {
                QDomElement subtag = subnode.toElement();
                if (subtag.tagName() == "Offset")
                {
                    if (axis == true)
                    {
                        QVERIFY(subtag.text() == "34");
                        xoff = true;
                    }
                    else
                    {
                        QVERIFY(subtag.text() == "27");
                        yoff = true;
                    }
                }
                else if (subtag.tagName() == "Frequency")
                {
                    if (axis == true)
                    {
                        QVERIFY(subtag.text() == "5");
                        xfreq = true;
                    }
                    else
                    {
                        QVERIFY(subtag.text() == "4");
                        yfreq = true;
                    }
                }
                else if (subtag.tagName() == "Phase")
                {
                    if (axis == true)
                    {
                        QVERIFY(subtag.text() == "163");
                        xpha = true;
                    }
                    else
                    {
                        QVERIFY(subtag.text() == "94");
                        ypha = true;
                    }
                }
                else
                {
                    QFAIL("Unexpected axis tag!");
                }

                subnode = subnode.nextSibling();
            }
        }
        else if (tag.tagName() == "Fixture")
        {
            int expectHead = 0;
            bool expectBackward = false;
            int expectIntensity = 255;
            int expectStartOffset = 0;

            QDomNode subnode = tag.firstChild();
            while (subnode.isNull() == false)
            {
                QDomElement subtag = subnode.toElement();
                if (subtag.tagName() == "ID")
                {
                    if (fixtures.contains(subtag.text()) == true)
                        QFAIL("Same fixture multiple times!");
                    else
                        fixtures.append(subtag.text());

                    if (subtag.text() == "34")
                    {
                        expectHead = 5;
                        expectIntensity = 64;
                        expectBackward = true;
                        expectStartOffset = 27;
                    }
                    else
                    {
                        if (subtag.text() == "12")
                        {
                            expectHead = 3;
                            expectIntensity = 128;
                        }
                        else
                        {
                            expectHead = 7;
                            expectIntensity = 255;
                        }
                        expectBackward = false;
                        expectStartOffset = 0;
                    }

                    fixtureid++;
                }
                else if (subtag.tagName() == "Head")
                {
                    QCOMPARE(subtag.text().toInt(), expectHead);
                    fixturehead++;
                }
                else if (subtag.tagName() == "Direction")
                {
                    if (expectBackward == false && subtag.text() == "Backward")
                    {
                        QFAIL("Not expecting reversal!");
                    }

                    fixturedirection++;
                }
                else if (subtag.tagName() == "StartOffset")
                {
                    QCOMPARE(subtag.text().toInt(), expectStartOffset);
                    fixtureStartOffset++;
                }
                else if (subtag.tagName() == "Intensity")
                {
                    QCOMPARE(subtag.text().toInt(), expectIntensity);
                    intensity = true;
                }
                else
                {
                    QFAIL("Unexpected fixture tag!");
                }

                subnode = subnode.nextSibling();
            }
        }
        else
        {
            QFAIL("Unexpected EFX tag!");
        }

        node = node.nextSibling();
    }

    QCOMPARE(fixtures.size(), 3);
    QCOMPARE(fixtureid, 3);
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
    QVERIFY(intensity == true);
}

void EFX_Test::preRunPostRun()
{
    QList<Universe*> ua;
    ua.append(new Universe(0, new GrandMaster()));
    MasterTimerStub timer(m_doc, ua);

    EFX* e = new EFX(m_doc);
    e->setName("Test EFX");
    QVERIFY(e->m_fader == NULL);

    QSignalSpy spy(e, SIGNAL(running(quint32)));
    e->preRun(&timer);
    QVERIFY(e->m_fader != NULL);
    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy[0].size(), 1);
    QCOMPARE(spy[0][0].toUInt(), e->id());
    e->postRun(&timer, ua);
    QVERIFY(e->m_fader == NULL);
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
    QCOMPARE(ef1->m_intensity, 0.2);
    QCOMPARE(ef2->m_intensity, 0.2);

    e->preRun(m_doc->masterTimer());

    e->adjustAttribute(0.5);
    QCOMPARE(e->m_fader->intensity(), 0.5);
    QCOMPARE(ef1->intensity(), 0.5);
    QCOMPARE(ef2->intensity(), 0.5);

    QList<Universe*> ua;
    ua.append(new Universe(0, new GrandMaster()));
    e->postRun(m_doc->masterTimer(), ua);
}

QTEST_APPLESS_MAIN(EFX_Test)
