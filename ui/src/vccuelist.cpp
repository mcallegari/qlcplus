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
#include <QCheckBox>
#include <QString>
#include <QLabel>
#include <QDebug>
#include <QtXml>

#include "vccuelistproperties.h"
#include "vcpropertieseditor.h"
#include "clickandgoslider.h"
#include "qlcinputchannel.h"
#include "virtualconsole.h"
#include "cuelistrunner.h"
#include "mastertimer.h"
#include "chaserstep.h"
#include "inputpatch.h"
#include "vccuelist.h"
#include "qlcmacros.h"
#include "function.h"
#include "inputmap.h"
#include "qlcfile.h"
#include "apputil.h"
#include "chaser.h"
#include "doc.h"

#define COL_NUM      0
#define COL_NAME     1
#define COL_FADEIN   2
#define COL_FADEOUT  3
#define COL_DURATION 4
#define COL_NOTES    5

#define PROP_ID  Qt::UserRole
#define HYSTERESIS 3 // Hysteresis for next/previous external input

const quint8 VCCueList::nextInputSourceId = 0;
const quint8 VCCueList::previousInputSourceId = 1;
const quint8 VCCueList::playbackInputSourceId = 2;
const quint8 VCCueList::cf1InputSourceId = 3;
const quint8 VCCueList::cf2InputSourceId = 4;

VCCueList::VCCueList(QWidget* parent, Doc* doc) : VCWidget(parent, doc)
    , m_chaserID(Function::invalidId())
    , m_runner(NULL)
    , m_primaryIndex(0)
    , m_secondaryIndex(0)
    , m_primaryLeft(true)
    , m_stop(false)
{
    /* Set the class name "VCCueList" as the object name as well */
    setObjectName(VCCueList::staticMetaObject.className());

    /* Create a layout for this widget */
    QGridLayout* grid = new QGridLayout(this);
    grid->setSpacing(2);

    m_linkCheck = new QCheckBox(tr("Link"));
    grid->addWidget(m_linkCheck, 0, 0, 1, 2, Qt::AlignVCenter | Qt::AlignCenter);

    m_blueStyle = "QLabel { background-color: #4E8DDE; color: white; border: 1px solid; border-radius: 3px; font: bold; }";
    m_orangeStyle = "QLabel { background-color: orange; color: black; border: 1px solid; border-radius: 3px; font: bold; }";
    m_noStyle = "QLabel { border: 1px solid; border-radius: 3px; font: bold; }";

    m_sl1TopLabel = new QLabel("100%");
    m_sl1TopLabel->setAlignment(Qt::AlignHCenter);
    grid->addWidget(m_sl1TopLabel, 1, 0, 1, 1);
    m_slider1 = new ClickAndGoSlider();
    m_slider1->setStyle(AppUtil::saneStyle());
    m_slider1->setFixedWidth(40);
    m_slider1->setRange(0, 100);
    m_slider1->setValue(100);
    grid->addWidget(m_slider1, 2, 0, 1, 1);
    m_sl1BottomLabel = new QLabel("");
    m_sl1BottomLabel->setStyleSheet(m_noStyle);
    m_sl1BottomLabel->setAlignment(Qt::AlignCenter);
    grid->addWidget(m_sl1BottomLabel, 3, 0, 1, 1);
    connect(m_slider1, SIGNAL(valueChanged(int)),
            this, SLOT(slotSlider1ValueChanged(int)));

    m_sl2TopLabel = new QLabel("0%");
    m_sl2TopLabel->setAlignment(Qt::AlignHCenter);
    grid->addWidget(m_sl2TopLabel, 1, 1, 1, 1);
    m_slider2 = new ClickAndGoSlider();
    m_slider2->setStyle(AppUtil::saneStyle());
    m_slider2->setFixedWidth(40);
    m_slider2->setRange(0, 100);
    m_slider2->setValue(0);
    m_slider2->setInvertedAppearance(true);
    grid->addWidget(m_slider2, 2, 1, 1, 1);
    m_sl2BottomLabel = new QLabel("");
    m_sl2BottomLabel->setStyleSheet(m_noStyle);
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

    /* Create control buttons */
    QHBoxLayout *hbox = new QHBoxLayout(this);
    hbox->setSpacing(2);

    m_crossfadeButton = new QToolButton(this);
    m_crossfadeButton->setIcon(QIcon(":/slider.png"));
    m_crossfadeButton->setIconSize(QSize(24, 24));
    m_crossfadeButton->setCheckable(true);
    m_crossfadeButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_crossfadeButton->setFixedHeight(32);
    m_crossfadeButton->setToolTip(tr("Show/Hide crossfade sliders"));
    connect(m_crossfadeButton, SIGNAL(toggled(bool)), this, SLOT(slotShowCrossfadePanel(bool)));
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

    grid->addItem(hbox, 3, 2);

    setFrameStyle(KVCFrameStyleSunken);
    setType(VCWidget::CueListWidget);
    setCaption(tr("Cue list"));

    QSettings settings;
    QVariant var = settings.value(SETTINGS_CUELIST_SIZE);
    if (var.isValid() == true)
        resize(var.toSize());
    else
        resize(QSize(300, 220));

    slotModeChanged(mode());

    connect(m_doc, SIGNAL(functionRemoved(quint32)),
            this, SLOT(slotFunctionRemoved(quint32)));
    connect(m_doc, SIGNAL(functionChanged(quint32)),
            this, SLOT(slotFunctionChanged(quint32)));

    m_nextLatestValue = 0;
    m_previousLatestValue = 0;
    m_playbackLatestValue = 0;
}

