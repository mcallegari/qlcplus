/*
  Q Light Controller Plus
  vccuelist.cpp

  Copyright (c) Massimo Callegari

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

#include "chasereditor.h"
#include "vccuelist.h"
#include "listmodel.h"
#include "qlcmacros.h"
#include "chaser.h"
#include "tardis.h"
#include "qmath.h"

#define INPUT_NEXT_STEP_ID          0
#define INPUT_PREVIOUS_STEP_ID      1
#define INPUT_PLAY_PAUSE_ID         2
#define INPUT_STOP_PAUSE_ID         3
#define INPUT_SIDE_FADER_ID         4

#define PROGRESS_INTERVAL           200

VCCueList::VCCueList(Doc *doc, QObject *parent)
    : VCWidget(doc, parent)
    , m_nextPrevBehavior(DefaultRunFirst)
    , m_playbackLayout(PlayPauseStop)
    , m_slidersMode(None)
    , m_sideFaderLevel(100)
    , m_nextStepIndex(-1)
    , m_primaryTop(true)
    , m_chaserID(Function::invalidId())
    , m_playbackIndex(-1)
    , m_timer(new QTimer())
{
    setType(VCWidget::CueListWidget);

    registerExternalControl(INPUT_NEXT_STEP_ID, tr("Next Cue"), true);
    registerExternalControl(INPUT_PREVIOUS_STEP_ID, tr("Previous Cue"), true);
    registerExternalControl(INPUT_PLAY_PAUSE_ID, tr("Play/Stop/Pause"), true);
    registerExternalControl(INPUT_STOP_PAUSE_ID, tr("Stop/Pause"), true);
    registerExternalControl(INPUT_SIDE_FADER_ID, tr("Side Fader"), false);

    m_stepsList = new ListModel(this);
    QStringList listRoles;
    listRoles << "funcID" << "isSelected" << "fadeIn" << "hold" << "fadeOut" << "duration" << "note";
    m_stepsList->setRoleNames(listRoles);

    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotProgressTimeout()));

    connect(m_doc, SIGNAL(functionRemoved(quint32)),
            this, SLOT(slotFunctionRemoved(quint32)));
    connect(m_doc, SIGNAL(functionNameChanged(quint32)),
            this, SLOT(slotFunctionNameChanged(quint32)));
}

VCCueList::~VCCueList()
{
    if (m_item)
        delete m_item;
}

QString VCCueList::defaultCaption()
{
    return tr("Cue List %1").arg(id() + 1);
}

void VCCueList::setupLookAndFeel(qreal pixelDensity, int page)
{
    setPage(page);
    setDefaultFontSize(pixelDensity * 3.5);
}

void VCCueList::render(QQuickView *view, QQuickItem *parent)
{
    if (view == nullptr || parent == nullptr)
        return;

    QQmlComponent *component = new QQmlComponent(view->engine(), QUrl("qrc:/VCCueListItem.qml"));

    if (component->isError())
    {
        qDebug() << component->errors();
        return;
    }

    m_item = qobject_cast<QQuickItem*>(component->create());

    m_item->setParentItem(parent);
    m_item->setProperty("cueListObj", QVariant::fromValue(this));
}

QString VCCueList::propertiesResource() const
{
    return QString("qrc:/VCCueListProperties.qml");
}

VCWidget *VCCueList::createCopy(VCWidget *parent)
{
    Q_ASSERT(parent != nullptr);

    VCCueList *cuelist = new VCCueList(m_doc, parent);
    if (cuelist->copyFrom(this) == false)
    {
        delete cuelist;
        cuelist = nullptr;
    }

    return cuelist;
}

bool VCCueList::copyFrom(const VCWidget *widget)
{
    const VCCueList *cuelist = qobject_cast<const VCCueList*> (widget);
    if (cuelist == nullptr)
        return false;

    /* Function list contents */
    setChaserID(cuelist->chaserID());

    setPlaybackLayout(cuelist->playbackLayout());
    setNextPrevBehavior(cuelist->nextPrevBehavior());

    /* Common stuff */
    return VCWidget::copyFrom(widget);
}

