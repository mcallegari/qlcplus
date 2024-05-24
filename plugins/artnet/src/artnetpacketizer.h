/*
  Q Light Controller Plus
  artnetpacketizer.h

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

#include <QHostAddress>
#include <QByteArray>
#include <QString>
#include <QHash>

#ifndef ARTNETPACKETIZER_H
#define ARTNETPACKETIZER_H

#define ARTNET_POLL           0x2000
#define ARTNET_POLLREPLY      0x2100
#define ARTNET_DIAGDATA       0x2300
#define ARTNET_COMMAND        0x2400
#define ARTNET_DMX            0x5000
#define ARTNET_NZS            0x5100
#define ARTNET_ADDRESS        0x6000
#define ARTNET_INPUT          0x7000
#define ARTNET_TODREQUEST     0x8000
#define ARTNET_TODDATA        0x8100
#define ARTNET_TODCONTROL     0x8200
#define ARTNET_RDM            0x8300
#define ARTNET_RDMSUB         0x8400
#define ARTNET_VIDEOSTEUP     0xa010
#define ARTNET_VIDEOPALETTE   0xa020
#define ARTNET_VIDEODATA      0xa040
#define ARTNET_MACMASTER      0xf000
#define ARTNET_MACSLAVE       0xf100
#define ARTNET_FIRMWAREMASTER 0xf200
#define ARTNET_FIRMWAREREPLY  0xf300
#define ARTNET_FILETNMASTER   0xf400
#define ARTNET_FILEFNMASTER   0xf500
#define ARTNET_FILEFNREPLY    0xf600
#define ARTNET_IPPROG         0xf800
#define ARTNET_IPREPLY        0xf900
#define ARTNET_MEDIA          0x9000
#define ARTNET_MEDIAPATCH     0x9100
#define ARTNET_MEDIACONTROL   0x9200
#define ARTNET_MEDIACONTROLREPLY 0x9300
#define ARTNET_TIMECODE       0x9700
#define ARTNET_TIMESYNC       0x9800
#define ARTNET_TRIGGER        0x9900
#define ARTNET_DIRECTORY      0x9a00
#define ARTNET_DIRECTORYREPLY 0x9b00

#define ARTNET_CODE_STR "Art-Net"

typedef struct
{
    QString shortName;
    QString longName;
    // ... can be extended with more info to be added by fillArtPollReplyInfo
} ArtNetNodeInfo;

class ArtNetPacketizer
{
    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    ArtNetPacketizer();
    ~ArtNetPacketizer();

public:
    /*********************************************************************
     * Sender functions
     *********************************************************************/
    /** Prepare an ArtNetPoll packet */
    void setupArtNetPoll(QByteArray& data);

    /** Prepare an ArtNetPollReply packet */
    void setupArtNetPollReply(QByteArray &data, QHostAddress ipAddr,
                              QString MACaddr, quint32 universe, bool isInput);

    /** Prepare an ArtNetDmx packet */
    void setupArtNetDmx(QByteArray& data, const int& universe, const QByteArray &values);

    /** Prepare an ArtTodRequest packet */
    void setupArtNetTodRequest(QByteArray& data, const int& universe);

    /** Prepare an ArtRdm packet */
    void setupArtNetRdm(QByteArray& data, const int &universe, uchar command, QVariantList params);

    /*********************************************************************
     * Receiver functions
     *********************************************************************/

    /** Verify the validity of an ArtNet packet and store the opCode in 'code' */
    bool checkPacketAndCode(QByteArray const& data, quint16 &code);

    bool fillArtPollReplyInfo(QByteArray const& data, ArtNetNodeInfo& info);

    /** Process a ArtDmx packet and extract the relevant DMX data */
    bool fillDMXdata(QByteArray const& data, QByteArray& dmx, quint32 &universe);

    /** Process a ArtTodData packet and extract the relevant information */
    bool processTODdata(const QByteArray &data, quint32 &universe, QVariantMap &values);

    /** Process a ArtRdm packet and extract the relevant information */
    bool processRDMdata(const QByteArray &data, quint32 &universe, QVariantMap &values);

private:
    QByteArray m_commonHeader;
    QHash<int, uchar> m_sequence;
};

#endif
