/*
  Q Light Controller Plus
  universe.cpp

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

#include <math.h>

#include "universe.h"
#include "grandmaster.h"
#include "qlcmacros.h"

Universe::Universe(GrandMaster *gm, QObject *parent)
    : QObject(parent)
    , m_grandMaster(gm)
    , m_usedChannels(0)
    , m_hasChanged(false)
    , m_channelsMask(new QByteArray(512, char(0)))
    , m_preGMValues(new QByteArray(512, char(0)))
    , m_postGMValues(new QByteArray(512, char(0)))
{
    m_relativeValues.fill(0, 512);

    connect(m_grandMaster, SIGNAL(valueChanged(uchar)),
            this, SLOT(slotGMValueChanged()));
}

Universe::~Universe()
{
    delete m_preGMValues;
    delete m_postGMValues;
}

void Universe::setName(QString name)
{
    m_name = name;
}

QString Universe::name()
{
    return m_name;
}

short Universe::usedChannels()
{
    return m_usedChannels;
}

void Universe::resetChanged()
{
    m_usedChannels = false;
}

bool Universe::hasChanged()
{
    return m_hasChanged;
}

void Universe::slotGMValueChanged()
{
    if (m_grandMaster->gMChannelMode() == GrandMaster::GMIntensity)
    {
        QSetIterator <int> it(m_gMIntensityChannels);
        while (it.hasNext() == true)
        {
            int channel(it.next());
            char chValue(m_preGMValues->data()[channel]);
            write(channel, chValue);
        }
    }

    if (m_grandMaster->gMChannelMode() == GrandMaster::GMAllChannels)
    {
        QSetIterator <int> it(m_gMNonIntensityChannels);
        while (it.hasNext() == true)
        {
            int channel(it.next());
            char chValue(m_preGMValues->data()[channel]);
            write(channel, chValue);
        }
    }
}

/************************************************************************
 * Values
 ************************************************************************/

void Universe::reset()
{
    m_preGMValues->fill(0);
    m_postGMValues->fill(0);
    m_relativeValues.fill(0);
    //m_doRelative = false;
}

void Universe::reset(int address, int range)
{
    for (int i = address; i < address + range && i < 512; i++)
    {
        m_preGMValues->data()[i] = 0;
        m_postGMValues->data()[i] = 0;
        m_gMIntensityChannels.remove(i);
        m_gMNonIntensityChannels.remove(i);
        m_relativeValues[i] = 0;
    }
}


void Universe::zeroIntensityChannels()
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

QHash<int, uchar> Universe::intensityChannels()
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

const QByteArray* Universe::postGMValues() const
{
    return m_postGMValues;
}

void Universe::zeroRelativeValues()
{
    m_relativeValues.fill(0);
}

const QByteArray Universe::preGMValues() const
{
    if (m_preGMValues->isNull())
        return QByteArray();
    return *m_preGMValues;
}

uchar Universe::applyGM(int channel, uchar value)
{
    if (value == 0)
        return 0;

    if ((m_grandMaster->gMChannelMode() == GrandMaster::GMIntensity && m_channelsMask->at(channel) & Intensity) ||
        (m_grandMaster->gMChannelMode() == GrandMaster::GMAllChannels))
    {
        if (m_grandMaster->gMValueMode() == GrandMaster::GMLimit)
            value = MIN(value, m_grandMaster->gMValue());
        else
            value = char(floor((double(value) * m_grandMaster->gMFraction()) + 0.5));
    }

    return value;
}

/************************************************************************
 * Channels capabilities
 ************************************************************************/

void Universe::setChannelCapability(ushort channel, QLCChannel::Group group, bool isHTP)
{
    if (channel >= (ushort)m_channelsMask->count())
        return;

    if (isHTP == true)
    {
        m_channelsMask->data()[channel] = char(HTP);
    }
    else
    {
        if (group == QLCChannel::Intensity)
        {
            m_channelsMask->data()[channel] = char(HTP & Intensity);
            m_gMIntensityChannels << channel;
        }
        else
        {
            m_channelsMask->data()[channel] = char(LTP);
            m_gMNonIntensityChannels << channel;
        }
    }
    return;
}

uchar Universe::channelCapabilities(ushort channel)
{
    if (channel >= (ushort)m_channelsMask->count())
        return Undefined;

    return m_channelsMask->data()[channel];
}

/****************************************************************************
 * Writing
 ****************************************************************************/

bool Universe::write(int channel, uchar value)
{
    if (channel >= 512)
        return false;

    if (channel > m_usedChannels)
        m_usedChannels = channel + 1;

    if ((m_channelsMask->data()[channel] & HTP) == 0 && value < m_preGMValues->data()[channel])
        return false;

    if (m_preGMValues != NULL)
        m_preGMValues->data()[channel] = char(value);

    value = applyGM(channel, value);
    m_postGMValues->data()[channel] = char(value);

    m_hasChanged = true;

    return true;
}

bool Universe::writeRelative(int channel, uchar value)
{
    if (channel >= 512)
        return false;

    if (channel > m_usedChannels)
        m_usedChannels = channel + 1;

    if (value == 127)
        return true;

    m_relativeValues[channel] += value - 127;

    int val = m_relativeValues[channel];
    if (m_preGMValues != NULL)
        val += (uchar)m_preGMValues->data()[channel];
    value = CLAMP(val, 0, UCHAR_MAX);

    value = applyGM(channel, value);
    m_postGMValues->data()[channel] = char(value);

    m_hasChanged = true;

    return true;
}

