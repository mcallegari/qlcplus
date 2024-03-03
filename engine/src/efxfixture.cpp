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
#include "fadechannel.h"
#include "mastertimer.h"
#include "efxfixture.h"
#include "qlcmacros.h"
#include "function.h"
#include "universe.h"
#include "gradient.h"
#include "efx.h"
#include "doc.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/
QImage EFXFixture::m_rgbGradient = QImage();

EFXFixture::EFXFixture(const EFX* parent)
    : m_parent(parent)
    , m_head()
    , m_universe(Universe::invalid())
    , m_direction(Function::Forward)
    , m_startOffset(0)
    , m_mode(EFXFixture::PanTilt)

    , m_serialNumber(0)
    , m_runTimeDirection(Function::Forward)
    , m_done(false)
    , m_started(false)
    , m_elapsed(0)
    , m_currentAngle(0)

    , m_firstMsbChannel(QLCChannel::invalid())
    , m_firstLsbChannel(QLCChannel::invalid())
    , m_secondMsbChannel(QLCChannel::invalid())
    , m_secondLsbChannel(QLCChannel::invalid())
{
    Q_ASSERT(parent != NULL);

    if (m_rgbGradient.isNull ())
        m_rgbGradient = Gradient::getRGBGradient (256, 256);
}

void EFXFixture::copyFrom(const EFXFixture* ef)
{
    // Don't copy m_parent because it is already assigned in constructor and might
    // be different than $ef's
    m_head = ef->m_head;
    m_universe = ef->m_universe;
    m_direction = ef->m_direction;
    m_startOffset = ef->m_startOffset;
    m_mode = ef->m_mode;

    m_serialNumber = ef->m_serialNumber;
    m_runTimeDirection = ef->m_runTimeDirection;
    m_done = ef->m_done;
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

    Fixture *fxi = doc()->fixture(head.fxi);
    if (fxi == NULL)
        return;

    m_universe = fxi->universe();

    QList<Mode> modes;

    if (fxi->channelNumber(QLCChannel::Pan, QLCChannel::MSB, head.head) != QLCChannel::invalid() ||
        fxi->channelNumber(QLCChannel::Tilt, QLCChannel::MSB, head.head) != QLCChannel::invalid())
        modes << PanTilt;

    if (fxi->masterIntensityChannel() != QLCChannel::invalid() ||
        fxi->channelNumber(QLCChannel::Intensity, QLCChannel::MSB, head.head) != QLCChannel::invalid())
        modes << Dimmer;

    if (fxi->rgbChannels(head.head).size() >= 3)
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

quint32 EFXFixture::universe()
{
    return m_universe;
}

bool EFXFixture::isValid() const
{
    Fixture *fxi = doc()->fixture(head().fxi);

    if (fxi == NULL)
        return false;
    else if (head().head >= fxi->heads())
        return false;
    else if (m_mode == PanTilt &&
             fxi->channelNumber(QLCChannel::Pan, QLCChannel::MSB, head().head) == QLCChannel::invalid() && // Maybe a device can pan OR tilt
             fxi->channelNumber(QLCChannel::Tilt, QLCChannel::MSB, head().head) == QLCChannel::invalid())   // but not both
        return false;
    else if (m_mode == Dimmer &&
             fxi->masterIntensityChannel() == QLCChannel::invalid() &&
             fxi->channelNumber(QLCChannel::Intensity, QLCChannel::MSB, head().head) == QLCChannel::invalid())
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
            float(0), float(m_parent->loopDuration()));

    // Serial or Asymmetric propagation mode:
    // we must substract the offset from the current position
    if (timeOffset())
    {
        if (m_elapsed < timeOffset())
            m_elapsed += m_parent->loopDuration();
        m_elapsed -= timeOffset();
    }
}

QStringList EFXFixture::modeList()
{
    Fixture* fxi = doc()->fixture(head().fxi);
    Q_ASSERT(fxi != NULL);

    QStringList modes;

    if (fxi->channelNumber(QLCChannel::Pan, QLCChannel::MSB, head().head) != QLCChannel::invalid() ||
       fxi->channelNumber(QLCChannel::Tilt, QLCChannel::MSB, head().head) != QLCChannel::invalid())
        modes << KXMLQLCEFXFixtureModePanTilt;

    if (fxi->masterIntensityChannel() != QLCChannel::invalid() ||
       fxi->channelNumber(QLCChannel::Intensity, QLCChannel::MSB, head().head) != QLCChannel::invalid())
        modes << KXMLQLCEFXFixtureModeDimmer;

    if (fxi->rgbChannels(head().head).size() >= 3)
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
    m_done = false;
    m_runTimeDirection = m_direction;
    m_started = false;
    m_elapsed = 0;
    m_currentAngle = 0;
}

