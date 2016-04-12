/*
  Q Light Controller Plus
  rgbmatrix.cpp

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
#include <QCoreApplication>
#include <QDebug>
#include <QTime>
#include <cmath>
#include <QDir>

#include "qlcfixturehead.h"
#include "fixturegroup.h"
#include "genericfader.h"
#include "fadechannel.h"
#include "rgbmatrix.h"
#include "qlcmacros.h"
#include "rgbaudio.h"
#include "rgbscriptscache.h"
#include "doc.h"

#define KXMLQLCRGBMatrixStartColor "MonoColor"
#define KXMLQLCRGBMatrixEndColor "EndColor"
#define KXMLQLCRGBMatrixFixtureGroup "FixtureGroup"
#define KXMLQLCRGBMatrixDimmerControl "DimmerControl"

#define KXMLQLCRGBMatrixProperty "Property"
#define KXMLQLCRGBMatrixPropertyName "Name"
#define KXMLQLCRGBMatrixPropertyValue "Value"

/****************************************************************************
 * Initialization
 ****************************************************************************/

RGBMatrix::RGBMatrix(Doc* doc)
    : Function(doc, Function::RGBMatrix)
    , m_dimmerControl(true)
    , m_fixtureGroupID(FixtureGroup::invalidId())
    , m_group(NULL)
    , m_algorithm(NULL)
    , m_algorithmMutex(QMutex::Recursive)
    , m_startColor(Qt::red)
    , m_endColor(QColor())
    , m_fader(NULL)
    , m_step(0)
    , m_roundTime(new QTime)
    , m_stepColor(QColor())
    , m_crDelta(0.0)
    , m_cgDelta(0.0)
    , m_cbDelta(0.0)
    , m_stepCount(0)
{
    setName(tr("New RGB Matrix"));
    setDuration(500);

    RGBScript scr = doc->rgbScriptsCache()->script("Stripes");
    setAlgorithm(scr.clone());
}

RGBMatrix::~RGBMatrix()
{
    delete m_algorithm;
    delete m_roundTime;
}

void RGBMatrix::setTotalDuration(quint32 msec)
{
    QMutexLocker algorithmLocker(&m_algorithmMutex);

    if (m_fixtureGroupID == FixtureGroup::invalidId() ||
        m_algorithm == NULL)
            return;

    FixtureGroup* grp = doc()->fixtureGroup(fixtureGroup());
    if (grp != NULL)
    {
        int steps = m_algorithm->rgbMapStepCount(grp->size());
        setDuration(msec / steps);
    }
}

quint32 RGBMatrix::totalDuration()
{
    QMutexLocker algorithmLocker(&m_algorithmMutex);

    if (m_fixtureGroupID == FixtureGroup::invalidId() ||
        m_algorithm == NULL)
            return 0;

    FixtureGroup* grp = doc()->fixtureGroup(fixtureGroup());
    if (grp != NULL)
    {
        qDebug () << "Algorithm steps:" << m_algorithm->rgbMapStepCount(grp->size());
        return m_algorithm->rgbMapStepCount(grp->size()) * duration();
    }

    return 0;
}

void RGBMatrix::setDimmerControl(bool dimmerControl)
{
    m_dimmerControl = dimmerControl;
}

bool RGBMatrix::dimmerControl() const
{
    return m_dimmerControl;
}

/****************************************************************************
 * Copying
 ****************************************************************************/

Function* RGBMatrix::createCopy(Doc* doc, bool addToDoc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new RGBMatrix(doc);
    if (copy->copyFrom(this) == false)
    {
        delete copy;
        copy = NULL;
    }
    if (addToDoc == true && doc->addFunction(copy) == false)
    {
        delete copy;
        copy = NULL;
    }

    return copy;
}

