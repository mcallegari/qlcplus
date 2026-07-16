/*
  Q Light Controller Plus
  signetpacketizer.h

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

#ifndef SIGNETPACKETIZER_H
#define SIGNETPACKETIZER_H

#include <QByteArray>
#include <QHostAddress>
#include <QList>
#include <QString>

#include "sig-net-types.hpp"

class SigNetPacketizer
{
public:
    struct TLV
    {
        quint16 type;
        QByteArray value;
    };

    struct Message
    {
        QString uri;
        SigNet::SigNetOptions options;
        QByteArray payload;
        QList<TLV> tlvs;
    };

public:
    static QHostAddress levelMulticastAddress(quint16 universe);
    static QHostAddress managerPollAddress();
    static QHostAddress managerSendAddress();
    static QHostAddress nodeSendAddress();
    static QHostAddress nodeLostAddress();
    static QHostAddress nodeBeaconAddress();

    static bool parseTuid(const QString& tuid, QByteArray& out);
    static QString tuidToString(const QByteArray& tuid);

    static QByteArray buildLevelPacket(const QString& scope,
                                       quint16 signetUniverse,
                                       const QByteArray& dmxData,
                                       const QByteArray& localTuid,
                                       quint16 senderEndpoint,
                                       quint32 sessionId,
                                       quint32 seqNum,
                                       quint16 messageId,
                                       const QByteArray& senderKey);

    static QByteArray buildPollPacket(const QString& scope,
                                      const QByteArray& localTuid,
                                      quint32 sessionId,
                                      quint32 seqNum,
                                      quint16 messageId,
                                      const QByteArray& managerGlobalKey);

    static QByteArray buildTodControlPacket(const QString& scope,
                                            const QByteArray& localTuid,
                                            const QByteArray& targetTuid,
                                            quint16 targetEndpoint,
                                            quint8 todCommand,
                                            quint32 sessionId,
                                            quint32 seqNum,
                                            quint16 messageId,
                                            const QByteArray& managerLocalKey);

    static QByteArray buildRdmCommandPacket(const QString& scope,
                                            const QByteArray& localTuid,
                                            const QByteArray& targetTuid,
                                            quint16 targetEndpoint,
                                            const QByteArray& rdmPayload,
                                            quint32 sessionId,
                                            quint32 seqNum,
                                            quint16 messageId,
                                            const QByteArray& managerLocalKey);

    static bool parseMessage(const QByteArray& datagram, Message& message, QString* error = nullptr);
    static bool verifyMessage(const Message& message, const QByteArray& key);
};

#endif
