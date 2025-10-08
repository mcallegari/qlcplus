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
    QVERIFY(m_doc->fixtureDefCache()->loadMap(dir) == true);
}

void GenericFader_Test::init()
{
    Fixture *fxi = new Fixture(m_doc);
    QLCFixtureDef *def = m_doc->fixtureDefCache()->fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);

    QLCFixtureMode *mode = def->mode("Mode 1");
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
    QList<Universe*> ua = m_doc->inputOutputMap()->universes();
    QSharedPointer<GenericFader> fader = QSharedPointer<GenericFader>(new GenericFader());

    FadeChannel fc(m_doc, 0, 0);
    FadeChannel fc1(m_doc, 0, 1);
    FadeChannel fc2(m_doc, 0, 2);
    FadeChannel wrong(m_doc, 0, QLCChannel::invalid());
    quint32 chHash = GenericFader::channelHash(fc.fixture(), fc.channel());

    QCOMPARE(fader->m_channels.count(), 0);
    QVERIFY(fader->m_channels.contains(chHash) == false);

    fader->add(fc);
    QVERIFY(fader->m_channels.contains(chHash) == true);
    QCOMPARE(fader->m_channels.count(), 1);

    fader->remove(&wrong);
    QVERIFY(fader->m_channels.contains(chHash) == true);
    QCOMPARE(fader->m_channels.count(), 1);

    FadeChannel *fc3 = fader->getChannelFader(m_doc, ua[0], 0, 0);
    fader->remove(fc3);
    QVERIFY(fader->m_channels.contains(chHash) == false);
    QCOMPARE(fader->m_channels.count(), 0);

    fader->add(fc);
    QVERIFY(fader->m_channels.contains(chHash) == true);

    fader->add(fc1);
    chHash = GenericFader::channelHash(fc.fixture(), fc.channel());
    QVERIFY(fader->m_channels.contains(chHash) == true);

    fader->add(fc2);
    chHash = GenericFader::channelHash(fc.fixture(), fc.channel());
    QVERIFY(fader->m_channels.contains(chHash) == true);
    QCOMPARE(fader->m_channels.count(), 3);

    fader->removeAll();
    QCOMPARE(fader->m_channels.count(), 0);

    fc.setTarget(127);
    fader->add(fc);
    chHash = GenericFader::channelHash(fc.fixture(), fc.channel());
    QCOMPARE(fader->m_channels.size(), 1);
    QCOMPARE(fader->m_channels[chHash].target(), uchar(127));

    fc.setTarget(63);
    fader->add(fc);
    QCOMPARE(fader->m_channels.size(), 1);
    QCOMPARE(fader->m_channels[chHash].target(), uchar(63));

    fc.setCurrent(63);
    fader->add(fc);
    QCOMPARE(fader->m_channels.size(), 1);
    QCOMPARE(fader->m_channels[chHash].target(), uchar(63));
}

void GenericFader_Test::writeZeroFade()
{
    QList<Universe*> ua = m_doc->inputOutputMap()->universes();
    QSharedPointer<GenericFader> fader = ua[0]->requestFader();

    FadeChannel fc(m_doc, 0, 5);
    fc.setStart(0);
    fc.setTarget(255);
    fc.setFadeTime(0);

    fader->add(fc);
    QCOMPARE(ua[0]->preGMValues()[15], (char) 0);
    fader->write(ua[0]);
    QCOMPARE(ua[0]->preGMValues()[15], (char) 255);
}

void GenericFader_Test::writeLoop()
{
    QList<Universe*> ua = m_doc->inputOutputMap()->universes();
    QSharedPointer<GenericFader> fader = ua[0]->requestFader();

    FadeChannel fc(m_doc, 0, 5);
    fc.setStart(0);
    fc.setTarget(250);
    fc.setFadeTime(1000);
    fader->add(fc);

    QCOMPARE(ua[0]->preGMValues()[15], (char) 0);

    int expected = 0;
    for (int i = MasterTimer::tick(); i <= 1000; i += MasterTimer::tick())
    {
        ua[0]->zeroIntensityChannels();
        fader->write(ua[0]);

        int actual = uchar(ua[0]->preGMValues()[15]);
        expected += 5;
        QCOMPARE(actual, expected);
    }
}

void GenericFader_Test::adjustIntensity()
{
    QList<Universe*> ua = m_doc->inputOutputMap()->universes();
    QSharedPointer<GenericFader> fader = ua[0]->requestFader();

    FadeChannel fc(m_doc, 0, 5);
    FadeChannel fc1(m_doc, 0, 0);

    // HTP channel
    fc.setStart(0);
    fc.setTarget(250);
    fc.setFadeTime(1000);
    fader->add(fc);

    // LTP channel
    fc1.setStart(0);
    fc1.setTarget(250);
    fc1.setFadeTime(1000);
    fader->add(fc1);

    qreal intensity = 0.5;
    fader->adjustIntensity(intensity);
    QCOMPARE(fader->intensity(), intensity);

    int expected = 0;
    for (int i = MasterTimer::tick(); i <= 1000; i += MasterTimer::tick())
    {
        ua[0]->zeroIntensityChannels();
        fader->write(ua[0]);

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
