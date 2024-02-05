/*
  Q Light Controller
  chasereditor.cpp

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

#include <QTreeWidgetItem>
#include <QRadioButton>
#include <QHeaderView>
#include <QTreeWidget>
#include <QPushButton>
#include <QMessageBox>
#include <QLineEdit>
#include <QSettings>
#include <QDebug>
#include <QUrl>
#include <QAction>

#include "functionselection.h"
#include "speeddialwidget.h"
#include "chasereditor.h"
#include "mastertimer.h"
#include "chaserstep.h"
#include "sequence.h"
#include "apputil.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"

#define SETTINGS_GEOMETRY "chasereditor/geometry"
#define PROP_STEP Qt::UserRole

#define COL_NUM      0
#define COL_NAME     1
#define COL_FADEIN   2
#define COL_HOLD     3
#define COL_FADEOUT  4
#define COL_DURATION 5
#define COL_NOTES    6

ChaserEditor::ChaserEditor(QWidget* parent, Chaser* chaser, Doc* doc, bool liveMode)
    : QWidget(parent)
    , m_doc(doc)
    , m_chaser(chaser)
    , m_speedDials(NULL)
    , m_liveMode(liveMode)
{
    Q_ASSERT(chaser != NULL);
    Q_ASSERT(doc != NULL);

    setupUi(this);

    /* Disable editing of steps number */
    m_tree->setItemDelegateForColumn(COL_NUM, new NoEditDelegate(this));
    if (m_chaser->type() == Function::SequenceType)
        m_tree->header()->setSectionHidden(COL_NAME, true);

    m_cutAction = new QAction(QIcon(":/editcut.png"), tr("Cut"), this);
    m_cutButton->setDefaultAction(m_cutAction);
    m_cutAction->setShortcut(QKeySequence(QKeySequence::Cut));
    connect(m_cutAction, SIGNAL(triggered(bool)), this, SLOT(slotCutClicked()));

    m_copyAction = new QAction(QIcon(":/editcopy.png"), tr("Copy"), this);
    m_copyButton->setDefaultAction(m_copyAction);
    m_copyAction->setShortcut(QKeySequence(QKeySequence::Copy));
    connect(m_copyAction, SIGNAL(triggered(bool)), this, SLOT(slotCopyClicked()));

    m_pasteAction = new QAction(QIcon(":/editpaste.png"), tr("Paste"), this);
    m_pasteButton->setDefaultAction(m_pasteAction);
    m_pasteAction->setShortcut(QKeySequence(QKeySequence::Paste));
    connect(m_pasteAction, SIGNAL(triggered(bool)), this, SLOT(slotPasteClicked()));

    /* Name edit */
    m_nameEdit->setText(m_chaser->name());
    m_nameEdit->setSelection(0, m_nameEdit->text().length());

    /* Fade In Mode */
    switch (m_chaser->fadeInMode())
    {
    default:
    case Chaser::Default:
        m_fadeInDefaultRadio->setChecked(true);
        break;
    case Chaser::Common:
        m_fadeInCommonRadio->setChecked(true);
        break;
    case Chaser::PerStep:
        m_fadeInPerStepRadio->setChecked(true);
        break;
    }

    /* Fade Out Mode */
    switch (m_chaser->fadeOutMode())
    {
    default:
    case Chaser::Default:
        m_fadeOutDefaultRadio->setChecked(true);
        break;
    case Chaser::Common:
        m_fadeOutCommonRadio->setChecked(true);
        break;
    case Chaser::PerStep:
        m_fadeOutPerStepRadio->setChecked(true);
        break;
    }

    /* Duration Mode */
    switch (m_chaser->durationMode())
    {
    default:
    case Chaser::Default:
    case Chaser::Common:
        m_durationCommonRadio->setChecked(true);
        break;
    case Chaser::PerStep:
        m_durationPerStepRadio->setChecked(true);
        break;
    }

    /* Running order */
    switch (m_chaser->runOrder())
    {
    default:
    case Chaser::Loop:
        m_loop->setChecked(true);
        break;
    case Chaser::PingPong:
        m_pingPong->setChecked(true);
        break;
    case Chaser::SingleShot:
        m_singleShot->setChecked(true);
        break;
    case Chaser::Random:
        m_random->setChecked(true);
        break;
    }

    /* Running direction */
    switch (m_chaser->direction())
    {
    default:
    case Chaser::Forward:
        m_forward->setChecked(true);
        break;
    case Chaser::Backward:
        m_backward->setChecked(true);
        break;
    }

    connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotNameEdited(const QString&)));
    connect(m_add, SIGNAL(clicked()),
            this, SLOT(slotAddClicked()));
    connect(m_remove, SIGNAL(clicked()),
            this, SLOT(slotRemoveClicked()));
    connect(m_raise, SIGNAL(clicked()),
            this, SLOT(slotRaiseClicked()));
    connect(m_lower, SIGNAL(clicked()),
            this, SLOT(slotLowerClicked()));
    connect(m_shuffle, SIGNAL(clicked()),
            this, SLOT(slotShuffleClicked()));
    connect(m_speeddial, SIGNAL(toggled(bool)),
            this, SLOT(slotSpeedDialToggle(bool)));
    connect(m_fadeInCommonRadio, SIGNAL(toggled(bool)),
            this, SLOT(slotFadeInToggled()));
    connect(m_fadeInPerStepRadio, SIGNAL(toggled(bool)),
            this, SLOT(slotFadeInToggled()));
    connect(m_fadeInDefaultRadio, SIGNAL(toggled(bool)),
            this, SLOT(slotFadeInToggled()));

    connect(m_fadeOutCommonRadio, SIGNAL(toggled(bool)),
            this, SLOT(slotFadeOutToggled()));
    connect(m_fadeOutPerStepRadio, SIGNAL(toggled(bool)),
            this, SLOT(slotFadeOutToggled()));
    connect(m_fadeOutDefaultRadio, SIGNAL(toggled(bool)),
            this, SLOT(slotFadeOutToggled()));

    connect(m_durationCommonRadio, SIGNAL(toggled(bool)),
            this, SLOT(slotDurationToggled()));
    connect(m_durationPerStepRadio, SIGNAL(toggled(bool)),
            this, SLOT(slotDurationToggled()));

    connect(m_loop, SIGNAL(clicked()),
            this, SLOT(slotLoopClicked()));
    connect(m_singleShot, SIGNAL(clicked()),
            this, SLOT(slotSingleShotClicked()));
    connect(m_pingPong, SIGNAL(clicked()),
            this, SLOT(slotPingPongClicked()));
    connect(m_random, SIGNAL(clicked()),
            this, SLOT(slotRandomClicked()));
    connect(m_random, SIGNAL(clicked()),
            this, SLOT(slotRestartTest()));

    connect(m_forward, SIGNAL(clicked()),
            this, SLOT(slotForwardClicked()));
    connect(m_backward, SIGNAL(clicked()),
            this, SLOT(slotBackwardClicked()));
    connect(m_forward, SIGNAL(clicked()),
            this, SLOT(slotRestartTest()));
    connect(m_backward, SIGNAL(clicked()),
            this, SLOT(slotRestartTest()));

    connect(m_tree, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotItemSelectionChanged()));
    connect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(slotItemChanged(QTreeWidgetItem*,int)));

    //connect(m_testButton, SIGNAL(toggled(bool)), this, SLOT(slotTestToggled(bool)));
    connect(m_testPlayButton, SIGNAL(clicked()), this, SLOT(slotTestPlay()));
    connect(m_testStopButton, SIGNAL(clicked()), this, SLOT(slotTestStop()));
    connect(m_testPreviousButton, SIGNAL(clicked()), this, SLOT(slotTestPreviousClicked()));
    connect(m_testNextButton, SIGNAL(clicked()), this, SLOT(slotTestNextClicked()));
    connect(m_doc, SIGNAL(modeChanged(Doc::Mode)), this, SLOT(slotModeChanged(Doc::Mode)));
    connect(m_chaser, SIGNAL(currentStepChanged(int)), this, SLOT(slotStepChanged(int)));

    updateTree(true);
    updateClipboardButtons();
    updateSpeedDials();

    slotModeChanged(m_doc->mode());

    // Set focus to the editor
    m_nameEdit->setFocus();

    m_testPreviousButton->setEnabled(false);
    m_testNextButton->setEnabled(false);
}