VCCueList::~VCCueList()
{
    m_doc->masterTimer()->unregisterDMXSource(this);
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

bool VCCueList::copyFrom(VCWidget* widget)
{
    VCCueList* cuelist = qobject_cast<VCCueList*> (widget);
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
        disconnect(old, SIGNAL(stopped(quint32)),
            this, SLOT(slotChaserStopped(quint32)));
    }

    Chaser* chaser = qobject_cast<Chaser*> (m_doc->function(id));

    if (chaser == NULL)
        m_chaserID = Function::invalidId();
    else
    {
        m_chaserID = id;
        connect(chaser, SIGNAL(stopped(quint32)),
                    this, SLOT(slotFunctionStopped(quint32)));
    }
    updateStepList();
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

    Chaser* chaser = qobject_cast<Chaser*> (m_doc->function(m_chaserID));
    if (chaser == NULL)
        return;

    QListIterator <ChaserStep> it(chaser->steps());
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

        switch (chaser->fadeInMode())
        {
            case Chaser::Common:
                item->setText(COL_FADEIN, Function::speedToString(chaser->fadeInSpeed()));
                break;
            case Chaser::PerStep:
                item->setText(COL_FADEIN, Function::speedToString(step.fadeIn));
                break;
            default:
            case Chaser::Default:
                item->setText(COL_FADEIN, QString());
        }

        switch (chaser->fadeOutMode())
        {
            case Chaser::Common:
                item->setText(COL_FADEOUT, Function::speedToString(chaser->fadeOutSpeed()));
                break;
            case Chaser::PerStep:
                item->setText(COL_FADEOUT, Function::speedToString(step.fadeOut));
                break;
            default:
            case Chaser::Default:
                item->setText(COL_FADEOUT, QString());
        }

        switch (chaser->durationMode())
        {
            case Chaser::Common:
                item->setText(COL_DURATION, Function::speedToString(chaser->duration()));
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

void VCCueList::slotFunctionRemoved(quint32 fid)
{
    if (fid == m_chaserID)
    {
        setChaser(Function::invalidId());
        updateStepList();
    }
}

void VCCueList::slotFunctionChanged(quint32 fid)
{
    if (fid == m_chaserID)
        updateStepList();
    else
    {
        // fid might be an ID of a ChaserStep of m_chaser
        Chaser* chaser = qobject_cast<Chaser*> (m_doc->function(m_chaserID));
        if (chaser == NULL)
            return;
        foreach (ChaserStep step, chaser->steps())
        {
            if (step.fid == fid)
            {
                updateStepList();
                return;
            }
        }
    }
}

void VCCueList::slotPlayback()
{
    if (mode() != Doc::Operate)
        return;

    if (m_runner != NULL)
    {
        slotStop();
    }
    else
    {
        m_mutex.lock();
        int index = getCurrentIndex();
        createRunner(index);
        m_mutex.unlock();
    }
}

void VCCueList::slotNextCue()
{
    if (mode() != Doc::Operate)
        return;

    /* Create the runner only when the first/last cue is engaged. */
    m_mutex.lock();
    if (m_runner == NULL)
        createRunner();
    else
        m_runner->next();
    m_mutex.unlock();
}

void VCCueList::slotPreviousCue()
{
    if (mode() != Doc::Operate)
        return;

    /* Create the runner only when the first/last cue is engaged. */
    m_mutex.lock();
    if (m_runner == NULL)
        createRunner(m_tree->topLevelItemCount() - 1); // Start from end
    else
        m_runner->previous();
    m_mutex.unlock();
}

void VCCueList::slotStop()
{
    if (mode() != Doc::Operate)
        return;

    m_mutex.lock();
    if (m_runner != NULL)
        m_stop = true;
    m_playbackButton->setIcon(QIcon(":/player_play.png"));
    m_sl1BottomLabel->setText("");
    m_sl1BottomLabel->setStyleSheet(m_noStyle);
    m_sl2BottomLabel->setText("");
    m_sl2BottomLabel->setStyleSheet(m_noStyle);
    // reset any previously set background
    QTreeWidgetItem *item = m_tree->topLevelItem(m_secondaryIndex);
    if (item != NULL)
        item->setBackground(COL_NUM, m_defCol);
    m_mutex.unlock();

    /* Start from the beginning */
    //m_tree->setCurrentItem(NULL);
}

void VCCueList::slotCurrentStepChanged(int stepNumber)
{
    Q_ASSERT(stepNumber < m_tree->topLevelItemCount() && stepNumber >= 0);
    QTreeWidgetItem* item = m_tree->topLevelItem(stepNumber);
    Q_ASSERT(item != NULL);
    m_tree->scrollToItem(item, QAbstractItemView::PositionAtCenter);
    m_tree->setCurrentItem(item);
    m_primaryIndex = stepNumber;
    setSlidersInfo(m_primaryIndex, NULL);
    emit stepChanged(m_primaryIndex);
}

void VCCueList::slotItemActivated(QTreeWidgetItem* item)
{
    if (mode() != Doc::Operate)
        return;

    m_mutex.lock();
    m_primaryIndex = m_tree->indexOfTopLevelItem(item);
    if (m_runner == NULL)
        createRunner(m_primaryIndex);
    else
        m_runner->setCurrentStep(m_primaryIndex, (qreal)m_slider1->value() / 100);

    setSlidersInfo(m_primaryIndex, NULL);
    m_mutex.unlock();
}

void VCCueList::slotItemChanged(QTreeWidgetItem *item, int column)
{
    if (m_listIsUpdating)
        return;

    Function *func = m_doc->function(m_chaserID);
    if (column != COL_NOTES || func == NULL)
        return;

    Chaser* chaser = qobject_cast<Chaser*> (func);
    QString itemText = item->text(column);
    int idx = m_tree->indexOfTopLevelItem(item);
    ChaserStep step = chaser->steps().at(idx);

    step.note = itemText;
    chaser->replaceStep(step, idx);
    updateStepList();
}

void VCCueList::slotFunctionStopped(quint32 fid)
{
    if (fid == m_chaserID && m_runner != NULL)
    {
        qDebug() << Q_FUNC_INFO << "Cue stopped";
        Chaser* chaser = qobject_cast<Chaser*> (m_doc->function(m_chaserID));
        if (chaser != NULL)
            chaser->useInternalRunner(true);
        slotStop();
    }
}

void VCCueList::createRunner(int startIndex)
{
    Q_ASSERT(m_runner == NULL);

    Chaser* chaser = qobject_cast<Chaser*> (m_doc->function(m_chaserID));
    if (chaser != NULL)
    {
        chaser->useInternalRunner(false);
        chaser->start(m_doc->masterTimer());
        m_runner = new CueListRunner(m_doc, chaser);
        Q_ASSERT(m_runner != NULL);
        //m_runner->moveToThread(QCoreApplication::instance()->thread());
        //m_runner->setParent(chaser);
        m_runner->setCurrentStep(startIndex, (qreal)m_slider1->value() / 100);
        m_primaryIndex = startIndex;

        connect(m_runner, SIGNAL(currentStepChanged(int)),
                this, SLOT(slotCurrentStepChanged(int)));
        m_playbackButton->setIcon(QIcon(":/player_stop.png"));
        setSlidersInfo(startIndex, chaser);
    }
}

/*****************************************************************************
 * Crossfade
 *****************************************************************************/
void VCCueList::setSlidersInfo(int pIndex, Chaser *chaser)
{
    Chaser *lChaser = chaser;
    if (lChaser == NULL)
        lChaser = qobject_cast<Chaser*> (m_doc->function(m_chaserID));

    if (lChaser == NULL)
        return;

    int tmpIndex = -1;
    if (lChaser->direction() == Function::Forward)
    {
        if (pIndex + 1 == m_tree->topLevelItemCount())
            tmpIndex = 0;
        else
            tmpIndex = pIndex + 1;
    }
    else
    {
        if (pIndex == 0)
            tmpIndex = m_tree->topLevelItemCount() - 1;
        else
            tmpIndex = pIndex - 1;
    }
    m_sl1BottomLabel->setText(QString("#%1").arg(m_primaryLeft ? pIndex + 1 : tmpIndex + 1));
    m_sl1BottomLabel->setStyleSheet(m_primaryLeft ? m_blueStyle : m_orangeStyle);

    m_sl2BottomLabel->setText(QString("#%1").arg(m_primaryLeft ? tmpIndex + 1 : pIndex + 1));
    m_sl2BottomLabel->setStyleSheet(m_primaryLeft ? m_orangeStyle : m_blueStyle);

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

    if (m_runner == NULL)
        return;

    m_runner->adjustIntensity((qreal)value / 100, m_primaryLeft ? m_primaryIndex: m_secondaryIndex);

    if(m_runner->runningStepsNumber() == 2)
    {
        if (m_primaryLeft == true && value == 0 && m_slider2->value() == 100)
        {
            m_runner->stopStep( m_primaryLeft ? m_primaryIndex: m_secondaryIndex);
            m_primaryLeft = false;
            switchFunction = true;
        }
        else if (m_primaryLeft == false && value == 100 && m_slider2->value() == 0)
        {
            m_runner->stopStep(m_primaryLeft ? m_secondaryIndex : m_primaryIndex);
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
        setSlidersInfo(m_primaryIndex, NULL);
    }
    updateFeedback();
}

void VCCueList::slotSlider2ValueChanged(int value)
{
    bool switchFunction = false;
    m_sl2TopLabel->setText(QString("%1%").arg(value));
    if (m_linkCheck->isChecked())
        m_slider1->setValue(100 - value);

    if (m_runner == NULL)
        return;

    m_runner->adjustIntensity((qreal)value / 100, m_primaryLeft ? m_secondaryIndex : m_primaryIndex);

    if (m_runner->runningStepsNumber() == 2)
    {
        if (m_primaryLeft == false && value == 0 && m_slider1->value() == 100)
        {
            m_runner->stopStep(m_primaryLeft ? m_secondaryIndex : m_primaryIndex);
            m_primaryLeft = true;
            switchFunction = true;
        }
        else if (m_primaryLeft == true && value == 100 && m_slider1->value() == 0)
        {
            m_runner->stopStep( m_primaryLeft ? m_primaryIndex: m_secondaryIndex);
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
        setSlidersInfo(m_primaryIndex, NULL);
    }
    updateFeedback();
}
/*****************************************************************************
 * DMX Source
 *****************************************************************************/

void VCCueList::writeDMX(MasterTimer* timer, UniverseArray* universes)
{
    m_mutex.lock();
    if (m_runner != NULL)
    {
        if (m_stop == false)
        {
            m_runner->write(timer, universes);
        }
        else
        {
            m_runner->postRun(timer, universes);
            delete m_runner;
            m_runner = NULL;
            m_stop = false;
            Chaser* chaser = qobject_cast<Chaser*> (m_doc->function(m_chaserID));
            if (chaser != NULL)
            {
                chaser->stop();
                chaser->useInternalRunner(true);
            }
        }
    }
    m_mutex.unlock();
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
    if (m_doc->mode() == Doc::Design)
        return;

    QLCInputSource src(universe, channel);

    if (src == inputSource(nextInputSourceId))
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
    else if (src == inputSource(previousInputSourceId))
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
    else if (src == inputSource(playbackInputSourceId))
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
    else if (src == inputSource(cf1InputSourceId))
    {
        float val = SCALE((float) value, (float) 0, (float) UCHAR_MAX,
                          (float) m_slider1->minimum(),
                          (float) m_slider1->maximum());
        m_slider1->setValue(val);
    }
    else if (src == inputSource(cf2InputSourceId))
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
        Q_ASSERT(m_runner == NULL);
        m_doc->masterTimer()->registerDMXSource(this);
        enable = true;
        // send the initial feedback for the current step slider
        updateFeedback();
    }
    else
    {
        m_doc->masterTimer()->unregisterDMXSource(this);
        m_mutex.lock();
        if (m_runner != NULL)
            delete m_runner;
        m_runner = NULL;
        m_mutex.unlock();
        m_sl1BottomLabel->setStyleSheet(m_noStyle);
        m_sl1BottomLabel->setText("");
        m_sl2BottomLabel->setStyleSheet(m_noStyle);
        m_sl2BottomLabel->setText("");
        // reset any previously set background
        QTreeWidgetItem *item = m_tree->topLevelItem(m_secondaryIndex);
        if (item != NULL)
            item->setBackground(COL_NUM, m_defCol);
    }

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

QString VCCueList::getCSS()
{
    QString str = "<style>\n"
            ".vccuelist {\n"
            "position: absolute;\n"
            "border: 1px solid #777777;\n"
            "border-radius: 3px;\n"
            "}\n"

            "table.hovertable {\n"
            " font-family: verdana,arial,sans-serif;\n"
            " font-size:11px;\n"
            " color:#333333;\n"
            " border-width: 1px;\n"
            " border-color: #999999;\n"
            " border-collapse: collapse;\n"
            "}\n"

            "table.hovertable th {\n"
            " background-color:#DCD9D6;\n"
            " border-width: 1px;\n"
            " padding: 3px;\n"
            " border-style: solid;\n"
            " border-color: #a9c6c9;\n"
            "}\n"

            "table.hovertable tr {\n"
            " background-color:#ffffff;\n"
            "}\n"

            "table.hovertable td {\n"
            " border-width: 1px;\n"
            " padding: 3px;\n"
            " border-style: solid;\n"
            " border-color: #a9c6c9;\n"
            "}\n"

            "</style>\n";

    return str;
}

QString VCCueList::getJS()
{
    QString str =
      "var cueListsIndices = new Array();\n\n"

      "function setCueIndex(id, idx) {\n"
      " var oldIdx = cueListsIndices[id];\n"
      " if (oldIdx != undefined) {\n"
      "   var oldCueObj = document.getElementById(id + \"_\" + oldIdx);\n"
      "   oldCueObj.style.backgroundColor='#FFFFFF';\n"
      " }\n"
      " cueListsIndices[id] = idx;\n"
      " var currCueObj = document.getElementById(id + \"_\" + idx);\n"
      " if (idx != \"-1\")\n"
      "   currCueObj.style.backgroundColor='#5E7FDF';\n"
      "}\n"

      "function sendCueCmd(id, cmd) {\n"
      " if (cmd == \"PLAY\") {\n"
      "   var obj = document.getElementById(\"play\" + id);\n"
      "   if (obj.innerHTML == \"Play\") {\n"
      "     obj.innerHTML = \"Stop\";\n"
      "     setCueIndex(id, 0);\n"
      "   }\n"
      "   else {\n"
      "     obj.innerHTML = \"Play\";\n"
      "     setCueIndex(id, -1);\n"
      "   }\n"
      " }\n"
      " websocket.send(id + \"|\" + cmd);\n"
      "}\n"

      "function checkMouseOut(id, idx) {\n"
      " var obj = document.getElementById(id + \"_\" + idx);\n"
      " if(idx == cueListsIndices[id])\n"
      "   obj.style.backgroundColor='#5E7FDF';\n"
      " else\n"
      "   obj.style.backgroundColor='#FFFFFF';\n"
      " }\n"

      "function enableCue(id, idx) {\n"
      " var btnObj = document.getElementById(\"play\" + id);\n"
      " btnObj.innerHTML = \"Stop\";\n"
      " setCueIndex(id, idx);\n"
      " websocket.send(id + \"|STEP|\" + idx);\n"
      "}\n";
    return str;
}

void VCCueList::playCueAtIndex(int idx)
{
    if (mode() != Doc::Operate)
        return;

    m_mutex.lock();
    m_primaryIndex = idx;
    if (m_runner == NULL)
        createRunner(m_primaryIndex);
    else
        m_runner->setCurrentStep(m_primaryIndex, (qreal)m_slider1->value() / 100);

    setSlidersInfo(m_primaryIndex, NULL);
    m_mutex.unlock();
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
                        setInputSource(QLCInputSource(uni, ch), nextInputSourceId);
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
                        setInputSource(QLCInputSource(uni, ch), previousInputSourceId);
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
                        setInputSource(QLCInputSource(uni, ch), playbackInputSourceId);
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
                        setInputSource(QLCInputSource(uni, ch), cf1InputSourceId);
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
                        setInputSource(QLCInputSource(uni, ch), cf2InputSourceId);
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
    if (inputSource(cf1InputSourceId).isValid())
    {
        tag = doc->createElement(KXMLQLCVCCueListCrossfadeLeft);
        root.appendChild(tag);
        saveXMLInput(doc, &tag, inputSource(cf1InputSourceId));
    }
    if (inputSource(cf2InputSourceId).isValid())
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
