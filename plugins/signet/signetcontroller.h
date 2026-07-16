/*
  Q Light Controller Plus
  signetcontroller.h

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

#ifndef SIGNETCONTROLLER_H
#define SIGNETCONTROLLER_H

#include <QByteArray>
#include <QDateTime>
#include <QHash>
#include <QHostAddress>
#include <QMap>
#include <QMutex>
#include <QNetworkAddressEntry>
#include <QNetworkInterface>
#include <QSet>
#include <QSharedPointer>
#include <QTimer>
#include <QUdpSocket>
#include <QVariantList>

#include "signetpacketizer.h"

class SigNetPlugin;

struct SigNetNodeInfo
{
    QString tuid;
    QHostAddress address;
    QString label;
    quint16 lastEndpoint = 0;
    quint16 endpointCount = 0;
    quint16 changeCount = 0;
    quint32 roleCapability = 0;
    bool offboarded = false;
    QDateTime lastSeen;
};

struct SigNetUniverseInfo
{
    int type = 0;
    quint16 signetUniverse = 1;
    quint16 senderEndpoint = 1;
    QString rdmTargetTuid;
    quint16 rdmTargetEndpoint = 1;
    QString rdmTargetAddress;
    QString activeRdmTargetTuid;
    quint16 activeRdmTargetEndpoint = 1;
    QStringList cachedRdmUids;
    QDateTime cachedRdmUidsUpdated;
    QByteArray inputData;
    QByteArray outputData;
};

class SigNetController final : public QObject
{
    Q_OBJECT

public:
    enum Type { Unknown = 0x0, Input = 0x01, Output = 0x02 };

    explicit SigNetController(SigNetPlugin* plugin,
                              const QNetworkInterface& iface,
                              const QNetworkAddressEntry& address,
                              quint32 line,
                              QObject* parent = nullptr);
    ~SigNetController() override;

    void addUniverse(quint32 universe, Type type);
    void removeUniverse(quint32 universe, Type type);
    void setSigNetUniverse(quint32 universe, quint16 signetUniverse);
    void setSenderEndpoint(quint32 universe, quint16 senderEndpoint);
    void setRdmTarget(quint32 universe, const QString& tuid, quint16 endpoint, const QString& address);

    void sendDmx(quint32 universe, const QByteArray& data, bool dataChanged);
    bool sendRDMCommand(quint32 universe, uchar command, QVariantList params);

    QString getNetworkIP() const;
    QList<quint32> universesList() const;
    SigNetUniverseInfo* getUniverseInfo(quint32 universe);
    Type type() const;
    quint32 line() const;
    quint64 getPacketSentNumber() const;
    quint64 getPacketReceivedNumber() const;
    QHash<QString, SigNetNodeInfo> discoveredNodes() const;

private:
    struct ReplayState
    {
        quint32 session = 0;
        quint32 sequence = 0;
    };

private:
    void ensureReceiveSocket();
    void refreshSubscriptions();
    void updateNodeInfo(const QString& tuid, const QHostAddress& sender, quint16 endpoint, const QList<SigNetPacketizer::TLV>& tlvs, bool offboarded);
    bool acceptFreshness(const SigNetPacketizer::Message& message);
    void handleLevelMessage(const SigNetPacketizer::Message& message);
    void handleNodeMessage(const SigNetPacketizer::Message& message, const QHostAddress& sender, bool offboarded);
    void handleRdmPayload(quint32 universe, const QByteArray& payload);
    void handleTodData(quint32 universe, const QByteArray& payload);
    bool emitCachedDiscovery(quint32 universe, qulonglong startUid, qulonglong endUid);
    QByteArray currentLocalTuid() const;

private slots:
    void processPendingPackets();
    void slotSendKeepAlive();
    void slotSendPoll();

signals:
    void valueChanged(quint32 universe, quint32 input, quint32 channel, uchar value);
    void rdmValueChanged(quint32 universe, quint32 line, QVariantMap data);

private:
    SigNetPlugin* m_plugin;
    QNetworkInterface m_interface;
    QHostAddress m_ipAddr;
    quint32 m_line;
    quint64 m_packetSent;
    quint64 m_packetReceived;
    QSharedPointer<QUdpSocket> m_sendSocket;
    QSharedPointer<QUdpSocket> m_receiveSocket;
    mutable QMutex m_dataMutex;
    QMap<quint32, SigNetUniverseInfo> m_universeMap;
    QSet<QHostAddress> m_levelSubscriptions;
    QHash<QString, SigNetNodeInfo> m_discoveredNodes;
    QHash<QByteArray, quint32> m_sessionByTuid;
    QHash<QByteArray, quint32> m_sequenceBySenderId;
    QTimer m_keepAliveTimer;
    QTimer m_pollTimer;
};

#endif