bool EFXFixture::isDone() const
{
    return m_done;
}

uint EFXFixture::timeOffset() const
{
    if (m_parent->propagationMode() == EFX::Asymmetric ||
        m_parent->propagationMode() == EFX::Serial)
    {
        return m_parent->loopDuration() / (m_parent->fixtures().size() + 1) * serialNumber();
    }
    else
    {
        return 0;
    }
}

/*****************************************************************************
 * Running
 *****************************************************************************/

void EFXFixture::start(QSharedPointer<GenericFader> fader)
{
    Fixture *fxi = doc()->fixture(head().fxi);

    /* Cache channels to reduce processing while running */
    switch (m_mode)
    {
        case PanTilt:
        {
            m_firstMsbChannel = fxi->channelNumber(QLCChannel::Pan, QLCChannel::MSB, head().head);
            m_firstLsbChannel = fxi->channelNumber(QLCChannel::Pan, QLCChannel::LSB, head().head);
            m_secondMsbChannel = fxi->channelNumber(QLCChannel::Tilt, QLCChannel::MSB, head().head);
            m_secondLsbChannel = fxi->channelNumber(QLCChannel::Tilt, QLCChannel::LSB, head().head);

            /* Check for non-contiguous channels */
            if ((m_firstLsbChannel != QLCChannel::invalid() && m_firstLsbChannel - m_firstMsbChannel != 1) ||
                (m_secondLsbChannel != QLCChannel::invalid() && m_secondLsbChannel - m_secondMsbChannel != 1))
            {
                fader->setHandleSecondary(false);
            }
        }
        break;

        case RGB:
        break;

        case Dimmer:
        {
            m_firstMsbChannel = fxi->channelNumber(QLCChannel::Intensity, QLCChannel::MSB, head().head);
            if (m_firstMsbChannel != QLCChannel::invalid())
            {
                m_firstLsbChannel = fxi->channelNumber(QLCChannel::Intensity, QLCChannel::LSB, head().head);

                /* Check for non-contiguous channels */
                if (m_firstLsbChannel != QLCChannel::invalid() && m_firstLsbChannel - m_firstMsbChannel != 1)
                {
                    fader->setHandleSecondary(false);
                }
            }
            else
            {
                m_firstMsbChannel = fxi->masterIntensityChannel();
            }
        }
        break;
    }
    m_started = true;
}

void EFXFixture::stop()
{
    m_started = false;
}

void EFXFixture::nextStep(QList<Universe *> universes, QSharedPointer<GenericFader> fader)
{
    // Nothing to do
    if (m_parent->loopDuration() == 0)
        return;

    // Bail out without doing anything if this fixture is ready (after single-shot)
    // or it has no pan&tilt channels (not valid).
    if (m_done == true || isValid() == false)
        return;

    m_elapsed += MasterTimer::tick();

    // Check time wrapping
    if (m_elapsed > m_parent->loopDuration())
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
            m_done = true;
            stop();
        }

        m_elapsed = 0;
    }

    // Bail out without doing anything if this fixture is waiting for its turn.
    if (m_parent->propagationMode() == EFX::Serial && m_elapsed < timeOffset() && !m_started)
        return;

    // Fade in
    if (m_started == false)
        start(fader);

    // Scale from elapsed time in relation to overall duration to a point in a circle
    uint pos = (m_elapsed + timeOffset()) % m_parent->loopDuration();
    m_currentAngle = SCALE(float(pos),
                           float(0), float(m_parent->loopDuration()),
                           float(0), float(M_PI * 2));

    float valX = 0;
    float valY = 0;

    m_parent->calculatePoint(m_runTimeDirection, m_startOffset, m_currentAngle, &valX, &valY);

    /* Set target values on faders/universes */
    switch (m_mode)
    {
        case PanTilt:
            setPointPanTilt(universes, fader, valX, valY);
        break;

        case RGB:
            setPointRGB(universes, fader, valX, valY);
        break;

        case Dimmer:
            //Use Y for coherence with RGB gradient.
            setPointDimmer(universes, fader, valY);
        break;
    }
}

void EFXFixture::updateFaderValues(FadeChannel *fc, quint32 value)
{
    fc->setStart(fc->current());
    fc->setTarget(value);
    fc->setElapsed(0);
    fc->setReady(false);
    fc->setFadeTime(0);
}

