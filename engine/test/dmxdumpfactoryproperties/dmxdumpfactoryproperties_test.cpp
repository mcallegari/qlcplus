/*
  Q Light Controller Plus - Unit test
  dmxdumpfactoryproperties_test.cpp

  Copyright (c) Massimo Callegari

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
#include <QByteArray>

#define protected public
#define private public
#include "dmxdumpfactoryproperties_test.h"
#include "dmxdumpfactoryproperties.h"
#undef private
#undef protected

void DmxDumpFactoryProperties_Test::initial()
{
    DmxDumpFactoryProperties props(2);
    QCOMPARE(props.dumpChannelsMode(), true);
    QCOMPARE(props.nonZeroValuesMode(), false);
    QCOMPARE(props.channelsMask().size(), 1024);
    QCOMPARE(props.selectedTarget(), DmxDumpFactoryProperties::Chaser);
}

void DmxDumpFactoryProperties_Test::maskAndChasers()
{
    DmxDumpFactoryProperties props(1);
    props.setDumpChannelsMode(false);
    props.setNonZeroValuesMode(true);
    QByteArray mask(512, 0);
    for (int i = 0; i < 10; ++i)
        mask[i] = 1;
    props.setChannelsMask(mask);
    props.addChaserID(5);
    props.addChaserID(10);
    props.removeChaserID(10);
    props.setSelectedTarget(DmxDumpFactoryProperties::VCSlider);

    QCOMPARE(props.dumpChannelsMode(), false);
    QCOMPARE(props.nonZeroValuesMode(), true);
    QCOMPARE(props.channelsMask().left(10), QByteArray(10, 1));
    QCOMPARE(props.isChaserSelected(5), true);
    QCOMPARE(props.isChaserSelected(10), false);
    QCOMPARE(props.selectedTarget(), DmxDumpFactoryProperties::VCSlider);
}

QTEST_APPLESS_MAIN(DmxDumpFactoryProperties_Test)
