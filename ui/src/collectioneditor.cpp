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

#include "qlcfixturedef.h"

#include "functionselection.h"
#include "collectioneditor.h"
#include "mastertimer.h"
#include "collection.h"
#include "function.h"
#include "fixture.h"
#include "apputil.h"
#include "doc.h"

#define PROP_ID Qt::UserRole

CollectionEditor::CollectionEditor(QWidget* parent, Collection* fc, Doc* doc)
    : QWidget(parent)
    , m_doc(doc)
    , m_fc(fc)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(fc != NULL);

    setupUi(this);

    connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotNameEdited(const QString&)));
    connect(m_add, SIGNAL(clicked()), this, SLOT(slotAdd()));
    connect(m_remove, SIGNAL(clicked()), this, SLOT(slotRemove()));

    m_nameEdit->setText(m_fc->name());
    m_nameEdit->setSelection(0, m_nameEdit->text().length());

    updateFunctionList();

    // Set focus to the editor
    m_nameEdit->setFocus();
}

CollectionEditor::~CollectionEditor()
{
}

void CollectionEditor::slotNameEdited(const QString& text)
{
    m_fc->setName(text);
}

void CollectionEditor::slotAdd()
{
    FunctionSelection fs(this, m_doc);
    fs.setDisabledFunctions(QList <quint32>() << m_fc->id());

    if (fs.exec() == QDialog::Accepted)
    {
        QListIterator <quint32> it(fs.selection());
        while (it.hasNext() == true)
            m_fc->addFunction(it.next());
        updateFunctionList();
    }
}

void CollectionEditor::slotRemove()
{
    QTreeWidgetItem* item = m_tree->currentItem();
    if (item != NULL)
    {
        quint32 id = item->data(0, PROP_ID).toUInt();
        m_fc->removeFunction(id);
        delete item;
    }
}

void CollectionEditor::updateFunctionList()
{
    m_tree->clear();

    QListIterator <quint32> it(m_fc->functions());
    while (it.hasNext() == true)
    {
        Function* function = m_doc->function(it.next());
        Q_ASSERT(function != NULL);

        QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
        item->setText(0, function->name());
        item->setData(0, PROP_ID, function->id());
        item->setIcon(0, Function::typeToIcon(function->type()));
    }
}
