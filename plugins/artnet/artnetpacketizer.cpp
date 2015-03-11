/*
  Q Light Controller Plus
  artnetpacketizer.cpp

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

#include "artnetpacketizer.h"

#include <QStringList>
#include <QDebug>

ArtNetPacketizer::ArtNetPacketizer()
{
    // Initialize a commond header.
    // Changing only the tenth byte will create a valid ArtNet header
    m_commonHeader.clear();
    m_commonHeader.append(ARTNET_CODE_STR);
    m_commonHeader.append('\0');

    // empty opcode
    m_commonHeader.append('\0');
    m_commonHeader.append('\0');

    // version 14 by default
    m_commonHeader.append('\0');
    m_commonHeader.append((char)0x0e);

    m_sequence[0] = 1;
    m_sequence[1] = 1;
    m_sequence[2] = 1;
    m_sequence[3] = 1;
}

/*********************************************************************
 * Sender functions
 *********************************************************************/

void ArtNetPacketizer::setupArtNetPoll(QByteArray& data)
{
    data.clear();
    data.append(m_commonHeader);
    const char opCodeMSB = (ARTNET_POLL >> 8);
    data[9] = opCodeMSB;
    data.append((char)0x02); // TalkToMe
    data.append('\0'); // Priority
}

void ArtNetPacketizer::setupArtNetPollReply(QByteArray &data, QHostAddress ipAddr, QString MACaddr)
{
    int i = 0;
    data.clear();
    data.append(m_commonHeader);
    data.remove(9, 2);
    const char opCodeMSB = (ARTNET_POLLREPLY >> 8);
    data[9] = opCodeMSB;
    QString ipStr = ipAddr.toString();
    QStringList ipAddrList = ipStr.split(".");
    foreach (QString val, ipAddrList)
        data.append((char)val.toInt()); // IP address[4]
    data.append((char)0x36);     // Port LSB
    data.append((char)0x19);     // Port MSB
    data.append((char)0x04);     // Version MSB
    data.append((char)0x20);     // Version LSB
    data.append((char)0x00);     // Sub Switch MSB
    data.append((char)0x00);     // Sub Switch LSB
    data.append((char)0xFF);     // OEM Value MSB
    data.append((char)0xFF);     // OEM Value LSB
    data.append((char)0x00);     // UBEA version
    data.append((char)0xF0);     // Status1 - Ready and booted
    data.append((char)0xFF);     // ESTA Manufacturer MSB
    data.append((char)0xFF);     // ESTA Manufacturer LSB
    data.append("QLC+");   // Short Name
    for (i = 0; i < 14; i++)
        data.append((char)0x00); // 14 bytes of stuffing
    data.append("Q Light Controller Plus - ArtNet interface"); // Long Name
    for (i = 0; i < 22; i++) // 64-42 bytes of stuffing. 42 is the lenght of the long name
        data.append((char)0x00);
    for (i = 0; i < 64; i++)
        data.append((char)0x00); // Node report
    data.append((char)0x00);     // NumPort MSB
    // FIXME: this should reflect the actual state of QLC+ output ports !
    data.append((char)0x01);     // NumPort LSB
    data.append((char)0x80);     // Port 1 type: can output DMX512 data
    data.append((char)0x80);     // Port 2 type: can output DMX512 data
    data.append((char)0x80);     // Port 3 type: can output DMX512 data
    data.append((char)0x80);     // Port 4 type: can output DMX512 data
    // FIXME: this should reflect the actual state of QLC+ output ports !
    for (i = 0; i < 12; i++)
        data.append((char)0x00); // Set GoodInput[4], GoodOutput[4] and SwIn[4] all to unknown state
    data.append((char)0x00);     // SwOut0 - output 0
    data.append((char)0x01);     // SwOut1 - output 1
    data.append((char)0x02);     // SwOut2 - output 2
    data.append((char)0x03);     // SwOut3 - output 3
    for (i = 0; i < 7; i++)
        data.append((char)0x00);  // SwVideo, SwMacro, SwRemote and 4 spare bytes
    QStringList MAC = MACaddr.split(":");
    foreach (QString couple, MAC)
    {
        bool ok;
        data.append((char)couple.toInt(&ok, 16));
    }
    for (i = 0; i < 32; i++)
        data.append((char)0x00); // bindIp[4], BindIndex, Status2 and filler
}

void ArtNetPacketizer::setupArtNetDmx(QByteArray& data, const int &universe, const QByteArray &values)
{
    data.clear();
    data.append(m_commonHeader);
    const char opCodeMSB = (ARTNET_DMX >> 8);
    data[9] = opCodeMSB;
    data.append(m_sequence[universe]); // Sequence
    data.append('\0'); // Physical
    data.append((char)(universe & 0x00FF));
    data.append((char)(universe >> 8));
    int padLength = values.isEmpty() ? 2 : (values.length() % 2); // length must be even in the range 2-512
    int len = values.length() + padLength;
    data.append((char)(len >> 8));
    data.append((char)(len & 0x00FF));
    data.append(values);
    data.append(QByteArray(padLength, 0));

    if (m_sequence[universe] == 0xff)
        m_sequence[universe] = 1;
    else
        m_sequence[universe]++;
}

/*********************************************************************
 * Receiver functions
 *********************************************************************/

bool ArtNetPacketizer::checkPacketAndCode(QByteArray& data, int &code)
{
    /* An ArtNet header must be at least 12 bytes long */
    if (data.length() < 12)
        return false;

    /* Check "Art-Net" keyword presence */
    if (data.indexOf(ARTNET_CODE_STR) != 0)
        return false;

    if (data.at(7) != 0x00)
        return false;

    code = ((int)data.at(9) << 8) + data.at(8);

    return true;
}

bool ArtNetPacketizer::fillArtPollReplyInfo(QByteArray& data, ArtNetNodeInfo &info)
{
    if (data.isNull())
        return false;

    QByteArray shortName = data.mid(26, 18);
    QByteArray longName = data.mid(44, 64);
    info.shortName = QString(shortName.data()).simplified();
    info.longName = QString(longName.data()).simplified();

    qDebug() << "getArtPollReplyInfo shortName: " << info.shortName;
    qDebug() << "getArtPollReplyInfo longName: " << info.longName;

    return true;
}

bool ArtNetPacketizer::fillDMXdata(QByteArray& data, QByteArray &dmx, quint32 &universe)
{
    if (data.isNull())
        return false;
    dmx.clear();
    //char sequence = data.at(12);
    //qDebug() << "Sequence: " << sequence;
    // char physical = data.at(13) // skipped
    universe = (data.at(15) << 8) + data.at(14);

    unsigned int msb = (data.at(16)&0xff);
    unsigned int lsb = (data.at(17)&0xff);
    int length = (msb << 8) | lsb;

    //qDebug() << "length: " << length;
    dmx.append(data.mid(18, length));
    return true;
}

