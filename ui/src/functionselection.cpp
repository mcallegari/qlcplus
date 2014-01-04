/*
  Q Light Controller
  functionselection.cpp

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
#include <QTreeWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QToolBar>
#include <QDebug>
#include <QSettings>

#include "functionselection.h"
#include "functionstreewidget.h"
#include "collectioneditor.h"
#include "rgbmatrixeditor.h"
#include "chasereditor.h"
#include "scripteditor.h"
#include "sceneeditor.h"
#include "mastertimer.h"
#include "collection.h"
#include "outputmap.h"
#include "rgbmatrix.h"
#include "efxeditor.h"
#include "inputmap.h"
#include "function.h"
#include "fixture.h"
#include "chaser.h"
#include "script.h"
#include "scene.h"
#include "efx.h"
#include "doc.h"

#define KColumnName 0

/*****************************************************************************
 * Initialization
 *****************************************************************************/

FunctionSelection::FunctionSelection(QWidget* parent, Doc* doc)
    : QDialog(parent)
    , m_doc(doc)
    , m_multiSelection(true)
    , m_runningOnlyFlag(false)
    , m_filter(Function::Scene | Function::Chaser | Function::Collection |
               Function::EFX | Function::Script | Function::RGBMatrix | Function::Show | Function::Audio)
    , m_disableFilters(0)
    , m_constFilter(false)
{
    Q_ASSERT(doc != NULL);

    setupUi(this);

    m_funcTree = new FunctionsTreeWidget(m_doc, this);
    QStringList labels;
    labels << tr("Functions");
    m_funcTree->setHeaderLabels(labels);
    m_funcTree->setRootIsDecorated(true);
    m_funcTree->setAllColumnsShowFocus(true);
    m_funcTree->setSortingEnabled(true);
    m_funcTree->sortByColumn(KColumnName, Qt::AscendingOrder);
    m_treeVbox->addWidget(m_funcTree);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    connect(m_allFunctionsRadio, SIGNAL(clicked()),
            this, SLOT(slotAllFunctionsChecked()));

    connect(m_runningFunctionsRadio, SIGNAL(clicked()),
            this, SLOT(slotRunningFunctionsChecked()));

    connect(m_sceneCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotSceneChecked(bool)));

    connect(m_chaserCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotChaserChecked(bool)));

    connect(m_efxCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotEFXChecked(bool)));

    connect(m_collectionCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotCollectionChecked(bool)));

    connect(m_scriptCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotScriptChecked(bool)));

    connect(m_rgbMatrixCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotRGBMatrixChecked(bool)));

    connect(m_showCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotShowChecked(bool)));

    connect(m_audioCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotAudioChecked(bool)));

    QSettings settings;
    QVariant var = settings.value(SETTINGS_FILTER);
    if (var.isValid() == true)
    {
        setFilter(var.toInt(), false);
    }
}

int FunctionSelection::exec()
{
    m_sceneCheck->setChecked(m_filter & Function::Scene);
    m_chaserCheck->setChecked(m_filter & Function::Chaser);
    m_efxCheck->setChecked(m_filter & Function::EFX);
    m_collectionCheck->setChecked(m_filter & Function::Collection);
    m_scriptCheck->setChecked(m_filter & Function::Script);
    m_rgbMatrixCheck->setChecked(m_filter & Function::RGBMatrix);
    m_showCheck->setChecked(m_filter & Function::Show);
    m_audioCheck->setChecked(m_filter & Function::Audio);

    if (m_constFilter == true)
    {
        m_sceneCheck->setEnabled(false);
        m_chaserCheck->setEnabled(false);
        m_efxCheck->setEnabled(false);
        m_collectionCheck->setEnabled(false);
        m_scriptCheck->setEnabled(false);
        m_rgbMatrixCheck->setEnabled(false);
        m_showCheck->setEnabled(false);
        m_audioCheck->setEnabled(false);
    }
    else
    {
        m_sceneCheck->setDisabled(m_disableFilters & Function::Scene);
        m_chaserCheck->setDisabled(m_disableFilters & Function::Chaser);
        m_efxCheck->setDisabled(m_disableFilters & Function::EFX);
        m_collectionCheck->setDisabled(m_disableFilters & Function::Collection);
        m_scriptCheck->setDisabled(m_disableFilters & Function::Script);
        m_rgbMatrixCheck->setDisabled(m_disableFilters & Function::RGBMatrix);
        m_showCheck->setDisabled(m_disableFilters & Function::Show);
        m_audioCheck->setDisabled(m_disableFilters & Function::Audio);
    }

    /* Multiple/single selection */
    if (m_multiSelection == true)
        m_funcTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    else
        m_funcTree->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(m_funcTree, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotItemSelectionChanged()));
    connect(m_funcTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotItemDoubleClicked(QTreeWidgetItem*)));

    refillTree();

    slotItemSelectionChanged();

    return QDialog::exec();
}

