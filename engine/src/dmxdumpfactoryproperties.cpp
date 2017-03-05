/*
  Q Light Controller Plus
  dmxdumpfactoryproperties.cpp

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

#include "dmxdumpfactoryproperties.h"

DmxDumpFactoryProperties::DmxDumpFactoryProperties(int universes)
    : m_dumpAllChannels(true)
    , m_dumpNonZeroValues(false)
    , m_selectedTarget(Chaser)
{
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
        m_channelsMask.replace(0, mask.length(), mask);
}

/************************************************************************
 * Dump target
 ***********************************************************************/

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

void DmxDumpFactoryProperties::setSelectedTarget(DmxDumpFactoryProperties::TargetType type)
{
    m_selectedTarget = type;
}

DmxDumpFactoryProperties::TargetType DmxDumpFactoryProperties::selectedTarget()
{
    return m_selectedTarget;
}

