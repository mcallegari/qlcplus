/*
  Q Light Controller Plus
  idnpacketizer.cpp

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
#include <QtEndian>


#ifdef _WIN32
    #include <Winsock2.h> //for htons, htonl
#else
  #include <arpa/inet.h> //for htons, htonl
#endif

#include "idnpacketizer.h"

/*********************************************************************
 * Build common header in Constructor
 *********************************************************************/
IdnPacketizer::IdnPacketizer(){}

IdnPacketizer::~IdnPacketizer(){}

/*********************************************************************
 * Build IDN Packet
 *********************************************************************/

// bekommt setupIdnDmx(dmxPacket, m_mode, justifiedData, NULL, m_seqnum, true);

 void IdnPacketizer::setupIdnDmx(QByteArray& data, const quint8 &mode, const quint8 &channelID, const QByteArray &values, const quint16 offset, const quint32 seqnum, const bool config, const quint8 serviceID)
 {
    data.clear();
    IDNDMX_PACKET packet;
    InitDMXPacket(&packet);
    setPacketHeader(&packet, IDNCMD_MESSAGE, seqnum);
    buildChannelMessage(&packet, channelID);
    if(config){
      if(mode == 4 || mode == 5){
        buildConfigHeader(&packet, IDNFLG_CHNCFG_ROUTING, IDNVAL_SMOD_LPEFX_DISCRETE, serviceID);
      }
      else{
        buildConfigHeader(&packet, IDNFLG_CHNCFG_ROUTING, IDNVAL_SMOD_DMX512_DISCRETE, serviceID);
      }
      addServiceModeConfigChunk(&packet, offset);
    }
    buildDimmerLevelHeader(&packet, IDNVAL_DL_VOID);
    addDmxData(&packet, (unsigned char*)(values.data()), values.length());
    finishPacket(&packet); //add size to packet
    data = QByteArray(reinterpret_cast<char*>(packet.buf), packet.end - packet.buf);
}

 void IdnPacketizer::setupIdnDmx(QByteArray& data, const quint8 &mode, const quint8 &channelID, const QByteArray &values, const QList<QPair<int, int> > ranges, const quint32 seqnum, const bool config, const quint8 serviceID)
 {
    data.clear();
    IDNDMX_PACKET packet;
    InitDMXPacket(&packet);
    setPacketHeader(&packet, IDNCMD_MESSAGE, seqnum);
    buildChannelMessage(&packet, channelID);
    if(config){
      if(mode == 5){
        buildConfigHeader(&packet, IDNFLG_CHNCFG_ROUTING, IDNVAL_SMOD_LPEFX_DISCRETE, serviceID);
      }
      else{
        buildConfigHeader(&packet, IDNFLG_CHNCFG_ROUTING, IDNVAL_SMOD_DMX512_DISCRETE, serviceID);
      }
      for(int i = 0; i < ranges.length(); i++){
        addServiceModeConfigChunk(&packet, ranges[i].first+1, ranges[i].second-ranges[i].first+1);
      }
    }
    buildDimmerLevelHeader(&packet, IDNVAL_DL_VOID);
    addDmxData(&packet, (unsigned char*)(values.data()), values.length());
    finishPacket(&packet); //add size to packet
    data = QByteArray(reinterpret_cast<char*>(packet.buf), packet.end - packet.buf);
}

void IdnPacketizer::generateNullPacket(QByteArray& data, const quint32 seqnum, const quint8 &mode, const quint8 channelID, const quint8 serviceID)
{
  QByteArray empty;
  empty.fill(0x00, 512);
  setupIdnDmx(data, mode, channelID, empty, 1, seqnum, true, serviceID);
}

void IdnPacketizer::generateIdlePacket(QByteArray& data, const quint32 seqnum, const quint8 channelID, const quint8 serviceID)
{
  data.clear();
  IDNDMX_PACKET packet;
  InitDMXPacket(&packet);
  setPacketHeader(&packet, IDNCMD_MESSAGE, seqnum);
  buildChannelMessage(&packet, channelID);
  buildConfigHeader(&packet, IDNFLG_CHNCFG_ROUTING, IDNVAL_SMOD_LPEFX_DISCRETE, serviceID);
  finishPacket(&packet); //add size to packet
  data = QByteArray(reinterpret_cast<char*>(packet.buf), packet.end - packet.buf);
}

