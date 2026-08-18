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
#include <QPointer>

#include "tardis.h"

class Doc;
class SimpleCrypt;
class NetworkPacketizer;
class VirtualConsole;
class SimpleDesk;
class WebAccessQml;

typedef struct
{
    /** Flag to recognize a host authenticated to the QLC+ network */
    bool isAuthenticated;
    /** The unique host name in the QLC+ network */
    QString hostName;
    /** The TCP socket for unicast client/server communication */
    QTcpSocket *tcpSocket;
} NetworkHost;

class NetworkManager final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString hostName READ hostName WRITE setHostName NOTIFY hostNameChanged)
    Q_PROPERTY(bool serverStarted READ serverStarted NOTIFY serverStartedChanged)
    Q_PROPERTY(bool nativeServerStarted READ nativeServerStarted NOTIFY serverStartedChanged)
    Q_PROPERTY(bool webServerStarted READ webServerStarted NOTIFY serverStartedChanged)
    Q_PROPERTY(int serverType READ serverType WRITE setServerType NOTIFY serverTypeChanged)
    Q_PROPERTY(bool startAutomatically READ startAutomatically WRITE setStartAutomatically NOTIFY startAutomaticallyChanged)
    Q_PROPERTY(QString serverPassword READ serverPassword WRITE setServerPassword NOTIFY serverPasswordChanged)
    Q_PROPERTY(QVariant serverList READ serverList NOTIFY serverListChanged)
    Q_PROPERTY(int clientStatus READ clientStatus WRITE setClientStatus NOTIFY clientStatusChanged)
    Q_PROPERTY(int connectionsCount READ connectionsCount NOTIFY connectionsCountChanged)

public:
    explicit NetworkManager(QObject *parent = nullptr, Doc *doc = nullptr,
                            VirtualConsole *vc = nullptr, SimpleDesk *sd = nullptr);
    ~NetworkManager();

    enum HostType
    {
        UnknownHostType,
        ServerHostType,
        ClientHostType
    };

    /** Types of server that can be enabled. These are flags,
     *  so both servers can run at the same time */
    enum ServerType
    {
        NoServer     = 0,
        NativeServer = 1 << 0,
        WebServer    = 1 << 1
    };
    Q_ENUM(ServerType)

    /** Get/Set the name of the host within the QLC+ network */
    QString hostName() const;
    void setHostName(QString hostName);

    /** Get/Set the mask of the enabled server types */
    int serverType() const;
    void setServerType(int typeMask);

    /** Enable/disable a single server type. If the currently
     *  enabled servers are running, $type is started/stopped as well */
    Q_INVOKABLE void enableServerType(int type, bool enable);

    /** Start $type if it is stopped, stop it otherwise. The enabled
     *  server types mask is updated accordingly, so that the autostart
     *  option follows what the user actually started.
     *  Return true if $type is running when returning */
    Q_INVOKABLE bool toggleServerType(int type);

    bool startAutomatically() const;
    void setStartAutomatically(bool startAutomatically);

    QString serverPassword() const;
    void setServerPassword(QString password);

    /** Store $key in the global QLC+ settings, encrypted with the QLC+
     *  master key, and make it the key in use. Running servers are
     *  restarted, since the key is the session shared secret */
    Q_INVOKABLE bool saveEncryptionKey(QString key);

    void setWebServerConfiguration(int portNumber, bool enableAuth, const QString &passwordFile);

    /** Force the given server types to be always enabled and running,
     *  regardless of the workspace network settings.
     *  This is used by the command line options */
    void setForcedServerTypes(int typeMask);

    int connectionsCount() const;

public slots:
    void sendAction(int code, TardisAction action);

protected:
    QString defaultName();

    /** Return the 64 bit key currently in use: the user encryption key
     *  if set, otherwise the QLC+ master key. It is both the packets
     *  cypher key and the secret exchanged on authentication */
    quint64 sessionKey() const;

    /** Feed the session key to the encryption engine */
    void applyEncryptionKey();

    /** Send the content of $packet using the provided $socket */
    bool sendTCPPacket(QTcpSocket *socket, QByteArray &packet, bool encrypt);

signals:
    void hostNameChanged(QString hostName);
    void serverTypeChanged(int type);
    void startAutomaticallyChanged(bool startAutomatically);
    void serverPasswordChanged(QString password);
    void connectionsCountChanged();
    void actionReady(int code, quint32 id, QVariant value);

protected slots:
    /** Async event raised when UDP packets are received */
    void slotProcessUDPPackets();

    /** Async event raised when unicast packets are received */
    void slotProcessTCPPackets();

    void slotDocLoaded();

    quint64 actionKey(int code, quint32 objID) const;
    void markActionSource(int code, quint32 objID, QTcpSocket *socket);
    bool shouldSkipEcho(const QTcpSocket *socket, int code, quint32 objID);

