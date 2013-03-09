/*
  Q Light Controller
  vcproperties.cpp

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

#include <QWidget>
#include <QtXml>

#include "qlcfile.h"

#include "virtualconsole.h"
#include "vcproperties.h"
#include "outputmap.h"
#include "inputmap.h"
#include "vcframe.h"
#include "doc.h"

/*****************************************************************************
 * Properties Initialization
 *****************************************************************************/

VCProperties::VCProperties()
    : m_size(QSize(1920, 1080))

    , m_tapModifier(Qt::ControlModifier)

    , m_gmChannelMode(UniverseArray::GMIntensity)
    , m_gmValueMode(UniverseArray::GMReduce)
    , m_gmSliderMode(UniverseArray::GMNormal)
    , m_gmInputUniverse(InputMap::invalidUniverse())
    , m_gmInputChannel(InputMap::invalidChannel())
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

void VCProperties::setGrandMasterChannelMode(UniverseArray::GMChannelMode mode)
{
    m_gmChannelMode = mode;
}

UniverseArray::GMChannelMode VCProperties::grandMasterChannelMode() const
{
    return m_gmChannelMode;
}

void VCProperties::setGrandMasterValueMode(UniverseArray::GMValueMode mode)
{
    m_gmValueMode = mode;
}

UniverseArray::GMValueMode VCProperties::grandMasterValueMode() const
{
    return m_gmValueMode;
}

void VCProperties::setGrandMasterSliderMode(UniverseArray::GMSliderMode mode)
{
    m_gmSliderMode = mode;
}

UniverseArray::GMSliderMode VCProperties::grandMasterSlideMode() const
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
            quint32 universe = InputMap::invalidUniverse();
            quint32 channel = InputMap::invalidChannel();

            str = tag.attribute(KXMLQLCVCPropertiesGrandMasterChannelMode);
            setGrandMasterChannelMode(UniverseArray::stringToGMChannelMode(str));

            str = tag.attribute(KXMLQLCVCPropertiesGrandMasterValueMode);
            setGrandMasterValueMode(UniverseArray::stringToGMValueMode(str));

            if (tag.hasAttribute(KXMLQLCVCPropertiesGrandMasterSliderMode))
            {
                str = tag.attribute(KXMLQLCVCPropertiesGrandMasterSliderMode);
                setGrandMasterSliderMode(UniverseArray::stringToGMSliderMode(str));
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
                     UniverseArray::gMChannelModeToString(m_gmChannelMode));

    /* Value mode */
    tag.setAttribute(KXMLQLCVCPropertiesGrandMasterValueMode,
                     UniverseArray::gMValueModeToString(m_gmValueMode));

    /* Slider mode */
    tag.setAttribute(KXMLQLCVCPropertiesGrandMasterSliderMode,
                     UniverseArray::gMSliderModeToString(m_gmSliderMode));

    /* Grand Master external input */
    if (m_gmInputUniverse != InputMap::invalidUniverse() &&
        m_gmInputChannel != InputMap::invalidChannel())
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
        *universe = InputMap::invalidUniverse();

    /* Channel */
    str = tag.attribute(KXMLQLCVCPropertiesInputChannel);
    if (str.isEmpty() == false)
        *channel = str.toUInt();
    else
        *channel = InputMap::invalidChannel();

    /* Verdict */
    if (*universe != InputMap::invalidUniverse() &&
        *channel != InputMap::invalidChannel()) {
        return true;
    } else {
        return false;
    }
}
