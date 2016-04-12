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
#include "qlcinputchannel.h"
#include "virtualconsole.h"
#include "chaserrunner.h"
#include "mastertimer.h"
#include "chaserstep.h"
#include "inputpatch.h"
#include "vccuelist.h"
#include "qlcmacros.h"
#include "function.h"
#include "qlcfile.h"
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
const quint8 VCCueList::cf1InputSourceId = 3;
const quint8 VCCueList::cf2InputSourceId = 4;
const quint8 VCCueList::stopInputSourceId = 5;

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

VCCueList::VCCueList(QWidget* parent, Doc* doc) : VCWidget(parent, doc)
    , m_chaserID(Function::invalidId())
    , m_nextPrevBehavior(DefaultRunFirst)
    , m_timer(NULL)
    , m_primaryIndex(0)
    , m_secondaryIndex(0)
    , m_primaryLeft(true)
    , m_slidersMode(Crossfade)
{
    /* Set the class name "VCCueList" as the object name as well */
    setObjectName(VCCueList::staticMetaObject.className());

    /* Create a layout for this widget */
    QGridLayout* grid = new QGridLayout(this);
    grid->setSpacing(2);

    m_linkCheck = new QCheckBox(tr("Link"));
    grid->addWidget(m_linkCheck, 0, 0, 1, 2, Qt::AlignVCenter | Qt::AlignCenter);

    m_sl1TopLabel = new QLabel("100%");
    m_sl1TopLabel->setAlignment(Qt::AlignHCenter);
    grid->addWidget(m_sl1TopLabel, 1, 0, 1, 1);
    m_slider1 = new ClickAndGoSlider();
    m_slider1->setSliderStyleSheet(CNG_DEFAULT_STYLE);
    m_slider1->setFixedWidth(32);
    m_slider1->setRange(0, 100);
    m_slider1->setValue(100);
    grid->addWidget(m_slider1, 2, 0, 1, 1);
    m_sl1BottomLabel = new QLabel("");
    m_sl1BottomLabel->setStyleSheet(cfLabelNoStyle);
    m_sl1BottomLabel->setAlignment(Qt::AlignCenter);
    grid->addWidget(m_sl1BottomLabel, 3, 0, 1, 1);
    connect(m_slider1, SIGNAL(valueChanged(int)),
            this, SLOT(slotSlider1ValueChanged(int)));

    m_sl2TopLabel = new QLabel("0%");
    m_sl2TopLabel->setAlignment(Qt::AlignHCenter);
    grid->addWidget(m_sl2TopLabel, 1, 1, 1, 1);
    m_slider2 = new ClickAndGoSlider();
    m_slider2->setSliderStyleSheet(CNG_DEFAULT_STYLE);
    m_slider2->setFixedWidth(32);
    m_slider2->setRange(0, 100);
    m_slider2->setValue(0);
    m_slider2->setInvertedAppearance(true);
    grid->addWidget(m_slider2, 2, 1, 1, 1);
    m_sl2BottomLabel = new QLabel("");
    m_sl2BottomLabel->setStyleSheet(cfLabelNoStyle);
    m_sl2BottomLabel->setAlignment(Qt::AlignCenter);
    grid->addWidget(m_sl2BottomLabel, 3, 1, 1, 1);
    connect(m_slider2, SIGNAL(valueChanged(int)),
            this, SLOT(slotSlider2ValueChanged(int)));

    slotShowCrossfadePanel(false);

    /* Create a list for scenes (cues) */
    m_tree = new QTreeWidget(this);
    grid->addWidget(m_tree, 0, 2, 3, 1);
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
    //m_tree->setAlternatingRowColors(true);
    m_tree->setAllColumnsShowFocus(true);
    m_tree->setRootIsDecorated(false);
    m_tree->setItemsExpandable(false);
    m_tree->header()->setSortIndicatorShown(false);
    m_tree->header()->setMinimumSectionSize(0); // allow columns to be hidden
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    m_tree->header()->setClickable(false);
    m_tree->header()->setMovable(false);
#else
    m_tree->header()->setSectionsClickable(false);
    m_tree->header()->setSectionsMovable(false);
#endif

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

    m_progress = new QProgressBar(this);
    m_progress->setOrientation(Qt::Horizontal);
    m_progress->setStyleSheet(progressDisabledStyle);
    m_progress->setProperty("status", 0);
    m_progress->setFixedHeight(20);
    grid->addWidget(m_progress, 3, 2);

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

    grid->addItem(hbox, 4, 2);

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

    m_linkCheck->setEnabled(enable);
    m_sl1TopLabel->setEnabled(enable);
    m_slider1->setEnabled(enable);
    m_sl1BottomLabel->setEnabled(enable);
    m_sl2TopLabel->setEnabled(enable);
    m_slider2->setEnabled(enable);
    m_sl2BottomLabel->setEnabled(enable);
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

VCWidget* VCCueList::createCopy(VCWidget* parent)
{
    Q_ASSERT(parent != NULL);

    VCCueList* cuelist = new VCCueList(parent, m_doc);
    if (cuelist->copyFrom(this) == false)
    {
        delete cuelist;
        cuelist = NULL;
    }

    return cuelist;
}

bool VCCueList::copyFrom(const VCWidget* widget)
{
    const VCCueList* cuelist = qobject_cast<const VCCueList*> (widget);
    if (cuelist == NULL)
        return false;

    /* Function list contents */
    setChaser(cuelist->chaserID());

    /* Key sequence */
    setNextKeySequence(cuelist->nextKeySequence());
    setPreviousKeySequence(cuelist->previousKeySequence());
    setPlaybackKeySequence(cuelist->playbackKeySequence());
    setStopKeySequence(cuelist->stopKeySequence());

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

    Chaser* chaser = qobject_cast<Chaser*> (m_doc->function(id));
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

    Chaser* ch = chaser();
    if (ch == NULL)
        return;

    QListIterator <ChaserStep> it(ch->steps());
    while (it.hasNext() == true)
    {
        ChaserStep step(it.next());

        Function* function = m_doc->function(step.fid);
        Q_ASSERT(function != NULL);

        QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
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
    Chaser* ch = chaser();
    if (ch == NULL)
        return -1;

    if (ch->direction() == Function::Forward)
        return getNextTreeIndex();
    else
        return getPrevTreeIndex();
}

int VCCueList::getPrevIndex()
{
    Chaser* ch = chaser();
    if (ch == NULL)
        return -1;

    if (ch->direction() == Function::Forward)
        return getPrevTreeIndex();
    else
        return getNextTreeIndex();
}

int VCCueList::getFirstIndex()
{
    Chaser* ch = chaser();
    if (ch == NULL)
        return -1;

    if (ch->direction() == Function::Forward)
        return getFirstTreeIndex();
    else
        return getLastTreeIndex();
}

int VCCueList::getLastIndex()
{
    Chaser* ch = chaser();
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
    qreal value;
    if (slidersMode() == Steps)
        value = 1.0;
    else
        value = (qreal)((m_primaryLeft ? m_slider1 : m_slider2)->value()) / 100;
    return value;
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
        setChaser(Function::invalidId());
}

void VCCueList::slotFunctionChanged(quint32 fid)
{
    if (fid == m_chaserID)
        m_updateTimer->start(UPDATE_TIMEOUT);
}

void VCCueList::slotFunctionNameChanged(quint32 fid)
{
    if (fid == m_chaserID)
        m_updateTimer->start(UPDATE_TIMEOUT);
    else
    {
        // fid might be an ID of a ChaserStep of m_chaser
        Chaser* ch = chaser();
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

    Chaser* ch = chaser();
    if (ch == NULL)
        return;

    if (ch->isRunning())
    {
        //stopChaser();
        if (ch->isPaused())
        {
            m_playbackButton->setStyleSheet(QString("QToolButton{ background: %1; }")
                                            .arg(m_stopButton->palette().background().color().name()));
            m_playbackButton->setIcon(QIcon(":/player_pause.png"));
        }
        else
        {
            m_playbackButton->setStyleSheet("QToolButton{ background: #5B81FF; }");
            m_playbackButton->setIcon(QIcon(":/player_play.png"));
        }
        ch->setPause(!ch->isPaused());
    }
    else
    {
        if (m_tree->currentItem() != NULL)
            startChaser(getCurrentIndex());
        else
            startChaser();
    }
}

void VCCueList::slotStop()
{
    if (mode() != Doc::Operate)
        return;

    Chaser* ch = chaser();
    if (ch == NULL)
        return;

    if (ch->isRunning())
    {
        stopChaser();
    }
    else
    {
        m_primaryIndex = 0;
        m_tree->setCurrentItem(m_tree->topLevelItem(getFirstIndex()));
    }
    m_playbackButton->setStyleSheet(QString("QToolButton{ background: %1; }")
                                    .arg(m_stopButton->palette().background().color().name()));
    m_progress->setFormat("");
    m_progress->setValue(0);
}

void VCCueList::slotNextCue()
{
    if (mode() != Doc::Operate)
        return;

    Chaser* ch = chaser();
    if (ch == NULL)
        return;

    if (ch->isRunning())
    {
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

    Chaser* ch = chaser();
    if (ch == NULL)
        return;

    if (ch->isRunning())
    {
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
    // Chaser is being edited, channels cound may change.
    // Wait for the CueList to update its steps.
    if (m_updateTimer->isActive())
        return;
    Q_ASSERT(stepNumber < m_tree->topLevelItemCount() && stepNumber >= 0);
    QTreeWidgetItem* item = m_tree->topLevelItem(stepNumber);
    Q_ASSERT(item != NULL);
    m_tree->scrollToItem(item, QAbstractItemView::PositionAtCenter);
    m_tree->setCurrentItem(item);
    m_primaryIndex = stepNumber;
    if (slidersMode() == Steps)
    {
        m_sl1BottomLabel->setStyleSheet(cfLabelBlueStyle);
        m_sl1BottomLabel->setText(QString("#%1").arg(m_primaryIndex + 1));

        float stepVal;
        int stepsCount = m_tree->topLevelItemCount();
        if (stepsCount < 256)
            stepVal = 255.0 / (float)stepsCount;
        else
            stepVal = 1.0;
        int slValue = (stepVal * (float)stepNumber);
        if (slValue > 255)
            slValue = 255;

        //qDebug() << "Slider value:" << m_slider1->value() << "Step range:" << (255 - slValue) << (255 - slValue - stepVal);
        // if the Step slider is already in range, then do not set its value
        // this means a user interaction is going on, either with the mouse or external controller
        if (m_slider1->value() < (255 - slValue - stepVal) || m_slider1->value() > (255 - slValue))
        {
            m_slider1->blockSignals(true);
            m_slider1->setValue(255 - slValue);
            m_sl1TopLabel->setText(QString("%1").arg(slValue));
            m_slider1->blockSignals(false);
        }
    }
    else
        setSlidersInfo(m_primaryIndex);
    emit stepChanged(m_primaryIndex);
}

void VCCueList::slotItemActivated(QTreeWidgetItem* item)
{
    if (mode() != Doc::Operate)
        return;

    playCueAtIndex(m_tree->indexOfTopLevelItem(item));
}

void VCCueList::slotItemChanged(QTreeWidgetItem *item, int column)
{
    if (m_listIsUpdating || column != COL_NOTES)
        return;

    Chaser* ch = chaser();
    if (ch == NULL)
        return;

    QString itemText = item->text(column);
    int idx = m_tree->indexOfTopLevelItem(item);
    ChaserStep step = ch->steps().at(idx);

    step.note = itemText;
    ch->replaceStep(step, idx);
}

void VCCueList::slotFunctionRunning(quint32 fid)
{
    if (fid == m_chaserID)
    {
        m_playbackButton->setIcon(QIcon(":/player_pause.png"));
        m_timer->start(PROGRESS_INTERVAL);
        updateFeedback();
    }
}

void VCCueList::slotFunctionStopped(quint32 fid)
{
    if (fid == m_chaserID)
    {
        m_playbackButton->setIcon(QIcon(":/player_play.png"));
        m_sl1BottomLabel->setText("");
        m_sl1BottomLabel->setStyleSheet(cfLabelNoStyle);
        m_sl2BottomLabel->setText("");
        m_sl2BottomLabel->setStyleSheet(cfLabelNoStyle);
        // reset any previously set background
        QTreeWidgetItem *item = m_tree->topLevelItem(m_secondaryIndex);
        if (item != NULL)
            item->setBackground(COL_NUM, m_defCol);

        emit stepChanged(-1);

        qDebug() << Q_FUNC_INFO << "Cue stopped";
        updateFeedback();
    }
}

void VCCueList::slotProgressTimeout()
{
    Chaser* ch = chaser();
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
            }
            else
            {
                m_progress->setValue(m_progress->maximum());
                m_progress->setFormat("");
            }
            return;
        }
        else
        {
            double progress = ((double)step.m_elapsed / (double)step.m_duration) * (double)m_progress->width();
            m_progress->setFormat(QString("-%1").arg(Function::speedToString(step.m_duration - step.m_elapsed)));
            m_progress->setValue(progress);
        }
    }
    else
        m_progress->setValue(0);
}

void VCCueList::startChaser(int startIndex)
{
    Chaser* ch = chaser();
    if (ch == NULL)
        return;
    ch->setStepIndex(startIndex);
    ch->setStartIntensity(getPrimaryIntensity());
    ch->adjustAttribute(intensity(), Function::Intensity);
    ch->start(m_doc->masterTimer(), functionParent());
    emit functionStarting(m_chaserID);
}

void VCCueList::stopChaser()
{
    Chaser* ch = chaser();
    if (ch == NULL)
        return;
    ch->stop(functionParent());
}

void VCCueList::setNextPrevBehavior(unsigned int nextPrev)
{
    Q_ASSERT(nextPrev == DefaultRunFirst
            || nextPrev == RunNext
            || nextPrev == Select
            || nextPrev == Nothing);
    m_nextPrevBehavior = nextPrev;
}

unsigned int VCCueList::nextPrevBehavior() const
{
    return m_nextPrevBehavior;
}

VCCueList::SlidersMode VCCueList::slidersMode() const
{
    return m_slidersMode;
}

void VCCueList::setSlidersMode(VCCueList::SlidersMode mode)
{
    m_slidersMode = mode;

    if (m_slider1->isVisible() == true)
    {
        bool show = (mode == Crossfade) ? true : false;
        m_linkCheck->setVisible(show);
        m_sl2TopLabel->setVisible(show);
        m_slider2->setVisible(show);
        m_sl2BottomLabel->setVisible(show);
    }
    if (mode == Steps)
    {
        m_slider1->setMaximum(255);
        m_slider1->setValue(255);
    }
    else
    {
        m_slider1->setMaximum(100);
        m_slider1->setValue(100);
    }
}

VCCueList::SlidersMode VCCueList::stringToSlidersMode(QString modeStr)
{
    if (modeStr == "Steps")
        return Steps;

    return Crossfade;
}

QString VCCueList::slidersModeToString(VCCueList::SlidersMode mode)
{
    if (mode == Steps)
        return "Steps";

    return "Crossfade";
}

/*****************************************************************************
 * Crossfade
 *****************************************************************************/
void VCCueList::setSlidersInfo(int index)
{
    Chaser* ch = chaser();
    if (ch == NULL || !ch->isRunning())
        return;

    int tmpIndex = ch->computeNextStep(index);

    m_sl1BottomLabel->setText(QString("#%1").arg(m_primaryLeft ? index + 1 : tmpIndex + 1));
    m_sl1BottomLabel->setStyleSheet(m_primaryLeft ? cfLabelBlueStyle : cfLabelOrangeStyle);

    m_sl2BottomLabel->setText(QString("#%1").arg(m_primaryLeft ? tmpIndex + 1 : index + 1));
    m_sl2BottomLabel->setStyleSheet(m_primaryLeft ? cfLabelOrangeStyle : cfLabelBlueStyle);

    // reset any previously set background
    QTreeWidgetItem *item = m_tree->topLevelItem(m_secondaryIndex);
    if (item != NULL)
        item->setBackground(COL_NUM, m_defCol);

    item = m_tree->topLevelItem(tmpIndex);
    if (item != NULL)
        item->setBackground(COL_NUM, QColor("#FF8000"));
    m_secondaryIndex = tmpIndex;
}

void VCCueList::slotShowCrossfadePanel(bool enable)
{
    m_sl1TopLabel->setVisible(enable);
    m_slider1->setVisible(enable);
    m_sl1BottomLabel->setVisible(enable);
    if (slidersMode() == Crossfade)
    {
        m_linkCheck->setVisible(enable);
        m_sl2TopLabel->setVisible(enable);
        m_slider2->setVisible(enable);
        m_sl2BottomLabel->setVisible(enable);
    }
}

void VCCueList::slotSlider1ValueChanged(int value)
{
    if (slidersMode() == Steps)
    {
        value = 255 - value;
        m_sl1TopLabel->setText(QString("%1").arg(value));
        Chaser* ch = chaser();
        if (ch == NULL || ch->stopped())
            return;
        int newStep = value; // by default we assume the Chaser has more than 256 steps
        if (ch->stepsCount() < 256)
        {
            float stepSize = 255.0 / (float)ch->stepsCount();
            if(value >= 255.0 - stepSize)
                newStep = ch->stepsCount() - 1;
            else
                newStep = qFloor((float)value / stepSize);
        }
        //qDebug() << "value:" << value << "steps:" << ch->stepsCount() << "new step:" << newStep;

        if (newStep == ch->currentStepIndex())
            return; // nothing to do

        ch->setStepIndex(newStep);
    }
    else
    {
        m_sl1TopLabel->setText(QString("%1%").arg(value));

        Chaser* ch = chaser();
        if (!(ch == NULL || ch->stopped()))
        {
            ch->adjustIntensity((qreal)value / 100, m_primaryLeft ? m_primaryIndex : m_secondaryIndex);
            stopStepIfNeeded(ch);
        }

        if (m_linkCheck->isChecked())
            m_slider2->setValue(100 - value);
    }

    updateFeedback();
}

void VCCueList::slotSlider2ValueChanged(int value)
{
    if (slidersMode() == Steps)
    {
        qWarning() << "[VCCueList] ERROR ! Slider2 value change should never happen !";
        return;
    }
    m_sl2TopLabel->setText(QString("%1%").arg(value));

    Chaser* ch = chaser();
    if (!(ch == NULL || ch->stopped()))
    {
        ch->adjustIntensity((qreal)value / 100, m_primaryLeft ? m_secondaryIndex : m_primaryIndex);
        stopStepIfNeeded(ch);
    }

    if (m_linkCheck->isChecked())
        m_slider1->setValue(100 - value);

    updateFeedback();
}

void VCCueList::stopStepIfNeeded(Chaser* ch)
{
    if (ch->runningStepsNumber() != 2)
        return;

    int primaryValue;
    int secondaryValue;
    if (m_primaryLeft)
    {
        primaryValue = m_slider1->value();
        secondaryValue = m_slider2->value();
    }
    else
    {
        primaryValue = m_slider2->value();
        secondaryValue = m_slider1->value();
    }

    if (primaryValue == 0)
    {
        m_primaryLeft = !m_primaryLeft;
        ch->stopStep(m_primaryIndex);
    }
    else if (secondaryValue == 0)
    {
        ch->stopStep(m_secondaryIndex);
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
    if (isEnabled() == false)
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
    int fbv = (int)SCALE(float(m_slider1->value()), float(0), float(100), float(0), float(UCHAR_MAX));
    sendFeedback(fbv, cf1InputSourceId);
    fbv = (int)SCALE(float(100 - m_slider2->value()), float(0), float(100), float(0), float(UCHAR_MAX));
    sendFeedback(fbv, cf2InputSourceId);

    Chaser* ch = chaser();
    if (ch == NULL)
        return;

    sendFeedback(ch->isRunning() ? UCHAR_MAX : 0, playbackInputSourceId);
}

/*****************************************************************************
 * External Input
 *****************************************************************************/

void VCCueList::slotInputValueChanged(quint32 universe, quint32 channel, uchar value)
{
    if (mode() == Doc::Design || isEnabled() == false)
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
    else if (checkInputSource(universe, pagedCh, value, sender(), cf1InputSourceId))
    {
        float val = SCALE((float) value, (float) 0, (float) UCHAR_MAX,
                          (float) m_slider1->minimum(),
                          (float) m_slider1->maximum());
        m_slider1->setValue(val);
    }
    else if (checkInputSource(universe, pagedCh, value, sender(), cf2InputSourceId))
    {
        float val = SCALE((float) value, (float) 0, (float) UCHAR_MAX,
                          (float) m_slider2->minimum(),
                          (float) m_slider2->maximum());
        m_slider2->setValue(100 - val);
    }
}

/*****************************************************************************
 * VCWidget-inherited methods
 *****************************************************************************/

void VCCueList::adjustIntensity(qreal val)
{
    Chaser* ch = chaser();
    if (ch != NULL)
    {
        ch->adjustAttribute(val, Function::Intensity);

        // Refresh intensity of current steps
        if (!ch->stopped() && slidersMode() == Crossfade)
        {
            if (m_slider1->value() != 0)
                ch->adjustIntensity((qreal)m_slider1->value() / 100, m_primaryLeft ? m_primaryIndex : m_secondaryIndex);
            if (m_slider2->value() != 0)
                ch->adjustIntensity((qreal)m_slider2->value() / 100, m_primaryLeft ? m_secondaryIndex : m_primaryIndex);
        }
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
        m_sl1BottomLabel->setStyleSheet(cfLabelNoStyle);
        m_sl1BottomLabel->setText("");
        m_sl2BottomLabel->setStyleSheet(cfLabelNoStyle);
        m_sl2BottomLabel->setText("");
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

    Chaser* ch = chaser();
    if (ch == NULL)
        return;

    if (ch->isRunning())
    {
        ch->setCurrentStep(m_primaryIndex, getPrimaryIntensity());
    }
    else
    {
        startChaser(m_primaryIndex);
    }

    if (slidersMode() == Crossfade)
        setSlidersInfo(m_primaryIndex);
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
            setSlidersMode(stringToSlidersMode(root.readElementText()));
        }
        else if (root.name() == KXMLQLCVCCueListCrossfadeLeft)
        {
            loadXMLSources(root, cf1InputSourceId);
        }
        else if (root.name() == KXMLQLCVCCueListCrossfadeRight)
        {
            loadXMLSources(root, cf2InputSourceId);
        }
        else if (root.name() == KXMLQLCVCWidgetKey) /* Legacy */
        {
            setNextKeySequence(QKeySequence(root.readElementText()));
        }
        else if (root.name() == KXMLQLCVCCueListChaser)
        {
            setChaser(root.readElementText().toUInt());
        }
        else if (root.name() == KXMLQLCVCCueListNextPrevBehavior)
        {
            unsigned int nextPrev = root.readElementText().toUInt();
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
        Chaser* chaser = new Chaser(m_doc);
        chaser->setName(caption());

        // Legacy cue lists relied on individual functions' timings and a common hold time
        chaser->setFadeInMode(Chaser::Default);
        chaser->setFadeOutMode(Chaser::Default);
        chaser->setDurationMode(Chaser::Common);

        foreach (quint32 id, legacyStepList)
        {
            Function* function = m_doc->function(id);
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
    doc->writeTextElement(KXMLQLCVCCueListSlidersMode, slidersModeToString(slidersMode()));

    QSharedPointer<QLCInputSource> cf1Src = inputSource(cf1InputSourceId);
    if (!cf1Src.isNull() && cf1Src->isValid())
    {
        doc->writeStartElement(KXMLQLCVCCueListCrossfadeLeft);
        saveXMLInput(doc, cf1Src);
        doc->writeEndElement();
    }
    QSharedPointer<QLCInputSource> cf2Src = inputSource(cf2InputSourceId);
    if (!cf2Src.isNull() && cf2Src->isValid())
    {
        doc->writeStartElement(KXMLQLCVCCueListCrossfadeRight);
        saveXMLInput(doc, cf2Src);
        doc->writeEndElement();
    }

    /* End the <CueList> tag */
    doc->writeEndElement();

    return true;
}
