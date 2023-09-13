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

#if defined(ANDROID)
#include <QScopedPointer>
#include <QSharedPointer>
#endif
#include <QNetworkInterface>
#include <QHostAddress>
#include <QUdpSocket>
#include <QVariant>
#include <QMutex>
#include <QTimer>

#include "artnetpacketizer.h"

#define ARTNET_PORT      6454

typedef struct _uinfo
{
    /** This is a bitmask to determine the capability
     *  of a Controller. Can be Input, Output or both */
    int type;

    /** When receiving input data, this is the universe
     *  the controller will process */
    ushort inputUniverse;

    /** Keeps the current dmx values to send only the ones that changed */
    /** It holds values for all the handled universes */
    QByteArray inputData;

    /** This is the destination IP address used when
     *  transmitting output data. Can be broadcast or unicast,
     *  including localhost. */
    QHostAddress outputAddress;

    /** When transmitting output data, this is the universe
     *  written on outgoing packets */
    ushort outputUniverse;

    /** This is the mode used to transmit output data.
     *  Enumerated in ArtNetController::TransmissionMode */
    int outputTransmissionMode;

    /** Universe data to be sent depending on the transmission mode */
    QByteArray outputData;
} UniverseInfo;

class ArtNetController : public QObject
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    enum Type { Unknown = 0x0, Input = 0x01, Output = 0x02 };

    enum TransmissionMode { Standard, Full, Partial };

    ArtNetController(QNetworkInterface const& iface,
                     QNetworkAddressEntry const& address,
                     QSharedPointer<QUdpSocket> const& udpSocket,
                     quint32 line, QObject *parent = 0);

    ~ArtNetController();

    /** Send DMX data to a specific port/universe */
    void sendDmx(const quint32 universe, const QByteArray& data, bool dataChanged);

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

    /** Send a RDM command */
    bool sendRDMCommand(const quint32 universe, uchar command, QVariantList params);

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

    /** Map of the QLC+ universes transmitted/received by this
     *  controller, with the related, specific parameters */
    QMap<quint32, UniverseInfo> m_universeMap;

    /** Mutex to handle the change of output IP address or in general
     *  variables that could be used to transmit/receive data */
    QMutex m_dataMutex;

    /** Timer used to send out an ArtPoll packet to
     *  discover available nodes on the network */
    QTimer m_pollTimer;

    /** Timer used to send output data every N seconds
     *  when data is not changing */
    QTimer m_sendTimer;

private:
    bool handleArtNetPollReply(QByteArray const& datagram, QHostAddress const& senderAddress);
    bool handleArtNetPoll(QByteArray const& datagram, QHostAddress const& senderAddress);
    bool handleArtNetDmx(QByteArray const& datagram, QHostAddress const& senderAddress);
    bool handleArtNetTodData(QByteArray const& datagram, QHostAddress const& senderAddress);
    bool handleArtNetRDM(QByteArray const& datagram, QHostAddress const& senderAddress);

public:
    // Handle a packet received to the ArtNet port.
    // Returns true if the packet has been handled,
    // or if the packet should not be handled by another controller.
    bool handlePacket(QByteArray const& datagram, QHostAddress const& senderAddress);

protected slots:
    void slotSendPoll();
    void slotSendAllUniverses();

signals:
    void valueChanged(quint32 universe, quint32 input, quint32 channel, uchar value);

    void rdmValueChanged(quint32 universe, quint32 line, QVariantMap data);
};

#endif
