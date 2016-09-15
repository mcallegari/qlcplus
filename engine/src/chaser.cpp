/*
  Q Light Controller Plus
  chaser.cpp

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
#include <QColor>
#include <QFile>

#include "qlcfixturedef.h"
#include "qlcfile.h"

#include "chaserrunner.h"
#include "mastertimer.h"
#include "chaserstep.h"
#include "function.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"
#include "bus.h"

#define KXMLQLCChaserTimingModes "SpeedModes"
#define KXMLQLCChaserTimingModeCommon "Common"
#define KXMLQLCChaserTimingModePerStep "PerStep"
#define KXMLQLCChaserTimingModeDefault "Default"
#define KXMLQLCChaserSequenceTag "Sequence"
#define KXMLQLCChaserSequenceBoundScene "BoundScene"
#define KXMLQLCChaserSequenceStartTime "StartTime"
#define KXMLQLCChaserSequenceColor "Color"
#define KXMLQLCChaserSequenceLocked "Locked"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Chaser::Chaser(Doc* doc)
    : Function(doc, Function::Chaser)
    , m_legacyHoldBus(Bus::invalid())
    , m_isSequence(false)
    , m_boundSceneID(-1)
    , m_startTime(UINT_MAX)
    , m_color(85, 107, 128)
    , m_locked(false)
    , m_fadeInMode(Default)
    , m_fadeOutMode(Default)
    , m_durationMode(Common)
    , m_startStepIndex(-1)
    , m_hasStartIntensity(false)
    , m_runnerMutex(QMutex::Recursive)
    , m_runner(NULL)
{
    setName(tr("New Chaser"));

    // Listen to member Function removals
    connect(doc, SIGNAL(functionRemoved(quint32)),
            this, SLOT(slotFunctionRemoved(quint32)));
}

Chaser::~Chaser()
{
}

/*****************************************************************************
 * Copying
 *****************************************************************************/

