/*
  Q Light Controller
  qlcphysical.cpp

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

#include <QString>
#include <QtXml>

#include "qlcphysical.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

QLCPhysical::QLCPhysical()
{
    /* Initialize only integer values since QStrings are null by default */
    m_bulbLumens = 0;
    m_bulbColourTemperature = 0;

    m_weight = 0;
    m_width = 0;
    m_height = 0;
    m_depth = 0;

    m_lensName = "Other";
    m_lensDegreesMin = 0;
    m_lensDegreesMax = 0;

    m_focusType = "Fixed";
    m_focusPanMax = 0;
    m_focusTiltMax = 0;

    m_powerConsumption = 0;
    m_dmxConnector = "5-pin";
}

QLCPhysical::~QLCPhysical()
{
}

QLCPhysical& QLCPhysical::operator=(const QLCPhysical& physical)
{
    if (this != &physical)
    {
        m_bulbType = physical.bulbType();
        m_bulbLumens = physical.bulbLumens();
        m_bulbColourTemperature = physical.bulbColourTemperature();

        m_weight = physical.weight();
        m_width = physical.width();
        m_height = physical.height();
        m_depth = physical.depth();

        m_lensName = physical.lensName();
        m_lensDegreesMin = physical.lensDegreesMin();
        m_lensDegreesMax = physical.lensDegreesMax();

        m_focusType = physical.focusType();
        m_focusPanMax = physical.focusPanMax();
        m_focusTiltMax = physical.focusTiltMax();

        m_powerConsumption = physical.powerConsumption();
        m_dmxConnector = physical.dmxConnector();
    }

    return *this;
}

/****************************************************************************
 * Properties
 ****************************************************************************/

void QLCPhysical::setBulbType(const QString& type)
{
    m_bulbType = type;
}

QString QLCPhysical::bulbType() const
{
    return m_bulbType;
}

void QLCPhysical::setBulbLumens(int lumens)
{
    m_bulbLumens = lumens;
}

int QLCPhysical::bulbLumens() const
{
    return m_bulbLumens;
}

void QLCPhysical::setBulbColourTemperature(int temp)
{
    m_bulbColourTemperature = temp;
}

int QLCPhysical::bulbColourTemperature() const
{
    return m_bulbColourTemperature;
}

void QLCPhysical::setWeight(double weight)
{
    m_weight = weight;
}

double QLCPhysical::weight() const
{
    return m_weight;
}

void QLCPhysical::setWidth(int width)
{
    m_width = width;
}

int QLCPhysical::width() const
{
    return m_width;
}

void QLCPhysical::setHeight(int height)
{
    m_height = height;
}

int QLCPhysical::height() const
{
    return m_height;
}

void QLCPhysical::setDepth(int depth)
{
    m_depth = depth;
}

int QLCPhysical::depth() const
{
    return m_depth;
}

void QLCPhysical::setLensName(const QString& name)
{
    m_lensName = name;
}

QString QLCPhysical::lensName() const
{
    return m_lensName;
}

void QLCPhysical::setLensDegreesMin(qreal degrees)
{
    m_lensDegreesMin = degrees;
}

qreal QLCPhysical::lensDegreesMin() const
{
    return m_lensDegreesMin;
}

void QLCPhysical::setLensDegreesMax(qreal degrees)
{
    m_lensDegreesMax = degrees;
}

qreal QLCPhysical::lensDegreesMax() const
{
    return m_lensDegreesMax;
}

void QLCPhysical::setFocusType(const QString& type)
{
    m_focusType = type;
}

QString QLCPhysical::focusType() const
{
    return m_focusType;
}

void QLCPhysical::setFocusPanMax(int pan)
{
    m_focusPanMax = pan;
}

int QLCPhysical::focusPanMax() const
{
    return m_focusPanMax;
}

void QLCPhysical::setFocusTiltMax(int tilt)
{
    m_focusTiltMax = tilt;
}

int QLCPhysical::focusTiltMax() const
{
    return m_focusTiltMax;
}

void QLCPhysical::setPowerConsumption(int watt)
{
    m_powerConsumption = watt;
}

int QLCPhysical::powerConsumption() const
{
    if (m_powerConsumption != 0)
    {
        return m_powerConsumption;
    }
    else
    {
        /* If power consumption value is missing, return bulb watts
         * plus a guesstimate 100W, since there's usually other
          * electronics consuming power as well. */
        int bulbWatts = bulbType().remove(QRegExp("[A-Z]")).toInt();
        if (bulbWatts > 0)
            return bulbWatts + 100;
        else
            return 0;
    }
}

void QLCPhysical::setDmxConnector(const QString& type)
{
    m_dmxConnector = type;
}

