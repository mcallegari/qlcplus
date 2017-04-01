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
#include "virtualconsole.h"
#include "vcsoloframe.h"
#include "vccuelist.h"
#include "rgbmatrix.h"
#include "vcwidget.h"
#include "vcbutton.h"
#include "vcslider.h"
#include "vcframe.h"
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
    m_paletteList.clear();
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

    m_doc->setModified();

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
        item->setIcon(KFixtureColumnName, fxi->getIconFromType());
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
                addFunctionsGroup(fxGrpItem, grpItem,
                                  PaletteGenerator::typetoString(PaletteGenerator::PrimaryColors),
                                  PaletteGenerator::PrimaryColors);
                addFunctionsGroup(fxGrpItem, grpItem,
                                  PaletteGenerator::typetoString(PaletteGenerator::SixteenColors),
                                  PaletteGenerator::SixteenColors);
                addFunctionsGroup(fxGrpItem, grpItem,
                                  PaletteGenerator::typetoString(PaletteGenerator::Animation),
                                  PaletteGenerator::Animation);
            }
            else if (cap == QLCChannel::groupToString(QLCChannel::Gobo))
                addFunctionsGroup(fxGrpItem, grpItem,
                                  PaletteGenerator::typetoString(PaletteGenerator::Gobos),
                                  PaletteGenerator::Gobos);
            else if (cap == QLCChannel::groupToString(QLCChannel::Shutter))
                addFunctionsGroup(fxGrpItem, grpItem,
                                  PaletteGenerator::typetoString(PaletteGenerator::Shutter),
                                  PaletteGenerator::Shutter);
            else if (cap == QLCChannel::groupToString(QLCChannel::Colour))
                addFunctionsGroup(fxGrpItem, grpItem,
                                  PaletteGenerator::typetoString(PaletteGenerator::ColourMacro),
                                  PaletteGenerator::ColourMacro);
        }
    }

    m_allFuncsTree->resizeColumnToContents(KFunctionName);

    connect(m_allFuncsTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
            this, SLOT(slotFunctionItemChanged(QTreeWidgetItem*,int)));
}

QTreeWidgetItem *FunctionWizard::getFunctionGroupItem(const Function *func)
{
    for (int i = 0; i < m_resFuncsTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = m_resFuncsTree->topLevelItem(i);
        int grpType = item->data(KFunctionName, Qt::UserRole).toInt();
        if (grpType == func->type())
            return item;
    }
    // if we're here then the group doesn't exist. Create it
    QTreeWidgetItem* newGrp = new QTreeWidgetItem(m_resFuncsTree);
    newGrp->setText(KFixtureColumnName, Function::typeToString(func->type()));
    newGrp->setIcon(KFixtureColumnName, func->getIcon());
    newGrp->setData(KFunctionName, Qt::UserRole, func->type());
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
                    QTreeWidgetItem *item = new QTreeWidgetItem(getFunctionGroupItem(scene));
                    item->setText(KFunctionName, scene->name());
                    item->setIcon(KFunctionName, scene->getIcon());
                }
                foreach(Chaser *chaser, palette->chasers())
                {
                    QTreeWidgetItem *item = new QTreeWidgetItem(getFunctionGroupItem(chaser));
                    item->setText(KFunctionName, chaser->name());
                    item->setIcon(KFunctionName, chaser->getIcon());
                }
                foreach(RGBMatrix *matrix, palette->matrices())
                {
                    QTreeWidgetItem *item = new QTreeWidgetItem(getFunctionGroupItem(matrix));
                    item->setText(KFunctionName, matrix->name());
                    item->setIcon(KFunctionName, matrix->getIcon());
                }
            }
        }
    }
}

