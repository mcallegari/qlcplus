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

#include "qlcfixturedef.h"
#include "qlcmacros.h"

#include "functionselection.h"
#include "speeddialwidget.h"
#include "chasereditor.h"
#include "mastertimer.h"
#include "chaserstep.h"
#include "outputmap.h"
#include "inputmap.h"
#include "apputil.h"
#include "fixture.h"
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
    , m_liveMode(liveMode)
{
    Q_ASSERT(chaser != NULL);
    Q_ASSERT(doc != NULL);

    setupUi(this);

    /* Disable editing of steps number */
    m_tree->setItemDelegateForColumn(COL_NUM, new NoEditDelegate(this));
    if (m_chaser->isSequence() == true)
    {
        m_tree->header()->setSectionHidden(COL_NAME, true);
        groupBox->hide();
        m_pingPong->hide();
        groupBox_2->hide();
    }

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
        delete m_speedDials;
    m_speedDials = NULL;

    // double check that the Chaser still exists !
    if (m_liveMode == false &&
        m_doc->functions().contains(m_chaser) == true &&
        m_chaser->stopped() == false)
            m_chaser->stopAndWait();
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
            delete m_speedDials;
        m_speedDials = NULL;
    }
}

void ChaserEditor::slotNameEdited(const QString& text)
{
    m_chaser->setName(text);
}

