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
#include "rdmprotocol.h"
#undef private

static void setAckResponse(RDMProtocol &rdm, QByteArray &data)
{
    Q_ASSERT(data.isEmpty() == false && data.at(0) == char(RDM_SC_SUB_MESSAGE));
    // Offset 15 is the response-type byte in a no-start-code RDM frame.
    data[15] = char(RESPONSE_TYPE_ACK);

    quint16 checksum = rdm.calculateChecksum(false, data, data.length() - 2);
    data[data.length() - 2] = char(checksum >> 8);
    data[data.length() - 1] = char(checksum & 0x00ff);
}

static void setParameterData(RDMProtocol &rdm, QByteArray &data,
                             const QByteArray &payload)
{
    const int pdlOffset = 22;
    const int payloadOffset = pdlOffset + 1;

    data.insert(payloadOffset, payload);
    data[pdlOffset] = char(payload.length());
    data[1] = char(data.length() - 1);

    setAckResponse(rdm, data);
}

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

void ArtNet_Test::rdmParsePacketRejectsShortHeader()
{
    RDMProtocol rdm;
    QVariantMap values;

    QByteArray data;
    data.append(char(RDM_START_CODE));

    QVERIFY(rdm.parsePacket(data, values) == false);

    data = QByteArray(22, 0);
    data[0] = char(RDM_SC_SUB_MESSAGE);
    QVERIFY(rdm.parsePacket(data, values) == false);
}

void ArtNet_Test::rdmParsePacketRejectsOversizedMessageLength()
{
    RDMProtocol builder;
    RDMProtocol parser;
    QVariantMap values;
    QVariantList params;
    QByteArray data;

    builder.setEstaID(0x1111);
    builder.setDeviceId(0x22222222);
    params << RDMProtocol::broadcastAddress() << PID_DEVICE_INFO;
    QVERIFY(builder.packetizeCommand(GET_COMMAND, params, false, data));

    setAckResponse(builder, data);
    data[1] = char(0xff);
    QVERIFY(parser.parsePacket(data, values) == false);
}

void ArtNet_Test::rdmParsePacketRejectsTruncatedPdl()
{
    RDMProtocol builder;
    RDMProtocol parser;
    QVariantMap values;
    QVariantList params;
    QByteArray data;

    builder.setEstaID(0x1111);
    builder.setDeviceId(0x22222222);
    params << RDMProtocol::broadcastAddress() << PID_DEVICE_INFO;
    QVERIFY(builder.packetizeCommand(GET_COMMAND, params, false, data));

    setAckResponse(builder, data);
    data[22] = char(21);
    QVERIFY(parser.parsePacket(data, values) == false);
}

void ArtNet_Test::rdmParsePacketRejectsShortPidPayload()
{
    RDMProtocol builder;
    RDMProtocol parser;
    QVariantMap values;
    QVariantList params;
    QByteArray data;

    builder.setEstaID(0x1111);
    builder.setDeviceId(0x22222222);
    params << RDMProtocol::broadcastAddress() << PID_DEVICE_INFO;
    QVERIFY(builder.packetizeCommand(GET_COMMAND, params, false, data));

    setAckResponse(builder, data);
    QVERIFY(parser.parsePacket(data, values) == false);
}

void ArtNet_Test::rdmParsePacketAcceptsValidPacket()
{
    RDMProtocol builder;
    RDMProtocol parser;
    QVariantMap values;
    QVariantList params;
    QByteArray data;

    builder.setEstaID(0x1111);
    builder.setDeviceId(0x22222222);
    params << RDMProtocol::broadcastAddress() << PID_SUPPORTED_PARAMETERS
           << 4 << 0x00600070;
    QVERIFY(builder.packetizeCommand(GET_COMMAND, params, false, data));
    setAckResponse(builder, data);

    QVERIFY(parser.parsePacket(data, values) == true);
    QCOMPARE(values.value("PID").toUInt(), uint(PID_SUPPORTED_PARAMETERS));
    const QVector<quint16> pidList =
        values.value("PID_LIST").value<QVector<quint16>>();
    QCOMPARE(pidList.size(), 2);
    QCOMPARE(pidList.at(0), quint16(PID_DEVICE_INFO));
    QCOMPARE(pidList.at(1), quint16(PID_PRODUCT_DETAIL_ID_LIST));
}

void ArtNet_Test::rdmParsePacketAcceptsValidDeviceInfoPacket()
{
    RDMProtocol builder;
    RDMProtocol parser;
    QVariantMap values;
    QVariantList params;
    QByteArray data;
    QByteArray payload(19, 0);

    builder.setEstaID(0x1111);
    builder.setDeviceId(0x22222222);
    params << RDMProtocol::broadcastAddress() << PID_DEVICE_INFO;
    QVERIFY(builder.packetizeCommand(GET_COMMAND, params, false, data));

    payload[18] = char(3);
    setParameterData(builder, data, payload);

    QVERIFY(parser.parsePacket(data, values) == true);
    QCOMPARE(values.value("PID").toUInt(), uint(PID_DEVICE_INFO));
    QCOMPARE(values.value("Number of sensors").toUInt(), uint(3));
}

QTEST_MAIN(ArtNet_Test)
