/*
  Q Light Controller
  vccuelistproperties.cpp

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

#include <QAction>

#include "vccuelistproperties.h"
#include "inputselectionwidget.h"
#include "functionselection.h"
#include "vccuelist.h"
#include "doc.h"

VCCueListProperties::VCCueListProperties(VCCueList* cueList, Doc* doc)
    : QDialog(cueList)
    , m_doc(doc)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(cueList != NULL);
    m_cueList = cueList;

    setupUi(this);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    /************************************************************************
     * Cues page
     ************************************************************************/

    /* Name */
    m_nameEdit->setText(cueList->caption());
    m_nameEdit->setSelection(0, cueList->caption().length());

    /* Chaser */
    m_chaserId = cueList->chaserID();
    updateChaserName();

    /* Next/Prev behavior */
    m_nextPrevBehaviorCombo->setCurrentIndex(m_cueList->nextPrevBehavior());

    /* Connections */
    connect(m_chaserAttachButton, SIGNAL(clicked()), this, SLOT(slotChaserAttachClicked()));
    connect(m_chaserDetachButton, SIGNAL(clicked()), this, SLOT(slotChaserDetachClicked()));

    /************************************************************************
     * Play/Stop Cue List page
     ************************************************************************/

    m_playInputWidget = new InputSelectionWidget(m_doc, this);
    m_playInputWidget->setTitle(tr("Play/Pause control"));
    m_playInputWidget->setCustomFeedbackVisibility(true);
    m_playInputWidget->setKeySequence(m_cueList->playbackKeySequence());
    m_playInputWidget->setInputSource(m_cueList->inputSource(VCCueList::playbackInputSourceId));
    m_playInputWidget->setWidgetPage(m_cueList->page());
    m_playInputWidget->show();
    m_playbackLayout->addWidget(m_playInputWidget);

    m_stopInputWidget = new InputSelectionWidget(m_doc, this);
    m_stopInputWidget->setTitle(tr("Stop control"));
    m_stopInputWidget->setCustomFeedbackVisibility(true);
    m_stopInputWidget->setKeySequence(m_cueList->stopKeySequence());
    m_stopInputWidget->setInputSource(m_cueList->inputSource(VCCueList::stopInputSourceId));
    m_stopInputWidget->setWidgetPage(m_cueList->page());
    m_stopInputWidget->show();
    m_stopLayout->addWidget(m_stopInputWidget);

    /************************************************************************
     * Next Cue page
     ************************************************************************/

    m_nextInputWidget = new InputSelectionWidget(m_doc, this);
    m_nextInputWidget->setCustomFeedbackVisibility(true);
    m_nextInputWidget->setKeySequence(m_cueList->nextKeySequence());
    m_nextInputWidget->setInputSource(m_cueList->inputSource(VCCueList::nextInputSourceId));
    m_nextInputWidget->setWidgetPage(m_cueList->page());
    m_nextInputWidget->show();
    m_nextLayout->addWidget(m_nextInputWidget);

    /************************************************************************
     * Previous Cue page
     ************************************************************************/

    m_prevInputWidget = new InputSelectionWidget(m_doc, this);
    m_prevInputWidget->setCustomFeedbackVisibility(true);
    m_prevInputWidget->setKeySequence(m_cueList->previousKeySequence());
    m_prevInputWidget->setInputSource(m_cueList->inputSource(VCCueList::previousInputSourceId));
    m_prevInputWidget->setWidgetPage(m_cueList->page());
    m_prevInputWidget->show();
    m_previousLayout->addWidget(m_prevInputWidget);

    /************************************************************************
     * Crossfade Cue List page
     ************************************************************************/

    if (cueList->sideFaderMode() == VCCueList::Steps)
        m_stepsRadio->setChecked(true);
    else if (cueList->sideFaderMode() == VCCueList::Crossfade)
        m_crossFadeRadio->setChecked(true);

    m_crossfadeInputWidget = new InputSelectionWidget(m_doc, this);
    m_crossfadeInputWidget->setTitle(tr("External Input"));
    m_crossfadeInputWidget->setKeyInputVisibility(false);
    m_crossfadeInputWidget->setInputSource(m_cueList->inputSource(VCCueList::sideFaderInputSourceId));
    m_crossfadeInputWidget->setWidgetPage(m_cueList->page());
    m_crossfadeInputWidget->show();
    m_crossFadeLayout->addWidget(m_crossfadeInputWidget);

    /* Playback layout */
    connect(m_play_stop_pause, SIGNAL(clicked(bool)), this, SLOT(slotPlaybackLayoutChanged()));
    connect(m_play_pause_stop, SIGNAL(clicked(bool)), this, SLOT(slotPlaybackLayoutChanged()));

    if (m_cueList->playbackLayout() == VCCueList::PlayStopPause)
        m_play_stop_pause->setChecked(true);
    else
        m_play_pause_stop->setChecked(true);
}

