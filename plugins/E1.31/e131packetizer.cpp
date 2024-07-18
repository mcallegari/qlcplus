/*
  Q Light Controller Plus
  e131packetizer.cpp

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

#include "e131packetizer.h"

#include <QStringList>
#include <QDebug>

E131Packetizer::E131Packetizer(QString MACaddr)
{
    // Initialize a commond header.
    m_commonHeader.clear();

    // Preamble Size
    m_commonHeader.append((char)0x00);
    m_commonHeader.append((char)0x10);

    // Post-amble Size
    m_commonHeader.append((char)0x00);
    m_commonHeader.append((char)0x00);

    // Identifies this packet as E1.17
    m_commonHeader.append((char)0x41);
    m_commonHeader.append((char)0x53);
    m_commonHeader.append((char)0x43);
    m_commonHeader.append((char)0x2D);
    m_commonHeader.append((char)0x45);
    m_commonHeader.append((char)0x31);
    m_commonHeader.append((char)0x2E);
    m_commonHeader.append((char)0x31);
    m_commonHeader.append((char)0x37);
    m_commonHeader.append((char)0x00);
    m_commonHeader.append((char)0x00);
    m_commonHeader.append((char)0x00);

    // empty flags & PDU length (bytes 16-17)
    m_commonHeader.append('\0');
    m_commonHeader.append('\0');

    // Identifies RLP Data as 1.31 Protocol PDU
    m_commonHeader.append((char)0x00);
    m_commonHeader.append((char)0x00);
    m_commonHeader.append((char)0x00);
    m_commonHeader.append((char)0x04);

    //Sender's CID
    m_commonHeader.append((char)0xFB);
    m_commonHeader.append((char)0x3C);
    m_commonHeader.append((char)0x10);
    m_commonHeader.append((char)0x65);
    m_commonHeader.append((char)0xA1);
    m_commonHeader.append((char)0x7F);
    m_commonHeader.append((char)0x4D);
    m_commonHeader.append((char)0xE2);
    m_commonHeader.append((char)0x99);
    m_commonHeader.append((char)0x19);

    QStringList MAC = MACaddr.split(":");
    if (MAC.length() == 6)
    {
        foreach (QString couple, MAC)
        {
            bool ok;
            m_commonHeader.append((char)couple.toInt(&ok, 16));
        }
    }
    else
    {
        m_commonHeader.append((char)0x31);
        m_commonHeader.append((char)0x7A);
        m_commonHeader.append((char)0x07);
        m_commonHeader.append((char)0xC1);
        m_commonHeader.append((char)0x00);
        m_commonHeader.append((char)0x52);
    }

    // empty flags & PDU length (bytes 38-39)
    m_commonHeader.append('\0');
    m_commonHeader.append('\0');

    // Identifies 1.31 data as DMP Protocol PDU
    m_commonHeader.append((char)0x00);
    m_commonHeader.append((char)0x00);
    m_commonHeader.append((char)0x00);
    m_commonHeader.append((char)0x02);

    // User Assigned Name of source !!must be 64 bytes long!!
    QString sourceName("Q Light Controller Plus - E1.31");
    m_commonHeader.append(sourceName.toUtf8());
    for (int i = 0; i < 64 - sourceName.length(); i++)
        m_commonHeader.append((char)0x00);

    // Data priority if multiple sources (default to 100) (byte 108)
    m_commonHeader.append((char)E131_PRIORITY_DEFAULT);

    // reserved
    m_commonHeader.append('\0');
    m_commonHeader.append('\0');

    // sequence counter (byte 111)
    m_commonHeader.append('\0');

    // Options Flags - default to none
    m_commonHeader.append('\0');

    // Universe (bytes 113-114)
    m_commonHeader.append('\0');
    m_commonHeader.append('\0');

    // Protocol flags and length (bytes 115-116)
    m_commonHeader.append('\0');
    m_commonHeader.append('\0');

    // Identifies DMP Set Property Message PDU
    m_commonHeader.append((char)0x02);

    // Address type & Data type - Identifies format of address and data
    m_commonHeader.append((char)0xA1);

    // First Property address - Identifies format of address and data
    m_commonHeader.append((char)0x00);
    m_commonHeader.append((char)0x00);

    // Address Increment - Indicates each property is 1 octet
    m_commonHeader.append((char)0x00);
    m_commonHeader.append((char)0x01);

    // Property value count -  Indicates 1+ the number of slots in packet (bytes 123-124)
    m_commonHeader.append('\0');
    m_commonHeader.append('\0');

    // DMX512-A START Code
    m_commonHeader.append((char)0x00);

    m_sequence[0] = 1;
    m_sequence[1] = 1;
    m_sequence[2] = 1;
    m_sequence[3] = 1;
}

E131Packetizer::~E131Packetizer()
{
}

/*********************************************************************
 * Sender functions
 *********************************************************************/

void E131Packetizer::setupE131Dmx(QByteArray& data, const int &universe, const int &priority, const QByteArray &values)
{
    data.clear();
    data.append(m_commonHeader);

    data.append(values);

    int rootLayerSize = data.length() - 16;
    int e131LayerSize = data.length() - 38;
    int dmpLayerSize = data.length() - 115;
    int valCountPlusOne = values.length() + 1;

    data[16] = 0x70 | (char)(rootLayerSize >> 8);
    data[17] = (char)(rootLayerSize & 0x00FF);

    data[38] = 0x70 | (char)(e131LayerSize >> 8);
    data[39] = (char)(e131LayerSize & 0x00FF);

    data[108] = (char) priority;

    data[111] = m_sequence[universe];

    data[113] = (char)(universe >> 8);
    data[114] = (char)(universe & 0x00FF);

    data[115] = 0x70 | (char)(dmpLayerSize >> 8);
    data[116] = (char)(dmpLayerSize & 0x00FF);

    data[123] = (char)(valCountPlusOne >> 8);
    data[124] = (char)(valCountPlusOne & 0x00FF);

    if (m_sequence[universe] == 0xff)
        m_sequence[universe] = 1;
    else
        m_sequence[universe]++;
}

bool E131Packetizer::checkPacket(QByteArray &data)
{
    /* An E1.31 packet must be at least 125 bytes long */
    if (data.length() < 125)
        return false;

    // check ACN packet identifier
    if (data[4] != 'A' || data[5] != 'S' || data[6] != 'C' ||
        data[7] != '-' || data[8] != 'E' || data[9] != '1' ||
        data[10] != '.' || data[11] != '1' || data[12] != '7' ||
        data[13] != (char)0x00 || data[14] != (char)0x00 || data[15] != (char)0x00)
            return false;

    // check streaming DMX vector
    if (data[40] != (char)0x00 || data[41] != (char)0x00 || data[42] != (char)0x00 || data[43] != (char)0x02)
        return false;

    return true;
}

/*********************************************************************
 * Receiver functions
 *********************************************************************/

bool E131Packetizer::fillDMXdata(QByteArray& data, QByteArray &dmx, quint32 &universe)
{
    if (data.isNull())
        return false;

    /* Check valid DMX start code */
    if (data[125] != (char)0x00)
        return false;

    universe = (uchar(data[113]) << 8) + uchar(data[114]);
    int length = (uchar(data[123]) << 8) + uchar(data[124]);

    qDebug() << "[E1.31 fillDMXdata] universe:" << universe << ", length:" << length - 1;

    dmx.clear();
    dmx.append(data.mid(126, length - 1));
    return true;
}
