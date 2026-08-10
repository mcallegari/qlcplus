/*
  Q Light Controller Plus
  e131packetizer_test.cpp

  Copyright (c) Q Light Controller Plus

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

#include <QTest>

#include "e131packetizer_test.h"
#include "e131packetizer.h"

void E131Packetizer_Test::checkPacketRejectsTruncatedPacket()
{
    E131Packetizer ep("00:11:22:33:44:55");
    QByteArray data;

    ep.setupE131Dmx(data, 0, E131_PRIORITY_DEFAULT, QByteArray());
    QVERIFY(ep.checkPacket(data) == true);

    data.truncate(125);
    QVERIFY(ep.checkPacket(data) == false);
}

void E131Packetizer_Test::fillDMXdataRejectsZeroPropertyCount()
{
    E131Packetizer ep("00:11:22:33:44:55");
    QByteArray data;
    QByteArray dmx;
    quint32 universe = 0;

    ep.setupE131Dmx(data, 0, E131_PRIORITY_DEFAULT, QByteArray());
    data[123] = char(0x00);
    data[124] = char(0x00);

    QVERIFY(ep.fillDMXdata(data, dmx, universe) == false);
}

void E131Packetizer_Test::fillDMXdataRejectsStartCodeOnly()
{
    E131Packetizer ep("00:11:22:33:44:55");
    QByteArray data;
    QByteArray dmx;
    quint32 universe = 0;

    ep.setupE131Dmx(data, 0, E131_PRIORITY_DEFAULT, QByteArray());

    QVERIFY(ep.checkPacket(data) == true);
    QVERIFY(ep.fillDMXdata(data, dmx, universe) == false);
}

void E131Packetizer_Test::fillDMXdataRejectsOversizedPropertyCount()
{
    E131Packetizer ep("00:11:22:33:44:55");
    QByteArray data;
    QByteArray dmx;
    quint32 universe = 0;

    ep.setupE131Dmx(data, 0, E131_PRIORITY_DEFAULT, QByteArray(512, 42));
    data[123] = char(0x02);
    data[124] = char(0x02);

    QVERIFY(ep.fillDMXdata(data, dmx, universe) == false);
}

void E131Packetizer_Test::fillDMXdataRejectsTruncatedPayload()
{
    E131Packetizer ep("00:11:22:33:44:55");
    QByteArray data;
    QByteArray dmx;
    quint32 universe = 0;

    ep.setupE131Dmx(data, 0, E131_PRIORITY_DEFAULT, QByteArray(2, 42));
    data.chop(1);

    QVERIFY(ep.checkPacket(data) == true);
    QVERIFY(ep.fillDMXdata(data, dmx, universe) == false);
}

void E131Packetizer_Test::fillDMXdataAcceptsValidPayload()
{
    E131Packetizer ep("00:11:22:33:44:55");
    QByteArray data;
    QByteArray dmx;
    quint32 universe = 0;

    ep.setupE131Dmx(data, 1, E131_PRIORITY_DEFAULT, QByteArray(2, 42));

    QVERIFY(ep.checkPacket(data) == true);
    QVERIFY(ep.fillDMXdata(data, dmx, universe) == true);
    QCOMPARE(universe, quint32(1));
    QCOMPARE(dmx, QByteArray(2, 42));
}

QTEST_MAIN(E131Packetizer_Test)
