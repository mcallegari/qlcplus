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
#include <QElapsedTimer>
#include <QDebug>
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

#define KXMLQLCRGBMatrixControlMode "ControlMode"
#define KXMLQLCRGBMatrixControlModeRgb "RGB"
#define KXMLQLCRGBMatrixControlModeAmber "Amber"
#define KXMLQLCRGBMatrixControlModeWhite "White"
#define KXMLQLCRGBMatrixControlModeUV "UV"
#define KXMLQLCRGBMatrixControlModeDimmer "Dimmer"
#define KXMLQLCRGBMatrixControlModeShutter "Shutter"

/****************************************************************************
 * Initialization
 ****************************************************************************/

RGBMatrix::RGBMatrix(Doc* doc)
    : Function(doc, Function::RGBMatrixType)
    , m_dimmerControl(false)
    , m_fixtureGroupID(FixtureGroup::invalidId())
    , m_group(NULL)
    , m_algorithm(NULL)
    , m_algorithmMutex(QMutex::Recursive)
    , m_startColor(Qt::red)
    , m_endColor(QColor())
    , m_stepHandler(new RGBMatrixStep())
    , m_roundTime(new QElapsedTimer())
    , m_stepsCount(0)
    , m_stepBeatDuration(0)
    , m_controlMode(RGBMatrix::ControlModeRgb)
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
    delete m_stepHandler;
}

QIcon RGBMatrix::getIcon() const
{
    return QIcon(":/rgbmatrix.png");
}

void RGBMatrix::setTotalDuration(quint32 msec)
{
    QMutexLocker algorithmLocker(&m_algorithmMutex);

    if (m_algorithm == NULL)
        return;

    FixtureGroup* grp = doc()->fixtureGroup(fixtureGroup());
    if (grp == NULL)
        return;

    int steps = m_algorithm->rgbMapStepCount(grp->size());
    setDuration(msec / steps);
}

