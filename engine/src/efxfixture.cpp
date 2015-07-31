/*
  Q Light Controller
  efxfixture.cpp

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

#include <QDomDocument>
#include <QDomElement>
#include <QDebug>
#include <math.h>
#include <QImage>
#include <QLinearGradient>
#include <QPainter>
#include <QRgb>

#include "genericfader.h"
#include "mastertimer.h"
#include "efxfixture.h"
#include "qlcmacros.h"
#include "function.h"
#include "universe.h"
#include "scene.h"
#include "efx.h"
#include "doc.h"
#include "gradient.h"


/*****************************************************************************
 * Initialization
 *****************************************************************************/

EFXFixture::EFXFixture(const EFX* parent)
    : m_parent(parent)
    , m_head()
    , m_direction(Function::Forward)
    , m_startOffset(0)
    , m_mode(EFXFixture::PanTilt)
    , m_fadeIntensity(255)

    , m_serialNumber(0)
    , m_runTimeDirection(Function::Forward)
    , m_ready(false)
    , m_started(false)
    , m_elapsed(0)
    , m_currentAngle(0)

    , m_intensity(1.0)
{
    Q_ASSERT(parent != NULL);
}

void EFXFixture::copyFrom(const EFXFixture* ef)
{
    // Don't copy m_parent because it is already assigned in constructor and might
    // be different than $ef's
    m_head = ef->m_head;
    m_direction = ef->m_direction;
    m_startOffset = ef->m_startOffset;
    m_mode = ef->m_mode;
    m_fadeIntensity = ef->m_fadeIntensity;

    m_serialNumber = ef->m_serialNumber;
    m_runTimeDirection = ef->m_runTimeDirection;
    m_ready = ef->m_ready;
    m_started = ef->m_started;
    m_elapsed = ef->m_elapsed;
    m_currentAngle = ef->m_currentAngle;

    m_intensity = ef->m_intensity;
}

EFXFixture::~EFXFixture()
{
}

/****************************************************************************
 * Public properties
 ****************************************************************************/

void EFXFixture::setHead(GroupHead const & head)
{
    m_head = head;
}

