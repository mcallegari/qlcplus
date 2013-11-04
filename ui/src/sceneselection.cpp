/*
  Q Light Controller
  sceneselection.cpp

  Copyright (c) Massimo Callegari

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
