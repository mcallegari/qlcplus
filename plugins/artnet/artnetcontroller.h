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
    ArtNetController(QString ipaddr, int universe, QList<QNetworkAddressEntry> interfaces, QObject *parent = 0);
    ~ArtNetController();

    /** Send DMX data to a specific port/universe */
    void sendDmx(const int& universe, const QByteArray& data);

    /** Return the controller IP address */
    QString getNetworkIP();

    /** Returns the map of Nodes discovered by ArtPoll */
    QHash<QHostAddress, ArtNetNodeInfo> getNodesList();

    /** add an output port to this controller (in DMX words, a universe */
    void addUniverse(int uni);

    /** Returns the number of universes managed by this controller */
    int getUniversesNumber();

    /** Remove a universe managed by this controller */
    bool removeUniverse(int uni);

private:
    /** The controller IP address as QHostAddress */
    QHostAddress m_ipAddr;

    /** The controller broadcast address as QHostAddress */
    /** This is where all ArtNet packets are sent to */
    QHostAddress m_broadcastAddr;

    /** List of universes managed by this controller */
    QList<int> m_universes;

    /** The UDP socket used to send/receive ArtNet packets */
    QUdpSocket *m_UdpSocket;

    /** Helper class used to create or parse ArtNet packets */
    ArtNetPacketizer *m_packetizer;

    /** Map of the ArtNet nodes discovered with ArtPoll */
    QHash<QHostAddress, ArtNetNodeInfo> m_nodesList;

private slots:
    /** Async event raised when new packets have been received */
    void processPendingPackets();
};

#endif
