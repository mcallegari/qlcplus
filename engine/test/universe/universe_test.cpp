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
#include "testmacros.h"

#define protected public
#include "universe.h"
#undef protected

#include "grandmaster.h"

void Universe_Test::init()
{
    m_gm = new GrandMaster(this);
    m_uni = new Universe(0, m_gm, this);
}

void Universe_Test::cleanup()
{
    delete m_uni; m_uni = 0;
    delete m_gm; m_gm = 0;
}

void Universe_Test::initial()
{
    QCOMPARE(m_uni->name(), QString("Universe 1"));
    QCOMPARE(m_uni->id(), quint32(0));
    QCOMPARE(m_uni->usedChannels(), short(0));
    QCOMPARE(m_uni->hasChanged(), false);
    QVERIFY(m_uni->inputPatch() == NULL);
    QVERIFY(m_uni->outputPatch() == NULL);
    QVERIFY(m_uni->feedbackPatch() == NULL);
    QVERIFY(m_uni->intensityChannels().isEmpty());
 
    QByteArray const preGM = m_uni->preGMValues();

    QCOMPARE(preGM.count(), 512);

    QByteArray const * postGM = m_uni->postGMValues();
    QVERIFY(postGM != NULL);
    QCOMPARE(postGM->count(), 512);

    for(ushort i = 0; i < 512; ++i)
    {
        QVERIFY(m_uni->channelCapabilities(i) == Universe::Undefined);
        QCOMPARE(int(preGM.at(i)), 0);
        QCOMPARE(int(postGM->at(i)), 0);
    }
}

void Universe_Test::channelCapabilities()
{
    m_uni->setChannelCapability(0, QLCChannel::Intensity);
    m_uni->setChannelCapability(1, QLCChannel::Intensity);
    m_uni->setChannelCapability(2, QLCChannel::Pan);
    m_uni->setChannelCapability(3, QLCChannel::Tilt, Universe::HTP);
    m_uni->setChannelCapability(4, QLCChannel::Intensity);

    QVERIFY(m_uni->channelCapabilities(0) == (Universe::Intensity|Universe::HTP));
    QVERIFY(m_uni->channelCapabilities(1) == (Universe::Intensity|Universe::HTP));
    QVERIFY(m_uni->channelCapabilities(2) == Universe::LTP);
    QVERIFY(m_uni->channelCapabilities(3) == Universe::HTP);
    QVERIFY(m_uni->channelCapabilities(4) == (Universe::Intensity|Universe::HTP));
}

void Universe_Test::grandMasterIntensityReduce()
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
    QCOMPARE(int(m_uni->postGMValues()->at(0)), int(2));
    QCOMPARE(int(m_uni->postGMValues()->at(1)), int(5));
    QCOMPARE(int(m_uni->postGMValues()->at(2)), int(30));
    QCOMPARE(int(m_uni->postGMValues()->at(3)), int(40));
    QCOMPARE(int(m_uni->postGMValues()->at(4)), int(12));
}

void Universe_Test::grandMasterIntensityLimit()
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

    m_gm->setValueMode(GrandMaster::Limit);

    m_gm->setValue(63);
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(10));
    QCOMPARE(quint8(m_uni->postGMValues()->at(1)), quint8(20));
    QCOMPARE(quint8(m_uni->postGMValues()->at(2)), quint8(30));
    QCOMPARE(quint8(m_uni->postGMValues()->at(3)), quint8(40));
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(50));

    m_gm->setValue(5);
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(5));
    QCOMPARE(quint8(m_uni->postGMValues()->at(1)), quint8(5));
    QCOMPARE(quint8(m_uni->postGMValues()->at(2)), quint8(30));
    QCOMPARE(quint8(m_uni->postGMValues()->at(3)), quint8(40));
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(5));
}

