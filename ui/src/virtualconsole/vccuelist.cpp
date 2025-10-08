/*
  Q Light Controller Plus
  vccuelist.cpp

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

#include <QStyledItemDelegate>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QTreeWidgetItem>
#include <QFontMetrics>
#include <QProgressBar>
#include <QTreeWidget>
#include <QHeaderView>
#include <QGridLayout>
#include <QSettings>
#include <QCheckBox>
#include <QString>
#include <QLabel>
#include <QDebug>
#include <QTimer>

#include "vccuelistproperties.h"
#include "vcpropertieseditor.h"
#include "clickandgoslider.h"
#include "chaserrunner.h"
#include "mastertimer.h"
#include "chaserstep.h"
#include "vccuelist.h"
#include "qlcmacros.h"
#include "function.h"
#include "vcwidget.h"
#include "apputil.h"
#include "chaser.h"
#include "qmath.h"
#include "doc.h"

#define COL_NUM      0
#define COL_NAME     1
#define COL_FADEIN   2
#define COL_FADEOUT  3
#define COL_DURATION 4
#define COL_NOTES    5

#define PROP_ID  Qt::UserRole
#define HYSTERESIS 3 // Hysteresis for next/previous external input

#define PROGRESS_INTERVAL 200
#define UPDATE_TIMEOUT 100

const quint8 VCCueList::nextInputSourceId = 0;
const quint8 VCCueList::previousInputSourceId = 1;
const quint8 VCCueList::playbackInputSourceId = 2;
const quint8 VCCueList::sideFaderInputSourceId = 3;
const quint8 VCCueList::stopInputSourceId = 4;

const QString progressDisabledStyle =
        "QProgressBar { border: 2px solid #C3C3C3; border-radius: 4px; background-color: #DCDCDC; }";
const QString progressFadeStyle =
        "QProgressBar { border: 2px solid grey; border-radius: 4px; background-color: #C3C3C3; text-align: center; }"
        "QProgressBar::chunk { background-color: #63C10B; width: 1px; }";
const QString progressHoldStyle =
        "QProgressBar { border: 2px solid grey; border-radius: 4px; background-color: #C3C3C3; text-align: center; }"
        "QProgressBar::chunk { background-color: #0F9BEC; width: 1px; }";

const QString cfLabelBlueStyle =
        "QLabel { background-color: #4E8DDE; color: white; border: 1px solid; border-radius: 3px; font: bold; }";
const QString cfLabelOrangeStyle =
        "QLabel { background-color: orange; color: black; border: 1px solid; border-radius: 3px; font: bold; }";
const QString cfLabelNoStyle =
        "QLabel { border: 1px solid; border-radius: 3px; font: bold; }";

VCCueList::VCCueList(QWidget *parent, Doc *doc) : VCWidget(parent, doc)
    , m_chaserID(Function::invalidId())
    , m_nextPrevBehavior(DefaultRunFirst)
    , m_playbackLayout(PlayPauseStop)
    , m_timer(NULL)
    , m_primaryIndex(0)
    , m_secondaryIndex(0)
    , m_primaryTop(true)
    , m_slidersMode(None)
{
    /* Set the class name "VCCueList" as the object name as well */
    setObjectName(VCCueList::staticMetaObject.className());

    /* Create a layout for this widget */
    QGridLayout *grid = new QGridLayout(this);
    grid->setSpacing(2);

    QFontMetrics m_fm = QFontMetrics(this->font());

    m_topPercentageLabel = new QLabel("100%");
    m_topPercentageLabel->setAlignment(Qt::AlignHCenter);
#if (QT_VERSION < QT_VERSION_CHECK(5, 11, 0))
    m_topPercentageLabel->setFixedWidth(m_fm.width("100%"));
#else
    m_topPercentageLabel->setFixedWidth(m_fm.horizontalAdvance("100%"));
#endif
    grid->addWidget(m_topPercentageLabel, 1, 0, 1, 1);

    m_topStepLabel = new QLabel("");
    m_topStepLabel->setStyleSheet(cfLabelNoStyle);
    m_topStepLabel->setAlignment(Qt::AlignCenter);
    m_topStepLabel->setFixedSize(32, 24);
    grid->addWidget(m_topStepLabel, 2, 0, 1, 1);

    m_sideFader = new ClickAndGoSlider();
    m_sideFader->setSliderStyleSheet(CNG_DEFAULT_STYLE);
    m_sideFader->setFixedWidth(32);
    m_sideFader->setRange(0, 100);
    m_sideFader->setValue(100);
    grid->addWidget(m_sideFader, 3, 0, 1, 1);

    m_bottomStepLabel = new QLabel("");
    m_bottomStepLabel->setStyleSheet(cfLabelNoStyle);
    m_bottomStepLabel->setAlignment(Qt::AlignCenter);
    m_bottomStepLabel->setFixedSize(32, 24);
    grid->addWidget(m_bottomStepLabel, 4, 0, 1, 1);

    m_bottomPercentageLabel = new QLabel("0%");
    m_bottomPercentageLabel->setAlignment(Qt::AlignHCenter);
#if (QT_VERSION < QT_VERSION_CHECK(5, 11, 0))
    m_bottomPercentageLabel->setFixedWidth(m_fm.width("100%"));
#else
    m_bottomPercentageLabel->setFixedWidth(m_fm.horizontalAdvance("100%"));