ChaserEditor::~ChaserEditor()
{
    if (m_speedDials != NULL)
        m_speedDials->deleteLater();
    m_speedDials = NULL;

    // double check that the Chaser still exists !
    if (m_liveMode == false &&
        m_doc->functions().contains(m_chaser) == true)
        m_chaser->stopAndWait();
}

void ChaserEditor::showOrderAndDirection(bool show)
{
    groupBox->setVisible(show);
    groupBox_2->setVisible(show);
}

void ChaserEditor::stopTest()
{
    m_chaser->stopAndWait();
}

void ChaserEditor::selectStepAtTime(quint32 time)
{
    quint32 stepTime = 0;
    for (int i = 0; i < m_chaser->stepsCount(); i++)
    {
        quint32 timeIncr = 0;
        if (m_chaser->durationMode() == Chaser::Common)
            timeIncr = m_chaser->duration();
        else // Chaser::PerStep
        {
            timeIncr += m_chaser->stepAt(i)->duration;
        }
        if (time < stepTime + timeIncr)
        {
            QTreeWidgetItem *item = m_tree->topLevelItem(i);
            m_tree->setCurrentItem(item);
            m_tree->scrollToItem(item, QAbstractItemView::PositionAtCenter);
            return;
        }
        stepTime += timeIncr;
    }
}

void ChaserEditor::slotFunctionManagerActive(bool active)
{
    if (active == true)
    {
        updateSpeedDials();
    }
    else
    {
        if (m_speedDials != NULL)
            m_speedDials->deleteLater();
        m_speedDials = NULL;
    }
}

void ChaserEditor::slotNameEdited(const QString& text)
{
    m_chaser->setName(text);
}