void VCCueList::adjustIntensity(qreal val)
{
    Chaser *ch = chaser();
    if (ch != nullptr)
    {
        adjustFunctionIntensity(ch, val);

        // Refresh intensity of current steps
        if (!ch->stopped() && sideFaderMode() == Crossfade && m_sideFaderLevel != 100)
        {
                ch->adjustStepIntensity(qreal(m_sideFaderLevel) / 100, m_primaryTop ? m_playbackIndex : m_nextStepIndex);
                ch->adjustStepIntensity(qreal(100 - m_sideFaderLevel) / 100, m_primaryTop ? m_nextStepIndex : m_playbackIndex);
        }
    }

    VCWidget::adjustIntensity(val);
}

/*********************************************************************
 * UI settings
 *********************************************************************/

VCCueList::NextPrevBehavior VCCueList::nextPrevBehavior() const
{
    return m_nextPrevBehavior;
}

void VCCueList::setNextPrevBehavior(NextPrevBehavior nextPrev)
{
    if (m_nextPrevBehavior == nextPrev)
        return;

    Q_ASSERT(nextPrev == DefaultRunFirst
            || nextPrev == RunNext
            || nextPrev == Select
            || nextPrev == Nothing);
    m_nextPrevBehavior = nextPrev;
    emit nextPrevBehaviorChanged();
}

VCCueList::PlaybackLayout VCCueList::playbackLayout() const
{
    return m_playbackLayout;
}

void VCCueList::setPlaybackLayout(VCCueList::PlaybackLayout layout)
{
    if (layout == m_playbackLayout)
        return;

    m_playbackLayout = layout;
    emit playbackLayoutChanged();
}

/*************************************************************************
 * Side fader
 *************************************************************************/
VCCueList::FaderMode VCCueList::sideFaderMode() const
{
    return m_slidersMode;
}

void VCCueList::setSideFaderMode(VCCueList::FaderMode mode)
{
    if (mode == m_slidersMode)
        return;

    m_slidersMode = mode;
    emit sideFaderModeChanged();

    if (mode == Steps)
        setSideFaderLevel(255);
    else if (mode == Crossfade)
        setSideFaderLevel(100);
}

VCCueList::FaderMode VCCueList::stringToFaderMode(QString modeStr)
{
    if (modeStr == "Crossfade")
        return Crossfade;
    else if (modeStr == "Steps")
        return Steps;

    return None;
}

QString VCCueList::faderModeToString(VCCueList::FaderMode mode)
{
    if (mode == Crossfade)
        return "Crossfade";
    else if (mode == Steps)
        return "Steps";

    return "None";
}

int VCCueList::sideFaderLevel() const
{
    return m_sideFaderLevel;
}

void VCCueList::setSideFaderLevel(int level)
{
    if (level == m_sideFaderLevel)
        return;

    m_sideFaderLevel = level;

    if (sideFaderMode() == Steps)
    {
        level = 255 - level;

        Chaser *ch = chaser();
        if (ch == nullptr || ch->stopped())
            return;

        int newStep = level; // by default we assume the Chaser has more than 256 steps
        if (ch->stepsCount() < 256)
        {
            float stepSize = 256 / float(ch->stepsCount()); //divide up the full 0..255 range
            stepSize = qFloor((stepSize * 100000.0) + 0.5) / 100000.0; //round to 5 decimals to fix corner cases
            if (level >= 256.0 - stepSize)
                newStep = ch->stepsCount() - 1;
            else
                newStep = qFloor(qreal(level) / qreal(stepSize));
            //qDebug() << "value:" << value << " new step:" << newStep << " stepSize:" << stepSize;
        }

        ChaserAction action;
        action.m_action = ChaserSetStepIndex;
        action.m_stepIndex = newStep;
        ch->setAction(action);

        if (newStep == ch->currentStepIndex())
            return;
    }
    else
    {
        Chaser *ch = chaser();
        if (!(ch == nullptr || ch->stopped()))
        {
            ch->adjustStepIntensity(qreal(level) / 100.0, m_primaryTop ? m_playbackIndex : m_nextStepIndex,
                                    Chaser::FadeControlMode(getFadeMode()));
            ch->adjustStepIntensity(qreal(100 - level) / 100.0, m_primaryTop ? m_nextStepIndex : m_playbackIndex,
                                    Chaser::FadeControlMode(getFadeMode()));
            stopStepIfNeeded(ch);
        }
    }

    sendFeedback(m_sideFaderLevel, INPUT_SIDE_FADER_ID, VCWidget::ExactValue);

    emit sideFaderLevelChanged();
}

