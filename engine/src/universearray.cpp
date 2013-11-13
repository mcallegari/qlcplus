/*
  Q Light Controller
  universearray.cpp

  Copyright (c) Heikki Junnila

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

#include <math.h>

#include "universearray.h"
#include "qlcmacros.h"

#include <QDebug>

#define KXMLQLCGMValueModeLimit "Limit"
#define KXMLQLCGMValueModeReduce "Reduce"
#define KXMLQLCGMChannelModeAllChannels "All"
#define KXMLQLCGMChannelModeIntensity "Intensity"
#define KXMLQLCGMSliderModeNormal "Normal"
#define KXMLQLCGMSliderModeInverted "Inverted"

/****************************************************************************
 * Initialization
 ****************************************************************************/

UniverseArray::UniverseArray(int size)
    : m_size(size)
    , m_preGMValues(new QByteArray(size, char(0)))
    , m_postGMValues(new QByteArray(size, char(0)))
    , m_doRelative( false )
{
    m_gMChannelMode = GMIntensity;
    m_gMValueMode = GMReduce;
    m_gMValue = 255;
    m_gMFraction = 1.0;
    m_relativeValues.fill(0, size);
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
    m_relativeValues.fill(0);
    m_doRelative = false;
}

void UniverseArray::reset(int address, int range)
{
    for (int i = address; i < address + range && i < size(); i++)
    {
        m_preGMValues->data()[i] = 0;
        m_postGMValues->data()[i] = 0;
        m_gMIntensityChannels.remove(i);
        m_gMNonIntensityChannels.remove(i);
        m_relativeValues[i] = 0;
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
        m_relativeValues[channel] = 0;
    }
}

QHash<int, uchar> UniverseArray::intensityChannels()
{
    QHash <int, uchar> intensityList;
    QSetIterator <int> it(m_gMIntensityChannels);
    while (it.hasNext() == true)
    {
        int channel(it.next());
        intensityList[channel] = m_preGMValues->data()[channel];
    }
    return intensityList;
}

bool UniverseArray::checkHTP(int channel, uchar value, QLCChannel::Group group) const
{
    QByteArray pGM = preGMValues();
    if (pGM.isNull() || channel < 0 || channel >= pGM.size())
        return false;

    if (group == QLCChannel::Intensity && value < uchar(pGM[channel]))
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

UniverseArray::GMSliderMode UniverseArray::stringToGMSliderMode(const QString &str)
{
    if (str == KXMLQLCGMSliderModeInverted)
        return UniverseArray::GMInverted;
    else
        return UniverseArray::GMNormal;
}

QString UniverseArray::gMSliderModeToString(UniverseArray::GMSliderMode mode)
{
    switch (mode)
    {
    case UniverseArray::GMInverted:
        return KXMLQLCGMSliderModeInverted;
    default:
    case UniverseArray::GMNormal:
        return KXMLQLCGMSliderModeNormal;
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

void UniverseArray::zeroRelativeValues()
{
    m_relativeValues.fill(0);
}

const QByteArray UniverseArray::preGMValues() const
{
    if (m_preGMValues->isNull())
        return QByteArray();
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

bool UniverseArray::write(int channel, uchar value, QLCChannel::Group group, bool isRelative)
{
    if (channel >= size())
        return false;

    if (isRelative)
    {
        if (value == 127)
            return true;

        m_doRelative = true;
        m_relativeValues[channel] += value - 127;
    }
    else
    {
        if (checkHTP(channel, value, group) == false)
            return false;

        if (m_preGMValues != NULL)
            m_preGMValues->data()[channel] = char(value);
    }


    if (m_doRelative)
    {
        int val = m_relativeValues[channel];
        if (m_preGMValues != NULL)
            val += (uchar)m_preGMValues->data()[channel];
        value = CLAMP(val, 0, UCHAR_MAX);
    }

    value = applyGM(channel, value, group);
    m_postGMValues->data()[channel] = char(value);

    return true;
}
