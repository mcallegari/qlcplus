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

#include "chaserrunner.h"
#include "mastertimer.h"
#include "chaserstep.h"
#include "function.h"
#include "chaser.h"
#include "doc.h"
#include "bus.h"

#define KXMLQLCChaserSpeedModeCommon "Common"
#define KXMLQLCChaserSpeedModePerStep "PerStep"
#define KXMLQLCChaserSpeedModeDefault "Default"

#define KXMLQLCChaserLegacySequence QString("Sequence")

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Chaser::Chaser(Doc *doc)
    : Function(doc, Function::ChaserType)
    , m_legacyHoldBus(Bus::invalid())
    , m_fadeInMode(Default)
    , m_fadeOutMode(Default)
    , m_holdMode(Common)
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    , m_runnerMutex(QMutex::Recursive)
#endif
    , m_runner(NULL)
{
    setName(tr("New Chaser"));

    // Listen to member Function removals
    connect(doc, SIGNAL(functionRemoved(quint32)),
            this, SLOT(slotFunctionRemoved(quint32)));

    m_startupAction.m_action = ChaserNoAction;
    m_startupAction.m_masterIntensity = 1.0;
    m_startupAction.m_stepIntensity = 1.0;
    m_startupAction.m_fadeMode = FromFunction;
    m_startupAction.m_stepIndex = -1;
}

Chaser::~Chaser()
{
}