quint32 RGBMatrix::totalDuration()
{
    QMutexLocker algorithmLocker(&m_algorithmMutex);

    if (m_algorithm == NULL)
        return 0;

    FixtureGroup* grp = doc()->fixtureGroup(fixtureGroup());
    if (grp == NULL)
        return 0;

    qDebug () << "Algorithm steps:" << m_algorithm->rgbMapStepCount(grp->size());
    return m_algorithm->rgbMapStepCount(grp->size()) * duration();
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

quint32 RGBMatrix::fixtureGroup() const
{
    return m_fixtureGroupID;
}

void RGBMatrix::setFixtureGroup(quint32 id)
{
    m_fixtureGroupID = id;
    {
        QMutexLocker algoLocker(&m_algorithmMutex);
        m_group = doc()->fixtureGroup(m_fixtureGroupID);
    }
    m_stepsCount = stepsCount();
}

QList<quint32> RGBMatrix::components()
{
    if (m_group != NULL)
        return m_group->fixtureList();

    return QList<quint32>();
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

        /** If there's been a change of Script algorithm "on the fly",
         *  then re-apply the properties currently set in this RGBMatrix */
        if (m_algorithm != NULL && m_algorithm->type() == RGBAlgorithm::Script)
        {
            RGBScript *script = static_cast<RGBScript*> (m_algorithm);
            QHashIterator<QString, QString> it(m_properties);
            while(it.hasNext())
            {
                it.next();
                if (script->setProperty(it.key(), it.value()) == false)
                {
                    /** If the new algorithm doesn't expose a property,
                     *  then remove it from the cached list, otherwise
                     *  it would be carried around forever (and saved on XML) */
                    m_properties.take(it.key());
                }
            }
        }
    }
    m_stepsCount = stepsCount();

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

void RGBMatrix::previewMap(int step, RGBMatrixStep *handler)
{
    QMutexLocker algorithmLocker(&m_algorithmMutex);
    if (m_algorithm == NULL || handler == NULL)
        return;

    if (m_group == NULL)
        m_group = doc()->fixtureGroup(fixtureGroup());

    if (m_group != NULL)
        m_algorithm->rgbMap(m_group->size(), handler->stepColor().rgb(), step, handler->m_map);
}

/****************************************************************************
 * Color
 ****************************************************************************/

void RGBMatrix::setStartColor(const QColor& c)
{
    m_startColor = c;
    {
        QMutexLocker algorithmLocker(&m_algorithmMutex);
        if (m_algorithm != NULL)
        {
            m_algorithm->setColors(m_startColor, m_endColor);
            updateColorDelta();
        }
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
        {
            m_algorithm->setColors(m_startColor, m_endColor);
            updateColorDelta();
        }
    }
    emit changed(id());
}

QColor RGBMatrix::endColor() const
{
    return m_endColor;
}

void RGBMatrix::updateColorDelta()
{
    m_stepHandler->calculateColorDelta(m_startColor, m_endColor);
}

/************************************************************************
 * Properties
 ************************************************************************/

void RGBMatrix::setProperty(QString propName, QString value)
{
    QMutexLocker algoLocker(&m_algorithmMutex);
    m_properties[propName] = value;
    if (m_algorithm != NULL && m_algorithm->type() == RGBAlgorithm::Script)
    {
        RGBScript *script = static_cast<RGBScript*> (m_algorithm);
        script->setProperty(propName, value);
    }
    m_stepsCount = stepsCount();
}

QString RGBMatrix::property(QString propName)
{
    QMutexLocker algoLocker(&m_algorithmMutex);

    /** If the property is cached, then return it right away */
    if (m_properties.contains(propName))
        return m_properties[propName];

    /** Otherwise, let's retrieve it from the Script */
    if (m_algorithm != NULL && m_algorithm->type() == RGBAlgorithm::Script)
    {
        RGBScript *script = static_cast<RGBScript*> (m_algorithm);
        return script->property(propName);
    }

    return QString();
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

    if (root.attributes().value(KXMLQLCFunctionType).toString() != typeToString(Function::RGBMatrixType))
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
        else if (root.name() == KXMLQLCRGBMatrixControlMode)
        {
            setControlMode(stringToControlMode(root.readElementText()));
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

    /* LEGACY - Dimmer Control */
    if (dimmerControl())
        doc->writeTextElement(KXMLQLCRGBMatrixDimmerControl, QString::number(dimmerControl()));

    /* Start Color */
    doc->writeTextElement(KXMLQLCRGBMatrixStartColor, QString::number(startColor().rgb()));

    /* End Color */
    if (endColor().isValid())
        doc->writeTextElement(KXMLQLCRGBMatrixEndColor, QString::number(endColor().rgb()));

    /* Control Mode */
    doc->writeTextElement(KXMLQLCRGBMatrixControlMode, RGBMatrix::controlModeToString(m_controlMode));

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
            roundCheck();
            resetElapsed();
        }
    }
}

void RGBMatrix::preRun(MasterTimer *timer)
{
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
            // Copy direction from parent class direction
            m_stepHandler->initializeDirection(direction(), m_startColor, m_endColor, m_stepsCount);

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

    m_roundTime->restart();

    Function::preRun(timer);
}

void RGBMatrix::write(MasterTimer *timer, QList<Universe *> universes)
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
            // Get a new map every time elapsed is reset to zero
            if (elapsed() < MasterTimer::tick())
            {
                if (tempoType() == Beats)
                    m_stepBeatDuration = beatsToTime(duration(), timer->beatTimeDuration());

                //qDebug() << "RGBMatrix step" << m_stepHandler->currentStepIndex() << ", color:" << QString::number(m_stepHandler->stepColor().rgb(), 16);
                m_algorithm->rgbMap(m_group->size(), m_stepHandler->stepColor().rgb(),
                                    m_stepHandler->currentStepIndex(), m_stepHandler->m_map);
                updateMapChannels(m_stepHandler->m_map, m_group, universes);
            }
        }
    }

    if (isPaused() == false)
    {
        // Increment the ms elapsed time
        incrementElapsed();

        /* Check if we need to change direction, stop completely or go to next step
         * The cases are:
         * 1- time tempo type: act normally, on ms elapsed time
         * 2- beat tempo type, beat occurred: check if the elapsed beats is a multiple of
         *    the step beat duration. If so, proceed to the next step
         * 3- beat tempo type, not beat: if the ms elapsed time reached the step beat
         *    duration in ms, and the ms time to the next beat is not less than 1/16 of
         *    the step beat duration in ms, then proceed to the next step. If the ms time to the
         *    next beat is less than 1/16 of the step beat duration in ms, then defer the step
         *    change to case #2, to resync the matrix to the next beat
         */
        if (tempoType() == Time && elapsed() >= duration())
        {
            roundCheck();
        }
        else if (tempoType() == Beats)
        {
            if (timer->isBeat())
            {
                incrementElapsedBeats();
                qDebug() << "Elapsed beats:" << elapsedBeats() << ", time elapsed:" << elapsed() << ", step time:" << m_stepBeatDuration;
                if (elapsedBeats() % duration() == 0)
                {
                    roundCheck();
                    resetElapsed();
                }
            }
            else if (elapsed() >= m_stepBeatDuration && (uint)timer->timeToNextBeat() > m_stepBeatDuration / 16)
            {
                qDebug() << "Elapsed exceeded";
                roundCheck();
            }
        }
    }
}

