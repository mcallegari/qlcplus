/*
  Q Light Controller Plus
  idnpacketizer.h

  Copyright (c) Daniel Schröder
  Updated by Mauritz Kauffmann, 2026

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
#ifndef IDNPACKETIZER_H
#define IDNPACKETIZER_H

#include <QByteArray>
#include <QList>
#include <QPair>
#include <QString>

  /*********************************************************************
  * IDN Specific Constants
  *********************************************************************/

#define IDNCMD_VOID                         0x00
#define IDNCMD_SCANREQUEST                  0x10
#define IDNCMD_SCANREPLY                    0x11
#define IDNCMD_SERVICEREQUEST               0x12
#define IDNCMD_SERVICEREPLY                 0x13
#define IDNCMD_MESSAGE                      0x40

#define IDNFLG_CONTENTID_CHANNELMSG         0x8000      // Channel message flag (specific bit assignments)
#define IDNFLG_CONTENTID_CONFIG_LSTFRG      0x4000      // Set for config header or last fragment
#define IDNMSK_CONTENTID_CHANNELID          0x3F00      // Channel ID bit mask
#define IDNMSK_CONTENTID_CNKTYPE            0x00FF      // Data chunk type bit mask

#define IDNVAL_CNKTYPE_VOID                 0x0        // Empty chunk (no data)
#define IDNVAL_CNKTYPE_OCTET_SEGMENT        0x10        // Delimited sequence of octets (can be multiple chunks)
#define IDNVAL_CNKTYPE_OCTET_STRING         0x11        // Discrete sequence of octets (single chunk)
#define IDNVAL_CNKTYPE_DIMMER_LEVELS        0x18        // Dimmer levels for DMX512 packets

// Data chunk types - fragment sequel
#define IDNVAL_CNKTYPE_LPGRF_FRAME_SEQUEL   0xC0        // Sample data array (sequel fragment)

// Channel configuration: Flags
#define IDNMSK_CHNCFG_DATA_MATCH            0x30        // Data/Configuration crosscheck
#define IDNFLG_CHNCFG_ROUTING               0x01        // Verify/Route/Open channel before message processing
#define IDNFLG_CHNCFG_CLOSE                 0x02        // Close channel after message processing

#define IDNVAL_SMOD_LPEFX_CONTINUOUS        0x03        // Transparent, octet segments only (includes start code)
#define IDNVAL_SMOD_LPEFX_DISCRETE          0x04        // Buffered, frames of effect data, may mix with strings
#define IDNVAL_SMOD_DMX512_CONTINUOUS       0x05        // Transparent, octet segments only (includes start code)
#define IDNVAL_SMOD_DMX512_DISCRETE         0x06        // Buffered, frames of effect data, may mix with strings

#define IDNVAL_DL_VOID                      0x00

#define IDNVAL_SMOD_CONFIG_SUBSET_BASE      0x42
#define IDNVAL_SMOD_CONFIG_SUBSET_COUNT     0x43
// Chunk header flags
#define IDNMSK_CNKHDR_CONFIG_MATCH          0x30        // Data/Configuration crosscheck
#define IDNFLG_OCTET_SEGMENT_DELIMITER      0x01        // Segment contains octet sequence delimiter

#define MAX_UDP_PAYLOAD     65280 //65507

#define _attribute_(spec)

class IdnPacketizer
{
	/*********************************************************************
    * Initialization
    *********************************************************************/
public:
    IdnPacketizer();
    ~IdnPacketizer();

public:
	/** Prepare an IDN packet */
    void setupIdnDmx(QByteArray& data, const quint8 &mode, const quint8 &channelID, const QByteArray &values, const quint16 offset, const quint32 seqnum, const bool config, const quint8 serviceID);
    void setupIdnDmx(QByteArray& data,  const quint8 &mode, const quint8 &channelID, const QByteArray &values, const QList<QPair<int, int> > ranges, const quint32 seqnum, const bool config, const quint8 serviceID);
    void generateNullPacket(QByteArray& data,const quint32 seqnum, const quint8 &mode, const quint8 channelID, const quint8 serviceID);
    void generateIdlePacket(QByteArray& data, const quint32 seqnum, const quint8 channelID, const quint8 serviceID);
    void generateClosePacket(QByteArray& data, const quint32 seqnum, const quint8 channelID, const quint8 serviceID);
    void generateScanRequestPacket(QByteArray& data);
    void generateServiceMapRequestPacket(QByteArray& data);
    bool validateReply(QByteArray datagram);
    bool validateServiceMapReply(QByteArray datagram);
private:
    qint64 timestamp;

    #pragma pack(push,1)
    typedef struct{
      unsigned char command;    // The command code (IDNCMD_*)
      unsigned char flags;
      unsigned short seqnum;  // Sequence counter
    } IDNHDR_PACKET;
    #pragma pack(pop)

    #pragma pack(push,1)
    typedef struct{
      unsigned short totalSize;
      unsigned short contentID;
      quint32 timestamp;
    } IDNHDR_CHANNEL_MESSAGE;
    #pragma pack(pop)

    #pragma pack(push,1)
    typedef struct{
      unsigned char wordCount;
      unsigned char flags;                // Upper 4 bit decoder flags (0x30: Version), lower config
      unsigned char serviceID;
      unsigned char serviceMode;
    } IDNHDR_CHANNEL_CONFIG;
    #pragma pack(pop)

    #pragma pack(push,1)
    typedef struct
    {
      unsigned char idprm;
      unsigned short base;
      unsigned char count;
    } IDNHDR_DMX_CONFIG;
    #pragma pack(pop)

    #pragma pack(push,1)
    typedef struct
    {
      unsigned char flags;
      unsigned char null[3];
    } IDNHDR_DIMMER_LEVEL;
    #pragma pack(pop)


    #pragma pack(push,1)
    typedef struct{
      IDNHDR_PACKET *packetHeader;
      IDNHDR_CHANNEL_MESSAGE *channelMessage;
      IDNHDR_CHANNEL_CONFIG   *channelConfig;
      IDNHDR_DMX_CONFIG  *serviceModeConfig;
      IDNHDR_DIMMER_LEVEL *dimmerLevelHeader;
      unsigned char *values;
      unsigned char *end;
      unsigned char       buf[MAX_UDP_PAYLOAD];
    } IDNDMX_PACKET;
    #pragma pack(pop)

    unsigned char *IDNAddLayerToPacket(IDNDMX_PACKET *packet, size_t size);
    unsigned char *IDNGetLayerInPacket(IDNDMX_PACKET *packet, size_t size);
    size_t IDNAddSizeToPacket(IDNDMX_PACKET *packet, size_t size);
    int finishPacket(IDNDMX_PACKET *packet);
    void InitDMXPacket(IDNDMX_PACKET *packet);
    void setPacketHeader(IDNDMX_PACKET *packet, unsigned char command, unsigned short seqnum);
    int buildChannelMessage(IDNDMX_PACKET *packet, quint8 channelID);
    void buildConfigHeader(IDNDMX_PACKET *packet, unsigned char flags, unsigned char serviceMode, unsigned char serviceID);
    int addServiceModeConfigChunk(IDNDMX_PACKET *packet, unsigned short base, unsigned char count);
    int addServiceModeConfigChunk(IDNDMX_PACKET *packet, unsigned short base);
    int buildDimmerLevelHeader(IDNDMX_PACKET *packet, unsigned char flags);
    int addDmxData(IDNDMX_PACKET *packet, const unsigned char *dataBuffer, unsigned int dataCounter);
};

#endif
