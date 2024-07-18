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
#include "rdmprotocol.h"

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

ArtNetPacketizer::~ArtNetPacketizer()
{
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

void ArtNetPacketizer::setupArtNetPollReply(QByteArray &data, QHostAddress ipAddr, QString MACaddr, quint32 universe, bool isInput)
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
    for (i = 0; i < 22; i++) // 64-42 bytes of stuffing. 42 is the length of the long name
        data.append((char)0x00);

    for (i = 0; i < 64; i++)
        data.append((char)0x00); // Node report
    data.append((char)0x00);     // NumPortsHi
    data.append((char)0x01);     // NumPortsLo
    data.append(isInput ? (char)0x40 : (char)0x80); // PortTypes[0]: can input or output DMX512 data
    data.append((char)0x00);     // PortTypes[1]: nothing
    data.append((char)0x00);     // PortTypes[2]: nothing
    data.append((char)0x00);     // PortTypes[3]: nothing

    data.append(isInput ? (char)0x80 : (char)0x00); // GoodInput[0] - input status port 1
    data.append((char)0x00);     // GoodInput[1] - input status port 2
    data.append((char)0x00);     // GoodInput[2] - input status port 3
    data.append((char)0x00);     // GoodInput[3] - input status port 4

    data.append(isInput ? (char)0x00 : (char)0x80); // GoodOutputA[0] - output status port 1
    data.append((char)0x00);     // GoodOutputA[0] - output status port 2
    data.append((char)0x00);     // GoodOutputA[0] - output status port 3
    data.append((char)0x00);     // GoodOutputA[0] - output status port 4

    data.append(isInput ? (char)universe : (char)0x00); // SwIn[0] - port 1
    data.append((char)0x00);     // SwIn[1] - port 2
    data.append((char)0x00);     // SwIn[2] - port 3
    data.append((char)0x00);     // SwIn[3] - port 4

    data.append(isInput ? (char)0x00 : (char)universe); // SwOut[0] - port 1
    data.append((char)0x00);     // SwOut[1] - port 2
    data.append((char)0x00);     // SwOut[2] - port 3
    data.append((char)0x00);     // SwOut[3] - port 4
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

void ArtNetPacketizer::setupArtNetTodRequest(QByteArray &data, const int &universe)
{
    data.clear();
    data.append(m_commonHeader);
    data[9] = char(ARTNET_TODREQUEST >> 8);
    data.append(char(0x00)); // Filler1
    data.append(char(0x00)); // Filler2
    data.append(char(0x00)); // Spare1
    data.append(char(0x00)); // Spare2
    data.append(char(0x00)); // Spare3
    data.append(char(0x00)); // Spare4
    data.append(char(0x00)); // Spare5
    data.append(char(0x00)); // Spare6
    data.append(char(0x00)); // Spare7

    data.append((char)(universe >> 8));     // Net
    data.append(char(0x00));                // Command: TodFull
    data.append(char(0x01));                // AddCount
    data.append((char)(universe & 0x00FF)); // Address
}

void ArtNetPacketizer::setupArtNetRdm(QByteArray &data, const int &universe, uchar command, QVariantList params)
{
    RDMProtocol rdm;
    QByteArray ba;

    data.clear();
    data.append(m_commonHeader);
    data[9] = char(ARTNET_RDM >> 8);
    data.append(char(0x01)); // RDM version 1.0
    data.append(char(0x00)); // Filler1
    data.append(char(0x00)); // Spare1
    data.append(char(0x00)); // Spare2
    data.append(char(0x00)); // Spare3
    data.append(char(0x00)); // Spare4
    data.append(char(0x00)); // Spare5
    data.append(char(0x00)); // Spare6
    data.append(char(0x00)); // Spare7

    data.append((char)(universe >> 8));     // Net
    data.append(char(0x00));                // ArProcess
    data.append((char)(universe & 0x00FF)); // Address

    rdm.packetizeCommand(command, params, false, ba);
    data.append(ba);
}

/*********************************************************************
 * Receiver functions
 *********************************************************************/

bool ArtNetPacketizer::checkPacketAndCode(QByteArray const& data, quint16 &code)
{
    /* An ArtNet header must be at least 12 bytes long */
    if (data.length() < 12)
        return false;

    /* Check "Art-Net" keyword presence */
    if (data.indexOf(ARTNET_CODE_STR) != 0)
        return false;

    if (data.at(7) != 0x00)
        return false;

    code = (quint16(data.at(9)) << 8) + quint16(data.at(8));

    return true;
}

bool ArtNetPacketizer::fillArtPollReplyInfo(QByteArray const& data, ArtNetNodeInfo& info)
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

bool ArtNetPacketizer::fillDMXdata(QByteArray const& data, QByteArray &dmx, quint32 &universe)
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

bool ArtNetPacketizer::processTODdata(const QByteArray &data, quint32 &universe, QVariantMap &values)
{
    if (data.isNull() || data.length() < 28)
        return false;

    // 0 - 11 ArtNet header
    // 12 RDM version
    // 13 Port
    // 14 - 20 Spare
    // 21, 23 address
    universe = (data.at(21) << 8) + data.at(23);
    // 22 Command response
    // 24 - 25 UID total
    //quint16 uidTotal = (quint8(data.at(24)) << 8) + quint8(data.at(25));
    // 26 BlockCount (consider only when total > 200)
    // 27 UID count
    quint8 uidCount = quint8(data.at(27));

    qDebug() << "UID count:" << uidCount;

    for (int i = 0; i < uidCount; i++)
    {
        quint16 ESTAId;
        quint32 deviceId;
        QString UID = RDMProtocol::byteArrayToUID(data.mid(28 + (i * 6), 6), ESTAId, deviceId);
        qDebug() << "UID:" << UID;
        values.insert(QString("UID-%1").arg(i), UID);
    }
    values.insert("DISCOVERY_COUNT", uidCount);

    return true;
}

bool ArtNetPacketizer::processRDMdata(const QByteArray &data, quint32 &universe, QVariantMap &values)
{
    if (data.isNull() || data.length() < 24)
        return false;

    // 0 - 11 ArtNet header
    // 12 RDM version
    // 13 - 20 zero fillers
    // 21, 23 address
    universe = (data.at(21) << 8) + data.at(23);

    RDMProtocol rdm;
    return rdm.parsePacket(data.mid(24), values);
}

