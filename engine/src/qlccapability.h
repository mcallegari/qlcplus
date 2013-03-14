/*
  Q Light Controller
  qlccapability.h

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

#ifndef QLCCAPABILITY_H
#define QLCCAPABILITY_H

#include <climits>
#include <QColor>
#include <QList>

#define KXMLQLCCapability    "Capability"
#define KXMLQLCCapabilityMin "Min"
#define KXMLQLCCapabilityMax "Max"
#define KXMLQLCCapabilityResource "Res"
#define KXMLQLCCapabilityColor1 "Color"
#define KXMLQLCCapabilityColor2 "Color2"

class QLCCapability;
class QDomDocument;
class QDomElement;
class QString;
class QFile;

/**
 * QLCCapability represents one value range with a special meaning in a
 * QLCChannel. For example, a sunburst gobo might be set on a "gobo" channel
 * with any DMX value between 15 and 25. This is represented as a
 * QLCCapability, whose min == 15, max == 25 and name == "Sunburst". Single
 * values can be represented by setting the same value to both, for example:
 * min == 15 and max == 15.
 */
class QLCCapability
{
    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    /** Default constructor */
    QLCCapability(uchar min = 0, uchar max = UCHAR_MAX,
                  const QString& name = QString(), const QString& resource = QString(),
                  const QColor &color1 = QColor(), const QColor &color2 = QColor());

    /** Copy constructor */
    QLCCapability(const QLCCapability* cap);

    /** Destructor */
    ~QLCCapability();

    /** Assignment operator */
    QLCCapability& operator=(const QLCCapability& capability);

    /** Comparing operator for qSort */
    bool operator<(const QLCCapability& capability) const;

    /********************************************************************
     * Properties
     ********************************************************************/
public:
    uchar min() const;
    void setMin(uchar value);

    uchar max() const;
    void setMax(uchar value);

    uchar middle() const;

    QString name() const;
    void setName(const QString& name);

    QString resourceName();
    void setResourceName(const QString& name);

    QColor resourceColor1();
    QColor resourceColor2();
    void setResourceColors(QColor col1, QColor col2);

    /** Check, whether the given capability overlaps with this */
    bool overlaps(const QLCCapability& cap);

protected:
    uchar m_min;
    uchar m_max;
    QString m_name;
    QString m_resourceName;
    QColor m_resourceColor1;
    QColor m_resourceColor2;

    /********************************************************************
     * Load & Save
     ********************************************************************/
public:
    /** Save the capability to a QDomDocument, under the given element */
    bool saveXML(QDomDocument* doc, QDomElement* root);

    /** Load capability contents from an XML element */
    bool loadXML(const QDomElement& root);
};

#endif
