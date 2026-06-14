/*
  Q Light Controller Plus
  idnclient.cpp

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
#include "idnclient.h"
#include "idn.h"

QTextStream out32(stdout);

IdnClient::IdnClient(QHostAddress const &clientAddress, QSharedPointer<QUdpSocket> const& udpSocket,
                     QSharedPointer<QMutex> const& socketMutex,
                     int const& port, int const& rangeBegin, int const& rangeEnd, int const& mode, int const& channelID, int const& serviceId)
  : m_address(clientAddress)
  , m_udpSocket(udpSocket)
  , m_socketMutex(socketMutex)
  , m_port(port)
  , m_rangeBegin(rangeBegin)
  , m_rangeEnd(rangeEnd)
  , m_mode(mode)
  , m_channelID(channelID)
  , m_serviceID(serviceId)

  , m_packetizer(new IdnPacketizer())
  , m_optimizer(new IdnOptimizer())
  , m_seqnum(0)
  , m_packetSent(0)
{
    //first packet has to be a config packet
  config = true;
  //init timestamp
  timestamp = QDateTime::currentMSecsSinceEpoch();
  lastsend = 0;
  blackCounter = 0;

  if (m_rangeBegin < 1){
	  m_rangeBegin = 1;
  }
  if (m_rangeEnd > 512){
	  m_rangeEnd = 512;
  }
  //timer to close the connection in case of a blackout
  closeTimer = new QTimer(this);
  closeTimer->setInterval(IDN_TIMEOUT);
  closeTimer->setSingleShot(true);
  connect(closeTimer, SIGNAL(timeout()), this, SLOT(sendClosePacket()));
}

IdnClient::~IdnClient()
{
	sendClosePacket();
}

void IdnClient::sendClosePacket()
{
  QByteArray dmxPacket;
  m_packetizer->generateClosePacket(dmxPacket, m_seqnum, m_channelID, m_serviceID);
  QMutexLocker socketLocker(m_socketMutex.data());
  qint64 sent = m_udpSocket->writeDatagram(dmxPacket, m_address, m_port);
  lastsend = QDateTime::currentMSecsSinceEpoch();
  if (sent < 0){
    qWarning() << "sendDmx failed";
    qWarning() << "Errno: " << m_udpSocket->error();
    qWarning() << "Errmgs: " << m_udpSocket->errorString();
  }else{
    m_packetSent++;
    m_seqnum++;
  }
}

QByteArray IdnClient::optimizedMode(const QByteArray &data)
{
  // Detect blackout: all 512 channels are zero — send a null packet
  if(data.count((char)0x0) == 512){
    QByteArray blackoutpacket;
    m_packetizer->generateNullPacket(blackoutpacket, m_seqnum, m_mode, m_channelID, m_serviceID);
    return blackoutpacket;
  }

  bool configTime = (QDateTime::currentMSecsSinceEpoch() - timestamp) > IDN_CONFIG_INTERVAL ? true : false;

  IdnOptimizer::PacketInformation pi = m_optimizer->optimize(data, configTime);
  //setze den Datachunk zusammen
  QByteArray dmxPacket, justifiedData;
  bool addRangeEndFrame = true;

  for(int i = 0; i < pi.ranges.length(); i++){
    for(int j = pi.ranges[i].first; j < pi.ranges[i].second+1; j++){
      if(pi.ranges[i].first == m_rangeEnd || pi.ranges[i].second == m_rangeEnd){
        addRangeEndFrame = false;
      }
      if (j < data.size()){
        justifiedData.append(data[j]);
      } else {
        justifiedData.append((char)0x0);
      }
    }
  }

  if(addRangeEndFrame){
    if(data.size() >= m_rangeEnd){
      justifiedData.append(data[m_rangeEnd-1]);
    }
    else{
      justifiedData.append((char)0x0);
    }
    pi.byteCount++;
    pi.numberOfSingleChannels++;
    QPair<int, int> finalFrame;
    finalFrame.first = m_rangeEnd-1;
    finalFrame.second = m_rangeEnd-1;
    pi.ranges.append(finalFrame);   
  }

  bool newConfig = false;
  if(pi.ranges.length() != oldpi.ranges.length()){
    newConfig = true;
  }else{
    for(int i = 0; i < pi.ranges.length(); i++){
      if(pi.ranges[i] == oldpi.ranges[i]){
        continue;
      }else{
        newConfig = true;
        break;
      }
    }
  }
  if(!newConfig){
    config = false;
    if(configTime){
      config = true;
    }else{
      config = false;
    }
  }else{
    config = true;
  }

  oldData = justifiedData;
  oldpi = pi;

  m_packetizer->setupIdnDmx(dmxPacket, m_mode, m_channelID, justifiedData, pi.ranges, m_seqnum, config, m_serviceID);

  if(config)
    timestamp = QDateTime::currentMSecsSinceEpoch();
  return dmxPacket; 
}

QByteArray IdnClient::rangeMode(const QByteArray &data)
{
  QByteArray dmxPacket;

  QByteArray universeRange(m_rangeEnd, (char)0x0);

    universeRange.replace(0, data.length(), data);
  QByteArray justifiedData = universeRange.right(m_rangeEnd-m_rangeBegin+1);

  if((QDateTime::currentMSecsSinceEpoch() - timestamp) > IDN_CONFIG_INTERVAL || m_packetSent == 0){
    config = true;
  }else{
    config = false;
  }

  m_packetizer->setupIdnDmx(dmxPacket, m_mode, m_channelID, justifiedData, m_rangeBegin, m_seqnum, config, m_serviceID);

  if(config)
    timestamp = QDateTime::currentMSecsSinceEpoch();
  
  return dmxPacket;   
}

/*****************************************************************************
 * Checks whether a config packet is necessary and calls the Packetizer to build
   the packet. Then the function sends the packet to the specific receiver.
 *****************************************************************************/
