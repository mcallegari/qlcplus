/*
  Q Light Controller
  vcproperties.cpp

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

#include <QWidget>
#include <QtXml>

#include "qlcfile.h"

#include "virtualconsole.h"
#include "inputoutputmap.h"
#include "vcproperties.h"
#include "vcframe.h"
#include "doc.h"

/*****************************************************************************
 * Properties Initialization
 *****************************************************************************/

VCProperties::VCProperties()
    : m_size(QSize(1920, 1080))

    , m_tapModifier(Qt::ControlModifier)

    , m_gmChannelMode(GrandMaster::Intensity)
    , m_gmValueMode(GrandMaster::Reduce)
    , m_gmSliderMode(GrandMaster::Normal)
    , m_gmInputUniverse(InputOutputMap::invalidUniverse())
    , m_gmInputChannel(QLCChannel::invalid())
{
}

VCProperties::VCProperties(const VCProperties& properties)
    : m_size(properties.m_size)

    , m_tapModifier(properties.m_tapModifier)

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
 * Keyboard
 *****************************************************************************/

void VCProperties::setTapModifier(Qt::KeyboardModifier mod)
{
    m_tapModifier = mod;
}

Qt::KeyboardModifier VCProperties::tapModifier() const
{
    return m_tapModifier;
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

bool VCProperties::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCVCProperties)
    {
        qWarning() << Q_FUNC_INFO << "Virtual console properties node not found";
        return false;
    }

    QString str;
    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCVCPropertiesSize)
        {
            QSize sz;

            /* Width */
            str = tag.attribute(KXMLQLCVCPropertiesSizeWidth);
            if (str.isEmpty() == false)
                sz.setWidth(str.toInt());

            /* Height */
            str = tag.attribute(KXMLQLCVCPropertiesSizeHeight);
            if (str.isEmpty() == false)
                sz.setHeight(str.toInt());

            /* Set size if both are valid */
            if (sz.isValid() == true)
                setSize(sz);
        }
        else if (tag.tagName() == KXMLQLCVCPropertiesKeyboard)
        {
            /* Tap modifier */
            str = tag.attribute(KXMLQLCVCPropertiesKeyboardTapModifier);
            if (str.isEmpty() == false)
                setTapModifier(Qt::KeyboardModifier(str.toInt()));
        }
        else if (tag.tagName() == KXMLQLCVCPropertiesGrandMaster)
        {
            quint32 universe = InputOutputMap::invalidUniverse();
            quint32 channel = QLCChannel::invalid();

            str = tag.attribute(KXMLQLCVCPropertiesGrandMasterChannelMode);
            setGrandMasterChannelMode(GrandMaster::stringToChannelMode(str));

            str = tag.attribute(KXMLQLCVCPropertiesGrandMasterValueMode);
            setGrandMasterValueMode(GrandMaster::stringToValueMode(str));

            if (tag.hasAttribute(KXMLQLCVCPropertiesGrandMasterSliderMode))
            {
                str = tag.attribute(KXMLQLCVCPropertiesGrandMasterSliderMode);
                setGrandMasterSliderMode(GrandMaster::stringToSliderMode(str));
            }

            /* External input */
            if (loadXMLInput(tag.firstChild().toElement(), &universe, &channel) == true)
                setGrandMasterInputSource(universe, channel);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown virtual console property tag:"
                       << tag.tagName();
        }

        /* Next node */
        node = node.nextSibling();
    }

    return true;
}

bool VCProperties::saveXML(QDomDocument* doc, QDomElement* wksp_root) const
{
    QDomElement prop_root;
    QDomElement subtag;
    QDomElement tag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    /* Properties entry */
    prop_root = doc->createElement(KXMLQLCVCProperties);
    wksp_root->appendChild(prop_root);

    /* Size */
    tag = doc->createElement(KXMLQLCVCPropertiesSize);
    tag.setAttribute(KXMLQLCVCPropertiesSizeWidth, QString::number(size().width()));
    tag.setAttribute(KXMLQLCVCPropertiesSizeHeight, QString::number(size().height()));
    prop_root.appendChild(tag);

    /* Keyboard */
    tag = doc->createElement(KXMLQLCVCPropertiesKeyboard);
    tag.setAttribute(KXMLQLCVCPropertiesKeyboardTapModifier, tapModifier());
    prop_root.appendChild(tag);

    /***********************
     * Grand Master slider *
     ***********************/
    tag = doc->createElement(KXMLQLCVCPropertiesGrandMaster);
    prop_root.appendChild(tag);

    /* Channel mode */
    tag.setAttribute(KXMLQLCVCPropertiesGrandMasterChannelMode,
                     GrandMaster::channelModeToString(m_gmChannelMode));

    /* Value mode */
    tag.setAttribute(KXMLQLCVCPropertiesGrandMasterValueMode,
                     GrandMaster::valueModeToString(m_gmValueMode));

    /* Slider mode */
    tag.setAttribute(KXMLQLCVCPropertiesGrandMasterSliderMode,
                     GrandMaster::sliderModeToString(m_gmSliderMode));

    /* Grand Master external input */
    if (m_gmInputUniverse != InputOutputMap::invalidUniverse() &&
        m_gmInputChannel != QLCChannel::invalid())
    {
        subtag = doc->createElement(KXMLQLCVCPropertiesInput);
        tag.appendChild(subtag);
        subtag.setAttribute(KXMLQLCVCPropertiesInputUniverse,
                            QString("%1").arg(m_gmInputUniverse));
        subtag.setAttribute(KXMLQLCVCPropertiesInputChannel,
                            QString("%1").arg(m_gmInputChannel));
    }

    return true;
}

bool VCProperties::loadXMLInput(const QDomElement& tag, quint32* universe, quint32* channel)
{
    /* External input */
    if (tag.tagName() != KXMLQLCVCPropertiesInput)
        return false;

    /* Universe */
    QString str = tag.attribute(KXMLQLCVCPropertiesInputUniverse);
    if (str.isEmpty() == false)
        *universe = str.toUInt();
    else
        *universe = InputOutputMap::invalidUniverse();

    /* Channel */
    str = tag.attribute(KXMLQLCVCPropertiesInputChannel);
    if (str.isEmpty() == false)
        *channel = str.toUInt();
    else
        *channel = QLCChannel::invalid();

    /* Verdict */
    if (*universe != InputOutputMap::invalidUniverse() &&
        *channel != QLCChannel::invalid()) {
        return true;
    } else {
        return false;
    }
}