bool RGBMatrix::copyFrom(const Function* function)
{
    const RGBMatrix* mtx = qobject_cast<const RGBMatrix*> (function);
    if (mtx == NULL)
        return false;

    setDimmerControl(mtx->dimmerControl());
    setFixtureGroup(mtx->fixtureGroup());
    if (mtx->algorithm() != NULL)
        setAlgorithm(mtx->algorithm()->clone());
    else
        setAlgorithm(NULL);
    setStartColor(mtx->startColor());
    setEndColor(mtx->endColor());

    return Function::copyFrom(function);
}

/****************************************************************************
 * Fixtures
 ****************************************************************************/

void RGBMatrix::setFixtureGroup(quint32 id)
{
    m_fixtureGroupID = id;
    QMutexLocker algoLocker(&m_algorithmMutex);
    m_group = doc()->fixtureGroup(m_fixtureGroupID);
}

quint32 RGBMatrix::fixtureGroup() const
{
    return m_fixtureGroupID;
}

/****************************************************************************
 * Algorithm
 ****************************************************************************/

void RGBMatrix::setAlgorithm(RGBAlgorithm* algo)
{
    {
        QMutexLocker algorithmLocker(&m_algorithmMutex);
        delete m_algorithm;
        m_algorithm = algo;
    }
    emit changed(id());
}

RGBAlgorithm* RGBMatrix::algorithm() const
{
    return m_algorithm;
}

QMutex& RGBMatrix::algorithmMutex()
{
    return m_algorithmMutex;
}

int RGBMatrix::stepsCount()
{
    QMutexLocker algorithmLocker(&m_algorithmMutex);

    if (m_algorithm == NULL)
        return 0;

    FixtureGroup* grp = doc()->fixtureGroup(fixtureGroup());
    if (grp != NULL)
        return m_algorithm->rgbMapStepCount(grp->size());

    return 0;
}

RGBMap RGBMatrix::previewMap(int step)
{
    RGBMap map;
    QMutexLocker algorithmLocker(&m_algorithmMutex);
    if (m_algorithm == NULL)
        return map;

    if (m_group == NULL)
        m_group = doc()->fixtureGroup(fixtureGroup());

    if (m_group != NULL)
        map = m_algorithm->rgbMap(m_group->size(), m_stepColor.rgb(), step);

    return map;
}

/****************************************************************************
 * Colour
 ****************************************************************************/

void RGBMatrix::setStartColor(const QColor& c)
{
    m_startColor = c;
    {
        QMutexLocker algorithmLocker(&m_algorithmMutex);
        if (m_algorithm != NULL)
            m_algorithm->setColors(m_startColor, m_endColor);
    }
    emit changed(id());
}

QColor RGBMatrix::startColor() const
{
    return m_startColor;
}

void RGBMatrix::setEndColor(const QColor &c)
{
    m_endColor = c;
    {
        QMutexLocker algorithmLocker(&m_algorithmMutex);
        if (m_algorithm != NULL)
            m_algorithm->setColors(m_startColor, m_endColor);
    }
    emit changed(id());
}

QColor RGBMatrix::endColor() const
{
    return m_endColor;
}

void RGBMatrix::calculateColorDelta()
{
    m_crDelta = 0;
    m_cgDelta = 0;
    m_cbDelta = 0;
    m_stepCount = 0;

    if (m_endColor.isValid())
    {
        if (doc() == NULL)
            return;

        FixtureGroup* grp = doc()->fixtureGroup(fixtureGroup());
        QMutexLocker algorithmLocker(&m_algorithmMutex);
        if (grp != NULL && m_algorithm != NULL)
        {
            m_stepCount = m_algorithm->rgbMapStepCount(grp->size()) - 1;
            if (m_stepCount > 0)
            {
                m_crDelta = m_endColor.red() - m_startColor.red();
                m_cgDelta = m_endColor.green() - m_startColor.green();
                m_cbDelta = m_endColor.blue() - m_startColor.blue();
            }
        }
    }
}

void RGBMatrix::setStepColor(QColor color)
{
    m_stepColor = color;
}

QColor RGBMatrix::stepColor()
{
    return m_stepColor;
}

