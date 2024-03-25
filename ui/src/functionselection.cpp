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
#include <QPushButton>
#include <QSettings>
#include <QToolBar>
#include <QDebug>

#include "functionselection.h"
#include "functionstreewidget.h"
#include "doc.h"

#define KColumnName 0

#define SETTINGS_GEOMETRY "functionselect/geometry"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

FunctionSelection::FunctionSelection(QWidget* parent, Doc* doc)
    : QDialog(parent)
    , m_doc(doc)
    , m_isInitializing(true)
    , m_none(false)
    , m_noneItem(NULL)
    , m_newTrack(false)
    , m_newTrackItem(NULL)
    , m_multiSelection(true)
    , m_runningOnlyFlag(false)
    , m_filter(Function::SceneType | Function::ChaserType | Function::SequenceType | Function::CollectionType |
               Function::EFXType | Function::ScriptType | Function::RGBMatrixType |
               Function::ShowType | Function::AudioType | Function::VideoType)
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

    connect(m_sequenceCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotSequenceChecked(bool)));

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

    connect(m_videoCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotVideoChecked(bool)));

    QSettings settings;
    QVariant var = settings.value(SETTINGS_FILTER);
    if (var.isValid() == true)
        setFilter(var.toInt(), false);

    var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        restoreGeometry(var.toByteArray());
}

int FunctionSelection::exec()
{
    m_sceneCheck->setChecked(m_filter & Function::SceneType);
    m_chaserCheck->setChecked(m_filter & Function::ChaserType);
    m_sequenceCheck->setChecked(m_filter & Function::SequenceType);
    m_efxCheck->setChecked(m_filter & Function::EFXType);
    m_collectionCheck->setChecked(m_filter & Function::CollectionType);
    m_scriptCheck->setChecked(m_filter & Function::ScriptType);
    m_rgbMatrixCheck->setChecked(m_filter & Function::RGBMatrixType);
    m_showCheck->setChecked(m_filter & Function::ShowType);
    m_audioCheck->setChecked(m_filter & Function::AudioType);
    m_videoCheck->setChecked(m_filter & Function::VideoType);

    if (m_constFilter == true)
    {
        m_sceneCheck->setEnabled(false);
        m_chaserCheck->setEnabled(false);
        m_sequenceCheck->setEnabled(false);
        m_efxCheck->setEnabled(false);
        m_collectionCheck->setEnabled(false);
        m_scriptCheck->setEnabled(false);
        m_rgbMatrixCheck->setEnabled(false);
        m_showCheck->setEnabled(false);
        m_audioCheck->setEnabled(false);
        m_videoCheck->setEnabled(false);
    }
    else
    {
        m_sceneCheck->setDisabled(m_disableFilters & Function::SceneType);
        m_chaserCheck->setDisabled(m_disableFilters & Function::ChaserType);
        m_sequenceCheck->setDisabled(m_disableFilters & Function::SequenceType);
        m_efxCheck->setDisabled(m_disableFilters & Function::EFXType);
        m_collectionCheck->setDisabled(m_disableFilters & Function::CollectionType);
        m_scriptCheck->setDisabled(m_disableFilters & Function::ScriptType);
        m_rgbMatrixCheck->setDisabled(m_disableFilters & Function::RGBMatrixType);
        m_showCheck->setDisabled(m_disableFilters & Function::ShowType);
        m_audioCheck->setDisabled(m_disableFilters & Function::AudioType);
        m_videoCheck->setDisabled(m_disableFilters & Function::VideoType);
    }

    /* Multiple/single selection */
    if (m_multiSelection == true)
        m_funcTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    else
        m_funcTree->setSelectionMode(QAbstractItemView::SingleSelection);

    m_isInitializing = false;

    refillTree();

    connect(m_funcTree, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotItemSelectionChanged()));
    connect(m_funcTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotItemDoubleClicked(QTreeWidgetItem*)));

    slotItemSelectionChanged();

    return QDialog::exec();
}

void FunctionSelection::showNone(bool show)
{
    m_none = show;

    refillTree();
}

void FunctionSelection::showNewTrack(bool show)
{
    m_newTrack = show;

    refillTree();
}

