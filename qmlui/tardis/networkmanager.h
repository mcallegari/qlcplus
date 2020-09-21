/*
  Q Light Controller Plus
  networkmanager.h

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

#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#if defined(WIN32) || defined(Q_OS_WIN)
  #include <windows.h>
#endif

#include <QTcpSocket>
#include <QTcpServer>
#include <QUdpSocket>
#include <QThread>
#include <QHash>

#include "tardis.h"

class Doc;
class SimpleCrypt;
class NetworkPacketizer;

typedef struct
{
    /** Flag to recognize a host authenticated to the QLC+ network */
    bool isAuthenticated;
    /** The unique host name in the QLC+ network */
    QString hostName;
    /** The TCP socket for unicast client/server communication */
    QTcpSocket *tcpSocket;
} NetworkHost;

class NetworkManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString hostName READ hostName WRITE setHostName NOTIFY hostNameChanged)
    Q_PROPERTY(bool serverStarted READ serverStarted WRITE setServerStarted NOTIFY serverStartedChanged)
    Q_PROPERTY(QVariant serverList READ serverList NOTIFY serverListChanged)
    Q_PROPERTY(int clientStatus READ clientStatus WRITE setClientStatus NOTIFY clientStatusChanged)
    Q_PROPERTY(int connectionsCount READ connectionsCount NOTIFY connectionsCountChanged)

public:
    explicit NetworkManager(QObject *parent = nullptr, Doc *doc = nullptr);
    ~NetworkManager();

    enum HostType
    {
        UnknownHostType,
        ServerHostType,
        ClientHostType
    };

    /** Get/Set the name of the host within the QLC+ network */
    QString hostName() const;
    void setHostName(QString hostName);

    int connectionsCount();

public slots:
    void sendAction(int code, TardisAction action);

protected:
    QString defaultName();

    /** Send the content of $packet using the provided $socket */
    bool sendTCPPacket(QTcpSocket *socket, QByteArray &packet, bool encrypt);

signals:
    void hostNameChanged(QString hostName);
    void connectionsCountChanged();
    void actionReady(int code, quint32 id, QVariant value);

protected slots:
    /** Async event raised when UDP packets are received */
    void slotProcessUDPPackets();

    /** Async event raised when unicast packets are received */
    void slotProcessTCPPackets();

private:
    /** Reference to the QLC+ Doc */
    Doc *m_doc;

    /** Global flag to enable/disable packets encryption */
    bool m_encryptPackets;

    /** The host name in the QLC+ network */
    QString m_hostName;

    /** The type of this host */
    HostType m_hostType;

    /** The UDP socket used to send/receive QLC+ announce packets */
    QUdpSocket *m_udpSocket;

    /** Reference to an encryption engine. For now we use SimpleCrypt */
    SimpleCrypt *m_crypt;

    /** Reference to a class in charge of packetize/extract data
     *  according to the QLC+ network protocol */
    NetworkPacketizer* m_packetizer;

    /*********************************************************************
     * Server
     *********************************************************************/
public:
    Q_INVOKABLE bool startServer();
    Q_INVOKABLE bool stopServer();

    Q_INVOKABLE bool setClientAccess(QString hostName, bool allow, int accessMask);
    Q_INVOKABLE bool sendWorkspaceToClient(QString hostName, QString filename);

    /** Get/Set the status of a QLC+ server instance */
    bool serverStarted() const;
    void setServerStarted(bool serverStarted);

protected:
    QHostAddress getHostFromName(QString name);

signals:
    void serverStartedChanged(bool serverStarted);
    void clientAccessRequest(QString hostName);

protected slots:
    /** Event raised when an incoming connection is requested on
     *  the TCP socket server side */
    void slotProcessNewTCPConnection();
    void slotHostDisconnected();

private:
    /** Instance of a TCP server used by a QLC+ server */
    QTcpServer *m_tcpServer;

    /** Flag that indicates if a server instance is running */
    bool m_serverStarted;

    /** Map of the QLC+ hosts detected on the network */
    QHash<QHostAddress, NetworkHost *> m_hostsMap;

    /*********************************************************************
     * Client
     *********************************************************************/
public:
    enum ConnectionStatus
    {
        Disconnected,
        WaitAuthentication,
        DownloadingProject,
        Connected
    };
    Q_ENUM(ConnectionStatus)

    Q_INVOKABLE bool initializeClient();
    Q_INVOKABLE bool connectClient(QString ipAddress);
    Q_INVOKABLE bool disconnectClient();

    QVariant serverList() const;

    /** Get/Set the connection status of a QLC+ client instance */
    int clientStatus() const;
    void setClientStatus(int clientStatus);

signals:
    void clientStatusChanged(bool clientStatus);
    void serverListChanged();
    void accessMaskChanged(int mask);
    void requestProjectLoad(QByteArray &data);

private:
    /** The socket used to send/receive unicast TCP packets */
    QTcpSocket *m_tcpSocket;

    /** Map used during automatic server discovery */
    QHash<QHostAddress, QString> m_serverList;

    /** The client connection status */
    int m_clientStatus;

    /** Project transfer variables */
    QByteArray m_projectData;
    int m_projectSize;
};

#endif /* NETWORKMANAGER_H */