void ChaserEditor::slotUpdateCurrentStep(SceneValue sv, bool enabled)
{
    qDebug() << "Value changed: " << sv.fxi << sv.channel << sv.value << enabled;
    QList <QTreeWidgetItem*> selected(m_tree->selectedItems());

    if (selected.size() == 0)
        return;

    QTreeWidgetItem* item(selected.first());
    int idx = m_tree->indexOfTopLevelItem(item);

    if (enabled == true)
    {
        bool created = false;
        int svIndex = m_chaser->stepAt(idx)->setValue(sv, -1, &created);

        if (created == true)
        {
            // this means the provided Scene value is new - for EVERY step.
            // All the non-selected steps should include the new value, but set to 0
            sv.value = 0;

            for (int i = 0; i < m_chaser->stepsCount(); i++)
            {
                // skip the original selected step, otherwise
                // the original Scene value would be overwritten
                if (i == idx)
                    continue;

                m_chaser->stepAt(i)->setValue(sv, svIndex);
                qDebug() << "[slotUpdateCurrentStep] Value added to step: " << i << "@pos" << svIndex;
            }
        }
    }
    else
    {
        int svIndex = m_chaser->stepAt(idx)->unSetValue(sv);

        if (svIndex == -1)
            return;

        for (int i = 0; i < m_chaser->stepsCount(); i++)
        {
            // do not unset again on the currently edited step
            if (i == idx)
                continue;

            m_chaser->stepAt(i)->unSetValue(sv, svIndex);
            qDebug() << "[slotUpdateCurrentStep] Value removed from step: " << i << "@pos" << svIndex;
        }
    }
}

/****************************************************************************
 * List manipulation
 ****************************************************************************/

void ChaserEditor::slotAddClicked()
{
    bool stepAdded = false;

    int insertionPoint = m_tree->topLevelItemCount();
    QTreeWidgetItem* item = m_tree->currentItem();
    if (item != NULL)
        insertionPoint = m_tree->indexOfTopLevelItem(item) + 1;

    if (m_chaser->type() == Function::SequenceType)
    {
        Sequence *sequence = qobject_cast<Sequence*>(m_chaser);
        ChaserStep step(sequence->boundSceneID());
        item = new QTreeWidgetItem;
        updateItem(item, step);
        // if this is the first step we add, then copy all DMX channels non-zero values
        Scene *currScene = qobject_cast<Scene*> (m_doc->function(sequence->boundSceneID()));
        QListIterator <SceneValue> it(currScene->values());
        qDebug() << "First step added !!";
        while (it.hasNext() == true)
        {
            SceneValue chan(it.next());
            step.values.append(chan);
            //qDebug() << "Value added: " << chan.value;
        }
        qDebug() << "Values added: " << step.values.count();

        m_tree->insertTopLevelItem(insertionPoint, item);
        m_chaser->addStep(step, insertionPoint);
        stepAdded = true;
    }
    else
    {
        FunctionSelection fs(this, m_doc);
        {
            QList<quint32> disabledList;
            disabledList << m_chaser->id();
            foreach (Function* function, m_doc->functions())
            {
                if (function->contains(m_chaser->id()))
                    disabledList << function->id();
            }
            fs.setDisabledFunctions(disabledList);
        }

        if (fs.exec() == QDialog::Accepted)
        {
            /* Append selected functions */
            QListIterator <quint32> it(fs.selection());
            while (it.hasNext() == true)
            {
                ChaserStep step(it.next());
                item = new QTreeWidgetItem;
                updateItem(item, step);
                m_tree->insertTopLevelItem(insertionPoint, item);
                qDebug() << "Insertion point: " << insertionPoint;
                m_chaser->addStep(step, insertionPoint++);
            }
            stepAdded = true;
        }
    }
    if (stepAdded == true)
    {
        // at last, select the newly created step, so in case of a Sequence,
        // the Scene Editor will show the current values, and users will
        // stop bugging us in the forums
        m_tree->setCurrentItem(item);
        updateStepNumbers();
        updateClipboardButtons();
        //printSteps();
    }
}

void ChaserEditor::slotRemoveClicked()
{
    return slotCutClicked();
}

void ChaserEditor::slotRaiseClicked()
{
    QList <QTreeWidgetItem*> items(m_tree->selectedItems());
    QListIterator <QTreeWidgetItem*> it(items);

    // Check, whether even one of the items would "bleed" over the edge and
    // cancel the operation if that is the case.
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item(it.next());
        int index = m_tree->indexOfTopLevelItem(item);
        if (index == 0)
            return;
    }

    // Move the items
    it.toFront();
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item(it.next());
        int index = m_tree->indexOfTopLevelItem(item);
        m_tree->takeTopLevelItem(index);
        m_tree->insertTopLevelItem(index - 1, item);
        m_chaser->moveStep(index, index - 1);
    }

    updateStepNumbers();

    // Select the moved items
    it.toFront();
    while (it.hasNext() == true)
        it.next()->setSelected(true);

    updateClipboardButtons();
    //printSteps();
}

void ChaserEditor::slotLowerClicked()
{
    QList <QTreeWidgetItem*> items(m_tree->selectedItems());
    QListIterator <QTreeWidgetItem*> it(items);

    // Check, whether even one of the items would "bleed" over the edge and
    // cancel the operation if that is the case.
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item(it.next());
        int index = m_tree->indexOfTopLevelItem(item);
        if (index == m_tree->topLevelItemCount() - 1)
            return;
    }

    // Move the items
    it.toBack();
    while (it.hasPrevious() == true)
    {
        QTreeWidgetItem* item(it.previous());
        int index = m_tree->indexOfTopLevelItem(item);
        m_tree->takeTopLevelItem(index);
        m_tree->insertTopLevelItem(index + 1, item);
        m_chaser->moveStep(index, index + 1);
    }

    updateStepNumbers();

    // Select the items
    it.toFront();
    while (it.hasNext() == true)
        it.next()->setSelected(true);

    updateClipboardButtons();
    //printSteps();
}