QIcon Chaser::getIcon() const
{
    return QIcon(":/chaser.png");
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
    const Chaser *chaser = qobject_cast<const Chaser*> (function);
    if (chaser == NULL)
        return false;

    // Copy chaser stuff
    m_steps = chaser->m_steps;
    m_fadeInMode = chaser->m_fadeInMode;
    m_fadeOutMode = chaser->m_fadeOutMode;
    m_holdMode = chaser->m_holdMode;

    // Copy common function stuff
    return Function::copyFrom(function);
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
        emit stepsListChanged(this->id());
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
        emit stepsListChanged(this->id());
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
        emit stepChanged(index);
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

int Chaser::stepsCount() const
{
    return m_steps.count();
}

ChaserStep *Chaser::stepAt(int idx)
{
    if (idx >= 0 && idx < m_steps.count())
        return &(m_steps[idx]);

    return NULL;
}

QList <ChaserStep> Chaser::steps() const
{
    return m_steps;
}

void Chaser::setTotalDuration(quint32 msec)
{
    if (durationMode() == Chaser::Common)
    {
        int stepsCount = m_steps.count();
        if (stepsCount == 0)
            stepsCount = 1;
        setDuration(msec / stepsCount);
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
            if (m_steps[i].hold)
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

    if (durationMode() == Chaser::Common)
        totalDuration = duration() * m_steps.count();
    else
    {
        foreach (ChaserStep step, m_steps)
            totalDuration += step.duration;
    }

    return totalDuration;
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

bool Chaser::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    /* Function tag */
    doc->writeStartElement(KXMLQLCFunction);

    /* Common attributes */
    saveXMLCommon(doc);

    /* Speed */
    saveXMLSpeed(doc);

    /* Direction */
    saveXMLDirection(doc);

    /* Run order */
    saveXMLRunOrder(doc);

    /* Speed modes */
    doc->writeStartElement(KXMLQLCChaserSpeedModes);
    doc->writeAttribute(KXMLQLCFunctionSpeedFadeIn, speedModeToString(fadeInMode()));
    doc->writeAttribute(KXMLQLCFunctionSpeedFadeOut, speedModeToString(fadeOutMode()));
    doc->writeAttribute(KXMLQLCFunctionSpeedDuration, speedModeToString(durationMode()));
    doc->writeEndElement();

    /* Steps */
    for (int i = 0; i < m_steps.count(); i++)
        m_steps.at(i).saveXML(doc, i, false);

    /* End the <Function> tag */
    doc->writeEndElement();

    return true;
}

bool Chaser::loadXMLSpeedModes(QXmlStreamReader &root)
{
    QXmlStreamAttributes attrs = root.attributes();
    QString str;

    str = attrs.value(KXMLQLCFunctionSpeedFadeIn).toString();
    setFadeInMode(stringToSpeedMode(str));

    str = attrs.value(KXMLQLCFunctionSpeedFadeOut).toString();
    setFadeOutMode(stringToSpeedMode(str));

    str = attrs.value(KXMLQLCFunctionSpeedDuration).toString();
    setDurationMode(stringToSpeedMode(str));
    root.skipCurrentElement();

    return true;
}

bool Chaser::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root.attributes().value(KXMLQLCFunctionType).toString() != typeToString(Function::ChaserType))
    {
        qWarning() << Q_FUNC_INFO << root.attributes().value(KXMLQLCFunctionType).toString()
                   << "is not a Chaser";
        return false;
    }

    /* Load chaser contents */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCBus)
        {
            m_legacyHoldBus = root.readElementText().toUInt();
        }
        else if (root.name() == KXMLQLCFunctionSpeed)
        {
            loadXMLSpeed(root);
        }
        else if (root.name() == KXMLQLCFunctionDirection)
        {
            loadXMLDirection(root);
        }
        else if (root.name() == KXMLQLCFunctionRunOrder)
        {
            loadXMLRunOrder(root);
        }
        else if (root.name() == KXMLQLCChaserSpeedModes)
        {
            loadXMLSpeedModes(root);
        }
        else if (root.name() == KXMLQLCFunctionStep)
        {
            //! @todo stepNumber is useless if the steps are in the wrong order
            ChaserStep step;
            int stepNumber = -1;

            if (step.loadXML(root, stepNumber, doc()) == true)
            {
                if (stepNumber >= m_steps.size())
                    m_steps.append(step);
                else
                    m_steps.insert(stepNumber, step);
            }
        }
        else if (root.name() == KXMLQLCChaserLegacySequence)
        {
            doc()->appendToErrorLog(QString("<b>Unsupported sequences found</b>. Please convert your project "
                                            "at <a href=http://www.qlcplus.org/sequence_migration.php>http://www.qlcplus.org/sequence_migration.php</a>"));
            root.skipCurrentElement();
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

    Doc *doc = this->doc();
    Q_ASSERT(doc != NULL);

    QMutableListIterator <ChaserStep> it(m_steps);
    while (it.hasNext() == true)
    {
        ChaserStep step(it.next());
        Function *function = doc->function(step.fid);

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

void Chaser::setAction(ChaserAction &action)
{
    QMutexLocker runnerLocker(&m_runnerMutex);
    if (m_runner != NULL)
    {
        m_runner->setAction(action);
    }
    else
    {
        m_startupAction.m_action = action.m_action;
        m_startupAction.m_stepIndex = action.m_stepIndex;
        m_startupAction.m_masterIntensity = action.m_masterIntensity;
        m_startupAction.m_stepIntensity = action.m_stepIntensity;
        m_startupAction.m_fadeMode = action.m_fadeMode;
    }
}

int Chaser::currentStepIndex() const
{
    int ret = m_startupAction.m_stepIndex;
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        QMutexLocker runnerLocker(const_cast<QMutex*>(&m_runnerMutex));
#else
        QMutexLocker runnerLocker(const_cast<QRecursiveMutex*>(&m_runnerMutex));
#endif
        if (m_runner != NULL)
            ret = m_runner->currentStepIndex();
    }
    return ret;
}

int Chaser::computeNextStep(int currentStepIndex) const
{
    int ret = m_startupAction.m_stepIndex;
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        QMutexLocker runnerLocker(const_cast<QMutex*>(&m_runnerMutex));
#else
        QMutexLocker runnerLocker(const_cast<QRecursiveMutex*>(&m_runnerMutex));
#endif
        if (m_runner != NULL)
            ret = m_runner->computeNextStep(currentStepIndex);
    }
    return ret;
}

