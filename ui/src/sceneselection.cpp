/*
  Q Light Controller
  sceneselection.cpp

  Copyright (c) Massimo Callegari

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
#include <QPushButton>
#include <QDebug>

#include "sceneselection.h"
#include "scene.h"
#include "doc.h"

#define KColumnName 0
#define KColumnID   1

/*****************************************************************************
 * Initialization
 *****************************************************************************/

SceneSelection::SceneSelection(QWidget* parent, Doc* doc)
    : QDialog(parent)
    , m_doc(doc)
    , m_selectedID(Scene::invalidId())
{
    Q_ASSERT(doc != NULL);

    setupUi(this);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);
}

int SceneSelection::exec()
{
    connect(m_tree, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotItemSelectionChanged()));
    connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotItemDoubleClicked(QTreeWidgetItem*)));

    refillTree();

    slotItemSelectionChanged();

    return QDialog::exec();
}

void SceneSelection::accept()
{
    QTreeWidgetItem* selItem = m_tree->selectedItems().first();
    m_selectedID = selItem->text(KColumnID).toUInt();
    QDialog::accept();
}

SceneSelection::~SceneSelection()
{
}

quint32 SceneSelection::getSelectedID()
{
    return m_selectedID;
}

/*****************************************************************************
 * Disabled functions
 *****************************************************************************/

void SceneSelection::setDisabledScenes(const QList <quint32>& ids)
{
    m_disabledScenes = ids;
}

QList <quint32> SceneSelection::disabledScenes() const
{
    return m_disabledScenes;
}

/*****************************************************************************
 * Tree
 *****************************************************************************/

void SceneSelection::updateFunctionItem(QTreeWidgetItem* item, Function* function)
{
    item->setText(KColumnName, function->name());
    item->setText(KColumnID, QString::number(function->id()));
}

void SceneSelection::refillTree()
{
    m_tree->clear();

    /* Add a convenience entry to create a Scene on the fly */
    QTreeWidgetItem *newItem = new QTreeWidgetItem(m_tree);
    newItem->setText(KColumnName, tr("<Create a new scene>"));
    newItem->setText(KColumnID, QString::number(Scene::invalidId()));

    /* Fill the tree */
    foreach (Function* function, m_doc->functions())
    {
        if (function->type() == Function::Scene)
        {
            QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
            updateFunctionItem(item, function);

            if (disabledScenes().contains(function->id()))
                item->setFlags(0); // Disables the item
        }
    }
}

void SceneSelection::slotItemSelectionChanged()
{
    if (m_tree->selectedItems().count() > 0)
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    else
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

void SceneSelection::slotItemDoubleClicked(QTreeWidgetItem* item)
{
    if (item == NULL)
        return;

    accept();
}
