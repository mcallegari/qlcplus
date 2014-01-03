/*
  Q Light Controller - Unit test
  grandmaster_test.cpp

  Copyright (c) Jano Svitok

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

#include "grandmaster_test.h"

#define protected public
#include "grandmaster.h"
#undef protected


void GrandMaster_Test::initTestCase()
{
    m_gm = new GrandMaster(this);
}

void GrandMaster_Test::cleanupTestCase()
{
    delete m_gm;
}

void GrandMaster_Test::initial()
{
    QCOMPARE(m_gm->valueMode(), GrandMaster::Reduce);
    QCOMPARE(m_gm->channelMode(), GrandMaster::Intensity);
    QCOMPARE(m_gm->value(), uchar(255));
    QCOMPARE(m_gm->fraction(), 1.0);
}

void GrandMaster_Test::channelMode()
{
    m_gm->setChannelMode(GrandMaster::AllChannels);
    QCOMPARE(m_gm->channelMode(), GrandMaster::AllChannels);

    m_gm->setChannelMode(GrandMaster::Intensity);
    QCOMPARE(m_gm->channelMode(), GrandMaster::Intensity);

    QCOMPARE(GrandMaster::stringToChannelMode("All"), GrandMaster::AllChannels);
    QCOMPARE(GrandMaster::stringToChannelMode("Intensity"), GrandMaster::Intensity);
    QCOMPARE(GrandMaster::stringToChannelMode("foobar"), GrandMaster::Intensity);

    QCOMPARE(GrandMaster::channelModeToString(GrandMaster::AllChannels), QString("All"));
    QCOMPARE(GrandMaster::channelModeToString(GrandMaster::Intensity), QString("Intensity"));
    QCOMPARE(GrandMaster::channelModeToString(GrandMaster::ChannelMode(42)), QString("Intensity"));
}

void GrandMaster_Test::valueMode()
{
    m_gm->setValueMode(GrandMaster::Limit);
    QCOMPARE(m_gm->valueMode(), GrandMaster::Limit);

    m_gm->setValueMode(GrandMaster::Reduce);
    QCOMPARE(m_gm->valueMode(), GrandMaster::Reduce);

    QCOMPARE(GrandMaster::stringToValueMode("Limit"), GrandMaster::Limit);
    QCOMPARE(GrandMaster::stringToValueMode("Reduce"), GrandMaster::Reduce);
    QCOMPARE(GrandMaster::stringToValueMode("xyzzy"), GrandMaster::Reduce);

    QCOMPARE(GrandMaster::valueModeToString(GrandMaster::Limit), QString("Limit"));
    QCOMPARE(GrandMaster::valueModeToString(GrandMaster::Reduce), QString("Reduce"));
    QCOMPARE(GrandMaster::valueModeToString(GrandMaster::ValueMode(31337)), QString("Reduce"));
}

void GrandMaster_Test::value()
{
    for (int i = 0; i < UCHAR_MAX; i++)
    {
        m_gm->setValue(uchar(i));
        QCOMPARE(m_gm->value(), uchar(i));
        QCOMPARE(m_gm->fraction(), (double(i) / double(UCHAR_MAX)));
    }

    m_gm->setValue(0);
    QCOMPARE(m_gm->value(), uchar(0));
    QCOMPARE(m_gm->fraction(), double(0));

    m_gm->setValue(255);
    QCOMPARE(m_gm->value(), uchar(255));
    QCOMPARE(m_gm->fraction(), double(1));
}

QTEST_APPLESS_MAIN(GrandMaster_Test)
