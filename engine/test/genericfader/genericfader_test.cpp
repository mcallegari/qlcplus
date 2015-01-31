/*
  Q Light Controller - Unit test
  genericfader_test.cpp

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

#include "genericfader_test.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "fadechannel.h"
#include "qlcchannel.h"
#include "universe.h"
#include "qlcfile.h"
#include "doc.h"

#define private public
#include "genericfader.h"
#undef private

#include "../common/resource_paths.h"

void GenericFader_Test::initTestCase()
{
    m_doc = new Doc(this);
    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_doc->fixtureDefCache()->load(dir) == true);
}

void GenericFader_Test::init()
{
    Fixture* fxi = new Fixture(m_doc);
    QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);

    QLCFixtureMode* mode = def->mode("Mode 1");
    QVERIFY(mode != NULL);

    fxi->setFixtureDefinition(def, mode);
    fxi->setAddress(10);
    m_doc->addFixture(fxi);
}

void GenericFader_Test::cleanup()
{
    m_doc->clearContents();
}

void GenericFader_Test::addRemove()
{
    GenericFader fader(m_doc);

    FadeChannel fc;
    fc.setFixture(m_doc, 0);
    fc.setChannel(0);

    FadeChannel wrong;
    fc.setFixture(m_doc, 0);

    QCOMPARE(fader.m_channels.count(), 0);
    QVERIFY(fader.m_channels.contains(fc) == false);

    fader.add(fc);
    QVERIFY(fader.m_channels.contains(fc) == true);
    QCOMPARE(fader.m_channels.count(), 1);

    fader.remove(wrong);
    QVERIFY(fader.m_channels.contains(fc) == true);
    QCOMPARE(fader.m_channels.count(), 1);

    fader.remove(fc);
    QVERIFY(fader.m_channels.contains(fc) == false);
    QCOMPARE(fader.m_channels.count(), 0);

    fc.setChannel(0);
    fader.add(fc);
    QVERIFY(fader.m_channels.contains(fc) == true);

    fc.setChannel(1);
    fader.add(fc);
    QVERIFY(fader.m_channels.contains(fc) == true);

    fc.setChannel(2);
    fader.add(fc);
    QVERIFY(fader.m_channels.contains(fc) == true);
    QCOMPARE(fader.m_channels.count(), 3);

    fader.removeAll();
    QCOMPARE(fader.m_channels.count(), 0);

    fc.setFixture(m_doc, 0);
    fc.setChannel(0);
    fc.setTarget(127);
    fader.add(fc);
    QCOMPARE(fader.m_channels.size(), 1);
    QCOMPARE(fader.m_channels[fc].target(), uchar(127));

    fc.setTarget(63);
    fader.add(fc);
    QCOMPARE(fader.m_channels.size(), 1);
    QCOMPARE(fader.m_channels[fc].target(), uchar(63));

    fc.setCurrent(63);
    fader.add(fc);
    QCOMPARE(fader.m_channels.size(), 1);
    QCOMPARE(fader.m_channels[fc].target(), uchar(63));
}

void GenericFader_Test::writeZeroFade()
{
    QList<Universe*> ua;
    ua.append(new Universe(0, new GrandMaster()));
    GenericFader fader(m_doc);

    FadeChannel fc;
    fc.setFixture(m_doc, 0);
    fc.setChannel(5);
    fc.setStart(0);
    fc.setTarget(255);
    fc.setFadeTime(0);

    fader.add(fc);
    QCOMPARE(ua[0]->preGMValues()[15], (char) 0);
    fader.write(ua);
    QCOMPARE(ua[0]->preGMValues()[15], (char) 255);
}

void GenericFader_Test::writeLoop()
{
    QList<Universe*> ua;
    ua.append(new Universe(0, new GrandMaster()));
    GenericFader fader(m_doc);

    FadeChannel fc;
    fc.setFixture(m_doc, 0);
    fc.setChannel(5);
    fc.setStart(0);
    fc.setTarget(250);
    fc.setFadeTime(1000);
    fader.add(fc);

    QCOMPARE(ua[0]->preGMValues()[15], (char) 0);

    int expected = 0;
    for (int i = MasterTimer::tick(); i <= 1000; i += MasterTimer::tick())
    {
        ua[0]->zeroIntensityChannels();
        fader.write(ua);

        int actual = uchar(ua[0]->preGMValues()[15]);
        expected += 5;
        QCOMPARE(actual, expected);
    }
}

void GenericFader_Test::adjustIntensity()
{
    QList<Universe*> ua;
    ua.append(new Universe(0, new GrandMaster()));
    GenericFader fader(m_doc);

    FadeChannel fc;

    // HTP channel
    fc.setFixture(m_doc, 0);
    fc.setChannel(5);
    fc.setStart(0);
    fc.setTarget(250);
    fc.setFadeTime(1000);
    fader.add(fc);

    // LTP channel
    fc.setChannel(0);
    fader.add(fc);

    qreal intensity = 0.5;
    fader.adjustIntensity(intensity);
    QCOMPARE(fader.intensity(), intensity);

    int expected = 0;
    for (int i = MasterTimer::tick(); i <= 1000; i += MasterTimer::tick())
    {
        ua[0]->zeroIntensityChannels();
        fader.write(ua);

        expected += 5;

        // GenericFader should apply intensity only to HTP channels
        int actual = uchar(ua[0]->preGMValues()[15]);
        int expectedWithIntensity = floor((qreal(expected) * intensity) + 0.5);
        QVERIFY(actual == expectedWithIntensity);

        // No intensity adjustment on LTP channels
        actual = uchar(ua[0]->preGMValues()[10]);
        QVERIFY(actual == expected);
    }
}

QTEST_APPLESS_MAIN(GenericFader_Test)