QString QLCPhysical::dmxConnector() const
{
    return m_dmxConnector;
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool QLCPhysical::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCPhysical)
    {
        qWarning() << Q_FUNC_INFO << "Physical node not found";
        return false;
    }

    /* Subtags */
    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCPhysicalBulb)
        {
            m_bulbType = tag.attribute(KXMLQLCPhysicalBulbType);
            m_bulbLumens = tag.attribute(KXMLQLCPhysicalBulbLumens).toInt();
            m_bulbColourTemperature = tag.attribute(KXMLQLCPhysicalBulbColourTemperature).toInt();
        }
        else if (tag.tagName() == KXMLQLCPhysicalDimensions)
        {
            m_weight = tag.attribute(KXMLQLCPhysicalDimensionsWeight).toDouble();
            m_width = tag.attribute(KXMLQLCPhysicalDimensionsWidth).toInt();
            m_height = tag.attribute(KXMLQLCPhysicalDimensionsHeight).toInt();
            m_depth = tag.attribute(KXMLQLCPhysicalDimensionsDepth).toInt();
        }
        else if (tag.tagName() == KXMLQLCPhysicalLens)
        {
            m_lensName = tag.attribute(KXMLQLCPhysicalLensName);
            m_lensDegreesMin = tag.attribute(KXMLQLCPhysicalLensDegreesMin).toDouble();
            m_lensDegreesMax = tag.attribute(KXMLQLCPhysicalLensDegreesMax).toDouble();
        }
        else if (tag.tagName() == KXMLQLCPhysicalFocus)
        {
            m_focusType = tag.attribute(KXMLQLCPhysicalFocusType);
            m_focusPanMax = tag.attribute(KXMLQLCPhysicalFocusPanMax).toInt();
            m_focusTiltMax = tag.attribute(KXMLQLCPhysicalFocusTiltMax).toInt();
        }
        else if (tag.tagName() == KXMLQLCPhysicalTechnical)
        {
            m_powerConsumption = tag.attribute(KXMLQLCPhysicalTechnicalPowerConsumption).toInt();
            m_dmxConnector = tag.attribute(KXMLQLCPhysicalTechnicalDmxConnector);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown Physical tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool QLCPhysical::saveXML(QDomDocument* doc, QDomElement* root)
{
    QDomElement tag;
    QDomElement subtag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(root != NULL);

    /* Physical entry */
    tag = doc->createElement(KXMLQLCPhysical);
    root->appendChild(tag);

    /* Bulb */
    subtag = doc->createElement(KXMLQLCPhysicalBulb);
    subtag.setAttribute(KXMLQLCPhysicalBulbType, m_bulbType);
    subtag.setAttribute(KXMLQLCPhysicalBulbLumens, m_bulbLumens);
    subtag.setAttribute(KXMLQLCPhysicalBulbColourTemperature, m_bulbColourTemperature);
    tag.appendChild(subtag);

    /* Dimensions */
    subtag = doc->createElement(KXMLQLCPhysicalDimensions);
    subtag.setAttribute(KXMLQLCPhysicalDimensionsWeight, QString::number(m_weight));
    subtag.setAttribute(KXMLQLCPhysicalDimensionsWidth, m_width);
    subtag.setAttribute(KXMLQLCPhysicalDimensionsHeight, m_height);
    subtag.setAttribute(KXMLQLCPhysicalDimensionsDepth, m_depth);
    tag.appendChild(subtag);

    /* Lens */
    subtag = doc->createElement(KXMLQLCPhysicalLens);
    subtag.setAttribute(KXMLQLCPhysicalLensName, m_lensName);
    subtag.setAttribute(KXMLQLCPhysicalLensDegreesMin, QString::number(m_lensDegreesMin));
    subtag.setAttribute(KXMLQLCPhysicalLensDegreesMax, QString::number(m_lensDegreesMax));
    tag.appendChild(subtag);

    /* Focus */
    subtag = doc->createElement(KXMLQLCPhysicalFocus);
    subtag.setAttribute(KXMLQLCPhysicalFocusType, m_focusType);
    subtag.setAttribute(KXMLQLCPhysicalFocusPanMax, m_focusPanMax);
    subtag.setAttribute(KXMLQLCPhysicalFocusTiltMax, m_focusTiltMax);
    tag.appendChild(subtag);

    /* Technical */
    subtag = doc->createElement(KXMLQLCPhysicalTechnical);
    subtag.setAttribute(KXMLQLCPhysicalTechnicalPowerConsumption, m_powerConsumption);
    subtag.setAttribute(KXMLQLCPhysicalTechnicalDmxConnector, m_dmxConnector);
    tag.appendChild(subtag);

    return true;
}
