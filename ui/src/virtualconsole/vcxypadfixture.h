/*
  Q Light Controller Plus
  vcxypadfixture.h

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

#ifndef VCXYPADFIXTURE
#define VCXYPADFIXTURE

#include <QStringList>
#include <QVariant>
#include <QString>

#include "grouphead.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class VCXYPadFixture;
class GenericFader;
class FadeChannel;
class Universe;
class Doc;

/** @addtogroup ui_vc_widgets
 * @{
 */

#define KXMLQLCVCXYPadFixture       QString("Fixture")
#define KXMLQLCVCXYPadFixtureID     QString("ID")
#define KXMLQLCVCXYPadFixtureHead   QString("Head")

#define KXMLQLCVCXYPadFixtureAxis           QString("Axis")
#define KXMLQLCVCXYPadFixtureAxisID         QString("ID")
#define KXMLQLCVCXYPadFixtureAxisX          QString("X")
#define KXMLQLCVCXYPadFixtureAxisY          QString("Y")
#define KXMLQLCVCXYPadFixtureAxisLowLimit   QString("LowLimit")
#define KXMLQLCVCXYPadFixtureAxisHighLimit  QString("HighLimit")
#define KXMLQLCVCXYPadFixtureAxisReverse    QString("Reverse")

/** This class manages one fixture head in a VCXYPad */
class VCXYPadFixture
{
    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    VCXYPadFixture(Doc *doc);

    /** Initialize from QVariant */
    VCXYPadFixture(Doc *doc, const QVariant& variant);
    VCXYPadFixture(const VCXYPadFixture &other);
    ~VCXYPadFixture();

    /** Assignment operator */
    VCXYPadFixture& operator=(const VCXYPadFixture& fxi);

    /** Comparing operator */
    bool operator==(const VCXYPadFixture& fxi) const;

    /* Serialization operator for VCXYPadFixtureEditor */
    operator QVariant() const;

private:
    Doc *m_doc;

    /********************************************************************
     * Fixture Head
     ********************************************************************/
public:
    void setHead(GroupHead const & head);
    GroupHead const & head() const;

    QString name() const;

    /** Return this head's range of movement in degrees (taken from fixture definition) */
    QRectF degreesRange() const;

private:
    GroupHead m_head;

    /********************************************************************
     * X-Axis
     ********************************************************************/
public:
    /** Set pan range and pan reverse
     *
     *   \param min <0.0; 1.0>
     *   \param max <0.0; 1.0>
     *   \param reverse
     */
    void setX(qreal min, qreal max, bool reverse);
    qreal xMin() const;
    qreal xMax() const;
    bool xReverse() const;

    /** min% - max% for displaying X limits in tree widget */
    QString xBrief() const;

private:

    /** Precompute m_*Offset and m_*Range to speed up actual writing
     *
     *  Another goal is to simplify formulas in writeDMX and readDMX
     */
    void precompute();

private:
    qreal m_xMin; //!< start of pan range; 0.0 <= m_xMin <= 1.0; default: 0.0
    qreal m_xMax; //!< end of pan range; 0.0 <= m_xMax <= 1.0; default: 1.0
    bool m_xReverse; //!< pan reverse; default: false

    quint32 m_xLSB; //!< fine pan channel (relative address)
    quint32 m_xMSB; //!< coarse pan channel (relative address)
    qreal m_xOffset; //!< precomputed value for writeDMX/readDMX
    qreal m_xRange; //!< precomputed value for writeDMX/readDMX

    /********************************************************************
     * Y-Axis
     ********************************************************************/
public:

    /** Set tilt range and tilt reverse
     *
     *   \param min <0.0; 1.0>
     *   \param max <0.0; 1.0>
     *   \param reverse
     */
    void setY(qreal min, qreal max, bool reverse);
    qreal yMin() const;
    qreal yMax() const;
    bool yReverse() const;

    /** min% - max% for displaying Y limits in tree widget */
    QString yBrief() const;

private:
    qreal m_yMin; //!< start of tilt range; 0.0 <= m_yMin <= 1.0; default: 0.0
    qreal m_yMax; //!< end of tilt range; 0.0 <= m_yMax <= 1.0; default: 1.0
    bool m_yReverse; //!< tilt reverse; default: false

    quint32 m_yLSB; //!< fine tilt channel (relative address)
    quint32 m_yMSB; //!< coarse tilt channel (relative address)
    qreal m_yOffset; //!< precomputed value for writeDMX/readDMX
    qreal m_yRange; //!< precomputed value for writeDMX/readDMX

    /********************************************************************
     * Display mode
     ********************************************************************/
public:
    enum DisplayMode
    {
        Percentage = 0,
        Degrees,
        DMX
    };

    void setDisplayMode(DisplayMode mode);
    DisplayMode displayMode() const;

private:
    DisplayMode m_displayMode;

    /********************************************************************
     * Load & Save
     ********************************************************************/
public:
    bool loadXML(QXmlStreamReader &root);
    bool saveXML(QXmlStreamWriter *doc) const;

    /********************************************************************
     * Running
     ********************************************************************/
public:
    /** Prepare for writing/reading - computes target channels */
    void arm();

    /** Drop information from arm() */
    void disarm();

    void setEnabled(bool enable);
    bool isEnabled() const;

    quint32 universe() const;

    /** Write the value using x & y multipliers for the actual range
     *
     *  \param xmul <0.0;1.0> - pan value scaled to range set by setX
     *      (0.0 => min, 1.0 => max, or vice versa if the range is reversed)
     *
     *  \param ymul <0.0;1.0> - tilt value scaled to range set by setY
     *      (0.0 => min, 1.0 => max, or vice versa if the range is reversed)
     *  \param universes universes where the values are written
     */
    void writeDMX(qreal xmul, qreal ymul, QSharedPointer<GenericFader> fader, Universe *universe);

    /** Read position from the current universe
     *  \param universeData universe values where this fixture is present
     *  \param xmul <0.0;1.0> - pan value in the range set by setX
     *      (min => 0.0, max => 1.0, or vice versa if the range is reversed)
     *  \param ymul <0.0;1.0> - tilt value in the range set by setY
     *      (min => 0.0, max => 1.0, or vice versa if the range is reversed)
     */
    void readDMX(const QByteArray &universeData, qreal & xmul, qreal & ymul);

private:
    void updateChannel(FadeChannel *fc, uchar value);

private:
    /** Flag to enable/disable this fixture at runtime */
    bool m_enabled;
    quint32 m_universe;
    quint32 m_fixtureAddress;
};

/** @} */

#endif
