/*
  Q Light Controller - Unit test
  universe_test.cpp

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
#include <sys/time.h>

#include "universe_test.h"

#define protected public
#include "universe.h"
#undef protected

#include "grandmaster.h"

void Universe_Test::initTestCase()
{
    m_gm = new GrandMaster(this);
    m_uni = new Universe(0, m_gm, this);
}

void Universe_Test::cleanupTestCase()
{
    delete m_uni;
    delete m_gm;
}

void Universe_Test::initial()
{
    QCOMPARE(m_uni->name(), QString());
    QCOMPARE(m_uni->id(), quint32(0));
    QCOMPARE(m_uni->usedChannels(), short(0));
    QCOMPARE(m_uni->hasChanged(), false);
    QVERIFY(m_uni->inputPatch() == NULL);
    QVERIFY(m_uni->outputPatch() == NULL);
    QVERIFY(m_uni->feedbackPatch() == NULL);
}

void Universe_Test::setGMValue()
{
    m_uni->setChannelCapability(0, QLCChannel::Intensity);
    m_uni->setChannelCapability(1, QLCChannel::Intensity);
    m_uni->setChannelCapability(2, QLCChannel::Pan);
    m_uni->setChannelCapability(3, QLCChannel::Tilt);
    m_uni->setChannelCapability(4, QLCChannel::Intensity);

    m_uni->write(0, 10);
    m_uni->write(1, 20);
    m_uni->write(2, 30);
    m_uni->write(3, 40);
    m_uni->write(4, 50);

    m_gm->setValue(63);
    QCOMPARE(m_uni->postGMValues()->at(0), char(2));
    QCOMPARE(m_uni->postGMValues()->at(1), char(5));
    QCOMPARE(m_uni->postGMValues()->at(2), char(30));
    QCOMPARE(m_uni->postGMValues()->at(3), char(40));
    QCOMPARE(m_uni->postGMValues()->at(4), char(12));

    m_gm->setChannelMode(GrandMaster::AllChannels);
    QCOMPARE(m_uni->postGMValues()->at(0), char(2));
    QCOMPARE(m_uni->postGMValues()->at(1), char(5));
    QCOMPARE(m_uni->postGMValues()->at(2), char(7));
    QCOMPARE(m_uni->postGMValues()->at(3), char(10));
    QCOMPARE(m_uni->postGMValues()->at(4), char(12));

    m_gm->setChannelMode(GrandMaster::Intensity);
    QCOMPARE(m_uni->postGMValues()->at(0), char(2));
    QCOMPARE(m_uni->postGMValues()->at(1), char(5));
    QCOMPARE(m_uni->postGMValues()->at(2), char(30));
    QCOMPARE(m_uni->postGMValues()->at(3), char(40));
    QCOMPARE(m_uni->postGMValues()->at(4), char(12));

    m_gm->setValueMode(GrandMaster::Limit);
    QCOMPARE(m_uni->postGMValues()->at(0), char(10));
    QCOMPARE(m_uni->postGMValues()->at(1), char(20));
    QCOMPARE(m_uni->postGMValues()->at(2), char(30));
    QCOMPARE(m_uni->postGMValues()->at(3), char(40));
    QCOMPARE(m_uni->postGMValues()->at(4), char(50));

    m_gm->setValue(5);
    QCOMPARE(m_uni->postGMValues()->at(0), char(5));
    QCOMPARE(m_uni->postGMValues()->at(1), char(5));
    QCOMPARE(m_uni->postGMValues()->at(2), char(30));
    QCOMPARE(m_uni->postGMValues()->at(3), char(40));
    QCOMPARE(m_uni->postGMValues()->at(4), char(5));

    m_gm->setChannelMode(GrandMaster::AllChannels);
    QCOMPARE(m_uni->postGMValues()->at(0), char(5));
    QCOMPARE(m_uni->postGMValues()->at(1), char(5));
    QCOMPARE(m_uni->postGMValues()->at(2), char(5));
    QCOMPARE(m_uni->postGMValues()->at(3), char(5));
    QCOMPARE(m_uni->postGMValues()->at(4), char(5));
}

void Universe_Test::applyGM()
{
    QCOMPARE(m_uni->m_gMIntensityChannels.size(), 0);
    QCOMPARE(m_uni->m_gMNonIntensityChannels.size(), 0);
/*
    QCOMPARE(m_uni->applyGM(0, 50, QLCChannel::Intensity), uchar(50));
    QCOMPARE(m_uni->applyGM(0, 200, QLCChannel::Colour), uchar(200));

    m_gm->setValue(127);
    QCOMPARE(m_uni->applyGM(0, 50, QLCChannel::Intensity), uchar(25));
    QCOMPARE(m_uni->applyGM(0, 200, QLCChannel::Intensity), uchar(100));
    QCOMPARE(m_uni->applyGM(0, 200, QLCChannel::Colour), uchar(200));

    m_gm->setValueMode(Universe::Limit);
    QCOMPARE(m_uni->applyGM(0, 50, QLCChannel::Intensity), uchar(50));
    QCOMPARE(m_uni->applyGM(0, 200, QLCChannel::Intensity), uchar(127));
    QCOMPARE(m_uni->applyGM(0, 255, QLCChannel::Colour), uchar(255));

    m_gm->setChannelMode(Universe::AllChannels);
    QCOMPARE(m_uni->applyGM(0, 50, QLCChannel::Intensity), uchar(50));
    QCOMPARE(m_uni->applyGM(0, 200, QLCChannel::Intensity), uchar(127));
    QCOMPARE(m_uni->applyGM(0, 255, QLCChannel::Colour), uchar(127));

    m_gm->setValueMode(Universe::Reduce);
    QCOMPARE(m_uni->applyGM(0, 50, QLCChannel::Intensity), uchar(25));
    QCOMPARE(m_uni->applyGM(0, 200, QLCChannel::Intensity), uchar(100));
    QCOMPARE(m_uni->applyGM(0, 255, QLCChannel::Colour), uchar(127));

    QCOMPARE(m_uni->m_gMIntensityChannels.size(), 1);
    QCOMPARE(m_uni->m_gMNonIntensityChannels.size(), 1);
*/
}