VCCueListProperties::~VCCueListProperties()
{
}

void VCCueListProperties::accept()
{
    /* Name */
    m_cueList->setCaption(m_nameEdit->text());

    /* Chaser */
    m_cueList->setChaser(m_chaserId);

    /* Playback layout */
    if (m_play_stop_pause->isChecked())
        m_cueList->setPlaybackLayout(VCCueList::PlayStopPause);
    else
        m_cueList->setPlaybackLayout(VCCueList::PlayPauseStop);

    /* Next/Prev behavior */
    m_cueList->setNextPrevBehavior(VCCueList::NextPrevBehavior(m_nextPrevBehaviorCombo->currentIndex()));

    /* Key sequences */
    m_cueList->setNextKeySequence(m_nextInputWidget->keySequence());
    m_cueList->setPreviousKeySequence(m_prevInputWidget->keySequence());
    m_cueList->setPlaybackKeySequence(m_playInputWidget->keySequence());
    m_cueList->setStopKeySequence(m_stopInputWidget->keySequence());

    /* Input sources */
    m_cueList->setInputSource(m_nextInputWidget->inputSource(), VCCueList::nextInputSourceId);
    m_cueList->setInputSource(m_prevInputWidget->inputSource(), VCCueList::previousInputSourceId);
    m_cueList->setInputSource(m_playInputWidget->inputSource(), VCCueList::playbackInputSourceId);
    m_cueList->setInputSource(m_stopInputWidget->inputSource(), VCCueList::stopInputSourceId);
    m_cueList->setInputSource(m_crossfadeInputWidget->inputSource(), VCCueList::sideFaderInputSourceId);

    if (m_noneRadio->isChecked())
        m_cueList->setSideFaderMode(VCCueList::None);
    else if (m_stepsRadio->isChecked())
        m_cueList->setSideFaderMode(VCCueList::Steps);
    else
        m_cueList->setSideFaderMode(VCCueList::Crossfade);

    QDialog::accept();
}

void VCCueListProperties::slotTabChanged()
{
    m_playInputWidget->stopAutoDetection();
    m_stopInputWidget->stopAutoDetection();
    m_nextInputWidget->stopAutoDetection();
    m_prevInputWidget->stopAutoDetection();

    m_crossfadeInputWidget->stopAutoDetection();
}

/****************************************************************************
 * Cues
 ****************************************************************************/

void VCCueListProperties::slotChaserAttachClicked()
{
    FunctionSelection fs(this, m_doc);
    fs.setMultiSelection(false);
    fs.setFilter(Function::ChaserType | Function::SequenceType, true);
    if (fs.exec() == QDialog::Accepted && fs.selection().size() > 0)
    {
        m_chaserId = fs.selection().first();
        updateChaserName();
    }
}

void VCCueListProperties::slotChaserDetachClicked()
{
    m_chaserId = Function::invalidId();
    updateChaserName();
}

void VCCueListProperties::slotPlaybackLayoutChanged()
{
    if (m_play_pause_stop->isChecked())
    {
        m_playInputWidget->setTitle(tr("Play/Pause control"));
        m_stopInputWidget->setTitle(tr("Stop control"));
    }
    else
    {
        m_playInputWidget->setTitle(tr("Play/Stop control"));
        m_stopInputWidget->setTitle(tr("Pause control"));
    }
}

void VCCueListProperties::updateChaserName()
{
    Function* function = m_doc->function(m_chaserId);
    if (function == NULL)
        m_chaserEdit->setText(tr("No function"));
    else
        m_chaserEdit->setText(function->name());
}
