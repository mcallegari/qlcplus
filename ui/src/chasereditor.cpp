/*
  Q Light Controller
  chasereditor.cpp

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QTreeWidgetItem>
#include <QRadioButton>
#include <QHeaderView>
#include <QTreeWidget>
#include <QPushButton>
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
#define COL_FADEOUT  3
#define COL_DURATION 4

ChaserEditor::ChaserEditor(QWidget* parent, Chaser* chaser, Doc* doc)
    : QWidget(parent)
    , m_doc(doc)
    , m_chaser(chaser)
{
    Q_ASSERT(chaser != NULL);
    Q_ASSERT(doc != NULL);

    setupUi(this);

    /* Resize columns to fit contents */
    m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);
    if (m_chaser->isSequence() == true)
    {
        m_tree->header()->setSectionHidden(COL_NAME, true);
        //groupBox->hide();
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

    connect(m_testButton, SIGNAL(toggled(bool)), this, SLOT(slotTestToggled(bool)));
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
}

ChaserEditor::~ChaserEditor()
{
    if (m_speedDials != NULL)
        delete m_speedDials;
    m_speedDials = NULL;

    if (m_testButton->isChecked() == true)
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
/*
        // here ? means a new value needs to be added
        step.values.append(sv);
        m_chaser->replaceStep(step, idx);
        qDebug() << Q_FUNC_INFO << "New value appended";
        printSteps();
*/
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
        ChaserStep step(m_chaser->getBoundedSceneID());
        QTreeWidgetItem* item = new QTreeWidgetItem;
        updateItem(item, step);
        // if this is the first step we add, then copy all DMX channels non-zero values
        Scene *currScene = qobject_cast<Scene*> (m_doc->function(m_chaser->getBoundedSceneID()));
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
        //updateChaserContents();
        updateClipboardButtons();
        printSteps();
    }
}

void ChaserEditor::slotRemoveClicked()
{
    slotCutClicked();
    m_clipboard.clear();
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
    //updateChaserContents();

    // Select the moved items
    it.toFront();
    while (it.hasNext() == true)
        it.next()->setSelected(true);

    updateClipboardButtons();
    printSteps();
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
    //updateChaserContents();

    // Select the items
    it.toFront();
    while (it.hasNext() == true)
        it.next()->setSelected(true);

    updateClipboardButtons();
    printSteps();
}

void ChaserEditor::slotItemSelectionChanged()
{
    if (m_chaser->isRunning() == false)
    {
        updateClipboardButtons();
        updateSpeedDials();
        applyStepValues();
    }
}

/****************************************************************************
 * Clipboard
 ****************************************************************************/

void ChaserEditor::slotCutClicked()
{
    m_clipboard.clear();
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item(it.next());
        m_clipboard << stepAtItem(item);
        int index = m_tree->indexOfTopLevelItem(item);
        m_chaser->removeStep(index);
        delete item;
    }

    m_tree->setCurrentItem(NULL);

    updateStepNumbers();
    //updateChaserContents();
    updateClipboardButtons();
}

void ChaserEditor::slotCopyClicked()
{
    m_clipboard.clear();
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    while (it.hasNext() == true)
        m_clipboard << stepAtItem(it.next());
    updateClipboardButtons();
}

void ChaserEditor::slotPasteClicked()
{
    if (m_clipboard.isEmpty() == true)
        return;

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

    QListIterator <ChaserStep> it(m_clipboard);
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem;
        ChaserStep step(it.next());
        updateItem(item, step);
        m_tree->insertTopLevelItem(insertionPoint, item);
        item->setSelected(true);
        insertionPoint = CLAMP(m_tree->indexOfTopLevelItem(item) + 1, 0, m_tree->topLevelItemCount() - 1);
        m_chaser->addStep(step, insertionPoint);
    }

    updateStepNumbers();
    //updateChaserContents();
    updateClipboardButtons();
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
            m_chaser->replaceStep(step, index);
            updateItem(item, step);
        }
        break;
    default:
    case Chaser::Default:
        break;
    }
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
            m_chaser->replaceStep(step, index);
            updateItem(item, step);
        }
        break;
    default:
    case Chaser::Default:
        break;
    }
}

void ChaserEditor::slotDurationDialChanged(int ms)
{
    switch (m_chaser->durationMode())
    {
    case Chaser::PerStep:
        foreach (QTreeWidgetItem* item, m_tree->selectedItems())
        {
            int index = m_tree->indexOfTopLevelItem(item);
            ChaserStep step = stepAtItem(item);
            step.duration = ms;
            m_chaser->replaceStep(step, index);
            updateItem(item, step);
        }
        break;
    case Chaser::Common:
        m_chaser->setDuration(ms);
        updateTree();
        break;
    default:
    case Chaser::Default:
        break;
    }
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
        connect(m_speedDials, SIGNAL(durationChanged(int)),
                this, SLOT(slotDurationDialChanged(int)));
    }

    m_speedDials->show();
}