FunctionSelection::~FunctionSelection()
{
    if (!m_constFilter)
    {
        QSettings settings;
        settings.setValue(SETTINGS_FILTER, m_filter);
    }

    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
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

void FunctionSelection::setSelection(QList<quint32> selection)
{
    m_selection = selection;
}

const QList <quint32> FunctionSelection::selection() const
{
    return m_selection;
}

/*****************************************************************************
 * Tree
 *****************************************************************************/

void FunctionSelection::refillTree()
{
    if (m_isInitializing == true)
        return;

    // m_selection will be erased when calling setSelected() on each new item.
    // A backup in made so current selection is applied correctly.
    QList<quint32> selection = m_selection;

    m_funcTree->clearTree();

    // Show a "none" entry
    if (m_none == true)
    {
        m_noneItem = new QTreeWidgetItem(m_funcTree);
        m_noneItem->setText(KColumnName, tr("<No function>"));
        m_noneItem->setIcon(KColumnName, QIcon(":/uncheck.png"));
        m_noneItem->setData(KColumnName, Qt::UserRole, Function::invalidId());
        m_noneItem->setSelected(selection.contains(Function::invalidId()));
    }

    if (m_newTrack == true)
    {
        m_newTrackItem = new QTreeWidgetItem(m_funcTree);
        m_newTrackItem->setText(KColumnName, tr("<Create a new track>"));
        m_newTrackItem->setIcon(KColumnName, QIcon(":/edit_add.png"));
        m_newTrackItem->setData(KColumnName, Qt::UserRole, Function::invalidId());
    }

    /* Fill the tree */
    foreach (Function* function, m_doc->functions())
    {
        if (m_runningOnlyFlag == true && !function->isRunning())
            continue;

        if (function->isVisible() == false)
            continue;

        if (m_filter & function->type())
        {
            QTreeWidgetItem* item = m_funcTree->addFunction(function->id());
            if (disabledFunctions().contains(function->id()))
                item->setFlags(Qt::NoItemFlags); // Disable the item
            else
                item->setSelected(selection.contains(function->id()));
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
    m_selection.clear();

    QListIterator <QTreeWidgetItem*> it(m_funcTree->selectedItems());
    while (it.hasNext() == true)
    {
        QTreeWidgetItem *item = it.next();
        quint32 id = item->data(KColumnName, Qt::UserRole).toUInt();
        if ((id != Function::invalidId() || item == m_noneItem || item == m_newTrackItem)
             && m_selection.contains(id) == false)
                    m_selection.append(id);
    }

    if (m_selection.isEmpty() == true)
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    else
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void FunctionSelection::slotItemDoubleClicked(QTreeWidgetItem* item)
{
    if (item == NULL)
        return;

    if (m_buttonBox->button(QDialogButtonBox::Ok)->isEnabled() == false)
        return;

    accept();
}

void FunctionSelection::slotSceneChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::SceneType);
    else
        m_filter = (m_filter & ~Function::SceneType);
    refillTree();
}

void FunctionSelection::slotChaserChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::ChaserType);
    else
        m_filter = (m_filter & ~Function::ChaserType);
    refillTree();
}

void FunctionSelection::slotSequenceChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::SequenceType);
    else
        m_filter = (m_filter & ~Function::SequenceType);
    refillTree();
}

void FunctionSelection::slotEFXChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::EFXType);
    else
        m_filter = (m_filter & ~Function::EFXType);
    refillTree();
}

void FunctionSelection::slotCollectionChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::CollectionType);
    else
        m_filter = (m_filter & ~Function::CollectionType);
    refillTree();
}

void FunctionSelection::slotScriptChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::ScriptType);
    else
        m_filter = (m_filter & ~Function::ScriptType);
    refillTree();
}

void FunctionSelection::slotRGBMatrixChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::RGBMatrixType);
    else
        m_filter = (m_filter & ~Function::RGBMatrixType);
    refillTree();
}

void FunctionSelection::slotShowChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::ShowType);
    else
        m_filter = (m_filter & ~Function::ShowType);
    refillTree();
}

void FunctionSelection::slotAudioChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::AudioType);
    else
        m_filter = (m_filter & ~Function::AudioType);
    refillTree();
}

void FunctionSelection::slotVideoChecked(bool state)
{
    if (state == true)
        m_filter = (m_filter | Function::VideoType);
    else
        m_filter = (m_filter & ~Function::VideoType);
    refillTree();
}