void ChaserEditor::slotShuffleClicked()
{
    int i;
    int selectedCount = m_tree->selectedItems().count();

    if (selectedCount == 1)
    {
        // it doesn't make sense shuffling one step
        return;
    }
    else if (selectedCount == 0)
    {
        m_tree->selectAll();
        selectedCount = m_tree->selectedItems().count();
    }

    QList <QTreeWidgetItem*> selectedItems(m_tree->selectedItems());
    int indicesToShuffle[selectedCount];

    // save the selected scenes and their indices into a sorted array
    QListIterator <QTreeWidgetItem*> it(selectedItems);
    for (i = 0; i < selectedCount; i++)
    {
        QTreeWidgetItem* item = it.next();
        indicesToShuffle[i] = m_tree->indexOfTopLevelItem(item);
    }
    std::sort(indicesToShuffle, indicesToShuffle + selectedCount);

    // shuffle the selected scenes using the Fisher-Yates algorithm
    // see https://bost.ocks.org/mike/shuffle/ for information on the algorithm
    int unshuffledCount = selectedCount;
    while (unshuffledCount > 0)
    {
        // pick a random unshuffled selected and swap it with the last unshuffled one -> now it is a shuffled step
        int toShuffle = rand() % unshuffledCount;
        unshuffledCount--;
        int indexToShuffle = indicesToShuffle[toShuffle];
        int lastUnshuffledIndex = indicesToShuffle[unshuffledCount];

        if (indexToShuffle != lastUnshuffledIndex)
        {
            QTreeWidgetItem* lastUnshuffledItem = m_tree->takeTopLevelItem(lastUnshuffledIndex);
            QTreeWidgetItem* itemToShuffle = m_tree->takeTopLevelItem(indexToShuffle);
            m_tree->insertTopLevelItem(indexToShuffle, lastUnshuffledItem);
            m_tree->insertTopLevelItem(lastUnshuffledIndex, itemToShuffle);
            m_chaser->moveStep(indexToShuffle, lastUnshuffledIndex);
            m_chaser->moveStep(lastUnshuffledIndex - 1, indexToShuffle);
        }
    }

    updateStepNumbers();
    updateClipboardButtons();

    // the selection is destroyed / weird after reordering scenes, so we restore it manually
    m_tree->clearSelection();
    for (i = 0; i < selectedCount; i++)
        m_tree->topLevelItem(indicesToShuffle[i])->setSelected(true);
}

void ChaserEditor::slotSpeedDialToggle(bool state)
{
    if (state == true)
        updateSpeedDials();
    else
    {
        if (m_speedDials != NULL)
            m_speedDials->deleteLater();
        m_speedDials = NULL;
    }
}

void ChaserEditor::slotItemSelectionChanged()
{
    if (m_chaser->isRunning())
        return;

    if (m_tree->selectedItems().count() > 0)
    {
        QTreeWidgetItem *item = m_tree->selectedItems().first();
        int idx = item->text(COL_NUM).toUInt() - 1;
        emit stepSelectionChanged(idx);
    }
    else
        emit stepSelectionChanged(-1);

    updateClipboardButtons();
    updateSpeedDials();
    applyStepValues();
}

void ChaserEditor::slotItemChanged(QTreeWidgetItem *item, int column)
{
    QString itemText = item->text(column);
    quint32 newValue = Function::stringToSpeed(itemText);
    int idx = m_tree->indexOfTopLevelItem(item);

    ChaserStep step = m_chaser->steps().at(idx);

    uint fadeIn = m_chaser->fadeInMode() == Chaser::Common ? m_chaser->fadeInSpeed() : step.fadeIn;
    uint fadeOut = m_chaser->fadeOutMode() == Chaser::Common ? m_chaser->fadeOutSpeed() : step.fadeOut;
    uint duration = m_chaser->durationMode() == Chaser::Common ? m_chaser->duration() : step.duration;
    uint hold = Function::speedSubtract(duration, fadeIn);
    bool updateTreeNeeded = false;

    if (column == COL_FADEIN)
    {
        fadeIn = newValue;
        if (m_chaser->fadeInMode() == Chaser::Common)
        {
            m_chaser->setFadeInSpeed(fadeIn);
            updateTreeNeeded = true;
            if (m_chaser->durationMode() == Chaser::Common)
                m_chaser->setDuration(Function::speedAdd(hold, fadeIn));
        }
        else
        {
            step.fadeIn = fadeIn;
            if (m_chaser->durationMode() != Chaser::Common)
                step.duration = Function::speedAdd(hold, fadeIn);
        }

    }
    else if (column == COL_HOLD)
    {
        hold = newValue;

        if (m_chaser->durationMode() == Chaser::Common)
        {
            m_chaser->setDuration(Function::speedAdd(hold, fadeIn));
            updateTreeNeeded = true;
        }
        else
        {
            step.hold = hold;
            step.duration = Function::speedAdd(hold, fadeIn);
        }
    }
    else if (column == COL_FADEOUT)
    {
        fadeOut = newValue;
        if (m_chaser->fadeOutMode() == Chaser::Common)
        {
            m_chaser->setFadeOutSpeed(fadeOut);
            updateTreeNeeded = true;
        }
        else
        {
            step.fadeOut = fadeOut;
        }
    }
    else if (column == COL_DURATION)
    {
        duration = newValue;

        if (m_chaser->durationMode() == Chaser::Common)
        {
            m_chaser->setDuration(duration);
            updateTreeNeeded = true;
        }
        else
        {
            step.duration = duration;
            step.hold = Function::speedSubtract(duration, fadeIn);
        }
    }
    else if (column == COL_NOTES)
    {
        step.note = itemText;
    }
    if (updateTreeNeeded)
        updateTree();

    m_chaser->replaceStep(step, idx);
    updateItem(item, step);

    m_tree->header()->resizeSections(QHeaderView::ResizeToContents);
}