#endif
    grid->addWidget(m_bottomPercentageLabel, 5, 0, 1, 1);

    connect(m_sideFader, SIGNAL(valueChanged(int)),
            this, SLOT(slotSideFaderValueChanged(int)));

    slotShowCrossfadePanel(false);

    QVBoxLayout *vbox = new QVBoxLayout();

    /* Create a list for scenes (cues) */
    m_tree = new QTreeWidget(this);
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
    //m_tree->setAlternatingRowColors(true);
    m_tree->setAllColumnsShowFocus(true);
    m_tree->setRootIsDecorated(false);
    m_tree->setItemsExpandable(false);
    m_tree->header()->setSortIndicatorShown(false);
    m_tree->header()->setMinimumSectionSize(0); // allow columns to be hidden
    m_tree->header()->setSectionsClickable(false);
    m_tree->header()->setSectionsMovable(false);

    // Make only the notes column editable
    m_tree->setItemDelegateForColumn(COL_NUM, new NoEditDelegate(this));
    m_tree->setItemDelegateForColumn(COL_NAME, new NoEditDelegate(this));
    m_tree->setItemDelegateForColumn(COL_FADEIN, new NoEditDelegate(this));
    m_tree->setItemDelegateForColumn(COL_FADEOUT, new NoEditDelegate(this));
    m_tree->setItemDelegateForColumn(COL_DURATION, new NoEditDelegate(this));

    connect(m_tree, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(slotItemActivated(QTreeWidgetItem*)));
    connect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(slotItemChanged(QTreeWidgetItem*,int)));
    vbox->addWidget(m_tree);

    m_progress = new QProgressBar(this);
    m_progress->setOrientation(Qt::Horizontal);
    m_progress->setStyleSheet(progressDisabledStyle);
    m_progress->setProperty("status", 0);
    m_progress->setFixedHeight(20);
    vbox->addWidget(m_progress);

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()),
            this, SLOT(slotProgressTimeout()));

    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, SIGNAL(timeout()),
            this, SLOT(slotUpdateStepList()));
    m_updateTimer->setSingleShot(true);

    /* Create control buttons */
    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->setSpacing(2);

    m_crossfadeButton = new QToolButton(this);
    m_crossfadeButton->setIcon(QIcon(":/slider.png"));
    m_crossfadeButton->setIconSize(QSize(24, 24));
    m_crossfadeButton->setCheckable(true);
    m_crossfadeButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_crossfadeButton->setFixedHeight(32);
    m_crossfadeButton->setToolTip(tr("Show/Hide crossfade sliders"));
    m_crossfadeButton->setVisible(false);
    connect(m_crossfadeButton, SIGNAL(toggled(bool)),
            this, SLOT(slotShowCrossfadePanel(bool)));
    hbox->addWidget(m_crossfadeButton);

    m_playbackButton = new QToolButton(this);
    m_playbackButton->setIcon(QIcon(":/player_play.png"));
    m_playbackButton->setIconSize(QSize(24, 24));
    m_playbackButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_playbackButton->setFixedHeight(32);
    m_playbackButton->setToolTip(tr("Play/Pause Cue list"));
    connect(m_playbackButton, SIGNAL(clicked()), this, SLOT(slotPlayback()));
    hbox->addWidget(m_playbackButton);

    m_stopButton = new QToolButton(this);
    m_stopButton->setIcon(QIcon(":/player_stop.png"));
    m_stopButton->setIconSize(QSize(24, 24));
    m_stopButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_stopButton->setFixedHeight(32);
    m_stopButton->setToolTip(tr("Stop Cue list"));
    connect(m_stopButton, SIGNAL(clicked()), this, SLOT(slotStop()));
    hbox->addWidget(m_stopButton);

    m_previousButton = new QToolButton(this);
    m_previousButton->setIcon(QIcon(":/back.png"));
    m_previousButton->setIconSize(QSize(24, 24));
    m_previousButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_previousButton->setFixedHeight(32);
    m_previousButton->setToolTip(tr("Go to previous step in the list"));
    connect(m_previousButton, SIGNAL(clicked()), this, SLOT(slotPreviousCue()));
    hbox->addWidget(m_previousButton);

    m_nextButton = new QToolButton(this);
    m_nextButton->setIcon(QIcon(":/forward.png"));
    m_nextButton->setIconSize(QSize(24, 24));
    m_nextButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_nextButton->setFixedHeight(32);
    m_nextButton->setToolTip(tr("Go to next step in the list"));
    connect(m_nextButton, SIGNAL(clicked()), this, SLOT(slotNextCue()));
    hbox->addWidget(m_nextButton);

    vbox->addItem(hbox);
    grid->addItem(vbox, 0, 1, 6);

    setFrameStyle(KVCFrameStyleSunken);
    setType(VCWidget::CueListWidget);
    setCaption(tr("Cue list"));

    QSettings settings;
    QVariant var = settings.value(SETTINGS_CUELIST_SIZE);
    if (var.isValid() == true)
        resize(var.toSize());
    else
        resize(QSize(300, 220));

    slotModeChanged(m_doc->mode());
    setLiveEdit(m_liveEdit);

    connect(m_doc, SIGNAL(functionRemoved(quint32)),
            this, SLOT(slotFunctionRemoved(quint32)));
    connect(m_doc, SIGNAL(functionChanged(quint32)),
            this, SLOT(slotFunctionChanged(quint32)));
    connect(m_doc, SIGNAL(functionNameChanged(quint32)),
            this, SLOT(slotFunctionNameChanged(quint32)));

    m_nextLatestValue = 0;
    m_previousLatestValue = 0;
    m_playbackLatestValue = 0;
    m_stopLatestValue = 0;
}

VCCueList::~VCCueList()
{
}

