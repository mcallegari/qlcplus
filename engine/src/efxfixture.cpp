/*
  Q Light Controller Plus
  efxfixture.cpp

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>
#include <math.h>

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
QImage EFXFixture::m_rgbGradient = QImage();

EFXFixture::EFXFixture(const EFX* parent)
    : m_parent(parent)
    , m_head()
    , m_direction(Function::Forward)
    , m_startOffset(0)
    , m_mode(EFXFixture::PanTilt)

    , m_serialNumber(0)
    , m_runTimeDirection(Function::Forward)
    , m_ready(false)
    , m_started(false)
    , m_elapsed(0)
    , m_currentAngle(0)
{
    Q_ASSERT(parent != NULL);

    if(m_rgbGradient.isNull ())
        m_rgbGradient = Gradient::getRGBGradient (256, 256);
}

void EFXFixture::copyFrom(const EFXFixture* ef)
{
    // Don't copy m_parent because it is already assigned in constructor and might
    // be different than $ef's
    m_head = ef->m_head;
    m_direction = ef->m_direction;
    m_startOffset = ef->m_startOffset;
    m_mode = ef->m_mode;

    m_serialNumber = ef->m_serialNumber;
    m_runTimeDirection = ef->m_runTimeDirection;
    m_ready = ef->m_ready;
    m_started = ef->m_started;
    m_elapsed = ef->m_elapsed;
    m_currentAngle = ef->m_currentAngle;
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

    Fixture* fxi = doc()->fixture(head.fxi);
    if (fxi == NULL)
        return;

    QList<Mode> modes;

    if((fxi->panMsbChannel(head.head) != QLCChannel::invalid()) ||
            (fxi->tiltMsbChannel(head.head) != QLCChannel::invalid()))
        modes << PanTilt;

    if((fxi->masterIntensityChannel(head.head) != QLCChannel::invalid()))
        modes << Dimmer;

    if((fxi->rgbChannels (head.head).size () >= 3))
        modes << RGB;

    if (!modes.contains(m_mode))
    {
        if (modes.size() > 0)
            m_mode = modes[0];
    }
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
    m_elapsed = SCALE(float(m_currentAngle),
            float(0), float(M_PI * 2),
            float(0), float(m_parent->duration()));

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

    if((fxi->panMsbChannel(head().head) != QLCChannel::invalid()) ||
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

bool EFXFixture::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCEFXFixture)
    {
        qWarning("EFX Fixture node not found!");
        return false;
    }

    GroupHead head;
    head.head = 0;

    /* New file format contains sub tags */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCEFXFixtureID)
        {
            /* Fixture ID */
            head.fxi = root.readElementText().toInt();
        }
        else if (root.name() == KXMLQLCEFXFixtureHead)
        {
            /* Fixture Head */
            head.head = root.readElementText().toInt();
        }
        else if (root.name() == KXMLQLCEFXFixtureMode)
        {
            /* Fixture Mode */
            setMode ((Mode) root.readElementText().toInt());
        }
        else if (root.name() == KXMLQLCEFXFixtureDirection)
        {
            /* Direction */
            Function::Direction dir = Function::stringToDirection(root.readElementText());
            setDirection(dir);
        }
        else if (root.name() == KXMLQLCEFXFixtureStartOffset)
        {
            /* Start offset */
            setStartOffset(root.readElementText().toInt());
        }
        else if (root.name() == KXMLQLCEFXFixtureIntensity)
        {
            /* Intensity - LEGACY */
            root.skipCurrentElement();
        }
        else
        {
            qWarning() << "Unknown EFX Fixture tag:" << root.name();
            root.skipCurrentElement();
        }
    }

    if (head.fxi != Fixture::invalidId())
       setHead(head);

    return true;
}

bool EFXFixture::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    /* EFXFixture */
    doc->writeStartElement(KXMLQLCEFXFixture);

    /* Fixture ID */
    doc->writeTextElement(KXMLQLCEFXFixtureID, QString("%1").arg(head().fxi));
    /* Fixture Head */
    doc->writeTextElement(KXMLQLCEFXFixtureHead, QString("%1").arg(head().head));
    /* Mode */
    doc->writeTextElement(KXMLQLCEFXFixtureMode, QString::number(mode()));
    /* Direction */
    doc->writeTextElement(KXMLQLCEFXFixtureDirection, Function::directionToString(m_direction));
    /* Start offset */
    doc->writeTextElement(KXMLQLCEFXFixtureStartOffset, QString::number(startOffset()));

    /* End the <Fixture> tag */
    doc->writeEndElement();

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
    m_currentAngle = SCALE(float(pos),
                           float(0), float(m_parent->duration()),
                           float(0), float(M_PI * 2));

    float valX = 0;
    float valY = 0;

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
            setPointRGB(universes, valX, valY);
            break;

        case Dimmer:
            //Use Y for coherence with RGB gradient.
            setPointDimmer(universes, valY);
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

void EFXFixture::setPointPanTilt(QList<Universe *> universes, float pan, float tilt)
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

void EFXFixture::setPointDimmer(QList<Universe *> universes, float dimmer)
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

void EFXFixture::setPointRGB(QList<Universe *> universes, float x, float y)
{
    Q_UNUSED(universes);

    Fixture* fxi = doc()->fixture(head().fxi);
    Q_ASSERT(fxi != NULL);

    QVector<quint32> rgbChannels = fxi->rgbChannels(head().head);

    /* Don't write dimmer data directly to universes but use FadeChannel to avoid steps at EFX loop restart */
    if (rgbChannels.size() >= 3)
    {
        QColor pixel = m_rgbGradient.pixel(x, y);

        setFadeChannel(rgbChannels[0], pixel.red());
        setFadeChannel(rgbChannels[1], pixel.green());
        setFadeChannel(rgbChannels[2], pixel.blue());
    }
}

void EFXFixture::start(MasterTimer* timer, QList<Universe *> universes)
{
    Q_UNUSED(universes);
    Q_UNUSED(timer);

    m_started = true;
}

void EFXFixture::stop(MasterTimer* timer, QList<Universe *> universes)
{
    Q_UNUSED(timer)
    Q_UNUSED(universes);

    m_started = false;
}

/*****************************************************************************
 * Helper Function
 *****************************************************************************/
void EFXFixture::setFadeChannel(quint32 nChannel, uchar val)
{
    FadeChannel fc(doc(), head().fxi, nChannel);

    fc.setTarget(val);
    m_parent->m_fader->forceAdd(fc);
}
