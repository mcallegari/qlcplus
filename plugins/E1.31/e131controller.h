/*
  Q Light Controller Plus
  e131node.h

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

#ifndef E131NODE_H
#define E131NODE_H

#include "e131packetizer.h"

#include <QtNetwork>
#include <QObject>

#define E131_DEFAULT_PORT     5568

class E131Controller : public QObject
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    enum Type { Unknown = 0x0, Input = 0x01, Output = 0x02 };

    E131Controller(QString ipaddr, QString macAddress,
                   Type type, quint32 line, QObject *parent = 0);

    ~E131Controller();

    /** Send DMX data to a specific port/universe */
    void sendDmx(const quint32 universe, const QByteArray& data);

    /** Return the controller IP address */
    QString getNetworkIP();

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

    /** The controller multicast addresses map as QHostAddress */
    /** This is where all E131 packets are sent to */
    QHash<quint32, QHostAddress> m_multicastAddr;

    /** The controller interface MAC address. Used only for ArtPollReply */
    QString m_MACAddress;

    quint64 m_packetSent;
    quint64 m_packetReceived;

    /** Type of this controller */
    /** A controller can be only output or only input */
    Type m_type;

    /** QLC+ line to be used when emitting a signal */
    quint32 m_line;

    /** The UDP socket used to send/receive E131 packets */
    QUdpSocket *m_UdpSocket;

    /** Helper class used to create or parse E131 packets */
    E131Packetizer *m_packetizer;

    /** Keeps the current dmx values to send only the ones that changed */
    /** It holds values for a whole 4 universes address (512 * 4) */
    QByteArray m_dmxValues;

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
