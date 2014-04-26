/*
  Q Light Controller Plus
  e131node.cpp

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

#include "e131controller.h"

#include <QDebug>

E131Controller::E131Controller(QString ipaddr, QString macAddress, Type type, quint32 line, QObject *parent)
    : QObject(parent)
{
    m_ipAddr = QHostAddress(ipaddr);
    m_MACAddress = macAddress;
    m_line = line;

    qDebug() << "[E131Controller] type: " << type;
    m_packetizer = new E131Packetizer();
    m_packetSent = 0;
    m_packetReceived = 0;
    m_type = type;
    m_inputRefCount = 0;
    m_outputRefCount = 0;

    m_UdpSocket = new QUdpSocket(this);

    // reset initial DMX values if we're an input
    if (type == Input)
    {
        m_dmxValues.fill(0, 512);
        if (m_UdpSocket->bind(E131_DEFAULT_PORT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint) == false)
        {
            qDebug() << Q_FUNC_INFO << "Socket input bind failed !!";
            return;
        }
        m_inputRefCount = 1;
    }
    else
    {
        if (m_UdpSocket->bind(m_ipAddr, E131_DEFAULT_PORT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint) == false)
        {
            qDebug() << Q_FUNC_INFO << "Socket output bind failed !!";
            return;
        }
        m_outputRefCount = 1;
    }

    connect(m_UdpSocket, SIGNAL(readyRead()),
            this, SLOT(processPendingPackets()));
}

E131Controller::~E131Controller()
{
    qDebug() << Q_FUNC_INFO;
    disconnect(m_UdpSocket, SIGNAL(readyRead()),
            this, SLOT(processPendingPackets()));
    m_UdpSocket->close();
}

void E131Controller::setType(Type type)
{
    m_type = type;
}

E131Controller::Type E131Controller::type()
{
    return m_type;
}

quint64 E131Controller::getPacketSentNumber()
{
    return m_packetSent;
}

quint64 E131Controller::getPacketReceivedNumber()
{
    return m_packetReceived;
}

void E131Controller::changeReferenceCount(E131Controller::Type type, int amount)
{
    if (type == Input)
    {
        m_inputRefCount += amount;
        m_dmxValues.resize(m_inputRefCount * 512);
    }
    else
        m_outputRefCount += amount;
}

int E131Controller::referenceCount(E131Controller::Type type)
{
    if (type == Input)
        return m_inputRefCount;
    else
        return m_outputRefCount;
}

QString E131Controller::getNetworkIP()
{
    return m_ipAddr.toString();
}

void E131Controller::sendDmx(const quint32 universe, const QByteArray &data)
{
    QByteArray dmxPacket;
    m_packetizer->setupE131Dmx(dmxPacket, universe, data);
    if (m_multicastAddr.contains(universe) == false)
    {
        m_multicastAddr[universe] = QHostAddress(QString("239.255.0.%1").arg(universe + 1));
        qDebug() << "[E131Controller] Universe:" << universe <<
                    ", multicast address:" << m_multicastAddr[universe].toString() <<
                    "(MAC:" << m_MACAddress << ")";
        if (m_type == Input)
            m_UdpSocket->joinMulticastGroup(m_multicastAddr[universe]);
    }
    qint64 sent = m_UdpSocket->writeDatagram(dmxPacket.data(), dmxPacket.size(),
                                             m_multicastAddr[universe], E131_DEFAULT_PORT);
    if (sent < 0)
    {
        qDebug() << "sendDmx failed";
        qDebug() << "Errno: " << m_UdpSocket->error();
        qDebug() << "Errmgs: " << m_UdpSocket->errorString();
    }
    else
        m_packetSent++;
}

void E131Controller::processPendingPackets()
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
            if (m_packetizer->checkPacket(datagram) == true)
            {
                QByteArray dmxData;
                quint32 universe;
                if (this->type() == Input)
                {
                    m_packetReceived++;
                    if (m_packetizer->fillDMXdata(datagram, dmxData, universe) == true)
                    {
                        if (universe >= (quint32)m_inputRefCount)
                            break;

                        quint32 uniAddr = universe << 9;
                        for (quint32 i = 0; i < (quint32)dmxData.length(); i++)
                        {
                            if (m_dmxValues.at(uniAddr + i) != dmxData.at(i))
                            {
                                m_dmxValues[uniAddr + i] =  dmxData[i];
                                emit valueChanged(universe, m_line, i, (uchar)dmxData.at(i));
                            }
                        }
                    }
                }
            }
        }
    }
}
