/*
  Q Light Controller Plus
  osccontroller.cpp

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

#include "osccontroller.h"

#include <QMutexLocker>
#include <QByteArray>
#include <QDebug>

OSCController::OSCController(QString ipaddr, Type type, quint32 line, QObject *parent)
    : QObject(parent)
{
    m_ipAddr = QHostAddress(ipaddr);
    m_line = line;

    qDebug() << "[OSCController] type: " << type;
    m_packetizer.reset(new OSCPacketizer());
    m_packetSent = 0;
    m_packetReceived = 0;

    m_outputSocket = new QUdpSocket(this);
}

OSCController::~OSCController()
{
    qDebug() << Q_FUNC_INFO;
    //disconnect(m_outputSocket, SIGNAL(readyRead()), this, SLOT(processPendingPackets()));
    if (m_outputSocket->isOpen())
        m_outputSocket->close();
    QMapIterator<quint32, QByteArray *> it(m_dmxValuesMap);
    while(it.hasNext())
    {
        it.next();
        QByteArray *ba = it.value();
        delete ba;
    }
    m_dmxValuesMap.clear();
}

QHostAddress OSCController::getNetworkIP() const
{
    return m_ipAddr;
}

void OSCController::addUniverse(quint32 universe, OSCController::Type type)
{
    qDebug() << "[OSC] addUniverse - universe" << universe << ", type" << type;
    if (m_universeMap.contains(universe))
    {
        m_universeMap[universe].type |= (int)type;
    }
    else
    {
        UniverseInfo info;
        info.inputSocket = NULL;
        info.inputPort = 7700 + universe;
        if (m_ipAddr == QHostAddress::LocalHost)
        {
            info.feedbackAddress = QHostAddress::LocalHost;
            info.outputAddress = QHostAddress::LocalHost;
        }
        else
        {
            info.feedbackAddress = QHostAddress::Null;
            info.outputAddress = QHostAddress::Null;
        }
        info.feedbackPort = 9000 + universe;
        info.outputPort = 9000 + universe;
        info.type = type;
        m_universeMap[universe] = info;
        m_portsMap[info.inputPort] = universe;
    }

    QUdpSocket *socket = m_universeMap[universe].inputSocket;
    if (type == Input && socket == NULL)
    {
        quint16 inPort = m_universeMap[universe].inputPort;
        socket = new QUdpSocket(this);
        if (socket->bind(inPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint) == false)
        {
            qDebug() << "[OSCController] failed to bind socket to port" << inPort;
            delete m_universeMap[universe].inputSocket;
            m_universeMap[universe].inputSocket = NULL;
        }
        else
            connect(socket, SIGNAL(readyRead()),
                    this, SLOT(processPendingPackets()));
    }
}

void OSCController::removeUniverse(quint32 universe, OSCController::Type type)
{
    if (m_universeMap.contains(universe))
    {
        if (type == Input)
        {
            m_portsMap.take(m_universeMap[universe].inputPort);
            if (m_universeMap[universe].inputSocket != NULL)
            {
                disconnect(m_universeMap[universe].inputSocket, SIGNAL(readyRead()),
                           this, SLOT(processPendingPackets()));
                delete m_universeMap[universe].inputSocket;
            }
        }
        if (m_universeMap[universe].type == type)
            m_universeMap.take(universe);
        else
            m_universeMap[universe].type &= ~type;
    }
}

bool OSCController::setInputPort(quint32 universe, quint16 port)
{
    if (!m_universeMap.contains(universe))
        return false;

    QMutexLocker locker(&m_dataMutex);
    if (m_portsMap.contains(m_universeMap[universe].inputPort))
        m_portsMap.take(m_universeMap[universe].inputPort);
    m_universeMap[universe].inputPort = port;
    m_portsMap[port] = universe;

    QUdpSocket *socket = m_universeMap[universe].inputSocket;
    if (socket == NULL)
    {
        socket = new QUdpSocket(this);
        qDebug() << "[OSC] new input socket created on universe" << universe;
    }
    else
    {
        disconnect(socket, SIGNAL(readyRead()),
                this, SLOT(processPendingPackets()));
        qDebug() << "[OSC] socket disconnected from universe" << universe;
    }

    if (socket->bind(port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint) == false)
    {
        qDebug() << "[OSCController] failed to bind socket to port" << port;
        return false;
    }

    qDebug() << "[OSC] socket connected for universe" << universe;
    connect(socket, SIGNAL(readyRead()),
            this, SLOT(processPendingPackets()));
    m_universeMap[universe].inputSocket = socket;

    return port == 7700 + universe;
}

bool OSCController::setFeedbackIPAddress(quint32 universe, QString address)
{
    if (m_universeMap.contains(universe) == false)
        return false;

    QMutexLocker locker(&m_dataMutex);
    m_universeMap[universe].feedbackAddress = QHostAddress(address);

    if (m_ipAddr == QHostAddress::LocalHost)
        return QHostAddress(address) == QHostAddress::LocalHost;
    return QHostAddress(address) == QHostAddress::Null;
}

bool OSCController::setFeedbackPort(quint32 universe, quint16 port)
{
    if (m_universeMap.contains(universe) == false)
        return false;

    QMutexLocker locker(&m_dataMutex);
    if (m_universeMap.contains(universe))
        m_universeMap[universe].feedbackPort = port;

    return port == 9000 + universe;
}

bool OSCController::setOutputIPAddress(quint32 universe, QString address)
{
    if (m_universeMap.contains(universe) == false)
        return false;

    QMutexLocker locker(&m_dataMutex);
    m_universeMap[universe].outputAddress = QHostAddress(address);

    if (m_ipAddr == QHostAddress::LocalHost)
        return QHostAddress(address) == QHostAddress::LocalHost;
    return QHostAddress(address) == QHostAddress::Null;
}

bool OSCController::setOutputPort(quint32 universe, quint16 port)
{
    if (m_universeMap.contains(universe) == false)
        return false;

    QMutexLocker locker(&m_dataMutex);
    if (m_universeMap.contains(universe))
        m_universeMap[universe].outputPort = port;

    return port == 9000 + universe;
}

QList<quint32> OSCController::universesList() const
{
    return m_universeMap.keys();
}

UniverseInfo* OSCController::getUniverseInfo(quint32 universe)
{
    if (m_universeMap.contains(universe))
        return &m_universeMap[universe];

    return NULL;
}

OSCController::Type OSCController::type() const
{
    int type = Unknown;
    foreach(UniverseInfo info, m_universeMap.values())
    {
        type |= info.type;
    }

    return Type(type);
}

quint32 OSCController::line() const
{
    return m_line;
}

quint64 OSCController::getPacketSentNumber() const
{
    return m_packetSent;
}

quint64 OSCController::getPacketReceivedNumber() const
{
    return m_packetReceived;
}

quint16 OSCController::getHash(QString path)
{
    quint16 hash;
    if (m_hashMap.contains(path))
        hash = m_hashMap[path];
    else
    {
        /** No existing hash found. Add a new key to the table */
        hash = qChecksum(path.toUtf8().data(), path.length());
        m_hashMap[path] = hash;
    }

    return hash;
}

