/*
  Q Light Controller Plus
  networkmanager.cpp

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

#include <QNetworkInterface>

#include "networkmanager.h"
#include "networkpacketizer.h"
#include "simplecrypt.h"
#include "tardisactions.h"

#define DEFAULT_UDP_PORT    9997
#define DEFAULT_TCP_PORT    9998

static const quint64 defaultKey = 0x5131632B4E33744B; // this is "Q1c+N3tK"

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , m_udpSocket(NULL)
    , m_tcpServer(NULL)
    , m_serverStarted(false)
    , m_tcpSocket(NULL)
    , m_clientConnected(false)
{
    m_hostType = UnknownHostType;
    setHostName(defaultName());
    m_crypt = new SimpleCrypt(defaultKey);
    m_packetizer = new NetworkPacketizer();
}

NetworkManager::~NetworkManager()
{
    stopServer();

    if (m_udpSocket != NULL)
    {
        m_udpSocket->close();
        delete m_udpSocket;
    }

    if (m_tcpServer != NULL)
    {
        m_tcpServer->close();
        delete m_tcpServer;
    }
}

QString NetworkManager::hostName() const
{
    return m_hostName;
}

void NetworkManager::setHostName(QString hostName)
{
    if (m_hostName == hostName)
        return;

    m_hostName = hostName;
    emit hostNameChanged(m_hostName);
}

QString NetworkManager::defaultName()
{
    foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
    {
        foreach (QNetworkAddressEntry entry, interface.addressEntries())
        {
            QHostAddress addr = entry.ip();
            if (addr.protocol() != QAbstractSocket::IPv6Protocol && addr != QHostAddress::LocalHost)
            {
                return QString("QLC+_%2").arg(QString::number(addr.toIPv4Address(), 16).toUpper());
            }
        }
    }
    return QString();
}

/*********************************************************************
 * Server
 *********************************************************************/

bool NetworkManager::startServer()
{
    if (serverStarted() == true)
        return false;

    m_udpSocket = new QUdpSocket(this);

    if (m_udpSocket->bind(DEFAULT_UDP_PORT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint) == false)
    {
        qDebug() << Q_FUNC_INFO << "Error in binding UDP socket on" << DEFAULT_UDP_PORT;
        return false;
    }
    qDebug() << "UDP socket opened";

    connect(m_udpSocket, &QUdpSocket::readyRead, this, &NetworkManager::slotProcessUDPPackets);

    m_tcpServer = new QTcpServer(this);

    if (m_tcpServer->listen(QHostAddress::Any, DEFAULT_TCP_PORT) == false)
    {
        qDebug() << Q_FUNC_INFO << "Error listening TCP socket on" << DEFAULT_TCP_PORT;
        return false;
    }
    connect(m_tcpServer, &QTcpServer::newConnection, this, &NetworkManager::slotProcessNewTCPConnection);

    m_hostType = ServerHostType;

    setServerStarted(true);

    return true;
}

bool NetworkManager::stopServer()
{
    if (serverStarted() == false)
        return false;

    disconnect(m_udpSocket, &QUdpSocket::readyRead, this, &NetworkManager::slotProcessUDPPackets);
    disconnect(m_tcpServer, &QTcpServer::newConnection, this, &NetworkManager::slotProcessNewTCPConnection);

    m_udpSocket->close();
    m_tcpServer->close();

    setServerStarted(false);

    delete m_udpSocket;
    delete m_tcpServer;

    m_udpSocket = NULL;
    m_tcpServer = NULL;

    return true;
}

bool NetworkManager::serverStarted() const
{
    return m_serverStarted;
}

void NetworkManager::setServerStarted(bool serverStarted)
{
    if (m_serverStarted == serverStarted)
        return;

    m_serverStarted = serverStarted;
    emit serverStartedChanged(m_serverStarted);
}

/*********************************************************************
 * Client
 *********************************************************************/

bool NetworkManager::initializeClient()
{
    QByteArray packet;

    if (m_udpSocket != NULL)
    {
        m_udpSocket->close();
        delete m_udpSocket;
    }

    m_udpSocket = new QUdpSocket(this);

    if (m_udpSocket->bind(DEFAULT_UDP_PORT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint) == false)
    {
        qDebug() << Q_FUNC_INFO << "Error in binding UDP socket on" << DEFAULT_UDP_PORT;
        return false;
    }
    qDebug() << "UDP socket opened";

    m_hostType = ClientHostType;

    m_serverList.clear();

    /* compose the announce packet */
    m_packetizer->initializePacket(packet, NetAnnounce);
    m_packetizer->addSection(packet, QVariant(m_hostType));
    m_packetizer->addSection(packet, QVariant(m_hostName));

    /* now send the packet on every network interface */
    foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
    {
        foreach (QNetworkAddressEntry entry, interface.addressEntries())
        {
            if (entry.ip().protocol() != QAbstractSocket::IPv6Protocol && entry.ip() != QHostAddress::LocalHost)
            {
                qDebug() << "Sending announcement on" << interface.name();
                m_udpSocket->writeDatagram(packet, entry.broadcast(), DEFAULT_UDP_PORT);
            }
        }
    }

    return true;
}

