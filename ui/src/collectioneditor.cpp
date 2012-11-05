/*
  Q Light Controller
  collectioneditor.cpp

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
#include <QSettings>
#include <QLineEdit>
#include <QLabel>

#include "qlcfixturedef.h"

#include "functionselection.h"
#include "collectioneditor.h"
#include "mastertimer.h"
#include "collection.h"
#include "outputmap.h"
#include "inputmap.h"
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
    }
}