Function* Chaser::createCopy(Doc* doc, bool addToDoc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new Chaser(doc);
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

bool Chaser::copyFrom(const Function* function)
{
    const Chaser* chaser = qobject_cast<const Chaser*> (function);
    if (chaser == NULL)
        return false;

    // Copy chaser stuff
    m_steps = chaser->m_steps;
    m_fadeInMode = chaser->m_fadeInMode;
    m_fadeOutMode = chaser->m_fadeOutMode;
    m_durationMode = chaser->m_durationMode;
    m_isSequence = chaser->m_isSequence;
    m_boundSceneID = chaser->m_boundSceneID;
    m_startTime = chaser->m_startTime;
    m_color = chaser->m_color;

    // Copy common function stuff
    return Function::copyFrom(function);
}

/*****************************************************************************
 * Sorting
 *****************************************************************************/
bool Chaser::operator<(const Chaser& chs) const
{
    if (m_startTime < chs.getStartTime())
        return true;
    else
        return false;
}

/*****************************************************************************
 * Contents
 *****************************************************************************/

bool Chaser::addStep(const ChaserStep& step, int index)
{
    if (step.fid != this->id())
    {
        {
            QMutexLocker stepListLocker(&m_stepListMutex);
            if (index < 0)
                m_steps.append(step);
            else if (index <= m_steps.size())
                m_steps.insert(index, step);
        }

        emit changed(this->id());
        return true;
    }
    else
    {
        return false;
    }
}

bool Chaser::removeStep(int index)
{
    if (index >= 0 && index < m_steps.size())
    {
        {
            QMutexLocker stepListLocker(&m_stepListMutex);
            m_steps.removeAt(index);
        }

        emit changed(this->id());
        return true;
    }
    else
    {
        return false;
    }
}

bool Chaser::replaceStep(const ChaserStep& step, int index)
{
    if (index >= 0 && index < m_steps.size())
    {
        {
            QMutexLocker stepListLocker(&m_stepListMutex);
            m_steps[index] = step;
        }

        emit changed(this->id());
        return true;
    }
    else
    {
        return false;
    }
}

bool Chaser::moveStep(int sourceIdx, int destIdx)
{
    if (sourceIdx < 0 || sourceIdx >= m_steps.size())
        return false;
    if (destIdx < 0 || destIdx >= m_steps.size() || destIdx == sourceIdx)
        return false;

    {
        QMutexLocker stepListLocker(&m_stepListMutex);
        ChaserStep cs = m_steps[sourceIdx];
        m_steps.removeAt(sourceIdx);
        m_steps.insert(destIdx, cs);
    }

    emit changed(this->id());

    return true;
}

int Chaser::stepsCount()
{
    return m_steps.count();
}

ChaserStep Chaser::stepAt(int idx)
{
    if (idx >= 0 && idx < m_steps.count())
        return m_steps.at(idx);
    return ChaserStep();
}

QList <ChaserStep> Chaser::steps() const
{
    return m_steps;
}

void Chaser::slotFunctionRemoved(quint32 fid)
{
    int count;
    {
        QMutexLocker stepListLocker(&m_stepListMutex);
        count = m_steps.removeAll(ChaserStep(fid));
    }

    if (count > 0)
        emit changed(this->id());
}

/*********************************************************************
 * Sequence mode
 *********************************************************************/
void Chaser::enableSequenceMode(quint32 sceneID)
{
    m_isSequence = true;
    m_boundSceneID = sceneID;
    //qDebug() << "[enableSequenceMode] Sequence" << id() << "bound to scene ID:" << m_boundSceneID;
}

bool Chaser::isSequence() const
{
    return m_isSequence;
}

quint32 Chaser::getBoundSceneID() const
{
    return m_boundSceneID;
}

void Chaser::setStartTime(quint32 time)
{
    m_startTime = time;
}

quint32 Chaser::getStartTime() const
{
    return m_startTime;
}

void Chaser::setColor(QColor color)
{
    m_color = color;
}

QColor Chaser::getColor()
{
    return m_color;
}

void Chaser::setLocked(bool locked)
{
    m_locked = locked;
}

bool Chaser::isLocked()
{
    return m_locked;
}

/*****************************************************************************
 * Timings modes
 *****************************************************************************/

void Chaser::setFadeInMode(Chaser::TimingMode mode)
{
    m_fadeInMode = mode;
    emit changed(this->id());
}

Chaser::TimingMode Chaser::fadeInMode() const
{
    return m_fadeInMode;
}

void Chaser::setFadeOutMode(Chaser::TimingMode mode)
{
    m_fadeOutMode = mode;
    emit changed(this->id());
}

Chaser::TimingMode Chaser::fadeOutMode() const
{
    return m_fadeOutMode;
}

void Chaser::setDurationMode(Chaser::TimingMode mode)
{
    m_durationMode = mode;
    emit changed(this->id());
}

Chaser::TimingMode Chaser::durationMode() const
{
    return m_durationMode;
}

QString Chaser::timingModeToString(Chaser::TimingMode mode)
{
    if (mode == Common)
        return KXMLQLCChaserTimingModeCommon;
    else if (mode == PerStep)
        return KXMLQLCChaserTimingModePerStep;
    else
        return KXMLQLCChaserTimingModeDefault;
}

Chaser::TimingMode Chaser::stringToTimingMode(const QString& str)
{
    if (str == KXMLQLCChaserTimingModeCommon)
        return Common;
    else if (str == KXMLQLCChaserTimingModePerStep)
        return PerStep;
    else
        return Default;
}

/*****************************************************************************
 * Save & Load
 *****************************************************************************/

bool Chaser::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    /* Function tag */
    doc->writeStartElement(KXMLQLCFunction);

    /* Common attributes */
    saveXMLCommon(doc);

    /* Timings */
    m_timings.saveXML(doc);

    /* Direction */
    saveXMLDirection(doc);

    /* Run order */
    saveXMLRunOrder(doc);

    /* Timings modes */
    doc->writeStartElement(KXMLQLCChaserTimingModes);
    doc->writeAttribute(KXMLQLCFunctionTimingsFadeIn, timingModeToString(fadeInMode()));
    doc->writeAttribute(KXMLQLCFunctionTimingsFadeOut, timingModeToString(fadeOutMode()));
    doc->writeAttribute(KXMLQLCFunctionTimingsHold, timingModeToString(durationMode()));
    doc->writeEndElement();

    if (m_isSequence == true)
    {
        doc->writeStartElement(KXMLQLCChaserSequenceTag);
        doc->writeAttribute(KXMLQLCChaserSequenceBoundScene, QString::number(m_boundSceneID));
        doc->writeEndElement();
    }

    /* Steps */
    int stepNumber = 0;
    QListIterator <ChaserStep> it(m_steps);
    while (it.hasNext() == true)
    {
        ChaserStep step(it.next());
        step.saveXML(doc, stepNumber++, m_isSequence);
    }

    /* End the <Function> tag */
    doc->writeEndElement();

    return true;
}

