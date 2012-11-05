/*
  Q Light Controller
  functionwizard.cpp

  Copyright (C) Heikki Junnila

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

#include <QString>
#include <QDebug>
#include <QHash>

#include <cstdlib>

#include "intensitygenerator.h"
#include "palettegenerator.h"
#include "fixtureselection.h"
#include "functionwizard.h"
#include "chaserstep.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"

#include "qlccapability.h"
#include "qlcchannel.h"

#define KColumnName 0
#define KColumnCaps 1

FunctionWizard::FunctionWizard(QWidget* parent, Doc* doc)
    : QDialog(parent)
    , m_doc(doc)
{
    Q_ASSERT(doc != NULL);
    setupUi(this);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    m_fixtureTree->sortItems(KColumnName, Qt::AscendingOrder);
}

FunctionWizard::~FunctionWizard()
{
}

void FunctionWizard::slotAddClicked()
{
    FixtureSelection fs(this, m_doc);
    fs.setMultiSelection(true);
    fs.setDisabledFixtures(fixtureIds());
    if (fs.exec() == QDialog::Accepted)
    {
        QListIterator <quint32> it(fs.selection());
        while (it.hasNext() == true)
            addFixture(it.next());
    }
}

void FunctionWizard::slotRemoveClicked()
{
    QListIterator <QTreeWidgetItem*> it(m_fixtureTree->selectedItems());
    while (it.hasNext() == true)
        delete it.next();
}

void FunctionWizard::accept()
{
    PaletteGenerator pal(m_doc, fixtures());

    if (m_coloursCheck->isChecked() == true)
        pal.createColours();
    if (m_goboCheck->isChecked() == true)
        pal.createGobos();
    if (m_shutterCheck->isChecked() == true)
        pal.createShutters();

    QDialog::accept();
}

/****************************************************************************
 * Fixtures
 ****************************************************************************/

void FunctionWizard::addFixture(quint32 fxi_id)
{
    Fixture* fxi = m_doc->fixture(fxi_id);
    Q_ASSERT(fxi != NULL);

    QTreeWidgetItem* item = new QTreeWidgetItem(m_fixtureTree);
    item->setText(KColumnName, fxi->name());
    item->setData(KColumnName, Qt::UserRole, fxi_id);

    QStringList caps;
    if (!PaletteGenerator::findChannels(fxi, QLCChannel::Colour).isEmpty())
        caps << QLCChannel::groupToString(QLCChannel::Colour);

    if (!PaletteGenerator::findChannels(fxi, QLCChannel::Gobo).isEmpty())
        caps << QLCChannel::groupToString(QLCChannel::Gobo);

    if (!PaletteGenerator::findChannels(fxi, QLCChannel::Shutter).isEmpty())
        caps << QLCChannel::groupToString(QLCChannel::Shutter);

    item->setText(KColumnCaps, caps.join(", "));
}

QList <Fixture*> FunctionWizard::fixtures() const
{
    QList <Fixture*> list;
    for (int i = 0; i < m_fixtureTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item(m_fixtureTree->topLevelItem(i));
        Q_ASSERT(item != NULL);

        quint32 id = item->data(KColumnName, Qt::UserRole).toInt();
        Fixture* fxi = m_doc->fixture(id);
        Q_ASSERT(fxi != NULL);

        list << fxi;
    }

    return list;
}

QList <quint32> FunctionWizard::fixtureIds() const
{
    QList <quint32> list;
    for (int i = 0; i < m_fixtureTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item(m_fixtureTree->topLevelItem(i));
        Q_ASSERT(item != NULL);

        list << item->data(KColumnName, Qt::UserRole).toInt();
    }

    return list;
}
