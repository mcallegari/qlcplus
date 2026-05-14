/*
  Q Light Controller Plus
  artnet_test.cpp

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

#include <QTest>

#define private public
#include "artnet_test.h"
#include "artnetpacketizer.h"
#undef private

/****************************************************************************
 * ArtNet tests
 ****************************************************************************/

void ArtNet_Test::setupArtNetDmx()
{
    ArtNetPacketizer ap;

    QByteArray data;
    const QByteArray empty;
    const QByteArray fifty(50, 10);
    const QByteArray fiftyone(51, 10);
    const QByteArray full(512, 20);

    // empty data
    ap.setupArtNetDmx(data, 0, empty);

    QCOMPARE(data.size(), 20);
    QCOMPARE(data.data(), "Art-Net");

    // full data
    ap.setupArtNetDmx(data, 0, full);

    QCOMPARE(data.size(), 18 + 512);
    QCOMPARE(data.data(), "Art-Net");

    // partial data
    ap.setupArtNetDmx(data, 0, fifty);

    QCOMPARE(data.size(), 18 + 50);
    QCOMPARE(data.data(), "Art-Net");

    ap.setupArtNetDmx(data, 0, fiftyone);

    QCOMPARE(data.size(), 18 + 52);
    QCOMPARE(data.data(), "Art-Net");
}

void ArtNet_Test::fillArtPollReplyInfoRejectsShortPacket()
{
    ArtNetPacketizer ap;
    ArtNetNodeInfo info;

    QByteArray data(186, 0);

    QVERIFY(ap.fillArtPollReplyInfo(data, info) == false);

    ap.setupArtNetPollReply(data, QHostAddress("127.0.0.1"),
                            "00:11:22:33:44:55", 1, true);
    QVERIFY(ap.fillArtPollReplyInfo(data, info) == true);
}

void ArtNet_Test::fillDMXdataRejectsShortPacket()
{
    ArtNetPacketizer ap;
    QByteArray dmx;
    quint32 universe = 0;

    QByteArray data(17, 0);
    QVERIFY(ap.fillDMXdata(data, dmx, universe) == false);

    data.resize(18);
    data[16] = char(0x00);
    data[17] = char(0x04);
    QVERIFY(ap.fillDMXdata(data, dmx, universe) == false);
}

void ArtNet_Test::processTODdataRejectsShortUidList()
{
    ArtNetPacketizer ap;
    QVariantMap values;
    quint32 universe = 0;

    QByteArray data(28, 0);
    data[27] = char(0x01);
    QVERIFY(ap.processTODdata(data, universe, values) == false);

    data.resize(34);
    QVERIFY(ap.processTODdata(data, universe, values) == true);
    QCOMPARE(values.value("DISCOVERY_COUNT").toInt(), 1);
}

QTEST_MAIN(ArtNet_Test)