void FunctionWizard::slotFunctionItemChanged(QTreeWidgetItem *item, int col)
{
    Q_UNUSED(col)
    Q_UNUSED(item)

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
        if (palette->type() == PaletteGenerator::Animation)
        {
            frame->setIcon(KWidgetName, VCWidget::typeToIcon(VCWidget::SoloFrameWidget));
            frame->setData(KWidgetName, Qt::UserRole, VCWidget::SoloFrameWidget);
        }
        else
        {
            frame->setIcon(KWidgetName, VCWidget::typeToIcon(VCWidget::FrameWidget));
            frame->setData(KWidgetName, Qt::UserRole, VCWidget::FrameWidget);
        }
        frame->setData(KWidgetName, Qt::UserRole + 1, qVariantFromValue((void *)palette));
        frame->setFlags(frame->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsTristate);
        frame->setCheckState(KWidgetName, Qt::Unchecked);

        QTreeWidgetItem *soloFrameItem = NULL;
        if (palette->scenes().count() > 0)
        {
            soloFrameItem = new QTreeWidgetItem(frame);
            soloFrameItem->setText(KWidgetName, tr("Presets solo frame"));
            soloFrameItem->setIcon(KWidgetName, VCWidget::typeToIcon(VCWidget::SoloFrameWidget));
            soloFrameItem->setFlags(soloFrameItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsTristate);
            soloFrameItem->setCheckState(KWidgetName, Qt::Unchecked);
            soloFrameItem->setData(KWidgetName, Qt::UserRole, VCWidget::SoloFrameWidget);
        }
        foreach(Scene *scene, palette->scenes())
        {
            QTreeWidgetItem *item = NULL;
            if (soloFrameItem != NULL)
                item = new QTreeWidgetItem(soloFrameItem);
            else
                item = new QTreeWidgetItem(frame);
            QString toRemove = " - " + palette->model();
            item->setText(KWidgetName, scene->name().remove(toRemove));
            item->setIcon(KWidgetName, VCWidget::typeToIcon(VCWidget::ButtonWidget));
            item->setCheckState(KWidgetName, Qt::Unchecked);
            item->setData(KWidgetName, Qt::UserRole, VCWidget::ButtonWidget);
            item->setData(KWidgetName, Qt::UserRole + 1, qVariantFromValue((void *)scene));

        }
        foreach(Chaser *chaser, palette->chasers())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(frame);
            QString toRemove = " - " + palette->model();
            item->setText(KWidgetName, chaser->name().remove(toRemove));
            item->setIcon(KWidgetName, VCWidget::typeToIcon(VCWidget::CueListWidget));
            item->setCheckState(KWidgetName, Qt::Unchecked);
            item->setData(KWidgetName, Qt::UserRole, VCWidget::CueListWidget);
            item->setData(KWidgetName, Qt::UserRole + 1, qVariantFromValue((void *)chaser));
        }

        foreach(RGBMatrix *matrix, palette->matrices())
        {
            QTreeWidgetItem *item = NULL;
            if (soloFrameItem != NULL)
                item = new QTreeWidgetItem(soloFrameItem);
            else
                item = new QTreeWidgetItem(frame);
            QString toRemove = " - " + palette->model();
            item->setText(KWidgetName, matrix->name().remove(toRemove));
            item->setIcon(KWidgetName, VCWidget::typeToIcon(VCWidget::ButtonWidget));
            item->setCheckState(KWidgetName, Qt::Unchecked);
            item->setData(KWidgetName, Qt::UserRole, VCWidget::ButtonWidget);
            item->setData(KWidgetName, Qt::UserRole + 1, qVariantFromValue((void *)matrix));
        }

        if (palette->scenes().count() > 0)
        {
            int pType = palette->type();
            QTreeWidgetItem *item = new QTreeWidgetItem(frame);
            if (pType == PaletteGenerator::PrimaryColors ||
                pType == PaletteGenerator::SixteenColors)
                    item->setText(KWidgetName, tr("Click & Go RGB"));
            else if (pType == PaletteGenerator::Gobos ||
                     pType == PaletteGenerator::Shutter ||
                     pType == PaletteGenerator::ColourMacro)
                        item->setText(KWidgetName, tr("Click & Go Macro"));

            item->setIcon(KWidgetName, VCWidget::typeToIcon(VCWidget::SliderWidget));
            item->setCheckState(KWidgetName, Qt::Unchecked);
            item->setData(KWidgetName, Qt::UserRole, VCWidget::SliderWidget);
            Scene *firstScene = palette->scenes().at(0);
            item->setData(KWidgetName, Qt::UserRole + 1, qVariantFromValue((void *)firstScene));
        }
    }
}