bool VCCueList::primaryTop() const
{
    return m_primaryTop;
}

int VCCueList::nextStepIndex() const
{
    return m_nextStepIndex;
}

qreal VCCueList::getPrimaryIntensity() const
{
    if (sideFaderMode() == FaderMode::None ||
        sideFaderMode() == FaderMode::Steps)
        return 1.0;

    return m_primaryTop ? qreal(m_sideFaderLevel / 100.0) : qreal((100 - m_sideFaderLevel) / 100.0);
}

int VCCueList::getFadeMode() const
{
    if (sideFaderMode() == Steps)
        return Chaser::FromFunction;

    if (m_sideFaderLevel != 0 && m_sideFaderLevel != 100)
        return Chaser::BlendedCrossfade;

    return Chaser::Blended;
}

void VCCueList::stopStepIfNeeded(Chaser *ch)
{
    if (ch->runningStepsNumber() != 2)
        return;

    int primaryValue = m_primaryTop ? m_sideFaderLevel : 100 - m_sideFaderLevel;
    int secondaryValue = m_primaryTop ? 100 - m_sideFaderLevel : m_sideFaderLevel;

    ChaserAction action;
    action.m_action = ChaserStopStep;

    if (primaryValue == 0)
    {
        m_primaryTop = !m_primaryTop;
        action.m_stepIndex = m_playbackIndex;
        ch->setAction(action);
        emit primaryTopChanged();
    }
    else if (secondaryValue == 0)
    {
        action.m_stepIndex = m_nextStepIndex;
        ch->setAction(action);
    }
}

/*********************************************************************
 * Chaser attachment
 *********************************************************************/

FunctionParent VCCueList::functionParent() const
{
    return FunctionParent(FunctionParent::ManualVCWidget, id());
}

Chaser *VCCueList::chaser()
{
    if (m_chaserID == Function::invalidId())
        return nullptr;
    Chaser *chaser = qobject_cast<Chaser*>(m_doc->function(m_chaserID));
    return chaser;
}

QVariant VCCueList::stepsList() const
{
    return QVariant::fromValue(m_stepsList);
}

void VCCueList::addFunctions(QVariantList idsList, int insertIndex)
{
    if (idsList.isEmpty())
        return;

    if (isEditing())
    {
        Chaser *ch = chaser();
        if (ch == nullptr)
            return;

        if (insertIndex == -1)
            insertIndex = ch->stepsCount();

        for (QVariant vID : idsList) // C++11
        {
            quint32 fid = vID.toUInt();
            ChaserStep step(fid);
            if (ch->durationMode() == Chaser::PerStep)
            {
                Function *func = m_doc->function(fid);
                if (func == nullptr)
                    continue;

                step.duration = func->totalDuration();
                if (step.duration == 0)
                    step.duration = 1000;
                step.hold = step.duration;
            }
            Tardis::instance()->enqueueAction(Tardis::ChaserAddStep, ch->id(), QVariant(), insertIndex);
            ch->addStep(step, insertIndex++);
        }

        ChaserEditor::updateStepsList(m_doc, chaser(), m_stepsList);
        emit stepsListChanged();
    }
    else
    {
        Function *f = m_doc->function(idsList.first().toUInt());
        if (f == nullptr || f->type() != Function::ChaserType)
            return;

        setChaserID(f->id());
    }
}