void ChaserEditor::slotUpdateCurrentStep(SceneValue sv)
{
    //qDebug() << "Value changed: " << sv.fxi << sv.channel << sv.value;
    QList <QTreeWidgetItem*> selected(m_tree->selectedItems());

    if (selected.size() > 0)
    {
        QTreeWidgetItem* item(selected.first());
        int idx = m_tree->indexOfTopLevelItem(item);
        ChaserStep step = m_chaser->steps().at(idx);
        for (int i = 0; i < step.values.count(); i++)
        {
            if (step.values.at(i) == sv)
            {
                step.values.replace(i, sv);
                m_chaser->replaceStep(step, idx);
                //qDebug() << Q_FUNC_INFO << "Value replaced at pos: " << i;
                return;
            }
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

    if (m_chaser->isSequence() == true)
    {
        ChaserStep step(m_chaser->getBoundSceneID());
        QTreeWidgetItem* item = new QTreeWidgetItem;
        updateItem(item, step);
        // if this is the first step we add, then copy all DMX channels non-zero values
        Scene *currScene = qobject_cast<Scene*> (m_doc->function(m_chaser->getBoundSceneID()));
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
        fs.setDisabledFunctions(QList <quint32>() << m_chaser->id());

        if (fs.exec() == QDialog::Accepted)
        {
            /* Append selected functions */
            QListIterator <quint32> it(fs.selection());
            while (it.hasNext() == true)
            {
                ChaserStep step(it.next());
                QTreeWidgetItem* item = new QTreeWidgetItem;
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
        m_tree->setCurrentItem(item);
        updateStepNumbers();
        updateClipboardButtons();
        //printSteps();
    }
}

void ChaserEditor::slotRemoveClicked()
{
    slotCutClicked();
    //m_clipboard.clear();
    updateClipboardButtons();
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

void ChaserEditor::slotSpeedDialToggle(bool state)
{
    if (state == true)
        updateSpeedDials();
    else
    {
        if (m_speedDials != NULL)
            delete m_speedDials;
        m_speedDials = NULL;
    }
}

void ChaserEditor::slotItemSelectionChanged()
{
    if (m_chaser->isRunning() == false)
    {
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
}

void ChaserEditor::slotItemChanged(QTreeWidgetItem *item, int column)
{
    QString itemText = item->text(column);
    quint32 newValue = 0;
    int idx = m_tree->indexOfTopLevelItem(item);

    if (itemText.contains(".") || itemText.contains("s") ||
        itemText.contains("m") || itemText.contains("h"))
            newValue = Function::stringToSpeed(itemText);
    else
        newValue = (itemText.toDouble() * 1000);

    ChaserStep step = m_chaser->steps().at(idx);
    if (column == COL_FADEIN)
    {
        step.fadeIn = newValue;
        step.duration = step.fadeIn + step.hold;
    }
    else if (column == COL_HOLD)
    {
        step.hold = newValue;
        if (m_chaser->fadeInMode() == Chaser::Common)
            step.duration = m_chaser->fadeInSpeed() + step.hold;
        else
            step.duration = step.fadeIn + step.hold;
    }
    else if (column == COL_FADEOUT)
    {
        step.fadeOut = newValue;
        step.duration = step.fadeIn + step.hold;
    }
    else if (column == COL_DURATION)
    {
        qint32 newDuration = (qint32)newValue - (qint32)step.fadeIn;
        if (newDuration >= 0)
        {
            step.duration = newValue;
            step.hold = step.duration - step.fadeIn;
        }
    }
    else if (column == COL_NOTES)
    {
        step.note = itemText;
    }
    m_chaser->replaceStep(step, idx);
    updateItem(item, step);

    m_tree->resizeColumnToContents(COL_NUM);
    m_tree->resizeColumnToContents(COL_NAME);
    m_tree->resizeColumnToContents(COL_FADEIN);
    m_tree->resizeColumnToContents(COL_HOLD);
    m_tree->resizeColumnToContents(COL_FADEOUT);
    m_tree->resizeColumnToContents(COL_DURATION);
    m_tree->resizeColumnToContents(COL_NOTES);
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
        copyList << stepAtItem(item);
        int index = m_tree->indexOfTopLevelItem(item);
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
    //m_clipboard.clear();
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
    if (m_chaser->isSequence())
    {
        quint32 sceneID = m_chaser->getBoundSceneID();
        Scene *scene = qobject_cast<Scene*>(m_doc->function(sceneID));
        foreach(ChaserStep step, pasteList)
        {
            if (step.fid != sceneID) // if IDs are the same then it's a valid step
            {
                foreach(SceneValue scv, step.values)
                {
                    if (scene->checkValue(scv) == false)
                    {
                        QMessageBox::warning(this, tr("Paste error"), tr("Trying to paste on an incompatible Scene. Operation cancelled."));
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
        m_chaser->setFadeInSpeed(ms);
        updateTree();
        break;
    case Chaser::PerStep:
        foreach (QTreeWidgetItem* item, m_tree->selectedItems())
        {
            int index = m_tree->indexOfTopLevelItem(item);
            ChaserStep step = stepAtItem(item);
            step.fadeIn = ms;
            step.duration = step.fadeIn + step.hold;
            m_chaser->replaceStep(step, index);
            updateItem(item, step);
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
        m_chaser->setFadeOutSpeed(ms);
        updateTree();
        break;
    case Chaser::PerStep:
        foreach (QTreeWidgetItem* item, m_tree->selectedItems())
        {
            int index = m_tree->indexOfTopLevelItem(item);
            ChaserStep step = stepAtItem(item);
            step.fadeOut = ms;
            step.duration = step.fadeIn + step.hold;
            m_chaser->replaceStep(step, index);
            updateItem(item, step);
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
    case Chaser::PerStep:
        foreach (QTreeWidgetItem* item, m_tree->selectedItems())
        {
            int index = m_tree->indexOfTopLevelItem(item);
            ChaserStep step = stepAtItem(item);
            step.hold = ms;
            if (ms < 0)
                step.duration = ms;
            else
                step.duration = step.fadeIn + step.hold;
            m_chaser->replaceStep(step, index);
            updateItem(item, step);
        }
        break;
    case Chaser::Common:
    {
        uint duration = m_chaser->fadeInSpeed() + ms;
        m_chaser->setDuration(duration);
        updateTree();
        break;
    }
    default:
    case Chaser::Default:
        break;
    }

    m_tree->resizeColumnToContents(COL_HOLD);
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
    QList <QTreeWidgetItem*> selected(m_tree->selectedItems());
    int index = 0;
    if (selected.size() > 0)
    {
        QTreeWidgetItem* item(selected.first());
        index = m_tree->indexOfTopLevelItem(item);
    }
    return index;
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

    if (m_chaser->stopped() == true)
    {
        int idx = getCurrentIndex();
        if (idx >= 0)
            m_chaser->setStepIndex(idx);
        m_chaser->start(m_doc->masterTimer());
    }
}

void ChaserEditor::slotTestStop()
{
    m_testPreviousButton->setEnabled(false);
    m_testNextButton->setEnabled(false);

    if (m_chaser->stopped() == false)
        m_chaser->stopAndWait();
}

void ChaserEditor::slotTestPreviousClicked()
{
    m_chaser->previous();
}

void ChaserEditor::slotTestNextClicked()
{
    m_chaser->next();
}

void ChaserEditor::slotModeChanged(Doc::Mode mode)
{
    if (mode == Doc::Operate)
    {
        m_testPlayButton->setEnabled(false);
        m_testStopButton->setEnabled(false);
        if (m_liveMode == false && m_chaser->stopped() == false)
            m_chaser->stop();
    }
    else
    {
        m_testPlayButton->setEnabled(true);
        m_testStopButton->setEnabled(true);
    }
}

void ChaserEditor::slotStepChanged(int stepNumber)
{
    if (m_tree->selectedItems().count() > 0)
        m_tree->selectedItems().first()->setSelected(false);
    m_tree->topLevelItem(stepNumber)->setSelected(true);
}

bool ChaserEditor::interruptRunning()
{
    if (m_chaser->stopped() == false)
    {
        m_chaser->stopAndWait();
        return true;
    }
    else
    {
        return false;
    }
}

void ChaserEditor::continueRunning(bool running)
{
    if (running == true)
    {
        if (m_doc->mode() == Doc::Operate)
            m_chaser->start(m_doc->masterTimer());
        else
            m_testStopButton->click();
    }
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

    m_tree->resizeColumnToContents(COL_NUM);
    m_tree->resizeColumnToContents(COL_NAME);
    m_tree->resizeColumnToContents(COL_FADEIN);
    m_tree->resizeColumnToContents(COL_HOLD);
    m_tree->resizeColumnToContents(COL_FADEOUT);
    m_tree->resizeColumnToContents(COL_DURATION);
    m_tree->resizeColumnToContents(COL_NOTES);
}

void ChaserEditor::updateItem(QTreeWidgetItem* item, ChaserStep& step)
{
    Function* function = step.resolveFunction(m_doc);
    Q_ASSERT(function != NULL);
    Q_ASSERT(item != NULL);

    m_tree->blockSignals(true);

    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    item->setText(COL_NUM, QString("%1").arg(m_tree->indexOfTopLevelItem(item) + 1));
    item->setText(COL_NAME, function->name());
    if (step.note.isEmpty() == false)
        item->setText(COL_NOTES, step.note);
    step.fid = function->id();

    switch (m_chaser->fadeInMode())
    {
    case Chaser::Common:
        item->setText(COL_FADEIN, Function::speedToString(m_chaser->fadeInSpeed()));
        item->setText(COL_DURATION, Function::speedToString(m_chaser->duration()));
        step.fadeIn = m_chaser->fadeInSpeed();
        break;
    case Chaser::PerStep:
        item->setText(COL_FADEIN, Function::speedToString(step.fadeIn));
        item->setText(COL_DURATION, Function::speedToString(step.duration));
        break;
    default:
    case Chaser::Default:
        item->setText(COL_FADEIN, QString());
    }

    switch (m_chaser->fadeOutMode())
    {
    case Chaser::Common:
        item->setText(COL_FADEOUT, Function::speedToString(m_chaser->fadeOutSpeed()));
        step.fadeOut = m_chaser->fadeOutSpeed();
        break;
    case Chaser::PerStep:
        item->setText(COL_FADEOUT, Function::speedToString(step.fadeOut));
        item->setText(COL_DURATION, Function::speedToString(step.duration));
        break;
    default:
    case Chaser::Default:
        item->setText(COL_FADEOUT, QString());
    }

    switch (m_chaser->durationMode())
    {
    default:
    case Chaser::Common:
        step.duration = m_chaser->duration();
        step.hold = step.duration - m_chaser->fadeInSpeed();
        if (step.duration == Function::infiniteSpeed())
            item->setText(COL_HOLD, Function::speedToString(step.duration));
        else
            item->setText(COL_HOLD, Function::speedToString(step.hold));
        item->setText(COL_DURATION, Function::speedToString(m_chaser->duration()));
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