/****************************************************************************
 * Clipboard
 ****************************************************************************/

void ChaserEditor::slotCutClicked()
{
    QList <ChaserStep> copyList;
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item(it.next());
        int index = m_tree->indexOfTopLevelItem(item);
        copyList << stepAtIndex(index);
        m_chaser->removeStep(index);
        delete item;
    }

    m_doc->clipboard()->copyContent(m_chaser->id(), copyList);
    m_tree->setCurrentItem(NULL);

    updateStepNumbers();
    updateClipboardButtons();
}

void ChaserEditor::slotCopyClicked()
{
    QList <ChaserStep> copyList;
    foreach (QTreeWidgetItem *item, m_tree->selectedItems())
        copyList << stepAtItem(item);
    QLCClipboard *clipboard = m_doc->clipboard();
    clipboard->copyContent(m_chaser->id(), copyList);
    updateClipboardButtons();
}

void ChaserEditor::slotPasteClicked()
{
    if (m_doc->clipboard()->hasChaserSteps() == false)
        return;
    QList <ChaserStep> pasteList = m_doc->clipboard()->getChaserSteps();

    // If the Chaser is a sequence, then perform a sanity
    // check on each Step to see if they really belong to
    // this scene
    if (m_chaser->type() == Function::SequenceType)
    {
        Sequence *sequence = qobject_cast<Sequence*>(m_chaser);
        quint32 sceneID = sequence->boundSceneID();
        Scene *scene = qobject_cast<Scene*>(m_doc->function(sceneID));
        foreach (ChaserStep step, pasteList)
        {
            if (step.fid != sceneID) // if IDs are the same then it's a valid step
            {
                foreach (SceneValue scv, step.values)
                {
                    if (scene->checkValue(scv) == false)
                    {
                        QMessageBox::warning(this, tr("Paste error"), tr("Trying to paste on an incompatible Scene. Operation canceled."));
                        return;
                    }
                }
            }
        }
    }

    int insertionPoint = 0;
    QTreeWidgetItem* currentItem = m_tree->currentItem();
    if (currentItem != NULL)
    {
        insertionPoint = m_tree->indexOfTopLevelItem(currentItem) + 1;
        currentItem->setSelected(false);
    }
    else
    {
        insertionPoint = m_tree->topLevelItemCount();
    }

    QList<QTreeWidgetItem*>selectionList;

    QListIterator <ChaserStep> it(pasteList);
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem;
        ChaserStep step(it.next());
        if (step.resolveFunction(m_doc) == NULL) // Function has been removed
        {
            qWarning() << Q_FUNC_INFO << "Trying to paste an invalid function (removed function?)";
            continue;
        }
        updateItem(item, step);
        m_tree->insertTopLevelItem(insertionPoint, item);
        m_chaser->addStep(step, insertionPoint);
        selectionList.append(item); // defer items selection cause of performances issues
        insertionPoint++;
    }

    updateStepNumbers();
    updateClipboardButtons();

    // this is done here cause of a misterious performance issue
    foreach (QTreeWidgetItem *item, selectionList)
        item->setSelected(true);
}

/****************************************************************************
 * Run order & Direction
 ****************************************************************************/

void ChaserEditor::slotLoopClicked()
{
    m_chaser->setRunOrder(Function::Loop);
}

void ChaserEditor::slotSingleShotClicked()
{
    m_chaser->setRunOrder(Function::SingleShot);
}

void ChaserEditor::slotPingPongClicked()
{
    m_chaser->setRunOrder(Function::PingPong);
}

void ChaserEditor::slotRandomClicked()
{
    m_chaser->setRunOrder(Function::Random);
}

void ChaserEditor::slotForwardClicked()
{
    m_chaser->setDirection(Function::Forward);
}

void ChaserEditor::slotBackwardClicked()
{
    m_chaser->setDirection(Function::Backward);
}

/****************************************************************************
 * Speed
 ****************************************************************************/