void VCCueList::setStepNote(int index, QString text)
{
    Chaser *ch = chaser();
    if (ch == nullptr)
        return;

    ChaserStep *step = ch->stepAt(index);
    if (step == nullptr)
        return;

    step->note = text;

    QModelIndex mIdx = m_stepsList->index(index, 0, QModelIndex());
    m_stepsList->setDataWithRole(mIdx, "note", text);
}

quint32 VCCueList::chaserID() const
{
    return m_chaserID;
}

void VCCueList::setChaserID(quint32 fid)
{
    bool running = false;

    if (m_chaserID == fid)
        return;

    Function *current = m_doc->function(m_chaserID);
    Function *function = m_doc->function(fid);

    if (current != nullptr)
    {
        /* Get rid of old function connections */
        disconnect(current, SIGNAL(running(quint32)),
                   this, SLOT(slotFunctionRunning(quint32)));
        disconnect(current, SIGNAL(stopped(quint32)),
                   this, SLOT(slotFunctionStopped(quint32)));
        disconnect(current, SIGNAL(currentStepChanged(int)),
                   this, SLOT(slotCurrentStepChanged(int)));
        connect(current, SIGNAL(stepChanged(int)),
                this, SLOT(slotStepChanged(int)));

        if (current->isRunning())
        {
            running = true;
            current->stop(functionParent());
        }
    }

    if (function != nullptr)
    {
        m_chaserID = fid;
        if ((isEditing() && caption().isEmpty()) || caption() == defaultCaption())
            setCaption(function->name());

        ChaserEditor::updateStepsList(m_doc, chaser(), m_stepsList);

        if (running)
        {
            function->start(m_doc->masterTimer(), functionParent());
        }
        /* Connect to the new function */
        connect(function, SIGNAL(running(quint32)),
                this, SLOT(slotFunctionRunning(quint32)));
        connect(function, SIGNAL(stopped(quint32)),
                this, SLOT(slotFunctionStopped(quint32)));
        connect(function, SIGNAL(currentStepChanged(int)),
                this, SLOT(slotCurrentStepChanged(int)));
        connect(function, SIGNAL(stepChanged(int)),
                this, SLOT(slotStepChanged(int)));
        connect(function, SIGNAL(stepsListChanged(quint32)),
                this, SLOT(slotStepsListChanged(quint32)));

        emit chaserIDChanged(fid);
    }
    else
    {
        /* No function attachment */
        m_chaserID = Function::invalidId();
        m_stepsList->clear();
        emit chaserIDChanged(-1);
    }

    emit stepsListChanged();

    Tardis::instance()->enqueueAction(Tardis::VCCueListSetChaserID, id(),
                                      current ? current->id() : Function::invalidId(),
                                      function ? function->id() : Function::invalidId());
}

void VCCueList::slotFunctionRemoved(quint32 fid)
{
    if (fid == m_chaserID)
    {
        m_chaserID = Function::invalidId();
        m_stepsList->clear();
        emit chaserIDChanged(-1);

        resetIntensityOverrideAttribute();
    }
}

void VCCueList::slotStepChanged(int index)
{
    ChaserStep *step = chaser()->stepAt(index);
    ChaserEditor::updateStepInListModel(m_doc, chaser(), m_stepsList, step, index);
}

void VCCueList::slotStepsListChanged(quint32 fid)
{
    if (fid == m_chaserID)
    {
        ChaserEditor::updateStepsList(m_doc, chaser(), m_stepsList);
        emit stepsListChanged();
    }
}