void FunctionSelection::showNone(bool show)
{
    m_none = show;

    refillTree();
}

FunctionSelection::~FunctionSelection()
{
    if(!m_constFilter)
    {
        QSettings settings;
        settings.setValue(SETTINGS_FILTER, m_filter);
    }
}

/*****************************************************************************
 * Multi-selection
 *****************************************************************************/

void FunctionSelection::setMultiSelection(bool multi)
{
    m_multiSelection = multi;
}

/*********************************************************************
 * Functions filter
 *********************************************************************/

void FunctionSelection::slotAllFunctionsChecked()
{
    m_runningOnlyFlag = false;
    refillTree();
}

void FunctionSelection::slotRunningFunctionsChecked()
{
    m_runningOnlyFlag = true;
    refillTree();
}

/*****************************************************************************
 * Filter
 *****************************************************************************/

void FunctionSelection::setFilter(int types, bool constFilter)
{
    m_filter = types;
    m_constFilter = constFilter;
}

void FunctionSelection::disableFilters(int types)
{
    m_disableFilters = types;
}

/*****************************************************************************
 * Disabled functions
 *****************************************************************************/

void FunctionSelection::setDisabledFunctions(const QList <quint32>& ids)
{
    m_disabledFunctions = ids;
}

QList <quint32> FunctionSelection::disabledFunctions() const
{
    return m_disabledFunctions;
}

/*****************************************************************************
 * Selection
 *****************************************************************************/

const QList <quint32> FunctionSelection::selection() const
{
    return m_selection;
}

/*****************************************************************************
 * Tree
 *****************************************************************************/

void FunctionSelection::refillTree()
{
    m_funcTree->clearTree();

    // Show a "none" entry
    if (m_none == true)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(m_funcTree);
        item->setText(KColumnName, tr("<No function>"));
        item->setIcon(KColumnName, QIcon(":/uncheck.png"));
        item->setData(KColumnName, Qt::UserRole, Function::invalidId());
    }

    /* Fill the tree */
    foreach (Function* function, m_doc->functions())
    {
        if (m_filter & function->type())
        {
            if (m_runningOnlyFlag == true && function->isRunning() == false)
                continue;

            QTreeWidgetItem* item = m_funcTree->functionAdded(function->id());
            if (disabledFunctions().contains(function->id()))
                item->setFlags(0); // Disables the item
        }
    }
    m_funcTree->resizeColumnToContents(KColumnName);
    for (int i = 0; i < m_funcTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = m_funcTree->topLevelItem(i);
        m_funcTree->expandItem(item);
    }
}

void FunctionSelection::slotItemSelectionChanged()
{
    QList <quint32> removeList(m_selection);

    QListIterator <QTreeWidgetItem*> it(m_funcTree->selectedItems());
    while (it.hasNext() == true)
    {
        quint32 id = it.next()->data(KColumnName, Qt::UserRole).toUInt();
        if (m_selection.contains(id) == false)
            m_selection.append(id);

        removeList.removeAll(id);
    }

    while (removeList.isEmpty() == false)
        m_selection.removeAll(removeList.takeFirst());

    if (m_selection.isEmpty() == true)
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    else
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void FunctionSelection::slotItemDoubleClicked(QTreeWidgetItem* item)
{
    if (item == NULL)
        return;

    accept();
}

void FunctionSelection::slotSceneChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::Scene);
    else
        m_filter = (m_filter & ~Function::Scene);
    refillTree();
}

void FunctionSelection::slotChaserChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::Chaser);
    else
        m_filter = (m_filter & ~Function::Chaser);
    refillTree();
}

void FunctionSelection::slotEFXChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::EFX);
    else
        m_filter = (m_filter & ~Function::EFX);
    refillTree();
}

void FunctionSelection::slotCollectionChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::Collection);
    else
        m_filter = (m_filter & ~Function::Collection);
    refillTree();
}

void FunctionSelection::slotScriptChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::Script);
    else
        m_filter = (m_filter & ~Function::Script);
    refillTree();
}

void FunctionSelection::slotRGBMatrixChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::RGBMatrix);
    else
        m_filter = (m_filter & ~Function::RGBMatrix);
    refillTree();
}

void FunctionSelection::slotShowChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::Show);
    else
        m_filter = (m_filter & ~Function::Show);
    refillTree();
}

void FunctionSelection::slotAudioChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::Audio);
    else
        m_filter = (m_filter & ~Function::Audio);
    refillTree();
}