void ChaserEditor::updateSpeedDials()
{
    static const QString fadeIn(tr("Fade In"));
    static const QString fadeOut(tr("Fade Out"));
    static const QString duration(tr("Duration"));
    static const QString globalFadeIn(tr("Common Fade In"));
    static const QString globalFadeOut(tr("Common Fade Out"));
    static const QString globalDuration(tr("Common Duration"));

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

    /* Duration */
    switch (m_chaser->durationMode())
    {
    default:
    case Chaser::Common:
        m_speedDials->setDuration(m_chaser->duration());
        m_speedDials->setDurationTitle(globalDuration);
        m_speedDials->setDurationEnabled(true);
        break;
    case Chaser::PerStep:
        if (selected.size() != 0)
        {
            m_speedDials->setDuration(step.duration);
            m_speedDials->setDurationEnabled(true);
        }
        else
        {
            m_speedDials->setFadeOutSpeed(0);
            m_speedDials->setDurationEnabled(false);
        }
        m_speedDials->setDurationTitle(duration);
        break;
    }
}

/****************************************************************************
 * Test
 ****************************************************************************/

void ChaserEditor::slotRestartTest()
{
    if (m_testButton->isChecked() == true)
    {
        // Toggle off, toggle on. Derp.
        m_testButton->click();
        m_testButton->click();
    }
}

void ChaserEditor::slotTestToggled(bool state)
{
    m_testPreviousButton->setEnabled(state);
    m_testNextButton->setEnabled(state);

    if (state == true)
    {
        if (m_chaser->stopped() == true)
            m_chaser->start(m_doc->masterTimer());
    }
    else
    {
        if (m_chaser->stopped() == false)
            m_chaser->stopAndWait();
    }
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
        m_testButton->setEnabled(false);
        if (m_testButton->isChecked() == true)
            m_chaser->stop();
    }
    else
    {
        m_testButton->setEnabled(true);
    }
}

void ChaserEditor::slotStepChanged(int stepNumber)
{
    if (m_tree->selectedItems().count() > 0)
        m_tree->selectedItems().first()->setSelected(false);
    m_tree->topLevelItem(stepNumber)->setSelected(true);
    //m_tree->itemAt(stepNumber)->setSelected(true);
}

bool ChaserEditor::interruptRunning()
{
    if (m_chaser->stopped() == false)
    {
        m_chaser->stopAndWait();
        m_testButton->setChecked(false);
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
            m_testButton->click();
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
}

void ChaserEditor::updateItem(QTreeWidgetItem* item, ChaserStep& step)
{
    Function* function = step.resolveFunction(m_doc);
    Q_ASSERT(function != NULL);
    Q_ASSERT(item != NULL);

    //item->setData(COL_NUM, PROP_STEP, step.toVariant());
    /*item->setText(COL_FADEIN, QString("%1").arg(step.fadeIn));
    item->setText(COL_FADEOUT, QString("%1").arg(step.fadeOut));
    item->setText(COL_DURATION, QString("%1").arg(step.duration));*/
    item->setText(COL_NUM, QString("%1").arg(m_tree->indexOfTopLevelItem(item) + 1));
    item->setText(COL_NAME, function->name());
    step.fid = function->id();

    switch (m_chaser->fadeInMode())
    {
    case Chaser::Common:
        item->setText(COL_FADEIN, Function::speedToString(m_chaser->fadeInSpeed()));
        step.fadeIn = m_chaser->fadeInSpeed();
        break;
    case Chaser::PerStep:
        item->setText(COL_FADEIN, Function::speedToString(step.fadeIn));
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
        break;
    default:
    case Chaser::Default:
        item->setText(COL_FADEOUT, QString());
    }

    switch (m_chaser->durationMode())
    {
    default:
    case Chaser::Common:
        item->setText(COL_DURATION, Function::speedToString(m_chaser->duration()));
        step.duration = m_chaser->duration();
        break;
    case Chaser::PerStep:
        item->setText(COL_DURATION, Function::speedToString(step.duration));
        break;
    }
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
/*
    QVariant var = item->data(COL_NUM, PROP_STEP);
    if (var.isValid() == true)
        return ChaserStep::fromVariant(var);
    else
        return ChaserStep();
*/

    int idx = item->text(COL_NUM).toInt() - 1;
    if (idx < 0)
        return ChaserStep();

    return m_chaser->steps().at(idx);
}

ChaserStep ChaserEditor::stepAtIndex(int index) const
{
/*
    if (index < 0 || index >= m_tree->topLevelItemCount())
        return ChaserStep();

    QTreeWidgetItem* item = m_tree->topLevelItem(index);
    return stepAtItem(item);
*/
    if (index < 0 || index > m_chaser->steps().count())
        return ChaserStep();

    return m_chaser->steps().at(index);
}

/*
void ChaserEditor::updateChaserContents()
{
    Q_ASSERT(m_chaser != NULL);
    qDebug() << "******************************* EVIL CLEAR *************************";

    // Stop running while modifying chaser contents
    bool running = interruptRunning();

    m_chaser->clear();
    for (int i = 0; i < m_tree->topLevelItemCount(); i++)
    {
        ChaserStep step = stepAtIndex(i);
        m_chaser->addStep(step);
    }

    // Continue running if appropriate
    continueRunning(running);
}
*/

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

    if (m_clipboard.size() > 0)
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


