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

class ArtNetNode : public QObject
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    ArtNetNode(QString ipaddr, int port, QObject *parent = 0);
    ~ArtNetNode();

    void sendDmx(const int& universe, const QByteArray& data);

private:
    void addPort(int port);

private:
    QString m_ipAddr;
    QList<int> m_ports;
    QUdpSocket *m_UdpSocket;
    QHostAddress m_broadcastAddr;
    ArtNetPacketizer *m_packetizer;

private slots:
    void processPendingPackets();
};

#endif
