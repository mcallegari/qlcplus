/*
  Q Light Controller
  vccuelist.cpp

  Copyright (c) Heikki Junnila, Massimo Callegari

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
#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QHeaderView>
#include <QGridLayout>
#include <QProgressBar>
#include <QCheckBox>
#include <QString>
#include <QLabel>
#include <QDebug>
#include <QTimer>
#include <QtXml>

#include "vccuelistproperties.h"
#include "vcpropertieseditor.h"
#include "clickandgoslider.h"
#include "qlcinputchannel.h"
#include "virtualconsole.h"
#include "mastertimer.h"
#include "chaserstep.h"
#include "inputpatch.h"
#include "vccuelist.h"
#include "qlcmacros.h"
#include "function.h"
#include "qlcfile.h"
#include "apputil.h"
#include "chaser.h"
#include "chaserrunner.h"
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

const QString progressDisabledStyle =
        "QProgressBar { border: 2px solid #C3C3C3; border-radius: 4px; background-color: #DCDCDC;";
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
    //, m_chaser(NULL)
    , m_timer(NULL)
    , m_primaryIndex(0)
    , m_secondaryIndex(0)
    , m_primaryLeft(true)
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
    m_slider1->setStyleSheet(CNG_DEFAULT_STYLE);
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
    m_slider2->setStyleSheet(CNG_DEFAULT_STYLE);
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
    m_playbackButton->setToolTip(tr("Play/Stop Cue list"));
    connect(m_playbackButton, SIGNAL(clicked()), this, SLOT(slotPlayback()));
    hbox->addWidget(m_playbackButton);

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
}

VCCueList::~VCCueList()
{
}

void VCCueList::enableWidgetUI(bool enable)
{
    m_tree->setEnabled(enable);
    m_playbackButton->setEnabled(enable);
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
    if (chaser != NULL && !chaser->stopped())
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

    m_tree->resizeColumnToContents(COL_NUM);
    m_tree->resizeColumnToContents(COL_NAME);
    m_tree->resizeColumnToContents(COL_FADEIN);
    m_tree->resizeColumnToContents(COL_FADEOUT);
    m_tree->resizeColumnToContents(COL_DURATION);
    m_tree->resizeColumnToContents(COL_NOTES);

    m_listIsUpdating = false;
}

int VCCueList::getCurrentIndex()
{
    QList <QTreeWidgetItem*> selected(m_tree->selectedItems());
    int index = 0;
    if (selected.size() > 0)
    {
        QTreeWidgetItem* item(selected.first());
        index = m_tree->indexOfTopLevelItem(item);
    }
    return index;
}

void VCCueList::notifyFunctionStarting(quint32 fid)
{
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
    if (mode() == Doc::Design)
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
        startChaser(getCurrentIndex());
    }
}

void VCCueList::slotNextCue()
{
    if (mode() != Doc::Operate)
        return;

    Chaser* ch = chaser();
    if (ch == NULL)
        return;

    /* Create the runner only when the first/last cue is engaged. */
    if (ch->isRunning())
    {
        ch->next();
    }
    else
    {
        startChaser();
    }
}

void VCCueList::slotPreviousCue()
{
    if (mode() != Doc::Operate)
        return;

    Chaser* ch = chaser();
    if (ch == NULL)
        return;

    /* Create the runner only when the first/last cue is engaged. */
    if (ch->isRunning())
    {
        ch->previous();
    }
    else
    {
        startChaser(m_tree->topLevelItemCount() - 1);
    }
}

void VCCueList::slotCurrentStepChanged(int stepNumber)
{
    Q_ASSERT(stepNumber < m_tree->topLevelItemCount() && stepNumber >= 0);
    QTreeWidgetItem* item = m_tree->topLevelItem(stepNumber);
    Q_ASSERT(item != NULL);
    m_tree->scrollToItem(item, QAbstractItemView::PositionAtCenter);
    m_tree->setCurrentItem(item);
    m_primaryIndex = stepNumber;
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
        m_playbackButton->setIcon(QIcon(":/player_stop.png"));
        m_timer->start(PROGRESS_INTERVAL);
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
    }
}