void RGBMatrix::updateStepColor(int step)
{
    if (m_stepCount <= 0)
        return;

    m_stepColor.setRed(m_startColor.red() + (m_crDelta * step / m_stepCount));
    m_stepColor.setGreen(m_startColor.green() + (m_cgDelta * step / m_stepCount));
    m_stepColor.setBlue(m_startColor.blue() + (m_cbDelta * step / m_stepCount));
}

/************************************************************************
 * Properties
 ************************************************************************/

void RGBMatrix::setProperty(QString propName, QString value)
{
    QMutexLocker algoLocker(&m_algorithmMutex);
    m_properties[propName] = value;
}

QString RGBMatrix::property(QString propName)
{
    QMutexLocker algoLocker(&m_algorithmMutex);
    return m_properties[propName];
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool RGBMatrix::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root.attributes().value(KXMLQLCFunctionType).toString() != typeToString(Function::RGBMatrix))
    {
        qWarning() << Q_FUNC_INFO << "Function is not an RGB matrix";
        return false;
    }

    /* Load matrix contents */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCFunctionSpeed)
        {
            loadXMLSpeed(root);
        }
        else if (root.name() == KXMLQLCRGBAlgorithm)
        {
            setAlgorithm(RGBAlgorithm::loader(doc(), root));
        }
        else if (root.name() == KXMLQLCRGBMatrixFixtureGroup)
        {
            setFixtureGroup(root.readElementText().toUInt());
        }
        else if (root.name() == KXMLQLCFunctionDirection)
        {
            loadXMLDirection(root);
        }
        else if (root.name() == KXMLQLCFunctionRunOrder)
        {
            loadXMLRunOrder(root);
        }
        else if (root.name() == KXMLQLCRGBMatrixStartColor)
        {
            setStartColor(QColor::fromRgb(QRgb(root.readElementText().toUInt())));
        }
        else if (root.name() == KXMLQLCRGBMatrixEndColor)
        {
            setEndColor(QColor::fromRgb(QRgb(root.readElementText().toUInt())));
        }
        else if (root.name() == KXMLQLCRGBMatrixProperty)
        {
            QString name = root.attributes().value(KXMLQLCRGBMatrixPropertyName).toString();
            QString value = root.attributes().value(KXMLQLCRGBMatrixPropertyValue).toString();
            setProperty(name, value);
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLQLCRGBMatrixDimmerControl)
        {
            setDimmerControl(root.readElementText().toInt());
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown RGB matrix tag:" << root.name();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool RGBMatrix::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    /* Function tag */
    doc->writeStartElement(KXMLQLCFunction);

    /* Common attributes */
    saveXMLCommon(doc);

    /* Speeds */
    saveXMLSpeed(doc);

    /* Direction */
    saveXMLDirection(doc);

    /* Run order */
    saveXMLRunOrder(doc);

    /* Algorithm */
    if (m_algorithm != NULL)
        m_algorithm->saveXML(doc);

    /* Dimmer Control */
    doc->writeTextElement(KXMLQLCRGBMatrixDimmerControl, QString::number(dimmerControl()));

    /* Start Color */
    doc->writeTextElement(KXMLQLCRGBMatrixStartColor, QString::number(startColor().rgb()));

    /* End Color */
    if (endColor().isValid())
    {
        doc->writeTextElement(KXMLQLCRGBMatrixEndColor, QString::number(endColor().rgb()));
    }

    /* Fixture Group */
    doc->writeTextElement(KXMLQLCRGBMatrixFixtureGroup, QString::number(fixtureGroup()));

    /* Properties */
    QHashIterator<QString, QString> it(m_properties);
    while(it.hasNext())
    {
        it.next();
        doc->writeStartElement(KXMLQLCRGBMatrixProperty);
        doc->writeAttribute(KXMLQLCRGBMatrixPropertyName, it.key());
        doc->writeAttribute(KXMLQLCRGBMatrixPropertyValue, it.value());
        doc->writeEndElement();
    }

    /* End the <Function> tag */
    doc->writeEndElement();

    return true;
}