void VCCueList::slotFunctionNameChanged(quint32 fid)
{
    if (fid == m_chaserID)
        emit chaserIDChanged(fid);
}

/*********************************************************************
 * Playback
 *********************************************************************/

int VCCueList::getNextIndex()
{
    Chaser *ch = chaser();
    if (ch == nullptr)
        return -1;

    if (ch->direction() == Function::Forward)
        return m_playbackIndex + 1 == ch->stepsCount() ? 0 : m_playbackIndex + 1;
    else
        return m_playbackIndex == 0 ? ch->stepsCount() - 1 : m_playbackIndex - 1;
}

int VCCueList::getPrevIndex()
{
    Chaser *ch = chaser();
    if (ch == nullptr)
        return -1;

    if (ch->direction() == Function::Forward)
        return m_playbackIndex == 0 ? ch->stepsCount() - 1 : m_playbackIndex - 1;
    else
        return m_playbackIndex + 1 == ch->stepsCount() ? 0 : m_playbackIndex + 1;
}

int VCCueList::getFirstIndex()
{
    Chaser *ch = chaser();
    if (ch == nullptr)
        return -1;

    if (ch->direction() == Function::Forward)
        return 0;
    else
        return ch->stepsCount() - 1;
}

int VCCueList::getLastIndex()
{
    Chaser *ch = chaser();
    if (ch == nullptr)
        return -1;

    if (ch->direction() == Function::Forward)
        return ch->stepsCount() - 1;
    else
        return 0;
}

int VCCueList::playbackIndex() const
{
    return m_playbackIndex;
}

void VCCueList::setPlaybackIndex(int playbackIndex)
{
    if (m_playbackIndex == playbackIndex)
        return;

    m_playbackIndex = playbackIndex;
    emit playbackIndexChanged(playbackIndex);

    Chaser *ch = chaser();
    if (ch == nullptr)
        return;

    m_nextStepIndex = playbackIndex >= 0 ? ch->computeNextStep(playbackIndex) : -1;
    emit nextStepIndexChanged();
}

VCCueList::PlaybackStatus VCCueList::playbackStatus()
{
    Chaser *ch = chaser();

    if (ch == nullptr)
        return Stopped;
    else if (ch->isPaused())
        return Paused;

    return ch->isRunning() ? Playing : Stopped;
}

void VCCueList::startChaser(int startIndex)
{
    Chaser *ch = chaser();
    if (ch == nullptr)
        return;

    adjustFunctionIntensity(ch, intensity());

    ChaserAction action;
    action.m_action = ChaserSetStepIndex;
    action.m_stepIndex = startIndex;
    action.m_masterIntensity = intensity();
    action.m_stepIntensity = getPrimaryIntensity();
    action.m_fadeMode = getFadeMode();
    ch->setAction(action);

    ch->start(m_doc->masterTimer(), functionParent());
    emit functionStarting(this, m_chaserID, intensity());
    emit playbackStatusChanged();
}

void VCCueList::stopChaser()
{
    Chaser *ch = chaser();
    if (ch == nullptr)
        return;

    ch->stop(functionParent());
    resetIntensityOverrideAttribute();
    emit playbackStatusChanged();
}

/*********************************************************************
 * External input
 *********************************************************************/

void VCCueList::updateFeedback()
{
    sendFeedback(m_sideFaderLevel, INPUT_SIDE_FADER_ID, VCWidget::ExactValue);

    Chaser *ch = chaser();
    if (ch == NULL)
        return;

    sendFeedback(ch->isRunning() ? UCHAR_MAX : 0, INPUT_PLAY_PAUSE_ID, VCWidget::ExactValue);
}