void Universe_Test::write()
{
/*
    QVERIFY(m_uni->write(10, 255, QLCChannel::Intensity) == false);
    QCOMPARE(m_uni->postGMValues()->at(9), char(0));
    QCOMPARE(m_uni->postGMValues()->at(4), char(0));
    QCOMPARE(m_uni->postGMValues()->at(0), char(0));

    QVERIFY(m_uni->write(9, 255, QLCChannel::Intensity) == true);
    QCOMPARE(m_uni->postGMValues()->at(9), char(255));
    QCOMPARE(m_uni->postGMValues()->at(4), char(0));
    QCOMPARE(m_uni->postGMValues()->at(0), char(0));

    QVERIFY(m_uni->write(0, 255, QLCChannel::Intensity) == true);
    QCOMPARE(m_uni->postGMValues()->at(9), char(255));
    QCOMPARE(m_uni->postGMValues()->at(4), char(0));
    QCOMPARE(m_uni->postGMValues()->at(0), char(255));

    m_gm->setValue(127);
    QCOMPARE(m_uni->postGMValues()->at(9), char(127));
    QCOMPARE(m_uni->postGMValues()->at(4), char(0));
    QCOMPARE(m_uni->postGMValues()->at(0), char(127));

    QVERIFY(m_uni->write(4, 200, QLCChannel::Intensity) == true);
    QCOMPARE(m_uni->postGMValues()->at(9), char(127));
    QCOMPARE(m_uni->postGMValues()->at(4), char(100));
    QCOMPARE(m_uni->postGMValues()->at(0), char(127));
*/
}