void VCCueList::slotProgressTimeout()
{
    Chaser* ch = chaser();
    if (ch == NULL || ch->stopped())
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
    ch->setStartIntensity((qreal)m_slider1->value() / 100.0);
    ch->start(m_doc->masterTimer());
    emit functionStarting(m_chaserID);
}

void VCCueList::stopChaser()
{
    Chaser* ch = chaser();
    if (ch == NULL)
        return;
    ch->stop();
}

/*****************************************************************************
 * Crossfade
 *****************************************************************************/
void VCCueList::setSlidersInfo(int index)
{
    Chaser* ch = chaser();
    if (ch == NULL || ch->stopped())
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
    m_linkCheck->setVisible(enable);
    m_sl1TopLabel->setVisible(enable);
    m_slider1->setVisible(enable);
    m_sl1BottomLabel->setVisible(enable);
    m_sl2TopLabel->setVisible(enable);
    m_slider2->setVisible(enable);
    m_sl2BottomLabel->setVisible(enable);
}

void VCCueList::slotSlider1ValueChanged(int value)
{
    bool switchFunction = false;
    m_sl1TopLabel->setText(QString("%1%").arg(value));
    if (m_linkCheck->isChecked())
        m_slider2->setValue(100 - value);

    Chaser* ch = chaser();
    if (ch == NULL || ch->stopped())
        return;

    ch->adjustIntensity((qreal)value / 100, m_primaryLeft ? m_primaryIndex: m_secondaryIndex);

    if(ch->runningStepsNumber() == 2)
    {
        if (m_primaryLeft == true && value == 0 && m_slider2->value() == 100)
        {
            ch->stopStep( m_primaryLeft ? m_primaryIndex: m_secondaryIndex);
            m_primaryLeft = false;
            switchFunction = true;
        }
        else if (m_primaryLeft == false && value == 100 && m_slider2->value() == 0)
        {
            ch->stopStep(m_primaryLeft ? m_secondaryIndex : m_primaryIndex);
            m_primaryLeft = true;
            switchFunction = true;
        }
    }

    if (switchFunction)
    {
        m_primaryIndex = m_secondaryIndex;
        QTreeWidgetItem* item = m_tree->topLevelItem(m_primaryIndex);
        if (item != NULL)
        {
            m_tree->scrollToItem(item, QAbstractItemView::PositionAtCenter);
            m_tree->setCurrentItem(item);
        }
        setSlidersInfo(m_primaryIndex);
    }
    updateFeedback();
}

void VCCueList::slotSlider2ValueChanged(int value)
{
    bool switchFunction = false;
    m_sl2TopLabel->setText(QString("%1%").arg(value));
    if (m_linkCheck->isChecked())
        m_slider1->setValue(100 - value);

    Chaser* ch = chaser();
    if (ch == NULL || ch->stopped())
        return;

    ch->adjustIntensity((qreal)value / 100, m_primaryLeft ? m_secondaryIndex : m_primaryIndex);

    if (ch->runningStepsNumber() == 2)
    {
        if (m_primaryLeft == false && value == 0 && m_slider1->value() == 100)
        {
            ch->stopStep(m_primaryLeft ? m_secondaryIndex : m_primaryIndex);
            m_primaryLeft = true;
            switchFunction = true;
        }
        else if (m_primaryLeft == true && value == 100 && m_slider1->value() == 0)
        {
            ch->stopStep( m_primaryLeft ? m_primaryIndex: m_secondaryIndex);
            m_primaryLeft = false;
            switchFunction = true;
        }
    }

    if (switchFunction)
    {
        m_primaryIndex = m_secondaryIndex;
        QTreeWidgetItem* item = m_tree->topLevelItem(m_primaryIndex);
        if (item != NULL)
        {
            m_tree->scrollToItem(item, QAbstractItemView::PositionAtCenter);
            m_tree->setCurrentItem(item);
        }
        setSlidersInfo(m_primaryIndex);
    }
    updateFeedback();
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
}

