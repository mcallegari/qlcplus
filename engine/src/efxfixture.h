/*
  Q Light Controller
  efxfixture.h

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

#ifndef EFXFIXTURE_H
#define EFXFIXTURE_H

#include "function.h"
#include "grouphead.h"

class MasterTimer;
class FadeChannel;
class EFXFixture;
class Scene;
class EFX;
class Doc;

#define KXMLQLCEFXFixture "Fixture"
#define KXMLQLCEFXFixtureID "ID"
#define KXMLQLCEFXFixtureHead "Head"
#define KXMLQLCEFXFixtureDirection "Direction"
#define KXMLQLCEFXFixtureStartOffset "StartOffset"
#define KXMLQLCEFXFixtureIntensity "Intensity"

class EFXFixture
{
    friend class EFX;

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

    /**
     * Set a value to fade the fixture's intensity channel(s) to
     * during start().
     */
    void setFadeIntensity(uchar value);

    /**
     * Get the value to fade the fixture's intensity channel(s) to
     * during start().
     */
    uchar fadeIntensity() const;

    /**
     * Check that this object has a fixture ID and at least LSB channel
     * for pan and/or tilt.
     */
    bool isValid() const;

private:
    GroupHead m_head;
    Function::Direction m_direction;
    int m_startOffset;
    uchar m_fadeIntensity;

    /*************************************************************************
     * Load & Save
     *************************************************************************/
public:
    bool loadXML(const QDomElement& root);
    bool saveXML(QDomDocument* doc, QDomElement* efx_root) const;

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

    /*************************************************************************
     * Running
     *************************************************************************/
private:
    /** Calculate the next step data for this fixture */
    void nextStep(MasterTimer* timer, UniverseArray* universes);

    /** Write this EFXFixture's channel data to universes */
    void setPoint(UniverseArray* universes, qreal pan, qreal tilt);

    /* Run the start scene if necessary */
    void start(MasterTimer* timer, UniverseArray* universes);

    /* Run the stop scene if necessary */
    void stop(MasterTimer* timer, UniverseArray* universes);

    /*************************************************************************
     * Intensity adjustment
     *************************************************************************/
public:
    /**
     * Adjust the intensity of the fixture by a fraction.
     *
     * @param fraction Intensity fraction 0.0 - 1.0
     */
    void adjustIntensity(qreal fraction);

    /**
     * Get the adjusted intensity percentage
     *
     * @return Intensity 0.0 - 1.0
     */
    qreal intensity() const;

private:
    qreal m_intensity;
};

#endif
