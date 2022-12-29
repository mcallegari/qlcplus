/*
  Q Light Controller Plus
  qlcphysical.cpp

  Copyright (C) Heikki Junnila
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
#include <QRegularExpression>
#include <QLocale>
#include <QString>
#include <QDebug>

#include "qlcphysical.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

QLCPhysical::QLCPhysical()
    : m_bulbLumens(0)
    , m_bulbColourTemperature(0)
    , m_weight(0)
    , m_width(0)
    , m_height(0)
    , m_depth(0)
    , m_lensDegreesMin(0)
    , m_lensDegreesMax(0)
    , m_focusPanMax(0)
    , m_focusTiltMax(0)
    , m_layout(QSize(1, 1))
    , m_powerConsumption(0)

{
    m_lensName = "Other";
    m_focusType = "Fixed";
    m_dmxConnector = "5-pin";
}

QLCPhysical::~QLCPhysical()
{
}

QLCPhysical::QLCPhysical(const QLCPhysical &other)
{
    *this = other;
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
        m_layout = physical.layoutSize();

        m_powerConsumption = physical.powerConsumption();
        m_dmxConnector = physical.dmxConnector();
    }

    return *this;
}

bool QLCPhysical::isEmpty() const
{
    if (m_bulbLumens == 0 &&
        m_bulbColourTemperature == 0 &&
        m_weight == 0 &&
        m_width == 0 &&
        m_height == 0 &&
        m_depth == 0 &&
        m_lensDegreesMin == 0 &&
        m_lensDegreesMax == 0 &&
        m_focusPanMax == 0 &&
        m_focusTiltMax == 0 &&
        m_powerConsumption == 0)
        return true;

    return false;
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

void QLCPhysical::setLensDegreesMin(double degrees)
{
    m_lensDegreesMin = degrees;
}

double QLCPhysical::lensDegreesMin() const
{
    return m_lensDegreesMin;
}

void QLCPhysical::setLensDegreesMax(double degrees)
{
    m_lensDegreesMax = degrees;
}

double QLCPhysical::lensDegreesMax() const
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

void QLCPhysical::setLayoutSize(QSize size)
{
    m_layout = size;
}

QSize QLCPhysical::layoutSize() const
{
    return m_layout;
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
        int bulbWatts = bulbType().remove(QRegularExpression("[A-Z]*")).toInt();
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

bool QLCPhysical::loadXML(QXmlStreamReader &doc)
{
    if (doc.name() != KXMLQLCPhysical)
    {
        qWarning() << Q_FUNC_INFO << "Physical node not found";
        return false;
    }

    /* Subtags */
    while (doc.readNextStartElement())
    {
        QXmlStreamAttributes attrs = doc.attributes();
        if (doc.name() == KXMLQLCPhysicalBulb)
        {
            m_bulbType = attrs.value(KXMLQLCPhysicalBulbType).toString();
            m_bulbLumens = attrs.value(KXMLQLCPhysicalBulbLumens).toString().toInt();
            m_bulbColourTemperature = attrs.value(KXMLQLCPhysicalBulbColourTemperature).toString().toInt();
        }
        else if (doc.name() == KXMLQLCPhysicalDimensions)
        {
            m_weight = QLocale::c().toDouble(attrs.value(KXMLQLCPhysicalDimensionsWeight).toString());
            m_width = attrs.value(KXMLQLCPhysicalDimensionsWidth).toString().toInt();
            m_height = attrs.value(KXMLQLCPhysicalDimensionsHeight).toString().toInt();
            m_depth = attrs.value(KXMLQLCPhysicalDimensionsDepth).toString().toInt();
        }
        else if (doc.name() == KXMLQLCPhysicalLens)
        {
            m_lensName = attrs.value(KXMLQLCPhysicalLensName).toString();
            m_lensDegreesMin = QLocale::c().toDouble(attrs.value(KXMLQLCPhysicalLensDegreesMin).toString());
            m_lensDegreesMax = QLocale::c().toDouble(attrs.value(KXMLQLCPhysicalLensDegreesMax).toString());
        }
        else if (doc.name() == KXMLQLCPhysicalFocus)
        {
            m_focusType = attrs.value(KXMLQLCPhysicalFocusType).toString();
            m_focusPanMax = attrs.value(KXMLQLCPhysicalFocusPanMax).toString().toInt();
            m_focusTiltMax = attrs.value(KXMLQLCPhysicalFocusTiltMax).toString().toInt();
        }
        else if (doc.name() == KXMLQLCPhysicalLayout)
        {
            int columns = 1, rows = 1;
            if (attrs.hasAttribute(KXMLQLCPhysicalDimensionsWidth))
                columns = attrs.value(KXMLQLCPhysicalDimensionsWidth).toString().toInt();
            if (attrs.hasAttribute(KXMLQLCPhysicalDimensionsHeight))
                rows = attrs.value(KXMLQLCPhysicalDimensionsHeight).toString().toInt();
            setLayoutSize(QSize(columns, rows));
        }
        else if (doc.name() == KXMLQLCPhysicalTechnical)
        {
            m_powerConsumption = attrs.value(KXMLQLCPhysicalTechnicalPowerConsumption).toString().toInt();
            m_dmxConnector = attrs.value(KXMLQLCPhysicalTechnicalDmxConnector).toString();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown Physical tag:" << doc.name();
        }
        doc.skipCurrentElement();
    }

    return true;
}

