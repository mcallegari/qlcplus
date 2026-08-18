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
#include <QCryptographicHash>
#include <QSettings>
#include <QFile>
#include <QDateTime>

#include "networkmanager.h"
#include "networkpacketizer.h"
#include "simplecrypt.h"
#include "tardis.h"
#include "doc.h"
#include "inputoutputmap.h"
#include "webaccess-qml.h"

#define DEFAULT_UDP_PORT    9997
#define DEFAULT_TCP_PORT    9998

#define WORKSPACE_CHUNK_SIZE    8 * 1024
#define ECHO_GUARD_WINDOW_MS    15000

#define SETTINGS_SERVER_KEY     QStringLiteral("network/serverkey")

static const quint64 defaultKey = 0x5131632B4E33744B; // this is "Q1c+N3tK"

NetworkManager::NetworkManager(QObject *parent, Doc *doc, VirtualConsole *vc, SimpleDesk *sd)
    : QObject(parent)
    , m_doc(doc)
    , m_virtualConsole(vc)
    , m_simpleDesk(sd)
    , m_encryptPackets(true)
    , m_serverType(NativeServer)
    , m_startAutomatically(false)
    , m_udpSocket(nullptr)
    , m_webAccess(nullptr)
    , m_webServerPort(0)
    , m_webServerAuth(false)
    , m_webServerPasswordFile(QString())
    , m_tcpServer(nullptr)
    , m_serverStartedMask(NoServer)
    , m_tcpSocket(nullptr)
    , m_clientStatus(Disconnected)
{
    m_hostType = UnknownHostType;
    m_crypt = new SimpleCrypt(defaultKey);
    m_packetizer = new NetworkPacketizer();

    // restore the encryption key saved in the global QLC+ settings
    QSettings settings;
    QVariant storedKey = settings.value(SETTINGS_SERVER_KEY);
    if (storedKey.isValid())
    {
        SimpleCrypt masterCrypt(defaultKey);
        m_serverPassword = masterCrypt.decryptToString(storedKey.toString());
    }
    applyEncryptionKey();

    if (m_doc != nullptr)
    {
        connect(m_doc, SIGNAL(loaded()), this, SLOT(slotDocLoaded()));
        slotDocLoaded();
    }
    else
    {
        setHostName(defaultName());
    }
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
    if (m_doc != nullptr && m_doc->inputOutputMap() != nullptr)
        m_doc->inputOutputMap()->setNetworkServerName(hostName);
    emit hostNameChanged(m_hostName);
}

int NetworkManager::serverType() const
{
    return m_serverType;
}

void NetworkManager::setServerType(int typeMask)
{
    typeMask = (typeMask & (NativeServer | WebServer)) | m_forcedServerTypes;

    if (m_serverType == typeMask)
        return;

    // stop the servers that are being disabled
    int toStop = m_serverType & ~typeMask;
    if (toStop & NativeServer)
        stopServerType(NativeServer);
    if (toStop & WebServer)
        stopServerType(WebServer);

    m_serverType = typeMask;
    if (m_doc != nullptr && m_doc->inputOutputMap() != nullptr)
    {
        int ioMask = InputOutputMap::NoServer;
        if (typeMask & NativeServer)
            ioMask |= InputOutputMap::NativeServer;
        if (typeMask & WebServer)
            ioMask |= InputOutputMap::WebServer;
        m_doc->inputOutputMap()->setNetworkServerType(ioMask);
    }

    emit serverTypeChanged(m_serverType);
}

void NetworkManager::enableServerType(int type, bool enable)
{
    if (type != NativeServer && type != WebServer)
        return;

    // if some server is already running, follow the user
    // intention and start/stop the requested type as well
    bool applyToRuntime = serverStarted();

    setServerType(enable ? (m_serverType | type) : (m_serverType & ~type));

    if (applyToRuntime && enable && (m_serverType & type))
        startServerType(type);
}

bool NetworkManager::toggleServerType(int type)
{
    if (type != NativeServer && type != WebServer)
        return false;

    // stopping: disabling the type stops it as well, so that
    // an autostart won't bring it up again on the next load
    if (m_serverStartedMask & type)
    {
        setServerType(m_serverType & ~type);
        return false;
    }

    // starting: make sure the type is enabled before running it
    setServerType(m_serverType | type);

    return startServerType(type);
}

bool NetworkManager::startAutomatically() const
{
    return m_startAutomatically;
}

void NetworkManager::setStartAutomatically(bool startAutomatically)
{
    if (m_startAutomatically == startAutomatically)
        return;

    m_startAutomatically = startAutomatically;
    if (m_doc != nullptr && m_doc->inputOutputMap() != nullptr)
        m_doc->inputOutputMap()->setNetworkServerAutoStart(startAutomatically);
    emit startAutomaticallyChanged(m_startAutomatically);
}