void Universe_Test::grandMasterAllChannelsReduce()
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

    m_gm->setChannelMode(GrandMaster::AllChannels);

    m_gm->setValue(63);
    QCOMPARE(int(m_uni->postGMValues()->at(0)), int(2));
    QCOMPARE(int(m_uni->postGMValues()->at(1)), int(5));
    QCOMPARE(int(m_uni->postGMValues()->at(2)), int(7));
    QCOMPARE(int(m_uni->postGMValues()->at(3)), int(10));
    QCOMPARE(int(m_uni->postGMValues()->at(4)), int(12));
}

void Universe_Test::grandMasterAllChannelsLimit()
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

    m_gm->setChannelMode(GrandMaster::AllChannels);
    m_gm->setValueMode(GrandMaster::Limit);

    m_gm->setValue(63);
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(10));
    QCOMPARE(quint8(m_uni->postGMValues()->at(1)), quint8(20));
    QCOMPARE(quint8(m_uni->postGMValues()->at(2)), quint8(30));
    QCOMPARE(quint8(m_uni->postGMValues()->at(3)), quint8(40));
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(50));

    m_gm->setValue(5);
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(5));
    QCOMPARE(quint8(m_uni->postGMValues()->at(1)), quint8(5));
    QCOMPARE(quint8(m_uni->postGMValues()->at(2)), quint8(5));
    QCOMPARE(quint8(m_uni->postGMValues()->at(3)), quint8(5));
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(5));
}

void Universe_Test::applyGM()
{
    m_uni->setChannelCapability(0, QLCChannel::Intensity);
    m_uni->setChannelCapability(1, QLCChannel::Pan);

    for (int i = 0; i < 256; ++i)
    {
        QCOMPARE(m_uni->applyGM(0, i), uchar(i));
        QCOMPARE(m_uni->applyGM(1, i), uchar(i));
    }

    m_gm->setValue(127);
    for (int i = 0; i < 256; ++i)
    {
        QCOMPARE(m_uni->applyGM(0, i), uchar(i/2));
        QCOMPARE(m_uni->applyGM(1, i), uchar(i));
    }

    m_gm->setChannelMode(GrandMaster::AllChannels);
    for (int i = 0; i < 256; ++i)
    {
        QCOMPARE(m_uni->applyGM(0, i), uchar(i/2));
        QCOMPARE(m_uni->applyGM(1, i), uchar(i/2));
    }

    m_gm->setValueMode(GrandMaster::Limit);
    m_gm->setChannelMode(GrandMaster::Intensity);
    for (int i = 0; i < 256; ++i)
    {
        QCOMPARE(m_uni->applyGM(0, i), uchar(i < 127 ? i : 127));
        QCOMPARE(m_uni->applyGM(1, i), uchar(i));
    }

    m_gm->setChannelMode(GrandMaster::AllChannels);
    for (int i = 0; i < 256; ++i)
    {
        QCOMPARE(m_uni->applyGM(0, i), uchar(i < 127 ? i : 127));
        QCOMPARE(m_uni->applyGM(1, i), uchar(i < 127 ? i : 127));
    }
}

void Universe_Test::write()
{
    m_uni->setChannelCapability(0, QLCChannel::Intensity);
    m_uni->setChannelCapability(4, QLCChannel::Intensity);
    m_uni->setChannelCapability(9, QLCChannel::Intensity);

    QVERIFY(m_uni->write(1000, 255) == false);
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(0));
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(0));
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(0));

    QVERIFY(m_uni->write(9, 255) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(255));
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(0));
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(0));

    QVERIFY(m_uni->write(0, 255) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(255));
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(0));
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(255));

    m_gm->setValue(127);
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(127));
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(0));
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(127));

    QVERIFY(m_uni->write(4, 200) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(127));
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(100));
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(127));
}

