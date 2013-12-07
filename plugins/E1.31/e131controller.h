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

    E131Controller(QString ipaddr, QList<QNetworkAddressEntry> interfaces,
                     QList<QString>macAddrList, Type type, QObject *parent = 0);

    ~E131Controller();

    /** Send DMX data to a specific port/universe */
    void sendDmx(const int& universe, const QByteArray& data);

    /** Return the controller IP address */
    QString getNetworkIP();

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

    /** The controller multicast addresses map as QHostAddress */
    /** This is where all E131 packets are sent to */
    QHash<int, QHostAddress> m_multicastAddr;

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

    /** The UDP socket used to send/receive E131 packets */
    QUdpSocket *m_UdpSocket;

    /** Helper class used to create or parse E131 packets */
    E131Packetizer *m_packetizer;

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