void VCCueList::enableWidgetUI(bool enable)
{
    m_tree->setEnabled(enable);
    m_playbackButton->setEnabled(enable);
    m_stopButton->setEnabled(enable);
    m_previousButton->setEnabled(enable);
    m_nextButton->setEnabled(enable);

    m_topPercentageLabel->setEnabled(enable);
    m_sideFader->setEnabled(enable);
    m_topStepLabel->setEnabled(enable);
    m_bottomPercentageLabel->setEnabled(enable);
    m_bottomStepLabel->setEnabled(enable);
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

VCWidget *VCCueList::createCopy(VCWidget *parent)
{
    Q_ASSERT(parent != NULL);

    VCCueList *cuelist = new VCCueList(parent, m_doc);
    if (cuelist->copyFrom(this) == false)
    {
        delete cuelist;
        cuelist = NULL;
    }

    return cuelist;
}

bool VCCueList::copyFrom(const VCWidget *widget)
{
    const VCCueList *cuelist = qobject_cast<const VCCueList*> (widget);
    if (cuelist == NULL)
        return false;

    /* Function list contents */
    setChaser(cuelist->chaserID());

    /* Key sequence */
    setNextKeySequence(cuelist->nextKeySequence());
    setPreviousKeySequence(cuelist->previousKeySequence());
    setPlaybackKeySequence(cuelist->playbackKeySequence());
    setStopKeySequence(cuelist->stopKeySequence());

    /* Sliders mode */
    setSideFaderMode(cuelist->sideFaderMode());

    /* Common stuff */
    return VCWidget::copyFrom(widget);
}

/*****************************************************************************
 * Cue list
 *****************************************************************************/

void VCCueList::setChaser(quint32 id)
{
    Function *old = m_doc->function(m_chaserID);
    if (old != NULL)
    {
        /* Get rid of old function connections */
        disconnect(old, SIGNAL(running(quint32)),
                this, SLOT(slotFunctionRunning(quint32)));
        disconnect(old, SIGNAL(stopped(quint32)),
                this, SLOT(slotFunctionStopped(quint32)));
        disconnect(old, SIGNAL(currentStepChanged(int)),
                this, SLOT(slotCurrentStepChanged(int)));
    }

    Chaser *chaser = qobject_cast<Chaser*> (m_doc->function(id));
    if (chaser != NULL)
    {
        /* Connect to the new function */
        connect(chaser, SIGNAL(running(quint32)),
                this, SLOT(slotFunctionRunning(quint32)));
        connect(chaser, SIGNAL(stopped(quint32)),
                this, SLOT(slotFunctionStopped(quint32)));
        connect(chaser, SIGNAL(currentStepChanged(int)),
                this, SLOT(slotCurrentStepChanged(int)));

        m_chaserID = id;
    }
    else
    {
        m_chaserID = Function::invalidId();
    }

    updateStepList();

    /* Current status */
    if (chaser != NULL && chaser->isRunning())
    {
        slotFunctionRunning(m_chaserID);
        slotCurrentStepChanged(chaser->currentStepIndex());
    }
    else
        slotFunctionStopped(m_chaserID);
}

quint32 VCCueList::chaserID() const
{
    return m_chaserID;
}

Chaser *VCCueList::chaser()
{
    if (m_chaserID == Function::invalidId())
        return NULL;
    Chaser *chaser = qobject_cast<Chaser*>(m_doc->function(m_chaserID));
    return chaser;
}

void VCCueList::updateStepList()
{
    m_listIsUpdating = true;

    m_tree->clear();

    Chaser *ch = chaser();
    if (ch == NULL)
        return;

    QListIterator <ChaserStep> it(ch->steps());
    while (it.hasNext() == true)
    {
        ChaserStep step(it.next());

        Function *function = m_doc->function(step.fid);
        Q_ASSERT(function != NULL);

        QTreeWidgetItem *item = new QTreeWidgetItem(m_tree);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        int index = m_tree->indexOfTopLevelItem(item) + 1;
        item->setText(COL_NUM, QString("%1").arg(index));
        item->setData(COL_NUM, PROP_ID, function->id());
        item->setText(COL_NAME, function->name());
        if (step.note.isEmpty() == false)
            item->setText(COL_NOTES, step.note);

        switch (ch->fadeInMode())
        {
            case Chaser::Common:
                item->setText(COL_FADEIN, Function::speedToString(ch->fadeInSpeed()));
                break;
            case Chaser::PerStep:
                item->setText(COL_FADEIN, Function::speedToString(step.fadeIn));
                break;
            default:
            case Chaser::Default:
                item->setText(COL_FADEIN, QString());
        }

        switch (ch->fadeOutMode())
        {
            case Chaser::Common:
                item->setText(COL_FADEOUT, Function::speedToString(ch->fadeOutSpeed()));
                break;
            case Chaser::PerStep:
                item->setText(COL_FADEOUT, Function::speedToString(step.fadeOut));
                break;
            default:
            case Chaser::Default:
                item->setText(COL_FADEOUT, QString());
        }

        switch (ch->durationMode())
        {
            case Chaser::Common:
                item->setText(COL_DURATION, Function::speedToString(ch->duration()));
                break;
            case Chaser::PerStep:
                item->setText(COL_DURATION, Function::speedToString(step.duration));
                break;
            default:
            case Chaser::Default:
                item->setText(COL_DURATION, QString());
        }
    }

    QTreeWidgetItem *item = m_tree->topLevelItem(0);
    if (item != NULL)
        m_defCol = item->background(COL_NUM);

    m_tree->header()->resizeSections(QHeaderView::ResizeToContents);
    m_tree->header()->setSectionHidden(COL_NAME, ch->type() == Function::SequenceType ? true : false);
    m_listIsUpdating = false;
}

int VCCueList::getCurrentIndex()
{
    int index = m_tree->indexOfTopLevelItem(m_tree->currentItem());
    if (index == -1)
        index = 0;
    return index;
}

int VCCueList::getNextIndex()
{
    Chaser *ch = chaser();
    if (ch == NULL)
        return -1;

    if (ch->direction() == Function::Forward)
        return getNextTreeIndex();
    else
        return getPrevTreeIndex();
}

int VCCueList::getPrevIndex()
{
    Chaser *ch = chaser();
    if (ch == NULL)
        return -1;

    if (ch->direction() == Function::Forward)
        return getPrevTreeIndex();
    else
        return getNextTreeIndex();
}

int VCCueList::getFirstIndex()
{
    Chaser *ch = chaser();
    if (ch == NULL)
        return -1;

    if (ch->direction() == Function::Forward)
        return getFirstTreeIndex();
    else
        return getLastTreeIndex();
}

int VCCueList::getLastIndex()
{
    Chaser *ch = chaser();
    if (ch == NULL)
        return -1;

    if (ch->direction() == Function::Forward)
        return getLastTreeIndex();
    else
        return getFirstTreeIndex();
}

int VCCueList::getNextTreeIndex()
{
    int count = m_tree->topLevelItemCount();
    if (count > 0)
        return (getCurrentIndex() + 1) % count;
    return 0;
}

int VCCueList::getPrevTreeIndex()
{
    int currentIndex = getCurrentIndex();
    if (currentIndex <= 0)
        return getLastTreeIndex();
    return currentIndex - 1;
}

int VCCueList::getFirstTreeIndex()
{
    return 0;
}

int VCCueList::getLastTreeIndex()
{
    return m_tree->topLevelItemCount() - 1;
}

qreal VCCueList::getPrimaryIntensity() const
{
    if (sideFaderMode() == Steps)
        return  1.0;

    return m_primaryTop ? qreal(m_sideFader->value() / 100.0) : qreal((100 - m_sideFader->value()) / 100.0);
}

void VCCueList::notifyFunctionStarting(quint32 fid, qreal intensity)
{
    Q_UNUSED(intensity);

    if (mode() == Doc::Design)
        return;

    if (fid == m_chaserID)
        return;

    stopChaser();
}

void VCCueList::slotFunctionRemoved(quint32 fid)
{
    if (fid == m_chaserID)
    {
        setChaser(Function::invalidId());
        resetIntensityOverrideAttribute();
    }
}

void VCCueList::slotFunctionChanged(quint32 fid)
{
    if (fid == m_chaserID && !m_updateTimer->isActive())
        m_updateTimer->start(UPDATE_TIMEOUT);
}

void VCCueList::slotFunctionNameChanged(quint32 fid)
{
    if (fid == m_chaserID)
        m_updateTimer->start(UPDATE_TIMEOUT);
    else
    {
        // fid might be an ID of a ChaserStep of m_chaser
        Chaser *ch = chaser();
        if (ch == NULL)
            return;
        foreach (ChaserStep step, ch->steps())
        {
            if (step.fid == fid)
            {
                m_updateTimer->start(UPDATE_TIMEOUT);
                return;
            }
        }
    }
}

void VCCueList::slotUpdateStepList()
{
    updateStepList();
}

void VCCueList::slotPlayback()
{
    if (mode() != Doc::Operate)
        return;

    Chaser *ch = chaser();
    if (ch == NULL)
        return;

    if (ch->isRunning())
    {
        if (playbackLayout() == PlayPauseStop)
        {
            if (ch->isPaused())
            {
                m_playbackButton->setStyleSheet(QString("QToolButton{ background: %1; }")
                                                .arg(m_stopButton->palette().window().color().name()));
                m_playbackButton->setIcon(QIcon(":/player_pause.png"));
            }
            else
            {
                m_playbackButton->setStyleSheet("QToolButton{ background: #5B81FF; }");
                m_playbackButton->setIcon(QIcon(":/player_play.png"));
            }

            // check if the item selection has been changed during pause
            int currentTreeIndex = m_tree->indexOfTopLevelItem(m_tree->currentItem());
            if (currentTreeIndex != ch->currentStepIndex())
                playCueAtIndex(currentTreeIndex);

            ch->setPause(!ch->isPaused());
        }
        else if (playbackLayout() == PlayStopPause)
        {
            stopChaser();
            m_stopButton->setStyleSheet(QString("QToolButton{ background: %1; }")
                                            .arg(m_playbackButton->palette().window().color().name()));
        }
    }
    else
    {
        if (m_tree->currentItem() != NULL)
            startChaser(getCurrentIndex());
        else
            startChaser();
    }

    emit playbackButtonClicked();
}

void VCCueList::slotStop()
{
    if (mode() != Doc::Operate)
        return;

    Chaser *ch = chaser();
    if (ch == NULL)
        return;

    if (ch->isRunning())
    {
        if (playbackLayout() == PlayPauseStop)
        {
            stopChaser();
            m_playbackButton->setStyleSheet(QString("QToolButton{ background: %1; }")
                                            .arg(m_stopButton->palette().window().color().name()));
            m_progress->setFormat("");
            m_progress->setValue(0);

            emit progressStateChanged();
        }
        else if (playbackLayout() == PlayStopPause)
        {
            if (ch->isPaused())
            {
                m_stopButton->setStyleSheet(QString("QToolButton{ background: %1; }")
                                                .arg(m_playbackButton->palette().window().color().name()));
                m_stopButton->setIcon(QIcon(":/player_pause.png"));
            }
            else
            {
                m_stopButton->setStyleSheet("QToolButton{ background: #5B81FF; }");
            }
            ch->setPause(!ch->isPaused());
        }
    }
    else
    {
        m_primaryIndex = 0;
        m_tree->setCurrentItem(m_tree->topLevelItem(getFirstIndex()));
    }

    emit stopButtonClicked();
}

void VCCueList::slotNextCue()
{
    if (mode() != Doc::Operate)
        return;

    Chaser *ch = chaser();
    if (ch == NULL)
        return;

    if (ch->isRunning())
    {
        if (ch->isPaused())
        {
            m_tree->setCurrentItem(m_tree->topLevelItem(getNextIndex()));
        }
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
                m_tree->setCurrentItem(m_tree->topLevelItem(getNextIndex()));
            break;
            case Nothing:
            break;
            default:
                Q_ASSERT(false);
        }
    }
}