private:
    /** Reference to the QLC+ Doc */
    Doc *m_doc;
    VirtualConsole *m_virtualConsole;
    SimpleDesk *m_simpleDesk;

    /** Global flag to enable/disable packets encryption */
    bool m_encryptPackets;

    /** The host name in the QLC+ network */
    QString m_hostName;

    /** Mask of the enabled server types: Native and/or Web */
    int m_serverType;

    /** Whether the selected server should autostart on workspace load */
    bool m_startAutomatically;

    /** Native server password */
    QString m_serverPassword;

    /** The type of this host */
    HostType m_hostType;

    /** The UDP socket used to send/receive QLC+ announce packets */
    QUdpSocket *m_udpSocket;

    /** Reference to an encryption engine. For now we use SimpleCrypt */
    SimpleCrypt *m_crypt;

    /** Reference to a class in charge of packetize/extract data
     *  according to the QLC+ network protocol */
    NetworkPacketizer* m_packetizer;

    /** Runtime instance of the web server */
    WebAccessQml *m_webAccess;

    /** Web server runtime options */
    int m_webServerPort;
    bool m_webServerAuth;
    QString m_webServerPasswordFile;

    /*********************************************************************
     * Server
     *********************************************************************/
public:
    /** Start/stop every server type currently enabled.
     *  Return true if at least one server changed state */
    Q_INVOKABLE bool startServer();
    Q_INVOKABLE bool stopServer();

    /** Start/stop a single server type, no matter if it is enabled or not */
    Q_INVOKABLE bool startServerType(int type);
    Q_INVOKABLE bool stopServerType(int type);

    Q_INVOKABLE bool setClientAccess(QString hostName, bool allow, int accessMask);
    Q_INVOKABLE bool sendWorkspaceToClient(QString hostName, QString filename);

    /** Tell every connected client that this server is about to replace its
     *  workspace, so they can clear their contents before the new project
     *  data (or a stream of actions on it) starts to arrive */
    void notifyProjectChanging();

    /** Tell every connected client that a new workspace is fully loaded and
     *  can be requested. The project is not pushed: each client asks for it
     *  when ready, so a busy or detached client is not forced to reload */
    void notifyProjectLoaded();

    /** Return true if at least one server instance is running */
    bool serverStarted() const;

    /** Get the running status of each server type */
    bool nativeServerStarted() const;
    bool webServerStarted() const;

protected:
    QHostAddress getHostFromName(QString name) const;

    /** Update the mask of the running servers and notify the changes */
    void setServerStartedMask(int mask);

signals:
    void serverStartedChanged(bool serverStarted);
    void clientAccessRequest(QString hostName);

    /** Emitted on a server when a client asks for the current workspace.
     *  The App answers it, since it owns the project file name */
    void clientProjectRequest(QString hostName);

protected slots:
    /** Event raised when an incoming connection is requested on
     *  the TCP socket server side */
    void slotProcessNewTCPConnection();
    void slotHostDisconnected();

private:
    /** Instance of a TCP server used by a QLC+ server */
    QTcpServer *m_tcpServer;

    /** Mask of the server types currently running */
    int m_serverStartedMask;

    /** Map of the QLC+ hosts detected on the network */
    QHash<QHostAddress, NetworkHost *> m_hostsMap;
    /** Incoming TCP data not yet forming a complete packet, per socket.
     *  TCP is a stream: a packet can be split across several readyRead
     *  signals, so the leftover must be kept until the rest arrives */
    QHash<QTcpSocket *, QByteArray> m_rxBuffers;
    /** Socket currently being processed on server RX path (used to avoid echoing back) */
    QTcpSocket *m_currentRxSocket = nullptr;
    /** Tracks the source socket of recently received actions, to suppress delayed echo */
    QHash<quint64, QPointer<QTcpSocket>> m_recentActionSources;
    /** Mask of the server types forced from the command line, kept
     *  enabled regardless of the workspace network settings */
    int m_forcedServerTypes = NoServer;

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

    /** Ask the connected server to send its current workspace */
    Q_INVOKABLE bool requestProjectFromServer();

    QVariant serverList() const;

    /** Get/Set the connection status of a QLC+ client instance */
    int clientStatus() const;
    void setClientStatus(int clientStatus);

signals:
    void clientStatusChanged(bool clientStatus);
    void serverListChanged();
    void accessMaskChanged(int mask);
    void requestProjectLoad(QByteArray &data);
    void storeAutostartProject(QString fileName);

    /** Emitted on a client when the server announces it is replacing its
     *  workspace. The client is expected to clear its own contents */
    void requestProjectClear();

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