void IdnPacketizer::generateClosePacket(QByteArray& data, const quint32 seqnum, const quint8 channelID, const quint8 serviceID)
{
  data.clear();
  IDNDMX_PACKET packet;
  InitDMXPacket(&packet);
  setPacketHeader(&packet, IDNCMD_MESSAGE, seqnum);
  buildChannelMessage(&packet, channelID);
  buildConfigHeader(&packet, IDNFLG_CHNCFG_CLOSE, IDNVAL_SMOD_LPEFX_DISCRETE, serviceID);
  finishPacket(&packet); //add size to packet
  data = QByteArray(reinterpret_cast<char*>(packet.buf), packet.end - packet.buf);
}

void IdnPacketizer::generateScanRequestPacket(QByteArray& data)
{
  data.clear();
  IDNDMX_PACKET scanPacket;
  InitDMXPacket(&scanPacket);
  setPacketHeader(&scanPacket, IDNCMD_SCANREQUEST, 0);
  data = QByteArray(reinterpret_cast<char*>(scanPacket.buf), scanPacket.end - scanPacket.buf);
}

void IdnPacketizer::generateServiceMapRequestPacket(QByteArray& data)
{
  data.clear();
  IDNDMX_PACKET scanPacket;
  InitDMXPacket(&scanPacket);
  setPacketHeader(&scanPacket, IDNCMD_SERVICEREQUEST, 0);
  data = QByteArray(reinterpret_cast<char*>(scanPacket.buf), scanPacket.end - scanPacket.buf);
}

bool IdnPacketizer::validateReply(QByteArray datagram)
{
    if(datagram.size() == 44 && datagram.data()[0] == IDNCMD_SCANREPLY){
        return true;
    }
    return false;
}

bool IdnPacketizer::validateServiceMapReply(QByteArray datagram)
{
    if(datagram.size() >= 8 && datagram.data()[0] == IDNCMD_SERVICEREPLY){
        return true;
    }
    return false;
}

 void IdnPacketizer::InitDMXPacket(IDNDMX_PACKET *packet)
 {
  packet->packetHeader = NULL;
  packet->channelMessage = NULL;
  packet->channelConfig = NULL;
  packet->serviceModeConfig = NULL;
  packet->dimmerLevelHeader = NULL;
  packet->values = NULL;
  packet->end = packet->buf;
}

void IdnPacketizer::setPacketHeader(IDNDMX_PACKET *packet, unsigned char command, unsigned short seqnum)
{
  packet->packetHeader = (IDNHDR_PACKET*)IDNAddLayerToPacket(packet, sizeof(IDNHDR_PACKET));
  packet->packetHeader->command = command;
  packet->packetHeader->flags = 0;
  packet->packetHeader->seqnum = htons(seqnum);
}

int IdnPacketizer::buildChannelMessage(IDNDMX_PACKET *packet, quint8 channelID)
{
  packet->channelMessage = (IDNHDR_CHANNEL_MESSAGE *)IDNAddLayerToPacket(packet, sizeof(IDNHDR_CHANNEL_MESSAGE));
  IDNAddSizeToPacket(packet, sizeof(IDNHDR_CHANNEL_MESSAGE));

  packet->channelMessage->contentID = (IDNFLG_CONTENTID_CHANNELMSG | IDNVAL_CNKTYPE_VOID);
  packet->channelMessage->contentID |= (unsigned short)((channelID << 8) & IDNMSK_CONTENTID_CHANNELID);
  //packet->channelMessage->contentID |= (unsigned short)((0x01 << 8) & IDNMSK_CONTENTID_CHANNELID);
  return 0;
}

