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

#include <QHostAddress>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QTcpServer>
#include <QThread>
#include <QHash>

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

    Q_PROPERTY(bool serverStarted READ serverStarted WRITE setServerStarted NOTIFY serverStartedChanged)
    Q_PROPERTY(bool clientConnected READ clientConnected WRITE setClientConnected NOTIFY clientConnectedChanged)
    Q_PROPERTY(QString hostName READ hostName WRITE setHostName NOTIFY hostNameChanged)

public:
    explicit NetworkManager(QObject *parent = 0);
    ~NetworkManager();

    enum HostType
    {
        UnknownHostType,
        ServerHostType,
        ClientHostType
    };

    Q_INVOKABLE bool startServer();
    Q_INVOKABLE bool stopServer();

    /** Get/Set the status of a QLC+ server instance */
    bool serverStarted() const;
    void setServerStarted(bool serverStarted);

    Q_INVOKABLE bool initializeClient();
    Q_INVOKABLE bool connectClient(QString ipAddress);
    Q_INVOKABLE bool disconnectClient();

    /** Get/Set the connection status of a QLC+ client instance */
    bool clientConnected() const;
    void setClientConnected(bool clientConnected);

    /** Get/Set the name of the host within the QLC+ network */
    QString hostName() const;
    void setHostName(QString hostName);

protected:
    QString defaultName();

signals:
    void serverStartedChanged(bool serverStarted);
    void clientConnectedChanged(bool clientConnected);
    void hostNameChanged(QString hostName);

protected slots:
    /** Async event raised when UDP packets are received */
    void slotProcessUDPPackets();

    /** Async event raised when unicast packets are received */
    void slotProcessTCPPackets();

    /** Event raised when an incoming connection is requested on
     *  the TCP socket server side */
    void slotProcessNewTCPConnection();

    void slotHostDisconnected();

private:
    /** The interface IP address as QHostAddress */
    QHostAddress m_ipAddr;

    /** The host broadcast address as QHostAddress */
    /** This is where QLC+ broadcast packets are sent to */
    QHostAddress m_broadcastAddr;

    /** The host name in the QLC+ network */
    QString m_hostName;

    /** The type of this host */
    HostType m_hostType;

    /** The UDP socket used to send/receive QLC+ announce packets */
    QUdpSocket *m_udpSocket;

    /** Instance of a TCP server used by a QLC+ server */
    QTcpServer *m_tcpServer;

    /** Interface thread that polls other QLC+ devices
     *  to monitor their alive status */
    QThread m_poller;

    /** Reference to an encryption engine. For now we use SimpleCrypt */
    SimpleCrypt *m_crypt;

    /** Reference to a class in charge of packetize/extract data
     *  according to the QLC+ network protocol */
    NetworkPacketizer* m_packetizer;

    /** Map of the QLC+ hosts detected on the network */
    QHash<QHostAddress, NetworkHost *>m_hostsMap;

    bool m_serverStarted;
    bool m_clientConnected;
};

#endif /* NETWORKMANAGER_H */
