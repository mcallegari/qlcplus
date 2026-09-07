/*
  Q Light Controller Plus
  signetcontroller.cpp

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

#include "signetcontroller.h"

#include <QDebug>
#include <QMutexLocker>
#include <utility>

#include "signetpacketizer.h"
#include "signetplugin.h"
#include "../interfaces/rdmprotocol.h"
#include "sig-net-constants.hpp"

namespace
{
constexpr quint16 KDefaultRdmEndpoint = 1;
constexpr int KKeepAliveRateMs = 1000;
constexpr int KPollRateMs = 3000;
constexpr int KBeaconTimeoutMs = 15000;

QStringList uriSegments(const QString& uri)
{
    return uri.split('/', Qt::SkipEmptyParts);
}

quint16 payloadToU16(const QByteArray& data)
{
    if (data.size() < 2)
        return 0;
    return (quint8(data.at(0)) << 8) | quint8(data.at(1));
}

quint32 payloadToU32(const QByteArray& data)
{
    if (data.size() < 4)
        return 0;
    return (quint32(quint8(data.at(0))) << 24) |
           (quint32(quint8(data.at(1))) << 16) |
           (quint32(quint8(data.at(2))) << 8) |
           quint8(data.at(3));
}

QString payloadToLabel(const QByteArray& data)
{
    return QString::fromUtf8(data);
}

qulonglong uidValue(const QString& uid)
{
    bool ok = false;
    const qulonglong value = uid.toULongLong(&ok, 16);
    return ok ? value : std::numeric_limits<qulonglong>::max();
}
}

SigNetController::SigNetController(SigNetPlugin* plugin,
                                   const QNetworkInterface& iface,
                                   const QNetworkAddressEntry& address,
                                   quint32 line,
                                   QObject* parent)
    : QObject(parent)
    , m_plugin(plugin)
    , m_interface(iface)
    , m_ipAddr(address.ip())
    , m_line(line)
    , m_packetSent(0)
    , m_packetReceived(0)
    , m_sendSocket(new QUdpSocket(this))
{
    m_sendSocket->bind(m_ipAddr, 0);
    m_sendSocket->setMulticastInterface(m_interface);
    m_sendSocket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, false);

    ensureReceiveSocket();

    m_keepAliveTimer.setInterval(KKeepAliveRateMs);
    connect(&m_keepAliveTimer, &QTimer::timeout, this, &SigNetController::slotSendKeepAlive);
    m_keepAliveTimer.start();

    m_pollTimer.setInterval(KPollRateMs);
    connect(&m_pollTimer, &QTimer::timeout, this, &SigNetController::slotSendPoll);
    m_pollTimer.start();
    slotSendPoll();
}

SigNetController::~SigNetController()
{
}

void SigNetController::ensureReceiveSocket()
{
    if (m_receiveSocket)
        return;

    m_receiveSocket.reset(new QUdpSocket(this));
    if (!m_receiveSocket->bind(QHostAddress::AnyIPv4,
                               SigNet::SIGNET_UDP_PORT,
                               QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))
    {
        qWarning() << "[SigNet] Unable to bind receive socket:" << m_receiveSocket->errorString();
        m_receiveSocket.clear();
        return;
    }

    connect(m_receiveSocket.data(), &QUdpSocket::readyRead, this, &SigNetController::processPendingPackets);

    m_receiveSocket->joinMulticastGroup(SigNetPacketizer::nodeSendAddress(), m_interface);
    m_receiveSocket->joinMulticastGroup(SigNetPacketizer::nodeLostAddress(), m_interface);
    m_receiveSocket->joinMulticastGroup(SigNetPacketizer::nodeBeaconAddress(), m_interface);
    m_receiveSocket->joinMulticastGroup(SigNetPacketizer::managerPollAddress(), m_interface);
}

QString SigNetController::getNetworkIP() const
{
    return m_ipAddr.toString();
}

void SigNetController::addUniverse(quint32 universe, SigNetController::Type type)
{
    QMutexLocker locker(&m_dataMutex);

    if (m_universeMap.contains(universe))
    {
        m_universeMap[universe].type |= type;
    }
    else
    {
        SigNetUniverseInfo info;
        info.type = type;
        info.signetUniverse = universe + 1;
        info.senderEndpoint = universe + 1;
        info.rdmTargetEndpoint = universe + 1;
        m_universeMap[universe] = info;
    }

    refreshSubscriptions();
}

void SigNetController::removeUniverse(quint32 universe, SigNetController::Type type)
{
    QMutexLocker locker(&m_dataMutex);

    if (!m_universeMap.contains(universe))
        return;

    if (m_universeMap[universe].type == type)
        m_universeMap.remove(universe);
    else
        m_universeMap[universe].type &= ~type;

    refreshSubscriptions();
}

void SigNetController::setSigNetUniverse(quint32 universe, quint16 signetUniverse)
{
    QMutexLocker locker(&m_dataMutex);
    if (!m_universeMap.contains(universe))
        return;

    m_universeMap[universe].signetUniverse = signetUniverse;
    refreshSubscriptions();
}

void SigNetController::setSenderEndpoint(quint32 universe, quint16 senderEndpoint)
{
    QMutexLocker locker(&m_dataMutex);
    if (m_universeMap.contains(universe))
        m_universeMap[universe].senderEndpoint = senderEndpoint;
}

void SigNetController::setRdmTarget(quint32 universe, const QString& tuid, quint16 endpoint, const QString& address)
{
    QMutexLocker locker(&m_dataMutex);
    if (!m_universeMap.contains(universe))
        return;

    m_universeMap[universe].rdmTargetTuid = tuid.toUpper();
    m_universeMap[universe].rdmTargetEndpoint = endpoint;
    m_universeMap[universe].rdmTargetAddress = address;
}

void SigNetController::refreshSubscriptions()
{
    if (!m_receiveSocket)
        return;

    QSet<QHostAddress> required;
    for (auto it = m_universeMap.cbegin(); it != m_universeMap.cend(); ++it)
    {
        if (it.value().type & Input)
            required.insert(SigNetPacketizer::levelMulticastAddress(it.value().signetUniverse));
    }

    for (const QHostAddress& address : std::as_const(m_levelSubscriptions))
    {
        if (!required.contains(address))
            m_receiveSocket->leaveMulticastGroup(address, m_interface);
    }

    for (const QHostAddress& address : required)
    {
        if (!m_levelSubscriptions.contains(address))
            m_receiveSocket->joinMulticastGroup(address, m_interface);
    }

    m_levelSubscriptions = required;
}

QList<quint32> SigNetController::universesList() const
{
    return m_universeMap.keys();
}

SigNetUniverseInfo* SigNetController::getUniverseInfo(quint32 universe)
{
    if (!m_universeMap.contains(universe))
        return nullptr;

    return &m_universeMap[universe];
}

SigNetController::Type SigNetController::type() const
{
    int types = Unknown;
    for (const SigNetUniverseInfo& info : m_universeMap)
        types |= info.type;

    return Type(types);
}

quint32 SigNetController::line() const
{
    return m_line;
}

quint64 SigNetController::getPacketSentNumber() const
{
    return m_packetSent;
}

quint64 SigNetController::getPacketReceivedNumber() const
{
    return m_packetReceived;
}

QHash<QString, SigNetNodeInfo> SigNetController::discoveredNodes() const
{
    QHash<QString, SigNetNodeInfo> nodes;
    const QDateTime cutoff = QDateTime::currentDateTimeUtc().addMSecs(-KBeaconTimeoutMs);
    for (auto it = m_discoveredNodes.cbegin(); it != m_discoveredNodes.cend(); ++it)
    {
        if (it.value().offboarded && it.value().lastSeen < cutoff)
            continue;
        nodes.insert(it.key(), it.value());
    }
    return nodes;
}

QByteArray SigNetController::currentLocalTuid() const
{
    return m_plugin->localTuidBytes();
}

void SigNetController::sendDmx(quint32 universe, const QByteArray& data, bool dataChanged)
{
    QMutexLocker locker(&m_dataMutex);
    if (!m_universeMap.contains(universe) || !(m_universeMap[universe].type & Output))
        return;

    SigNetUniverseInfo& info = m_universeMap[universe];
    if (dataChanged || info.outputData.isEmpty())
        info.outputData = data.left(SigNet::MAX_DMX_SLOTS);

    if (!dataChanged || !m_plugin->securityReady())
        return;

    const QByteArray packet = SigNetPacketizer::buildLevelPacket(m_plugin->scope(),
                                                                 info.signetUniverse,
                                                                 info.outputData,
                                                                 currentLocalTuid(),
                                                                 info.senderEndpoint,
                                                                 m_plugin->sessionId(),
                                                                 m_plugin->nextSequence(info.senderEndpoint),
                                                                 m_plugin->nextMessageId(),
                                                                 m_plugin->senderKey());
    if (packet.isEmpty())
        return;

    if (m_sendSocket->writeDatagram(packet,
                                    SigNetPacketizer::levelMulticastAddress(info.signetUniverse),
                                    SigNet::SIGNET_UDP_PORT) > 0)
    {
        ++m_packetSent;
    }
}

bool SigNetController::sendRDMCommand(quint32 universe, uchar command, QVariantList params)
{
    if (!m_plugin->securityReady())
        return false;

    QMutexLocker locker(&m_dataMutex);
    if (!m_universeMap.contains(universe))
        return false;

    if (command == DISCOVERY_COMMAND)
    {
        quint16 pid = params.value(1).toUInt();
        if (pid == PID_DISC_MUTE || pid == PID_DISC_UN_MUTE)
            return true;

        if (pid == PID_DISC_UNIQUE_BRANCH)
        {
            const qulonglong startUid = params.value(2).toULongLong();
            const qulonglong endUid = params.value(3).toULongLong();
            if (emitCachedDiscovery(universe, startUid, endUid))
                return true;
        }
    }

    SigNetUniverseInfo& info = m_universeMap[universe];
    QString targetTuidString = info.rdmTargetTuid.toUpper();
    if (targetTuidString.isEmpty() && !params.isEmpty())
    {
        const QString requestedUid = params.first().toString().trimmed().toUpper();
        if (m_discoveredNodes.contains(requestedUid))
            targetTuidString = requestedUid;
    }

    if (targetTuidString.isEmpty() && m_discoveredNodes.size() == 1)
        targetTuidString = m_discoveredNodes.cbegin().key();

    if (targetTuidString.isEmpty())
    {
        qWarning().nospace().noquote()
            << "[SigNet] Unable to resolve target node TUID for RDM on universe " << universe
            << ". Configure " << SIGNET_RDM_TUID
            << " with the Sig-Net node TUID, not the downstream RDM responder UID.";
        return false;
    }

    quint16 targetEndpoint = info.rdmTargetEndpoint;
    QString targetAddress = info.rdmTargetAddress;
    const SigNetNodeInfo discoveredNode = m_discoveredNodes.value(targetTuidString);
    if (!discoveredNode.tuid.isEmpty())
    {
        if (targetAddress.isEmpty() && !discoveredNode.address.isNull())
            targetAddress = discoveredNode.address.toString();
    }

    if (targetEndpoint == 0)
        targetEndpoint = KDefaultRdmEndpoint;

    QByteArray targetTuid;
    if (!SigNetPacketizer::parseTuid(targetTuidString, targetTuid))
        return false;

    const QByteArray managerLocalKey = m_plugin->managerLocalKey(targetTuidString);
    if (managerLocalKey.isEmpty())
        return false;

    QByteArray packet;
    if (command == DISCOVERY_COMMAND)
    {
        packet = SigNetPacketizer::buildTodControlPacket(m_plugin->scope(),
                                                         currentLocalTuid(),
                                                         targetTuid,
                                                         targetEndpoint,
                                                         0x01,
                                                         m_plugin->sessionId(),
                                                         m_plugin->nextSequence(0),
                                                         m_plugin->nextMessageId(),
                                                         managerLocalKey);
    }
    else
    {
        RDMProtocol rdm;
        QByteArray rdmPayload;
        if (!rdm.packetizeCommand(command, params, true, rdmPayload))
            return false;

        packet = SigNetPacketizer::buildRdmCommandPacket(m_plugin->scope(),
                                                         currentLocalTuid(),
                                                         targetTuid,
                                                         targetEndpoint,
                                                         rdmPayload,
                                                         m_plugin->sessionId(),
                                                         m_plugin->nextSequence(0),
                                                         m_plugin->nextMessageId(),
                                                         managerLocalKey);
    }

    if (packet.isEmpty())
        return false;

    QHostAddress destination = SigNetPacketizer::managerSendAddress();
    if (discoveredNode.address.isNull() == false)
        destination = discoveredNode.address;
    else if (!targetAddress.isEmpty())
        destination = QHostAddress(targetAddress);

    const qint64 written = m_sendSocket->writeDatagram(packet, destination, SigNet::SIGNET_UDP_PORT);
    if (written > 0)
    {
        info.activeRdmTargetTuid = targetTuidString;
        info.activeRdmTargetEndpoint = targetEndpoint;
        ++m_packetSent;
        return true;
    }

    return false;
}

bool SigNetController::acceptFreshness(const SigNetPacketizer::Message& message)
{
    const QByteArray senderId(reinterpret_cast<const char*>(message.options.sender_id), SigNet::SENDER_ID_LENGTH);
    const QByteArray tuid(senderId.constData(), SigNet::TUID_LENGTH);

    const quint32 storedSession = m_sessionByTuid.value(tuid, 0);
    if (!m_sessionByTuid.contains(tuid))
    {
        m_sessionByTuid.insert(tuid, message.options.session_id);
        m_sequenceBySenderId.insert(senderId, message.options.seq_num);
        return true;
    }

    if (message.options.session_id < storedSession)
        return false;

    if (message.options.session_id > storedSession)
    {
        m_sessionByTuid[tuid] = message.options.session_id;
        m_sequenceBySenderId[senderId] = message.options.seq_num;
        return true;
    }

    if (!m_sequenceBySenderId.contains(senderId))
    {
        m_sequenceBySenderId.insert(senderId, message.options.seq_num);
        return true;
    }

    if (message.options.seq_num <= m_sequenceBySenderId.value(senderId))
        return false;

    m_sequenceBySenderId[senderId] = message.options.seq_num;
    return true;
}

void SigNetController::handleLevelMessage(const SigNetPacketizer::Message& message)
{
    const QStringList segments = uriSegments(message.uri);
    if (segments.size() < 5)
        return;

    bool ok = false;
    const quint16 signetUniverse = segments.at(4).toUShort(&ok);
    if (!ok)
        return;

    QByteArray levelData;
    for (const SigNetPacketizer::TLV& tlv : message.tlvs)
    {
        if (tlv.type == SigNet::TID_LEVEL)
        {
            levelData = tlv.value;
            break;
        }
    }

    if (levelData.isEmpty())
        return;

    for (auto it = m_universeMap.begin(); it != m_universeMap.end(); ++it)
    {
        if (!(it.value().type & Input) || it.value().signetUniverse != signetUniverse)
            continue;

        if (it.value().inputData.size() < levelData.size())
            it.value().inputData.fill(0, levelData.size());

        for (int i = 0; i < levelData.size(); ++i)
        {
            if (it.value().inputData.at(i) == levelData.at(i))
                continue;

            it.value().inputData[i] = levelData.at(i);
            emit valueChanged(it.key(), m_line, i, static_cast<uchar>(levelData.at(i)));
        }
    }
}

void SigNetController::handlePollMessage(const SigNetPacketizer::Message& message)
{
    const QByteArray localTuid = currentLocalTuid();
    for (const SigNetPacketizer::TLV& tlv : message.tlvs)
    {
        if (tlv.type != SigNet::TID_POLL || tlv.value.size() != 25)
            continue;

        const QByteArray tuidLo = tlv.value.mid(10, SigNet::TUID_LENGTH);
        const QByteArray tuidHi = tlv.value.mid(16, SigNet::TUID_LENGTH);
        const quint16 targetEndpoint = payloadToU16(tlv.value.mid(22, 2));
        const quint8 queryLevel = quint8(tlv.value.at(24));
        if (queryLevel < SigNet::QUERY_FULL || localTuid < tuidLo || localTuid > tuidHi ||
            (targetEndpoint != 0 && targetEndpoint != 0xFFFF))
        {
            continue;
        }

        quint32 roleCapability = SigNet::ROLE_CAP_MANAGER;
        {
            QMutexLocker locker(&m_dataMutex);
            const Type controllerType = type();
            if (controllerType & Output)
                roleCapability |= SigNet::ROLE_CAP_SENDER;
            if (controllerType & Input)
                roleCapability |= SigNet::ROLE_CAP_VISUALISER;
        }

        const QByteArray packet = SigNetPacketizer::buildPollReplyPacket(
            m_plugin->scope(),
            localTuid,
            m_plugin->sessionId(),
            m_plugin->nextSequence(0),
            m_plugin->nextMessageId(),
            m_plugin->citizenKey(),
            QStringLiteral("QLC+ SigNet"),
            roleCapability);
        if (packet.isEmpty())
            return;

        if (m_sendSocket->writeDatagram(packet,
                                        SigNetPacketizer::nodeSendAddress(),
                                        SigNet::SIGNET_UDP_PORT) > 0)
        {
            ++m_packetSent;
        }
        return;
    }
}

void SigNetController::updateNodeInfo(const QString& tuid,
                                      const QHostAddress& sender,
                                      quint16 endpoint,
                                      const QList<SigNetPacketizer::TLV>& tlvs,
                                      bool offboarded)
{
    SigNetNodeInfo info = m_discoveredNodes.value(tuid);
    bool pollReplySeen = false;
    const QDateTime now = QDateTime::currentDateTimeUtc();
    if (offboarded && info.offboarded && !info.address.isNull() && info.address != sender &&
        info.lastSeen.msecsTo(now) <= KBeaconTimeoutMs)
    {
        info.beaconCollision = true;
        qWarning().nospace().noquote()
            << "[SigNet] SNOW beacon collision for TUID " << tuid
            << ": " << info.address.toString() << " and " << sender.toString();
    }
    info.tuid = tuid;
    info.address = sender;
    info.lastEndpoint = endpoint;
    info.offboarded = offboarded;
    info.lastSeen = now;

    for (const SigNetPacketizer::TLV& tlv : tlvs)
    {
        switch (tlv.type)
        {
            case SigNet::TID_POLL_REPLY:
                if (tlv.value.size() >= 12)
                {
                    pollReplySeen = true;
                    info.soemCode = (quint32(quint8(tlv.value.at(6))) << 24) |
                                    (quint32(quint8(tlv.value.at(7))) << 16) |
                                    (quint32(quint8(tlv.value.at(8))) << 8) |
                                    quint8(tlv.value.at(9));
                    info.changeCount = payloadToU16(tlv.value.mid(10, 2));
                }
                break;
            case SigNet::TID_RT_DEVICE_LABEL:
                info.label = payloadToLabel(tlv.value);
                break;
            case SigNet::TID_RT_ENDPOINT_COUNT:
                info.endpointCount = payloadToU16(tlv.value);
                break;
            case SigNet::TID_RT_ROLE_CAPABILITY:
                // Protocol v1.07 uses a big-endian 32-bit role bitfield. Accept
                // the previous one-byte representation as a compatibility aid.
                info.roleCapability = tlv.value.size() >= 4 ? payloadToU32(tlv.value) :
                                      (tlv.value.isEmpty() ? 0 : quint8(tlv.value.at(0)));
                break;
            case SigNet::TID_RT_OTW_CAPABILITY:
                if (tlv.value.size() == 3)
                {
                    info.otwPort = payloadToU16(tlv.value);
                    info.otwCapabilities = quint8(tlv.value.at(2));
                }
                break;
            case SigNet::TID_EP_DIRECTION:
                if (endpoint != 0 && !tlv.value.isEmpty())
                {
                    if (quint8(tlv.value.at(0)) & SigNet::EP_DIR_RDM_ENABLE)
                        info.rdmEndpoints.insert(endpoint);
                    else
                        info.rdmEndpoints.remove(endpoint);
                }
                break;
            default:
                break;
        }
    }

    m_discoveredNodes.insert(tuid, info);

    if (pollReplySeen)
    {
        qDebug().nospace().noquote()
            << "[SigNet] POLL_REPLY UID " << info.tuid
            << " address " << info.address.toString()
            << " endpoint " << info.lastEndpoint
            << " endpoints " << info.endpointCount
            << " label \"" << info.label << "\"";
    }
}

void SigNetController::requestTodData(const QString& targetTuid,
                                      quint16 targetEndpoint,
                                      const QHostAddress& destination)
{
    if (targetEndpoint == 0 || destination.isNull())
        return;

    QByteArray targetTuidBytes;
    if (!SigNetPacketizer::parseTuid(targetTuid, targetTuidBytes))
        return;

    const QByteArray managerLocalKey = m_plugin->managerLocalKey(targetTuid);
    if (managerLocalKey.isEmpty())
        return;

    const QByteArray packet = SigNetPacketizer::buildTodControlPacket(m_plugin->scope(),
                                                                        currentLocalTuid(),
                                                                        targetTuidBytes,
                                                                        targetEndpoint,
                                                                        SigNet::TOD_CONTROL_FLUSH_FULL,
                                                                        m_plugin->sessionId(),
                                                                        m_plugin->nextSequence(0),
                                                                        m_plugin->nextMessageId(),
                                                                        managerLocalKey);
    if (packet.isEmpty())
        return;

    if (m_sendSocket->writeDatagram(packet, destination, SigNet::SIGNET_UDP_PORT) > 0)
        ++m_packetSent;
}

void SigNetController::handleRdmPayload(quint32 universe, const QByteArray& payload)
{
    QVariantMap values;
    RDMProtocol rdm;
    if (rdm.parsePacket(payload, values))
        emit rdmValueChanged(universe, m_line, values);
}

void SigNetController::handleTodData(quint32 universe, const QByteArray& payload)
{
    QVariantMap values;
    QStringList cachedUids;

    if (payload.size() < 2)
        return;

    const int totalPackets = quint8(payload.at(1));
    const int uidCount = qMax(0, (payload.size() - 2) / 6);
    Q_UNUSED(totalPackets)

    values.insert(QStringLiteral("DISCOVERY_COUNT"), uidCount);
    for (int i = 0; i < uidCount; ++i)
    {
        quint16 estaId = 0;
        quint32 deviceId = 0;
        const QString uid = RDMProtocol::byteArrayToUID(payload.mid(2 + (i * 6), 6), estaId, deviceId);
        cachedUids << uid;
        values.insert(QStringLiteral("UID-%1").arg(i), uid);
    }

    QMutexLocker locker(&m_dataMutex);
    if (m_universeMap.contains(universe))
    {
        m_universeMap[universe].cachedRdmUids = cachedUids;
        m_universeMap[universe].cachedRdmUidsUpdated = QDateTime::currentDateTimeUtc();
    }

    emit rdmValueChanged(universe, m_line, values);
}

void SigNetController::recordTodData(const QString& targetTuid, quint16 endpoint, const QByteArray& payload)
{
    if (payload.size() < 2 || !m_discoveredNodes.contains(targetTuid))
        return;

    QStringList& cachedUids = m_discoveredNodes[targetTuid].rdmTodUids[endpoint];
    QStringList receivedUids;
    const int uidCount = qMax(0, (payload.size() - 2) / 6);
    for (int i = 0; i < uidCount; ++i)
    {
        quint16 estaId = 0;
        quint32 deviceId = 0;
        const QString uid = RDMProtocol::byteArrayToUID(payload.mid(2 + (i * 6), 6), estaId, deviceId);
        if (!uid.isEmpty())
        {
            receivedUids.append(uid);
            m_discoveredRdmUids.insert(uid);
            if (!cachedUids.contains(uid))
                cachedUids.append(uid);
        }
    }
    cachedUids.sort();

    qDebug().nospace().noquote()
        << "[SigNet] RDM TOD_DATA from " << targetTuid
        << " endpoint " << endpoint
        << " contains " << receivedUids.size() << " UID(s): "
        << receivedUids.join(", ");
}

bool SigNetController::emitCachedDiscovery(quint32 universe, qulonglong startUid, qulonglong endUid)
{
    if (!m_universeMap.contains(universe))
        return false;

    QSet<QString> cachedUidSet;
    for (const QString& uid : m_universeMap.value(universe).cachedRdmUids)
        cachedUidSet.insert(uid);
    cachedUidSet.unite(m_discoveredRdmUids);

    QStringList cachedUids = cachedUidSet.values();
    if (cachedUids.isEmpty())
    {
        for (auto it = m_discoveredNodes.cbegin(); it != m_discoveredNodes.cend(); ++it)
        {
            if (!it.value().offboarded)
                cachedUids << it.key();
        }
        cachedUids.removeDuplicates();
    }

    cachedUids.sort();

    if (cachedUids.isEmpty())
        return false;

    QVariantMap values;
    int count = 0;
    for (const QString& uid : cachedUids)
    {
        const qulonglong numericUid = uidValue(uid);
        if (numericUid == std::numeric_limits<qulonglong>::max())
            continue;
        if (numericUid < startUid || numericUid > endUid)
            continue;

        values.insert(QStringLiteral("UID-%1").arg(count), uid);
        ++count;
    }

    values.insert(QStringLiteral("DISCOVERY_COUNT"), count);
    qDebug().nospace().noquote()
        << "[SigNet] Cached RDM discovery for universe " << universe
        << " returned " << count << " UID(s)";
    emit rdmValueChanged(universe, m_line, values);
    return true;
}

void SigNetController::handleNodeMessage(const SigNetPacketizer::Message& message, const QHostAddress& sender, bool offboarded)
{
    const QStringList segments = uriSegments(message.uri);
    if (segments.size() < 5)
        return;

    const QString targetTuid = segments.at(4).toUpper();
    bool ok = true;
    quint16 endpoint = 0;
    if (segments.size() >= 6)
    {
        endpoint = segments.at(5).toUShort(&ok);
        if (!ok)
            return;
    }

    updateNodeInfo(targetTuid, sender, endpoint, message.tlvs, offboarded);

    for (const SigNetPacketizer::TLV& tlv : message.tlvs)
    {
        if (tlv.type == SigNet::TID_EP_DIRECTION &&
            endpoint != 0 &&
            !tlv.value.isEmpty() &&
            (quint8(tlv.value.at(0)) & SigNet::EP_DIR_RDM_ENABLE))
        {
            requestTodData(targetTuid, endpoint, sender);
        }
        else if (tlv.type == SigNet::TID_RDM_TOD_DATA)
        {
            recordTodData(targetTuid, endpoint, tlv.value);
        }
    }

    for (auto it = m_universeMap.cbegin(); it != m_universeMap.cend(); ++it)
    {
        const bool configuredMatch = it.value().rdmTargetTuid.compare(targetTuid, Qt::CaseInsensitive) == 0;
        const bool activeMatch = it.value().activeRdmTargetTuid.compare(targetTuid, Qt::CaseInsensitive) == 0;
        if (!configuredMatch && !activeMatch)
            continue;

        const quint32 universe = it.key();
        for (const SigNetPacketizer::TLV& tlv : message.tlvs)
        {
            if (tlv.type == SigNet::TID_RDM_RESPONSE)
                handleRdmPayload(universe, tlv.value);
            else if (tlv.type == SigNet::TID_RDM_TOD_DATA)
                handleTodData(universe, tlv.value);
        }
    }
}

void SigNetController::processPendingPackets()
{
    if (!m_receiveSocket)
        return;

    while (m_receiveSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(int(m_receiveSocket->pendingDatagramSize()));
        QHostAddress sender;
        quint16 port = 0;
        m_receiveSocket->readDatagram(datagram.data(), datagram.size(), &sender, &port);
        Q_UNUSED(port)

        SigNetPacketizer::Message message;
        QString error;
        if (!SigNetPacketizer::parseMessage(datagram, message, &error))
            continue;

        const QStringList segments = uriSegments(message.uri);
        if (segments.size() < 4)
            continue;

        if (segments.at(0) != QLatin1String(SigNet::SIGNET_URI_PREFIX) ||
            segments.at(1) != QLatin1String(SigNet::SIGNET_URI_VERSION))
        {
            continue;
        }

        const QString scope = segments.at(2);
        const QString resource = segments.at(3);

        // SNOW discovery is deliberately scoped to local, even when this
        // manager operates the show on another Sig-Net scope.
        QByteArray verificationKey;
        if (message.options.security_mode == SigNet::SECURITY_MODE_UNPROVISIONED)
        {
            if (scope == QLatin1String(SigNet::SIGNET_URI_SCOPE_DEFAULT) &&
                resource == QLatin1String("node_beacon"))
                handleNodeMessage(message, sender, true);
            continue;
        }

        if (scope != m_plugin->scope())
            continue;

        if (resource == QLatin1String("level"))
            verificationKey = m_plugin->senderKey();
        else if (resource == QLatin1String("node") || resource == QLatin1String("node_lost"))
            verificationKey = m_plugin->citizenKey();
        else if (resource == QLatin1String("poll"))
            verificationKey = m_plugin->managerGlobalKey();
        else
            continue;

        if (!SigNetPacketizer::verifyMessage(message, verificationKey))
            continue;

        if (!acceptFreshness(message))
            continue;

        ++m_packetReceived;

        if (resource == QLatin1String("level"))
            handleLevelMessage(message);
        else if (resource == QLatin1String("poll"))
            handlePollMessage(message);
        else
            handleNodeMessage(message, sender, resource == QLatin1String("node_lost"));
    }
}

void SigNetController::slotSendKeepAlive()
{
    if (!m_plugin->securityReady())
        return;

    QMutexLocker locker(&m_dataMutex);
    for (auto it = m_universeMap.begin(); it != m_universeMap.end(); ++it)
    {
        if (!(it.value().type & Output) || it.value().outputData.isEmpty())
            continue;

        const QByteArray packet = SigNetPacketizer::buildLevelPacket(m_plugin->scope(),
                                                                     it.value().signetUniverse,
                                                                     it.value().outputData,
                                                                     currentLocalTuid(),
                                                                     it.value().senderEndpoint,
                                                                     m_plugin->sessionId(),
                                                                     m_plugin->nextSequence(it.value().senderEndpoint),
                                                                     m_plugin->nextMessageId(),
                                                                     m_plugin->senderKey());
        if (packet.isEmpty())
            continue;

        if (m_sendSocket->writeDatagram(packet,
                                        SigNetPacketizer::levelMulticastAddress(it.value().signetUniverse),
                                        SigNet::SIGNET_UDP_PORT) > 0)
        {
            ++m_packetSent;
        }
    }
}

void SigNetController::slotSendPoll()
{
    if (!m_plugin->securityReady())
        return;

    const QByteArray packet = SigNetPacketizer::buildPollPacket(m_plugin->scope(),
                                                                currentLocalTuid(),
                                                                m_plugin->sessionId(),
                                                                m_plugin->nextSequence(0),
                                                                m_plugin->nextMessageId(),
                                                                m_plugin->managerGlobalKey(),
                                                                m_initialFullPoll ? SigNet::QUERY_FULL : SigNet::QUERY_HEARTBEAT);
    if (packet.isEmpty())
        return;

    if (m_sendSocket->writeDatagram(packet,
                                    SigNetPacketizer::managerPollAddress(),
                                    SigNet::SIGNET_UDP_PORT) > 0)
    {
        ++m_packetSent;
        m_initialFullPoll = false;
    }
}
