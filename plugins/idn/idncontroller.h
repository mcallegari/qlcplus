/*
  Q Light Controller Plus
  idncontroller.h

  Copyright (c) Daniel Schröder

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

#ifndef IDNCONTROLLER_H
#define IDNCONTROLLER_H

#include <QObject>
#include <QtNetwork>
#include <QScopedPointer>
#include <QTimer>

#include "idnclient.h"

//INTERVAL FOR CONFIG PACKETS in ms
#define IDN_CONFIG_INTERVAL 180
#define IDN_TIMEOUT 250
#define IDN_SCAN_TIMEOUT 1000
#define IDN_PORT 7255
#define IDN_MAX_CLIENTS 8

typedef struct
{
    QHostAddress outputAddress;
    ushort outputUniverse;
} UniverseInfo;

typedef struct
{
    int port;
    QString serviceName;
    QString unitName;
    int serviceID;
    int serviceType;
    int mode;
    int idnChannel;
    int rangeBegin;
    int rangeEnd;
    quint32 universe;
    QHostAddress iface;
	bool scan;
  bool disabled;
}IdnSettings;

typedef struct
{
    int port;
    int mode;
    int idnChannel;
    int rangeBegin;
    int rangeEnd;
    int serviceID;
    quint32 universe;
    QHostAddress iface;
    IdnClient *client;
}IdnClientInfo;

struct IdnHostInfo
{
  QHostAddress address;
  int serviceId;

  bool operator==(const IdnHostInfo &other) const
  {
      return address == other.address && serviceId == other.serviceId;
  }
};

inline size_t qHash(const IdnHostInfo &key, size_t seed = 0)
{
    return qHash(key.address, seed) ^ qHash(key.serviceId, seed);
}

typedef struct {
    IdnHostInfo hostInfo;
    IdnSettings settings;
} IdnHostClientSettings;

class IdnController : public QObject
{
    Q_OBJECT

public:
  IdnController(QNetworkAddressEntry const& address, quint32 line, QHash<IdnHostInfo, IdnSettings> settings, QObject *parent = 0);

	~IdnController();

	/** Send DMX data to a specific port/universe */
  void handleDmx(const quint32 universe, const QByteArray& data);
  
  /** Return the controller IP address */
  QString getNetworkIP();
  
  /** Returns the map of all clients */
  QHash<IdnHostInfo, IdnClientInfo> getClientsList();

  /** Add a universe to the map of this controller */
  void addUniverse(quint32 universe);

  /** Remove a universe from the map of this controller */
  void removeUniverse(quint32 universe);

  /** Return the list of the universes handled by this controller */
  QList<quint32> universesList();

  /** Return the specific information for the given universe */
  UniverseInfo *getUniverseInfo(quint32 universe);

  /** Return the plugin line associated to this controller */
  quint32 line();

  /** Get the number of packets sent by this controller */
  quint64 getPacketSentNumber();
    // Debug
  void DBG_CheckSettings();

  bool closeByUniverse(quint32 universe);

private:
  /** The network interface associated to this controller */
  QNetworkInterface m_interface;
  QNetworkAddressEntry m_address;
  int m_prefixLength;
  /** The controller IP address as QHostAddress */
  QHostAddress m_ipAddr;
  
  /** The controller broadcast address as QHostAddress */
  QHostAddress m_broadcastAddr;

  /** Counter for transmitted packets */
  quint64 m_packetSent;

  /** QLC+ line to be used when emitting a signal */
  quint32 m_line;

  /** The UDP socket used to send/receive IDN packets */
  QSharedPointer<QUdpSocket> m_socket;
  QUdpSocket *m_scanSocket;

  /** Helper class used to create or parse IDN packets */
  QScopedPointer<IdnPacketizer> m_packetizer;

  /** Map of the IDN clients discovered */
  QHash<IdnHostInfo, IdnClientInfo> m_clientsList;
  QHash<IdnHostInfo, IdnSettings> m_fileSettings;

  /** Keeps the current dmx values to send only the ones that changed */
  /** It holds values for all the handled universes */
  QMap<int, QByteArray *> m_dmxValuesMap;

  /** Map of the QLC+ universes transmitted/received by this
   *  controller, with the related, specific parameters */
  QMap<quint32, UniverseInfo> m_universeMap;

  /** Mutex to handle the change of output IP address or in general
    *  variables that could be used to transmit/receive data */
  QMutex m_dataMutex;

 public:
  void sendScan();

private:
  void initClients();

  /** Shared mutex to protect concurrent writeDatagram() calls on m_socket */
  QSharedPointer<QMutex> m_socketMutex;

  //QSharedPointer<QUdpSocket> getUdpSocket();
  //quint32 blackCounter;

  //int blackCounter;

  /** if data does not change the old data will be send */
  //QByteArray oldData;
  /** old packetinformation */
  //IdnOptimizer::PacketInformation oldpi;

	/** Timestamp to check wheater a config packet is neccessary */
	//qint64 timestamp;
  /** Timestamp of the last submitted packet */
  //qint64 lastsend;
	/** Sequence Number */
	//quint32 m_seqnum;

	/** Range of Channels */
	//int m_port; 
  //int m_rangeBegin;
	//int m_rangeEnd;
	//quint8 m_mode;
	QScopedPointer<IdnOptimizer> m_optimizer;

  //QByteArray optimizedMode(const QByteArray &data, const IdnClientInfo info);
  //QByteArray rangeMode(const QByteArray &data);

  //void sendClosePacket();
  //Timer for the blackout functionality to close the IDN channel
//  QTimer *closeTimer;

  // public slots:
  //   void waitforScanReply();
};

#endif