void RGBMatrix::postRun(MasterTimer *timer, QList<Universe *> universes)
{
    uint fadeout = overrideFadeOutSpeed() == defaultSpeed() ? fadeOutSpeed() : overrideFadeOutSpeed();

    /* If no fade out is needed, dismiss all the requested faders.
     * Otherwise, set all the faders to fade out and let Universe dismiss them
     * when done */
    if (fadeout == 0)
    {
        dismissAllFaders();
    }
    else
    {
        if (tempoType() == Beats)
            fadeout = beatsToTime(fadeout, timer->beatTimeDuration());

        foreach (QSharedPointer<GenericFader> fader, m_fadersMap.values())
        {
            if (!fader.isNull())
                fader->setFadeOut(true, fadeout);
        }
    }

    m_fadersMap.clear();

    {
        QMutexLocker algorithmLocker(&m_algorithmMutex);
        if (m_algorithm != NULL)
            m_algorithm->postRun();
    }

    Function::postRun(timer, universes);
}

void RGBMatrix::roundCheck()
{
    QMutexLocker algorithmLocker(&m_algorithmMutex);
    if (m_algorithm == NULL)
        return;

    if (m_stepHandler->checkNextStep(runOrder(), m_startColor, m_endColor, m_stepsCount) == false)
        stop(FunctionParent::master());

    m_roundTime->restart();

    if (tempoType() == Beats)
        roundElapsed(m_stepBeatDuration);
    else
        roundElapsed(duration());
}

FadeChannel *RGBMatrix::getFader(QList<Universe *> universes, quint32 universeID, quint32 fixtureID, quint32 channel)
{
    // get the universe Fader first. If doesn't exist, create it
    QSharedPointer<GenericFader> fader = m_fadersMap.value(universeID, QSharedPointer<GenericFader>());
    if (fader.isNull())
    {
        fader = universes[universeID]->requestFader();
        fader->adjustIntensity(getAttributeValue(Intensity));
        fader->setBlendMode(blendMode());
        fader->setName(name());
        fader->setParentFunctionID(id());
        m_fadersMap[universeID] = fader;
    }

    return fader->getChannelFader(doc(), universes[universeID], fixtureID, channel);
}

void RGBMatrix::updateFaderValues(FadeChannel *fc, uchar value, uint fadeTime)
{
    fc->setStart(fc->current());
    fc->setTarget(value);
    fc->setElapsed(0);
    fc->setReady(false);
    // fade in/out depends on target value
    if (value == 0)
        fc->setFadeTime(fadeOutSpeed());
    else
        fc->setFadeTime(fadeTime);
}

