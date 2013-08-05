/*
  Q Light Controller
  qlcinputsource.cpp

  Copyright (C) Heikki Junnila

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

#include <QtCore>

#include "qlcinputchannel.h"
#include "qlcinputsource.h"
#include "inputpatch.h"
#include "inputmap.h"

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

