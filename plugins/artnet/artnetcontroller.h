/*
  Q Light Controller Plus
  artnetnode.h

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

#ifndef ARTNETNODE_H
#define ARTNETNODE_H

#include "artnetpacketizer.h"

#include <QtNetwork>
#include <QObject>

#define ARTNET_DEFAULT_PORT     6454

class ArtNetController : public QObject
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    enum Type { Unknown = 0x0, Input = 0x01, Output = 0x02 };

    ArtNetController(QString ipaddr, QList<QNetworkAddressEntry> interfaces,
                     QList<QString>macAddrList, Type type, QObject *parent = 0);

    ~ArtNetController();

    /** Send DMX data to a specific port/universe */
    void sendDmx(const int& universe, const QByteArray& data);

    /** Return the controller IP address */
    QString getNetworkIP();

    /** Returns the map of Nodes discovered by ArtPoll */
    QHash<QHostAddress, ArtNetNodeInfo> getNodesList();

    /** add an output port to this controller (in DMX words, a universe */
    void addUniverse(quint32 line, int uni);

    /** Returns the number of universes managed by this controller */
    int getUniversesNumber();

    /** Remove a universe managed by this controller */
    bool removeUniverse(int uni);

    /** Get the type of this controller */
    int getType();

    /** Get the number of packets sent by this controller */
    quint64 getPacketSentNumber();

    /** Get the number of packets received by this controller */
    quint64 getPacketReceivedNumber();

private:
    /** The controller IP address as QHostAddress */
    QHostAddress m_ipAddr;

    /** The controller broadcast address as QHostAddress */
    /** This is where all ArtNet packets are sent to */
    QHostAddress m_broadcastAddr;

    /** The controller interface MAC address. Used only for ArtPollReply */
    QString m_MACAddress;

    quint64 m_packetSent;
    quint64 m_packetReceived;

    /** List of universes managed by this controller */
    /** Coupled as universe/QLC+ line */
    QHash<int, quint32> m_universes;

    /** Type of this controller */
    /** A controller can be only output or only input */
    Type m_type;

    /** The UDP socket used to send/receive ArtNet packets */
    QUdpSocket *m_UdpSocket;

    /** Helper class used to create or parse ArtNet packets */
    ArtNetPacketizer *m_packetizer;

    /** Map of the ArtNet nodes discovered with ArtPoll */
    QHash<QHostAddress, ArtNetNodeInfo> m_nodesList;

    /** Keeps the current dmx values to send only the ones that changed */
    /** It holds values for a whole 4 universes address (512 * 4) */
    QByteArray m_dmxValues;

private slots:
    /** Async event raised when new packets have been received */
    void processPendingPackets();

signals:
    void valueChanged(quint32 input, int channel, uchar value);
};

#endif