void VCCueList::slotPreviousCue()
{
    if (mode() != Doc::Operate)
        return;

    Chaser *ch = chaser();
    if (ch == NULL)
        return;

    if (ch->isRunning())
    {
        if (ch->isPaused())
        {
            m_tree->setCurrentItem(m_tree->topLevelItem(getPrevIndex()));
        }
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
                m_tree->setCurrentItem(m_tree->topLevelItem(getPrevIndex()));
            break;
            case Nothing:
            break;
            default:
                Q_ASSERT(false);
        }
    }
}

void VCCueList::slotCurrentStepChanged(int stepNumber)
{
    // Chaser is being edited, channels count may change.
    // Wait for the CueList to update its steps.
    if (m_updateTimer->isActive())
        return;

    Q_ASSERT(stepNumber < m_tree->topLevelItemCount() && stepNumber >= 0);
    QTreeWidgetItem *item = m_tree->topLevelItem(stepNumber);
    Q_ASSERT(item != NULL);
    m_tree->scrollToItem(item, QAbstractItemView::PositionAtCenter);
    m_tree->setCurrentItem(item);
    m_primaryIndex = stepNumber;
    if (sideFaderMode() == Steps)
    {
        m_bottomStepLabel->setStyleSheet(cfLabelBlueStyle);
        m_bottomStepLabel->setText(QString("#%1").arg(m_primaryIndex + 1));

        float stepVal;
        int stepsCount = m_tree->topLevelItemCount();
        if (stepsCount < 256) 
        {
            stepVal = 256.0 / (float)stepsCount; //divide up the full 0..255 range
            stepVal = qFloor((stepVal * 100000.0) + 0.5) / 100000.0; //round to 5 decimals to fix corner cases
        }
        else 
        {
            stepVal = 1.0;
        }
        
        // value->step# truncates down in slotSideFaderValueChanged; so use ceiling for step#->value
        float slValue = stepVal * (float)stepNumber;
        if (slValue > 255)
            slValue = 255.0;

        int upperBound = 255 - qCeil(slValue);
        int lowerBound = qFloor(256.0 - slValue - stepVal);
        // if the Step slider is already in range, then do not set its value
        // this means a user interaction is going on, either with the mouse or external controller
        if (m_sideFader->value() < lowerBound || m_sideFader->value() >= upperBound)
        {
            m_sideFader->blockSignals(true);
            m_sideFader->setValue(upperBound);
            m_topPercentageLabel->setText(QString("%1").arg(qCeil(slValue)));
            m_sideFader->blockSignals(false);

            //qDebug() << "Slider value:" << m_sideFader->value() << "->" << 255-qCeil(slValue) 
            //    << "(disp:" << slValue << ")" << "Step range:" << upperBound << lowerBound 
            //    << "(stepSize:" << stepVal << ")" 
            //    << "(raw lower:" << (256.0 - slValue - stepVal) << ")";
        }
    }
    else
    {
        setFaderInfo(m_primaryIndex);
    }
    emit stepChanged(m_primaryIndex);
    emit sideFaderValueChanged();
}

void VCCueList::slotItemActivated(QTreeWidgetItem *item)
{
    if (mode() != Doc::Operate)
        return;

    playCueAtIndex(m_tree->indexOfTopLevelItem(item));
}

void VCCueList::slotItemChanged(QTreeWidgetItem *item, int column)
{
    if (m_listIsUpdating || column != COL_NOTES)
        return;

    Chaser *ch = chaser();
    if (ch == NULL)
        return;

    QString itemText = item->text(column);
    int idx = m_tree->indexOfTopLevelItem(item);
    ChaserStep step = ch->steps().at(idx);

    step.note = itemText;
    ch->replaceStep(step, idx);

    emit stepNoteChanged(idx, itemText);
}

