/*
  Q Light Controller Plus
  vcproperties.cpp

  Copyright (c) Heikki Junnila
                Massimo Callegari

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QWidget>
#include <QDebug>

#include "inputoutputmap.h"
#include "vcproperties.h"
#include "qlcchannel.h"

/*****************************************************************************
 * Properties Initialization
 *****************************************************************************/

VCProperties::VCProperties()
    : m_size(QSize(1920, 1080))
    , m_gmChannelMode(GrandMaster::Intensity)
    , m_gmValueMode(GrandMaster::Reduce)
    , m_gmSliderMode(GrandMaster::Normal)
    , m_gmInputUniverse(InputOutputMap::invalidUniverse())
    , m_gmInputChannel(QLCChannel::invalid())
{
}

VCProperties::VCProperties(const VCProperties& properties)
    : m_size(properties.m_size)
    , m_gmChannelMode(properties.m_gmChannelMode)
    , m_gmValueMode(properties.m_gmValueMode)
    , m_gmSliderMode(properties.m_gmSliderMode)
    , m_gmInputUniverse(properties.m_gmInputUniverse)
    , m_gmInputChannel(properties.m_gmInputChannel)
{
}

VCProperties::~VCProperties()
{
}

VCProperties &VCProperties::operator=(const VCProperties &props)
{
    if (this != &props)
    {
        m_size = props.m_size;
        m_gmChannelMode = props.m_gmChannelMode;
        m_gmValueMode = props.m_gmValueMode;
        m_gmSliderMode = props.m_gmSliderMode;
        m_gmInputUniverse = props.m_gmInputUniverse;
        m_gmInputChannel = props.m_gmInputChannel;
    }

    return *this;
}

/*****************************************************************************
 * Size
 *****************************************************************************/

void VCProperties::setSize(const QSize& size)
{
    m_size = size;
}

QSize VCProperties::size() const
{
    return m_size;
}

/*****************************************************************************
 * Grand Master
 *****************************************************************************/

void VCProperties::setGrandMasterChannelMode(GrandMaster::ChannelMode mode)
{
    m_gmChannelMode = mode;
}

GrandMaster::ChannelMode VCProperties::grandMasterChannelMode() const
{
    return m_gmChannelMode;
}

void VCProperties::setGrandMasterValueMode(GrandMaster::ValueMode mode)
{
    m_gmValueMode = mode;
}

GrandMaster::ValueMode VCProperties::grandMasterValueMode() const
{
    return m_gmValueMode;
}

void VCProperties::setGrandMasterSliderMode(GrandMaster::SliderMode mode)
{
    m_gmSliderMode = mode;
}

GrandMaster::SliderMode VCProperties::grandMasterSlideMode() const
{
    return m_gmSliderMode;
}

void VCProperties::setGrandMasterInputSource(quint32 universe, quint32 channel)
{
    m_gmInputUniverse = universe;
    m_gmInputChannel = channel;
}

quint32 VCProperties::grandMasterInputUniverse() const
{
    return m_gmInputUniverse;
}

