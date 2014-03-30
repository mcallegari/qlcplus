/*
  Q Light Controller
  fixtureselection.cpp

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
#include <QLabel>

#include "qlcfixturedef.h"

#include "fixturetreewidget.h"
#include "fixtureselection.h"
#include "fixturegroup.h"
#include "fixture.h"
#include "doc.h"

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

    m_treeFlags = FixtureTreeWidget::UniverseNumber |
                  FixtureTreeWidget::HeadsNumber |
                  FixtureTreeWidget::Manufacturer |
                  FixtureTreeWidget::Model |
                  FixtureTreeWidget::ShowGroups;

    m_tree = new FixtureTreeWidget(m_doc, m_treeFlags, this);
    m_mainLayout->addWidget(m_tree);

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
    //fillTree();
    m_tree->updateTree();
    return QDialog::exec();
}

/****************************************************************************
 * Selected fixtures
 ****************************************************************************/

QList <quint32> FixtureSelection::selection() const
{
    return m_selectedFixtures;
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
        m_treeFlags = m_treeFlags & (~FixtureTreeWidget::ShowHeads);
    }
    else
    {
        m_tree->setRootIsDecorated(true);
        m_tree->setItemsExpandable(true);
        m_treeFlags = m_treeFlags | FixtureTreeWidget::ShowHeads;
    }
    m_tree->setFlags(m_treeFlags);
}

/****************************************************************************
 * Disabled items
 ****************************************************************************/

void FixtureSelection::setDisabledFixtures(const QList <quint32>& disabled)
{
    m_tree->setDisabledFixtures(disabled);
}

void FixtureSelection::setDisabledHeads(const QList <GroupHead>& disabled)
{
    m_tree->setDisabledHeads(disabled);
}

/****************************************************************************
 * Tree
 ****************************************************************************/
/*
void FixtureSelection::fillTree()
{
    m_tree->clear();

    foreach (Fixture* fixture, m_doc->fixtures())
    {
        Q_ASSERT(fixture != NULL);

        QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
        item->setData(0, PROP_ID, fixture->id());
        item->setText(KColumnName, fixture->name());
        item->setIcon(KColumnName, fixture->getIconFromType(fixture->type()));
        item->setText(KColumnUniverse, QString::number(fixture->universe() + 1));
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
        m_tree->sortItems(KColumnUniverse, Qt::AscendingOrder);
        m_buttonBox->setStandardButtons(QDialogButtonBox::Cancel);
    }
    m_tree->resizeColumnToContents(KColumnName);
    m_tree->resizeColumnToContents(KColumnUniverse);
    m_tree->resizeColumnToContents(KColumnHeads);
    m_tree->resizeColumnToContents(KColumnManufacturer);
    m_tree->resizeColumnToContents(KColumnModel);
}
*/
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
    m_selectedFixtures = m_tree->selectedFixtures();
    m_selectedHeads = m_tree->selectedHeads();

    QDialog::accept();
}
