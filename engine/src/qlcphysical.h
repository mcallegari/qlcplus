/*
  Q Light Controller
  qlcphysical.h

  Copyright (C) Heikki Junnila

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

#ifndef QLCPHYSICAL_H
#define QLCPHYSICAL_H

#include <QString>

class QDomElement;
class QDomDocument;

#define KXMLQLCPhysical "Physical"

#define KXMLQLCPhysicalBulb "Bulb"
#define KXMLQLCPhysicalBulbType "Type"
#define KXMLQLCPhysicalBulbLumens "Lumens"
#define KXMLQLCPhysicalBulbColourTemperature "ColourTemperature"

#define KXMLQLCPhysicalDimensions "Dimensions"
#define KXMLQLCPhysicalDimensionsWeight "Weight"
#define KXMLQLCPhysicalDimensionsWidth "Width"
#define KXMLQLCPhysicalDimensionsHeight "Height"
#define KXMLQLCPhysicalDimensionsDepth "Depth"

#define KXMLQLCPhysicalLens "Lens"
#define KXMLQLCPhysicalLensName "Name"
#define KXMLQLCPhysicalLensDegreesMin "DegreesMin"
#define KXMLQLCPhysicalLensDegreesMax "DegreesMax"

#define KXMLQLCPhysicalFocus "Focus"
#define KXMLQLCPhysicalFocusType "Type"
#define KXMLQLCPhysicalFocusPanMax "PanMax"
#define KXMLQLCPhysicalFocusTiltMax "TiltMax"

#define KXMLQLCPhysicalTechnical "Technical"
#define KXMLQLCPhysicalTechnicalPowerConsumption "PowerConsumption"
#define KXMLQLCPhysicalTechnicalDmxConnector "DmxConnector"


/**
 * QLCPhysical represents the physical properties of a fixture (in a certain
 * mode). These properties include weight, dimensions, light source, lens,
 * movement capabilities and beam width.
 */
class QLCPhysical
{
    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    QLCPhysical();
    virtual ~QLCPhysical();

    QLCPhysical& operator=(const QLCPhysical& physical);

    /************************************************************************
     * Properties
     ************************************************************************/
public:
    void setBulbType(const QString& type);
    QString bulbType() const;

    void setBulbLumens(int lumens);
    int bulbLumens() const;

    void setBulbColourTemperature(int temp);
    int bulbColourTemperature() const;

    void setWeight(double weight);
    double weight() const;

    void setWidth(int width);
    int width() const;

    void setHeight(int height);
    int height() const;

    void setDepth(int depth);
    int depth() const;

    void setLensName(const QString& name);
    QString lensName() const;

    void setLensDegreesMin(int degrees);
    int lensDegreesMin() const;

    void setLensDegreesMax(int degrees);
    int lensDegreesMax() const;

    void setFocusType(const QString& type);
    QString focusType() const;

    void setFocusPanMax(int pan);
    int focusPanMax() const;

    void setFocusTiltMax(int tilt);
    int focusTiltMax() const;

    void setPowerConsumption(int watt);
    int powerConsumption() const;

    void setDmxConnector(const QString& type);
    QString dmxConnector() const;

protected:
    QString m_bulbType;
    int m_bulbLumens;
    int m_bulbColourTemperature;

    double m_weight;
    int m_width;
    int m_height;
    int m_depth;

    QString m_lensName;
    int m_lensDegreesMin;
    int m_lensDegreesMax;

    QString m_focusType;
    int m_focusPanMax;
    int m_focusTiltMax;

    int m_powerConsumption;
    QString m_dmxConnector;

    /************************************************************************
     * Load & Save
     ************************************************************************/
public:
    /** Load physical values from the given QDomElement */
    bool loadXML(const QDomElement& root);

    /** Save physical values to the given XML tag in the given document */
    bool saveXML(QDomDocument* doc, QDomElement* root);
};

#endif