void VCCueList::slotInputValueChanged(quint8 id, uchar value)
{
    switch(id)
    {
        case INPUT_NEXT_STEP_ID:
        case INPUT_PREVIOUS_STEP_ID:
        case INPUT_PLAY_PAUSE_ID:
        case INPUT_STOP_PAUSE_ID:
            if (value != UCHAR_MAX)
                return;
        break;
        default:
        break;
    }

    switch(id)
    {
        case INPUT_NEXT_STEP_ID:
            nextClicked();
        break;
        case INPUT_PREVIOUS_STEP_ID:
            previousClicked();
        break;
        case INPUT_PLAY_PAUSE_ID:
            playClicked();
        break;
        case INPUT_STOP_PAUSE_ID:
            stopClicked();
        break;
        case INPUT_SIDE_FADER_ID:
        {
            float val = SCALE(float(value), 0, float(UCHAR_MAX), 0,
                              float(sideFaderMode() == Crossfade ? 100 : 255));
            setSideFaderLevel(int(val));
        }
        break;
    }
}

void VCCueList::playClicked()
{
    Chaser *ch = chaser();
    if (ch == nullptr)
        return;

    if (ch->isRunning())
    {
        if (playbackLayout() == PlayPauseStop)
        {
            ChaserAction action;
            action.m_action = ChaserSetStepIndex;
            action.m_stepIndex = m_playbackIndex;
            action.m_masterIntensity = intensity();
            action.m_stepIntensity = getPrimaryIntensity();
            action.m_fadeMode = getFadeMode();
            ch->setAction(action);

            ch->setPause(!ch->isPaused());
            emit playbackStatusChanged();
        }
        else if (playbackLayout() == PlayStopPause)
        {
            stopChaser();
        }
    }
    else
    {
        startChaser(m_playbackIndex == -1 ? 0 : m_playbackIndex);
    }
}

void VCCueList::stopClicked()
{
    Chaser *ch = chaser();
    if (ch == nullptr)
        return;

    if (ch->isRunning())
    {
        if (playbackLayout() == PlayPauseStop)
        {
            stopChaser();
        }
        else if (playbackLayout() == PlayStopPause)
        {
            ch->setPause(!ch->isPaused());
            emit playbackStatusChanged();
        }
    }
    else
    {
        //m_primaryIndex = 0;
        //m_tree->setCurrentItem(m_tree->topLevelItem(getFirstIndex()));
    }
}

void VCCueList::previousClicked()
{
    Chaser *ch = chaser();
    if (ch == nullptr)
        return;

    if (ch->isRunning())
    {
        if (ch->isPaused())
            setPlaybackIndex(getPrevIndex());
        else
        {
            ChaserAction action;
            action.m_action = ChaserPreviousStep;
            action.m_masterIntensity = intensity();
            action.m_stepIntensity = getPrimaryIntensity();
            action.m_fadeMode = getFadeMode();
            ch->setAction(action);
        }
    }
    else
    {
        switch (m_nextPrevBehavior)
        {
            case DefaultRunFirst:
                startChaser(getLastIndex());
            break;
            case RunNext:
                startChaser(getPrevIndex());
            break;
            case Select:
                setPlaybackIndex(getPrevIndex());
            break;
            case Nothing:
            break;
            default:
                Q_ASSERT(false);
        }
    }
}

void VCCueList::nextClicked()
{
    Chaser *ch = chaser();
    if (ch == nullptr)
        return;

    if (ch->isRunning())
    {
        if (ch->isPaused())
            setPlaybackIndex(getNextIndex());
        else
        {
            ChaserAction action;
            action.m_action = ChaserNextStep;
            action.m_masterIntensity = intensity();
            action.m_stepIntensity = getPrimaryIntensity();
            action.m_fadeMode = getFadeMode();
            ch->setAction(action);
        }
    }
    else
    {
        switch (m_nextPrevBehavior)
        {
            case DefaultRunFirst:
                startChaser(getFirstIndex());
            break;
            case RunNext:
                startChaser(getNextIndex());
            break;
            case Select:
                setPlaybackIndex(getNextIndex());
            break;
            case Nothing:
            break;
            default:
                Q_ASSERT(false);
        }
    }
}

