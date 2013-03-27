/*
  Q Light Controller
  efxfixture.cpp

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

#include <QDomDocument>
#include <QDomElement>
#include <QDebug>
#include <math.h>

#include "universearray.h"
#include "genericfader.h"
#include "mastertimer.h"
#include "efxfixture.h"
#include "qlcmacros.h"
#include "function.h"
#include "scene.h"
#include "efx.h"
#include "doc.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

EFXFixture::EFXFixture(const EFX* parent)
    : m_parent(parent)
    , m_fixture(Fixture::invalidId())
    , m_direction(Function::Forward)
    , m_startOffset(0)
    , m_fadeIntensity(255)

    , m_serialNumber(0)
    , m_runTimeDirection(Function::Forward)
    , m_ready(false)
    , m_started(false)
    , m_elapsed(0)

    , m_intensity(1.0)
{
    Q_ASSERT(parent != NULL);
}

void EFXFixture::copyFrom(const EFXFixture* ef)
{
    // Don't copy m_parent because it is already assigned in constructor and might
    // be different than $ef's
    m_fixture = ef->m_fixture;
    m_direction = ef->m_direction;
    m_startOffset = ef->m_startOffset;
    m_fadeIntensity = ef->m_fadeIntensity;

    m_serialNumber = ef->m_serialNumber;
    m_runTimeDirection = ef->m_runTimeDirection;
    m_ready = ef->m_ready;
    m_started = ef->m_started;
    m_elapsed = ef->m_elapsed;

    m_intensity = ef->m_intensity;
}

EFXFixture::~EFXFixture()
{
}

/****************************************************************************
 * Public properties
 ****************************************************************************/

void EFXFixture::setFixture(quint32 id)
{
    m_fixture = id;
}

quint32 EFXFixture::fixture() const
{
    return m_fixture;
}

void EFXFixture::setDirection(Function::Direction dir)
{
    m_direction = dir;
    m_runTimeDirection = dir;
}

Function::Direction EFXFixture::direction() const
{
    return m_direction;
}

void EFXFixture::setStartOffset(int startOffset)
{
    m_startOffset = CLAMP(startOffset, 0, 359);
}

int EFXFixture::startOffset() const
{
    return m_startOffset;
}

void EFXFixture::setFadeIntensity(uchar value)
{
    m_fadeIntensity = value;
}

uchar EFXFixture::fadeIntensity() const
{
    return m_fadeIntensity;
}