void RGBMatrix::updateMapChannels(const RGBMap& map, const FixtureGroup *grp, QList<Universe *> universes)
{
    uint fadeTime = (overrideFadeInSpeed() == defaultSpeed()) ? fadeInSpeed() : overrideFadeInSpeed();

    // Create/modify fade channels for ALL heads in the group
    QMapIterator<QLCPoint, GroupHead> it(grp->headsMap());
    while (it.hasNext())
    {
        it.next();
        QLCPoint pt = it.key();
        GroupHead grpHead = it.value();
        Fixture *fxi = doc()->fixture(grpHead.fxi);
        if (fxi == NULL)
            continue;

        QLCFixtureHead head = fxi->head(grpHead.head);

        if (pt.y() >= map.count() || pt.x() >= map[pt.y()].count())
            continue;

        uint col = map[pt.y()][pt.x()];

        if (m_controlMode == ControlModeRgb)
        {
            QVector <quint32> rgb = head.rgbChannels();
            QVector <quint32> cmy = head.cmyChannels();

            if (rgb.size() == 3)
            {
                // RGB color mixing
                FadeChannel *fc = getFader(universes, fxi->universe(), grpHead.fxi, rgb.at(0));
                updateFaderValues(fc, qRed(col), fadeTime);

                fc = getFader(universes, fxi->universe(), grpHead.fxi, rgb.at(1));
                updateFaderValues(fc, qGreen(col), fadeTime);

                fc = getFader(universes, fxi->universe(), grpHead.fxi, rgb.at(2));
                updateFaderValues(fc, qBlue(col), fadeTime);
            }
            else if (cmy.size() == 3)
            {
                // CMY color mixing
                QColor cmyCol(col);

                FadeChannel *fc = getFader(universes, fxi->universe(), grpHead.fxi, cmy.at(0));
                updateFaderValues(fc, cmyCol.cyan(), fadeTime);

                fc = getFader(universes, fxi->universe(), grpHead.fxi, cmy.at(1));
                updateFaderValues(fc, cmyCol.magenta(), fadeTime);

                fc = getFader(universes, fxi->universe(), grpHead.fxi, cmy.at(2));
                updateFaderValues(fc, cmyCol.yellow(), fadeTime);
            }
        }
        else if (m_controlMode == ControlModeWhite)
        {
            quint32 white = head.channelNumber(QLCChannel::White, QLCChannel::MSB);

            if (white != QLCChannel::invalid())
            {
                FadeChannel *fc = getFader(universes, fxi->universe(), grpHead.fxi, white);
                updateFaderValues(fc, rgbToGrey(col), fadeTime);
            }
        }
        else if (m_controlMode == ControlModeAmber)
        {
            quint32 amber = head.channelNumber(QLCChannel::Amber, QLCChannel::MSB);

            if (amber != QLCChannel::invalid())
            {
                FadeChannel *fc = getFader(universes, fxi->universe(), grpHead.fxi, amber);
                updateFaderValues(fc, rgbToGrey(col), fadeTime);
            }
        }
        else if (m_controlMode == ControlModeUV)
        {
            quint32 uv = head.channelNumber(QLCChannel::UV, QLCChannel::MSB);

            if (uv != QLCChannel::invalid())
            {
                FadeChannel *fc = getFader(universes, fxi->universe(), grpHead.fxi, uv);
                updateFaderValues(fc, rgbToGrey(col), fadeTime);
            }
        }
        else if (m_controlMode == ControlModeShutter)
        {
            QVector <quint32> shutters = head.shutterChannels();

            if (shutters.size())
            {
                FadeChannel *fc = getFader(universes, fxi->universe(), grpHead.fxi, shutters.first());
                updateFaderValues(fc, rgbToGrey(col), fadeTime);
            }
        }

        if (m_controlMode == ControlModeDimmer || m_dimmerControl)
        {
            quint32 masterDim = fxi->masterIntensityChannel();
            quint32 headDim = head.channelNumber(QLCChannel::Intensity, QLCChannel::MSB);
            QVector <quint32> dimmers;

            // Collect all dimmers that affect current head:
            // They are the master dimmer (affects whole fixture)
            // and per-head dimmer.
            //
            // If there are no RGB or CMY channels, the least important* dimmer channel
            // is used to create grayscale image.
            //
            // The rest of the dimmer channels are set to full if dimmer control is
            // enabled and target color is > 0 (see
            // http://www.qlcplus.org/forum/viewtopic.php?f=29&t=11090)
            //
            // Note: If there is only one head, and only one dimmer channel,
            // make it a master dimmer in fixture definition.
            //
            // *least important - per head dimmer if present,
            // otherwise per fixture dimmer if present

            if (masterDim != QLCChannel::invalid())
                dimmers << masterDim;

            if (headDim != QLCChannel::invalid())
                dimmers << headDim;

            if (dimmers.size())
            {
                // Set dimmer to value of the color (e.g. for PARs)
                FadeChannel *fc = getFader(universes, fxi->universe(), grpHead.fxi, dimmers.last());
                updateFaderValues(fc, rgbToGrey(col), fadeTime);
                dimmers.pop_back();
            }

            // Set the rest of the dimmer channels to full on
            foreach (quint32 ch, dimmers)
            {
                FadeChannel *fc = getFader(universes, fxi->universe(), grpHead.fxi, ch);
                updateFaderValues(fc, col == 0 ? 0 : 255, fadeTime);
            }
        }
    }
}