void ChaserEditor::slotFadeInToggled()
{
    if (m_fadeInCommonRadio->isChecked() == true)
        m_chaser->setFadeInMode(Chaser::Common);
    else if (m_fadeInPerStepRadio->isChecked() == true)
        m_chaser->setFadeInMode(Chaser::PerStep);
    else
        m_chaser->setFadeInMode(Chaser::Default);

    updateTree();
    updateSpeedDials();
}

void ChaserEditor::slotFadeOutToggled()
{
    if (m_fadeOutCommonRadio->isChecked() == true)
        m_chaser->setFadeOutMode(Chaser::Common);
    else if (m_fadeOutPerStepRadio->isChecked() == true)
        m_chaser->setFadeOutMode(Chaser::PerStep);
    else
        m_chaser->setFadeOutMode(Chaser::Default);

    updateTree();
    updateSpeedDials();
}

void ChaserEditor::slotDurationToggled()
{
    if (m_durationPerStepRadio->isChecked() == true)
        m_chaser->setDurationMode(Chaser::PerStep);
    else
        m_chaser->setDurationMode(Chaser::Common);

    updateTree();
    updateSpeedDials();
}

void ChaserEditor::slotFadeInDialChanged(int ms)
{
    switch (m_chaser->fadeInMode())
    {
    case Chaser::Common:
        if (QTreeWidgetItem* item = m_tree->topLevelItem(0))
            item->setText(COL_FADEIN, Function::speedToString(ms));
        else
            m_chaser->setFadeInSpeed(Function::speedNormalize(ms));
        break;
    case Chaser::PerStep:
        foreach (QTreeWidgetItem* item, m_tree->selectedItems())
        {
            item->setText(COL_FADEIN, Function::speedToString(ms));
        }
        break;
    default:
    case Chaser::Default:
        break;
    }

    m_tree->resizeColumnToContents(COL_FADEIN);
}

void ChaserEditor::slotFadeOutDialChanged(int ms)
{
    switch (m_chaser->fadeOutMode())
    {
    case Chaser::Common:
        if (QTreeWidgetItem* item = m_tree->topLevelItem(0))
            item->setText(COL_FADEOUT, Function::speedToString(ms));
        else
            m_chaser->setFadeOutSpeed(Function::speedNormalize(ms));
        break;
    case Chaser::PerStep:
        foreach (QTreeWidgetItem* item, m_tree->selectedItems())
        {
            item->setText(COL_FADEOUT, Function::speedToString(ms));
        }
        break;
    default:
    case Chaser::Default:
        break;
    }

    m_tree->resizeColumnToContents(COL_FADEOUT);
}

void ChaserEditor::slotHoldDialChanged(int ms)
{
    switch (m_chaser->durationMode())
    {
    case Chaser::Common:
        if (QTreeWidgetItem* item = m_tree->topLevelItem(0))
            item->setText(COL_HOLD, Function::speedToString(ms));
        else
        {
            if (m_chaser->fadeInMode() == Chaser::Common)
                m_chaser->setDuration(Function::speedAdd(ms, m_chaser->fadeInSpeed()));
            else
                m_chaser->setDuration(Function::speedNormalize(ms));
        }
        break;
    case Chaser::PerStep:
        foreach (QTreeWidgetItem* item, m_tree->selectedItems())
        {
            item->setText(COL_HOLD, Function::speedToString(ms));
        }
        break;
    default:
    case Chaser::Default:
        break;
    }

    m_tree->resizeColumnToContents(COL_HOLD);
}

void ChaserEditor::slotDialDestroyed(QObject *)
{
    m_speeddial->setChecked(false);
}

void ChaserEditor::createSpeedDials()
{
    if (m_speedDials == NULL)
    {
        m_speedDials = new SpeedDialWidget(this);
        m_speedDials->setAttribute(Qt::WA_DeleteOnClose);

        connect(m_speedDials, SIGNAL(fadeInChanged(int)),
                this, SLOT(slotFadeInDialChanged(int)));
        connect(m_speedDials, SIGNAL(fadeOutChanged(int)),
                this, SLOT(slotFadeOutDialChanged(int)));
        connect(m_speedDials, SIGNAL(holdChanged(int)),
                this, SLOT(slotHoldDialChanged(int)));
        connect(m_speedDials, SIGNAL(destroyed(QObject*)),
                this, SLOT(slotDialDestroyed(QObject*)));
    }

    m_speedDials->show();
}