int Chaser::runningStepsNumber() const
{
    int ret = 0;
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        QMutexLocker runnerLocker(const_cast<QMutex*>(&m_runnerMutex));
#else
        QMutexLocker runnerLocker(const_cast<QRecursiveMutex*>(&m_runnerMutex));
#endif
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
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        QMutexLocker runnerLocker(const_cast<QMutex*>(&m_runnerMutex));
#else
        QMutexLocker runnerLocker(const_cast<QRecursiveMutex*>(&m_runnerMutex));
#endif
        if (m_runner != NULL)
        {
            ChaserRunnerStep *step = m_runner->currentRunningStep();
            if (step != NULL)
                ret = *step;
        }
    }
    return ret;
}

bool Chaser::contains(quint32 functionId)
{
    Doc *doc = this->doc();
    Q_ASSERT(doc != NULL);

    foreach (ChaserStep step, m_steps)
    {
        Function *function = doc->function(step.fid);
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

QList<quint32> Chaser::components()
{
    QList<quint32> ids;

    foreach (ChaserStep step, m_steps)
        ids.append(step.fid);

    return ids;
}

/*****************************************************************************
 * Running
 *****************************************************************************/

void Chaser::createRunner(quint32 startTime)
{
    Q_ASSERT(m_runner == NULL);

    {
        QMutexLocker stepListLocker(&m_stepListMutex);
        m_runner = new ChaserRunner(doc(), this, startTime);
    }
    m_runner->moveToThread(QCoreApplication::instance()->thread());
    m_runner->setParent(this);
    m_runner->setAction(m_startupAction);
    m_startupAction.m_action = ChaserNoAction;
}

void Chaser::preRun(MasterTimer* timer)
{
    {
        QMutexLocker runnerLocker(&m_runnerMutex);
        createRunner(elapsed());
        connect(m_runner, SIGNAL(currentStepChanged(int)), this, SIGNAL(currentStepChanged(int)));
    }

    Function::preRun(timer);
}

void Chaser::setPause(bool enable)
{
    QMutexLocker runnerLocker(&m_runnerMutex);
    if (m_runner != NULL)
    {
        // request a change of pause state at the next write call
        m_startupAction.m_action = ChaserPauseRequest;
        // use fade mode to pass through enable/disable flag
        m_startupAction.m_fadeMode = enable ? 1 : 0;
    }
    Function::setPause(enable);
}

void Chaser::write(MasterTimer* timer, QList<Universe *> universes)
{
    if (isPaused() && m_startupAction.m_action != ChaserPauseRequest)
        return;

    if (m_startupAction.m_action == ChaserPauseRequest)
    {
        qDebug() << "[Chaser] Request PAUSE" << m_startupAction.m_fadeMode;
        m_runner->setAction(m_startupAction);
        m_startupAction.m_action = ChaserNoAction;
    }

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

        if (isPaused())
            m_runner->setPause(false, universes);

        m_runner->postRun(timer, universes);

        delete m_runner;
        m_runner = NULL;
    }

    Function::postRun(timer, universes);
}

/*****************************************************************************
 * Intensity
 *****************************************************************************/

int Chaser::adjustAttribute(qreal fraction, int attributeId)
{
    int attrIndex = Function::adjustAttribute(fraction, attributeId);

    if (attrIndex == Intensity)
    {
        QMutexLocker runnerLocker(&m_runnerMutex);
        QMutexLocker stepListLocker(&m_stepListMutex);
        if (m_runner != NULL)
        {
            m_runner->adjustStepIntensity(getAttributeValue(Function::Intensity));
        }
        else
        {
            m_startupAction.m_masterIntensity = getAttributeValue(Function::Intensity);
        }
    }

    return attrIndex;
}

void Chaser::adjustStepIntensity(qreal fraction, int stepIndex, FadeControlMode fadeControl)
{
    QMutexLocker runnerLocker(&m_runnerMutex);
    if (m_runner != NULL)
    {
        m_runner->adjustStepIntensity(fraction, stepIndex, fadeControl);
    }
    else
    {
        m_startupAction.m_masterIntensity = getAttributeValue(Function::Intensity);
        m_startupAction.m_stepIntensity = fraction;
    }
}
