/*
  Q Light Controller Plus
  artnetcontroller.h

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
                     QString macAddress, Type type, quint32 line, QObject *parent = 0);

    ~ArtNetController();

    /** Send DMX data to a specific port/universe */
    void sendDmx(const quint32 universe, const QByteArray& data);

    /** Return the controller IP address */
    QString getNetworkIP();

    /** Returns the map of Nodes discovered by ArtPoll */
    QHash<QHostAddress, ArtNetNodeInfo> getNodesList();

    /** Set the controller type */
    void setType(Type type);

    /** Get the type of this controller */
    Type type();

    /** Get the number of packets sent by this controller */
    quint64 getPacketSentNumber();

    /** Get the number of packets received by this controller */
    quint64 getPacketReceivedNumber();

    /** Increase or decrease the reference count of the given type */
    void changeReferenceCount(Type type, int amount);

    /** Retrieve the reference count of the given type */
    int referenceCount(Type type);

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

    /** Type of this controller */
    /** A controller can be only output or only input */
    Type m_type;

    /** QLC+ line to be used when emitting a signal */
    quint32 m_line;

    /** The UDP socket used to send/receive ArtNet packets */
    QUdpSocket *m_UdpSocket;

    /** Helper class used to create or parse ArtNet packets */
    ArtNetPacketizer *m_packetizer;

    /** Map of the ArtNet nodes discovered with ArtPoll */
    QHash<QHostAddress, ArtNetNodeInfo> m_nodesList;

    /** Keeps the current dmx values to send only the ones that changed */
    /** It holds values for all the handled universes */
    QMap<int, QByteArray *> m_dmxValuesMap;

    /** Count the number of input universes using this controller */
    int m_inputRefCount;

    /** Count the number of output universes using this controller */
    int m_outputRefCount;

private slots:
    /** Async event raised when new packets have been received */
    void processPendingPackets();

signals:
    void valueChanged(quint32 universe, quint32 input, quint32 channel, uchar value);
};

#endif
