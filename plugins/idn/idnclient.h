/*
  Q Light Controller Plus
  idnclient.h

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

#ifndef IDNCLIENT_H
#define IDNCLIENT_H

#include <QObject>
#include <QtNetwork>
#include <QScopedPointer>
#include <QTimer>
#include <QMutex> 

#include "idnoptimizer.h"
#include "idnpacketizer.h"

#define IDN_CONFIG_INTERVAL 180
#define IDN_TIMEOUT 250

// Timing constants for packet rate throttling.
// Based on the DMX512 bit-slot time at 250 kbit/s (44 µs per slot).
// Minimum inter-packet gap = (IDN_PACKET_OVERHEAD_US + IDN_DMX_SLOT_TIME_US * numChannels) / 1000 ms.
#define IDN_DMX_SLOT_TIME_US   44   // microseconds per DMX slot at 250 kbit/s
#define IDN_PACKET_OVERHEAD_US 140  // baseline per-packet overhead in microseconds

class IdnClient : public QObject
{
    Q_OBJECT

public:
    IdnClient(QHostAddress const &clientAddress, QSharedPointer<QUdpSocket> const& udpSocket,
              QSharedPointer<QMutex> const& socketMutex,
              int const& port, int const& rangeBegin, int const& rangeEnd, int const& mode, int const& channelID, int const& serviceID);
    ~IdnClient();
    void sendDmx(const QByteArray &data);
    quint64 getPacketSentNumber();
  private:
    QHostAddress m_address;
    QSharedPointer<QUdpSocket> m_udpSocket;
    /** Shared mutex protecting concurrent writeDatagram() calls on m_udpSocket */
    QSharedPointer<QMutex> m_socketMutex;
    quint16 m_port; 
    quint16 m_rangeBegin;
    quint16 m_rangeEnd;
    quint8 m_mode;
    quint8 m_channelID;
    quint8 m_serviceID;

    QScopedPointer<IdnPacketizer> m_packetizer;
    QScopedPointer<IdnOptimizer> m_optimizer;

    //////////////////////
    ///For packet processing
    //////////////////////
    /** true = packet with config header - false = packet without config header */
    bool config;
    int blackCounter;

    /** if data does not change the old data will be send */
    QByteArray oldData;
    /** old packetinformation */
    IdnOptimizer::PacketInformation oldpi;
    /** Timestamp to check whether a config packet is necessary */
    qint64 timestamp;
    /** Timestamp of the last submitted packet */
    qint64 lastsend;
    /** Sequence Number */
    quint32 m_seqnum;
	quint32 m_packetSent;
    
    QMutex m_dataMutex;

    QTimer *closeTimer;

    QByteArray optimizedMode(const QByteArray &data);
    QByteArray rangeMode(const QByteArray &data);
  private slots:
    void sendClosePacket();
};

#endif
