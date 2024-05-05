/*
  Q Light Controller - Unit test
  fadechannel_test.cpp

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

#include "fadechannel_test.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "qlcchannel.h"
#include "qlcmacros.h"
#include "qlcfile.h"
#include "universe.h"

#define private public
#include "fadechannel.h"
#undef private

#include "../common/resource_paths.h"

void FadeChannel_Test::address()
{
    Doc doc(this);
    Fixture *fxi = new Fixture(&doc);
    fxi->setAddress(400);
    fxi->setChannels(5);
    doc.addFixture(fxi);

    FadeChannel fc(&doc, Fixture::invalidId(), 2);
    QCOMPARE(fc.address(), quint32(2));

    FadeChannel fc1(&doc, fxi->id(), 2);
    QCOMPARE(fc1.address(), quint32(402));

    FadeChannel fc2(&doc, 12345, QLCChannel::invalid());
    QCOMPARE(fc2.address(), QLCChannel::invalid());
}

void FadeChannel_Test::addressInUniverse()
{
    Doc doc(this);
    Fixture *fxi = new Fixture(&doc);
    fxi->setAddress(UNIVERSE_SIZE);
    fxi->setChannels(5);
    doc.addFixture(fxi);

    FadeChannel fc(&doc, Fixture::invalidId(), 2);
    QCOMPARE(fc.addressInUniverse(), quint32(2));

    FadeChannel fc1(&doc, fxi->id(), QLCChannel::invalid());
    QCOMPARE(fc1.addressInUniverse(), QLCChannel::invalid());

    FadeChannel fc2(&doc, 12345, QLCChannel::invalid());
    QCOMPARE(fc2.addressInUniverse(), QLCChannel::invalid());
}

void FadeChannel_Test::comparison()
{
    Doc doc(this);

    FadeChannel ch1(&doc, 0, 0);
    FadeChannel ch2(&doc, 1, 0);
    FadeChannel ch3(&doc, 0, 0);
    QVERIFY((ch1 == ch2) == false);
    QVERIFY((ch1 == ch3) == true);
}

void FadeChannel_Test::type()
{
    Doc doc(this);

    FadeChannel fc(&doc, Fixture::invalidId(), 2);

    // Only a channel given, no fixture at the address -> intensity
    QCOMPARE(fc.flags() & FadeChannel::HTP, (int)FadeChannel::HTP);
    QCOMPARE(fc.flags() & FadeChannel::Intensity, (int)FadeChannel::Intensity);
    QCOMPARE(fc.flags() & FadeChannel::CanFade, (int)FadeChannel::CanFade);

    Fixture *fxi = new Fixture(&doc);
    fxi->setAddress(10);
    fxi->setChannels(5);
    doc.addFixture(fxi);

    // Fixture and channel given, fixture is a dimmer -> intensity
    FadeChannel fc1(&doc, fxi->id(), 2);
    QCOMPARE(fc1.flags() & FadeChannel::HTP, (int)FadeChannel::HTP);
    QCOMPARE(fc1.flags() & FadeChannel::Intensity, (int)FadeChannel::Intensity);
    QCOMPARE(fc1.flags() & FadeChannel::CanFade, (int)FadeChannel::CanFade);

    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(doc.fixtureDefCache()->loadMap(dir) == true);

    QLCFixtureDef *def = doc.fixtureDefCache()->fixtureDef("Futurelight", "DJScan250");
    QVERIFY(def != NULL);

    QLCFixtureMode *mode = def->modes().first();
    QVERIFY(mode != NULL);

    fxi = new Fixture(&doc);
    fxi->setAddress(0);
    fxi->setFixtureDefinition(def, mode);
    doc.addFixture(fxi);

    // Fixture and channel given, but channel is beyond fixture's channels -> intensity
    FadeChannel fc2(&doc, fxi->id(), 50);
    QCOMPARE(fc2.flags() & FadeChannel::HTP, (int)FadeChannel::HTP);
    QCOMPARE(fc2.flags() & FadeChannel::Intensity, (int)FadeChannel::Intensity);
    QCOMPARE(fc2.flags() & FadeChannel::CanFade, (int)FadeChannel::CanFade);

    // Only a channel given, no fixture given but a fixture occupies the address.
    // Check that reverse address -> fixture lookup works.
    FadeChannel fc3(&doc, Fixture::invalidId(), 2);
    QCOMPARE(fc3.flags() & FadeChannel::LTP, (int)FadeChannel::LTP);
    QCOMPARE(fc3.flags() & FadeChannel::CanFade, (int)FadeChannel::CanFade);

    // Fixture and channel given, but fixture doesn't exist -> intensity
    FadeChannel fc4(&doc, 12345, 2);
    QCOMPARE(fc4.flags() & FadeChannel::HTP, (int)FadeChannel::HTP);
    QCOMPARE(fc4.flags() & FadeChannel::Intensity, (int)FadeChannel::Intensity);
    QCOMPARE(fc4.flags() & FadeChannel::CanFade, (int)FadeChannel::CanFade);

    // channel 3 cannot fade
    fxi->setChannelCanFade(3, false);

    FadeChannel fc5(&doc, fxi->id(), 3);
    QCOMPARE(fc5.flags() & FadeChannel::LTP, (int)FadeChannel::LTP);
    QCOMPARE(fc5.flags() & FadeChannel::CanFade, 0);

    // force channel 0 (Pan) to be HTP
    QList<int> forced;
    forced << 0;
    fxi->setForcedHTPChannels(forced);

    FadeChannel fc6(&doc, fxi->id(), 0);
    QCOMPARE(fc6.flags() & FadeChannel::HTP, (int)FadeChannel::HTP);
    QCOMPARE(fc6.flags() & FadeChannel::LTP, 0);
    QCOMPARE(fc6.flags() & FadeChannel::CanFade, (int)FadeChannel::CanFade);

    // add another generic dimmer
    fxi = new Fixture(&doc);
    fxi->setAddress(100);
    fxi->setChannels(5);
    forced.clear();
    forced << 2;
    // force channel 2 to be LTP
    fxi->setForcedLTPChannels(forced);
    doc.addFixture(fxi);

    FadeChannel fc7(&doc, fxi->id(), 2);
    QCOMPARE(fc7.flags() & FadeChannel::LTP, (int)FadeChannel::LTP);
    QCOMPARE(fc7.flags() & FadeChannel::HTP, 0);
    QCOMPARE(fc7.flags() & FadeChannel::CanFade, (int)FadeChannel::CanFade);

    // unset a flag
    fc.removeFlag(FadeChannel::CanFade);
    QCOMPARE(fc.flags() & FadeChannel::CanFade, 0);

    // set a flag
    QCOMPARE(fc.flags() & FadeChannel::Flashing, 0);
    fc.addFlag(FadeChannel::Flashing);
    QCOMPARE(fc.flags() & FadeChannel::Flashing, (int)FadeChannel::Flashing);
}

void FadeChannel_Test::start()
{
    FadeChannel fch;
    QCOMPARE(fch.start(), uchar(0));

    for (uint i = 0; i <= 255; i++)
    {
        fch.setStart(i);
        QCOMPARE(fch.start(), uchar(i));
    }
}

void FadeChannel_Test::target()
{
    FadeChannel fch;
    QCOMPARE(fch.target(), uchar(0));

    for (uint i = 0; i <= 255; i++)
    {
        fch.setTarget(i);
        QCOMPARE(fch.target(), uchar(i));
    }
}

void FadeChannel_Test::current()
{
    FadeChannel fch;
    QCOMPARE(fch.current(), uchar(0));

    for (uint i = 0; i <= 255; i++)
    {
        fch.setCurrent(i);
        QCOMPARE(fch.current(), uchar(i));
        QCOMPARE(fch.current(0.4), uchar(floor((qreal(i) * 0.4) + 0.5)));
    }
}

void FadeChannel_Test::ready()
{
    FadeChannel ch;
    QVERIFY(ch.isReady() == false);
    ch.setReady(true);
    QVERIFY(ch.isReady() == true);
}

void FadeChannel_Test::fadeTime()
{
    FadeChannel ch;
    QVERIFY(ch.fadeTime() == 0);
    ch.setFadeTime(50);
    QVERIFY(ch.fadeTime() == 50);
}

void FadeChannel_Test::nextStep()
{
    FadeChannel fc;
    fc.setStart(0);
    fc.setTarget(250);

    fc.setFadeTime(1000);

    for (int i = 5; i < 250; i += 5)
    {
        int value = fc.nextStep(MasterTimer::tick());
        QCOMPARE(value, i);
    }

    fc.setCurrent(0);
    fc.setReady(false);
    fc.setFadeTime(0);
    fc.setElapsed(0);
    QCOMPARE(fc.nextStep(MasterTimer::tick()), uchar(250));

    fc.setCurrent(0);
    fc.setReady(false);
    fc.setFadeTime(MasterTimer::tick() / 5);
    fc.setElapsed(0);
    QCOMPARE(fc.nextStep(MasterTimer::tick()), uchar(250));

    fc.setCurrent(0);
    fc.setReady(false);
    fc.setFadeTime(1 * MasterTimer::tick());
    fc.setElapsed(0);
    QCOMPARE(fc.nextStep(MasterTimer::tick()), uchar(250));

    fc.setCurrent(0);
    fc.setReady(false);
    fc.setFadeTime(2 * MasterTimer::tick());
    fc.setElapsed(0);
    QCOMPARE(fc.nextStep(MasterTimer::tick()), uchar(125));
    QCOMPARE(fc.nextStep(MasterTimer::tick()), uchar(250));

    fc.setCurrent(0);
    fc.setReady(false);
    fc.setFadeTime(5 * MasterTimer::tick());
    fc.setElapsed(0);
    QCOMPARE(fc.nextStep(MasterTimer::tick()), uchar(50));
    QCOMPARE(fc.nextStep(MasterTimer::tick()), uchar(100));
    QCOMPARE(fc.nextStep(MasterTimer::tick()), uchar(150));
    QCOMPARE(fc.nextStep(MasterTimer::tick()), uchar(200));
    QCOMPARE(fc.nextStep(MasterTimer::tick()), uchar(250));

    // Maximum elapsed() reached
    fc.setCurrent(0);
    fc.setTarget(255);
    fc.setReady(false);
    fc.setElapsed(UINT_MAX);
    fc.setFadeTime(5 * MasterTimer::tick());
    QCOMPARE(fc.nextStep(MasterTimer::tick()), uchar(255));
    QCOMPARE(fc.elapsed(), UINT_MAX);
    QCOMPARE(fc.nextStep(MasterTimer::tick()), uchar(255));
    QCOMPARE(fc.elapsed(), UINT_MAX);
    QCOMPARE(fc.nextStep(MasterTimer::tick()), uchar(255));
    QCOMPARE(fc.elapsed(), UINT_MAX);
    QCOMPARE(fc.nextStep(MasterTimer::tick()), uchar(255));
    QCOMPARE(fc.elapsed(), UINT_MAX);
    QCOMPARE(fc.nextStep(MasterTimer::tick()), uchar(255));
    QCOMPARE(fc.elapsed(), UINT_MAX);

    // Channel marked as ready
    fc.setReady(true);
    fc.setElapsed(0);
    QCOMPARE(fc.nextStep(MasterTimer::tick()), uchar(255));
    QCOMPARE(fc.elapsed(), MasterTimer::tick() * 1);
    QCOMPARE(fc.nextStep(MasterTimer::tick()), uchar(255));
    QCOMPARE(fc.elapsed(), MasterTimer::tick() * 2);
    QCOMPARE(fc.nextStep(MasterTimer::tick()), uchar(255));
    QCOMPARE(fc.elapsed(), MasterTimer::tick() * 3);
}

void FadeChannel_Test::calculateCurrent()
{
    FadeChannel fch;
    fch.setStart(0);
    fch.setTarget(255);

    // Simple: 255 ticks to fade from 0 to 255
    for (uint time = 0; time < 255; time++)
        QCOMPARE(fch.calculateCurrent(255, time), uchar(time));

    // Same thing, but the value should stay at 255 same after 255 ticks
    for (uint time = 0; time <= 512; time++)
        QCOMPARE(fch.calculateCurrent(255, time), uchar(MIN(time, 255)));

    // Simple reverse: 255 ticks to fade from 255 to 0
    fch.setStart(255);
    fch.setTarget(0);
    fch.setReady(false);
    for (uint time = 0; time <= 255; time++)
        QCOMPARE(fch.calculateCurrent(255, time), uchar(255 - time));

    // A bit more complex involving decimals that don't produce round integers
    fch.setStart(13);
    fch.setTarget(147);
    fch.setReady(false);
    QCOMPARE(fch.calculateCurrent(13, 0), uchar(13));
    QCOMPARE(fch.calculateCurrent(13, 1), uchar(23));
    QCOMPARE(fch.calculateCurrent(13, 2), uchar(33));
    QCOMPARE(fch.calculateCurrent(13, 3), uchar(43));
    QCOMPARE(fch.calculateCurrent(13, 4), uchar(54));
    QCOMPARE(fch.calculateCurrent(13, 5), uchar(64));
    QCOMPARE(fch.calculateCurrent(13, 6), uchar(74));
    QCOMPARE(fch.calculateCurrent(13, 7), uchar(85));
    QCOMPARE(fch.calculateCurrent(13, 8), uchar(95));
    QCOMPARE(fch.calculateCurrent(13, 9), uchar(105));
    QCOMPARE(fch.calculateCurrent(13, 10), uchar(116));
    QCOMPARE(fch.calculateCurrent(13, 11), uchar(126));
    QCOMPARE(fch.calculateCurrent(13, 12), uchar(136));
    QCOMPARE(fch.calculateCurrent(13, 13), uchar(147));

    // One more to check slower operation (200 ticks for 144 steps)
    fch.setStart(245);
    fch.setTarget(101);
    fch.setReady(false);
    QCOMPARE(fch.calculateCurrent(200, 0), uchar(245));
    QCOMPARE(fch.calculateCurrent(200, 1), uchar(245));
    QCOMPARE(fch.calculateCurrent(200, 2), uchar(244));
    QCOMPARE(fch.calculateCurrent(200, 3), uchar(243));
    QCOMPARE(fch.calculateCurrent(200, 4), uchar(243));
    QCOMPARE(fch.calculateCurrent(200, 5), uchar(242));
    QCOMPARE(fch.calculateCurrent(200, 6), uchar(241));
    QCOMPARE(fch.calculateCurrent(200, 7), uchar(240));
    QCOMPARE(fch.calculateCurrent(200, 8), uchar(240));
    QCOMPARE(fch.calculateCurrent(200, 9), uchar(239));
    QCOMPARE(fch.calculateCurrent(200, 10), uchar(238));
    QCOMPARE(fch.calculateCurrent(200, 11), uchar(238));
    // Skip...
    QCOMPARE(fch.calculateCurrent(200, 100), uchar(173));
    QCOMPARE(fch.calculateCurrent(200, 101), uchar(173));
    QCOMPARE(fch.calculateCurrent(200, 102), uchar(172));
    // Skip...
    QCOMPARE(fch.calculateCurrent(200, 198), uchar(103));
    QCOMPARE(fch.calculateCurrent(200, 199), uchar(102));
    QCOMPARE(fch.calculateCurrent(200, 200), uchar(101));
}

QTEST_APPLESS_MAIN(FadeChannel_Test)