void ChaserEditor::updateSpeedDials()
{
    if (m_speeddial->isChecked() == false)
        return;

    static const QString fadeIn(tr("Fade In"));
    static const QString fadeOut(tr("Fade Out"));
    static const QString hold(tr("Hold"));
    static const QString globalFadeIn(tr("Common Fade In"));
    static const QString globalFadeOut(tr("Common Fade Out"));
    static const QString globalHold(tr("Common Hold"));

    createSpeedDials();

    QList <QTreeWidgetItem*> selected(m_tree->selectedItems());

    ChaserStep step;
    if (selected.size() != 0)
    {
        const QTreeWidgetItem* item(selected.first());
        step = stepAtItem(item);

        QString title;
        if (selected.size() == 1)
            title = QString("%1: %2").arg(item->text(COL_NUM)).arg(item->text(COL_NAME));
        else
            title = tr("Multiple Steps");
        m_speedDials->setWindowTitle(title);
    }
    else
    {
        m_speedDials->setWindowTitle(m_chaser->name());
    }

    /* Fade in */
    switch (m_chaser->fadeInMode())
    {
    case Chaser::Common:
        m_speedDials->setFadeInSpeed(m_chaser->fadeInSpeed());
        m_speedDials->setFadeInTitle(globalFadeIn);
        m_speedDials->setFadeInEnabled(true);
        break;
    case Chaser::PerStep:
        if (selected.size() != 0)
        {
            m_speedDials->setFadeInSpeed(step.fadeIn);
            m_speedDials->setFadeInEnabled(true);
        }
        else
        {
            m_speedDials->setFadeInSpeed(0);
            m_speedDials->setFadeInEnabled(false);
        }
        m_speedDials->setFadeInTitle(fadeIn);
        break;
    default:
    case Chaser::Default:
        m_speedDials->setFadeInTitle(fadeIn);
        m_speedDials->setFadeInEnabled(false);
        break;
    }

    /* Fade out */
    switch (m_chaser->fadeOutMode())
    {
    case Chaser::Common:
        m_speedDials->setFadeOutSpeed(m_chaser->fadeOutSpeed());
        m_speedDials->setFadeOutTitle(globalFadeOut);
        m_speedDials->setFadeOutEnabled(true);
        break;
    case Chaser::PerStep:
        if (selected.size() != 0)
        {
            m_speedDials->setFadeOutSpeed(step.fadeOut);
            m_speedDials->setFadeOutEnabled(true);
        }
        else
        {
            m_speedDials->setFadeOutSpeed(0);
            m_speedDials->setFadeOutEnabled(false);
        }
        m_speedDials->setFadeOutTitle(fadeOut);
        break;
    default:
    case Chaser::Default:
        m_speedDials->setFadeOutTitle(fadeOut);
        m_speedDials->setFadeOutEnabled(false);
        break;
    }

    /* Hold */
    switch (m_chaser->durationMode())
    {
    default:
    case Chaser::Common:
    {
        if ((int)m_chaser->duration() < 0)
            m_speedDials->setDuration(m_chaser->duration());
        else
            m_speedDials->setDuration(m_chaser->duration() - m_chaser->fadeInSpeed());
        m_speedDials->setDurationTitle(globalHold);
        m_speedDials->setDurationEnabled(true);
        break;
    }
    case Chaser::PerStep:
        if (selected.size() != 0)
        {
            m_speedDials->setDuration(step.hold);
            m_speedDials->setDurationEnabled(true);
        }
        else
        {
            m_speedDials->setFadeOutSpeed(0);
            m_speedDials->setDurationEnabled(false);
        }
        m_speedDials->setDurationTitle(hold);
        break;
    }
}

/****************************************************************************
 * Test
 ****************************************************************************/
int ChaserEditor::getCurrentIndex()
{
    // Return the index of the current selected item
    // Return -1 if nothing is selected
    return m_tree->indexOfTopLevelItem(m_tree->currentItem());
}

void ChaserEditor::slotRestartTest()
{
    if (m_chaser->stopped() == false)
    {
        // Toggle off, toggle on. Derp.
        m_testStopButton->click();
        m_testPlayButton->click();
    }
}

void ChaserEditor::slotTestPlay()
{
    m_testPreviousButton->setEnabled(true);
    m_testNextButton->setEnabled(true);

    int idx = getCurrentIndex();
    if (idx >= 0)
    {
        ChaserAction action;
        action.m_action = ChaserSetStepIndex;
        action.m_stepIndex = idx;
        action.m_masterIntensity = 1.0;
        action.m_stepIntensity = 1.0;
        action.m_fadeMode = Chaser::FromFunction;
        m_chaser->setAction(action);
    }
    m_chaser->start(m_doc->masterTimer(), functionParent());
}

FunctionParent ChaserEditor::functionParent() const
{
    return FunctionParent::master();
}

void ChaserEditor::slotTestStop()
{
    m_testPreviousButton->setEnabled(false);
    m_testNextButton->setEnabled(false);

    m_chaser->stopAndWait();
}

void ChaserEditor::slotTestPreviousClicked()
{
    ChaserAction action;
    action.m_action = ChaserPreviousStep;
    action.m_masterIntensity = 1.0;
    action.m_stepIntensity = 1.0;
    action.m_fadeMode = Chaser::FromFunction;
    m_chaser->setAction(action);
}

void ChaserEditor::slotTestNextClicked()
{
    ChaserAction action;
    action.m_action = ChaserNextStep;
    action.m_masterIntensity = 1.0;
    action.m_stepIntensity = 1.0;
    action.m_fadeMode = Chaser::FromFunction;
    m_chaser->setAction(action);
}

void ChaserEditor::slotModeChanged(Doc::Mode mode)
{
    if (mode == Doc::Operate)
    {
        m_testPlayButton->setEnabled(false);
        m_testStopButton->setEnabled(false);
        if (m_liveMode == false)
            m_chaser->stop(functionParent());
    }
    else
    {
        m_testPlayButton->setEnabled(true);
        m_testStopButton->setEnabled(true);
    }
}

void ChaserEditor::slotStepChanged(int stepNumber)
{
    // Select only the item at step StepNumber
    // If stepNumber is outside of bounds, select nothing
    m_tree->setCurrentItem(m_tree->topLevelItem(stepNumber));
}

