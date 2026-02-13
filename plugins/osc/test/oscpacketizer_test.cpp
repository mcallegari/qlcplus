/*
  Q Light Controller Plus
  oscpacketizer_test.cpp

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
#include "oscpacketizer_test.h"
#include "oscpacketizer.h"
#undef private

/****************************************************************************
 * OSC Packetizer tests
 ****************************************************************************/

void OscPacketizer_Test::parseEmptyPacket()
{
    OSCPacketizer op;

    // An empty QByteArray should not crash and should return an empty list
    QByteArray empty;
    QList<QPair<QString, QByteArray> > result = op.parsePacket(empty);
    QCOMPARE(result.size(), 0);

    // A single null byte should also be handled gracefully
    QByteArray singleNull(1, '\0');
    result = op.parsePacket(singleNull);
    QCOMPARE(result.size(), 0);
}

void OscPacketizer_Test::parseTruncatedTags()
{
    OSCPacketizer op;

    // Create OSC message with a path followed by a comma and tag characters
    // but no null terminator for the tag string and no actual value data.
    // Path: "/test" padded to 8 bytes, then ",ff" with no terminator and no data
    QByteArray data;
    data.append("/test");               // 5 bytes
    data.append(QByteArray(3, '\0'));    // pad to 8 bytes
    data.append(",ff");                 // tag string without null terminator, no value data

    QList<QPair<QString, QByteArray> > result = op.parsePacket(data);
    // The parser should handle truncated tags without crashing.
    // It may or may not extract a path, but it must not crash.
    // If it does parse, the values should be empty since there is no data for the floats
    if (result.size() > 0)
    {
        QCOMPARE(result[0].first, QString("/test"));
        // values should be empty because there is not enough data for two floats
        QCOMPARE(result[0].second.size(), 0);
    }
}

void OscPacketizer_Test::parseNegativeIndexOf()
{
    OSCPacketizer op;

    // Create an OSC-like message that has no comma at all.
    // This tests the case where indexOf(0x2C) returns -1 in parseMessage.
    QByteArray noComma;
    noComma.append("/no_comma_here");
    noComma.append(QByteArray(4, '\0'));

    QList<QPair<QString, QByteArray> > result = op.parsePacket(noComma);
    // parseMessage should return false when no comma is found, so no messages
    QCOMPARE(result.size(), 0);
}

void OscPacketizer_Test::parseMsgSizeOverflow()
{
    OSCPacketizer op;

    // Create a bundle packet with a message size field set to 0xFFFFFFFF.
    // The parser should detect that the size exceeds available data and bail out.
    QByteArray bundle;
    bundle.append("#bundle");           // 7 bytes
    bundle.append('\0');                // null terminator -> 8 bytes
    bundle.append(QByteArray(8, '\0')); // timestamp -> 16 bytes total

    // Message size = 0xFFFFFFFF (4 bytes, big-endian)
    bundle.append((char)0xFF);
    bundle.append((char)0xFF);
    bundle.append((char)0xFF);
    bundle.append((char)0xFF);

    // Only a few bytes of actual message data (far less than 0xFFFFFFFF)
    bundle.append("/x");
    bundle.append(QByteArray(2, '\0'));
    bundle.append(",i");
    bundle.append(QByteArray(2, '\0'));
    bundle.append(QByteArray(4, '\0'));

    QList<QPair<QString, QByteArray> > result = op.parsePacket(bundle);
    // The parser should detect the overflow and not process the message
    QCOMPARE(result.size(), 0);
}

void OscPacketizer_Test::parseValidOSCMessage()
{
    OSCPacketizer op;

    // Build a valid OSC message: path="/value", type tag=",i", int value=128
    QByteArray data;
    data.append("/value");              // 6 bytes
    data.append(QByteArray(2, '\0'));   // pad to 8 bytes

    data.append(",i");                  // type tag with comma + 'i'
    data.append(QByteArray(2, '\0'));   // pad to 4 bytes

    // Integer value 128 in big-endian
    data.append((char)0x00);
    data.append((char)0x00);
    data.append((char)0x00);
    data.append((char)0x80);

    QList<QPair<QString, QByteArray> > result = op.parsePacket(data);
    QCOMPARE(result.size(), 1);
    QCOMPARE(result[0].first, QString("/value"));

    // The parser converts integer values < 256 directly to a char
    // value 128 should be appended as (char)128
    QCOMPARE(result[0].second.size(), 1);
    QCOMPARE((uchar)result[0].second.at(0), (uchar)128);

    // Also test round-trip: use setupOSCDmx to create a packet, then parse it
    QByteArray dmxData;
    op.setupOSCDmx(dmxData, 1, 5, 200);

    QList<QPair<QString, QByteArray> > dmxResult = op.parsePacket(dmxData);
    QCOMPARE(dmxResult.size(), 1);
    QCOMPARE(dmxResult[0].first, QString("/1/dmx/5"));
    // The float value should round-trip: 200/255.0 as float, then back to char(255.0 * fVal)
    // Due to float precision, verify it's close to 200
    QVERIFY(dmxResult[0].second.size() == 1);
    uchar parsedVal = (uchar)dmxResult[0].second.at(0);
    QVERIFY(qAbs((int)parsedVal - 200) <= 1);
}

QTEST_MAIN(OscPacketizer_Test)
