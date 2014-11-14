/*
  Q Light Controller
  chaser.cpp

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

#include <QDebug>
#include <QColor>
#include <QFile>
#include <QtXml>

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

#define KXMLQLCChaserSpeedModes "SpeedModes"
#define KXMLQLCChaserSpeedModeCommon "Common"
#define KXMLQLCChaserSpeedModePerStep "PerStep"
#define KXMLQLCChaserSpeedModeDefault "Default"
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
    , m_holdMode(Common)
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
    m_steps.clear();
    m_steps = chaser->m_steps;
    m_fadeInMode = chaser->m_fadeInMode;
    m_fadeOutMode = chaser->m_fadeOutMode;
    m_holdMode = chaser->m_holdMode;
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
        m_stepListMutex.lock();
        if (index < 0)
            m_steps.append(step);
        else if (index <= m_steps.size())
            m_steps.insert(index, step);
        m_stepListMutex.unlock();

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
        m_stepListMutex.lock();
        m_steps.removeAt(index);
        m_stepListMutex.unlock();

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
        m_stepListMutex.lock();
        m_steps[index] = step;
        m_stepListMutex.unlock();

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

    m_stepListMutex.lock();
    ChaserStep cs = m_steps[sourceIdx];
    m_steps.removeAt(sourceIdx);
    m_steps.insert(destIdx, cs);
    m_stepListMutex.unlock();

    emit changed(this->id());

    return true;
}

void Chaser::clear()
{
    m_steps.clear();
    emit changed(this->id());
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

void Chaser::setTotalDuration(quint32 msec)
{
    if (durationMode() == Chaser::Common)
    {
        setDuration(msec / m_steps.count());
    }
    else
    {
        // scale all the Chaser steps to resize
        // to the desired duration
        double dtDuration = (double)totalDuration();
        for (int i = 0; i < m_steps.count(); i++)
        {
            uint origDuration = m_steps[i].duration;
            m_steps[i].duration = ((double)m_steps[i].duration * msec) / dtDuration;
            if(m_steps[i].hold)
                m_steps[i].hold = ((double)m_steps[i].hold * (double)m_steps[i].duration) / (double)origDuration;
            m_steps[i].fadeIn = m_steps[i].duration - m_steps[i].hold;
            if (m_steps[i].fadeOut)
                m_steps[i].fadeOut = ((double)m_steps[i].fadeOut * (double)m_steps[i].duration) / (double)origDuration;
        }
    }
    emit changed(this->id());
}

quint32 Chaser::totalDuration()
{
    quint32 totalDuration = 0;

    foreach (ChaserStep step, m_steps)
    {
        totalDuration += step.duration;
    }

    return totalDuration;
}

void Chaser::slotFunctionRemoved(quint32 fid)
{
    m_stepListMutex.lock();
    int count = m_steps.removeAll(ChaserStep(fid));
    m_stepListMutex.unlock();

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
 * Speed modes
 *****************************************************************************/

void Chaser::setFadeInMode(Chaser::SpeedMode mode)
{
    m_fadeInMode = mode;
    emit changed(this->id());
}

Chaser::SpeedMode Chaser::fadeInMode() const
{
    return m_fadeInMode;
}

void Chaser::setFadeOutMode(Chaser::SpeedMode mode)
{
    m_fadeOutMode = mode;
    emit changed(this->id());
}

Chaser::SpeedMode Chaser::fadeOutMode() const
{
    return m_fadeOutMode;
}

void Chaser::setDurationMode(Chaser::SpeedMode mode)
{
    m_holdMode = mode;
    emit changed(this->id());
}

Chaser::SpeedMode Chaser::durationMode() const
{
    return m_holdMode;
}

QString Chaser::speedModeToString(Chaser::SpeedMode mode)
{
    if (mode == Common)
        return KXMLQLCChaserSpeedModeCommon;
    else if (mode == PerStep)
        return KXMLQLCChaserSpeedModePerStep;
    else
        return KXMLQLCChaserSpeedModeDefault;
}

Chaser::SpeedMode Chaser::stringToSpeedMode(const QString& str)
{
    if (str == KXMLQLCChaserSpeedModeCommon)
        return Common;
    else if (str == KXMLQLCChaserSpeedModePerStep)
        return PerStep;
    else
        return Default;
}

/*****************************************************************************
 * Save & Load
 *****************************************************************************/