QString NetworkManager::serverPassword() const
{
    return m_serverPassword;
}

void NetworkManager::setServerPassword(QString password)
{
    if (m_serverPassword == password)
        return;

    m_serverPassword = password;
    emit serverPasswordChanged(m_serverPassword);
}

quint64 NetworkManager::sessionKey() const
{
    // an empty key means "no custom key": fall back to
    // the QLC+ master key, so that a default setup still works
    if (m_serverPassword.isEmpty())
        return defaultKey;

    // fold the user key into the 64 bits SimpleCrypt expects. A hash is
    // used so that keys sharing a prefix don't end up being equivalent
    QByteArray digest = QCryptographicHash::hash(m_serverPassword.toUtf8(),
                                                 QCryptographicHash::Sha256);
    quint64 key = 0;
    for (int i = 0; i < 8; i++)
        key = (key << 8) | quint8(digest.at(i));

    return key;
}

void NetworkManager::applyEncryptionKey()
{
    m_crypt->setKey(sessionKey());
}

bool NetworkManager::saveEncryptionKey(QString key)
{
    setServerPassword(key);

    // store the key in the global QLC+ settings, encrypted with the master key
    QSettings settings;
    if (key.isEmpty())
    {
        settings.remove(SETTINGS_SERVER_KEY);
    }
    else
    {
        SimpleCrypt masterCrypt(defaultKey);
        settings.setValue(SETTINGS_SERVER_KEY, masterCrypt.encryptToString(key));
    }

    // the key is the shared secret of the whole session: restart
    // the running servers so peers negotiate with the new one
    int runningMask = m_serverStartedMask;

    if (runningMask & NativeServer)
        stopServerType(NativeServer);
    if (runningMask & WebServer)
        stopServerType(WebServer);

    applyEncryptionKey();

    if (runningMask & NativeServer)
        startServerType(NativeServer);
    if (runningMask & WebServer)
        startServerType(WebServer);

    return true;
}

void NetworkManager::setWebServerConfiguration(int portNumber, bool enableAuth, const QString &passwordFile)
{
    m_webServerPort = portNumber;
    m_webServerAuth = enableAuth;
    m_webServerPasswordFile = passwordFile;
}

void NetworkManager::setForcedServerTypes(int typeMask)
{
    m_forcedServerTypes = typeMask & (NativeServer | WebServer);

    // the command line defines the enabled types exactly: only the
    // requested ones must run, no matter what the default or the
    // workspace settings ask for
    if (m_forcedServerTypes != NoServer)
        setServerType(m_forcedServerTypes);
}

int NetworkManager::connectionsCount() const
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
            if (host != nullptr && host->tcpSocket == m_currentRxSocket)
            {
                ++i;
                continue;
            }
            if (host != nullptr && shouldSkipEcho(host->tcpSocket, code, action.m_objID))
            {
                ++i;
                continue;
            }
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

quint64 NetworkManager::actionKey(int code, quint32 objID) const
{
    return (quint64(quint32(code)) << 32) | quint64(objID);
}

void NetworkManager::markActionSource(int code, quint32 objID, QTcpSocket *socket)
{
    if (socket == nullptr)
        return;

    socket->setProperty("lastActionTs", QDateTime::currentMSecsSinceEpoch());
    m_recentActionSources[actionKey(code, objID)] = socket;
}

bool NetworkManager::shouldSkipEcho(const QTcpSocket *socket, int code, quint32 objID)
{
    if (socket == nullptr)
        return false;

    quint64 key = actionKey(code, objID);
    if (!m_recentActionSources.contains(key))
        return false;

    QPointer<QTcpSocket> sourceSocket = m_recentActionSources.value(key);
    if (sourceSocket.isNull())
    {
        m_recentActionSources.remove(key);
        return false;
    }

    if (sourceSocket != socket)
        return false;

    qint64 ts = sourceSocket->property("lastActionTs").toLongLong();
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (ts <= 0 || now - ts > ECHO_GUARD_WINDOW_MS)
    {
        m_recentActionSources.remove(key);
        return false;
    }

    // One-shot suppression: only the first delayed self-echo is discarded.
    m_recentActionSources.remove(key);
    return true;
}

/*********************************************************************
 * Server
 *********************************************************************/

bool NetworkManager::startServer()
{
    bool result = false;

    if (m_serverType & NativeServer)
        result |= startServerType(NativeServer);

    if (m_serverType & WebServer)
        result |= startServerType(WebServer);

    return result;
}