GroupHead const & EFXFixture::head() const
{
    return m_head;
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

void EFXFixture::setMode(Mode mode)
{
    m_mode = mode;
}

EFXFixture::Mode EFXFixture::mode() const
{
    return m_mode;
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
    Fixture* fxi = doc()->fixture(head().fxi);

    if (fxi == NULL)
        return false;
    else if (head().head >= fxi->heads())
        return false;
    else if (m_mode == PanTilt && fxi->panMsbChannel(head().head) == QLCChannel::invalid() && // Maybe a device can pan OR tilt
             fxi->tiltMsbChannel(head().head) == QLCChannel::invalid())   // but not both. Teh sux0r.
        return false;
    else if (m_mode == Dimmer && fxi->masterIntensityChannel(head().head) == QLCChannel::invalid() )
        return false;
    else if (m_mode == RGB && fxi->rgbChannels(head().head).size () == 0)
        return false;
    else
        return true;
}

void EFXFixture::durationChanged()
{
    // To avoid jumps when changing duration,
    // the elapsed time is rescaled to the
    // new duration.
    m_elapsed = SCALE(qreal(m_currentAngle),
            qreal(0), qreal(M_PI * 2),
            qreal(0), qreal(m_parent->duration()));

    // Serial or Asymmetric propagation mode:
    // we must substract the offset from the current position
    if (timeOffset())
    {
        if (m_elapsed < timeOffset())
            m_elapsed += m_parent->duration();
        m_elapsed -= timeOffset();
    }
}

QStringList EFXFixture::modeList()
{
    Fixture* fxi = doc()->fixture(head().fxi);
    Q_ASSERT(fxi != NULL);

    QStringList modes;

    if((fxi->panMsbChannel(head().head) != QLCChannel::invalid()) &&
            (fxi->tiltMsbChannel(head().head) != QLCChannel::invalid()) )
        modes << KXMLQLCEFXFixtureModePanTilt;

    if((fxi->masterIntensityChannel(head().head) != QLCChannel::invalid()))
        modes << KXMLQLCEFXFixtureModeDimmer;

    if((fxi->rgbChannels (head().head).size () >= 3))
        modes << KXMLQLCEFXFixtureModeRGB;

    return modes;
}

QString EFXFixture::modeToString(Mode mode)
{
    switch (mode)
    {
        default:
        case PanTilt:
            return QString(KXMLQLCEFXFixtureModePanTilt);
        case Dimmer:
            return QString(KXMLQLCEFXFixtureModeDimmer);
        case RGB:
            return QString(KXMLQLCEFXFixtureModeRGB);
    }
}

EFXFixture::Mode EFXFixture::stringToMode(const QString& str)
{
    if (str == QString(KXMLQLCEFXFixtureModePanTilt))
        return PanTilt;
    else if (str == QString(KXMLQLCEFXFixtureModeDimmer))
        return Dimmer;
    else if (str == QString(KXMLQLCEFXFixtureModeRGB))
        return RGB;
    else
        return PanTilt;
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

    GroupHead head;
    head.head = 0;

    /* New file format contains sub tags */
    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCEFXFixtureID)
        {
            /* Fixture ID */
            head.fxi = tag.text().toInt();
        }
        else if (tag.tagName() == KXMLQLCEFXFixtureHead)
        {
            /* Fixture Head */
            head.head = tag.text().toInt();
        }
        else if (tag.tagName() == KXMLQLCEFXFixtureMode)
        {
            /* Fixture Mode */
            setMode ((Mode) tag.text().toInt());
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
            setStartOffset(tag.text().toInt());
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

    if (head.fxi != Fixture::invalidId())
       setHead(head);

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
    text = doc->createTextNode(QString("%1").arg(head().fxi));
    subtag.appendChild(text);

    /* Fixture Head */
    subtag = doc->createElement(KXMLQLCEFXFixtureHead);
    tag.appendChild(subtag);
    text = doc->createTextNode(QString("%1").arg(head().head));
    subtag.appendChild(text);

    /* Mode */
    subtag = doc->createElement(KXMLQLCEFXFixtureMode);
    tag.appendChild(subtag);
    text = doc->createTextNode(QString::number(mode ()));
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
    m_currentAngle = 0;
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
#include <stdio.h>
void EFXFixture::nextStep(MasterTimer* timer, QList<Universe *> universes)
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
    m_currentAngle = SCALE(qreal(pos),
                           qreal(0), qreal(m_parent->duration()),
                           qreal(0), qreal(M_PI * 2));

    qreal valX = 0;
    qreal valY = 0;

    if ((m_parent->propagationMode() == EFX::Serial &&
        m_elapsed < (m_parent->duration() + timeOffset()))
        || m_elapsed < m_parent->duration())
    {
        m_parent->calculatePoint(m_runTimeDirection, m_startOffset, m_currentAngle, &valX, &valY);

        /* Write this fixture's data to universes. */
        switch(m_mode)
        {
        case PanTilt:
            setPointPanTilt(universes, valX, valY);
            break;

        case RGB:
            setPointRGB (universes, valX, valY);
            break;

        case Dimmer:
            //Use Y for coherence with RGB gradient.
            setPointDimmer (universes, valY);
            break;
        }
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

        m_elapsed %= m_parent->duration();
    }
}

void EFXFixture::setPointPanTilt(QList<Universe *> universes, qreal pan, qreal tilt)
{
    Fixture* fxi = doc()->fixture(head().fxi);
    Q_ASSERT(fxi != NULL);

    /* Write coarse point data to universes */
    if (fxi->panMsbChannel(head().head) != QLCChannel::invalid())
    {
        if (m_parent->isRelative())
            universes[fxi->universe()]->writeRelative(fxi->address() + fxi->panMsbChannel(head().head), static_cast<char>(pan));
        else
            universes[fxi->universe()]->write(fxi->address() + fxi->panMsbChannel(head().head), static_cast<char>(pan));
    }
    if (fxi->tiltMsbChannel(head().head) != QLCChannel::invalid())
    {
        if (m_parent->isRelative())
            universes[fxi->universe()]->writeRelative(fxi->address() + fxi->tiltMsbChannel(head().head), static_cast<char> (tilt));
        else
            universes[fxi->universe()]->write(fxi->address() + fxi->tiltMsbChannel(head().head), static_cast<char> (tilt));
    }

    /* Write fine point data to universes if applicable */
    if (fxi->panLsbChannel(head().head) != QLCChannel::invalid())
    {
        /* Leave only the fraction */
        char value = static_cast<char> ((pan - floor(pan)) * double(UCHAR_MAX));
        if (m_parent->isRelative())
            universes[fxi->universe()]->writeRelative(fxi->address() + fxi->panLsbChannel(head().head), value);
        else
            universes[fxi->universe()]->write(fxi->address() + fxi->panLsbChannel(head().head), value);
    }

    if (fxi->tiltLsbChannel(head().head) != QLCChannel::invalid())
    {
        /* Leave only the fraction */
        char value = static_cast<char> ((tilt - floor(tilt)) * double(UCHAR_MAX));
        if (m_parent->isRelative())
            universes[fxi->universe()]->writeRelative(fxi->address() + fxi->tiltLsbChannel(head().head), value);
        else
            universes[fxi->universe()]->write(fxi->address() + fxi->tiltLsbChannel(head().head), value);
    }
}

void EFXFixture::setPointDimmer(QList<Universe *> universes, qreal dimmer)
{
    Q_UNUSED(universes);

    Fixture* fxi = doc()->fixture(head().fxi);
    Q_ASSERT(fxi != NULL);

    /* Don't write dimmer data directly to universes but use FadeChannel to avoid steps at EFX loop restart */
    if (fxi->masterIntensityChannel(head().head) != QLCChannel::invalid())
    {
        setFadeChannel(fxi->masterIntensityChannel(head().head),  dimmer);
    }
}

void EFXFixture::setPointRGB(QList<Universe *> universes, qreal x, qreal y)
{
    Q_UNUSED(universes);

    Fixture* fxi = doc()->fixture(head().fxi);
    Q_ASSERT(fxi != NULL);

    QVector<quint32> rgbChannels = fxi->rgbChannels(head().head);

    /* Don't write dimmer data directly to universes but use FadeChannel to avoid steps at EFX loop restart */
    if (rgbChannels.size () >= 3)
    {
        QColor pixel = Gradient::getRGBColor (x, y);

        setFadeChannel(rgbChannels[0], pixel.red ());
        setFadeChannel(rgbChannels[1], pixel.green ());
        setFadeChannel(rgbChannels[2], pixel.blue ());
    }
}

void EFXFixture::start(MasterTimer* timer, QList<Universe *> universes)
{
    Q_UNUSED(universes);
    Q_UNUSED(timer);

    if (fadeIntensity() > 0 && m_started == false)
    {
        Fixture* fxi = doc()->fixture(head().fxi);
        Q_ASSERT(fxi != NULL);

        if (fxi->masterIntensityChannel(head().head) != QLCChannel::invalid())
        {
            FadeChannel fc;
            fc.setFixture(doc(), head().fxi);
            fc.setChannel(fxi->masterIntensityChannel(head().head));
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

void EFXFixture::stop(MasterTimer* timer, QList<Universe *> universes)
{
    Q_UNUSED(universes);

    if (fadeIntensity() > 0 && m_started == true)
    {
        Fixture* fxi = doc()->fixture(head().fxi);
        Q_ASSERT(fxi != NULL);

        if (fxi->masterIntensityChannel(head().head) != QLCChannel::invalid())
        {
            FadeChannel fc;
            fc.setFixture(doc(), head().fxi);
            fc.setChannel(fxi->masterIntensityChannel(head().head));

            if (m_parent->overrideFadeOutSpeed() != Function::defaultSpeed())
                fc.setFadeTime(m_parent->overrideFadeOutSpeed());
            else
                fc.setFadeTime(m_parent->fadeOutSpeed());

            fc.setStart(uchar(floor((qreal(fadeIntensity()) * intensity()) + 0.5)));
            fc.setCurrent(fc.start());
            fc.setTarget(0);
            // Give zero-fading to MasterTimer because EFX will stop after this call
            timer->faderAdd(fc);
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


/*****************************************************************************
 * Helper Function
 *****************************************************************************/
void EFXFixture::setFadeChannel(quint32 nChannel, uchar val)
{
    FadeChannel fc;
    fc.setFixture(doc(), head().fxi);
    fc.setChannel(nChannel);

    fc.setTarget(val);
    m_parent->m_fader->forceAdd(fc);
}