bool Chaser::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root.attributes().value(KXMLQLCFunctionType).toString() != typeToString(Function::Chaser))
    {
        qWarning() << Q_FUNC_INFO << root.attributes().value(KXMLQLCFunctionType).toString()
                   << "is not a chaser";
        return false;
    }

    /* Load chaser contents */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCBus)
        {
            m_legacyHoldBus = root.readElementText().toUInt();
        }
        else if (root.name() == KXMLQLCFunctionTimings)
        {
            m_timings.loadXML(root);
        }
        else if (root.name() == KXMLQLCFunctionDirection)
        {
            loadXMLDirection(root);
        }
        else if (root.name() == KXMLQLCFunctionRunOrder)
        {
            loadXMLRunOrder(root);
        }
        else if (root.name() == KXMLQLCChaserTimingModes)
        {
            QXmlStreamAttributes attrs = root.attributes();
            QString str;

            str = attrs.value(KXMLQLCFunctionTimingsFadeIn).toString();
            setFadeInMode(stringToTimingMode(str));

            str = attrs.value(KXMLQLCFunctionTimingsFadeOut).toString();
            setFadeOutMode(stringToTimingMode(str));

            str = attrs.value(KXMLQLCFunctionTimingsHold).toString();
            setDurationMode(stringToTimingMode(str));
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLQLCChaserSequenceTag)
        {
            QXmlStreamAttributes attrs = root.attributes();
            QString str = attrs.value(KXMLQLCChaserSequenceBoundScene).toString();
            enableSequenceMode(str.toUInt());
            if (attrs.hasAttribute(KXMLQLCChaserSequenceStartTime))
                setStartTime(attrs.value(KXMLQLCChaserSequenceStartTime).toString().toUInt());
            if (attrs.hasAttribute(KXMLQLCChaserSequenceColor))
                setColor(QColor(attrs.value(KXMLQLCChaserSequenceColor).toString()));
            if (attrs.hasAttribute(KXMLQLCChaserSequenceLocked))
                setLocked(true);
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLQLCFunctionStep)
        {
            //! @todo stepNumber is useless if the steps are in the wrong order
            ChaserStep step;
            int stepNumber = -1;

            if (step.loadXML(root, stepNumber) == true)
            {
                if (isSequence() == true)
                    step.fid = getBoundSceneID();
                if (stepNumber >= m_steps.size())
                    m_steps.append(step);
                else
                    m_steps.insert(stepNumber, step);
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown chaser tag:" << root.name();
            root.skipCurrentElement();
        }
    }

    return true;
}

void Chaser::postLoad()
{
    if (m_legacyHoldBus != Bus::invalid())
    {
        quint32 value = Bus::instance()->value(m_legacyHoldBus);
        setDuration((value / MasterTimer::frequency()) * 1000);
    }

    Doc* doc = this->doc();
    Q_ASSERT(doc != NULL);

    QMutableListIterator <ChaserStep> it(m_steps);
    while (it.hasNext() == true)
    {
        ChaserStep step(it.next());
        Function* function = doc->function(step.fid);

        if (function == NULL)
            it.remove();
        else if (function->contains(id())) // forbid self-containment
            it.remove();
    }
}

/*****************************************************************************
 * Next/Previous
 * Protected ChaserRunner wrappers
 *****************************************************************************/

void Chaser::tap()
{
    QMutexLocker runnerLocker(&m_runnerMutex);
    if (m_runner != NULL && durationMode() == Common)
        m_runner->tap();
}

void Chaser::setStepIndex(int idx)
{
    QMutexLocker runnerLocker(&m_runnerMutex);
    if (m_runner != NULL)
        m_runner->setCurrentStep(idx, getAttributeValue(Intensity));
    else
        m_startStepIndex = idx;
}

void Chaser::setStartIntensity(qreal startIntensity)
{
    m_startIntensity = startIntensity;
    m_hasStartIntensity = true;
}

void Chaser::previous()
{
    QMutexLocker runnerLocker(&m_runnerMutex);
    if (m_runner != NULL)
        m_runner->previous();
}

void Chaser::next()
{
    QMutexLocker runnerLocker(&m_runnerMutex);
    if (m_runner != NULL)
        m_runner->next();
}

void Chaser::stopStep(int stepIndex)
{
    QMutexLocker runnerLocker(&m_runnerMutex);
    if (m_runner != NULL)
        m_runner->stopStep(stepIndex);
}

void Chaser::setCurrentStep(int step, qreal intensity)
{
    QMutexLocker runnerLocker(&m_runnerMutex);
    if (m_runner != NULL)
        m_runner->setCurrentStep(step, intensity * getAttributeValue(Intensity));
}

