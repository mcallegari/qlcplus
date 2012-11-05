/*
  Q Light Controller
  fixtureselection.cpp

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
#include <QLabel>

#include "qlcfixturedef.h"

#include "fixtureselection.h"
#include "fixturegroup.h"
#include "fixture.h"
#include "doc.h"

#define KColumnName         0
#define KColumnHeads        1
#define KColumnManufacturer 2
#define KColumnModel        3

#define PROP_ID             Qt::UserRole
#define PROP_HEAD           Qt::UserRole + 1

FixtureSelection::FixtureSelection(QWidget* parent, Doc* doc)
    : QDialog(parent)
    , m_doc(doc)
    , m_selectionMode(Fixtures)
{
    Q_ASSERT(doc != NULL);

    setupUi(this);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotItemDoubleClicked()));

    connect(m_tree, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotSelectionChanged()));
}

FixtureSelection::~FixtureSelection()
{
}

int FixtureSelection::exec()
{
    fillTree();
    return QDialog::exec();
}

/****************************************************************************
 * Selected fixtures
 ****************************************************************************/

QList <quint32> FixtureSelection::selection() const
{
    return m_selection;
}

QList <GroupHead> FixtureSelection::selectedHeads() const
{
    return m_selectedHeads;
}

/****************************************************************************
 * Multi-selection
 ****************************************************************************/

void FixtureSelection::setMultiSelection(bool multi)
{
    if (multi == true)
        m_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    else
        m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
}

/****************************************************************************
 * Selection mode
 ****************************************************************************/

void FixtureSelection::setSelectionMode(SelectionMode mode)
{
    m_selectionMode = mode;
    if (mode == Fixtures)
    {
        m_tree->setRootIsDecorated(false);
        m_tree->setItemsExpandable(false);
    }
    else
    {
        m_tree->setRootIsDecorated(true);
        m_tree->setItemsExpandable(true);
    }
}

/****************************************************************************
 * Disabled fixtures
 ****************************************************************************/

void FixtureSelection::setDisabledFixtures(const QList <quint32>& disabled)
{
    m_disabledHeads.clear();
    m_disabledFixtures = disabled;
}

void FixtureSelection::setDisabledHeads(const QList <GroupHead>& disabled)
{
    m_disabledFixtures.clear();
    m_disabledHeads = disabled;
}

/****************************************************************************
 * Tree
 ****************************************************************************/

void FixtureSelection::fillTree()
{
    m_tree->clear();

    foreach (Fixture* fixture, m_doc->fixtures())
    {
        Q_ASSERT(fixture != NULL);

        QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
        item->setData(0, PROP_ID, fixture->id());
        item->setText(KColumnName, fixture->name());
        item->setText(KColumnHeads, QString::number(fixture->heads()));

        if (fixture->fixtureDef() == NULL)
        {
            item->setText(KColumnManufacturer, tr("Generic"));
            item->setText(KColumnModel, tr("Generic"));
        }
        else
        {
            item->setText(KColumnManufacturer, fixture->fixtureDef()->manufacturer());
            item->setText(KColumnModel, fixture->fixtureDef()->model());
        }

        if (m_disabledFixtures.contains(fixture->id()) == true)
        {
            // Disable selection
            item->setFlags(0);
        }
        else if (m_selectionMode == Heads)
        {
            int disabled = 0;

            for (int i = 0; i < fixture->heads(); i++)
            {
                QTreeWidgetItem* headItem = new QTreeWidgetItem(item);
                headItem->setText(KColumnName, QString("%1 %2").arg(tr("Head")).arg(i + 1));
                headItem->setData(0, PROP_HEAD, i);
                if (m_disabledHeads.contains(GroupHead(fixture->id(), i)) == true)
                {
                    headItem->setFlags(0); // Disable selection
                    disabled++;
                }
            }

            // Disable the whole fixture if all heads are disabled
            if (disabled == fixture->heads())
                item->setFlags(0);
        }
    }

    if (m_tree->topLevelItemCount() == 0)
    {
        m_tree->setHeaderLabel(tr("No fixtures available"));
        m_tree->header()->hideSection(KColumnManufacturer);
        m_tree->header()->hideSection(KColumnModel);
        QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
        item->setText(0, tr("Go to the Fixture Manager and add some fixtures first."));
        m_tree->setEnabled(false);
        m_buttonBox->setStandardButtons(QDialogButtonBox::Cancel);
    }
    else
    {
        m_tree->sortItems(KColumnName, Qt::AscendingOrder);
        m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);
        m_buttonBox->setStandardButtons(QDialogButtonBox::Cancel);
    }
}

void FixtureSelection::slotItemDoubleClicked()
{
    if (m_tree->selectedItems().isEmpty() == false)
        accept();
}

void FixtureSelection::slotSelectionChanged()
{
    if (m_tree->selectedItems().size() > 0)
        m_buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    else
        m_buttonBox->setStandardButtons(QDialogButtonBox::Cancel);
}

void FixtureSelection::accept()
{
    m_selection.clear();
    m_selectedHeads.clear();

    if (m_selectionMode == Fixtures)
    {
        QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
        while (it.hasNext() == true)
            m_selection << it.next()->data(0, PROP_ID).toUInt();
    }
    else
    {
        QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
        while (it.hasNext() == true)
        {
            const QTreeWidgetItem* item(it.next());
            if (item->parent() == NULL)
            {
                // Fixture item, select all non-disabled heads under it, unless already selected
                quint32 fxi = item->data(0, PROP_ID).toUInt();
                for (int i = 0; i < item->childCount(); i++)
                {
                    QTreeWidgetItem* child = item->child(i);
                    Q_ASSERT(child != NULL);
                    if (child->isDisabled() == false)
                    {
                        int head = child->data(0, PROP_HEAD).toInt();
                        GroupHead gh(fxi, head);
                        if (m_selectedHeads.contains(gh) == false)
                            m_selectedHeads << gh;
                    }
                }
            }
            else
            {
                // Head item, select unless already selected
                Q_ASSERT(item->parent() != NULL);
                quint32 fxi = item->parent()->data(0, PROP_ID).toUInt();
                int head = item->data(0, PROP_HEAD).toInt();
                GroupHead gh(fxi, head);
                if (m_selectedHeads.contains(gh) == false)
                    m_selectedHeads << gh;
            }
        }
    }

    QDialog::accept();
}
