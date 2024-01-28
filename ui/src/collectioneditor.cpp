/*
  Q Light Controller
  collectioneditor.cpp

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
#include <QSettings>
#include <QLineEdit>
#include <QLabel>

#include "functionselection.h"
#include "collectioneditor.h"
#include "mastertimer.h"
#include "collection.h"
#include "function.h"
#include "doc.h"

#define PROP_ID Qt::UserRole

CollectionEditor::CollectionEditor(QWidget* parent, Collection* fc, Doc* doc)
    : QWidget(parent)
    , m_doc(doc)
    , m_collection(fc)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(fc != NULL);

    setupUi(this);

    connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotNameEdited(const QString&)));
    connect(m_add, SIGNAL(clicked()), this, SLOT(slotAdd()));
    connect(m_remove, SIGNAL(clicked()), this, SLOT(slotRemove()));
    connect(m_moveUp, SIGNAL(clicked()), this, SLOT(slotMoveUp()));
    connect(m_moveDown, SIGNAL(clicked()), this, SLOT(slotMoveDown()));
    connect(m_testButton, SIGNAL(clicked()),
            this, SLOT(slotTestClicked()));

    m_nameEdit->setText(m_collection->name());
    m_nameEdit->setSelection(0, m_nameEdit->text().length());

    updateFunctionList();

    // Set focus to the editor
    m_nameEdit->setFocus();
}

CollectionEditor::~CollectionEditor()
{
    if (m_testButton->isChecked())
        m_collection->stopAndWait ();
}

void CollectionEditor::slotNameEdited(const QString& text)
{
    m_collection->setName(text);
}

void CollectionEditor::slotAdd()
{
    FunctionSelection fs(this, m_doc);
    {
        QList<quint32> disabledList;
        disabledList << m_collection->id();
        foreach (Function* function, m_doc->functions())
        {
            if (function->contains(m_collection->id()))
                disabledList << function->id();
        }
        fs.setDisabledFunctions(disabledList);
    }

    if (fs.exec() == QDialog::Accepted)
    {
        QListIterator <quint32> it(fs.selection());
        while (it.hasNext() == true)
            m_collection->addFunction(it.next());
        updateFunctionList();
    }
}

void CollectionEditor::slotRemove()
{
    QList <QTreeWidgetItem*> items(m_tree->selectedItems());
    QListIterator <QTreeWidgetItem*> it(items);

    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item(it.next());
        quint32 id = item->data(0, PROP_ID).toUInt();
        m_collection->removeFunction(id);
        delete item;
    }
}

void CollectionEditor::slotMoveUp()
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

        quint32 id = item->data(0, PROP_ID).toUInt();
        m_collection->removeFunction(id);
        m_collection->addFunction(id, index - 1);
    }

    // Select the moved items
    it.toFront();
    while (it.hasNext() == true)
        it.next()->setSelected(true);
}

void CollectionEditor::slotMoveDown()
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

        quint32 id = item->data(0, PROP_ID).toUInt();
        m_collection->removeFunction(id);
        m_collection->addFunction(id, index + 1);
    }

    // Select the items
    it.toFront();
    while (it.hasNext() == true)
        it.next()->setSelected(true);
}

void CollectionEditor::slotTestClicked()
{
    if (m_testButton->isChecked() == true)
        m_collection->start(m_doc->masterTimer(), functionParent());
    else
        m_collection->stopAndWait();
}

FunctionParent CollectionEditor::functionParent() const
{
    return FunctionParent::master();
}

void CollectionEditor::updateFunctionList()
{
    m_tree->clear();

    foreach (QVariant fid, m_collection->functions())
    {
        Function* function = m_doc->function(fid.toUInt());
        Q_ASSERT(function != NULL);

        QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
        item->setText(0, function->name());
        item->setData(0, PROP_ID, function->id());
        item->setIcon(0, function->getIcon());
    }
}
