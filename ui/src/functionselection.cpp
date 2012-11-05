/*
  Q Light Controller
  functionselection.cpp

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
#include <QTreeWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QToolBar>
#include <QDebug>

#include "functionselection.h"
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
#define KColumnType 1
#define KColumnID   2

/*****************************************************************************
 * Initialization
 *****************************************************************************/

FunctionSelection::FunctionSelection(QWidget* parent, Doc* doc)
    : QDialog(parent)
    , m_doc(doc)
    , m_multiSelection(true)
    , m_filter(Function::Scene | Function::Chaser | Function::Collection |
               Function::EFX | Function::Script | Function::RGBMatrix)
    , m_constFilter(false)
{
    Q_ASSERT(doc != NULL);

    setupUi(this);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

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
}

int FunctionSelection::exec()
{
    m_sceneCheck->setChecked(m_filter & Function::Scene);
    m_chaserCheck->setChecked(m_filter & Function::Chaser);
    m_efxCheck->setChecked(m_filter & Function::EFX);
    m_collectionCheck->setChecked(m_filter & Function::Collection);
    m_scriptCheck->setChecked(m_filter & Function::Script);
    m_rgbMatrixCheck->setChecked(m_filter & Function::RGBMatrix);

    if (m_constFilter == true)
    {
        m_sceneCheck->setEnabled(false);
        m_chaserCheck->setEnabled(false);
        m_efxCheck->setEnabled(false);
        m_collectionCheck->setEnabled(false);
        m_scriptCheck->setEnabled(false);
        m_rgbMatrixCheck->setEnabled(false);
    }

    /* Multiple/single selection */
    if (m_multiSelection == true)
        m_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    else
        m_tree->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(m_tree, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotItemSelectionChanged()));
    connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotItemDoubleClicked(QTreeWidgetItem*)));

    refillTree();
    m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);

    slotItemSelectionChanged();

    return QDialog::exec();
}

FunctionSelection::~FunctionSelection()
{
}

/*****************************************************************************
 * Multi-selection
 *****************************************************************************/

void FunctionSelection::setMultiSelection(bool multi)
{
    m_multiSelection = multi;
}

/*****************************************************************************
 * Filter
 *****************************************************************************/

void FunctionSelection::setFilter(int types, bool constFilter)
{
    m_filter = types;
    m_constFilter = constFilter;
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

void FunctionSelection::updateFunctionItem(QTreeWidgetItem* item, Function* function)
{
    item->setText(KColumnName, function->name());
    item->setText(KColumnType, function->typeString());
    item->setText(KColumnID, QString::number(function->id()));
}

void FunctionSelection::refillTree()
{
    m_tree->clear();

    /* Fill the tree */
    foreach (Function* function, m_doc->functions())
    {
        if (m_filter & function->type())
        {
            QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
            updateFunctionItem(item, function);

            if (disabledFunctions().contains(function->id()))
                item->setFlags(0); // Disables the item
        }
    }
}

void FunctionSelection::slotItemSelectionChanged()
{
    QList <quint32> removeList(m_selection);

    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    while (it.hasNext() == true)
    {
        quint32 id = it.next()->text(KColumnID).toInt();
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
