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
#include <QVariant>
#include <QDebug>

#define TRANSMIT_FULL    "Full"
#define TRANSMIT_PARTIAL "Partial"

E131Controller::E131Controller(QNetworkInterface const& iface, QNetworkAddressEntry const& address,
                               quint32 line, QObject *parent)
    : QObject(parent)
    , m_interface(iface)
    , m_ipAddr(address.ip())
    , m_packetSent(0)
    , m_packetReceived(0)
    , m_line(line)
    , m_UdpSocket(new QUdpSocket(this))
    , m_packetizer(new E131Packetizer(iface.hardwareAddress()))
{
    qDebug() << Q_FUNC_INFO;
    m_UdpSocket->bind(m_ipAddr, 0);
    // Output multicast on the correct interface
    m_UdpSocket->setMulticastInterface(m_interface);
    // Don't send multicast to self
    m_UdpSocket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, false);
}

E131Controller::~E131Controller()
{
    qDebug() << Q_FUNC_INFO;
    qDeleteAll(m_dmxValuesMap);
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
        info.inputMulticast = true;
        info.inputMcastAddress = QHostAddress(QString("239.255.0.%1").arg(universe + 1));
        info.inputUcastPort = E131_DEFAULT_PORT;
        info.inputUniverse = universe + 1;
        info.inputSocket.clear();
        info.outputMulticast = true;
        info.outputMcastAddress = QHostAddress(QString("239.255.0.%1").arg(universe + 1));
        if (m_ipAddr != QHostAddress::LocalHost)
            info.outputUcastAddress = QHostAddress(quint32((m_ipAddr.toIPv4Address() & 0xFFFFFF00) + (universe + 1)));
        else
            info.outputUcastAddress = m_ipAddr;
        info.outputUcastPort = E131_DEFAULT_PORT;
        info.outputUniverse = universe + 1;
        info.outputTransmissionMode = Full;
        info.outputPriority = E131_PRIORITY_DEFAULT;
        info.type = type;
        m_universeMap[universe] = info;
    }

    if (type == Input)
    {
        UniverseInfo& info = m_universeMap[universe];
        info.inputSocket.clear();
        info.inputSocket = getInputSocket(true, info.inputMcastAddress, E131_DEFAULT_PORT);
    }
}

void E131Controller::removeUniverse(quint32 universe, E131Controller::Type type)
{
    if (m_universeMap.contains(universe))
    {
        UniverseInfo& info = m_universeMap[universe];
        if (type == Input)
            info.inputSocket.clear();

        if (info.type == type)
            m_universeMap.take(universe);
        else
            info.type &= ~type;
    }
}

void E131Controller::setInputMulticast(quint32 universe, bool multicast)
{
    if (m_universeMap.contains(universe) == false)
        return;

    QMutexLocker locker(&m_dataMutex);
    UniverseInfo& info = m_universeMap[universe];

    if (info.inputMulticast == multicast)
        return;
    info.inputMulticast = multicast;

    info.inputSocket.clear();
    if (multicast)
        info.inputSocket = getInputSocket(true, info.inputMcastAddress, E131_DEFAULT_PORT);
    else
        info.inputSocket = getInputSocket(false, m_ipAddr, info.inputUcastPort);
}

