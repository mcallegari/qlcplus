/*
  Q Light Controller
  qlcinputsource.cpp

  Copyright (C) Heikki Junnila

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

#include <QtCore>

#include "qlcinputchannel.h"
#include "qlcinputsource.h"
#include "inputpatch.h"

quint32 QLCInputSource::invalidUniverse = UINT_MAX;
quint32 QLCInputSource::invalidChannel = UINT_MAX;

QLCInputSource::QLCInputSource()
    : m_universe(invalidUniverse)
    , m_channel(invalidChannel)
{
}

QLCInputSource::QLCInputSource(quint32 universe, quint32 channel)
    : m_universe(universe)
    , m_channel(channel)
{
}

bool QLCInputSource::isValid() const
{
    if (universe() != invalidUniverse && channel() != invalidChannel)
        return true;
    else
        return false;
}

QLCInputSource& QLCInputSource::operator=(const QLCInputSource& source)
{
    setUniverse(source.universe());
    setChannel(source.channel());
    return *this;
}

bool QLCInputSource::operator==(const QLCInputSource& source) const
{
    if (isValid() == true && source.isValid() == true &&
        universe() == source.universe() && channel() == source.channel())
    {
        return true;
    }
    else
    {
        return false;
    }
}

void QLCInputSource::setUniverse(quint32 uni)
{
    m_universe = uni;
}

quint32 QLCInputSource::universe() const
{
    return m_universe;
}

void QLCInputSource::setChannel(quint32 ch)
{
    m_channel = ch;
}

quint32 QLCInputSource::channel() const
{
    return m_channel;
}

void QLCInputSource::setPage(ushort pgNum)
{
    quint32 chCopy = m_channel & 0x0000FFFF;
    m_channel = ((quint32)pgNum << 16) | chCopy;
}

ushort QLCInputSource::page() const
{
    return (ushort)(m_channel >> 16);
}