VCWidget *FunctionWizard::createWidget(int type, VCWidget *parent, int xpos, int ypos,
                                       Function *func, int pType)
{
    VirtualConsole *vc = VirtualConsole::instance();
    VCWidget *widget = NULL;

    if (parent == NULL)
        return NULL;

    switch(type)
    {
        case VCWidget::FrameWidget:
        {
            VCFrame* frame = new VCFrame(parent, m_doc, true);
            vc->setupWidget(frame, parent);
            frame->move(QPoint(xpos, ypos));
            widget = frame;
        }
        break;
        case VCWidget::SoloFrameWidget:
        {
            VCSoloFrame* frame = new VCSoloFrame(parent, m_doc, true);
            vc->setupWidget(frame, parent);
            frame->move(QPoint(xpos, ypos));
            widget = frame;
        }
        break;
        case VCWidget::ButtonWidget:
        {
            VCButton* button = new VCButton(parent, m_doc);
            vc->setupWidget(button, parent);
            button->move(QPoint(xpos, ypos));
            if (func != NULL)
                button->setFunction(func->id());

            widget = button;
        }
        break;
        case VCWidget::CueListWidget:
        {
            VCCueList* cuelist = new VCCueList(parent, m_doc);
            vc->setupWidget(cuelist, parent);
            cuelist->move(QPoint(xpos, ypos));
            if (func != NULL)
                cuelist->setChaser(func->id());
            widget = cuelist;
        }
        break;
        case VCWidget::SliderWidget:
        {
            VCSlider* slider = new VCSlider(parent, m_doc);
            vc->setupWidget(slider, parent);
            slider->move(QPoint(xpos, ypos));
            if (func != NULL)
            {
                Scene *scene = qobject_cast<Scene*> (func);
                foreach (SceneValue scv, scene->values())
                    slider->addLevelChannel(scv.fxi, scv.channel);

                if (pType == PaletteGenerator::PrimaryColors ||
                    pType == PaletteGenerator::SixteenColors)
                        slider->setClickAndGoType(ClickAndGoWidget::RGB);
                else
                    slider->setClickAndGoType(ClickAndGoWidget::Preset);
                slider->setSliderMode(VCSlider::Level);
            }
            widget = slider;
        }
        break;
        default:
        break;
    }

    if (widget != NULL && func != NULL)
    {
        if (func->type() == Function::Scene && type == VCWidget::ButtonWidget)
        {
            Scene *scene = qobject_cast<Scene*> (func);

            if (pType == PaletteGenerator::PrimaryColors ||
                pType == PaletteGenerator::SixteenColors ||
                pType == PaletteGenerator::ColourMacro)
            {
                QColor col = scene->colorValue();
                if (col.isValid())
                    widget->setBackgroundColor(col);
            }
            else if (pType == PaletteGenerator::Gobos)
            {
                foreach(SceneValue scv, scene->values())
                {
                    Fixture *fixture = m_doc->fixture(scv.fxi);
                    if (fixture == NULL)
                        continue;

                    const QLCChannel* channel(fixture->channel(scv.channel));
                    if (channel->group() == QLCChannel::Gobo)
                    {
                        QLCCapability *cap = channel->searchCapability(scv.value);
                        if (cap->resourceName().isEmpty() == false)
                        {
                            widget->setBackgroundImage(cap->resourceName());
                            break;
                        }
                    }
                }
            }
        }
    }

    return widget;
}

QSize FunctionWizard::recursiveCreateWidget(QTreeWidgetItem *item, VCWidget *parent, int type)
{
    QSize groupSize(100, 50);
    int subX = 10, subY = 40;

    for (int c = 0; c < item->childCount(); c++)
    {
        QTreeWidgetItem *childItem = item->child(c);

        if (childItem->checkState(KWidgetName) == Qt::Checked ||
            childItem->checkState(KWidgetName) == Qt::PartiallyChecked)
        {
            int cType = childItem->data(KWidgetName, Qt::UserRole).toInt();
            Function *func = (Function *) childItem->data(KWidgetName, Qt::UserRole + 1).value<void *>();

            VCWidget *childWidget = createWidget(cType, parent, subX, subY, func, type);
            if (childWidget != NULL)
            {
                childWidget->setCaption(childItem->text(KWidgetName));

                if (childItem->childCount() > 0)
                {
                    childWidget->resize(QSize(1000, 1000));

                    QSize size = recursiveCreateWidget(childItem, childWidget, type);

                    childWidget->resize(size);

                }

                if (subX + childWidget->width() > groupSize.width())
                    groupSize.setWidth(subX + childWidget->width() + 10);
                if (subY + childWidget->height() > groupSize.height())
                    groupSize.setHeight(subY + childWidget->height() + 10);


                if (c > 0 && (c + 1)%4 == 0)
                {
                    subX = 10;
                    subY += childWidget->height() + 10;
                }
                else
                    subX += childWidget->width() + 10;
            }
        }
    }

    return groupSize;
}



void FunctionWizard::addWidgetsToVirtualConsole()
{
    int xPos = 10;
    int yPos = 10;

    VirtualConsole *vc = VirtualConsole::instance();
    VCFrame *mainFrame = vc->contents();
    Q_ASSERT(mainFrame != NULL);

    for (int i = 0; i < m_widgetsTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *wItem = m_widgetsTree->topLevelItem(i);

        if (wItem->checkState(KWidgetName) == Qt::Checked ||
            wItem->checkState(KWidgetName) == Qt::PartiallyChecked)
        {
            int wType = wItem->data(KWidgetName, Qt::UserRole).toInt();
            VCWidget *widget = createWidget(wType, mainFrame, xPos, yPos);
            if (widget == NULL)
                continue;

            widget->resize(QSize(1000, 1000));
            PaletteGenerator *pal = (PaletteGenerator *) wItem->data(KWidgetName, Qt::UserRole + 1).value<void *>();
            int pType = pal->type();

            widget->setCaption(wItem->text(KWidgetName));

            QSize size = recursiveCreateWidget(wItem, widget, pType);

            widget->resize(size);
            xPos += widget->width() + 10;
        }
    }
}
