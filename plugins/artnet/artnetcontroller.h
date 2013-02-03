/*
  Q Light Controller
  artnetnode.h

  Copyright (c) Massimo Callegari

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
    enum ControllerType { Unknown = 0x0, Input = 0x01, Output = 0x02 };

    ArtNetController(QString ipaddr, QList<QNetworkAddressEntry> interfaces,
                     QList<QString>macAddrList, ControllerType type, QObject *parent = 0);

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

private:
    /** The controller IP address as QHostAddress */
    QHostAddress m_ipAddr;

    /** The controller broadcast address as QHostAddress */
    /** This is where all ArtNet packets are sent to */
    QHostAddress m_broadcastAddr;

    /** The controller interface MAC address. Used only for ArtPollReply */
    QString m_MACAddress;

    /** List of universes managed by this controller */
    /** Coupled as universe/QLC+ line */
    QHash<int, quint32> m_universes;

    /** Type of this controller */
    /** A controller can be only output or only input */
    ControllerType m_type;

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