void VCCueList::playCurrentStep()
{
    Chaser *ch = chaser();
    if (ch == nullptr)
        return;

    if (ch->isRunning())
    {
        ChaserAction action;
        action.m_action = ChaserSetStepIndex;
        action.m_stepIndex = m_playbackIndex;
        action.m_masterIntensity = intensity();
        action.m_stepIntensity = getPrimaryIntensity();
        action.m_fadeMode = getFadeMode();
        ch->setAction(action);
    }
    else
    {
        startChaser(m_playbackIndex == -1 ? 0 : m_playbackIndex);
    }

}

void VCCueList::slotFunctionRunning(quint32 fid)
{
    if (fid == m_chaserID)
    {
        emit playbackStatusChanged();
        sendFeedback(UCHAR_MAX, INPUT_PLAY_PAUSE_ID, VCWidget::ExactValue);

        m_timer->start(PROGRESS_INTERVAL);
    }
}

void VCCueList::slotFunctionStopped(quint32 fid)
{
    if (fid == m_chaserID)
    {
        emit playbackStatusChanged();
        setPlaybackIndex(-1);
        sendFeedback(0, INPUT_PLAY_PAUSE_ID, VCWidget::ExactValue);

        m_timer->stop();

        if (m_item != nullptr)
        {
            m_item->setProperty("progressStatus", ProgressIdle);
            m_item->setProperty("progressValue", 0);
            m_item->setProperty("progressText", "");
        }
    }
}

void VCCueList::slotCurrentStepChanged(int stepNumber)
{
    setPlaybackIndex(stepNumber);
}