bool EFXFixture::isValid() const
{
    Fixture* fxi = doc()->fixture(fixture());
    if (fxi == NULL)
        return false;
    else if (fxi->panMsbChannel() == QLCChannel::invalid() && // Maybe a device can pan OR tilt
             fxi->tiltMsbChannel() == QLCChannel::invalid())   // but not both. Teh sux0r.
        return false;
    else
        return true;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool EFXFixture::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCEFXFixture)
    {
        qWarning("EFX Fixture node not found!");
        return false;
    }

    /* New file format contains sub tags */
    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCEFXFixtureID)
        {
            /* Fixture ID */
            setFixture(tag.text().toInt());
        }
        else if (tag.tagName() == KXMLQLCEFXFixtureDirection)
        {
            /* Direction */
            Function::Direction dir = Function::stringToDirection(tag.text());
            setDirection(dir);
        }
        else if (tag.tagName() == KXMLQLCEFXFixtureStartOffset)
        {
            /* Start offset */
            setStartOffset(int(tag.text().toInt()));
        }
        else if (tag.tagName() == KXMLQLCEFXFixtureIntensity)
        {
            /* Intensity */
            setFadeIntensity(uchar(tag.text().toUInt()));
        }
        else
        {
            qWarning() << "Unknown EFX Fixture tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool EFXFixture::saveXML(QDomDocument* doc, QDomElement* efx_root) const
{
    QDomElement subtag;
    QDomElement tag;
    QDomText text;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(efx_root != NULL);

    /* EFXFixture */
    tag = doc->createElement(KXMLQLCEFXFixture);
    efx_root->appendChild(tag);

    /* Fixture ID */
    subtag = doc->createElement(KXMLQLCEFXFixtureID);
    tag.appendChild(subtag);
    text = doc->createTextNode(QString("%1").arg(fixture()));
    subtag.appendChild(text);

    /* Direction */
    subtag = doc->createElement(KXMLQLCEFXFixtureDirection);
    tag.appendChild(subtag);
    text = doc->createTextNode(Function::directionToString(m_direction));
    subtag.appendChild(text);

    /* Start offset */
    subtag = doc->createElement(KXMLQLCEFXFixtureStartOffset);
    tag.appendChild(subtag);
    text = doc->createTextNode(QString::number(startOffset()));
    subtag.appendChild(text);

    /* Intensity */
    subtag = doc->createElement(KXMLQLCEFXFixtureIntensity);
    tag.appendChild(subtag);
    text = doc->createTextNode(QString::number(fadeIntensity()));
    subtag.appendChild(text);

    return true;
}

/****************************************************************************
 * Run-time properties
 ****************************************************************************/

const Doc* EFXFixture::doc() const
{
    Q_ASSERT(m_parent != NULL);
    return m_parent->doc();
}

void EFXFixture::setSerialNumber(int number)
{
    m_serialNumber = number;
}

int EFXFixture::serialNumber() const
{
    return m_serialNumber;
}

void EFXFixture::reset()
{
    m_ready = false;
    m_runTimeDirection = m_direction;
    m_started = false;
    m_elapsed = 0;
}

bool EFXFixture::isReady() const
{
    return m_ready;
}

uint EFXFixture::timeOffset() const
{
    if (m_parent->propagationMode() == EFX::Asymmetric ||
        m_parent->propagationMode() == EFX::Serial)
    {
        return m_parent->duration() / (m_parent->fixtures().size() + 1) * serialNumber();
    }
    else
    {
        return 0;
    }
}

/*****************************************************************************
 * Running
 *****************************************************************************/

void EFXFixture::nextStep(MasterTimer* timer, UniverseArray* universes)
{
    m_elapsed += MasterTimer::tick();

    // Bail out without doing anything if this fixture is ready (after single-shot)
    // or it has no pan&tilt channels (not valid).
    if (m_ready == true || isValid() == false)
        return;

    // Bail out without doing anything if this fixture is waiting for its turn.
    if (m_parent->propagationMode() == EFX::Serial && m_elapsed < timeOffset() && !m_started)
        return;

    // Fade in
    if (m_started == false)
        start(timer, universes);

    // Nothing to do
    if (m_parent->duration() == 0)
        return;

    // Scale from elapsed time in relation to overall duration to a point in a circle
    uint pos = (m_elapsed + timeOffset()) % m_parent->duration();
    qreal iterator = SCALE(qreal(pos),
                           qreal(0), qreal(m_parent->duration()),
                           qreal(0), qreal(M_PI * 2));

    qreal pan = 0;
    qreal tilt = 0;

    if ((m_parent->propagationMode() == EFX::Serial &&
        m_elapsed < (m_parent->duration() + timeOffset()))
        || m_elapsed < m_parent->duration())
    {
        m_parent->calculatePoint(m_runTimeDirection, m_startOffset, iterator, &pan, &tilt);

        /* Write this fixture's data to universes. */
        setPoint(universes, pan, tilt);
    }
    else
    {
        if (m_parent->runOrder() == Function::PingPong)
        {
            /* Reverse direction for ping-pong EFX. */
            if (m_runTimeDirection == Function::Forward)
                m_runTimeDirection = Function::Backward;
            else
                m_runTimeDirection = Function::Forward;
        }
        else if (m_parent->runOrder() == Function::SingleShot)
        {
            /* De-initialize the fixture and mark as ready. */
            m_ready = true;
            stop(timer, universes);
        }

        m_elapsed = 0;
    }
}

void EFXFixture::setPoint(UniverseArray* universes, qreal pan, qreal tilt)
{
    Q_ASSERT(universes != NULL);

    Fixture* fxi = doc()->fixture(fixture());
    Q_ASSERT(fxi != NULL);

    /* Write coarse point data to universes */
    if (fxi->panMsbChannel() != QLCChannel::invalid())
        universes->write(fxi->universeAddress() + fxi->panMsbChannel(),
                         static_cast<char>(pan), QLCChannel::Pan);
    if (fxi->tiltMsbChannel() != QLCChannel::invalid())
        universes->write(fxi->universeAddress() + fxi->tiltMsbChannel(),
                         static_cast<char> (tilt), QLCChannel::Tilt);

    /* Write fine point data to universes if applicable */
    if (fxi->panLsbChannel() != QLCChannel::invalid())
    {
        /* Leave only the fraction */
        char value = static_cast<char> ((pan - floor(pan)) * double(UCHAR_MAX));
        universes->write(fxi->universeAddress() + fxi->panLsbChannel(), value, QLCChannel::Pan);
    }

    if (fxi->tiltLsbChannel() != QLCChannel::invalid())
    {
        /* Leave only the fraction */
        char value = static_cast<char> ((tilt - floor(tilt)) * double(UCHAR_MAX));
        universes->write(fxi->universeAddress() + fxi->tiltLsbChannel(), value, QLCChannel::Tilt);
    }
}

void EFXFixture::start(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(universes);
    Q_UNUSED(timer);

    if (fadeIntensity() > 0 && m_started == false)
    {
        Fixture* fxi = doc()->fixture(fixture());
        Q_ASSERT(fxi != NULL);

        if (fxi->masterIntensityChannel() != QLCChannel::invalid())
        {
            FadeChannel fc;
            fc.setFixture(fixture());
            fc.setChannel(fxi->masterIntensityChannel());
            if (m_parent->overrideFadeInSpeed() != Function::defaultSpeed())
                fc.setFadeTime(m_parent->overrideFadeInSpeed());
            else
                fc.setFadeTime(m_parent->fadeInSpeed());

            fc.setStart(0);
            fc.setCurrent(fc.start());
            // Don't use intensity() multiplier because EFX's GenericFader takes care of that
            fc.setTarget(fadeIntensity());
            // Fade channel up with EFX's own GenericFader to allow manual intensity control
            m_parent->m_fader->add(fc);
        }
    }

    m_started = true;
}

void EFXFixture::stop(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(universes);

    if (fadeIntensity() > 0 && m_started == true)
    {
        Fixture* fxi = doc()->fixture(fixture());
        Q_ASSERT(fxi != NULL);

        if (fxi->masterIntensityChannel() != QLCChannel::invalid())
        {
            FadeChannel fc;
            fc.setFixture(fixture());
            fc.setChannel(fxi->masterIntensityChannel());

            if (m_parent->overrideFadeOutSpeed() != Function::defaultSpeed())
                fc.setFadeTime(m_parent->overrideFadeOutSpeed());
            else
                fc.setFadeTime(m_parent->fadeOutSpeed());

            fc.setStart(uchar(floor((qreal(fadeIntensity()) * intensity()) + 0.5)));
            fc.setCurrent(fc.start());
            fc.setTarget(0);
            // Give zero-fading to MasterTimer because EFX will stop after this call
            timer->fader()->add(fc);
            // Remove the previously up-faded channel from EFX's internal fader to allow
            // MasterTimer's fader take HTP precedence.
            m_parent->m_fader->remove(fc);
        }
    }

    m_started = false;
}

/*****************************************************************************
 * Intensity
 *****************************************************************************/

void EFXFixture::adjustIntensity(qreal fraction)
{
    m_intensity = fraction;
}

qreal EFXFixture::intensity() const
{
    return m_intensity;
}
