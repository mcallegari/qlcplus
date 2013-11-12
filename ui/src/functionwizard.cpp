/*
  Q Light Controller
  functionwizard.cpp

  Copyright (C) Heikki Junnila

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

#include <QMessageBox>
#include <QString>
#include <QDebug>
#include <QHash>

#include "palettegenerator.h"
#include "fixtureselection.h"
#include "functionwizard.h"
#include "vcwidget.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"

#include "qlcfixturedef.h"
#include "qlccapability.h"
#include "qlcchannel.h"

#define KFixtureColumnName          0
#define KFixtureColumnCaps          1
#define KFixtureColumnManufacturer  2
#define KFixtureColumnModel         3

#define KFunctionName               0
#define KFunctionOddEven            1

#define KWidgetName                 0

FunctionWizard::FunctionWizard(QWidget* parent, Doc* doc)
    : QDialog(parent)
    , m_doc(doc)
{
    Q_ASSERT(doc != NULL);
    setupUi(this);

    QString trbgSS = "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(0, 0, 0, 0), stop:1 rgba(255, 255, 255, 0));";
    m_wizardLogo->setStyleSheet(trbgSS);
    m_introText->setStyleSheet(trbgSS);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    m_fixtureTree->sortItems(KFixtureColumnName, Qt::AscendingOrder);

    connect(m_nextButton, SIGNAL(clicked()),
            this, SLOT(slotNextPageClicked()));

    connect(m_tabWidget, SIGNAL(currentChanged(int)),
            this, SLOT(slotTabClicked()));

    checkTabsAndButtons();
}

FunctionWizard::~FunctionWizard()
{
}

void FunctionWizard::slotNextPageClicked()
{
    int newIdx = m_tabWidget->currentIndex() + 1;
    if (newIdx == 4)
        return;

    m_tabWidget->setCurrentIndex(newIdx);
    checkTabsAndButtons();
}

void FunctionWizard::slotTabClicked()
{
    checkTabsAndButtons();
}

void FunctionWizard::accept()
{

    foreach (PaletteGenerator *palette, m_paletteList)
        palette->addToDoc();

    addWidgetsToVirtualConsole();

    QDialog::accept();
}

void FunctionWizard::checkTabsAndButtons()
{
    switch(m_tabWidget->currentIndex())
    {
        case 0:
        {
            m_nextButton->setEnabled(true);
            m_tabWidget->setTabEnabled(2, false);
            m_tabWidget->setTabEnabled(3, false);
        }
        break;
        case 1:
        {
            if (m_allFuncsTree->topLevelItemCount() == 0)
            {
                m_nextButton->setEnabled(false);
                m_tabWidget->setTabEnabled(2, false);
            }
            else
            {
                m_nextButton->setEnabled(true);
                m_tabWidget->setTabEnabled(2, true);
            }
        }
        break;
        case 2:
        {
            if (m_paletteList.isEmpty() == false)
            {
                m_tabWidget->setTabEnabled(3, true);
                m_nextButton->setEnabled(true);
            }
            else
            {
                m_tabWidget->setTabEnabled(3, false);
                m_nextButton->setEnabled(false);
            }
        }
        break;
        case 3:
            m_nextButton->setEnabled(false);
        break;
    }
}

/****************************************************************************
 * Fixtures
 ****************************************************************************/

QTreeWidgetItem *FunctionWizard::getFixtureGroupItem(QString manufacturer, QString model)
{
    for (int i = 0; i < m_fixtureTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = m_fixtureTree->topLevelItem(i);
        if (item->text(KFixtureColumnManufacturer) == manufacturer &&
            item->text(KFixtureColumnModel) == model)
                return item;
    }
    // if we're here then the group doesn't exist. Create it
    QTreeWidgetItem* newGrp = new QTreeWidgetItem(m_fixtureTree);
    newGrp->setText(KFixtureColumnName, tr("%1 group").arg(model));
    newGrp->setIcon(KFixtureColumnName, QIcon(":/group.png"));
    newGrp->setText(KFixtureColumnManufacturer, manufacturer);
    newGrp->setText(KFixtureColumnModel, model);
    newGrp->setExpanded(true);
    return newGrp;
}