bool Chaser::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    QDomElement root;
    //QDomText text;
    //QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    /* Function tag */
    root = doc->createElement(KXMLQLCFunction);
    wksp_root->appendChild(root);

    /* Common attributes */
    saveXMLCommon(&root);

    /* Speed */
    saveXMLSpeed(doc, &root);

    /* Direction */
    saveXMLDirection(doc, &root);

    /* Run order */
    saveXMLRunOrder(doc, &root);

    /* Speed modes */
    QDomElement spd = doc->createElement(KXMLQLCChaserSpeedModes);
    spd.setAttribute(KXMLQLCFunctionSpeedFadeIn, speedModeToString(fadeInMode()));
    spd.setAttribute(KXMLQLCFunctionSpeedFadeOut, speedModeToString(fadeOutMode()));
    spd.setAttribute(KXMLQLCFunctionSpeedDuration, speedModeToString(durationMode()));
    root.appendChild(spd);

    if (m_isSequence == true)
    {
        QDomElement seq = doc->createElement(KXMLQLCChaserSequenceTag);
        seq.setAttribute(KXMLQLCChaserSequenceBoundScene, m_boundSceneID);
        root.appendChild(seq);
    }

    /* Steps */
    int stepNumber = 0;
    QListIterator <ChaserStep> it(m_steps);
    while (it.hasNext() == true)
    {
        ChaserStep step(it.next());
        step.saveXML(doc, &root, stepNumber++, m_isSequence);
    }

    return true;
}

bool Chaser::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root.attribute(KXMLQLCFunctionType) != typeToString(Function::Chaser))
    {
        qWarning() << Q_FUNC_INFO << root.attribute(KXMLQLCFunctionType)
                   << "is not a chaser";
        return false;
    }

    /* Load chaser contents */
    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();

        if (tag.tagName() == KXMLQLCBus)
        {
            m_legacyHoldBus = tag.text().toUInt();
        }
        else if (tag.tagName() == KXMLQLCFunctionSpeed)
        {
            loadXMLSpeed(tag);
        }
        else if (tag.tagName() == KXMLQLCFunctionDirection)
        {
            loadXMLDirection(tag);
        }
        else if (tag.tagName() == KXMLQLCFunctionRunOrder)
        {
            loadXMLRunOrder(tag);
        }
        else if (tag.tagName() == KXMLQLCChaserSpeedModes)
        {
            QString str;

            str = tag.attribute(KXMLQLCFunctionSpeedFadeIn);
            setFadeInMode(stringToSpeedMode(str));

            str = tag.attribute(KXMLQLCFunctionSpeedFadeOut);
            setFadeOutMode(stringToSpeedMode(str));

            str = tag.attribute(KXMLQLCFunctionSpeedDuration);
            setDurationMode(stringToSpeedMode(str));
        }
        else if (tag.tagName() == KXMLQLCChaserSequenceTag)
        {
            QString str = tag.attribute(KXMLQLCChaserSequenceBoundScene);
            enableSequenceMode(str.toUInt());
            if (tag.hasAttribute(KXMLQLCChaserSequenceStartTime))
                setStartTime(tag.attribute(KXMLQLCChaserSequenceStartTime).toUInt());
            if (tag.hasAttribute(KXMLQLCChaserSequenceColor))
                setColor(QColor(tag.attribute(KXMLQLCChaserSequenceColor)));
            if (tag.hasAttribute(KXMLQLCChaserSequenceLocked))
                setLocked(true);
        }
        else if (tag.tagName() == KXMLQLCFunctionStep)
        {
            //! @todo stepNumber is useless if the steps are in the wrong order
            ChaserStep step;
            int stepNumber = -1;

            if (step.loadXML(tag, stepNumber) == true)
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
            qWarning() << Q_FUNC_INFO << "Unknown chaser tag:" << tag.tagName();
        }

        node = node.nextSibling();
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

    QMutableListIterator <ChaserStep> it(m_steps);
    while (it.hasNext() == true)
    {
        ChaserStep step(it.next());
        Function* function = doc()->function(step.fid);
        if (function == NULL)
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
        m_runner->setCurrentStep(idx);
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
        m_runner->setCurrentStep(step, intensity);
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
        m_runner->adjustIntensity(fraction, stepIndex);
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
        if (m_hasStartIntensity)
            m_runner->setCurrentStep(m_startStepIndex, m_startIntensity);
        m_hasStartIntensity = false;
        m_startStepIndex = -1;
        connect(m_runner, SIGNAL(currentStepChanged(int)), this, SIGNAL(currentStepChanged(int)));
    }

    Function::preRun(timer);
}

void Chaser::write(MasterTimer* timer, QList<Universe *> universes)
{
    {
        QMutexLocker runnerLocker(&m_runnerMutex);
        QMutexLocker stepListLocker(&m_stepListMutex);
        Q_ASSERT(m_runner != NULL);

        if (m_runner->write(timer, universes) == false)
            stop();
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
    {
        QMutexLocker runnerLocker(&m_runnerMutex);
        QMutexLocker stepListLocker(&m_stepListMutex);
        if (m_runner != NULL && attributeIndex == Intensity)
            m_runner->adjustIntensity(fraction);
    }
    Function::adjustAttribute(fraction, attributeIndex);
}
