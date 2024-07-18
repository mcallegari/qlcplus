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

#include <QXmlStreamWriter>
#include <QNetworkInterface>
#include <QtCore/qbuffer.h>
#include <QFile>

#include "networkmanager.h"
#include "networkpacketizer.h"
#include "simplecrypt.h"
#include "tardis.h"
#include "doc.h"

#define DEFAULT_UDP_PORT    9997
#define DEFAULT_TCP_PORT    9998

#define WORKSPACE_CHUNK_SIZE    8 * 1024

static const quint64 defaultKey = 0x5131632B4E33744B; // this is "Q1c+N3tK"

NetworkManager::NetworkManager(QObject *parent, Doc *doc)
    : QObject(parent)
    , m_doc(doc)
    , m_encryptPackets(true)
    , m_udpSocket(nullptr)
    , m_tcpServer(nullptr)
    , m_serverStarted(false)
    , m_tcpSocket(nullptr)
    , m_clientStatus(Disconnected)
{
    m_hostType = UnknownHostType;
    setHostName(defaultName());
    m_crypt = new SimpleCrypt(defaultKey);
    m_packetizer = new NetworkPacketizer();
}

NetworkManager::~NetworkManager()
{
    stopServer();

    if (m_udpSocket != nullptr)
    {
        m_udpSocket->close();
        delete m_udpSocket;
    }

    if (m_tcpServer != nullptr)
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

int NetworkManager::connectionsCount()
{
    if (m_hostType == ServerHostType)
        return m_hostsMap.count();
    else if (m_hostType == ClientHostType)
        return m_clientStatus == Connected ? 1 : 0;

    return 0;
}

void NetworkManager::sendAction(int code, TardisAction action)
{
    QByteArray packet;
    m_packetizer->initializePacket(packet, code);
    m_packetizer->addSection(packet, action.m_objID);

    switch (action.m_action)
    {
        case Tardis::FixtureCreate:
        case Tardis::FixtureGroupCreate:
        case Tardis::FunctionCreate:
        case Tardis::ChaserAddStep:
        case Tardis::EFXAddFixture:
        case Tardis::VCWidgetCreate:
            m_packetizer->addSection(packet, action.m_newValue);
        break;

        case Tardis::FixtureDelete:
        case Tardis::FixtureGroupDelete:
        case Tardis::FunctionDelete:
        case Tardis::VCWidgetDelete:
            m_packetizer->addSection(packet, action.m_oldValue);
        break;

        default:
            m_packetizer->addSection(packet, action.m_newValue);
        break;
    }

    if (m_hostType == ServerHostType)
    {
        /* Send packet to all connected clients */
        auto i = m_hostsMap.constBegin();
        while (i != m_hostsMap.constEnd())
        {
            NetworkHost *host = i.value();
            sendTCPPacket(host->tcpSocket, packet, m_encryptPackets);
            ++i;
        }
    }
    else
    {
        /* Send packet to the connected server */
        sendTCPPacket(m_tcpSocket, packet, m_encryptPackets);
    }
}

QString NetworkManager::defaultName()
{
    for (QNetworkInterface iface : QNetworkInterface::allInterfaces())
    {
        for (QNetworkAddressEntry entry : iface.addressEntries())
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

bool NetworkManager::sendTCPPacket(QTcpSocket *socket, QByteArray &packet, bool encrypt)
{
    if (socket == nullptr)
        return false;

    qint64 sent = 0;
    quint64 totalBytesSent = 0;

    if (encrypt)
    {
        QByteArray encPacket = m_packetizer->encryptPacket(packet, m_crypt);
        while (totalBytesSent < (quint64)encPacket.length())
        {
            sent = socket->write(encPacket.data() + totalBytesSent, encPacket.length() - totalBytesSent);
            totalBytesSent += sent;
            if (sent < 0)
                break;
        }
    }
    else
    {
        while (totalBytesSent < (quint64)packet.length())
        {
            sent = socket->write(packet.data() + totalBytesSent, packet.length() - totalBytesSent);
            totalBytesSent += sent;
            if (sent < 0)
                break;
        }
    }

    if (sent < 0)
    {
        qDebug() << "Unable to send packet";
        qDebug() << "Error number:" << socket->error();
        qDebug() << "Socket state:" << socket->state();
        qDebug() << "Error message:" << socket->errorString();

        if (socket->state() == QAbstractSocket::UnconnectedState)
        {
            // remove this host from the connected hosts map
            qDebug() << "Host disconnected";
            socket->close();
            delete socket;
            socket = nullptr;
            return false;
        }
    }

    return true;
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

    m_udpSocket = nullptr;
    m_tcpServer = nullptr;

    return true;
}

bool NetworkManager::setClientAccess(QString hostName, bool allow, int accessMask)
{
    QHostAddress clientAddress = getHostFromName(hostName);
    NetworkHost *host = m_hostsMap.value(clientAddress, nullptr);

    if (host == nullptr || clientAddress.isNull())
        return false;

    if (!allow)
        host->isAuthenticated = false;

    QByteArray reply;
    m_packetizer->initializePacket(reply, Tardis::NetAuthenticationReply);

    if (allow)
    {
        m_packetizer->addSection(reply, QVariant("Success"));
        m_packetizer->addSection(reply, QVariant(accessMask));
    }
    else
    {
        m_packetizer->addSection(reply, QVariant("Failed"));
    }

    sendTCPPacket(host->tcpSocket, reply, m_encryptPackets);

    return true;
}

bool NetworkManager::sendWorkspaceToClient(QString hostName, QString filename)
{
    QByteArray packet;
    int pktCounter = 0;
    QFile workspace(filename);
    QHostAddress clientAddress = getHostFromName(hostName);
    NetworkHost *host = m_hostsMap.value(clientAddress, nullptr);

    if (host == nullptr || clientAddress.isNull())
        return false;

    if (workspace.exists() == false)
    {
        m_packetizer->initializePacket(packet, Tardis::NetProjectTransfer);
        m_packetizer->addSection(packet, QVariant(0));
        m_packetizer->addSection(packet, QVariant(0));
        sendTCPPacket(host->tcpSocket, packet, m_encryptPackets);
        return false;
    }

    if (!workspace.open(QIODevice::ReadOnly))
        return false;

    while (!workspace.atEnd())
    {
        QByteArray data = workspace.read(WORKSPACE_CHUNK_SIZE);
        m_packetizer->initializePacket(packet, Tardis::NetProjectTransfer);

        qDebug() << "Data read:" << data.length();

        if (pktCounter == 0)
        {
            m_packetizer->addSection(packet, QVariant(0));
            m_packetizer->addSection(packet, QVariant((int)workspace.size()));

        }
        else if (data.length() < WORKSPACE_CHUNK_SIZE)
        {
            m_packetizer->addSection(packet, QVariant(2));
        }
        else
        {
            m_packetizer->addSection(packet, QVariant(1));
        }

        m_packetizer->addSection(packet, QVariant(data));

        sendTCPPacket(host->tcpSocket, packet, m_encryptPackets);

        pktCounter++;
    }

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

QHostAddress NetworkManager::getHostFromName(QString name)
{
    auto i = m_hostsMap.constBegin();
    while (i != m_hostsMap.constEnd())
    {
        NetworkHost *host = i.value();
        if (host->hostName == name)
            return i.key();

        ++i;
    }

    return QHostAddress();
}

/*********************************************************************
 * Client
 *********************************************************************/

bool NetworkManager::initializeClient()
{
    QByteArray packet;

    if (m_udpSocket != nullptr)
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
    m_packetizer->initializePacket(packet, Tardis::NetAnnounce);
    m_packetizer->addSection(packet, QVariant(m_hostType));
    m_packetizer->addSection(packet, QVariant(m_hostName));

    /* now send the packet on every network interface */
    foreach (QNetworkInterface iface, QNetworkInterface::allInterfaces())
    {
        foreach (QNetworkAddressEntry entry, iface.addressEntries())
        {
            if (entry.ip().protocol() != QAbstractSocket::IPv6Protocol && entry.ip() != QHostAddress::LocalHost)
            {
                qDebug() << "Sending announcement on" << iface.name();
                m_udpSocket->writeDatagram(packet, entry.broadcast(), DEFAULT_UDP_PORT);
            }
        }
    }

    connect(m_udpSocket, &QUdpSocket::readyRead, this, &NetworkManager::slotProcessUDPPackets);

    return true;
}

bool NetworkManager::connectClient(QString ipAddress)
{
    QHostAddress serverAddr(ipAddress);

    if (m_tcpSocket != nullptr)
    {
        m_tcpSocket->close();
        delete m_tcpSocket;
    }

    m_tcpSocket = new QTcpSocket();
    m_tcpSocket->connectToHost(serverAddr, DEFAULT_TCP_PORT);

    if (m_tcpSocket->waitForConnected(10000) == false)
    {
        qDebug() << "Error in connecting to TCP host:" << ipAddress;
        delete m_tcpSocket;
        m_tcpSocket = nullptr;
        return false;
    }
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &NetworkManager::slotProcessTCPPackets);
    connect(m_tcpSocket, &QTcpSocket::disconnected, this, &NetworkManager::slotHostDisconnected);

    QByteArray packet;
    m_packetizer->initializePacket(packet, Tardis::NetAuthentication);
    m_packetizer->addSection(packet, QVariant(QString::number(defaultKey, 16).toUtf8()));
    m_packetizer->addSection(packet, QVariant(hostName()));

    setClientStatus(WaitAuthentication);

    return sendTCPPacket(m_tcpSocket, packet, m_encryptPackets);
}

bool NetworkManager::disconnectClient()
{
    if (m_udpSocket != nullptr)
    {
        m_udpSocket->close();
        delete m_udpSocket;
    }

    setClientStatus(Disconnected);

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
        serverMap.insert("address", QHostAddress(i.key().toIPv4Address()).toString());
        serverList.append(serverMap);

        ++i;
    }

    return QVariant::fromValue(serverList);
}

int NetworkManager::clientStatus() const
{
    return m_clientStatus;
}

void NetworkManager::setClientStatus(int clientStatus)
{
    if (m_clientStatus == clientStatus)
        return;

    m_clientStatus = clientStatus;
    emit clientStatusChanged(m_clientStatus);
}

void NetworkManager::slotProcessUDPPackets()
{
    while (m_udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        QHostAddress senderAddress;
        datagram.resize(m_udpSocket->pendingDatagramSize());
        m_udpSocket->readDatagram(datagram.data(), datagram.size(), &senderAddress);

        qDebug() << "[UDP] received" << datagram.size() << "bytes from" << senderAddress.toString();

        int opCode = 0;
        QVariantList paramsList;
        int read = m_packetizer->decodePacket(datagram, opCode, paramsList, nullptr);

        qDebug() << "Bytes processed" << read << QString::number(opCode, 16) << paramsList;

        switch (opCode)
        {
            case Tardis::NetAnnounce:
            {
                QByteArray packet;
                m_packetizer->initializePacket(packet, Tardis::NetAnnounceReply);
                m_packetizer->addSection(packet, QVariant(m_hostType));
                m_packetizer->addSection(packet, QVariant(m_hostName));
                m_udpSocket->writeDatagram(packet, senderAddress, DEFAULT_UDP_PORT);
                qDebug() << "Announce reply sent to" << senderAddress.toString();
            }
            break;

            case Tardis::NetAnnounceReply:
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
    if (socket == nullptr)
        return;

    QHostAddress senderAddress = socket->peerAddress();
    qint64 bytesProcessed = 0;
    qint64 bytesAvailable = 0;
    QByteArray wholeData;

    wholeData.append(socket->readAll());
    bytesAvailable = wholeData.length();

    qDebug() << "[TCP] Received" << bytesAvailable << "bytes from" << senderAddress.toString();

    while (bytesAvailable)
    {
        int actionCode = 0;
        QVariantList paramsList;
        QByteArray datagram = wholeData.mid(bytesProcessed);
        int read = m_packetizer->decodePacket(datagram, actionCode, paramsList, m_crypt);

        qDebug() << "Bytes processed" << read << "action" << QString::number(actionCode, 16) << "params" << paramsList.count();

        if (read < 0)
        {
            /* if more data is needed, get it from the socket */
            QByteArray moreData = socket->readAll();
            if (moreData.length() == 0)
                return;
            wholeData.append(moreData);
            bytesAvailable = wholeData.length();
            continue;
        }

        if (read == 0)
            break;

        switch (actionCode)
        {
            case Tardis::NetAuthentication:
            {
                bool success = false;

                if (!paramsList.isEmpty())
                {
                    QByteArray decrPayload = paramsList.at(0).toByteArray();
                    if (QString::fromUtf8(decrPayload) == QString::number(defaultKey, 16))
                    {
                        qDebug() << "Key matches!";
                        success = true;
                    }
                }

                NetworkHost *host = m_hostsMap[senderAddress];
                if (success == true)
                {
                    host->isAuthenticated = true;
                    host->hostName = paramsList.at(1).toString();
                    // emit a signal to acquire the host permissions
                    emit clientAccessRequest(host->hostName);
                }
                else
                {
                    host->isAuthenticated = false;
                    QByteArray reply;
                    m_packetizer->initializePacket(reply, Tardis::NetAuthenticationReply);
                    m_packetizer->addSection(reply, QVariant("Failed"));
                    sendTCPPacket(host->tcpSocket, reply, m_encryptPackets);
                }
            }
            break;
            case Tardis::NetAuthenticationReply:
            {
                if (!paramsList.isEmpty() && paramsList.at(0).toString() == "Success")
                {
                    setClientStatus(DownloadingProject);
                    if (paramsList.count() > 1)
                        emit accessMaskChanged(paramsList.at(1).toInt());
                }
                else
                {
                    disconnectClient();
                }
            }
            break;
            case Tardis::NetProjectTransfer:
            {
                if (m_hostType != ClientHostType || paramsList.count() < 2)
                    break;

                int seqType = paramsList.at(0).toInt();

                if (seqType == 0)
                {
                    m_projectSize = paramsList.at(1).toInt();
                    if (m_projectSize == 0)
                    {
                        setClientStatus(Connected);
                        emit connectionsCountChanged();
                        break;
                    }
                    m_projectData.clear();
                    m_projectData.append(paramsList.at(2).toByteArray());
                }
                else
                    m_projectData.append(paramsList.at(1).toByteArray());

                qDebug() << "Project progress:" << m_projectData.length() << "of" << m_projectSize;

                if (seqType == 2 || m_projectData.length() == m_projectSize)
                {
                    emit requestProjectLoad(m_projectData);
                    m_projectData.clear();
                    setClientStatus(Connected);
                    emit connectionsCountChanged();
                }
            }
            break;

            default:
            {
                if (paramsList.count() == 2)
                    emit actionReady(actionCode, paramsList.at(0).toUInt(), paramsList.at(1));
                else
                    emit actionReady(actionCode, paramsList.at(0).toUInt(), QVariant());

                //qDebug() << "Unsupported opCode" << opCode;
            }
            break;
        }

        bytesProcessed += read;
        bytesAvailable -= read;
    }
}

void NetworkManager::slotProcessNewTCPConnection()
{
    qDebug() << Q_FUNC_INFO;
    QTcpSocket *clientConnection = m_tcpServer->nextPendingConnection();
    if (clientConnection == nullptr)
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
        emit connectionsCountChanged();
    }
    connect(clientConnection, SIGNAL(readyRead()),
            this, SLOT(slotProcessTCPPackets()));
}

void NetworkManager::slotHostDisconnected()
{
    QTcpSocket *socket = (QTcpSocket *)sender();
    QHostAddress senderAddress = socket->peerAddress();
    qDebug() << "Host with address" << senderAddress.toString() << "disconnected!";

    if (m_hostsMap.contains(senderAddress) == true)
    {
        NetworkHost *host = m_hostsMap.take(senderAddress);
        delete host;
        emit connectionsCountChanged();
    }
}
