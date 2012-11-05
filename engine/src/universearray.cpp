/*
  Q Light Controller
  universearray.cpp

  Copyright (c) Heikki Junnila

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

#include <math.h>

#include "universearray.h"
#include "qlcmacros.h"

#define KXMLQLCGMValueModeLimit "Limit"
#define KXMLQLCGMValueModeReduce "Reduce"
#define KXMLQLCGMChannelModeAllChannels "All"
#define KXMLQLCGMChannelModeIntensity "Intensity"

/****************************************************************************
 * Initialization
 ****************************************************************************/

UniverseArray::UniverseArray(int size)
    : m_size(size)
    , m_preGMValues(new QByteArray(size, char(0)))
    , m_postGMValues(new QByteArray(size, char(0)))
{
    m_gMChannelMode = GMIntensity;
    m_gMValueMode = GMReduce;
    m_gMValue = 255;
    m_gMFraction = 1.0;
}

UniverseArray::~UniverseArray()
{
    delete m_preGMValues;
    delete m_postGMValues;
}

int UniverseArray::size() const
{
    return m_size;
}

void UniverseArray::reset()
{
    m_preGMValues->fill(0);
    m_postGMValues->fill(0);
    m_gMIntensityChannels.clear();
    m_gMNonIntensityChannels.clear();
}

void UniverseArray::reset(int address, int range)
{
    for (int i = address; i < address + range && i < size(); i++)
    {
        m_preGMValues->data()[i] = 0;
        m_postGMValues->data()[i] = 0;
        m_gMIntensityChannels.remove(i);
        m_gMNonIntensityChannels.remove(i);
    }
}

/****************************************************************************
 * Highest Takes Precedence
 ****************************************************************************/

void UniverseArray::zeroIntensityChannels()
{
    QSetIterator <int> it(m_gMIntensityChannels);
    while (it.hasNext() == true)
    {
        int channel(it.next());
        m_preGMValues->data()[channel] = 0;
        m_postGMValues->data()[channel] = 0;
    }
}

bool UniverseArray::checkHTP(int channel, uchar value, QLCChannel::Group group) const
{
    if (group == QLCChannel::Intensity && value < uchar(preGMValues()[channel]))
    {
        /* Current value is higher than new value and HTP applies: reject. */
        return false;
    }
    else
    {
        /* Current value is below new value or HTP does not apply: accept. */
        return true;
    }
}

/****************************************************************************
 * Grand Master
 ****************************************************************************/

UniverseArray::GMValueMode UniverseArray::stringToGMValueMode(const QString& str)
{
    if (str == KXMLQLCGMValueModeLimit)
        return UniverseArray::GMLimit;
    else
        return UniverseArray::GMReduce;
}

QString UniverseArray::gMValueModeToString(UniverseArray::GMValueMode mode)
{
    switch (mode)
    {
    case UniverseArray::GMLimit:
        return KXMLQLCGMValueModeLimit;
    default:
    case UniverseArray::GMReduce:
        return KXMLQLCGMValueModeReduce;
    }
}

UniverseArray::GMChannelMode UniverseArray::stringToGMChannelMode(const QString& str)
{
    if (str == KXMLQLCGMChannelModeAllChannels)
        return UniverseArray::GMAllChannels;
    else
        return UniverseArray::GMIntensity;
}

QString UniverseArray::gMChannelModeToString(UniverseArray::GMChannelMode mode)
{
    switch (mode)
    {
    case UniverseArray::GMAllChannels:
        return KXMLQLCGMChannelModeAllChannels;
    default:
    case UniverseArray::GMIntensity:
        return KXMLQLCGMChannelModeIntensity;
    }
}

void UniverseArray::setGMValueMode(UniverseArray::GMValueMode mode)
{
    if (m_gMValueMode != mode)
    {
        m_gMValueMode = mode;
        setGMValue(gMValue());
    }
}

UniverseArray::GMValueMode UniverseArray::gMValueMode() const
{
    return m_gMValueMode;
}

void UniverseArray::setGMChannelMode(UniverseArray::GMChannelMode mode)
{
    if (m_gMChannelMode != mode)
    {
        m_gMChannelMode = mode;
        setGMValue(gMValue());
    }

    if (gMChannelMode() == GMIntensity)
    {
        QSetIterator <int> it(m_gMNonIntensityChannels);
        while (it.hasNext() == true)
        {
            int channel(it.next());
            char chValue(m_preGMValues->data()[channel]);
            write(channel, chValue, QLCChannel::NoGroup);
        }
    }
}

UniverseArray::GMChannelMode UniverseArray::gMChannelMode() const
{
    return m_gMChannelMode;
}

void UniverseArray::setGMValue(uchar value)
{
    m_gMValue = value;
    m_gMFraction = CLAMP(double(value) / double(UCHAR_MAX), 0.0, 1.0);

    QSetIterator <int> it(m_gMIntensityChannels);
    while (it.hasNext() == true)
    {
        int channel(it.next());
        char chValue(m_preGMValues->data()[channel]);
        write(channel, chValue, QLCChannel::Intensity);
    }

    if (gMChannelMode() == GMAllChannels)
    {
        QSetIterator <int> it(m_gMNonIntensityChannels);
        while (it.hasNext() == true)
        {
            int channel(it.next());
            char chValue(m_preGMValues->data()[channel]);
            write(channel, chValue, QLCChannel::NoGroup);
        }
    }
}

uchar UniverseArray::gMValue() const
{
    return m_gMValue;
}

double UniverseArray::gMFraction() const
{
    return m_gMFraction;
}

const QByteArray* UniverseArray::postGMValues() const
{
    return m_postGMValues;
}

const QByteArray UniverseArray::preGMValues() const
{
    return *m_preGMValues;
}

uchar UniverseArray::applyGM(int channel, uchar value, QLCChannel::Group group)
{
    if (value == 0)
    {
        if (group == QLCChannel::Intensity)
            m_gMIntensityChannels.remove(channel);
        else
            m_gMNonIntensityChannels.remove(channel);
        return value;
    }

    if ((gMChannelMode() == GMIntensity && group == QLCChannel::Intensity) ||
        (gMChannelMode() == GMAllChannels))
    {
        if (gMValueMode() == GMLimit)
            value = MIN(value, gMValue());
        else
            value = char(floor((double(value) * gMFraction()) + 0.5));
    }

    if (group == QLCChannel::Intensity)
        m_gMIntensityChannels << channel;
    else
        m_gMNonIntensityChannels << channel;

    return value;
}

/****************************************************************************
 * Writing
 ****************************************************************************/

bool UniverseArray::write(int channel, uchar value, QLCChannel::Group group)
{
    if (channel >= size())
        return false;

    if (checkHTP(channel, value, group) == false)
        return false;

    m_preGMValues->data()[channel] = char(value);
    value = applyGM(channel, value, group);
    m_postGMValues->data()[channel] = char(value);

    return true;
}
