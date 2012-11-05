/*
  Q Light Controller
  vcxypadfixture.h

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

#ifndef VCXYPADFIXTURE
#define VCXYPADFIXTURE

#include <QStringList>
#include <QVariant>
#include <QString>

class VCXYPadFixture;
class UniverseArray;
class QDomDocument;
class QDomElement;
class Doc;

#define KXMLQLCVCXYPadFixture "Fixture"
#define KXMLQLCVCXYPadFixtureID "ID"

#define KXMLQLCVCXYPadFixtureAxis "Axis"
#define KXMLQLCVCXYPadFixtureAxisID "ID"
#define KXMLQLCVCXYPadFixtureAxisX "X"
#define KXMLQLCVCXYPadFixtureAxisY "Y"
#define KXMLQLCVCXYPadFixtureAxisLowLimit "LowLimit"
#define KXMLQLCVCXYPadFixtureAxisHighLimit "HighLimit"
#define KXMLQLCVCXYPadFixtureAxisReverse "Reverse"

class VCXYPadFixture
{
    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    VCXYPadFixture(Doc* doc);
    VCXYPadFixture(Doc* doc, const QVariant& variant);
    ~VCXYPadFixture();

    /** Assignment operator */
    VCXYPadFixture& operator=(const VCXYPadFixture& fxi);

    /** Comparing operator */
    bool operator==(const VCXYPadFixture& fxi) const;

    /* Serialization operator for VCXYPadFixtureEditor */
    operator QVariant() const;

private:
    Doc* m_doc;

    /********************************************************************
     * Fixture
     ********************************************************************/
public:
    void setFixture(quint32 fxi_id);
    quint32 fixture() const;

    QString name() const;

private:
    quint32 m_fixture;

    /********************************************************************
     * X-Axis
     ********************************************************************/
public:
    void setX(qreal min, qreal max, bool reverse);
    qreal xMin() const;
    qreal xMax() const;
    bool xReverse() const;

    /** min% - max% for displaying X limits in tree widget */
    QString xBrief() const;

private:
    qreal m_xMin;
    qreal m_xMax;
    bool m_xReverse;

    quint32 m_xLSB;
    quint32 m_xMSB;

    /********************************************************************
     * Y-Axis
     ********************************************************************/
public:
    void setY(qreal min, qreal max, bool reverse);
    qreal yMin() const;
    qreal yMax() const;
    bool yReverse() const;

    /** min% - max% for displaying Y limits in tree widget */
    QString yBrief() const;

private:
    qreal m_yMin;
    qreal m_yMax;
    bool m_yReverse;

    quint32 m_yLSB;
    quint32 m_yMSB;

    /********************************************************************
     * Load & Save
     ********************************************************************/
public:
    bool loadXML(const QDomElement& root);
    bool saveXML(QDomDocument* doc, QDomElement* root) const;

    /********************************************************************
     * Running
     ********************************************************************/
public:
    void arm();
    void disarm();

    /** Write the value using x & y multipliers for the actual range */
    void writeDMX(qreal xmul, qreal ymul, UniverseArray* universes);
};

#endif