/****************************************************************************
 * Utilities
 ****************************************************************************/

void ChaserEditor::updateTree(bool clear)
{
    if (clear == true)
        m_tree->clear();

    for (int i = 0; i < m_chaser->steps().size(); i++)
    {
        QTreeWidgetItem* item = NULL;

        if (clear == true)
            item = new QTreeWidgetItem(m_tree);
        else
            item = m_tree->topLevelItem(i);
        Q_ASSERT(item != NULL);

        ChaserStep step(m_chaser->steps().at(i));
        updateItem(item, step);
    }

    m_tree->header()->resizeSections(QHeaderView::ResizeToContents);
}

void ChaserEditor::updateItem(QTreeWidgetItem* item, ChaserStep& step)
{
    Function* function = step.resolveFunction(m_doc);
    Q_ASSERT(function != NULL);
    Q_ASSERT(item != NULL);

    m_tree->blockSignals(true);

    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    item->setText(COL_NUM, QString("%1").arg(m_tree->indexOfTopLevelItem(item) + 1));
    if (m_chaser->type() == Function::ChaserType)
    {
        item->setText(COL_NAME, function->name());
        item->setIcon(COL_NAME, function->getIcon());
    }

    if (step.note.isEmpty() == false)
        item->setText(COL_NOTES, step.note);
    step.fid = function->id();

    switch (m_chaser->fadeInMode())
    {
        case Chaser::Common:
            step.fadeIn = m_chaser->fadeInSpeed();
            item->setText(COL_FADEIN, Function::speedToString(step.fadeIn));
        break;
        case Chaser::PerStep:
            item->setText(COL_FADEIN, Function::speedToString(step.fadeIn));
        break;
        default:
            item->setText(COL_FADEIN, QString());
        break;
    }

    switch (m_chaser->fadeOutMode())
    {
        case Chaser::Common:
            step.fadeOut = m_chaser->fadeOutSpeed();
            item->setText(COL_FADEOUT, Function::speedToString(step.fadeOut));
        break;
        case Chaser::PerStep:
            item->setText(COL_FADEOUT, Function::speedToString(step.fadeOut));
        break;
        default:
            item->setText(COL_FADEOUT, QString());
        break;
    }

    switch (m_chaser->durationMode())
    {
        default:
        case Chaser::Common:
            step.duration = m_chaser->duration();
            step.hold = Function::speedSubtract(step.duration, step.fadeIn);
            item->setText(COL_HOLD, Function::speedToString(step.hold));
            item->setText(COL_DURATION, Function::speedToString(step.duration));
        break;
        case Chaser::PerStep:
            item->setText(COL_HOLD, Function::speedToString(step.hold));
            item->setText(COL_DURATION, Function::speedToString(step.duration));
        break;
    }

    m_tree->blockSignals(false);
}

void ChaserEditor::updateStepNumbers()
{
    for (int i = 0; i < m_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_tree->topLevelItem(i);
        Q_ASSERT(item != NULL);
        item->setText(COL_NUM, QString("%1").arg(i + 1));
    }
}

ChaserStep ChaserEditor::stepAtItem(const QTreeWidgetItem* item) const
{
    Q_ASSERT(item != NULL);

    int idx = item->text(COL_NUM).toInt() - 1;
    if (idx < 0 || idx >= m_chaser->steps().count())
        return ChaserStep();

    return m_chaser->steps().at(idx);
}

ChaserStep ChaserEditor::stepAtIndex(int index) const
{
    if (index < 0 || index >= m_chaser->steps().count())
        return ChaserStep();

    return m_chaser->steps().at(index);
}

void ChaserEditor::updateClipboardButtons()
{
    if (m_tree->selectedItems().size() > 0)
    {
        m_cutAction->setEnabled(true);
        m_copyAction->setEnabled(true);
    }
    else
    {
        m_cutAction->setEnabled(false);
        m_copyAction->setEnabled(false);
    }

    if (m_doc->clipboard()->hasChaserSteps())
        m_pasteAction->setEnabled(true);
    else
        m_pasteAction->setEnabled(false);
}

void ChaserEditor::applyStepValues()
{
    QList <QTreeWidgetItem*> selected(m_tree->selectedItems());

    if (selected.size() != 0)
    {
        QTreeWidgetItem* item(selected.first());
        int idx = m_tree->indexOfTopLevelItem(item);
        qDebug() << "Idx: " << idx << ", steps: " << m_chaser->steps().count();
        if (m_chaser != NULL && idx < m_chaser->steps().count())
        {
            ChaserStep step = m_chaser->steps().at(idx);

            if (step.values.count() > 0)
                emit applyValues(step.values);
        }
    }
}

void ChaserEditor::printSteps()
{
    int i = 0;
    QListIterator <ChaserStep> it(m_chaser->steps());
    while (it.hasNext() == true)
    {
        ChaserStep st(it.next());
        qDebug() << "Step #" << i << ": id: " << st.fid << ": fadeIn: " << st.fadeIn << ", fadeOut: " << st.fadeOut << ", duration: " << st.duration;
        if (st.values.count() > 0)
            qDebug() << "-----> values found: " << st.values.count();
    }
}