quint32 VCProperties::grandMasterInputChannel() const
{
    return m_gmInputChannel;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCProperties::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCProperties)
    {
        qWarning() << Q_FUNC_INFO << "Virtual console properties node not found";
        return false;
    }

    QString str;
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCVCPropertiesSize)
        {
            QSize sz;

            /* Width */
            str = root.attributes().value(KXMLQLCVCPropertiesSizeWidth).toString();
            if (str.isEmpty() == false)
                sz.setWidth(str.toInt());

            /* Height */
            str = root.attributes().value(KXMLQLCVCPropertiesSizeHeight).toString();
            if (str.isEmpty() == false)
                sz.setHeight(str.toInt());

            /* Set size if both are valid */
            if (sz.isValid() == true)
                setSize(sz);
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLQLCVCPropertiesGrandMaster)
        {
            QXmlStreamAttributes attrs = root.attributes();

            str = attrs.value(KXMLQLCVCPropertiesGrandMasterChannelMode).toString();
            setGrandMasterChannelMode(GrandMaster::stringToChannelMode(str));

            str = attrs.value(KXMLQLCVCPropertiesGrandMasterValueMode).toString();
            setGrandMasterValueMode(GrandMaster::stringToValueMode(str));

            if (attrs.hasAttribute(KXMLQLCVCPropertiesGrandMasterSliderMode))
            {
                str = attrs.value(KXMLQLCVCPropertiesGrandMasterSliderMode).toString();
                setGrandMasterSliderMode(GrandMaster::stringToSliderMode(str));
            }

            QXmlStreamReader::TokenType tType = root.readNext();
            if (tType == QXmlStreamReader::Characters)
                tType = root.readNext();

            // check if there is a Input tag defined
            if (tType == QXmlStreamReader::StartElement)
            {
                if (root.name() == KXMLQLCVCPropertiesInput)
                {
                    quint32 universe = InputOutputMap::invalidUniverse();
                    quint32 channel = QLCChannel::invalid();
                    /* External input */
                    if (loadXMLInput(root, &universe, &channel) == true)
                        setGrandMasterInputSource(universe, channel);
                }
                root.skipCurrentElement();
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown virtual console property tag:"
                       << root.name().toString();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool VCProperties::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    /* Properties entry */
    doc->writeStartElement(KXMLQLCVCProperties);

    /* Size */
    doc->writeStartElement(KXMLQLCVCPropertiesSize);
    doc->writeAttribute(KXMLQLCVCPropertiesSizeWidth, QString::number(size().width()));
    doc->writeAttribute(KXMLQLCVCPropertiesSizeHeight, QString::number(size().height()));
    doc->writeEndElement();

    /***********************
     * Grand Master slider *
     ***********************/
    doc->writeStartElement(KXMLQLCVCPropertiesGrandMaster);

    /* Channel mode */
    doc->writeAttribute(KXMLQLCVCPropertiesGrandMasterChannelMode,
                     GrandMaster::channelModeToString(m_gmChannelMode));

    /* Value mode */
    doc->writeAttribute(KXMLQLCVCPropertiesGrandMasterValueMode,
                     GrandMaster::valueModeToString(m_gmValueMode));

    /* Slider mode */
    doc->writeAttribute(KXMLQLCVCPropertiesGrandMasterSliderMode,
                     GrandMaster::sliderModeToString(m_gmSliderMode));

    /* Grand Master external input */
    if (m_gmInputUniverse != InputOutputMap::invalidUniverse() &&
        m_gmInputChannel != QLCChannel::invalid())
    {
        doc->writeStartElement(KXMLQLCVCPropertiesInput);
        doc->writeAttribute(KXMLQLCVCPropertiesInputUniverse,
                            QString("%1").arg(m_gmInputUniverse));
        doc->writeAttribute(KXMLQLCVCPropertiesInputChannel,
                            QString("%1").arg(m_gmInputChannel));
        doc->writeEndElement();
    }
    /* End the <GrandMaster> tag */
    doc->writeEndElement();

    /* End the <Properties> tag */
    doc->writeEndElement();

    return true;
}

bool VCProperties::loadXMLInput(QXmlStreamReader &root, quint32* universe, quint32* channel)
{
    /* External input */
    if (root.name() != KXMLQLCVCPropertiesInput)
        return false;

    QXmlStreamAttributes attrs = root.attributes();

    /* Universe */
    QString str = attrs.value(KXMLQLCVCPropertiesInputUniverse).toString();
    if (str.isEmpty() == false)
        *universe = str.toUInt();
    else
        *universe = InputOutputMap::invalidUniverse();

    /* Channel */
    str = attrs.value(KXMLQLCVCPropertiesInputChannel).toString();
    if (str.isEmpty() == false)
        *channel = str.toUInt();
    else
        *channel = QLCChannel::invalid();

    root.skipCurrentElement();

    /* Verdict */
    if (*universe != InputOutputMap::invalidUniverse() &&
        *channel != QLCChannel::invalid())
    {
        return true;
    }
    else
    {
        return false;
    }
}
