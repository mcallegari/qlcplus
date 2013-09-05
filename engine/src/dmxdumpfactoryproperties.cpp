/*
  Q Light Controller Plus
  dmxdumpfactoryproperties.cpp

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

#include "dmxdumpfactoryproperties.h"

DmxDumpFactoryProperties::DmxDumpFactoryProperties(int universes)
{
    m_dumpAllChannels = true;
    m_dumpNonZeroValues = false;

    m_selectedTarget = 0;

    m_channelsMask = QByteArray(universes * 512, 0);
}

bool DmxDumpFactoryProperties::dumpChannelsMode()
{
    return m_dumpAllChannels;
}

void DmxDumpFactoryProperties::setDumpChannelsMode(bool mode)
{
    m_dumpAllChannels = mode;
}

bool DmxDumpFactoryProperties::nonZeroValuesMode()
{
    return m_dumpNonZeroValues;
}

void DmxDumpFactoryProperties::setNonZeroValuesMode(bool mode)
{
    m_dumpNonZeroValues = mode;
}

QByteArray DmxDumpFactoryProperties::channelsMask()
{
    return m_channelsMask;
}

void DmxDumpFactoryProperties::setChannelsMask(QByteArray mask)
{
    if (mask.isEmpty() == false)
        m_channelsMask = mask;
}

void DmxDumpFactoryProperties::addChaserID(quint32 id)
{
    if (m_selectedChaserIDs.contains(id) == false)
        m_selectedChaserIDs.append(id);
}

void DmxDumpFactoryProperties::removeChaserID(quint32 id)
{
    if (m_selectedChaserIDs.contains(id) == true)
        m_selectedChaserIDs.removeAll(id);
}

bool DmxDumpFactoryProperties::isChaserSelected(quint32 id)
{
    return m_selectedChaserIDs.contains(id);
}

void DmxDumpFactoryProperties::setSelectedTarget(int idx)
{
    if (idx >= 0 && idx < 3)
        m_selectedTarget = idx;
}

int DmxDumpFactoryProperties::selectedTarget()
{
    return m_selectedTarget;
}