void IdnClient::sendDmx(const QByteArray &data)
{

  QMutexLocker locker(&m_dataMutex);
  if(closeTimer->isActive()){
    closeTimer->stop();
  }
  QByteArray dmxPacket;

  if(m_seqnum == 0){
      m_packetizer->generateNullPacket(dmxPacket, m_seqnum, m_mode, m_channelID, m_serviceID);
      QMutexLocker socketLocker(m_socketMutex.data());
      qint64 sent = m_udpSocket->writeDatagram(dmxPacket, m_address, m_port);
      lastsend = QDateTime::currentMSecsSinceEpoch();
        
      if (sent < 0){
          qWarning() << "sendDmx failed";
          qWarning() << "Errno: " << m_udpSocket->error();
          qWarning() << "Errmgs: " << m_udpSocket->errorString();
      }else{
        m_packetSent++;
        m_seqnum++;
      }
  }

  

  //count the 512 Byte long data chunks. The first packet is allowed to pass the throttle because the chunk can be a blackout chunk
  data.count((char)0x0) == 512 ? blackCounter++ : blackCounter = 0;

  if(m_mode == 5 || m_mode == 7){
    // Throttle: minimum gap = (overhead + slot_time * numChannels) converted from µs to ms
    const int minGapMs = (IDN_PACKET_OVERHEAD_US + IDN_DMX_SLOT_TIME_US * m_rangeEnd) / 1000;
    if((QDateTime::currentMSecsSinceEpoch() - lastsend) > minGapMs || blackCounter == 1){
      if(blackCounter == 1){
        m_packetizer->generateNullPacket(dmxPacket, m_seqnum, m_mode, m_channelID, m_serviceID);
      }else{
        dmxPacket = optimizedMode(data);
      }
      QMutexLocker socketLocker(m_socketMutex.data());
      qint64 sent = m_udpSocket->writeDatagram(dmxPacket, m_address, m_port);
      if (sent < 0){
        qWarning() << "sendDmx (optimized) failed, error:" << m_udpSocket->errorString();
      }
      else{
        lastsend = QDateTime::currentMSecsSinceEpoch();
        m_packetSent++;
        m_seqnum++;
      }
    }
  }else{
    // Throttle: minimum gap based on actual channel range
    const int minGapMs = (IDN_PACKET_OVERHEAD_US + IDN_DMX_SLOT_TIME_US * (m_rangeEnd - m_rangeBegin)) / 1000;
    if((QDateTime::currentMSecsSinceEpoch() - lastsend) > minGapMs || blackCounter == 1){
      if(blackCounter == 1){
        m_packetizer->generateNullPacket(dmxPacket, m_seqnum, m_mode, m_channelID, m_serviceID);
      }else{
        dmxPacket = rangeMode(data);
      }
      QMutexLocker socketLocker(m_socketMutex.data());
      qint64 sent = m_udpSocket->writeDatagram(dmxPacket, m_address, m_port);
      if (sent < 0){
        qWarning() << "sendDmx (range) failed, error:" << m_udpSocket->errorString();
      }
      else{
        lastsend = QDateTime::currentMSecsSinceEpoch();
        m_packetSent++;
        m_seqnum++;
      }
    }
  }
  if(data.count((char)0x0) == 512){
    //start Blackouttimer
    closeTimer->start();
  }
}

quint64 IdnClient::getPacketSentNumber()
{
    return m_packetSent;
}
