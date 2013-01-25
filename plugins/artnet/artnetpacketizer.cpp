/*
  Q Light Controller
  artnetpacketizer.cpp

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

#include "artnetpacketizer.h"

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

    m_sequence = 1;
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

void ArtNetPacketizer::setupArtNetDmx(QByteArray& data, const int &universe, const QByteArray &values)
{
    data.clear();
    data.append(m_commonHeader);
    const char opCodeMSB = (ARTNET_DMX >> 8);
    data[9] = opCodeMSB;
    data.append(m_sequence); // Sequence
    data.append('\0'); // Physical
    data.append((char)(universe >> 8));
    data.append((char)(universe & 0x00FF));
    int len = values.length();
    data.append((char)(len >> 8));
    data.append((char)(len & 0x00FF));
    data.append(values);

    if (m_sequence == 0xff)
        m_sequence = 1;
    else
        m_sequence++;
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
    info.shortName = QString(shortName.data());
    info.longName = QString(longName.data());

    qDebug() << "getArtPollReplyInfo shortName: " << info.shortName;
    qDebug() << "getArtPollReplyInfo longName: " << info.longName;

    return true;
}

