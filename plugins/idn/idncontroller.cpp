/*
  Q Light Controller Plus
  idncontroller.cpp

  Copyright (c) Daniel Schröder

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

#include <QDateTime>
#include <QThread>
#include <QtAlgorithms>
#include <QList>
#include <QPair>
#include <QMap>
#include <QFileInfo>
#include <QSettings>
#include <QHash>
#include <QHashIterator>
#include <QMutableHashIterator>
#include <QTextStream>

#include "idncontroller.h"
#include "idn.h"

IdnController::IdnController(QNetworkAddressEntry const &address, quint32 line, QHash<IdnHostInfo, IdnSettings> settings, QObject *parent)
    : QObject(parent), m_ipAddr(address.ip()), m_packetSent(0), m_line(line)
    , m_socket(new QUdpSocket(this)), m_packetizer(new IdnPacketizer())
    , m_fileSettings(settings), m_socketMutex(new QMutex())
{
    m_broadcastAddr = address.broadcast();
    m_prefixLength = address.prefixLength();
    m_scanSocket = new QUdpSocket(this);

    m_socket->bind(m_ipAddr, 0, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    initClients();
}

IdnController::~IdnController()
{
}

void IdnController::initClients()
{
    QHashIterator<IdnHostInfo, IdnSettings> settings(m_fileSettings);
    while (settings.hasNext())
    {
        settings.next();
        if (settings.value().disabled == false)
        {
            IdnClientInfo client;
            client.port = settings.value().port;
            client.mode = settings.value().mode;
            client.idnChannel = settings.value().idnChannel;
            client.rangeBegin = settings.value().rangeBegin;
            client.rangeEnd = settings.value().rangeEnd;
            client.universe = settings.value().universe;
            client.iface = settings.value().iface;
            client.serviceID = settings.value().serviceID;
            client.client = new IdnClient(settings.key().address, m_socket, m_socketMutex, client.port,
                                          client.rangeBegin, client.rangeEnd, client.mode, client.idnChannel, client.serviceID);
            m_clientsList[settings.key()] = client;
        }
    }
}

/*****************************************************************************
 * Checks wheater a config packet is necessary and calls the Packetizer to build
   the packet. Then the function sends the packet to the specific receiver.
 *****************************************************************************/
void IdnController::handleDmx(const quint32 universe, const QByteArray &data)
{
    QHashIterator<IdnHostInfo, IdnClientInfo> client(getClientsList());
    quint64 t_packetSend = 0;
    while (client.hasNext())
    {
        client.next();
        if (client.value().universe - 1 == universe && client.value().iface == m_ipAddr)
        {
            client.value().client->sendDmx(data);
            t_packetSend += client.value().client->getPacketSentNumber();
        }
    }
    m_packetSent = t_packetSend;
}

QString IdnController::getNetworkIP()
{
    return m_ipAddr.toString();
}

QHash<IdnHostInfo, IdnClientInfo> IdnController::getClientsList()
{
    return m_clientsList;
}

void IdnController::addUniverse(quint32 universe)
{
    qDebug() << "[IDN] addUniverse - universe" << universe;
    if (!m_universeMap.contains(universe))
    {
        UniverseInfo info;
        info.outputAddress = m_broadcastAddr;
        info.outputUniverse = universe;
        m_universeMap[universe] = info;
    }
    // hier muss der Scan nach Devices aktiviert werden
}

void IdnController::removeUniverse(quint32 universe)
{
    if (m_universeMap.contains(universe))
    {
        m_universeMap.take(universe);
    }
}

QList<quint32> IdnController::universesList()
{
    return m_universeMap.keys();
}

UniverseInfo *IdnController::getUniverseInfo(quint32 universe)
{
    if (m_universeMap.contains(universe))
        return &m_universeMap[universe];

    return NULL;
}

quint32 IdnController::line()
{
    return m_line;
}

quint64 IdnController::getPacketSentNumber()
{
    return m_packetSent;
}

/*****************************************************************************
 * Initializes the ini-File (if its not exists yet)
 *****************************************************************************/

bool IdnController::closeByUniverse(quint32 universe)
{
    QMutableHashIterator<IdnHostInfo, IdnClientInfo> clients(m_clientsList);
    while (clients.hasNext())
    {
        clients.next();
        if (clients.value().universe - 1 == universe)
        {
            delete clients.value().client;
            clients.remove();
        }
    }
    return m_clientsList.isEmpty();
}

