/*
  Q Light Controller Plus
  signetpacketizer.cpp

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

#include "signetpacketizer.h"

#include <QStringList>

#include "sig-net-coap.hpp"
#include "sig-net-constants.hpp"
#include "sig-net-crypto.hpp"
#include "sig-net-parse.hpp"
#include "sig-net-security.hpp"
#include "sig-net-tlv.hpp"

namespace
{
const quint16 KUriPathOption = SigNet::COAP_OPTION_URI_PATH;

bool encodeUriPath(SigNet::PacketBuffer& buffer, const QStringList& segments)
{
    quint16 prevOption = 0;
    for (const QString& segment : segments)
    {
        const QByteArray bytes = segment.toUtf8();
        if (SigNet::CoAP::EncodeCoAPOption(buffer,
                                           KUriPathOption,
                                           prevOption,
                                           reinterpret_cast<const uint8_t*>(bytes.constData()),
                                           static_cast<quint16>(bytes.size())) != SigNet::SIGNET_SUCCESS)
        {
            return false;
        }
        prevOption = KUriPathOption;
    }

    return true;
}

QByteArray finalizePacket(SigNet::PacketBuffer& packet,
                          const QString& uri,
                          SigNet::SigNetOptions& options,
                          const SigNet::PacketBuffer& payload,
                          const QByteArray& signingKey)
{
    const uint8_t* payloadPtr = payload.GetSize() > 0 ? payload.GetBuffer() : reinterpret_cast<const uint8_t*>("");
    if (SigNet::Security::CalculateAndEncodeHMAC(packet,
                                                 uri.toUtf8().constData(),
                                                 options,
                                                 payloadPtr,
                                                 payload.GetSize(),
                                                 reinterpret_cast<const uint8_t*>(signingKey.constData()),
                                                 SigNet::SIGNET_OPTION_SEQ_NUM) != SigNet::SIGNET_SUCCESS)
    {
        return QByteArray();
    }

    if (payload.GetSize() > 0)
    {
        if (packet.WriteByte(SigNet::COAP_PAYLOAD_MARKER) != SigNet::SIGNET_SUCCESS)
            return QByteArray();

        if (packet.WriteBytes(payload.GetBuffer(), payload.GetSize()) != SigNet::SIGNET_SUCCESS)
            return QByteArray();
    }

    return QByteArray(reinterpret_cast<const char*>(packet.GetBuffer()), packet.GetSize());
}

QByteArray buildSignedPacket(const QString& scope,
                             const QStringList& resourceSegments,
                             const SigNet::PacketBuffer& payload,
                             const QByteArray& localTuid,
                             quint16 senderEndpoint,
                             quint32 sessionId,
                             quint32 seqNum,
                             quint16 messageId,
                             const QByteArray& signingKey)
{
    if (localTuid.size() != static_cast<int>(SigNet::TUID_LENGTH) || signingKey.size() != static_cast<int>(SigNet::DERIVED_KEY_LENGTH))
        return QByteArray();

    SigNet::PacketBuffer packet;
    if (SigNet::CoAP::BuildCoAPHeader(packet, messageId) != SigNet::SIGNET_SUCCESS)
        return QByteArray();

    QStringList uriSegments;
    uriSegments << QString(SigNet::SIGNET_URI_PREFIX)
                << QString(SigNet::SIGNET_URI_VERSION)
                << scope;
    uriSegments.append(resourceSegments);
    if (!encodeUriPath(packet, uriSegments))
        return QByteArray();

    SigNet::SigNetOptions options;
    options.security_mode = SigNet::SECURITY_MODE_HMAC_SHA256;
    options.mfg_code = 0x0000;
    options.session_id = sessionId;
    options.seq_num = seqNum;
    if (SigNet::Security::BuildSenderID(reinterpret_cast<const uint8_t*>(localTuid.constData()),
                                        senderEndpoint,
                                        options.sender_id) != SigNet::SIGNET_SUCCESS)
    {
        return QByteArray();
    }

    if (SigNet::Security::BuildSigNetOptionsWithoutHMAC(packet,
                                                        options,
                                                        KUriPathOption) != SigNet::SIGNET_SUCCESS)
    {
        return QByteArray();
    }

    const QString uri = QString("/%1").arg(uriSegments.join('/'));
    return finalizePacket(packet, uri, options, payload, signingKey);
}
}

QHostAddress SigNetPacketizer::levelMulticastAddress(quint16 universe)
{
    const quint16 index = ((universe - 1) % 109) + 1;
    return QHostAddress(QString("239.254.0.%1").arg(index));
}

QHostAddress SigNetPacketizer::managerPollAddress()
{
    return QHostAddress(QString::fromLatin1(SigNet::MULTICAST_MANAGER_POLL_IP));
}

QHostAddress SigNetPacketizer::managerSendAddress()
{
    return QHostAddress(QString::fromLatin1(SigNet::MULTICAST_MANAGER_SEND_IP));
}

QHostAddress SigNetPacketizer::nodeSendAddress()
{
    return QHostAddress(QString::fromLatin1(SigNet::MULTICAST_NODE_SEND_IP));
}

QHostAddress SigNetPacketizer::nodeLostAddress()
{
    return QHostAddress(QString::fromLatin1(SigNet::MULTICAST_NODE_LOST_IP));
}

QHostAddress SigNetPacketizer::nodeBeaconAddress()
{
    return QHostAddress(QString::fromLatin1(SigNet::MULTICAST_NODE_BEACON_IP));
}

bool SigNetPacketizer::parseTuid(const QString& tuid, QByteArray& out)
{
    out.resize(SigNet::TUID_LENGTH);
    if (SigNet::Crypto::TUID_FromHexString(tuid.toUtf8().constData(),
                                           reinterpret_cast<uint8_t*>(out.data())) != SigNet::SIGNET_SUCCESS)
    {
        out.clear();
        return false;
    }

    return true;
}

QString SigNetPacketizer::tuidToString(const QByteArray& tuid)
{
    if (tuid.size() != static_cast<int>(SigNet::TUID_LENGTH))
        return QString();

    char value[SigNet::TUID_HEX_LENGTH + 1];
    value[SigNet::TUID_HEX_LENGTH] = '\0';
    SigNet::Crypto::TUID_ToHexString(reinterpret_cast<const uint8_t*>(tuid.constData()), value, sizeof(value));
    return QString::fromLatin1(value);
}

QByteArray SigNetPacketizer::buildLevelPacket(const QString& scope,
                                              quint16 signetUniverse,
                                              const QByteArray& dmxData,
                                              const QByteArray& localTuid,
                                              quint16 senderEndpoint,
                                              quint32 sessionId,
                                              quint32 seqNum,
                                              quint16 messageId,
                                              const QByteArray& senderKey)
{
    SigNet::PacketBuffer payload;
    const uint16_t slotCount = qBound<quint16>(1, static_cast<quint16>(dmxData.size()), SigNet::MAX_DMX_SLOTS);
    if (SigNet::TLV::BuildDMXLevelPayload(payload,
                                          reinterpret_cast<const uint8_t*>(dmxData.constData()),
                                          slotCount) != SigNet::SIGNET_SUCCESS)
    {
        return QByteArray();
    }

    return buildSignedPacket(scope,
                             QStringList() << QString(SigNet::SIGNET_URI_LEVEL) << QString::number(signetUniverse),
                             payload,
                             localTuid,
                             senderEndpoint,
                             sessionId,
                             seqNum,
                             messageId,
                             senderKey);
}

QByteArray SigNetPacketizer::buildPollPacket(const QString& scope,
                                             const QByteArray& localTuid,
                                             quint32 sessionId,
                                             quint32 seqNum,
                                             quint16 messageId,
                                             const QByteArray& managerGlobalKey)
{
    SigNet::PacketBuffer payload;
    uint8_t tuidLo[SigNet::TUID_LENGTH] = { 0, 0, 0, 0, 0, 0 };
    uint8_t tuidHi[SigNet::TUID_LENGTH] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    if (SigNet::TLV::BuildPollPayload(payload,
                                      reinterpret_cast<const uint8_t*>(localTuid.constData()),
                                      0x7FF8,
                                      0x0001,
                                      tuidLo,
                                      tuidHi,
                                      0xFFFF,
                                      SigNet::QUERY_HEARTBEAT) != SigNet::SIGNET_SUCCESS)
    {
        return QByteArray();
    }

    return buildSignedPacket(scope,
                             QStringList() << QString(SigNet::SIGNET_URI_POLL),
                             payload,
                             localTuid,
                             0,
                             sessionId,
                             seqNum,
                             messageId,
                             managerGlobalKey);
}

QByteArray SigNetPacketizer::buildTodControlPacket(const QString& scope,
                                                   const QByteArray& localTuid,
                                                   const QByteArray& targetTuid,
                                                   quint16 targetEndpoint,
                                                   quint8 todCommand,
                                                   quint32 sessionId,
                                                   quint32 seqNum,
                                                   quint16 messageId,
                                                   const QByteArray& managerLocalKey)
{
    SigNet::PacketBuffer payload;
    const uint8_t value = todCommand;
    const SigNet::TLVBlock tlv(SigNet::TID_RDM_TOD_CONTROL, 1, &value);
    if (SigNet::TLV::EncodeTLV(payload, tlv) != SigNet::SIGNET_SUCCESS)
        return QByteArray();

    return buildSignedPacket(scope,
                             QStringList() << QStringLiteral("manager")
                                           << tuidToString(targetTuid)
                                           << QString::number(targetEndpoint),
                             payload,
                             localTuid,
                             0,
                             sessionId,
                             seqNum,
                             messageId,
                             managerLocalKey);
}

QByteArray SigNetPacketizer::buildRdmCommandPacket(const QString& scope,
                                                   const QByteArray& localTuid,
                                                   const QByteArray& targetTuid,
                                                   quint16 targetEndpoint,
                                                   const QByteArray& rdmPayload,
                                                   quint32 sessionId,
                                                   quint32 seqNum,
                                                   quint16 messageId,
                                                   const QByteArray& managerLocalKey)
{
    SigNet::PacketBuffer payload;
    const SigNet::TLVBlock tlv(SigNet::TID_RDM_COMMAND,
                               static_cast<quint16>(rdmPayload.size()),
                               reinterpret_cast<const uint8_t*>(rdmPayload.constData()));
    if (SigNet::TLV::EncodeTLV(payload, tlv) != SigNet::SIGNET_SUCCESS)
        return QByteArray();

    return buildSignedPacket(scope,
                             QStringList() << QStringLiteral("manager")
                                           << tuidToString(targetTuid)
                                           << QString::number(targetEndpoint),
                             payload,
                             localTuid,
                             0,
                             sessionId,
                             seqNum,
                             messageId,
                             managerLocalKey);
}

bool SigNetPacketizer::parseMessage(const QByteArray& datagram, Message& message, QString* error)
{
    message = Message();

    SigNet::Parse::PacketReader reader(reinterpret_cast<const uint8_t*>(datagram.constData()), datagram.size());
    SigNet::CoAPHeader header;
    if (SigNet::Parse::ParseCoAPHeader(reader, header) != SigNet::SIGNET_SUCCESS)
    {
        if (error)
            *error = QStringLiteral("Invalid CoAP header");
        return false;
    }

    if (header.GetVersion() != SigNet::COAP_VERSION || header.code != SigNet::COAP_CODE_POST)
    {
        if (error)
            *error = QStringLiteral("Unsupported CoAP packet");
        return false;
    }

    if (SigNet::Parse::SkipToken(reader, header.GetTokenLength()) != SigNet::SIGNET_SUCCESS)
    {
        if (error)
            *error = QStringLiteral("Invalid CoAP token");
        return false;
    }

    char uriBuffer[256];
    uint16_t uriLength = 0;
    if (SigNet::Parse::ExtractURIString(reader,
                                        uriBuffer,
                                        sizeof(uriBuffer),
                                        uriLength) != SigNet::SIGNET_SUCCESS)
    {
        if (error)
            *error = QStringLiteral("Invalid URI");
        return false;
    }
    Q_UNUSED(uriLength)
    message.uri = QString::fromLatin1(uriBuffer);

    if (SigNet::Parse::ParseSigNetOptions(reader,
                                          message.options,
                                          KUriPathOption) != SigNet::SIGNET_SUCCESS)
    {
        if (error)
            *error = QStringLiteral("Invalid Sig-Net options");
        return false;
    }

    if (reader.GetRemaining() > 0)
    {
        uint8_t marker = 0;
        if (reader.PeekByte(marker) && marker == SigNet::COAP_PAYLOAD_MARKER)
        {
            reader.Skip(1);
            message.payload = QByteArray(reinterpret_cast<const char*>(reader.GetCurrentPtr()), reader.GetRemaining());
        }
    }

    if (!message.payload.isEmpty())
    {
        SigNet::Parse::PacketReader payloadReader(reinterpret_cast<const uint8_t*>(message.payload.constData()),
                                                  message.payload.size());
        while (payloadReader.GetRemaining() > 0)
        {
            SigNet::TLVBlock tlv;
            if (SigNet::Parse::ParseTLVBlock(payloadReader, tlv) != SigNet::SIGNET_SUCCESS)
            {
                if (error)
                    *error = QStringLiteral("Malformed TLV payload");
                return false;
            }

            TLV outTlv;
            outTlv.type = tlv.type_id;
            outTlv.value = QByteArray(reinterpret_cast<const char*>(tlv.value), tlv.length);
            message.tlvs.append(outTlv);
        }
    }

    return true;
}

bool SigNetPacketizer::verifyMessage(const Message& message, const QByteArray& key)
{
    if (key.size() != static_cast<int>(SigNet::DERIVED_KEY_LENGTH))
        return false;

    const uint8_t dummy = 0;
    const uint8_t* payloadPtr = message.payload.isEmpty()
        ? &dummy
        : reinterpret_cast<const uint8_t*>(message.payload.constData());

    return SigNet::Parse::VerifyPacketHMAC(message.uri.toUtf8().constData(),
                                           message.options,
                                           payloadPtr,
                                           static_cast<quint16>(message.payload.size()),
                                           reinterpret_cast<const uint8_t*>(key.constData())) == SigNet::SIGNET_SUCCESS;
}
