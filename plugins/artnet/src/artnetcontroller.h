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

#ifndef ARTNETCONTROLLER_H
#define ARTNETCONTROLLER_H

#include <QtNetwork>
#include <QObject>
#include <QScopedPointer>

#include "artnetpacketizer.h"

#define ARTNET_PORT      6454

class QTimer;

typedef struct
{
    ushort inputUniverse;

    QHostAddress outputAddress;
    ushort outputUniverse;
    int outputTransmissionMode;

    int type;
} UniverseInfo;

class ArtNetController : public QObject
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    enum Type { Unknown = 0x0, Input = 0x01, Output = 0x02 };

    enum TransmissionMode { Full, Partial };

    ArtNetController(QNetworkInterface const& interface,
                     QNetworkAddressEntry const& address,
                     QSharedPointer<QUdpSocket> const& udpSocket,
                     quint32 line, QObject *parent = 0);

    ~ArtNetController();

    /** Send DMX data to a specific port/universe */
    void sendDmx(const quint32 universe, const QByteArray& data);

    /** Return the controller IP address */
    QString getNetworkIP();

    /** Returns the map of Nodes discovered by ArtPoll */
    QHash<QHostAddress, ArtNetNodeInfo> getNodesList();

    /** Add a universe to the map of this controller */
    void addUniverse(quint32 universe, Type type);

    /** Remove a universe from the map of this controller */
    void removeUniverse(quint32 universe, Type type);

    /** Set a specific ArtNet input universe for the given QLC+ universe.
     *  Return true if this restores default input universe */
    bool setInputUniverse(quint32 universe, quint32 artnetUni);

    /** Set a specific output IP address for the given QLC+ universe.
     *  Return true if this restores default output IP address */
    bool setOutputIPAddress(quint32 universe, QString address);

    /** Set a specific ArtNet output universe for the given QLC+ universe.
     *  Return true if this restores default output universe */
    bool setOutputUniverse(quint32 universe, quint32 artnetUni);

    /** Set the transmission mode of the ArtNet DMX packets over the network.
     *  It can be 'Full', which transmits always 512 channels, or
     *  'Partial', which transmits only the channels actually used in a
     *  universe.
     *  Return true if this restores default transmission mode */
    bool setTransmissionMode(quint32 universe, TransmissionMode mode);

    /** Converts a TransmissionMode value into a human readable string */
    static QString transmissionModeToString(TransmissionMode mode);

    /** Converts a human readable string into a TransmissionMode value */
    static TransmissionMode stringToTransmissionMode(const QString& mode);

    /** Return the list of the universes handled by
     *  this controller */
    QList<quint32> universesList();

    /** Return the specific information for the given universe */
    UniverseInfo *getUniverseInfo(quint32 universe);

    /** Return the global type of this controller */
    Type type();

    /** Return the plugin line associated to this controller */
    quint32 line();

    /** Get the number of packets sent by this controller */
    quint64 getPacketSentNumber();

    /** Get the number of packets received by this controller */
    quint64 getPacketReceivedNumber();

    /** Is the UDP socket capable of receiving packets ? */
    bool socketBound() const;

private:
    /** The network interface associated to this controller */
    QNetworkInterface m_interface;
    QNetworkAddressEntry m_address;
    /** The controller IP address as QHostAddress */
    QHostAddress m_ipAddr;

    /** The controller broadcast address as QHostAddress */
    /** This is where all ArtNet packets are sent to */
    QHostAddress m_broadcastAddr;

    QString m_MACAddress;

    /** Counter for transmitted packets */
    quint64 m_packetSent;

    /** Counter for received packets */
    quint64 m_packetReceived;

    /** QLC+ line to be used when emitting a signal */
    quint32 m_line;

    /** The UDP socket used to send/receive ArtNet packets */
    QSharedPointer<QUdpSocket> m_udpSocket;

    /** Helper class used to create or parse ArtNet packets */
    QScopedPointer<ArtNetPacketizer> m_packetizer;

    /** Map of the ArtNet nodes discovered with ArtPoll */
    QHash<QHostAddress, ArtNetNodeInfo> m_nodesList;

    /** Keeps the current dmx values to send only the ones that changed */
    /** It holds values for all the handled universes */
    QMap<int, QByteArray *> m_dmxValuesMap;

    /** Map of the QLC+ universes transmitted/received by this
     *  controller, with the related, specific parameters */
    QMap<quint32, UniverseInfo> m_universeMap;

    /** Mutex to handle the change of output IP address or in general
     *  variables that could be used to transmit/receive data */
    QMutex m_dataMutex;

    QTimer* m_pollTimer;

private:
    bool handleArtNetPollReply(QByteArray const& datagram, QHostAddress const& senderAddress);
    bool handleArtNetPoll(QByteArray const& datagram, QHostAddress const& senderAddress);
    bool handleArtNetDmx(QByteArray const& datagram, QHostAddress const& senderAddress);

public:
    // Handle a packet received to the ArtNet port.
    // Returns true if the packet has been handled,
    // or if the packet should not be handled by another controller.
    bool handlePacket(QByteArray const& datagram, QHostAddress const& senderAddress);

protected slots:
    void slotPollTimeout();
protected:
    void sendPoll();

signals:
    void valueChanged(quint32 universe, quint32 input, quint32 channel, uchar value);
};

#endif