void FunctionWizard::addFixture(quint32 fxi_id)
{
    Fixture* fxi = m_doc->fixture(fxi_id);
    Q_ASSERT(fxi != NULL);

    QStringList caps = PaletteGenerator::getCapabilities(fxi);

    if (caps.join(", ").isEmpty())
    {
        QMessageBox::warning(this, tr("Error"), tr("%1 has no capability supported by this wizard.").arg(fxi->name()));
        return;
    }
    else
    {
        QTreeWidgetItem *groupItem = getFixtureGroupItem(fxi->fixtureDef()->manufacturer(), fxi->fixtureDef()->model());
        Q_ASSERT(groupItem != NULL);

        QTreeWidgetItem* item = new QTreeWidgetItem(groupItem);
        item->setText(KFixtureColumnName, fxi->name());
        item->setIcon(KFixtureColumnName, fxi->getIconFromType(fxi->type()));
        item->setData(KFixtureColumnName, Qt::UserRole, fxi_id);
        item->setText(KFixtureColumnCaps, caps.join(", "));
    }
    m_fixtureTree->resizeColumnToContents(KFixtureColumnName);
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

        if (m_fixtureTree->topLevelItemCount() > 0)
            updateAvailableFunctionsTree();
    }
    checkTabsAndButtons();
}

void FunctionWizard::slotRemoveClicked()
{
    QListIterator <QTreeWidgetItem*> it(m_fixtureTree->selectedItems());
    while (it.hasNext() == true)
        delete it.next();

    checkTabsAndButtons();
}

QList <quint32> FunctionWizard::fixtureIds() const
{
    QList <quint32> list;
    for (int i = 0; i < m_fixtureTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item(m_fixtureTree->topLevelItem(i));
        Q_ASSERT(item != NULL);

        for (int j = 0; j < item->childCount(); j++)
        {
            QTreeWidgetItem *child = item->child(j);
            Q_ASSERT(child != NULL);

            list << child->data(KFixtureColumnName, Qt::UserRole).toInt();
        }
    }

    return list;
}

/********************************************************************
 * Functions
 ********************************************************************/

void FunctionWizard::addFunctionsGroup(QTreeWidgetItem *fxGrpItem, QTreeWidgetItem *grpItem,
                                       QString name, PaletteGenerator::PaletteType type)
{
    if (grpItem == NULL)
        return;

    QTreeWidgetItem *item = new QTreeWidgetItem(grpItem);
    item->setText(KFunctionName, name);
    item->setCheckState(KFunctionName, Qt::Unchecked);
    item->setData(KFunctionName, Qt::UserRole, type);

    if (fxGrpItem != NULL && fxGrpItem->childCount() > 1)
    {
        item->setCheckState(KFunctionOddEven, Qt::Unchecked);
    }
}

void FunctionWizard::updateAvailableFunctionsTree()
{
    disconnect(m_allFuncsTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
            this, SLOT(slotFunctionItemChanged(QTreeWidgetItem*,int)));

    m_allFuncsTree->clear();
    m_resFuncsTree->clear();

    for (int i = 0; i < m_fixtureTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *fxGrpItem = m_fixtureTree->topLevelItem(i);
        Q_ASSERT(fxGrpItem != NULL);

        if (fxGrpItem->childCount() == 0)
            continue;

        QTreeWidgetItem *grpItem = new QTreeWidgetItem(m_allFuncsTree);
        grpItem->setText(KFunctionName, fxGrpItem->text(KFixtureColumnName));
        grpItem->setIcon(KFunctionName, fxGrpItem->icon(KFixtureColumnName));
        grpItem->setExpanded(true);

        // since groups contain fixture of the same type, get the first
        // child and create categories on its capabilities
        QTreeWidgetItem *fxItem = fxGrpItem->child(0);
        quint32 fxID = fxItem->data(KFixtureColumnName, Qt::UserRole).toUInt();
        Fixture* fxi = m_doc->fixture(fxID);
        Q_ASSERT(fxi != NULL);

        QStringList caps = PaletteGenerator::getCapabilities(fxi);

        foreach(QString cap, caps)
        {
            if (cap == KQLCChannelRGB || cap == KQLCChannelCMY)
            {
                addFunctionsGroup(fxGrpItem, grpItem, tr("Primary Colors"), PaletteGenerator::PrimaryColors);
                addFunctionsGroup(fxGrpItem, grpItem, tr("16 Colors"), PaletteGenerator::SixteenColors);
            }
            else if (cap == QLCChannel::groupToString(QLCChannel::Gobo))
                addFunctionsGroup(fxGrpItem, grpItem, tr("Gobos"), PaletteGenerator::Gobos);
            else if (cap == QLCChannel::groupToString(QLCChannel::Shutter))
                addFunctionsGroup(fxGrpItem, grpItem, tr("Shutters"), PaletteGenerator::Shutter);
        }
    }

    m_allFuncsTree->resizeColumnToContents(KFunctionName);

    connect(m_allFuncsTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
            this, SLOT(slotFunctionItemChanged(QTreeWidgetItem*,int)));
}