void VCCueList::slotStepNoteChanged(int idx, QString note)
{
    Chaser *ch = chaser();
    if (ch == NULL)
        return;
    ChaserStep step = ch->steps().at(idx);
    step.note = note;
    ch->replaceStep(step, idx);
}

void VCCueList::slotFunctionRunning(quint32 fid)
{
    if (fid != m_chaserID)
        return;

    if (playbackLayout() == PlayPauseStop)
        m_playbackButton->setIcon(QIcon(":/player_pause.png"));
    else if (playbackLayout() == PlayStopPause)
        m_playbackButton->setIcon(QIcon(":/player_stop.png"));
    m_timer->start(PROGRESS_INTERVAL);
    emit playbackStatusChanged();
    updateFeedback();
}

void VCCueList::slotFunctionStopped(quint32 fid)
{
    if (fid != m_chaserID)
        return;

    m_playbackButton->setIcon(QIcon(":/player_play.png"));
    m_topStepLabel->setText("");
    m_topStepLabel->setStyleSheet(cfLabelNoStyle);
    m_bottomStepLabel->setText("");
    m_bottomStepLabel->setStyleSheet(cfLabelNoStyle);
    // reset any previously set background
    QTreeWidgetItem *item = m_tree->topLevelItem(m_secondaryIndex);
    if (item != NULL)
        item->setBackground(COL_NUM, m_defCol);

    emit stepChanged(-1);

    m_progress->setFormat("");
    m_progress->setValue(0);    

    emit progressStateChanged();
    emit sideFaderValueChanged();
    emit playbackStatusChanged();

    qDebug() << Q_FUNC_INFO << "Cue stopped";
    updateFeedback();
}

void VCCueList::slotProgressTimeout()
{
    Chaser *ch = chaser();
    if (ch == NULL || !ch->isRunning())
        return;

    ChaserRunnerStep step(ch->currentRunningStep());
    if (step.m_function != NULL)
    {
        int status = m_progress->property("status").toInt();
        int newstatus;
        if (step.m_fadeIn == Function::defaultSpeed())
            newstatus = 1;
        else if (step.m_elapsed > (quint32)step.m_fadeIn)
            newstatus = 1;
        else
            newstatus = 0;

        if (newstatus != status)
        {
            if (newstatus == 0)
                m_progress->setStyleSheet(progressFadeStyle);
            else
                m_progress->setStyleSheet(progressHoldStyle);
            m_progress->setProperty("status", newstatus);
        }
        if (step.m_duration == Function::infiniteSpeed())
        {
            if (newstatus == 0 && step.m_fadeIn != Function::defaultSpeed())
            {
                double progress = ((double)step.m_elapsed / (double)step.m_fadeIn) * (double)m_progress->width();
                m_progress->setFormat(QString("-%1").arg(Function::speedToString(step.m_fadeIn - step.m_elapsed)));
                m_progress->setValue(progress);

                emit progressStateChanged();
            }
            else
            {
                m_progress->setValue(m_progress->maximum());
                m_progress->setFormat("");

                emit progressStateChanged();
            }
            return;
        }
        else
        {
            double progress = ((double)step.m_elapsed / (double)step.m_duration) * (double)m_progress->width();
            m_progress->setFormat(QString("-%1").arg(Function::speedToString(step.m_duration - step.m_elapsed)));
            m_progress->setValue(progress);

            emit progressStateChanged();
        }
    }
    else
    {
        m_progress->setValue(0);
    }
}

QString VCCueList::progressText()
{
    return m_progress->text();
}

double VCCueList::progressPercent()
{
    return ((double)m_progress->value() * 100) / (double)m_progress->width();
}

void VCCueList::startChaser(int startIndex)
{
    Chaser *ch = chaser();
    if (ch == NULL)
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
    emit functionStarting(m_chaserID);
}

void VCCueList::stopChaser()
{
    Chaser *ch = chaser();
    if (ch == NULL)
        return;

    ch->stop(functionParent());
    resetIntensityOverrideAttribute();
}

int VCCueList::getFadeMode()
{
    if (sideFaderMode() != Crossfade)
        return Chaser::FromFunction;

    if (m_sideFader->value() != 0 && m_sideFader->value() != 100)
        return Chaser::BlendedCrossfade;

    return Chaser::Blended;
}

void VCCueList::setNextPrevBehavior(NextPrevBehavior nextPrev)
{
    Q_ASSERT(nextPrev == DefaultRunFirst
            || nextPrev == RunNext
            || nextPrev == Select
            || nextPrev == Nothing);
    m_nextPrevBehavior = nextPrev;
}

VCCueList::NextPrevBehavior VCCueList::nextPrevBehavior() const
{
    return m_nextPrevBehavior;
}

void VCCueList::setPlaybackLayout(VCCueList::PlaybackLayout layout)
{
    if (layout == m_playbackLayout)
        return;

    if (layout == PlayStopPause)
    {
        m_stopButton->setIcon(QIcon(":/player_pause.png"));
        m_playbackButton->setToolTip(tr("Play/Stop Cue list"));
        m_stopButton->setToolTip(tr("Pause Cue list"));
    }
    else if (layout == PlayPauseStop)
    {
        m_stopButton->setIcon(QIcon(":/player_stop.png"));
        m_playbackButton->setToolTip(tr("Play/Pause Cue list"));
        m_stopButton->setToolTip(tr("Stop Cue list"));
    }
    else
    {
        qWarning() << "Playback layout" << layout << "doesn't exist!";
        layout = PlayPauseStop;
    }

    m_playbackLayout = layout;
}

VCCueList::PlaybackLayout VCCueList::playbackLayout() const
{
    return m_playbackLayout;
}

VCCueList::FaderMode VCCueList::sideFaderMode() const
{
    return m_slidersMode;
}