void EFXFixture::setPointPanTilt(QList<Universe *> universes, QSharedPointer<GenericFader> fader,
                                 float pan, float tilt)
{
    if (fader.isNull())
        return;

    Universe *uni = universes[universe()];

    //qDebug() << "Pan value: " << pan << ", tilt value:" << tilt;

    /* Check for outbound values */
    if (pan < 0)
        pan = 0;

    if (tilt < 0)
        tilt = 0;

    /* Write full 16bit point data to universes */
    if (m_firstMsbChannel != QLCChannel::invalid())
    {
        quint32 panValue = quint32(pan);
        FadeChannel *fc = fader->getChannelFader(doc(), uni, head().fxi, m_firstMsbChannel);
        if (m_firstLsbChannel != QLCChannel::invalid())
        {
            if (fader->handleSecondary())
            {
                fc = fader->getChannelFader(doc(), uni, head().fxi, m_firstLsbChannel);
                panValue = (panValue << 8) + quint32((pan - floor(pan)) * float(UCHAR_MAX));
            }
            else
            {
                FadeChannel *lsbFc = fader->getChannelFader(doc(), uni, head().fxi, m_firstLsbChannel);
                updateFaderValues(lsbFc, quint32((pan - floor(pan)) * float(UCHAR_MAX)));
            }
        }
        if (m_parent->isRelative())
            fc->addFlag(FadeChannel::Relative);

        updateFaderValues(fc, panValue);
    }
    if (m_secondMsbChannel != QLCChannel::invalid())
    {
        quint32 tiltValue = quint32(tilt);
        FadeChannel *fc = fader->getChannelFader(doc(), uni, head().fxi, m_secondMsbChannel);
        if (m_secondLsbChannel != QLCChannel::invalid())
        {
            if (fader->handleSecondary())
            {
                fc = fader->getChannelFader(doc(), uni, head().fxi, m_secondLsbChannel);
                tiltValue = (tiltValue << 8) + quint32((tilt - floor(tilt)) * float(UCHAR_MAX));
            }
            else
            {
                FadeChannel *lsbFc = fader->getChannelFader(doc(), uni, head().fxi, m_secondLsbChannel);
                updateFaderValues(lsbFc, quint32((tilt - floor(tilt)) * float(UCHAR_MAX)));
            }
        }
        if (m_parent->isRelative())
            fc->addFlag(FadeChannel::Relative);

        updateFaderValues(fc, tiltValue);
    }
}

void EFXFixture::setPointDimmer(QList<Universe *> universes, QSharedPointer<GenericFader> fader, float dimmer)
{
    if (fader.isNull())
        return;

    Universe *uni = universes[universe()];

    /* Don't write dimmer data directly to universes but use FadeChannel to avoid steps at EFX loop restart */
    if (m_firstMsbChannel != QLCChannel::invalid())
    {
        quint32 dimmerValue = quint32(dimmer);
        FadeChannel *fc = fader->getChannelFader(doc(), uni, head().fxi, m_firstMsbChannel);

        if (m_firstLsbChannel != QLCChannel::invalid())
        {
            if (fader->handleSecondary())
            {
                fc = fader->getChannelFader(doc(), uni, head().fxi, m_firstLsbChannel);
                dimmerValue = (dimmerValue << 8) + quint32((dimmer - floor(dimmer)) * float(UCHAR_MAX));
            }
        }
        updateFaderValues(fc, dimmerValue);
    }
}

void EFXFixture::setPointRGB(QList<Universe *> universes, QSharedPointer<GenericFader> fader, float x, float y)
{
    if (fader.isNull())
        return;

    Fixture* fxi = doc()->fixture(head().fxi);
    Q_ASSERT(fxi != NULL);
    Universe *uni = universes[universe()];

    QVector<quint32> rgbChannels = fxi->rgbChannels(head().head);

    /* Don't write dimmer data directly to universes but use FadeChannel to avoid steps at EFX loop restart */
    if (rgbChannels.size() >= 3 && !fader.isNull())
    {
        QColor pixel = m_rgbGradient.pixel(x, y);

        FadeChannel *fc = fader->getChannelFader(doc(), uni, fxi->id(), rgbChannels[0]);
        updateFaderValues(fc, pixel.red());
        fc = fader->getChannelFader(doc(), uni, fxi->id(), rgbChannels[1]);
        updateFaderValues(fc, pixel.green());
        fc = fader->getChannelFader(doc(), uni, fxi->id(), rgbChannels[2]);
        updateFaderValues(fc, pixel.blue());
    }
}