void IdnPacketizer::buildConfigHeader(IDNDMX_PACKET *packet, unsigned char flags, unsigned char serviceMode, unsigned char serviceID)
{
  packet->channelConfig = (IDNHDR_CHANNEL_CONFIG *)IDNAddLayerToPacket(packet, sizeof(IDNHDR_CHANNEL_CONFIG));
  packet->channelMessage->contentID |= IDNFLG_CONTENTID_CONFIG_LSTFRG;
  packet->channelConfig->wordCount = 0;
  packet->channelConfig->flags = flags;
  packet->channelConfig->serviceID = serviceID;
  packet->channelConfig->serviceMode = serviceMode;
}

int IdnPacketizer::addServiceModeConfigChunk(IDNDMX_PACKET *packet, unsigned short base)
{
  packet->serviceModeConfig = (IDNHDR_DMX_CONFIG *)IDNAddLayerToPacket(packet, sizeof(IDNHDR_DMX_CONFIG));
  packet->serviceModeConfig->idprm = IDNVAL_SMOD_CONFIG_SUBSET_BASE;
  packet->serviceModeConfig->base = htons(base);
  packet->serviceModeConfig->count = 0;
  packet->channelConfig->wordCount++;
  return 0;
}

int IdnPacketizer::addServiceModeConfigChunk(IDNDMX_PACKET *packet, unsigned short base, unsigned char count)
{
  packet->serviceModeConfig = (IDNHDR_DMX_CONFIG *)IDNAddLayerToPacket(packet, sizeof(IDNHDR_DMX_CONFIG));
  packet->serviceModeConfig->idprm = IDNVAL_SMOD_CONFIG_SUBSET_COUNT;
  packet->serviceModeConfig->base = htons(base);
  packet->serviceModeConfig->count = count;
  packet->channelConfig->wordCount++;
  return 0;
}

int IdnPacketizer::buildDimmerLevelHeader(IDNDMX_PACKET *packet, unsigned char flags)
{
  packet->dimmerLevelHeader = (IDNHDR_DIMMER_LEVEL *)IDNAddLayerToPacket(packet, sizeof(IDNHDR_DIMMER_LEVEL));
  packet->dimmerLevelHeader->flags = flags;
  return 0;
}

int IdnPacketizer::addDmxData(IDNDMX_PACKET *packet, unsigned char *dataBuffer, unsigned int dataCounter)
{
  packet->channelMessage->contentID |= IDNVAL_CNKTYPE_DIMMER_LEVELS;
  packet->values = IDNAddLayerToPacket(packet, dataCounter);
  memcpy(packet->values, dataBuffer, dataCounter);
  return 0;
}

int IdnPacketizer::finishPacket(IDNDMX_PACKET *packet)
{
  if (packet->channelMessage != NULL)
  {
    packet->channelMessage->totalSize = htons(packet->channelMessage->totalSize);
  }

  packet->channelMessage->contentID = htons(packet->channelMessage->contentID);  
  timestamp = QDateTime::currentMSecsSinceEpoch()*1000;
  packet->channelMessage->timestamp = ((timestamp>>24)&0xff) | // move byte 3 to byte 0
                     ((timestamp<<8)&0xff0000) | // move byte 1 to byte 2
                     ((timestamp>>8)&0xff00) | // move byte 2 to byte 1
                     ((timestamp<<24)&0xff000000);
  return 0;
}

unsigned char *IdnPacketizer::IDNAddLayerToPacket(IDNDMX_PACKET *packet, size_t size)
{
  unsigned char *ret = IDNGetLayerInPacket(packet, size);
  if (ret == NULL)
  {
    return NULL;
  }

  memset(ret, 0, size);

  if (packet->channelMessage != NULL)
  {
    packet->channelMessage->totalSize += (unsigned short)size;
  }
  return ret;
}

unsigned char *IdnPacketizer::IDNGetLayerInPacket(IDNDMX_PACKET *packet, size_t size)
{
  unsigned char *ret = packet->end;

  if (packet->end + size > &packet->buf[sizeof packet->buf])
  {
    return NULL;
  }

  packet->end += size;

  return ret;
}

size_t IdnPacketizer::IDNAddSizeToPacket(IDNDMX_PACKET *packet, size_t size)
{
  if (packet->channelMessage != NULL)
  {
    packet->channelMessage->totalSize += (unsigned short)size;
    return size;
  }

  return 0;
}
