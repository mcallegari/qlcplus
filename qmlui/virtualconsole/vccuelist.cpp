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
#include "chaser.h"
#include "tardis.h"

#define INPUT_NEXT_STEP_ID          0
#define INPUT_PREVIOUS_STEP_ID      1
#define INPUT_PLAY_PAUSE_ID         2
#define INPUT_CROSSFADE_L_ID        3
#define INPUT_CROSSFADE_R_ID        4
#define INPUT_STOP_PAUSE_ID         5

VCCueList::VCCueList(Doc *doc, QObject *parent)
    : VCWidget(doc, parent)
    , m_nextPrevBehavior(DefaultRunFirst)
    , m_playbackLayout(PlayPauseStop)
    , m_chaserID(Function::invalidId())
    , m_playbackIndex(-1)
{
    setType(VCWidget::CueListWidget);

    registerExternalControl(INPUT_NEXT_STEP_ID, tr("Next Cue"), true);
    registerExternalControl(INPUT_PREVIOUS_STEP_ID, tr("Previous Cue"), true);
    registerExternalControl(INPUT_PLAY_PAUSE_ID, tr("Play/Stop/Pause"), true);
    registerExternalControl(INPUT_CROSSFADE_L_ID, tr("Left Crossfade"), false);
    registerExternalControl(INPUT_CROSSFADE_R_ID, tr("Right Crossfade"), false);
    registerExternalControl(INPUT_STOP_PAUSE_ID, tr("Stop/Pause"), true);

    m_stepsList = new ListModel(this);
    QStringList listRoles;
    listRoles << "funcID" << "isSelected" << "fadeIn" << "hold" << "fadeOut" << "duration" << "note";
    m_stepsList->setRoleNames(listRoles);
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
    if (view == NULL || parent == NULL)
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
        return NULL;
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
        if (ch == NULL)
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
                if (func == NULL)
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
        if (f == NULL || f->type() != Function::ChaserType)
            return;

        setChaserID(f->id());
    }
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

    if (current != NULL)
    {
        /* Get rid of old function connections */
        disconnect(current, SIGNAL(running(quint32)),
                   this, SLOT(slotFunctionRunning(quint32)));
        disconnect(current, SIGNAL(stopped(quint32)),
                   this, SLOT(slotFunctionStopped(quint32)));
        disconnect(current, SIGNAL(currentStepChanged(int)),
                   this, SLOT(slotCurrentStepChanged(int)));

        if(current->isRunning())
        {
            running = true;
            current->stop(functionParent());
        }
    }

    if (function != NULL)
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

/*********************************************************************
 * Playback
 *********************************************************************/

int VCCueList::getNextIndex()
{
    Chaser *ch = chaser();
    if (ch == NULL)
        return -1;

    if (ch->direction() == Function::Forward)
        return m_playbackIndex + 1 == ch->stepsCount() ? 0 : m_playbackIndex + 1;
    else
        return m_playbackIndex == 0 ? ch->stepsCount() - 1 : m_playbackIndex - 1;
}

int VCCueList::getPrevIndex()
{
    Chaser *ch = chaser();
    if (ch == NULL)
        return -1;

    if (ch->direction() == Function::Forward)
        return m_playbackIndex == 0 ? ch->stepsCount() - 1 : m_playbackIndex - 1;
    else
        return m_playbackIndex + 1 == ch->stepsCount() ? 0 : m_playbackIndex + 1;
}

int VCCueList::getFirstIndex()
{
    Chaser *ch = chaser();
    if (ch == NULL)
        return -1;

    if (ch->direction() == Function::Forward)
        return 0;
    else
        return ch->stepsCount() - 1;
}

int VCCueList::getLastIndex()
{
    Chaser *ch = chaser();
    if (ch == NULL)
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
}

VCCueList::PlaybackStatus VCCueList::playbackStatus()
{
    Chaser *ch = chaser();

    if (ch == NULL)
        return Stopped;
    else if (ch->isPaused())
        return Paused;

    return ch->isRunning() ? Playing : Stopped;
}

void VCCueList::startChaser(int startIndex)
{
    Chaser *ch = chaser();
    if (ch == NULL)
        return;

    ch->setStepIndex(startIndex);
    //ch->setStartIntensity(getPrimaryIntensity());
    adjustFunctionIntensity(ch, intensity());
    ch->start(m_doc->masterTimer(), functionParent());
    emit functionStarting(this, m_chaserID, intensity());
    emit playbackStatusChanged();
}

void VCCueList::stopChaser()
{
    Chaser *ch = chaser();
    if (ch == NULL)
        return;

    ch->stop(functionParent());
    resetIntensityOverrideAttribute();
    emit playbackStatusChanged();
}

void VCCueList::playClicked()
{
    Chaser *ch = chaser();
    if (ch == NULL)
        return;

    if (ch->isRunning())
    {
        if (playbackLayout() == PlayPauseStop)
        {
#if 0 // TODO
            // check if the item selection has been changed during pause
            if (m_playbackIndex != ch->currentStepIndex())
                ch->setCurrentStep(m_playbackIndex, getPrimaryIntensity());
#endif
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
    if (ch == NULL)
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
    if (ch == NULL)
        return;

    if (ch->isRunning())
    {
        if (ch->isPaused())
            setPlaybackIndex(getPrevIndex());
        else
            ch->previous();
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
    if (ch == NULL)
        return;

    if (ch->isRunning())
    {
        if (ch->isPaused())
            setPlaybackIndex(getNextIndex());
        else
            ch->next();
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

void VCCueList::slotFunctionRunning(quint32 fid)
{
    if (fid == m_chaserID)
    {
        emit playbackStatusChanged();
        // updateFeedback(); TODO
    }
}

void VCCueList::slotFunctionStopped(quint32 fid)
{
    if (fid == m_chaserID)
    {
        emit playbackStatusChanged();
        setPlaybackIndex(-1);
        // updateFeedback(); TODO
    }
}

void VCCueList::slotCurrentStepChanged(int stepNumber)
{
    setPlaybackIndex(stepNumber);
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
            loadXMLSources(root, INPUT_CROSSFADE_L_ID);
        }
        else if (root.name() == KXMLQLCVCCueListCrossfadeRight)
        {
            loadXMLSources(root, INPUT_CROSSFADE_R_ID);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown label tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool VCCueList::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

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

    /* Input controls */
    saveXMLInputControl(doc, INPUT_NEXT_STEP_ID, KXMLQLCVCCueListNext);
    saveXMLInputControl(doc, INPUT_PREVIOUS_STEP_ID, KXMLQLCVCCueListPrevious);
    saveXMLInputControl(doc, INPUT_PLAY_PAUSE_ID, KXMLQLCVCCueListPlayback);
    saveXMLInputControl(doc, INPUT_STOP_PAUSE_ID, KXMLQLCVCCueListStop);

    /* End the <CueList> tag */
    doc->writeEndElement();

    return true;
}