bool NetworkManager::connectClient(QString ipAddress)
{
    Q_UNUSED(ipAddress);

    return true;
}

bool NetworkManager::disconnectClient()
{
    if (m_udpSocket != NULL)
    {
        m_udpSocket->close();
        delete m_udpSocket;
    }

    return true;
}

QVariant NetworkManager::serverList() const
{
    QVariantList serverList;

    auto i = m_serverList.constBegin();
    while (i != m_serverList.constEnd())
    {
        QVariantMap serverMap;
        serverMap.insert("name", i.value());
        serverMap.insert("address", i.key().toString());
        serverList.append(serverMap);

        ++i;
    }

    return QVariant::fromValue(serverList);
}

bool NetworkManager::clientConnected() const
{
    return m_clientConnected;
}

void NetworkManager::setClientConnected(bool clientConnected)
{
    if (m_clientConnected == clientConnected)
        return;

    m_clientConnected = clientConnected;
    emit clientConnectedChanged(m_clientConnected);
}

void NetworkManager::slotProcessUDPPackets()
{
    qDebug() << "------- slotProcessUDPPackets";

    while (m_udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        QHostAddress senderAddress;
        datagram.resize(m_udpSocket->pendingDatagramSize());
        m_udpSocket->readDatagram(datagram.data(), datagram.size(), &senderAddress);

        qDebug() << "UDP packet received" << datagram.size() << "bytes";

        int opCode = 0;
        QVariantList paramsList;
        int read = m_packetizer->decodePacket(datagram, opCode, paramsList);

        qDebug() << "Bytes processed" << read << opCode << paramsList;

        switch (opCode)
        {
            case NetAnnounce:
            {
                QByteArray packet;
                m_packetizer->initializePacket(packet, NetAnnounceReply);
                m_packetizer->addSection(packet, QVariant(m_hostType));
                m_packetizer->addSection(packet, QVariant(m_hostName));
                m_udpSocket->writeDatagram(packet, senderAddress, DEFAULT_UDP_PORT);
                qDebug() << "Announce reply sent to" << senderAddress.toString();
            }
            break;

            case NetAnnounceReply:
            {
                if (m_hostType == ClientHostType &&
                    paramsList.count() == 2 &&
                    paramsList.at(0).toInt() == ServerHostType)
                {
                    m_serverList[senderAddress] = paramsList.at(1).toString();
                    emit serverListChanged();
                }
            }
            break;

            default:
                qDebug() << "Unsupported opCode" << opCode;
            break;
        }
    }
}

void NetworkManager::slotProcessTCPPackets()
{
    QTcpSocket *socket = (QTcpSocket *)sender();
    if (socket == NULL)
        return;

    QHostAddress senderAddress = socket->peerAddress();
    qint64 bytesAvailable = socket->bytesAvailable();
    QByteArray wholeData;

    wholeData.append(socket->readAll());

    qDebug() << "[TCP] Received" << bytesAvailable << "bytes from" << senderAddress.toString();
}

void NetworkManager::slotProcessNewTCPConnection()
{
    qDebug() << Q_FUNC_INFO;
    QTcpSocket *clientConnection = m_tcpServer->nextPendingConnection();
    if (clientConnection == NULL)
        return;

    QHostAddress senderAddress = clientConnection->peerAddress();
    if (m_hostsMap.contains(senderAddress) == true)
    {
        NetworkHost *host = m_hostsMap[senderAddress];
        host->isAuthenticated = false;
        host->tcpSocket = clientConnection;
    }
    else
    {
        qDebug() << "[slotProcessNewTCPConnection] Adding a new host to map:" << senderAddress.toString();
        NetworkHost *newHost = new NetworkHost;
        newHost->isAuthenticated = false;
        newHost->tcpSocket = clientConnection;
        m_hostsMap[senderAddress] = newHost;
    }
    connect(clientConnection, SIGNAL(readyRead()),
            this, SLOT(slotProcessTCPPackets()));
}

void NetworkManager::slotHostDisconnected()
{
    QTcpSocket *socket = (QTcpSocket *)sender();
    qDebug() << "Host with address" << socket->peerAddress().toString() << "disconnected !";
}
