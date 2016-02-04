/*
  Q Light Controller Plus
  efxfixture.h

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

#ifndef EFXFIXTURE_H
#define EFXFIXTURE_H

#include <QImage>
#include "function.h"
#include "grouphead.h"

class MasterTimer;
class FadeChannel;
class EFXFixture;
class Scene;
class EFX;
class Doc;
class QImage;

/** @addtogroup engine_functions Functions
 * @{
 */

#define KXMLQLCEFXFixture "Fixture"
#define KXMLQLCEFXFixtureID "ID"
#define KXMLQLCEFXFixtureHead "Head"
#define KXMLQLCEFXFixtureMode "Mode"
#define KXMLQLCEFXFixtureDirection "Direction"
#define KXMLQLCEFXFixtureStartOffset "StartOffset"
#define KXMLQLCEFXFixtureIntensity "Intensity"

#define KXMLQLCEFXFixtureModePanTilt "Position"
#define KXMLQLCEFXFixtureModeDimmer "Dimmer"
#define KXMLQLCEFXFixtureModeRGB "RGB"

class EFXFixture
{
    friend class EFX;

public:
    enum Mode
    {
        PanTilt,
        Dimmer,
        RGB
    };

    /*************************************************************************
     * Initialization
     *************************************************************************/
public:
    /** Constructor */
    EFXFixture(const EFX* parent);

    /** Destructor */
    ~EFXFixture();

    /** Copy contents from another EFXFixture */
    void copyFrom(const EFXFixture* ef);

private:
    /** The EFX function that this fixture belongs to. */
    const EFX* const m_parent;

    /*************************************************************************
     * Public properties
     *************************************************************************/
public:
    /** Set the Fixture Head that this EFXFixture represents. */
    void setHead(GroupHead const & head);

    /** Get the Fixture Head that this EFXFixture represents. */
    GroupHead const & head() const;

    /** Set this fixture's initial direction. */
    void setDirection(Function::Direction dir);

    /** Get this fixture's initial direction. */
    Function::Direction direction() const;

    /** Set this fixture's start offset */
    void setStartOffset(int startOffset);

    /** Get this fixture's start offset */
    int startOffset() const;

    /** Set the parameter(s) that this efx will animate (ie. dimmer, RGB, ...) */
    void setMode(Mode mode);

    /** Get the parameter(s) that this efx will animate (ie. dimmer, RGB, ...) */
    Mode mode() const;

    /**
     * Check that this object has a fixture ID and at least LSB channel
     * for pan and/or tilt.
     */
    bool isValid() const;

    void durationChanged();

public:
    /** Get the supported mode for this fixture in a string list */
    QStringList modeList();

    /** Convert a mode to a string */
    static QString modeToString(Mode algo);

    /** Convert a string to an mode type */
    static Mode stringToMode(const QString& str);
 
private:
    GroupHead m_head;
    Function::Direction m_direction;
    int m_startOffset;
    Mode m_mode;

    /*************************************************************************
     * Load & Save
     *************************************************************************/
public:
    bool loadXML(QXmlStreamReader &root);
    bool saveXML(QXmlStreamWriter *doc) const;

    /*************************************************************************
     * Run-time properties
     *************************************************************************/
private:
    /** Get the master engine instance */
    const Doc* doc() const;

    /** Set the order number in serial propagation mode */
    void setSerialNumber(int number);

    /** Get the order number in serial propagation mode */
    int serialNumber() const;

    /** Reset the fixture when the EFX is stopped */
    void reset();

    /** Check, whether this EFXFixture is ready (no more events).
        This can happen basically only if SingleShot mode is enabled. */
    bool isReady() const;

    /** Get this fixture's time offset (in serial and asymmetric modes) */
    uint timeOffset() const;

private:
    /** This fixture's order number in serial propagation mode */
    int m_serialNumber;

    /** This fixture's current run-time direction */
    Function::Direction m_runTimeDirection;

    /** When running in single shot mode, the fixture is marked ready
        after it has completed a full cycle. */
    bool m_ready;

    /** Indicates, whether start() has been called for this fixture */
    bool m_started;

    /** Elapsed milliseconds since last reset() */
    uint m_elapsed;

    /** 0..M_PI*2, current position, recomputed on each timer tick; depends on elapsed() and parent->duration() */
    float m_currentAngle;

    /*************************************************************************
     * Running
     *************************************************************************/
private:
    /** Calculate the next step data for this fixture */
    void nextStep(MasterTimer* timer, QList<Universe *> universes);

    /** Write this EFXFixture's channel data to universes */
    void setPointPanTilt(QList<Universe *> universes, float pan, float tilt);
    void setPointDimmer(QList<Universe *> universes, float dimmer);
    void setPointRGB (QList<Universe *> universes, float x, float y);

    /* Run the start scene if necessary */
    void start(MasterTimer* timer, QList<Universe *> universes);

    /* Run the stop scene if necessary */
    void stop(MasterTimer* timer, QList<Universe *> universes);

private:
    static QImage m_rgbGradient;

    void setFadeChannel(quint32 nChannel, uchar val);
};

/** @} */

#endif