bool NetworkManager::stopServer()
{
    bool result = false;

    if (m_serverStartedMask & NativeServer)
        result |= stopServerType(NativeServer);

    if (m_serverStartedMask & WebServer)
        result |= stopServerType(WebServer);

    return result;
}

bool NetworkManager::startServerType(int type)
{
    if (m_serverStartedMask & type)
        return false;

    if (type == WebServer)
    {
        if (m_doc == nullptr || m_virtualConsole == nullptr || m_simpleDesk == nullptr)
            return false;

        m_webAccess = new WebAccessQml(m_doc, m_virtualConsole, m_simpleDesk,
                                       m_webServerPort, m_webServerAuth, m_webServerPasswordFile, this);
        connect(m_webAccess, &WebAccessQml::loadProject, this,
                [this](const QByteArray &xmlData)
        {
            QByteArray xmlCopy = xmlData;
            emit requestProjectLoad(xmlCopy);
        });
        connect(m_webAccess, &WebAccessQml::storeAutostartProject,
                this, &NetworkManager::storeAutostartProject);
        setServerStartedMask(m_serverStartedMask | WebServer);
        return true;
    }

    if (type != NativeServer)
        return false;

    m_udpSocket = new QUdpSocket(this);

    if (m_udpSocket->bind(DEFAULT_UDP_PORT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint) == false)
    {
        qDebug() << Q_FUNC_INFO << "Error in binding UDP socket on" << DEFAULT_UDP_PORT;
        delete m_udpSocket;
        m_udpSocket = nullptr;
        return false;
    }
    qDebug() << "UDP socket opened";

    connect(m_udpSocket, &QUdpSocket::readyRead, this, &NetworkManager::slotProcessUDPPackets);

    m_tcpServer = new QTcpServer(this);

    if (m_tcpServer->listen(QHostAddress::Any, DEFAULT_TCP_PORT) == false)
    {
        qDebug() << Q_FUNC_INFO << "Error listening TCP socket on" << DEFAULT_TCP_PORT;
        disconnect(m_udpSocket, &QUdpSocket::readyRead, this, &NetworkManager::slotProcessUDPPackets);
        m_udpSocket->close();
        delete m_udpSocket;
        m_udpSocket = nullptr;
        delete m_tcpServer;
        m_tcpServer = nullptr;
        return false;
    }
    connect(m_tcpServer, &QTcpServer::newConnection, this, &NetworkManager::slotProcessNewTCPConnection);

    m_hostType = ServerHostType;

    setServerStartedMask(m_serverStartedMask | NativeServer);

    return true;
}

bool NetworkManager::stopServerType(int type)
{
    if ((m_serverStartedMask & type) == 0)
        return false;

    if (type == WebServer)
    {
        if (m_webAccess != nullptr)
        {
            m_webAccess->closeServer();
            m_webAccess->deleteLater();
            m_webAccess = nullptr;
        }
        setServerStartedMask(m_serverStartedMask & ~WebServer);
        return true;
    }

    if (type != NativeServer)
        return false;

    disconnect(m_udpSocket, &QUdpSocket::readyRead, this, &NetworkManager::slotProcessUDPPackets);
    disconnect(m_tcpServer, &QTcpServer::newConnection, this, &NetworkManager::slotProcessNewTCPConnection);

    m_udpSocket->close();
    m_tcpServer->close();

    if (m_hostType == ServerHostType)
        m_hostType = UnknownHostType;

    setServerStartedMask(m_serverStartedMask & ~NativeServer);

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
    return m_serverStartedMask != NoServer;
}

bool NetworkManager::nativeServerStarted() const
{
    return (m_serverStartedMask & NativeServer) ? true : false;
}

bool NetworkManager::webServerStarted() const
{
    return (m_serverStartedMask & WebServer) ? true : false;
}

void NetworkManager::setServerStartedMask(int mask)
{
    if (m_serverStartedMask == mask)
        return;

    m_serverStartedMask = mask;
    emit serverStartedChanged(serverStarted());
    emit connectionsCountChanged();
}

