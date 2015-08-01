/*
  Q Light Controller Plus
  e131controller.cpp

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

#include <QMutexLocker>
#include <QDebug>

#define TRANSMIT_FULL    "Full"
#define TRANSMIT_PARTIAL "Partial"

E131Controller::E131Controller(QString ipaddr, Type type, quint32 line, QObject *parent)
    : QObject(parent)
{
    m_ipAddr = QHostAddress(ipaddr);
    m_line = line;

    qDebug() << "[E131Controller] type: " << type;
    m_packetizer.reset(new E131Packetizer());
    m_packetSent = 0;
    m_packetReceived = 0;

    m_UdpSocket = new QUdpSocket(this);

    // reset initial DMX values if we're an input
    if (type == Input)
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        if (m_UdpSocket->bind(QHostAddress::AnyIPv4, E131_DEFAULT_PORT,
                              QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint) == false)
#else
        if (m_UdpSocket->bind(QHostAddress::Any, E131_DEFAULT_PORT,
                              QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint) == false)
#endif
        {
            qDebug() << Q_FUNC_INFO << "Socket input bind failed !!";
            return;
        }
    }
    else
    {
        if (m_UdpSocket->bind(m_ipAddr, E131_DEFAULT_PORT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint) == false)
        {
            qDebug() << Q_FUNC_INFO << "Socket output bind failed !!";
            return;
        }
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
    QMapIterator<int, QByteArray *> it(m_dmxValuesMap);
    while(it.hasNext())
    {
        it.next();
        QByteArray *ba = it.value();
        delete ba;
    }
    m_dmxValuesMap.clear();
}

QString E131Controller::getNetworkIP()
{
    return m_ipAddr.toString();
}

void E131Controller::addUniverse(quint32 universe, E131Controller::Type type)
{
    qDebug() << "[E1.31] addUniverse - universe" << universe << ", type" << type;
    if (m_universeMap.contains(universe))
    {
        m_universeMap[universe].type |= (int)type;
    }
    else
    {
        UniverseInfo info;
        if (m_ipAddr == QHostAddress::LocalHost)
            info.mcastAddress = QHostAddress::LocalHost;
        else
            info.mcastAddress = QHostAddress(QString("239.255.0.%1").arg(universe + 1));
        info.outputUniverse = universe;
        info.outputPriority = 100;
        info.trasmissionMode = Full;
        info.type = type;
        m_universeMap[universe] = info;
    }
    if (type == Input)
        m_UdpSocket->joinMulticastGroup(m_universeMap[universe].mcastAddress);
}

void E131Controller::removeUniverse(quint32 universe, E131Controller::Type type)
{
    if (m_universeMap.contains(universe))
    {
        if (m_universeMap[universe].type == type)
            m_universeMap.take(universe);
        else
            m_universeMap[universe].type &= ~type;
    }
}

void E131Controller::setIPAddress(quint32 universe, QString address)
{
    if (m_universeMap.contains(universe) == false)
        return;

    QMutexLocker locker(&m_dataMutex);
    m_universeMap[universe].mcastAddress = QHostAddress(QString("239.255.0.%1").arg(address));
}

void E131Controller::setOutputUniverse(quint32 universe, quint32 e131Uni)
{
    if (m_universeMap.contains(universe) == false)
        return;

    QMutexLocker locker(&m_dataMutex);
    m_universeMap[universe].outputUniverse = e131Uni;
}

void E131Controller::setOutputPriority(quint32 universe, quint32 e131Priority)
{
    if (m_universeMap.contains(universe) == false)
        return;

    QMutexLocker locker(&m_dataMutex);
    m_universeMap[universe].outputPriority = e131Priority;
}

void E131Controller::setTransmissionMode(quint32 universe, E131Controller::TransmissionMode mode)
{
    if (m_universeMap.contains(universe) == false)
        return;

    QMutexLocker locker(&m_dataMutex);
    m_universeMap[universe].trasmissionMode = int(mode);
}

QString E131Controller::transmissionModeToString(E131Controller::TransmissionMode mode)
{
    switch (mode)
    {
        default:
        case Full:
            return QString(TRANSMIT_FULL);
        break;
        case Partial:
            return QString(TRANSMIT_PARTIAL);
        break;
    }
}

E131Controller::TransmissionMode E131Controller::stringToTransmissionMode(const QString &mode)
{
    if (mode == QString(TRANSMIT_PARTIAL))
        return Partial;
    else
        return Full;
}

QList<quint32> E131Controller::universesList()
{
    return m_universeMap.keys();
}

UniverseInfo *E131Controller::getUniverseInfo(quint32 universe)
{
    if (m_universeMap.contains(universe))
        return &m_universeMap[universe];

    return NULL;
}

E131Controller::Type E131Controller::type()
{
    int type = Unknown;
    foreach(UniverseInfo info, m_universeMap.values())
    {
        type |= info.type;
    }

    return Type(type);
}

quint32 E131Controller::line()
{
    return m_line;
}

quint64 E131Controller::getPacketSentNumber()
{
    return m_packetSent;
}

quint64 E131Controller::getPacketReceivedNumber()
{
    return m_packetReceived;
}

void E131Controller::sendDmx(const quint32 universe, const QByteArray &data)
{
    QMutexLocker locker(&m_dataMutex);
    QByteArray dmxPacket;
    QHostAddress outAddress = QHostAddress(QString("239.255.0.%1").arg(universe + 1));
    quint32 outUniverse = universe;
    quint32 outPriority = 100;
    TransmissionMode transmitMode = Full;

    if (m_universeMap.contains(universe))
    {
        UniverseInfo info = m_universeMap[universe];
        outAddress = info.mcastAddress;
        outUniverse = info.outputUniverse;
        outPriority = info.outputPriority;
        transmitMode = TransmissionMode(info.trasmissionMode);
    }

    if (transmitMode == Full)
    {
        QByteArray wholeuniverse(512, 0);
        wholeuniverse.replace(0, data.length(), data);
        m_packetizer->setupE131Dmx(dmxPacket, outUniverse, outPriority, wholeuniverse);
    }
    else
        m_packetizer->setupE131Dmx(dmxPacket, outUniverse, outPriority, data);

    qint64 sent = m_UdpSocket->writeDatagram(dmxPacket.data(), dmxPacket.size(),
                                             outAddress, E131_DEFAULT_PORT);
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
                        QByteArray *dmxValues;
                        if (m_dmxValuesMap.contains(universe) == false)
                            m_dmxValuesMap[universe] = new QByteArray(512, 0);
                        dmxValues = m_dmxValuesMap[universe];

                        for (int i = 0; i < dmxData.length(); i++)
                        {
                            if (dmxValues->at(i) != dmxData.at(i))
                            {
                                dmxValues->replace(i, 1, (const char *)(dmxData.data() + i), 1);
                                emit valueChanged(universe, m_line, i, (uchar)dmxData.at(i));
                            }
                        }
                    }
                }
            }
        }
    }
}