QSharedPointer<QUdpSocket> E131Controller::getInputSocket(bool multicast, QHostAddress const& address, quint16 port)
{
    foreach (UniverseInfo const& info, m_universeMap)
    {
        if (info.inputSocket && info.inputMulticast == multicast)
        {
            if (info.inputMulticast && info.inputMcastAddress == address)
                return info.inputSocket;
            if (!info.inputMulticast && info.inputUcastPort == port)
                return info.inputSocket;
        }
    }

    QSharedPointer<QUdpSocket> inputSocket(new QUdpSocket(this));

    if (multicast)
    {
        inputSocket->bind(QHostAddress::AnyIPv4, E131_DEFAULT_PORT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
        inputSocket->joinMulticastGroup(address, m_interface);
    }
    else
    {
        inputSocket->bind(m_ipAddr, port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    }

    connect(inputSocket.data(), SIGNAL(readyRead()),
            this, SLOT(processPendingPackets()));

    return inputSocket;
}

void E131Controller::setInputMCastAddress(quint32 universe, QString address, bool legacy)
{
    if (m_universeMap.contains(universe) == false)
        return;

    QMutexLocker locker(&m_dataMutex);
    UniverseInfo& info = m_universeMap[universe];

    QHostAddress newAddress = legacy ?
        QHostAddress(QString("239.255.0.%1").arg(address)) : QHostAddress(address);

    if (info.inputMcastAddress == newAddress)
        return;

    info.inputMcastAddress = newAddress;

    if (!info.inputMulticast)
    {
        info.inputSocket.clear();
        info.inputSocket = getInputSocket(true, info.inputMcastAddress, E131_DEFAULT_PORT);
    }
}

void E131Controller::setInputUCastPort(quint32 universe, quint16 port)
{
    if (m_universeMap.contains(universe) == false)
        return;

    QMutexLocker locker(&m_dataMutex);
    UniverseInfo& info = m_universeMap[universe];

    if (info.inputUcastPort == port)
        return;
    info.inputUcastPort = port;

    if (!info.inputMulticast)
    {
        info.inputSocket.clear();
        info.inputSocket = getInputSocket(false, m_ipAddr, info.inputUcastPort);
    }
}

void E131Controller::setInputUniverse(quint32 universe, quint32 e131Uni)
{
    if (m_universeMap.contains(universe) == false)
        return;

    QMutexLocker locker(&m_dataMutex);
    UniverseInfo& info = m_universeMap[universe];

    if (info.inputUniverse == e131Uni)
        return;
    info.inputUniverse = e131Uni;
}

void E131Controller::setOutputMulticast(quint32 universe, bool multicast)
{
    if (m_universeMap.contains(universe) == false)
        return;

    QMutexLocker locker(&m_dataMutex);
    m_universeMap[universe].outputMulticast = multicast;
}

void E131Controller::setOutputMCastAddress(quint32 universe, QString address, bool legacy)
{
    if (m_universeMap.contains(universe) == false)
        return;

    QMutexLocker locker(&m_dataMutex);
    m_universeMap[universe].outputMcastAddress = legacy ?
        QHostAddress(QString("239.255.0.%1").arg(address)) : QHostAddress(address);
}

void E131Controller::setOutputUCastAddress(quint32 universe, QString address)
{
    if (m_universeMap.contains(universe) == false)
        return;

    QMutexLocker locker(&m_dataMutex);
    m_universeMap[universe].outputUcastAddress = QHostAddress(address);
}

void E131Controller::setOutputUCastPort(quint32 universe, quint16 port)
{
    if (m_universeMap.contains(universe) == false)
        return;

    QMutexLocker locker(&m_dataMutex);
    m_universeMap[universe].outputUcastPort = port;
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

void E131Controller::setOutputTransmissionMode(quint32 universe, E131Controller::TransmissionMode mode)
{
    if (m_universeMap.contains(universe) == false)
        return;

    QMutexLocker locker(&m_dataMutex);
    m_universeMap[universe].outputTransmissionMode = int(mode);
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
    foreach (UniverseInfo info, m_universeMap.values())
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
    quint16 outPort = E131_DEFAULT_PORT;
    quint32 outUniverse = universe;
    quint32 outPriority = E131_PRIORITY_DEFAULT;
    TransmissionMode transmitMode = Full;

    if (m_universeMap.contains(universe))
    {
        UniverseInfo const& info = m_universeMap[universe];
        if (info.outputMulticast)
        {
            outAddress = info.outputMcastAddress;
        }
        else
        {
            outAddress = info.outputUcastAddress;
            outPort = info.outputUcastPort;
        }
        outUniverse = info.outputUniverse;
        outPriority = info.outputPriority;
        transmitMode = TransmissionMode(info.outputTransmissionMode);
    }
    else
        qWarning() << Q_FUNC_INFO << "universe" << universe << "unknown";

    if (transmitMode == Full)
    {
        QByteArray wholeuniverse(512, 0);
        wholeuniverse.replace(0, data.length(), data);
        m_packetizer->setupE131Dmx(dmxPacket, outUniverse, outPriority, wholeuniverse);
    }
    else
        m_packetizer->setupE131Dmx(dmxPacket, outUniverse, outPriority, data);

    qint64 sent = m_UdpSocket->writeDatagram(dmxPacket.data(), dmxPacket.size(),
                                             outAddress, outPort);
    if (sent < 0)
    {
        qDebug() << "sendDmx failed";
        qDebug() << "Errno: " << m_UdpSocket->error();
        qDebug() << "Errmsg: " << m_UdpSocket->errorString();
    }
    else
        m_packetSent++;
}

void E131Controller::processPendingPackets()
{
    QUdpSocket* socket = qobject_cast<QUdpSocket*>(sender());
    Q_ASSERT(socket != NULL);

    while (socket->hasPendingDatagrams())
    {
        QByteArray datagram;
        QHostAddress senderAddress;
        datagram.resize(socket->pendingDatagramSize());
        socket->readDatagram(datagram.data(), datagram.size(), &senderAddress);

        QByteArray dmxData;
        quint32 e131universe;

        if (m_packetizer->checkPacket(datagram) &&
            m_packetizer->fillDMXdata(datagram, dmxData, e131universe))
        {
            qDebug() << "Received packet with size: " << datagram.size() << ", from: " << senderAddress.toString()
                << ", for E1.31 universe: " << e131universe;
            ++m_packetReceived;

            for (QMap<quint32, UniverseInfo>::iterator it = m_universeMap.begin(); it != m_universeMap.end(); ++it)
            {
                quint32 universe = it.key();
                UniverseInfo const& info = it.value();
                if (info.inputSocket == socket && info.inputUniverse == e131universe)
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
        else
        {
            qDebug() << "Received packet with size: " << datagram.size() << ", from: " << senderAddress.toString()
                << ", that does not look like E1.31";
        }
    }
}