/****************************************************************************
 * Running
 ****************************************************************************/

void RGBMatrix::tap()
{
    if (stopped() == false)
    {
        FixtureGroup* grp = doc()->fixtureGroup(fixtureGroup());
        // Filter out taps that are too close to each other
        if (grp != NULL && uint(m_roundTime->elapsed()) >= (duration() / 4))
        {
            roundCheck(grp->size());
            resetElapsed();
        }
    }
}

void RGBMatrix::preRun(MasterTimer* timer)
{
    Q_UNUSED(timer);

    {
        QMutexLocker algorithmLocker(&m_algorithmMutex);

        m_group = doc()->fixtureGroup(m_fixtureGroupID);
        if (m_group == NULL)
        {
            // No fixture group to control
            stop(FunctionParent::master());
            return;
        }

        if (m_algorithm != NULL)
        {
            Q_ASSERT(m_fader == NULL);
            m_fader = new GenericFader(doc());
            m_fader->adjustIntensity(getAttributeValue(Intensity));
            m_fader->setBlendMode(blendMode());

            // Copy direction from parent class direction
            m_direction = direction();

            if (m_direction == Forward)
            {
                m_step = 0;
                m_stepColor = m_startColor.rgb();
            }
            else
            {
                m_step = m_algorithm->rgbMapStepCount(m_group->size()) - 1;
                if (m_endColor.isValid())
                {
                    m_stepColor = m_endColor.rgb();
                }
                else
                {
                    m_stepColor = m_startColor.rgb();
                }
            }
            calculateColorDelta();
            if (m_algorithm->type() == RGBAlgorithm::Script)
            {
                RGBScript *script = static_cast<RGBScript*> (m_algorithm);
                QHashIterator<QString, QString> it(m_properties);
                while(it.hasNext())
                {
                    it.next();
                    script->setProperty(it.key(), it.value());
                }
            }
        }
    }

    m_roundTime->start();

    Function::preRun(timer);
}

void RGBMatrix::write(MasterTimer* timer, QList<Universe *> universes)
{
    Q_UNUSED(timer);

    {
        QMutexLocker algorithmLocker(&m_algorithmMutex);
        if (m_group == NULL)
        {
            // No fixture group to control
            stop(FunctionParent::master());
            return;
        }

        // No time to do anything.
        if (duration() == 0)
            return;

        // Invalid/nonexistent script
        if (m_algorithm == NULL || m_algorithm->apiVersion() == 0)
            return;

        if (isPaused() == false)
        {
            // Get new map every time when elapsed is reset to zero
            if (elapsed() < MasterTimer::tick())
            {
                qDebug() << "RGBMatrix stepColor:" << QString::number(m_stepColor.rgb(), 16);
                RGBMap map = m_algorithm->rgbMap(m_group->size(), m_stepColor.rgb(), m_step);
                updateMapChannels(map, m_group);
            }
        }
    }

    // Run the generic fader that takes care of fading in/out individual channels
    m_fader->write(universes, isPaused());

    if (isPaused() == false)
    {
        // Increment elapsed time
        incrementElapsed();

        // Check if we need to change direction, stop completely or go to next step
        if (elapsed() >= duration())
            roundCheck(m_group->size());
    }
}