void Universe_Test::writeRelative()
{
/*
    // past the end of the array
    QVERIFY(m_uni->write(10, 255, QLCChannel::Pan, true) == false);
    QCOMPARE(m_uni-> m_relativeValues[9], short(0));
    QCOMPARE(m_uni->m_relativeValues[4], short(0));
    QCOMPARE(m_uni->m_relativeValues[0], short(0));
    QCOMPARE(m_uni->postGMValues()->at(9), char(0));
    QCOMPARE(m_uni->postGMValues()->at(4), char(0));
    QCOMPARE(m_uni->postGMValues()->at(0), char(0));

    // 127 == 0
    QVERIFY(m_uni->write(9, 127, QLCChannel::Pan, true) == true);
    QCOMPARE(m_uni->m_relativeValues[9], short(0));
    QCOMPARE(m_uni->m_relativeValues[4], short(0));
    QCOMPARE(m_uni->m_relativeValues[0], short(0));
    QCOMPARE(int(m_uni->postGMValues()->at(9)), 0);
    QCOMPARE(int(m_uni->postGMValues()->at(4)), 0);
    QCOMPARE(int(m_uni->postGMValues()->at(0)), 0);

    // 255 == -128
    QVERIFY(m_uni->write(9, 255, QLCChannel::Pan, true) == true);
    QCOMPARE(m_uni->m_relativeValues[9], short(128));
    QCOMPARE(m_uni->m_relativeValues[4], short(0));
    QCOMPARE(m_uni->m_relativeValues[0], short(0));
    QCOMPARE(m_uni->postGMValues()->at(9), char(128));
    QCOMPARE(m_uni->postGMValues()->at(4), char(0));
    QCOMPARE(m_uni->postGMValues()->at(0), char(0));

    // 0 == -127
    QVERIFY(m_uni->write(9, 0, QLCChannel::Pan, true) == true);
    QCOMPARE(m_uni->m_relativeValues[9], short(1));
    QCOMPARE(m_uni->m_relativeValues[4], short(0));
    QCOMPARE(m_uni->m_relativeValues[0], short(0));
    QCOMPARE(m_uni->postGMValues()->at(9), char(1));
    QCOMPARE(m_uni->postGMValues()->at(4), char(0));
    QCOMPARE(m_uni->postGMValues()->at(0), char(0));

    m_uni->reset();

    QVERIFY(m_uni->write(9, 85, QLCChannel::Pan, false) == true);
    QCOMPARE(m_uni->postGMValues()->at(9), char(85));
    QVERIFY(m_uni->write(9, 117, QLCChannel::Pan, true) == true);
    QCOMPARE(m_uni->postGMValues()->at(9), char(75));
    QVERIFY(m_uni->write(9, 75, QLCChannel::Pan, false) == true);
    QCOMPARE(m_uni->postGMValues()->at(9), char(65));

    m_uni->reset();

    QVERIFY(m_uni->write(9, 255, QLCChannel::Pan, false) == true);
    QCOMPARE(m_uni->postGMValues()->at(9), char(255));
    QVERIFY(m_uni->write(9, 255, QLCChannel::Pan, true) == true);
    QCOMPARE(m_uni->postGMValues()->at(9), char(255));

    m_uni->reset();

    QVERIFY(m_uni->write(9, 0, QLCChannel::Pan, false) == true);
    QCOMPARE(m_uni->postGMValues()->at(9), char(0));
    QVERIFY(m_uni->write(9, 0, QLCChannel::Pan, true) == true);
    QCOMPARE(m_uni->postGMValues()->at(9), char(0));
*/
}

void Universe_Test::reset()
{
    int i;

    for (i = 0; i < 128; i++)
    {
        m_uni->write(i, 200);
        QCOMPARE(m_uni->postGMValues()->at(i), char(200));
    }

    // Reset channels 10-127 (512 shouldn't cause a crash)
    m_uni->reset(10, 512);
    for (i = 0; i < 10; i++)
        QCOMPARE(m_uni->postGMValues()->at(i), char(200));
    for (i = 10; i < 128; i++)
        QCOMPARE(m_uni->postGMValues()->at(i), char(0));

    // Reset all
    m_uni->reset();
    for (i = 0; i < 128; i++)
        QCOMPARE(m_uni->postGMValues()->at(i), char(0));
}

void Universe_Test::setGMValueEfficiency()
{
    int i;

    for (i = 0; i < 512; i++)
        m_uni->write(i, 200);

    /* This applies 50%(127) Grand Master to ALL channels in all universes.
       I'm not really sure what kinds of figures to expect here, since this
       is just one part in the overall processor load. Typically I get ~0.37ms
       on an Intel Core 2 E6550@2.33GHz, which looks plausible to me:
       DMX frame interval is 1/44Hz =~ 23ms. Applying GM to ALL channels takes
       less than 1ms so there's a full 22ms to spare after GM. */
    QBENCHMARK
    {
        // This is slower than plain write() because UA has to dig out each
        // Intensity-enabled channel from its internal QSet.
        m_gm->setValue(127);
    }

    for (i = 0; i < 512; i++)
        QCOMPARE(m_uni->postGMValues()->at(i), char(100));
}

void Universe_Test::writeEfficiency()
{
    m_gm->setValue(127);

    int i;
    /* This applies 50%(127) Grand Master to ALL channels in all universes.
       I'm not really sure what kinds of figures to expect here, since this
       is just one part in the overall processor load. Typically I get ~0.15ms
       on an Intel Core 2 E6550@2.33GHz, which looks plausible to me:
       DMX frame interval is 1/44Hz =~ 23ms. Applying GM to ALL channels takes
       less than 1ms so there's a full 22ms to spare after GM. */
    QBENCHMARK
    {
        for (i = 0; i < int(512); i++)
            m_uni->write(i, 200);
    }

    for (i = 0; i < int(512 * 4); i++)
        QCOMPARE(m_uni->postGMValues()->at(i), char(100));
}

QTEST_APPLESS_MAIN(Universe_Test)
