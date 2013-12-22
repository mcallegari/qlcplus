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

#define KXMLQLCGMValueModeLimit "Limit"
#define KXMLQLCGMValueModeReduce "Reduce"
#define KXMLQLCGMChannelModeAllChannels "All"
#define KXMLQLCGMChannelModeIntensity "Intensity"
#define KXMLQLCGMSliderModeNormal "Normal"
#define KXMLQLCGMSliderModeInverted "Inverted"

#include "grandmaster.h"
#include "qlcmacros.h"

GrandMaster::GrandMaster(QObject *parent)
    : QObject(parent)
{
    m_gMChannelMode = GMIntensity;
    m_gMValueMode = GMReduce;
    m_gMValue = 255;
    m_gMFraction = 1.0;
}

GrandMaster::~GrandMaster()
{
}

GrandMaster::GMValueMode GrandMaster::stringToGMValueMode(const QString& str)
{
    if (str == KXMLQLCGMValueModeLimit)
        return GrandMaster::GMLimit;
    else
        return GrandMaster::GMReduce;
}

QString GrandMaster::gMValueModeToString(GrandMaster::GMValueMode mode)
{
    switch (mode)
    {
    case GrandMaster::GMLimit:
        return KXMLQLCGMValueModeLimit;
    default:
    case GrandMaster::GMReduce:
        return KXMLQLCGMValueModeReduce;
    }
}

GrandMaster::GMChannelMode GrandMaster::stringToGMChannelMode(const QString& str)
{
    if (str == KXMLQLCGMChannelModeAllChannels)
        return GrandMaster::GMAllChannels;
    else
        return GrandMaster::GMIntensity;
}

QString GrandMaster::gMChannelModeToString(GrandMaster::GMChannelMode mode)
{
    switch (mode)
    {
    case GrandMaster::GMAllChannels:
        return KXMLQLCGMChannelModeAllChannels;
    default:
    case GrandMaster::GMIntensity:
        return KXMLQLCGMChannelModeIntensity;
    }
}

GrandMaster::GMSliderMode GrandMaster::stringToGMSliderMode(const QString &str)
{
    if (str == KXMLQLCGMSliderModeInverted)
        return GrandMaster::GMInverted;
    else
        return GrandMaster::GMNormal;
}

QString GrandMaster::gMSliderModeToString(GrandMaster::GMSliderMode mode)
{
    switch (mode)
    {
    case GrandMaster::GMInverted:
        return KXMLQLCGMSliderModeInverted;
    default:
    case GrandMaster::GMNormal:
        return KXMLQLCGMSliderModeNormal;
    }
}

void GrandMaster::setGMValueMode(GrandMaster::GMValueMode mode)
{
    if (m_gMValueMode != mode)
    {
        m_gMValueMode = mode;
        setGMValue(gMValue());
    }
}

GrandMaster::GMValueMode GrandMaster::gMValueMode() const
{
    return m_gMValueMode;
}

void GrandMaster::setGMChannelMode(GrandMaster::GMChannelMode mode)
{
    if (m_gMChannelMode != mode)
    {
        m_gMChannelMode = mode;
        setGMValue(gMValue());
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

GrandMaster::GMChannelMode GrandMaster::gMChannelMode() const
{
    return m_gMChannelMode;
}

void GrandMaster::setGMValue(uchar value)
{
    m_gMValue = value;
    m_gMFraction = CLAMP(double(value) / double(UCHAR_MAX), 0.0, 1.0);

    emit valueChanged(value);
}

uchar GrandMaster::gMValue() const
{
    return m_gMValue;
}

double GrandMaster::gMFraction() const
{
    return m_gMFraction;
}