void RGBMatrix::postRun(MasterTimer* timer, QList<Universe *> universes)
{
    if (m_fader != NULL)
    {
        QHashIterator <FadeChannel,FadeChannel> it(m_fader->channels());
        while (it.hasNext() == true)
        {
            it.next();
            FadeChannel fc = it.value();
            // fade out only intensity channels
            if (fc.group(doc()) != QLCChannel::Intensity)
                continue;

            bool canFade = true;
            Fixture *fixture = doc()->fixture(fc.fixture());
            if (fixture != NULL)
                canFade = fixture->channelCanFade(fc.channel());
            fc.setStart(fc.current(getAttributeValue(Intensity)));
            fc.setCurrent(fc.current(getAttributeValue(Intensity)));

            fc.setElapsed(0);
            fc.setReady(false);
            if (canFade == false)
            {
                fc.setFadeTime(0);
                fc.setTarget(fc.current(getAttributeValue(Intensity)));
            }
            else
            {
                if (overrideFadeOutSpeed() == defaultSpeed())
                    fc.setFadeTime(fadeOutSpeed());
                else
                    fc.setFadeTime(overrideFadeOutSpeed());
                fc.setTarget(0);
            }
            timer->faderAdd(fc);
        }

        delete m_fader;
        m_fader = NULL;
    }

    {
        QMutexLocker algorithmLocker(&m_algorithmMutex);
        if (m_algorithm != NULL)
            m_algorithm->postRun();
    }

    Function::postRun(timer, universes);
}

void RGBMatrix::roundCheck(const QSize& size)
{
    QMutexLocker algorithmLocker(&m_algorithmMutex);
    if (m_algorithm == NULL)
        return;

    if (runOrder() == PingPong)
    {
        if (m_direction == Forward && (m_step + 1) == m_algorithm->rgbMapStepCount(size))
        {
            m_direction = Backward;
            m_step = m_algorithm->rgbMapStepCount(size) - 2;
            if (m_endColor.isValid())
                m_stepColor = m_endColor;

            updateStepColor(m_step);
        }
        else if (m_direction == Backward && (m_step - 1) < 0)
        {
            m_direction = Forward;
            m_step = 1;
            m_stepColor = m_startColor;
            updateStepColor(m_step);
        }
        else
        {
            if (m_direction == Forward)
                m_step++;
            else
                m_step--;
            updateStepColor(m_step);
        }
    }
    else if (runOrder() == SingleShot)
    {
        if (m_direction == Forward)
        {
            if (m_step >= m_algorithm->rgbMapStepCount(size) - 1)
                stop(FunctionParent::master());
            else
            {
                m_step++;
                updateStepColor(m_step);
            }
        }
        else
        {
            if (m_step <= 0)
                stop(FunctionParent::master());
            else
            {
                m_step--;
                updateStepColor(m_step);
            }
        }
    }
    else
    {
        if (m_direction == Forward)
        {
            if (m_step >= m_algorithm->rgbMapStepCount(size) - 1)
            {
                m_step = 0;
                m_stepColor = m_startColor;
            }
            else
            {
                m_step++;
                updateStepColor(m_step);
            }
        }
        else
        {
            if (m_step <= 0)
            {
                m_step = m_algorithm->rgbMapStepCount(size) - 1;
                if (m_endColor.isValid())
                    m_stepColor = m_endColor;
            }
            else
            {
                m_step--;
                updateStepColor(m_step);
            }
        }
    }

    m_roundTime->restart();
    roundElapsed(duration());
}

