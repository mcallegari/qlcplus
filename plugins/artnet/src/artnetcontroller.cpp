/*
  Q Light Controller Plus
  artnetcontroller.cpp

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

#include "artnetcontroller.h"
#include "rdmprotocol.h"

#include <QMutexLocker>
#include <QStringList>
#include <QDebug>

#define POLL_INTERVAL_MS   3000
#define SEND_INTERVAL_MS   2000

#define TRANSMIT_STANDARD  "Standard"
#define TRANSMIT_FULL      "Full"
#define TRANSMIT_PARTIAL   "Partial"

#define _DEBUG_RECEIVED_PACKETS 0

ArtNetController::ArtNetController(QNetworkInterface const& iface, QNetworkAddressEntry const& address,
                                   QSharedPointer<QUdpSocket> const& udpSocket,
                                   quint32 line, QObject *parent)
    : QObject(parent)
    , m_interface(iface)
    , m_address(address)
    , m_ipAddr(address.ip())
    , m_packetSent(0)
    , m_packetReceived(0)
    , m_line(line)
    , m_udpSocket(udpSocket)
    , m_packetizer(new ArtNetPacketizer())
    , m_pollTimer(NULL)
{
    if (m_ipAddr == QHostAddress::LocalHost)
    {
        m_broadcastAddr = QHostAddress::LocalHost;
        m_MACAddress = "11:22:33:44:55:66";
    }
    else
    {
        m_broadcastAddr = address.broadcast();
        m_MACAddress = iface.hardwareAddress();
    }

    qDebug() << "[ArtNetController] IP Address:" << m_ipAddr.toString() << " Broadcast address:" << m_broadcastAddr.toString() << "(MAC:" << m_MACAddress << ")";
}

ArtNetController::~ArtNetController()
{
    qDebug() << Q_FUNC_INFO;
}

ArtNetController::Type ArtNetController::type()
{
    int type = Unknown;
    foreach (UniverseInfo info, m_universeMap.values())
    {
        type |= info.type;
    }

    return Type(type);
}

quint32 ArtNetController::line()
{
    return m_line;
}

quint64 ArtNetController::getPacketSentNumber()
{
    return m_packetSent;
}

quint64 ArtNetController::getPacketReceivedNumber()
{
    return m_packetReceived;
}

bool ArtNetController::socketBound() const
{
    return m_udpSocket->state() == QAbstractSocket::BoundState;
}

QString ArtNetController::getNetworkIP()
{
    return m_ipAddr.toString();
}

QHash<QHostAddress, ArtNetNodeInfo> ArtNetController::getNodesList()
{
    return m_nodesList;
}

void ArtNetController::addUniverse(quint32 universe, ArtNetController::Type type)
{
    qDebug() << "[ArtNet] addUniverse - universe" << universe << ", type" << type;
    if (m_universeMap.contains(universe))
    {
        m_universeMap[universe].type |= (int)type;
    }
    else
    {
        UniverseInfo info;
        info.inputUniverse = universe;
        info.outputAddress = m_broadcastAddr;
        info.outputUniverse = universe;
        info.outputTransmissionMode = Standard;
        info.type = type;
        m_universeMap[universe] = info;
    }

    if (type == Output)
    {
        // activate ArtPoll if we open an Output
        if (m_pollTimer.isActive() == false)
        {
            m_pollTimer.setInterval(POLL_INTERVAL_MS);
            connect(&m_pollTimer, SIGNAL(timeout()), this, SLOT(slotSendPoll()));
            m_pollTimer.start();

            slotSendPoll();
        }
        if (m_sendTimer.isActive() == false &&
            m_universeMap[universe].outputTransmissionMode == Standard)
        {
            m_sendTimer.setInterval(SEND_INTERVAL_MS);
            connect(&m_sendTimer, SIGNAL(timeout()), this, SLOT(slotSendAllUniverses()));
            m_sendTimer.start();
        }
    }
}

void ArtNetController::removeUniverse(quint32 universe, ArtNetController::Type type)
{
    if (m_universeMap.contains(universe))
    {
        if (m_universeMap[universe].type == type)
            m_universeMap.take(universe);
        else
            m_universeMap[universe].type &= ~type;

        if (type == Output && ((this->type() & Output) == 0))
        {
            m_pollTimer.stop();
            disconnect(&m_pollTimer, SIGNAL(timeout()),
                       this, SLOT(slotSendPoll()));
        }
    }
}

bool ArtNetController::setInputUniverse(quint32 universe, quint32 artnetUni)
{
    if (!m_universeMap.contains(universe))
        return false;

    QMutexLocker locker(&m_dataMutex);
    m_universeMap[universe].inputUniverse = artnetUni;

    return universe == artnetUni;
}

bool ArtNetController::setOutputIPAddress(quint32 universe, QString address)
{
    if (!m_universeMap.contains(universe))
        return false;

    if (address.size() == 0)
    {
        m_universeMap[universe].outputAddress = m_broadcastAddr;
        return true;
    }

    QMutexLocker locker(&m_dataMutex);

    QHostAddress hostAddress(address);
    if (hostAddress.isNull() || !address.contains("."))
    {
        // IP addresses are now always fully saved
        qDebug() << "[setOutputIPAddress] Legacy IP style detected:" << address;
        QStringList iFaceIP = m_ipAddr.toString().split(".");
        QStringList addList = address.split(".");

        for (int i = 0; i < addList.count(); i++)
            iFaceIP.replace(4 - addList.count() + i , addList.at(i));

        QString newIP = iFaceIP.join(".");
        hostAddress = QHostAddress(newIP);
    }

    qDebug() << "[setOutputIPAddress] transmit to IP: " << hostAddress.toString();

    m_universeMap[universe].outputAddress = hostAddress;

    return hostAddress == m_broadcastAddr;
}

bool ArtNetController::setOutputUniverse(quint32 universe, quint32 artnetUni)
{
    if (!m_universeMap.contains(universe))
        return false;

    QMutexLocker locker(&m_dataMutex);
    m_universeMap[universe].outputUniverse = artnetUni;

    return universe == artnetUni;
}

bool ArtNetController::setTransmissionMode(quint32 universe, ArtNetController::TransmissionMode mode)
{
    if (!m_universeMap.contains(universe))
        return false;

    QMutexLocker locker(&m_dataMutex);
    m_universeMap[universe].outputTransmissionMode = int(mode);

    return mode == ArtNetController::Standard;
}

QString ArtNetController::transmissionModeToString(ArtNetController::TransmissionMode mode)
{
    switch (mode)
    {
        default:
        case Standard:
            return QString(TRANSMIT_STANDARD);
        break;
        case Full:
            return QString(TRANSMIT_FULL);
        break;
        case Partial:
            return QString(TRANSMIT_PARTIAL);
        break;
    }
}

ArtNetController::TransmissionMode ArtNetController::stringToTransmissionMode(const QString &mode)
{
    if (mode == QString(TRANSMIT_FULL))
        return Full;
    else if (mode == QString(TRANSMIT_PARTIAL))
        return Partial;
    else
        return Standard;
}

QList<quint32> ArtNetController::universesList()
{
    return m_universeMap.keys();
}

UniverseInfo *ArtNetController::getUniverseInfo(quint32 universe)
{
    if (m_universeMap.contains(universe))
        return &m_universeMap[universe];

    return NULL;
}

void ArtNetController::slotSendAllUniverses()
{
    QMutexLocker locker(&m_dataMutex);
    for (QMap<quint32, UniverseInfo>::iterator it = m_universeMap.begin(); it != m_universeMap.end(); ++it)
    {
        UniverseInfo &info = it.value();

        if ((info.type & Output) && info.outputTransmissionMode == Standard)
        {
            QByteArray dmxPacket;
            if (info.outputData.size() == 0)
                info.outputData.fill(0, 512);

            m_packetizer->setupArtNetDmx(dmxPacket, info.outputUniverse, info.outputData);

            qint64 sent = m_udpSocket->writeDatagram(dmxPacket, info.outputAddress, ARTNET_PORT);
            if (sent < 0)
            {
                qWarning() << "sendDmx failed";
                qWarning() << "Errno: " << m_udpSocket->error();
                qWarning() << "Errmgs: " << m_udpSocket->errorString();
            }
            else
            {
                m_packetSent++;
            }
        }
    }
}

void ArtNetController::sendDmx(const quint32 universe, const QByteArray &data, bool dataChanged)
{
    QMutexLocker locker(&m_dataMutex);
    QByteArray dmxPacket;
    QHostAddress outAddress = m_broadcastAddr;
    quint32 outUniverse = universe;
    TransmissionMode transmitMode = Standard;
    UniverseInfo *info = getUniverseInfo(universe);

    if (info == NULL)
    {
        qWarning() << "sendDmx: universe" << universe << "not registered as output!";
        return;
    }

    outAddress = info->outputAddress;
    outUniverse = info->outputUniverse;
    transmitMode = TransmissionMode(info->outputTransmissionMode);

    // if data has not changed since previous tick don't do anything.
    // A timer will refresh all universes every N seconds
    if (transmitMode == Standard && !dataChanged)
        return;

    if (transmitMode == Full || (transmitMode == Standard && dataChanged))
    {
        if (info->outputData.size() == 0)
            info->outputData.fill(0, 512);

        info->outputData.replace(0, data.length(), data);
        m_packetizer->setupArtNetDmx(dmxPacket, outUniverse, info->outputData);
    }
    else
    {
        m_packetizer->setupArtNetDmx(dmxPacket, outUniverse, data);
    }

    qint64 sent = m_udpSocket->writeDatagram(dmxPacket, outAddress, ARTNET_PORT);
    if (sent < 0)
    {
        qWarning() << "sendDmx failed";
        qWarning() << "Errno: " << m_udpSocket->error();
        qWarning() << "Errmgs: " << m_udpSocket->errorString();
    }
    else
    {
        m_packetSent++;
    }
}

bool ArtNetController::sendRDMCommand(const quint32 universe, uchar command, QVariantList params)
{
    QByteArray rdmPacket;
    QHostAddress outAddress = m_broadcastAddr;
    quint32 outUniverse = universe;

    bool result = false;

    if (m_universeMap.contains(universe))
    {
        UniverseInfo info = m_universeMap[universe];
        outAddress = info.outputAddress;
        outUniverse = info.outputUniverse;
    }

    if (command == DISCOVERY_COMMAND)
    {
        // ArtNet doesn't care about mute/unmute
        if (params.length() >= 2)
        {
            quint16 pid = params.at(1).toUInt();
            if (pid == PID_DISC_MUTE || pid == PID_DISC_UN_MUTE)
                return false;
        }
        m_packetizer->setupArtNetTodRequest(rdmPacket, outUniverse);
    }
    else
    {
        m_packetizer->setupArtNetRdm(rdmPacket, outUniverse, command, params);
    }

    //qDebug() << "Sending RDM command" << rdmPacket.toHex(',');

    qint64 sent = m_udpSocket->writeDatagram(rdmPacket, outAddress, ARTNET_PORT);
    if (sent < 0)
    {
        qWarning() << "sendDmx failed";
        qWarning() << "Errno: " << m_udpSocket->error();
        qWarning() << "Errmgs: " << m_udpSocket->errorString();
    }
    else
    {
        m_packetSent++;
        result = true;
    }

    return result;
}

bool ArtNetController::handleArtNetPollReply(QByteArray const& datagram, QHostAddress const& senderAddress)
{
    ArtNetNodeInfo newNode;
    if (!m_packetizer->fillArtPollReplyInfo(datagram, newNode))
    {
        qWarning() << "[ArtNet] Bad ArtPollReply received";
        return false;
    }

#if _DEBUG_RECEIVED_PACKETS
    qDebug() << "[ArtNet] ArtPollReply received";
#endif

    if (m_nodesList.contains(senderAddress) == false)
        m_nodesList[senderAddress] = newNode;

    ++m_packetReceived;
    return true;
}

bool ArtNetController::handleArtNetPoll(QByteArray const& datagram, QHostAddress const& senderAddress)
{
    Q_UNUSED(datagram);

#if _DEBUG_RECEIVED_PACKETS
    qDebug() << "[ArtNet] ArtPoll received";
#endif
    QByteArray pollReplyPacket;
    for (QMap<quint32, UniverseInfo>::iterator it = m_universeMap.begin(); it != m_universeMap.end(); ++it)
    {
        quint32 universe = it.key();
        UniverseInfo &info = it.value();
        bool isInput = (info.type & Input) ? true : false;

        m_packetizer->setupArtNetPollReply(pollReplyPacket, m_ipAddr, m_MACAddress, universe, isInput);
        m_udpSocket->writeDatagram(pollReplyPacket, senderAddress, ARTNET_PORT);
        ++m_packetSent;
    }
    ++m_packetReceived;
    return true;
}

bool ArtNetController::handleArtNetDmx(QByteArray const& datagram, QHostAddress const& senderAddress)
{
    Q_UNUSED(senderAddress);

    QByteArray dmxData;
    quint32 artnetUniverse;
    if (!m_packetizer->fillDMXdata(datagram, dmxData, artnetUniverse))
    {
        qWarning() << "[ArtNet] Bad DMX packet received";
        return false;
    }

#if _DEBUG_RECEIVED_PACKETS
    qDebug() << "[ArtNet] DMX data received. Universe:" << artnetUniverse << ", Data size:" << dmxData.size()
        << ", data[0]=" << (int)dmxData[0]
        << ", from=" << QHostAddress(senderAddress.toIPv4Address()).toString();
#endif

    for (QMap<quint32, UniverseInfo>::iterator it = m_universeMap.begin(); it != m_universeMap.end(); ++it)
    {
        quint32 universe = it.key();
        UniverseInfo &info = it.value();

        if ((info.type & Input) && info.inputUniverse == artnetUniverse)
        {
            if (info.inputData.size() == 0)
                info.inputData.fill(0, 512);

#if _DEBUG_RECEIVED_PACKETS
            qDebug() << "[ArtNet] -> universe" << (universe + 1);
#endif

            for (int i = 0; i < dmxData.length(); i++)
            {
                if (info.inputData.at(i) != dmxData.at(i))
                {
#if _DEBUG_RECEIVED_PACKETS
                    qDebug() << "[ArtNet] a value differs";
#endif
                    info.inputData.replace(i, 1, (const char *)(dmxData.data() + i), 1);
                    emit valueChanged(universe, m_line, i, (uchar)dmxData.at(i));
                }
            }
            ++m_packetReceived;
            return true;
        }
    }
    return false;
}

bool ArtNetController::handleArtNetTodData(const QByteArray &datagram, const QHostAddress &senderAddress)
{
    QVariantMap values;
    quint32 universe;

    Q_UNUSED(senderAddress);

    if (m_packetizer->processTODdata(datagram, universe, values) == true)
    {
        emit rdmValueChanged(universe, m_line, values);
        return true;
    }

    return false;
}

bool ArtNetController::handleArtNetRDM(const QByteArray &datagram, const QHostAddress &senderAddress)
{
    QVariantMap values;
    quint32 universe;

    Q_UNUSED(senderAddress);

    if (m_packetizer->processRDMdata(datagram, universe, values) == true)
    {
        emit rdmValueChanged(universe, m_line, values);
        return true;
    }

    return false;
}

bool ArtNetController::handlePacket(QByteArray const& datagram, QHostAddress const& senderAddress)
{
    //if (senderAddress.toIPv4Address() == m_ipAddr.toIPv4Address())
    //    return false;

#if _DEBUG_RECEIVED_PACKETS
    qDebug() << "Received packet with size: " << datagram.size() << ", host: " << QHostAddress(senderAddress.toIPv4Address()).toString();
#endif
    quint16 opCode = -1;

    if (m_packetizer->checkPacketAndCode(datagram, opCode) == true)
    {
        switch (opCode)
        {
            case ARTNET_POLLREPLY:
                return handleArtNetPollReply(datagram, senderAddress);
            case ARTNET_POLL:
                return handleArtNetPoll(datagram, senderAddress);
            case ARTNET_DMX:
                return handleArtNetDmx(datagram, senderAddress);
            case ARTNET_TODDATA:
                return handleArtNetTodData(datagram, senderAddress);
            case ARTNET_RDM:
                return handleArtNetRDM(datagram, senderAddress);
            default:
                qDebug().nospace().noquote() << "[ArtNet] opCode not supported yet (0x" << QString::number(opCode, 16) << ")";
                break;
        }
    }
    else
        qWarning() << "[ArtNet] Malformed packet received";

    return true;
}

void ArtNetController::slotSendPoll()
{
#if 0
    QList<QHostAddress>addressList;

    /* first, retrieve a list of unique output addresses */
    foreach (quint32 universe, universesList())
    {
        UniverseInfo info = m_universeMap[universe];
        if (addressList.contains(info.outputAddress) == false)
            addressList.append(info.outputAddress);
    }

    /* then send a poll to each address collected */
    foreach (QHostAddress addr, addressList)
    {
        QByteArray pollPacket;
        m_packetizer->setupArtNetPoll(pollPacket);
        qint64 sent = m_udpSocket->writeDatagram(pollPacket, addr, ARTNET_PORT);
        if (sent < 0)
            qWarning() << "Unable to send Poll packet: errno=" << m_udpSocket->error() << "(" << m_udpSocket->errorString() << ")";
        else
            m_packetSent++;
    }
#else
    QByteArray pollPacket;
    m_packetizer->setupArtNetPoll(pollPacket);

    qint64 sent = m_udpSocket->writeDatagram(pollPacket, m_broadcastAddr, ARTNET_PORT);
    if (sent < 0)
        qWarning() << "Unable to send Poll packet: errno=" << m_udpSocket->error() << "(" << m_udpSocket->errorString() << ")";
    else
        m_packetSent++;
#endif
}