void Universe_Test::writeRelative()
{
    // past the end of the array
    QVERIFY(m_uni->writeRelative(1000, 255) == false);
    QCOMPARE(m_uni-> m_relativeValues[9], short(0));
    QCOMPARE(m_uni->m_relativeValues[4], short(0));
    QCOMPARE(m_uni->m_relativeValues[0], short(0));
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(0));
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(0));
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(0));

    // 127 == 0
    QVERIFY(m_uni->writeRelative(9, 127) == true);
    QCOMPARE(m_uni->m_relativeValues[9], short(0));
    QCOMPARE(m_uni->m_relativeValues[4], short(0));
    QCOMPARE(m_uni->m_relativeValues[0], short(0));
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(0));
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(0));
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(0));

    // 255 == +128
    QVERIFY(m_uni->writeRelative(9, 255) == true);
    QCOMPARE(m_uni->m_relativeValues[9], short(128)); // 0 + 128
    QCOMPARE(m_uni->m_relativeValues[4], short(0));
    QCOMPARE(m_uni->m_relativeValues[0], short(0));
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(128));
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(0));
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(0));

    // 0 == -127
    QVERIFY(m_uni->writeRelative(9, 0) == true);
    QCOMPARE(m_uni->m_relativeValues[9], short(1)); // 128 - 127
    QCOMPARE(m_uni->m_relativeValues[4], short(0));
    QCOMPARE(m_uni->m_relativeValues[0], short(0));
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(1));
    QCOMPARE(quint8(m_uni->postGMValues()->at(4)), quint8(0));
    QCOMPARE(quint8(m_uni->postGMValues()->at(0)), quint8(0));

    m_uni->reset();
    QCOMPARE(m_uni->m_relativeValues[9], short(0));

    QVERIFY(m_uni->write(9, 85) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(85));

    QVERIFY(m_uni->writeRelative(9, 117) == true);
    QCOMPARE(m_uni->m_relativeValues[9], short(-10));

    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(75));
    QVERIFY(m_uni->write(9, 65) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(55));

    m_uni->reset();

    QVERIFY(m_uni->write(9, 255) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(255));
    QVERIFY(m_uni->writeRelative(9, 255) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(255));

    m_uni->reset();

    QVERIFY(m_uni->write(9, 0) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(0));
    QVERIFY(m_uni->writeRelative(9, 0) == true);
    QCOMPARE(quint8(m_uni->postGMValues()->at(9)), quint8(0));
}

void Universe_Test::reset()
{
    int i;

    for (i = 0; i < 512; i++)
        m_uni->setChannelCapability(i, QLCChannel::Intensity);

    for (i = 0; i < 128; i++)
    {
        m_uni->write(i, 200);
        QCOMPARE(quint8(m_uni->postGMValues()->at(i)), quint8(200));
    }

    // Reset channels 10-127 (512 shouldn't cause a crash)
    m_uni->reset(10, 512);
    for (i = 0; i < 10; i++)
        QCOMPARE(quint8(m_uni->postGMValues()->at(i)), quint8(200));
    for (i = 10; i < 128; i++)
        QCOMPARE(int(m_uni->postGMValues()->at(i)), 0);

    // Reset all
    m_uni->reset();
    for (i = 0; i < 128; i++)
        QCOMPARE((int)m_uni->postGMValues()->at(i), 0);
}

void Universe_Test::setGMValueEfficiency()
{
    int i;

    for (i = 0; i < 512; i++)
        m_uni->setChannelCapability(i, QLCChannel::Intensity);

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
        QCOMPARE(int(m_uni->postGMValues()->at(i)), int(100));
}

void Universe_Test::writeEfficiency()
{
    m_gm->setValue(127);

    int i;
    for (i = 0; i < 512; i++)
        m_uni->setChannelCapability(i, QLCChannel::Intensity);

    /* This applies 50%(127) Grand Master to ALL channels in all universes.
       I'm not really sure what kinds of figures to expect here, since this
       is just one part in the overall processor load. Typically I get ~0.15ms
       on an Intel Core 2 E6550@2.33GHz, which looks plausible to me:
       DMX frame interval is 1/44Hz =~ 23ms. Applying GM to ALL channels takes
       less than 1ms so there's a full 22ms to spare after GM. */
    QBENCHMARK
    {
        for (i = 0; i < 512; i++)
            m_uni->write(i, 200);
    }

    for (i = 0; i < 512; i++)
        QCOMPARE(int(m_uni->postGMValues()->at(i)), int(100));
}

QTEST_APPLESS_MAIN(Universe_Test)