void VCCueList::setSideFaderMode(VCCueList::FaderMode mode)
{
    m_slidersMode = mode;

    bool show = (mode == None) ? false : true;
    m_crossfadeButton->setVisible(show);
    m_topPercentageLabel->setVisible(show);
    m_topStepLabel->setVisible(mode == Steps ? false : show);
    m_sideFader->setVisible(show);
    m_bottomPercentageLabel->setVisible(mode == Steps ? false : show);
    m_bottomStepLabel->setVisible(show);
    m_sideFader->setMaximum(mode == Steps ? 255 : 100);
    m_sideFader->setValue(mode == Steps ? 255 : 100);
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

/*****************************************************************************
 * Crossfade
 *****************************************************************************/
void VCCueList::setFaderInfo(int index)
{
    Chaser *ch = chaser();
    if (ch == NULL || !ch->isRunning())
        return;

    int tmpIndex = ch->computeNextStep(index);

    m_topStepLabel->setText(QString("#%1").arg(m_primaryTop ? index + 1 : tmpIndex + 1));
    m_topStepLabel->setStyleSheet(m_primaryTop ? cfLabelBlueStyle : cfLabelOrangeStyle);

    m_bottomStepLabel->setText(QString("#%1").arg(m_primaryTop ? tmpIndex + 1 : index + 1));
    m_bottomStepLabel->setStyleSheet(m_primaryTop ? cfLabelOrangeStyle : cfLabelBlueStyle);

    // reset any previously set background
    QTreeWidgetItem *item = m_tree->topLevelItem(m_secondaryIndex);
    if (item != NULL)
        item->setBackground(COL_NUM, m_defCol);

    item = m_tree->topLevelItem(tmpIndex);
    if (item != NULL)
        item->setBackground(COL_NUM, QColor("#FF8000"));
    m_secondaryIndex = tmpIndex;

    emit sideFaderValueChanged();
}

void VCCueList::slotShowCrossfadePanel(bool enable)
{
    m_topPercentageLabel->setVisible(enable);
    m_topStepLabel->setVisible(enable);
    m_sideFader->setVisible(enable);
    m_bottomStepLabel->setVisible(enable);
    m_bottomPercentageLabel->setVisible(enable);

    emit sideFaderButtonToggled();
}

QString VCCueList::topPercentageValue()
{
    return m_topPercentageLabel->text();
}

QString VCCueList::bottomPercentageValue()
{
    return m_bottomPercentageLabel->text();
}

QString VCCueList::topStepValue()
{
    return m_topStepLabel->text();
}

QString VCCueList::bottomStepValue()
{
    return m_bottomStepLabel->text();
}

int VCCueList::sideFaderValue()
{
    return m_sideFader->value();
}

bool VCCueList::primaryTop()
{
    return m_primaryTop;
}

void VCCueList::slotSideFaderButtonChecked(bool enable)
{
    m_crossfadeButton->setChecked(enable);
    emit sideFaderButtonChecked();
}

bool VCCueList::isSideFaderVisible()
{
    return m_sideFader->isVisible();
}

bool VCCueList::sideFaderButtonIsChecked()
{
    return m_crossfadeButton->isChecked();
}

void VCCueList::slotSetSideFaderValue(int value)
{
    m_sideFader->setValue(value);
}

void VCCueList::slotSideFaderValueChanged(int value)
{
    if (sideFaderMode() == Steps)
    {
        value = 255 - value;
        m_topPercentageLabel->setText(QString("%1").arg(value));

        emit sideFaderValueChanged();

        Chaser *ch = chaser();
        if (ch == NULL || ch->stopped())
            return;

        int newStep = value; // by default we assume the Chaser has more than 256 steps
        if (ch->stepsCount() < 256)
        {
            float stepSize = 256.0 / (float)ch->stepsCount();  //divide up the full 0..255 range
            stepSize = qFloor((stepSize * 100000.0) + 0.5) / 100000.0; //round to 5 decimals to fix corner cases
            if (value >= 256.0 - stepSize)
                newStep = ch->stepsCount() - 1;
            else
                newStep = qFloor((float)value / stepSize);
            //qDebug() << "value:" << value << " new step:" << newStep << " stepSize:" << stepSize;
        }

        if (newStep == ch->currentStepIndex())
            return;

        ChaserAction action;
        action.m_action = ChaserSetStepIndex;
        action.m_stepIndex = newStep;
        action.m_masterIntensity = intensity();
        action.m_stepIntensity = getPrimaryIntensity();
        action.m_fadeMode = getFadeMode();
        ch->setAction(action);
    }
    else
    {
        m_topPercentageLabel->setText(QString("%1%").arg(value));
        m_bottomPercentageLabel->setText(QString("%1%").arg(100 - value));

        emit sideFaderValueChanged();

        Chaser *ch = chaser();
        if (!(ch == NULL || ch->stopped()))
        {
            ch->adjustStepIntensity(qreal(value) / 100.0, m_primaryTop ? m_primaryIndex : m_secondaryIndex,
                                    Chaser::FadeControlMode(getFadeMode()));
            ch->adjustStepIntensity(qreal(100 - value) / 100.0, m_primaryTop ? m_secondaryIndex : m_primaryIndex,
                                    Chaser::FadeControlMode(getFadeMode()));
            stopStepIfNeeded(ch);
        }
    }

    updateFeedback();
}

void VCCueList::stopStepIfNeeded(Chaser *ch)
{
    if (ch->runningStepsNumber() != 2)
        return;

    int primaryValue = m_primaryTop ? m_sideFader->value() : 100 - m_sideFader->value();
    int secondaryValue = m_primaryTop ? 100 - m_sideFader->value() : m_sideFader->value();

    ChaserAction action;
    action.m_action = ChaserStopStep;

    if (primaryValue == 0)
    {
        m_primaryTop = !m_primaryTop;
        action.m_stepIndex = m_primaryIndex;
        ch->setAction(action);
    }
    else if (secondaryValue == 0)
    {
        action.m_stepIndex = m_secondaryIndex;
        ch->setAction(action);
    }
}

/*****************************************************************************
 * Key Sequences
 *****************************************************************************/

void VCCueList::setNextKeySequence(const QKeySequence& keySequence)
{
    m_nextKeySequence = QKeySequence(keySequence);
}

QKeySequence VCCueList::nextKeySequence() const
{
    return m_nextKeySequence;
}

void VCCueList::setPreviousKeySequence(const QKeySequence& keySequence)
{
    m_previousKeySequence = QKeySequence(keySequence);
}

QKeySequence VCCueList::previousKeySequence() const
{
    return m_previousKeySequence;
}

void VCCueList::setPlaybackKeySequence(const QKeySequence& keySequence)
{
    m_playbackKeySequence = QKeySequence(keySequence);
}

QKeySequence VCCueList::playbackKeySequence() const
{
    return m_playbackKeySequence;
}

void VCCueList::setStopKeySequence(const QKeySequence &keySequence)
{
    m_stopKeySequence = QKeySequence(keySequence);
}

QKeySequence VCCueList::stopKeySequence() const
{
    return m_stopKeySequence;
}

void VCCueList::slotKeyPressed(const QKeySequence& keySequence)
{
    if (acceptsInput() == false)
        return;

    if (m_nextKeySequence == keySequence)
        slotNextCue();
    else if (m_previousKeySequence == keySequence)
        slotPreviousCue();
    else if (m_playbackKeySequence == keySequence)
        slotPlayback();
    else if (m_stopKeySequence == keySequence)
        slotStop();
}

void VCCueList::updateFeedback()
{
    int fbv = int(SCALE(float(m_sideFader->value()), 
                        float(m_sideFader->minimum()),
                        float(m_sideFader->maximum()), 
                        float(0), float(UCHAR_MAX)));
    sendFeedback(fbv, sideFaderInputSourceId);

    Chaser *ch = chaser();
    if (ch == NULL)
        return;

    sendFeedback(ch->isRunning() ? UCHAR_MAX : 0, playbackInputSourceId);
}

/*****************************************************************************
 * External Input
 *****************************************************************************/

void VCCueList::slotInputValueChanged(quint32 universe, quint32 channel, uchar value)
{
    /* Don't let input data through in design mode or if disabled */
    if (acceptsInput() == false)
        return;

    quint32 pagedCh = (page() << 16) | channel;

    if (checkInputSource(universe, pagedCh, value, sender(), nextInputSourceId))
    {
        // Use hysteresis for values, in case the cue list is being controlled
        // by a slider. The value has to go to zero before the next non-zero
        // value is accepted as input. And the non-zero values have to visit
        // above $HYSTERESIS before a zero is accepted again.
        if (m_nextLatestValue == 0 && value > 0)
        {
            slotNextCue();
            m_nextLatestValue = value;
        }
        else if (m_nextLatestValue > HYSTERESIS && value == 0)
        {
            m_nextLatestValue = 0;
        }

        if (value > HYSTERESIS)
            m_nextLatestValue = value;
    }
    else if (checkInputSource(universe, pagedCh, value, sender(), previousInputSourceId))
    {
        // Use hysteresis for values, in case the cue list is being controlled
        // by a slider. The value has to go to zero before the next non-zero
        // value is accepted as input. And the non-zero values have to visit
        // above $HYSTERESIS before a zero is accepted again.
        if (m_previousLatestValue == 0 && value > 0)
        {
            slotPreviousCue();
            m_previousLatestValue = value;
        }
        else if (m_previousLatestValue > HYSTERESIS && value == 0)
        {
            m_previousLatestValue = 0;
        }

        if (value > HYSTERESIS)
            m_previousLatestValue = value;
    }
    else if (checkInputSource(universe, pagedCh, value, sender(), playbackInputSourceId))
    {
        // Use hysteresis for values, in case the cue list is being controlled
        // by a slider. The value has to go to zero before the next non-zero
        // value is accepted as input. And the non-zero values have to visit
        // above $HYSTERESIS before a zero is accepted again.
        if (m_playbackLatestValue == 0 && value > 0)
        {
            slotPlayback();
            m_playbackLatestValue = value;
        }
        else if (m_playbackLatestValue > HYSTERESIS && value == 0)
        {
            m_playbackLatestValue = 0;
        }

        if (value > HYSTERESIS)
            m_playbackLatestValue = value;
    }
    else if (checkInputSource(universe, pagedCh, value, sender(), stopInputSourceId))
    {
        // Use hysteresis for values, in case the cue list is being controlled
        // by a slider. The value has to go to zero before the next non-zero
        // value is accepted as input. And the non-zero values have to visit
        // above $HYSTERESIS before a zero is accepted again.
        if (m_stopLatestValue == 0 && value > 0)
        {
            slotStop();
            m_stopLatestValue = value;
        }
        else if (m_stopLatestValue > HYSTERESIS && value == 0)
        {
            m_stopLatestValue = 0;
        }

        if (value > HYSTERESIS)
            m_stopLatestValue = value;
    }
    else if (checkInputSource(universe, pagedCh, value, sender(), sideFaderInputSourceId))
    {
        if (sideFaderMode() == None)
            return;

        float val = SCALE((float) value, (float) 0, (float) UCHAR_MAX,
                          (float) m_sideFader->minimum(),
                          (float) m_sideFader->maximum());
        m_sideFader->setValue(val);
    }
}

/*****************************************************************************
 * VCWidget-inherited methods
 *****************************************************************************/

void VCCueList::adjustIntensity(qreal val)
{
    Chaser *ch = chaser();
    if (ch != NULL)
    {
        adjustFunctionIntensity(ch, val);
/*
        // Refresh intensity of current steps
        if (!ch->stopped() && sideFaderMode() == Crossfade && m_sideFader->value() != 100)
        {
                ch->adjustStepIntensity((qreal)m_sideFader->value() / 100, m_primaryTop ? m_primaryIndex : m_secondaryIndex);
                ch->adjustStepIntensity((qreal)(100 - m_sideFader->value()) / 100, m_primaryTop ? m_secondaryIndex : m_primaryIndex);
        }
*/
    }

    VCWidget::adjustIntensity(val);
}

void VCCueList::setCaption(const QString& text)
{
    VCWidget::setCaption(text);

    QStringList list;
    list << "#" << text << tr("Fade In") << tr("Fade Out") << tr("Duration") << tr("Notes");
    m_tree->setHeaderLabels(list);
}

void VCCueList::setFont(const QFont& font)
{
    VCWidget::setFont(font);

    QFontMetrics m_fm = QFontMetrics(font);
#if (QT_VERSION < QT_VERSION_CHECK(5, 11, 0))
    int w = m_fm.width("100%");
#else
    int w = m_fm.horizontalAdvance("100%");
#endif
    m_topPercentageLabel->setFixedWidth(w);
    m_bottomPercentageLabel->setFixedWidth(w);
}

void VCCueList::slotModeChanged(Doc::Mode mode)
{
    bool enable = false;
    if (mode == Doc::Operate)
    {
        m_progress->setStyleSheet(progressFadeStyle);
        m_progress->setRange(0, m_progress->width());
        enable = true;
        // send the initial feedback for the current step slider
        updateFeedback();
    }
    else
    {
        m_topStepLabel->setStyleSheet(cfLabelNoStyle);
        m_topStepLabel->setText("");
        m_bottomStepLabel->setStyleSheet(cfLabelNoStyle);
        m_bottomStepLabel->setText("");
        m_progress->setStyleSheet(progressDisabledStyle);
        // reset any previously set background
        QTreeWidgetItem *item = m_tree->topLevelItem(m_secondaryIndex);
        if (item != NULL)
            item->setBackground(COL_NUM, m_defCol);
    }

    enableWidgetUI(enable);

    /* Always start from the beginning */
    m_tree->setCurrentItem(NULL);

    VCWidget::slotModeChanged(mode);

    emit sideFaderValueChanged();
}

void VCCueList::editProperties()
{
    VCCueListProperties prop(this, m_doc);
    if (prop.exec() == QDialog::Accepted)
        m_doc->setModified();
}

void VCCueList::playCueAtIndex(int idx)
{
    if (mode() != Doc::Operate)
        return;

    m_primaryIndex = idx;

    Chaser *ch = chaser();
    if (ch == NULL)
        return;

    if (ch->isRunning())
    {
        ChaserAction action;
        action.m_action = ChaserSetStepIndex;
        action.m_stepIndex = m_primaryIndex;
        action.m_masterIntensity = intensity();
        action.m_stepIntensity = getPrimaryIntensity();
        action.m_fadeMode = getFadeMode();
        ch->setAction(action);
    }
    else
    {
        startChaser(m_primaryIndex);
    }

    if (sideFaderMode() == Crossfade)
        setFaderInfo(m_primaryIndex);
}

FunctionParent VCCueList::functionParent() const
{
    return FunctionParent(FunctionParent::ManualVCWidget, id());
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCCueList::loadXML(QXmlStreamReader &root)
{
    QList <quint32> legacyStepList;

    if (root.name() != KXMLQLCVCCueList)
    {
        qWarning() << Q_FUNC_INFO << "CueList node not found";
        return false;
    }

    /* Widget commons */
    loadXMLCommon(root);

    /* Children */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCWindowState)
        {
            bool visible = false;
            int x = 0, y = 0, w = 0, h = 0;
            loadXMLWindowState(root, &x, &y, &w, &h, &visible);
            setGeometry(x, y, w, h);
        }
        else if (root.name() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(root);
        }
        else if (root.name() == KXMLQLCVCCueListNext)
        {
            QString str = loadXMLSources(root, nextInputSourceId);
            if (str.isEmpty() == false)
                m_nextKeySequence = stripKeySequence(QKeySequence(str));
        }
        else if (root.name() == KXMLQLCVCCueListPrevious)
        {
            QString str = loadXMLSources(root, previousInputSourceId);
            if (str.isEmpty() == false)
                m_previousKeySequence = stripKeySequence(QKeySequence(str));
        }
        else if (root.name() == KXMLQLCVCCueListPlayback)
        {
            QString str = loadXMLSources(root, playbackInputSourceId);
            if (str.isEmpty() == false)
                m_playbackKeySequence = stripKeySequence(QKeySequence(str));
        }
        else if (root.name() == KXMLQLCVCCueListStop)
        {
            QString str = loadXMLSources(root, stopInputSourceId);
            if (str.isEmpty() == false)
                m_stopKeySequence = stripKeySequence(QKeySequence(str));
        }
        else if (root.name() == KXMLQLCVCCueListSlidersMode)
        {
            setSideFaderMode(stringToFaderMode(root.readElementText()));
        }
        else if (root.name() == KXMLQLCVCCueListCrossfadeLeft)
        {
            loadXMLSources(root, sideFaderInputSourceId);
        }
        else if (root.name() == KXMLQLCVCCueListCrossfadeRight) /* Legacy */
        {
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLQLCVCWidgetKey) /* Legacy */
        {
            setNextKeySequence(QKeySequence(root.readElementText()));
        }
        else if (root.name() == KXMLQLCVCCueListChaser)
        {
            setChaser(root.readElementText().toUInt());
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
            if (nextPrev != DefaultRunFirst
                    && nextPrev != RunNext
                    && nextPrev != Select
                    && nextPrev != Nothing)
            {
                qWarning() << Q_FUNC_INFO << "Next/Prev behavior" << nextPrev << "does not exist.";
                nextPrev = DefaultRunFirst;
            }
            setNextPrevBehavior(nextPrev);
        }
        else if (root.name() == KXMLQLCVCCueListCrossfade)
        {
            m_crossfadeButton->setChecked(true);
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLQLCVCCueListFunction)
        {
            // Collect legacy file format steps into a list
            legacyStepList << root.readElementText().toUInt();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown cuelist tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    if (legacyStepList.isEmpty() == false)
    {
        /* Construct a new chaser from legacy step functions and use that chaser */
        Chaser *chaser = new Chaser(m_doc);
        chaser->setName(caption());

        // Legacy cue lists relied on individual functions' timings and a common hold time
        chaser->setFadeInMode(Chaser::Default);
        chaser->setFadeOutMode(Chaser::Default);
        chaser->setDurationMode(Chaser::Common);

        foreach (quint32 id, legacyStepList)
        {
            Function *function = m_doc->function(id);
            if (function == NULL)
                continue;

            // Legacy cuelists relied on individual functions' fadein/out speed and
            // infinite duration. So don't touch them at all.
            ChaserStep step(id);
            chaser->addStep(step);
        }

        // Add the chaser to Doc and attach it to the cue list
        m_doc->addFunction(chaser);
        setChaser(chaser->id());
    }

    return true;
}

bool VCCueList::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    /* VC CueList entry */
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
    doc->writeTextElement(KXMLQLCVCCueListNextPrevBehavior, QString::number(nextPrevBehavior()));

    /* Next cue */
    doc->writeStartElement(KXMLQLCVCCueListNext);
    if (m_nextKeySequence.toString().isEmpty() == false)
        doc->writeTextElement(KXMLQLCVCWidgetKey, m_nextKeySequence.toString());
    saveXMLInput(doc, inputSource(nextInputSourceId));
    doc->writeEndElement();

    /* Previous cue */
    doc->writeStartElement(KXMLQLCVCCueListPrevious);
    if (m_previousKeySequence.toString().isEmpty() == false)
        doc->writeTextElement(KXMLQLCVCWidgetKey, m_previousKeySequence.toString());
    saveXMLInput(doc, inputSource(previousInputSourceId));
    doc->writeEndElement();

    /* Cue list playback */
    doc->writeStartElement(KXMLQLCVCCueListPlayback);
    if (m_playbackKeySequence.toString().isEmpty() == false)
        doc->writeTextElement(KXMLQLCVCWidgetKey, m_playbackKeySequence.toString());
    saveXMLInput(doc, inputSource(playbackInputSourceId));
    doc->writeEndElement();

    /* Cue list stop */
    doc->writeStartElement(KXMLQLCVCCueListStop);
    if (m_stopKeySequence.toString().isEmpty() == false)
        doc->writeTextElement(KXMLQLCVCWidgetKey, m_stopKeySequence.toString());
    saveXMLInput(doc, inputSource(stopInputSourceId));
    doc->writeEndElement();

    /* Crossfade cue list */
    if (sideFaderMode() != None)
        doc->writeTextElement(KXMLQLCVCCueListSlidersMode, faderModeToString(sideFaderMode()));

    QSharedPointer<QLCInputSource> cf1Src = inputSource(sideFaderInputSourceId);
    if (!cf1Src.isNull() && cf1Src->isValid())
    {
        doc->writeStartElement(KXMLQLCVCCueListCrossfadeLeft);
        saveXMLInput(doc, cf1Src);
        doc->writeEndElement();
    }

    /* End the <CueList> tag */
    doc->writeEndElement();

    return true;
}