QTreeWidgetItem *FunctionWizard::getFunctionGroupItem(Function::Type type)
{
    for (int i = 0; i < m_resFuncsTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = m_resFuncsTree->topLevelItem(i);
        int grpType = item->data(KFunctionName, Qt::UserRole).toInt();
        if (grpType == type)
            return item;
    }
    // if we're here then the group doesn't exist. Create it
    QTreeWidgetItem* newGrp = new QTreeWidgetItem(m_resFuncsTree);
    newGrp->setText(KFixtureColumnName, Function::typeToString(type));
    newGrp->setIcon(KFixtureColumnName, Function::typeToIcon(type));
    newGrp->setData(KFunctionName, Qt::UserRole, type);
    newGrp->setExpanded(true);
    return newGrp;
}

void FunctionWizard::updateResultFunctionsTree()
{
    m_resFuncsTree->clear();
    m_paletteList.clear();

    for (int i = 0; i < m_allFuncsTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *funcGrpItem = m_allFuncsTree->topLevelItem(i);
        Q_ASSERT(funcGrpItem != NULL);

        if (funcGrpItem->childCount() == 0)
            continue;

        // retrieve the list of fixtures involved in this group
        QList <Fixture *> fxList;
        QTreeWidgetItem *fxiGrpItem = m_fixtureTree->topLevelItem(i);

        for (int f = 0; f < fxiGrpItem->childCount(); f++)
        {
            QTreeWidgetItem *fItem = fxiGrpItem->child(f);
            quint32 fxID = fItem->data(KFixtureColumnName, Qt::UserRole).toUInt();
            Fixture *fixture = m_doc->fixture(fxID);
            if (fixture != NULL)
                fxList.append(fixture);
        }

        // iterate through the function group children to see which are checked
        for (int c = 0; c < funcGrpItem->childCount(); c++)
        {
            QTreeWidgetItem *funcItem = funcGrpItem->child(c);
            if (funcItem->checkState(KFunctionName) == Qt::Checked)
            {
                int type = funcItem->data(KFunctionName, Qt::UserRole).toInt();
                int subType = PaletteGenerator::All;
                if (funcItem->checkState(KFunctionOddEven) == Qt::Checked)
                    subType = PaletteGenerator::OddEven;
                PaletteGenerator *palette = new PaletteGenerator(m_doc, fxList,
                                                                 (PaletteGenerator::PaletteType)type,
                                                                 (PaletteGenerator::PaletteSubType)subType);
                m_paletteList.append(palette);

                foreach(Scene *scene, palette->scenes())
                {
                    QTreeWidgetItem *item = new QTreeWidgetItem(getFunctionGroupItem(Function::Scene));
                    item->setText(KFunctionName, scene->name());
                    item->setIcon(KFunctionName, Function::typeToIcon(Function::Scene));
                }
                foreach(Chaser *chaser, palette->chasers())
                {
                    QTreeWidgetItem *item = new QTreeWidgetItem(getFunctionGroupItem(Function::Chaser));
                    item->setText(KFunctionName, chaser->name());
                    item->setIcon(KFunctionName, Function::typeToIcon(Function::Chaser));
                }
            }
        }
    }
}

void FunctionWizard::slotFunctionItemChanged(QTreeWidgetItem *item, int col)
{
    Q_UNUSED(col)
    Q_ASSERT(item != NULL);

    updateResultFunctionsTree();

    if (m_paletteList.isEmpty() == false)
        updateWidgetsTree();

    checkTabsAndButtons();
}

/********************************************************************
 * Widgets
 ********************************************************************/

void FunctionWizard::updateWidgetsTree()
{
    m_widgetsTree->clear();

    foreach(PaletteGenerator *palette, m_paletteList)
    {
        QTreeWidgetItem *frame = new QTreeWidgetItem(m_widgetsTree);
        frame->setText(KWidgetName, palette->fullName());
        frame->setIcon(KWidgetName, VCWidget::typeToIcon(VCWidget::FrameWidget));
        frame->setData(KWidgetName, Qt::UserRole, VCWidget::FrameWidget);
        frame->setFlags(frame->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsTristate);
        frame->setCheckState(KWidgetName, Qt::Unchecked);

        foreach(Scene *scene, palette->scenes())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(frame);
            QString toRemove = " - " + palette->model();
            item->setText(KWidgetName, scene->name().remove(toRemove));
            item->setIcon(KWidgetName, VCWidget::typeToIcon(VCWidget::ButtonWidget));
            item->setData(KWidgetName, Qt::UserRole, VCWidget::ButtonWidget);
            item->setCheckState(KWidgetName, Qt::Unchecked);
        }
        foreach(Chaser *chaser, palette->chasers())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(frame);
            QString toRemove = " - " + palette->model();
            item->setText(KWidgetName, chaser->name().remove(toRemove));
            item->setIcon(KWidgetName, VCWidget::typeToIcon(VCWidget::CueListWidget));
            item->setData(KWidgetName, Qt::UserRole, VCWidget::CueListWidget);
            item->setCheckState(KWidgetName, Qt::Unchecked);
        }
    }
}

void FunctionWizard::addWidgetsToVirtualConsole()
{

}
