/*
  Q Light Controller
  vcxypadfixture.h

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

#ifndef VCXYPADFIXTURE
#define VCXYPADFIXTURE

#include <QStringList>
#include <QVariant>
#include <QString>

#include "grouphead.h"

class VCXYPadFixture;
class UniverseArray;
class QDomDocument;
class QDomElement;
class Doc;

#define KXMLQLCVCXYPadFixture "Fixture"
#define KXMLQLCVCXYPadFixtureID "ID"
#define KXMLQLCVCXYPadFixtureHead "Head"

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
     * Fixture Head
     ********************************************************************/
public:
    void setHead(GroupHead const & head);
    GroupHead const & head() const;

    QString name() const;

private:
    GroupHead m_head;

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