bool QLCPhysical::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    /* Physical entry */
    doc->writeStartElement(KXMLQLCPhysical);

    /* Bulb */
    doc->writeStartElement(KXMLQLCPhysicalBulb);
    doc->writeAttribute(KXMLQLCPhysicalBulbType, m_bulbType);
    doc->writeAttribute(KXMLQLCPhysicalBulbLumens, QString::number(m_bulbLumens));
    doc->writeAttribute(KXMLQLCPhysicalBulbColourTemperature, QString::number(m_bulbColourTemperature));
    doc->writeEndElement();

    /* Dimensions */
    doc->writeStartElement(KXMLQLCPhysicalDimensions);
    doc->writeAttribute(KXMLQLCPhysicalDimensionsWeight, QLocale::c().toString(m_weight));
    doc->writeAttribute(KXMLQLCPhysicalDimensionsWidth, QString::number(m_width));
    doc->writeAttribute(KXMLQLCPhysicalDimensionsHeight, QString::number(m_height));
    doc->writeAttribute(KXMLQLCPhysicalDimensionsDepth, QString::number(m_depth));
    doc->writeEndElement();

    /* Lens */
    doc->writeStartElement(KXMLQLCPhysicalLens);
    doc->writeAttribute(KXMLQLCPhysicalLensName, m_lensName);
    doc->writeAttribute(KXMLQLCPhysicalLensDegreesMin, QLocale::c().toString(m_lensDegreesMin));
    doc->writeAttribute(KXMLQLCPhysicalLensDegreesMax, QLocale::c().toString(m_lensDegreesMax));
    doc->writeEndElement();

    /* Focus */
    doc->writeStartElement(KXMLQLCPhysicalFocus);
    doc->writeAttribute(KXMLQLCPhysicalFocusType, m_focusType);
    doc->writeAttribute(KXMLQLCPhysicalFocusPanMax, QString::number(m_focusPanMax));
    doc->writeAttribute(KXMLQLCPhysicalFocusTiltMax, QString::number(m_focusTiltMax));
    doc->writeEndElement();

    /* Layout */
    if (layoutSize() != QSize(1, 1))
    {
        doc->writeStartElement(KXMLQLCPhysicalLayout);
        doc->writeAttribute(KXMLQLCPhysicalDimensionsWidth, QString::number(m_layout.width()));
        doc->writeAttribute(KXMLQLCPhysicalDimensionsHeight, QString::number(m_layout.height()));
        doc->writeEndElement();
    }

    /* Technical */
    doc->writeStartElement(KXMLQLCPhysicalTechnical);
    doc->writeAttribute(KXMLQLCPhysicalTechnicalPowerConsumption, QString::number(m_powerConsumption));
    doc->writeAttribute(KXMLQLCPhysicalTechnicalDmxConnector, m_dmxConnector);
    doc->writeEndElement();

    doc->writeEndElement();

    return true;
}
