/*
  Q Light Controller
  artnetpacketizer.h

  Copyright (c) Massimo Callegari

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

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

    /** Prepare an ArtNetDmx packet */
    void setupArtNetDmx(QByteArray& data, const int& universe, const QByteArray &values);

    /*********************************************************************
     * Receiver functions
     *********************************************************************/

    /** Verify the validity of an ArtNet packet and store the opCode in 'code' */
    bool checkPacketAndCode(QByteArray& data, int &code);

    bool fillArtPollReplyInfo(QByteArray& data, ArtNetNodeInfo &info);

private:
    QByteArray m_commonHeader;
    QHash<int, uchar> m_sequence;
};

#endif