uchar RGBMatrix::rgbToGrey(uint col)
{
    // the weights are taken from
    // https://en.wikipedia.org/wiki/YUV#SDTV_with_BT.601
    return (0.299 * qRed(col) + 0.587 * qGreen(col) + 0.114 * qBlue(col));
}

/*********************************************************************
 * Attributes
 *********************************************************************/

int RGBMatrix::adjustAttribute(qreal fraction, int attributeId)
{
    int attrIndex = Function::adjustAttribute(fraction, attributeId);

    if (attrIndex == Intensity)
    {
        foreach (QSharedPointer<GenericFader> fader, m_fadersMap.values())
        {
            if (!fader.isNull())
                fader->adjustIntensity(getAttributeValue(Function::Intensity));
        }
    }

    return attrIndex;
}

/*************************************************************************
 * Blend mode
 *************************************************************************/

void RGBMatrix::setBlendMode(Universe::BlendMode mode)
{
    if (mode == blendMode())
        return;

    foreach (QSharedPointer<GenericFader> fader, m_fadersMap.values())
    {
        if (!fader.isNull())
            fader->setBlendMode(mode);
    }

    Function::setBlendMode(mode);
    emit changed(id());
}

/*************************************************************************
 * Control Mode
 *************************************************************************/

RGBMatrix::ControlMode RGBMatrix::controlMode() const
{
    return m_controlMode;
}

void RGBMatrix::setControlMode(RGBMatrix::ControlMode mode)
{
    m_controlMode = mode;
    emit changed(id());
}

RGBMatrix::ControlMode RGBMatrix::stringToControlMode(QString mode)
{
    if (mode == KXMLQLCRGBMatrixControlModeRgb)
        return ControlModeRgb;
    else if (mode == KXMLQLCRGBMatrixControlModeAmber)
        return ControlModeAmber;
    else if (mode == KXMLQLCRGBMatrixControlModeWhite)
        return ControlModeWhite;
    else if (mode == KXMLQLCRGBMatrixControlModeUV)
        return ControlModeUV;
    else if (mode == KXMLQLCRGBMatrixControlModeDimmer)
        return ControlModeDimmer;
    else if (mode == KXMLQLCRGBMatrixControlModeShutter)
        return ControlModeShutter;

    return ControlModeRgb;
}

QString RGBMatrix::controlModeToString(RGBMatrix::ControlMode mode)
{
    switch(mode)
    {
        default:
        case ControlModeRgb:
            return QString(KXMLQLCRGBMatrixControlModeRgb);
        break;
        case ControlModeAmber:
            return QString(KXMLQLCRGBMatrixControlModeAmber);
        break;
        case ControlModeWhite:
            return QString(KXMLQLCRGBMatrixControlModeWhite);
        break;
        case ControlModeUV:
            return QString(KXMLQLCRGBMatrixControlModeUV);
        break;
        case ControlModeDimmer:
            return QString(KXMLQLCRGBMatrixControlModeDimmer);
        break;
        case ControlModeShutter:
            return QString(KXMLQLCRGBMatrixControlModeShutter);
        break;
    }
}


/*************************************************************************
 *************************************************************************
 *                          RGBMatrixStep class
 *************************************************************************
 *************************************************************************/

RGBMatrixStep::RGBMatrixStep()
    : m_direction(Function::Forward)
    , m_currentStepIndex(0)
    , m_stepColor(QColor())
    , m_crDelta(0)
    , m_cgDelta(0)
    , m_cbDelta(0)
{

}

void RGBMatrixStep::setCurrentStepIndex(int index)
{
    m_currentStepIndex = index;
}

int RGBMatrixStep::currentStepIndex() const
{
    return m_currentStepIndex;
}

