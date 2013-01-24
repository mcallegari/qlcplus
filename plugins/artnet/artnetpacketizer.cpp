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
}

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
    data.append('\0'); // Sequence (do I need this ?)
    data.append('\0'); // Physical
    data.append((char)(universe >> 8));
    data.append((char)(universe & 0x00FF));
    int len = values.length();
    data.append((char)(len >> 8));
    data.append((char)(len & 0x00FF));
    data.append(values);
}