void OSCController::sendDmx(const quint32 universe, const QByteArray &dmxData)
{
    QMutexLocker locker(&m_dataMutex);
    QByteArray dmxPacket;
    QHostAddress outAddress = QHostAddress::Null;
    quint32 outPort = 7700 + universe;

    if (m_universeMap.contains(universe))
    {
        outAddress = m_universeMap[universe].outputAddress;
        outPort = m_universeMap[universe].outputPort;
    }

    for (int i = 0; i < dmxData.length(); i++)
    {
        if (m_dmxValuesMap.contains(universe) == false)
            m_dmxValuesMap[universe] = new QByteArray(512, 0);

        QByteArray *dmxValues = m_dmxValuesMap[universe];

        if (dmxData[i] != dmxValues->at(i))
        {
            dmxValues->replace(i, 1, (const char *)(dmxData.data() + i), 1);
            m_packetizer->setupOSCDmx(dmxPacket, universe, i, dmxData[i]);
            qint64 sent = m_outputSocket->writeDatagram(dmxPacket.data(), dmxPacket.size(),
                                                     outAddress, outPort);
            if (sent < 0)
            {
                qDebug() << "[OSC] sendDmx failed. Errno: " << m_outputSocket->error();
                qDebug() << "Errmgs: " << m_outputSocket->errorString();
            }
            else
                m_packetSent++;
        }
    }
}