void RGBMatrixStep::calculateColorDelta(QColor startColor, QColor endColor)
{
    m_crDelta = 0;
    m_cgDelta = 0;
    m_cbDelta = 0;

    if (endColor.isValid())
    {
        m_crDelta = endColor.red() - startColor.red();
        m_cgDelta = endColor.green() - startColor.green();
        m_cbDelta = endColor.blue() - startColor.blue();

        //qDebug() << "Color deltas:" << m_crDelta << m_cgDelta << m_cbDelta;
    }
}

void RGBMatrixStep::setStepColor(QColor color)
{
    m_stepColor = color;
}

QColor RGBMatrixStep::stepColor()
{
    return m_stepColor;
}

void RGBMatrixStep::updateStepColor(int stepIndex, QColor startColor, int stepsCount)
{
    if (stepsCount <= 0)
        return;

    if (stepsCount == 1)
    {
        m_stepColor = startColor;
    }
    else
    {
        m_stepColor.setRed(startColor.red() + (m_crDelta * stepIndex / (stepsCount - 1)));
        m_stepColor.setGreen(startColor.green() + (m_cgDelta * stepIndex / (stepsCount - 1)));
        m_stepColor.setBlue(startColor.blue() + (m_cbDelta * stepIndex / (stepsCount - 1)));
    }

    //qDebug() << "RGBMatrix step" << stepIndex << ", color:" << QString::number(m_stepColor.rgb(), 16);
}

void RGBMatrixStep::initializeDirection(Function::Direction direction, QColor startColor, QColor endColor, int stepsCount)
{
    m_direction = direction;

    if (m_direction == Function::Forward)
    {
        setCurrentStepIndex(0);
        setStepColor(startColor);
    }
    else
    {
        setCurrentStepIndex(stepsCount - 1);

        if (endColor.isValid())
            setStepColor(endColor);
        else
            setStepColor(startColor);
    }

    calculateColorDelta(startColor, endColor);
}

bool RGBMatrixStep::checkNextStep(Function::RunOrder order,
                                  QColor startColor, QColor endColor, int stepsNumber)
{
    if (order == Function::PingPong)
    {
        if (m_direction == Function::Forward && (m_currentStepIndex + 1) == stepsNumber)
        {
            m_direction = Function::Backward;
            m_currentStepIndex = stepsNumber - 2;
            if (endColor.isValid())
                m_stepColor = endColor;

            updateStepColor(m_currentStepIndex, startColor, stepsNumber);
        }
        else if (m_direction == Function::Backward && (m_currentStepIndex - 1) < 0)
        {
            m_direction = Function::Forward;
            m_currentStepIndex = 1;
            m_stepColor = startColor;
            updateStepColor(m_currentStepIndex, startColor, stepsNumber);
        }
        else
        {
            if (m_direction == Function::Forward)
                m_currentStepIndex++;
            else
                m_currentStepIndex--;
            updateStepColor(m_currentStepIndex, startColor, stepsNumber);
        }
    }
    else if (order == Function::SingleShot)
    {
        if (m_direction == Function::Forward)
        {
            if (m_currentStepIndex >= stepsNumber - 1)
                return false;
            else
            {
                m_currentStepIndex++;
                updateStepColor(m_currentStepIndex, startColor, stepsNumber);
            }
        }
        else
        {
            if (m_currentStepIndex <= 0)
                return false;
            else
            {
                m_currentStepIndex--;
                updateStepColor(m_currentStepIndex, startColor, stepsNumber);
            }
        }
    }
    else
    {
        if (m_direction == Function::Forward)
        {
            if (m_currentStepIndex >= stepsNumber - 1)
            {
                m_currentStepIndex = 0;
                m_stepColor = startColor;
            }
            else
            {
                m_currentStepIndex++;
                updateStepColor(m_currentStepIndex, startColor, stepsNumber);
            }
        }
        else
        {
            if (m_currentStepIndex <= 0)
            {
                m_currentStepIndex = stepsNumber - 1;
                if (endColor.isValid())
                    m_stepColor = endColor;
            }
            else
            {
                m_currentStepIndex--;
                updateStepColor(m_currentStepIndex, startColor, stepsNumber);
            }
        }
    }

    return true;
}

