/*
  Q Light Controller Plus
  e131packetizer_test.cpp

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

#include <QTest>

#define private public
#include "e131packetizer_test.h"
#include "e131packetizer.h"
#undef private

/****************************************************************************
 * E1.31 Packetizer tests
 ****************************************************************************/

void E131Packetizer_Test::fillDMXdataMinSize()
{
    E131Packetizer ep("00:11:22:33:44:55");

    // Create a QByteArray of exactly 125 bytes -- below the 126-byte minimum
    // required by fillDMXdata
    QByteArray tooSmall(125, '\0');
    QByteArray dmx;
    quint32 universe = 0;

    bool result = ep.fillDMXdata(tooSmall, dmx, universe);
    QCOMPARE(result, false);

    // Also verify that a null QByteArray returns false
    QByteArray nullData;
    result = ep.fillDMXdata(nullData, dmx, universe);
    QCOMPARE(result, false);
}

void E131Packetizer_Test::fillDMXdataZeroLength()
{
    E131Packetizer ep("00:11:22:33:44:55");

    // Create a valid E1.31 packet using setupE131Dmx, then manually set
    // the DMX length field (bytes 123-124) to 0
    QByteArray dmxValues(10, (char)0x80);
    QByteArray packet;
    ep.setupE131Dmx(packet, 1, E131_PRIORITY_DEFAULT, dmxValues);

    // Verify the packet is large enough
    QVERIFY(packet.size() >= 126);

    // Set property value count to 0 (bytes 123-124)
    packet[123] = (char)0x00;
    packet[124] = (char)0x00;

    QByteArray dmx;
    quint32 universe = 0;

    // fillDMXdata checks for length==0 and returns true without extracting data
    bool result = ep.fillDMXdata(packet, dmx, universe);
    QCOMPARE(result, true);
    QCOMPARE(dmx.size(), 0);
}

void E131Packetizer_Test::fillDMXdataValid()
{
    E131Packetizer ep("00:11:22:33:44:55");

    // Create a properly formed E1.31 packet with known DMX data
    QByteArray dmxValues;
    for (int i = 0; i < 512; i++)
        dmxValues.append((char)(i & 0xFF));

    QByteArray packet;
    int testUniverse = 3;
    ep.setupE131Dmx(packet, testUniverse, E131_PRIORITY_DEFAULT, dmxValues);

    // Verify the packet was created with proper size
    // Header is 125 bytes + DMX start code (1 byte at index 125) + 512 DMX bytes
    QVERIFY(packet.size() >= 126 + 512);

    // Now parse it back
    QByteArray extractedDmx;
    quint32 extractedUniverse = 0;

    // First verify checkPacket
    bool valid = ep.checkPacket(packet);
    QCOMPARE(valid, true);

    // Then extract DMX data
    bool result = ep.fillDMXdata(packet, extractedDmx, extractedUniverse);
    QCOMPARE(result, true);
    QCOMPARE(extractedUniverse, (quint32)testUniverse);
    QCOMPARE(extractedDmx.size(), 512);

    // Verify the extracted DMX data matches what we sent
    for (int i = 0; i < 512; i++)
        QCOMPARE((uchar)extractedDmx.at(i), (uchar)(i & 0xFF));
}

void E131Packetizer_Test::checkPacketBoundary()
{
    E131Packetizer ep("00:11:22:33:44:55");

    // A packet of exactly 125 bytes is the boundary -- checkPacket requires >= 125
    // but the ACN identifier must also match at the right positions

    // First test: 124 bytes should fail
    QByteArray tooShort(124, '\0');
    QCOMPARE(ep.checkPacket(tooShort), false);

    // Second test: 125 bytes of zeros should fail because ACN identifier doesn't match
    QByteArray justEnough(125, '\0');
    QCOMPARE(ep.checkPacket(justEnough), false);

    // Third test: build a valid packet and verify checkPacket passes
    QByteArray dmxValues(1, '\0');
    QByteArray validPacket;
    ep.setupE131Dmx(validPacket, 1, E131_PRIORITY_DEFAULT, dmxValues);
    QCOMPARE(ep.checkPacket(validPacket), true);

    // Fourth test: corrupt the ACN identifier and verify it fails
    QByteArray corruptedPacket = validPacket;
    corruptedPacket[4] = 'X'; // change 'A' to 'X' in "ASC-E1.17"
    QCOMPARE(ep.checkPacket(corruptedPacket), false);
}

QTEST_MAIN(E131Packetizer_Test)
