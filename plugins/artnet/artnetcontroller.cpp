/*
  Q Light Controller Plus
  artnetnode.cpp

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

#include "artnetcontroller.h"

#include <QDebug>

ArtNetController::ArtNetController(QString ipaddr, QList<QNetworkAddressEntry> interfaces,
                                   QList<QString> macAddrList, Type type, QObject *parent)
    : QObject(parent)
{
    m_ipAddr = QHostAddress(ipaddr);

    int i = 0;
    foreach(QNetworkAddressEntry iface, interfaces)
    {
        if (iface.ip() == m_ipAddr)
        {
            m_broadcastAddr = iface.broadcast();
            m_MACAddress = macAddrList.at(i);
            break;
        }
        i++;
    }

    qDebug() << "[ArtNetController] Broadcast address:" << m_broadcastAddr.toString() << "(MAC:" << m_MACAddress << ")";
    qDebug() << "[ArtNetController] type: " << type;
    m_packetizer = new ArtNetPacketizer();
    m_packetSent = 0;
    m_packetReceived = 0;

    m_UdpSocket = new QUdpSocket(this);

    if (m_UdpSocket->bind(ARTNET_DEFAULT_PORT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint) == false)
        return;

    connect(m_UdpSocket, SIGNAL(readyRead()),
            this, SLOT(processPendingPackets()));

    // don't send a Poll if we're an input
    if (type == Output)
    {
        QByteArray pollPacket;
        m_packetizer->setupArtNetPoll(pollPacket);
        qint64 sent = m_UdpSocket->writeDatagram(pollPacket.data(), pollPacket.size(),
                                                 m_broadcastAddr, ARTNET_DEFAULT_PORT);
        if (sent < 0)
        {
            qDebug() << "Unable to send initial Poll packet";
            qDebug() << "Errno: " << m_UdpSocket->error();
            qDebug() << "Errmgs: " << m_UdpSocket->errorString();
        }
        else
            m_packetSent++;
    }
    else
    {
        m_dmxValues.fill(0, 2048);
    }

    m_type = type;
}

ArtNetController::~ArtNetController()
{
    qDebug() << Q_FUNC_INFO;
    disconnect(m_UdpSocket, SIGNAL(readyRead()),
            this, SLOT(processPendingPackets()));
    m_UdpSocket->close();
}

void ArtNetController::addUniverse(quint32 line, int uni)
{
    if (m_universes.contains(uni) == false)
    {
        m_universes[uni] = line;
        qDebug() << "[ArtNetController::addUniverse] Added new universe: " << uni;
    }
}

int ArtNetController::getUniversesNumber()
{
    return m_universes.size();
}

bool ArtNetController::removeUniverse(int uni)
{
    if (m_universes.contains(uni))
    {

        qDebug() << Q_FUNC_INFO << "Removing universe " << uni;
        return m_universes.remove(uni);
    }
    return false;
}

int ArtNetController::getType()
{
    return m_type;
}

quint64 ArtNetController::getPacketSentNumber()
{
    return m_packetSent;
}

quint64 ArtNetController::getPacketReceivedNumber()
{
    return m_packetReceived;
}

QString ArtNetController::getNetworkIP()
{
    return m_ipAddr.toString();
}

QHash<QHostAddress, ArtNetNodeInfo> ArtNetController::getNodesList()
{
    return m_nodesList;
}

void ArtNetController::sendDmx(const int &universe, const QByteArray &data)
{
    QByteArray dmxPacket;
    m_packetizer->setupArtNetDmx(dmxPacket, universe, data);
    qint64 sent = m_UdpSocket->writeDatagram(dmxPacket.data(), dmxPacket.size(),
                                             m_broadcastAddr, ARTNET_DEFAULT_PORT);
    if (sent < 0)
    {
        qDebug() << "sendDmx failed";
        qDebug() << "Errno: " << m_UdpSocket->error();
        qDebug() << "Errmgs: " << m_UdpSocket->errorString();
    }
    else
        m_packetSent++;
}

void ArtNetController::processPendingPackets()
{
    while (m_UdpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        QHostAddress senderAddress;
        datagram.resize(m_UdpSocket->pendingDatagramSize());
        m_UdpSocket->readDatagram(datagram.data(), datagram.size(), &senderAddress);
        if (senderAddress != m_ipAddr)
        {
            qDebug() << "Received packet with size: " << datagram.size() << ", host: " << senderAddress.toString();
            int opCode = -1;
            if (m_packetizer->checkPacketAndCode(datagram, opCode) == true)
            {
                switch (opCode)
                {
                    case ARTNET_POLLREPLY:
                    {
                        qDebug() << "ArtPollReply received";
                        ArtNetNodeInfo newNode;
                        if (m_packetizer->fillArtPollReplyInfo(datagram, newNode) == true)
                        {
                            if (m_nodesList.contains(senderAddress) == false)
                                m_nodesList[senderAddress] = newNode;
                        }
                        QByteArray pollReplyPacket;
                        m_packetizer->setupArtNetPollReply(pollReplyPacket, m_ipAddr, m_MACAddress);
                        m_UdpSocket->writeDatagram(pollReplyPacket.data(), pollReplyPacket.size(),
                                                   senderAddress, ARTNET_DEFAULT_PORT);
                        m_packetReceived++;
                        m_packetSent++;
                    }
                    break;
                    case ARTNET_POLL:
                    {
                        qDebug() << "ArtPoll received";
                        QByteArray pollReplyPacket;
                        m_packetizer->setupArtNetPollReply(pollReplyPacket, m_ipAddr, m_MACAddress);
                        m_UdpSocket->writeDatagram(pollReplyPacket.data(), pollReplyPacket.size(),
                                                   senderAddress, ARTNET_DEFAULT_PORT);
                        m_packetReceived++;
                        m_packetSent++;
                    }
                    break;
                    case ARTNET_DMX:
                    {
                        qDebug() << "DMX data received";
                        QByteArray dmxData;
                        int universe;
                        if (this->getType() == Input)
                        {
                            m_packetReceived++;
                            if (m_packetizer->fillDMXdata(datagram, dmxData, universe) == true)
                            {
                                if ((universe * 512) > m_dmxValues.length() || m_universes.contains(universe) == false)
                                {
                                    qDebug() << "Universe " << universe << "not supported !";
                                    break;
                                }
                                for (int i = 0; i < dmxData.length(); i++)
                                {
                                    if (m_dmxValues.at(i + (universe * 512)) != dmxData.at(i))
                                    {
                                        m_dmxValues[i + (universe * 512)] =  dmxData[i];
                                        emit valueChanged(m_universes[universe], i, (uchar)dmxData.at(i));
                                    }
                                }
                            }
                        }
                    }
                    break;
                    default:
                        qDebug() << "opCode not supported yet (" << opCode << ")";
                        m_packetReceived++;
                    break;
                }
            }
            else
                qDebug() << "Malformed packet received";
        }
     }
}