int Chaser::currentStepIndex() const
{
    int ret = m_startStepIndex;
    {
        QMutexLocker runnerLocker(const_cast<QMutex*>(&m_runnerMutex));
        if (m_runner != NULL)
            ret = m_runner->currentStepIndex();
    }
    return ret;
}

int Chaser::computeNextStep(int currentStepIndex) const
{
    int ret = m_startStepIndex;
    {
        QMutexLocker runnerLocker(const_cast<QMutex*>(&m_runnerMutex));
        if (m_runner != NULL)
            ret = m_runner->computeNextStep(currentStepIndex);
    }
    return ret;
}

int Chaser::runningStepsNumber() const
{
    int ret = 0;
    {
        QMutexLocker runnerLocker(const_cast<QMutex*>(&m_runnerMutex));
        if (m_runner != NULL)
            ret = m_runner->runningStepsNumber();
    }
    return ret;
}

ChaserRunnerStep Chaser::currentRunningStep() const
{
    ChaserRunnerStep ret;
    ret.m_function = NULL;
    {
        QMutexLocker runnerLocker(const_cast<QMutex*>(&m_runnerMutex));
        if (m_runner != NULL)
        {
            ChaserRunnerStep* step = m_runner->currentRunningStep();
            if (step != NULL)
            {
                ret = *step;
            }
        }
    }
    return ret;
}

void Chaser::adjustIntensity(qreal fraction, int stepIndex)
{
    QMutexLocker runnerLocker(&m_runnerMutex);
    if (m_runner != NULL)
        m_runner->adjustIntensity(fraction * getAttributeValue(Intensity), stepIndex);
}

bool Chaser::contains(quint32 functionId)
{
    Doc* doc = this->doc();
    Q_ASSERT(doc != NULL);

    foreach(ChaserStep step, m_steps)
    {
        Function* function = doc->function(step.fid);
        // contains() can be called during init, function may be NULL
        if (function == NULL)
            continue;

        if (function->id() == functionId)
            return true;
        if (function->contains(functionId))
            return true;
    }

    return false;
}

/*****************************************************************************
 * Running
 *****************************************************************************/

void Chaser::createRunner(quint32 startTime, int startStepIdx)
{
    Q_ASSERT(m_runner == NULL);

    {
        QMutexLocker stepListLocker(&m_stepListMutex);
        m_runner = new ChaserRunner(doc(), this, startTime);
    }
    m_runner->moveToThread(QCoreApplication::instance()->thread());
    m_runner->setParent(this);
    if (startStepIdx != -1)
        m_runner->setCurrentStep(startStepIdx);
}

void Chaser::preRun(MasterTimer* timer)
{
    {
        QMutexLocker runnerLocker(&m_runnerMutex);
        createRunner(elapsed(), m_startStepIndex);
        qreal intensity = getAttributeValue(Intensity);
        if (m_hasStartIntensity)
            intensity *= m_startIntensity;
        m_runner->adjustIntensity(intensity);
        m_hasStartIntensity = false;
        m_startStepIndex = -1;
        connect(m_runner, SIGNAL(currentStepChanged(int)), this, SIGNAL(currentStepChanged(int)));
    }

    Function::preRun(timer);
}

void Chaser::setPause(bool enable)
{
    QMutexLocker runnerLocker(&m_runnerMutex);
    if (m_runner != NULL)
        m_runner->setPause(enable);
    Function::setPause(enable);
}

void Chaser::write(MasterTimer* timer, QList<Universe *> universes)
{
    if (isPaused())
        return;

    {
        QMutexLocker runnerLocker(&m_runnerMutex);
        QMutexLocker stepListLocker(&m_stepListMutex);
        Q_ASSERT(m_runner != NULL);

        if (m_runner->write(timer, universes) == false)
            stop(FunctionParent::master());
    }

    incrementElapsed();
}

void Chaser::postRun(MasterTimer* timer, QList<Universe *> universes)
{
    {
        QMutexLocker runnerLocker(&m_runnerMutex);
        Q_ASSERT(m_runner != NULL);
        m_runner->postRun(timer, universes);

        delete m_runner;
        m_runner = NULL;
    }

    Function::postRun(timer, universes);
}

/*****************************************************************************
 * Intensity
 *****************************************************************************/

void Chaser::adjustAttribute(qreal fraction, int attributeIndex)
{
    if (attributeIndex == Intensity)
    {
        QMutexLocker runnerLocker(&m_runnerMutex);
        QMutexLocker stepListLocker(&m_stepListMutex);
        if (m_runner != NULL)
            m_runner->adjustIntensity(fraction);
    }
    Function::adjustAttribute(fraction, attributeIndex);
}
