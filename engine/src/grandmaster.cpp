/*
  Q Light Controller Plus
  grandmaster.cpp

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

#include <climits>

#include "grandmaster.h"
#include "qlcmacros.h"

#define KXMLQLCGMValueModeLimit "Limit"
#define KXMLQLCGMValueModeReduce "Reduce"
#define KXMLQLCGMChannelModeAllChannels "All"
#define KXMLQLCGMChannelModeIntensity "Intensity"
#define KXMLQLCGMSliderModeNormal "Normal"
#define KXMLQLCGMSliderModeInverted "Inverted"

GrandMaster::GrandMaster(QObject *parent)
    : QObject(parent)
    , m_valueMode(Reduce)
    , m_channelMode(Intensity)
    , m_value(255)
    , m_fraction(1.0)
{
}

GrandMaster::~GrandMaster()
{
}

GrandMaster::ValueMode GrandMaster::stringToValueMode(const QString& str)
{
    if (str == KXMLQLCGMValueModeLimit)
        return GrandMaster::Limit;
    else
        return GrandMaster::Reduce;
}

QString GrandMaster::valueModeToString(GrandMaster::ValueMode mode)
{
    switch (mode)
    {
    case GrandMaster::Limit:
        return KXMLQLCGMValueModeLimit;
    default:
    case GrandMaster::Reduce:
        return KXMLQLCGMValueModeReduce;
    }
}

GrandMaster::ChannelMode GrandMaster::stringToChannelMode(const QString& str)
{
    if (str == KXMLQLCGMChannelModeAllChannels)
        return GrandMaster::AllChannels;
    else
        return GrandMaster::Intensity;
}

QString GrandMaster::channelModeToString(GrandMaster::ChannelMode mode)
{
    switch (mode)
    {
    case GrandMaster::AllChannels:
        return KXMLQLCGMChannelModeAllChannels;
    default:
    case GrandMaster::Intensity:
        return KXMLQLCGMChannelModeIntensity;
    }
}

GrandMaster::SliderMode GrandMaster::stringToSliderMode(const QString &str)
{
    if (str == KXMLQLCGMSliderModeInverted)
        return GrandMaster::Inverted;
    else
        return GrandMaster::Normal;
}

QString GrandMaster::sliderModeToString(GrandMaster::SliderMode mode)
{
    switch (mode)
    {
    case GrandMaster::Inverted:
        return KXMLQLCGMSliderModeInverted;
    default:
    case GrandMaster::Normal:
        return KXMLQLCGMSliderModeNormal;
    }
}

void GrandMaster::setValueMode(GrandMaster::ValueMode mode)
{
    if (m_valueMode != mode)
    {
        m_valueMode = mode;
        setValue(value());
    }
}

GrandMaster::ValueMode GrandMaster::valueMode() const
{
    return m_valueMode;
}

void GrandMaster::setChannelMode(GrandMaster::ChannelMode mode)
{
    if (m_channelMode != mode)
    {
        m_channelMode = mode;
        setValue(value());
    }
/*
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
*/
}

GrandMaster::ChannelMode GrandMaster::channelMode() const
{
    return m_channelMode;
}

void GrandMaster::setValue(uchar value)
{
    m_value = value;
    m_fraction = CLAMP(double(value) / double(UCHAR_MAX), 0.0, 1.0);

    emit valueChanged(value);
}

uchar GrandMaster::value() const
{
    return m_value;
}

double GrandMaster::fraction() const
{
    return m_fraction;
}