QHostAddress NetworkManager::getHostFromName(QString name) const
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
    m_packetizer->addSection(packet, QVariant(QString::number(sessionKey(), 16).toUtf8()));
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

    /* Append to whatever was left over from the previous reads on this
     * socket: a single packet may well be split across several of them */
    QByteArray &wholeData = m_rxBuffers[socket];
    wholeData.append(socket->readAll());

    qDebug() << "[TCP] Received" << wholeData.length() << "bytes from" << senderAddress.toString();

    while (bytesProcessed < wholeData.length())
    {
        int actionCode = 0;
        QVariantList paramsList;
        QByteArray datagram = wholeData.mid(bytesProcessed);
        int read = m_packetizer->decodePacket(datagram, actionCode, paramsList, m_crypt);

        qDebug() << "Bytes processed" << read << "action" << QString::number(actionCode, 16) << "params" << paramsList.count();

        /* Incomplete packet: keep the remainder and wait for the next read */
        if (read < 0)
            break;

        /* Undecodable data: there's no way to resync, drop the whole buffer */
        if (read == 0)
        {
            bytesProcessed = wholeData.length();
            break;
        }

        switch (actionCode)
        {
            case Tardis::NetAuthentication:
            {
                bool success = false;

                if (!paramsList.isEmpty())
                {
                    QByteArray decrPayload = paramsList.at(0).toByteArray();
                    if (QString::fromUtf8(decrPayload) == QString::number(sessionKey(), 16))
                    {
                        qDebug() << "Key matches!";
                        success = true;
                    }
                }

                NetworkHost *host = m_hostsMap[senderAddress];
                if (success == true && paramsList.count() > 1)
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
                    /* Never load a partial project: doing so silently drops
                     * whole sections (Monitor, Virtual Console, ...) and looks
                     * like data corruption to the user */
                    if (m_projectData.length() != m_projectSize)
                    {
                        qWarning() << "[TCP] Incomplete project transfer:"
                                   << m_projectData.length() << "of" << m_projectSize
                                   << "bytes received. Project discarded";
                        m_projectData.clear();
                        setClientStatus(Connected);
                        emit connectionsCountChanged();
                        break;
                    }

                    emit requestProjectLoad(m_projectData);
                    m_projectData.clear();
                    setClientStatus(Connected);
                    emit connectionsCountChanged();
                }
            }
            break;

            default:
            {
                if (paramsList.isEmpty())
                {
                    qWarning() << "[TCP] Dropping packet with empty params list. action"
                               << QString::number(actionCode, 16);
                    break;
                }

                markActionSource(actionCode, paramsList.at(0).toUInt(), socket);

                QTcpSocket *prevRxSocket = m_currentRxSocket;
                m_currentRxSocket = socket;
                if (paramsList.count() == 2)
                    emit actionReady(actionCode, paramsList.at(0).toUInt(), paramsList.at(1));
                else if (paramsList.count() > 2)
                    emit actionReady(actionCode, paramsList.at(0).toUInt(), paramsList.mid(1));
                else
                    emit actionReady(actionCode, paramsList.at(0).toUInt(), QVariant());
                m_currentRxSocket = prevRxSocket;

                //qDebug() << "Unsupported opCode" << opCode;
            }
            break;
        }

        bytesProcessed += read;
    }

    /* Drop what has been consumed, keep any partial packet for the next read */
    if (bytesProcessed > 0)
        wholeData.remove(0, bytesProcessed);
}

void NetworkManager::slotDocLoaded()
{
    if (m_doc == nullptr || m_doc->inputOutputMap() == nullptr)
        return;

    InputOutputMap *ioMap = m_doc->inputOutputMap();
    int typeMask = NoServer;

    if (ioMap->networkServerType() & InputOutputMap::NativeServer)
        typeMask |= NativeServer;
    if (ioMap->networkServerType() & InputOutputMap::WebServer)
        typeMask |= WebServer;

    // the types requested from the command line always win: when they
    // are set they replace the workspace ones, so that a project can't
    // bring up a server that was not asked for
    setServerType(m_forcedServerTypes != NoServer ? m_forcedServerTypes : typeMask);
    setStartAutomatically(ioMap->networkServerAutoStart());
    // NOTE: the encryption key is deliberately not taken from the project.
    // It is a machine-wide setting stored in the global QLC+ configuration

    QString name = ioMap->networkServerName();
    if (name.isEmpty())
        name = defaultName();
    setHostName(name);

    if (m_hostType != ClientHostType &&
        (m_startAutomatically || m_forcedServerTypes != NoServer))
        startServer();
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
    connect(clientConnection, &QTcpSocket::disconnected,
            this, &NetworkManager::slotHostDisconnected);
}

void NetworkManager::slotHostDisconnected()
{
    QTcpSocket *socket = (QTcpSocket *)sender();
    QHostAddress senderAddress = socket->peerAddress();
    qDebug() << "Host with address" << senderAddress.toString() << "disconnected!";

    m_rxBuffers.remove(socket);

    if (m_hostsMap.contains(senderAddress) == true)
    {
        NetworkHost *host = m_hostsMap.take(senderAddress);
        delete host;
        emit connectionsCountChanged();
    }
}
