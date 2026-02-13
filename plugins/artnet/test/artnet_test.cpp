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

void ArtNet_Test::fillArtPollReplySmallPacket()
{
    ArtNetPacketizer ap;

    // A valid ArtPollReply must be at least 207 bytes.
    // Passing only 100 bytes should return false.
    QByteArray smallPacket(100, '\0');
    ArtNetNodeInfo info;
    QCOMPARE(ap.fillArtPollReplyInfo(smallPacket, info), false);

    // Also test with a null QByteArray
    QByteArray nullData;
    QCOMPARE(ap.fillArtPollReplyInfo(nullData, info), false);

    // Test with exactly 206 bytes (one less than minimum)
    QByteArray borderPacket(206, '\0');
    QCOMPARE(ap.fillArtPollReplyInfo(borderPacket, info), false);

    // Test with exactly 207 bytes -- should return true (even if data is zeros)
    QByteArray justEnough(207, '\0');
    QCOMPARE(ap.fillArtPollReplyInfo(justEnough, info), true);
}

void ArtNet_Test::processTODdataFakeCount()
{
    ArtNetPacketizer ap;

    // Build a minimal TOD data packet (28 bytes header) with uidCount claiming
    // 1000 UIDs, but the packet only has 30 total bytes.
    // The parser should detect that 28 + 1000*6 > 30 and return false.
    QByteArray fakePacket(30, '\0');

    // Set byte 27 (uidCount) to a large value
    // The uidCount is quint8, so max is 255, but 255*6 = 1530 >> 30 bytes
    fakePacket[27] = (char)0xFF; // uidCount = 255

    quint32 universe = 0;
    QVariantMap values;
    QCOMPARE(ap.processTODdata(fakePacket, universe, values), false);

    // Also test with uidCount=1 but only 30 bytes total (needs 28 + 6 = 34)
    QByteArray shortPacket(30, '\0');
    shortPacket[27] = (char)0x01; // uidCount = 1
    QCOMPARE(ap.processTODdata(shortPacket, universe, values), false);

    // Test with null data
    QByteArray nullData;
    QCOMPARE(ap.processTODdata(nullData, universe, values), false);

    // Test with too-short data (less than 28 bytes)
    QByteArray tooShort(20, '\0');
    QCOMPARE(ap.processTODdata(tooShort, universe, values), false);
}

void ArtNet_Test::fillDMXdataSmallPacket()
{
    ArtNetPacketizer ap;

    // fillDMXdata requires at least 18 bytes.
    // Passing only 10 bytes should return false.
    QByteArray smallPacket(10, '\0');
    QByteArray dmx;
    quint32 universe = 0;
    QCOMPARE(ap.fillDMXdata(smallPacket, dmx, universe), false);

    // Also test with a null QByteArray
    QByteArray nullData;
    QCOMPARE(ap.fillDMXdata(nullData, dmx, universe), false);

    // Test with exactly 17 bytes (one less than minimum)
    QByteArray borderPacket(17, '\0');
    QCOMPARE(ap.fillDMXdata(borderPacket, dmx, universe), false);

    // Test with exactly 18 bytes -- should return true (with zero-length DMX)
    QByteArray justEnough(18, '\0');
    QCOMPARE(ap.fillDMXdata(justEnough, dmx, universe), true);
}

QTEST_MAIN(ArtNet_Test)