void RGBMatrix::updateMapChannels(const RGBMap& map, const FixtureGroup* grp)
{
    quint32 mdAssigned = QLCChannel::invalid();
    quint32 mdFxi = Fixture::invalidId();

    uint fadeTime = 0;
    if (overrideFadeInSpeed() == defaultSpeed())
        fadeTime = fadeInSpeed();
    else
        fadeTime = overrideFadeInSpeed();

    // Create/modify fade channels for ALL pixels in the color map.
    for (int y = 0; y < map.size(); y++)
    {
        for (int x = 0; x < map[y].size(); x++)
        {
            QLCPoint pt(x, y);
            GroupHead grpHead(grp->head(pt));
            Fixture* fxi = doc()->fixture(grpHead.fxi);
            if (fxi == NULL)
                continue;

            if (grpHead.fxi != mdFxi)
            {
                mdAssigned = QLCChannel::invalid();
                mdFxi = grpHead.fxi;
            }

            QLCFixtureHead head = fxi->head(grpHead.head);

            QVector <quint32> rgb = head.rgbChannels();
            QVector <quint32> cmy = head.cmyChannels();
            if (rgb.size() == 3)
            {
                // RGB color mixing
                {
                    FadeChannel fc(doc(), grpHead.fxi, rgb.at(0));
                    fc.setTarget(qRed(map[y][x]));
                    insertStartValues(fc, fadeTime);
                    m_fader->add(fc);
                }

                {
                    FadeChannel fc(doc(), grpHead.fxi, rgb.at(1));
                    fc.setTarget(qGreen(map[y][x]));
                    insertStartValues(fc, fadeTime);
                    m_fader->add(fc);
                }

                {
                    FadeChannel fc(doc(), grpHead.fxi, rgb.at(2));
                    fc.setTarget(qBlue(map[y][x]));
                    insertStartValues(fc, fadeTime);
                    m_fader->add(fc);
                }
            }
            else if (cmy.size() == 3)
            {
                // CMY color mixing
                QColor col(map[y][x]);

                {
                    FadeChannel fc(doc(), grpHead.fxi, cmy.at(0));
                    fc.setTarget(col.cyan());
                    insertStartValues(fc, fadeTime);
                    m_fader->add(fc);
                }

                {
                    FadeChannel fc(doc(), grpHead.fxi, cmy.at(1));
                    fc.setTarget(col.magenta());
                    insertStartValues(fc, fadeTime);
                    m_fader->add(fc);
                }

                {
                    FadeChannel fc(doc(), grpHead.fxi, cmy.at(2));
                    fc.setTarget(col.yellow());
                    insertStartValues(fc, fadeTime);
                    m_fader->add(fc);
                }
            }

            if (m_dimmerControl &&
                head.masterIntensityChannel() != QLCChannel::invalid())
            {
                //qDebug() << "RGBMatrix: found dimmer at" << head.masterIntensityChannel();
                // Simple intensity (dimmer) channel
                QColor col(map[y][x]);
                FadeChannel fc(doc(), grpHead.fxi, head.masterIntensityChannel());
                if (col.value() == 0 && mdAssigned != head.masterIntensityChannel())
                    fc.setTarget(0);
                else
                {
                    fc.setTarget(255);
                    if (mdAssigned == QLCChannel::invalid())
                        mdAssigned = head.masterIntensityChannel();
                }
                insertStartValues(fc, fadeTime);
                m_fader->add(fc);
            }
        }
    }
}

void RGBMatrix::insertStartValues(FadeChannel& fc, uint fadeTime) const
{
    Q_ASSERT(m_fader != NULL);

    // To create a nice and smooth fade, get the starting value from
    // m_fader's existing FadeChannel (if any). Otherwise just assume
    // we're starting from zero.
    QHash <FadeChannel,FadeChannel>::const_iterator oldChannelIterator = m_fader->channels().find(fc);
    if (oldChannelIterator != m_fader->channels().end())
    {
        FadeChannel old = oldChannelIterator.value();
        fc.setCurrent(old.current());
        if (fc.target() == old.target())
        {
            fc.setStart(old.start());
            fc.setElapsed(old.elapsed());
        }
        else
            fc.setStart(old.current());
    }
    else
    {
        fc.setCurrent(0);
        fc.setStart(0);
    }

    // The channel is not ready yet
    fc.setReady(false);

    // Fade in speed is used for all non-zero targets
    if (fc.target() == 0)
        fc.setFadeTime(fadeOutSpeed());
    else
        fc.setFadeTime(fadeTime);
}

/*********************************************************************
 * Attributes
 *********************************************************************/

void RGBMatrix::adjustAttribute(qreal fraction, int attributeIndex)
{
    if (m_fader != NULL && attributeIndex == Function::Intensity)
        m_fader->adjustIntensity(fraction);
    Function::adjustAttribute(fraction, attributeIndex);
}

/*************************************************************************
 * Blend mode
 *************************************************************************/

void RGBMatrix::setBlendMode(Universe::BlendMode mode)
{
    if (m_fader != NULL)
        m_fader->setBlendMode(mode);
    Function::setBlendMode(mode);
    emit changed(id());
}