void VCCueList::slotProgressTimeout()
{
    Chaser *ch = chaser();
    if (ch == nullptr || !ch->isRunning() || m_item == nullptr)
        return;

    ChaserRunnerStep step(ch->currentRunningStep());
    if (step.m_function != NULL)
    {
        ProgressStatus status = ProgressIdle;
        double progress = 0;

        if (step.m_fadeIn == Function::infiniteSpeed())
            status = ProgressInfinite;
        else if (step.m_elapsed <= (quint32)step.m_fadeIn)
            status = ProgressFadeIn;
        else
            status = ProgressHold;

        m_item->setProperty("progressStatus", status);

        if (step.m_duration == Function::infiniteSpeed())
        {
            if (status == ProgressFadeIn && step.m_fadeIn != Function::defaultSpeed())
            {
                progress = ((double)step.m_elapsed / (double)step.m_fadeIn);
                m_item->setProperty("progressValue", progress);
                m_item->setProperty("progressText", QString("-%1").arg(Function::speedToString(step.m_fadeIn - step.m_elapsed)));
            }
            else
            {
                m_item->setProperty("progressValue", 100);
                m_item->setProperty("progressText", "");
            }
        }
        else
        {
            progress = ((double)step.m_elapsed / (double)step.m_duration);
            m_item->setProperty("progressValue", progress);
            m_item->setProperty("progressText", QString("-%1").arg(Function::speedToString(step.m_duration - step.m_elapsed)));
        }
    }
    else
    {
        m_item->setProperty("progressValue", 0);
    }
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

bool VCCueList::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCCueList)
    {
        qWarning() << Q_FUNC_INFO << "Cue List node not found";
        return false;
    }

    /* Widget commons */
    loadXMLCommon(root);

    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCWindowState)
        {
            bool visible = false;
            int x = 0, y = 0, w = 0, h = 0;
            loadXMLWindowState(root, &x, &y, &w, &h, &visible);
            setGeometry(QRect(x, y, w, h));
        }
        else if (root.name() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(root);
        }
        else if (root.name() == KXMLQLCVCCueListChaser)
        {
            setChaserID(root.readElementText().toUInt());
        }
        else if (root.name() == KXMLQLCVCCueListPlaybackLayout)
        {
            PlaybackLayout layout = PlaybackLayout(root.readElementText().toInt());
            if (layout != PlayPauseStop && layout != PlayStopPause)
            {
                qWarning() << Q_FUNC_INFO << "Playback layout" << layout << "does not exist.";
                layout = PlayPauseStop;
            }
            setPlaybackLayout(layout);
        }
        else if (root.name() == KXMLQLCVCCueListNextPrevBehavior)
        {
            NextPrevBehavior nextPrev = NextPrevBehavior(root.readElementText().toInt());
            if (nextPrev != DefaultRunFirst && nextPrev != RunNext &&
                nextPrev != Select && nextPrev != Nothing)
            {
                qWarning() << Q_FUNC_INFO << "Next/Prev behavior" << nextPrev << "does not exist.";
                nextPrev = DefaultRunFirst;
            }
            setNextPrevBehavior(nextPrev);
        }
        else if (root.name() == KXMLQLCVCCueListSlidersMode)
        {
            setSideFaderMode(stringToFaderMode(root.readElementText()));
        }
        else if (root.name() == KXMLQLCVCCueListNext)
        {
            loadXMLSources(root, INPUT_NEXT_STEP_ID);
        }
        else if (root.name() == KXMLQLCVCCueListPrevious)
        {
            loadXMLSources(root, INPUT_PREVIOUS_STEP_ID);
        }
        else if (root.name() == KXMLQLCVCCueListPlayback)
        {
            loadXMLSources(root, INPUT_PLAY_PAUSE_ID);
        }
        else if (root.name() == KXMLQLCVCCueListStop)
        {
            loadXMLSources(root, INPUT_STOP_PAUSE_ID);
        }
        else if (root.name() == KXMLQLCVCCueListCrossfadeLeft)
        {
            loadXMLSources(root, INPUT_SIDE_FADER_ID);
        }
        else if (root.name() == KXMLQLCVCCueListCrossfadeRight) /* Legacy */
        {
            root.skipCurrentElement();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown VC Cue list tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool VCCueList::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != nullptr);

    /* VC Cue List entry */
    doc->writeStartElement(KXMLQLCVCCueList);

    saveXMLCommon(doc);

    /* Window state */
    saveXMLWindowState(doc);

    /* Appearance */
    saveXMLAppearance(doc);

    /* Chaser */
    doc->writeTextElement(KXMLQLCVCCueListChaser, QString::number(chaserID()));

    /* Playback layout */
    if (playbackLayout() != PlayPauseStop)
        doc->writeTextElement(KXMLQLCVCCueListPlaybackLayout, QString::number(playbackLayout()));

    /* Next/Prev behavior */
    if (nextPrevBehavior() != DefaultRunFirst)
        doc->writeTextElement(KXMLQLCVCCueListNextPrevBehavior, QString::number(nextPrevBehavior()));

    /* Crossfade cue list */
    if (sideFaderMode() != None)
        doc->writeTextElement(KXMLQLCVCCueListSlidersMode, faderModeToString(sideFaderMode()));

    /* Input controls */
    saveXMLInputControl(doc, INPUT_NEXT_STEP_ID, false, KXMLQLCVCCueListNext);
    saveXMLInputControl(doc, INPUT_PREVIOUS_STEP_ID, false, KXMLQLCVCCueListPrevious);
    saveXMLInputControl(doc, INPUT_PLAY_PAUSE_ID, false, KXMLQLCVCCueListPlayback);
    saveXMLInputControl(doc, INPUT_STOP_PAUSE_ID, false, KXMLQLCVCCueListStop);
    saveXMLInputControl(doc, INPUT_SIDE_FADER_ID, false, KXMLQLCVCCueListCrossfadeLeft);

    /* End the <CueList> tag */
    doc->writeEndElement();

    return true;
}