void OSCController::sendFeedback(const quint32 universe, quint32 channel, uchar value, const QString &key)
{
    QMutexLocker locker(&m_dataMutex);
    QHostAddress outAddress = QHostAddress::Null;
    quint32 outPort = 9000 + universe;

    if (m_universeMap.contains(universe))
    {
        outAddress = m_universeMap[universe].feedbackAddress;
        outPort = m_universeMap[universe].feedbackPort;
    }

    QString path = key;
    // on invalid key try to retrieve the OSC path from the hash table.
    // This works only if the OSC widget has been previously moved by the user
    if (key.isEmpty())
        path = m_hashMap.key(channel);

    qDebug() << "[OSC] sendFeedBack - Key:" << path << "value:" << value;

    QByteArray values;
    QByteArray oscPacket;

    // multiple value path
    if (path.length() > 2 && path.at(path.length() - 2) == '_')
    {
        int valIdx = QString(path.at(path.length() - 1)).toInt();
        path.chop(2);
        if (m_universeMap[universe].multipartCache.contains(path) == false)
        {
            qDebug() << "[OSC] Multi-value path NOT in cache. Allocating default.";
            m_universeMap[universe].multipartCache[path] = QByteArray((int)2, (char)0);
        }

        values = m_universeMap[universe].multipartCache[path];
        if (values.count() <= valIdx)
            values.resize(valIdx + 1);
        values[valIdx] = (char)value;
        m_universeMap[universe].multipartCache[path] = values;

        //qDebug() << "Values to send:" << QString::number((uchar)values.at(0)) <<
        //            QString::number((uchar)values.at(1)) << values.count();
    }
    else
        values.append((char)value); // single value path

    QString pTypes;
    pTypes.fill('f', values.count());

    m_packetizer->setupOSCGeneric(oscPacket, path, pTypes, values);
    qint64 sent = m_outputSocket->writeDatagram(oscPacket.data(), oscPacket.size(),
                                             outAddress, outPort);
    if (sent < 0)
    {
        qDebug() << "[OSC] sendDmx failed. Errno: " << m_outputSocket->error();
        qDebug() << "Errmgs: " << m_outputSocket->errorString();
    }
    else
        m_packetSent++;
}

void OSCController::processPendingPackets()
{
    QUdpSocket *socket = qobject_cast<QUdpSocket *>(sender());
    while (socket->hasPendingDatagrams())
    {
        QByteArray datagram;
        QHostAddress senderAddress;
        datagram.resize(socket->pendingDatagramSize());
        socket->readDatagram(datagram.data(), datagram.size(), &senderAddress);
        //if (senderAddress != m_ipAddr)
        {
            //qDebug() << "Received packet with size: " << datagram.size() << ", host: " << senderAddress.toString();
            QList< QPair<QString, QByteArray> > messages = m_packetizer->parsePacket(datagram);

            QListIterator <QPair<QString,QByteArray> > it(messages);
            while (it.hasNext() == true)
            {
                QPair <QString,QByteArray> msg(it.next());

                QString path = msg.first;
                QByteArray values = msg.second;

                //qDebug() << "[OSC] message has path:" << path << "values:" << values.count();
                if (values.isEmpty())
                    continue;

                quint16 port = socket->localPort();
                if (m_portsMap.contains(port))
                {
                    if (values.count() > 1)
                    {
                        quint32 uni = m_portsMap[port];
                        if (m_universeMap.contains(uni))
                        {
                            //m_universeMap[uni].multipartCache[path].resize(values.count());
                            m_universeMap[uni].multipartCache[path] = values;
                        }
                        for(int i = 0; i < values.count(); i++)
                        {
                            QString modPath = QString("%1_%2").arg(path).arg(i);
                            emit valueChanged(m_portsMap[port], m_line, getHash(modPath), (uchar)values.at(i), modPath);
                        }
                    }
                    else
                        emit valueChanged(m_portsMap[port], m_line, getHash(path), (uchar)values.at(0), path);
                }
            }
            m_packetReceived++;
        }
    }
}