void VCCueList::updateFeedback()
{
    int fbv = (int)SCALE(float(m_slider1->value()), float(0), float(100), float(0), float(UCHAR_MAX));
    sendFeedback(fbv, cf1InputSourceId);
    fbv = (int)SCALE(float(100 - m_slider2->value()), float(0), float(100), float(0), float(UCHAR_MAX));
    sendFeedback(fbv, cf2InputSourceId);
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

    if (!ch->stopped())
    {
        ch->setCurrentStep(m_primaryIndex, (qreal)m_slider1->value() / 100);
    }
    else
    {
        startChaser(m_primaryIndex);
    }

    setSlidersInfo(m_primaryIndex);
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCCueList::loadXML(const QDomElement* root)
{
    QList <quint32> legacyStepList;

    QDomNode node;
    QDomElement tag;
    QString str;

    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCVCCueList)
    {
        qWarning() << Q_FUNC_INFO << "CueList node not found";
        return false;
    }

    /* Widget commons */
    loadXMLCommon(root);

    /* Children */
    node = root->firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();
        if (tag.tagName() == KXMLQLCWindowState)
        {
            bool visible = false;
            int x = 0, y = 0, w = 0, h = 0;
            loadXMLWindowState(&tag, &x, &y, &w, &h, &visible);
            setGeometry(x, y, w, h);
        }
        else if (tag.tagName() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(&tag);
        }
        else if (tag.tagName() == KXMLQLCVCCueListNext)
        {
            QDomNode subNode = tag.firstChild();
            while (subNode.isNull() == false)
            {
                QDomElement subTag = subNode.toElement();
                if (subTag.tagName() == KXMLQLCVCWidgetInput)
                {
                    quint32 uni = 0, ch = 0;
                    if (loadXMLInput(subTag, &uni, &ch) == true)
                        setInputSource(new QLCInputSource(uni, ch), nextInputSourceId);
                }
                else if (subTag.tagName() == KXMLQLCVCCueListKey)
                {
                    m_nextKeySequence = stripKeySequence(QKeySequence(subTag.text()));
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << "Unknown CueList Next tag" << subTag.tagName();
                }

                subNode = subNode.nextSibling();
            }
        }
        else if (tag.tagName() == KXMLQLCVCCueListPrevious)
        {
            QDomNode subNode = tag.firstChild();
            while (subNode.isNull() == false)
            {
                QDomElement subTag = subNode.toElement();
                if (subTag.tagName() == KXMLQLCVCWidgetInput)
                {
                    quint32 uni = 0, ch = 0;
                    if (loadXMLInput(subTag, &uni, &ch) == true)
                        setInputSource(new QLCInputSource(uni, ch), previousInputSourceId);
                }
                else if (subTag.tagName() == KXMLQLCVCCueListKey)
                {
                    m_previousKeySequence = stripKeySequence(QKeySequence(subTag.text()));
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << "Unknown CueList Previous tag" << subTag.tagName();
                }

                subNode = subNode.nextSibling();
            }
        }
        else if (tag.tagName() == KXMLQLCVCCueListPlayback)
        {
            QDomNode subNode = tag.firstChild();
            while (subNode.isNull() == false)
            {
                QDomElement subTag = subNode.toElement();
                if (subTag.tagName() == KXMLQLCVCWidgetInput)
                {
                    quint32 uni = 0, ch = 0;
                    if (loadXMLInput(subTag, &uni, &ch) == true)
                        setInputSource(new QLCInputSource(uni, ch), playbackInputSourceId);
                }
                else if (subTag.tagName() == KXMLQLCVCCueListKey)
                {
                    m_playbackKeySequence = stripKeySequence(QKeySequence(subTag.text()));
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << "Unknown CueList Stop tag" << subTag.tagName();
                }

                subNode = subNode.nextSibling();
            }
        }
        else if (tag.tagName() == KXMLQLCVCCueListCrossfadeLeft)
        {
            QDomNode subNode = tag.firstChild();
            if (subNode.isNull() == false)
            {
                QDomElement subTag = subNode.toElement();
                if (subTag.tagName() == KXMLQLCVCWidgetInput)
                {
                    quint32 uni = 0, ch = 0;
                    if (loadXMLInput(subTag, &uni, &ch) == true)
                        setInputSource(new QLCInputSource(uni, ch), cf1InputSourceId);
                }
            }
        }
        else if (tag.tagName() == KXMLQLCVCCueListCrossfadeRight)
        {
            QDomNode subNode = tag.firstChild();
            if (subNode.isNull() == false)
            {
                QDomElement subTag = subNode.toElement();
                if (subTag.tagName() == KXMLQLCVCWidgetInput)
                {
                    quint32 uni = 0, ch = 0;
                    if (loadXMLInput(subTag, &uni, &ch) == true)
                        setInputSource(new QLCInputSource(uni, ch), cf2InputSourceId);
                }
            }
        }
        else if (tag.tagName() == KXMLQLCVCCueListKey) /* Legacy */
        {
            setNextKeySequence(QKeySequence(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCVCCueListChaser)
        {
            setChaser(tag.text().toUInt());
        }
        else if (tag.tagName() == KXMLQLCVCCueListFunction)
        {
            // Collect legacy file format steps into a list
            legacyStepList << tag.text().toUInt();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown cuelist tag:" << tag.tagName();
        }

        node = node.nextSibling();
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

bool VCCueList::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
    QDomElement root;
    QDomElement tag;
    QDomElement subtag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(vc_root != NULL);

    /* VC CueList entry */
    root = doc->createElement(KXMLQLCVCCueList);
    vc_root->appendChild(root);

    saveXMLCommon(doc, &root);

    /* Chaser */
    tag = doc->createElement(KXMLQLCVCCueListChaser);
    root.appendChild(tag);
    text = doc->createTextNode(QString::number(chaserID()));
    tag.appendChild(text);

    /* Next cue */
    tag = doc->createElement(KXMLQLCVCCueListNext);
    root.appendChild(tag);
    subtag = doc->createElement(KXMLQLCVCCueListKey);
    tag.appendChild(subtag);
    text = doc->createTextNode(m_nextKeySequence.toString());
    subtag.appendChild(text);
    saveXMLInput(doc, &tag, inputSource(nextInputSourceId));

    /* Previous cue */
    tag = doc->createElement(KXMLQLCVCCueListPrevious);
    root.appendChild(tag);
    subtag = doc->createElement(KXMLQLCVCCueListKey);
    tag.appendChild(subtag);
    text = doc->createTextNode(m_previousKeySequence.toString());
    subtag.appendChild(text);
    saveXMLInput(doc, &tag, inputSource(previousInputSourceId));

    /* Stop cue list */
    tag = doc->createElement(KXMLQLCVCCueListPlayback);
    root.appendChild(tag);
    subtag = doc->createElement(KXMLQLCVCCueListKey);
    tag.appendChild(subtag);
    text = doc->createTextNode(m_playbackKeySequence.toString());
    subtag.appendChild(text);
    saveXMLInput(doc, &tag, inputSource(playbackInputSourceId));

    /* Crossfade cue list */
    QLCInputSource *cf1Src = inputSource(cf1InputSourceId);
    if (cf1Src != NULL && cf1Src->isValid())
    {
        tag = doc->createElement(KXMLQLCVCCueListCrossfadeLeft);
        root.appendChild(tag);
        saveXMLInput(doc, &tag, inputSource(cf1InputSourceId));
    }
    QLCInputSource *cf2Src = inputSource(cf2InputSourceId);
    if (cf2Src != NULL && cf2Src->isValid())
    {
        tag = doc->createElement(KXMLQLCVCCueListCrossfadeRight);
        root.appendChild(tag);
        saveXMLInput(doc, &tag, inputSource(cf2InputSourceId));
    }

    /* Window state */
    saveXMLWindowState(doc, &root);

    /* Appearance */
    saveXMLAppearance(doc, &root);

    return true;
}
