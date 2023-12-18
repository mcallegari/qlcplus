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
#include <QAction>

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
#include "vcxypad.h"
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
            m_nextButton->setEnabled(m_widgetsTree->topLevelItemCount() > 0);            
        }
        break;
        case 3:
            m_nextButton->setEnabled(false);
        break;
    }

    // enable VC Widget Page if tree not empty
    m_tabWidget->setTabEnabled(3, m_widgetsTree->topLevelItemCount() > 0);
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
        {
            updateAvailableFunctionsTree();
            updateWidgetsTree();
        }
    }
    checkTabsAndButtons();
}

void FunctionWizard::slotRemoveClicked()
{
    QListIterator <QTreeWidgetItem*> it(m_fixtureTree->selectedItems());
    while (it.hasNext() == true)
        delete it.next();
    
    updateWidgetsTree();
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

        foreach (QString cap, caps)
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

                foreach (Scene *scene, palette->scenes())
                {
                    QTreeWidgetItem *item = new QTreeWidgetItem(getFunctionGroupItem(scene));
                    item->setText(KFunctionName, scene->name());
                    item->setIcon(KFunctionName, scene->getIcon());
                }
                foreach (Chaser *chaser, palette->chasers())
                {
                    QTreeWidgetItem *item = new QTreeWidgetItem(getFunctionGroupItem(chaser));
                    item->setText(KFunctionName, chaser->name());
                    item->setIcon(KFunctionName, chaser->getIcon());
                }
                foreach (RGBMatrix *matrix, palette->matrices())
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
    updateWidgetsTree();

    checkTabsAndButtons();
}

/********************************************************************
 * Widgets
 ********************************************************************/

void FunctionWizard::addWidgetItem(QTreeWidgetItem *grpItem, QString name, int type,
                             QTreeWidgetItem *fxGrpItem, quint32 *channels)
{
    if (grpItem == NULL)
        return;

    QString channelsStr = "(";
    size_t cnt = 1;
    if(name.contains("RGB"))cnt = 3;
    if(name.contains("XY PAD"))cnt = 4;

    for (size_t i = 0; i < cnt; i++)
    {   
        if((channels[i]+1)==0) continue;
        channelsStr.append( QString::number((uint)channels[i]+1)+ ", ");
    }
    channelsStr.chop(2);
    channelsStr.append(")");

    QTreeWidgetItem *item = new QTreeWidgetItem(grpItem);
    item->setText(KWidgetName, name + " " + channelsStr );
    item->setCheckState(KWidgetName, Qt::Unchecked);
    item->setData(KWidgetName, Qt::UserRole, type);
    item->setData(KWidgetName, Qt::UserRole + 1, QVariant::fromValue((void*)fxGrpItem));
    item->setData(KWidgetName, Qt::UserRole + 2, QVariant::fromValue(channels[0]));
    item->setIcon(KWidgetName, VCWidget::typeToIcon(type));
    if(name.toLower().contains("speed")) item->setIcon(KWidgetName, QIcon(":/knob.png"));
    
}

void FunctionWizard::checkPanTilt(QTreeWidgetItem *grpItem, 
                            QTreeWidgetItem *fxGrpItem, qint32* channels){
        
    //Check if all required Channels are set
    if(channels[0] < 0) return;
    if(channels[2] < 0) return;
    if(channels[1] > 0 && channels[3] < 0) return;
    
    quint32 PanTilt[4] = {};
    for (size_t i = 0; i < 4; i++) PanTilt[i] = channels[i];    

    addWidgetItem(grpItem, "XY PAD", VCWidget::XYPadWidget, fxGrpItem, PanTilt);

    channels[0] = -1;
    channels[1] = -1;
    channels[2] = -1;
    channels[3] = -1;

}

void FunctionWizard::checkRGB(QTreeWidgetItem *grpItem, 
                            QTreeWidgetItem *fxGrpItem, qint32* channels){
    
    //Check if all required Channels are set
    for (size_t i = 0; i < 3; i++) if(channels[i] < 0) return;

    quint32 RGB[3] = {};
    for (size_t i = 0; i < 3; i++) RGB[i] = channels[i];

    addWidgetItem(grpItem, "RGB - Click & Go", VCWidget::SliderWidget, fxGrpItem, RGB);

    RGB[0] = -1;
    RGB[1] = -1;
    RGB[2] = -1;

}

void FunctionWizard::updateWidgetsTree()
{
    m_widgetsTree->clear();

    // Populate palette Widgets
    foreach (PaletteGenerator *palette, m_paletteList)
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
        frame->setData(KWidgetName, Qt::UserRole + 1, QVariant::fromValue((void *)palette));
        frame->setFlags(frame->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
        frame->setCheckState(KWidgetName, Qt::Unchecked);

        QTreeWidgetItem *soloFrameItem = NULL;
        if (palette->scenes().count() > 0)
        {
            soloFrameItem = new QTreeWidgetItem(frame);
            soloFrameItem->setText(KWidgetName, tr("Presets solo frame"));
            soloFrameItem->setIcon(KWidgetName, VCWidget::typeToIcon(VCWidget::SoloFrameWidget));
            soloFrameItem->setFlags(soloFrameItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
            soloFrameItem->setCheckState(KWidgetName, Qt::Unchecked);
            soloFrameItem->setData(KWidgetName, Qt::UserRole, VCWidget::SoloFrameWidget);
        }
        foreach (Scene *scene, palette->scenes())
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
            item->setData(KWidgetName, Qt::UserRole + 1, QVariant::fromValue((void *)scene));

        }
        foreach (Chaser *chaser, palette->chasers())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(frame);
            QString toRemove = " - " + palette->model();
            item->setText(KWidgetName, chaser->name().remove(toRemove));
            item->setIcon(KWidgetName, VCWidget::typeToIcon(VCWidget::CueListWidget));
            item->setCheckState(KWidgetName, Qt::Unchecked);
            item->setData(KWidgetName, Qt::UserRole, VCWidget::CueListWidget);
            item->setData(KWidgetName, Qt::UserRole + 1, QVariant::fromValue((void *)chaser));
        }

        foreach (RGBMatrix *matrix, palette->matrices())
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
            item->setData(KWidgetName, Qt::UserRole + 1, QVariant::fromValue((void *)matrix));
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
            item->setData(KWidgetName, Qt::UserRole + 1, QVariant::fromValue((void *)firstScene));
        }
    }

    // Populate Fixture channel Widgets
    for (int i = 0; i < m_fixtureTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *fxGrpItem = m_fixtureTree->topLevelItem(i);
        Q_ASSERT(fxGrpItem != NULL);

        if (fxGrpItem->childCount() == 0)
            continue;

        QTreeWidgetItem *frame = new QTreeWidgetItem(m_widgetsTree);
        frame->setText(KWidgetName, "Channels - " + fxGrpItem->text(KFixtureColumnName));
        frame->setIcon(KWidgetName, VCWidget::typeToIcon(VCWidget::FrameWidget));
        frame->setData(KWidgetName, Qt::UserRole, VCWidget::FrameWidget);
        frame->setData(KWidgetName, Qt::UserRole + 1, QVariant::fromValue((void *)NULL));
        frame->setFlags(frame->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
        frame->setCheckState(KWidgetName, Qt::Unchecked);
        frame->setExpanded(true);

        // since groups contain fixture of the same type, get the first
        // child and create categories on its capabilities
        QTreeWidgetItem *fxItem = fxGrpItem->child(0);
        quint32 fxID = fxItem->data(KFixtureColumnName, Qt::UserRole).toUInt();
        Fixture* fxi = m_doc->fixture(fxID);
        Q_ASSERT(fxi != NULL);

        frame->setData(KWidgetName, Qt::UserRole + 1, QVariant::fromValue((void *)fxi));
        
        qint32 lastPanTilt[] = {-1, -1, -1, -1};
        qint32 lastRGB[] = {-1, -1, -1};

        for (quint32 ch = 0; ch < fxi->channels(); ch++)
        {
            const QLCChannel* channel(fxi->channel(ch));
            Q_ASSERT(channel != NULL);
                    
            switch (channel->group())
            {
                case QLCChannel::Pan: {
                    if(channel->preset()==QLCChannel::PositionPan) lastPanTilt[0] = ch;
                    if(channel->preset()==QLCChannel::PositionPanFine) lastPanTilt[1] = ch;

                    checkPanTilt(frame, fxGrpItem, lastPanTilt);
                } 
                break;
                case QLCChannel::Tilt: {
                    if(channel->preset()==QLCChannel::PositionTilt) lastPanTilt[2] = ch;
                    if(channel->preset()==QLCChannel::PositionTiltFine) lastPanTilt[3] = ch;

                    checkPanTilt(frame, fxGrpItem, lastPanTilt);
                } 
                break;
                
                // Glick & Go's
                case QLCChannel::Gobo: 
                case QLCChannel::Shutter:
                case QLCChannel::Prism:
                case QLCChannel::Beam:
                case QLCChannel::Effect:
                case QLCChannel::Colour:
                    addWidgetItem(frame, QLCChannel::groupToString(channel->group()) + " - Click & Go", VCWidget::SliderWidget, fxGrpItem, &ch);
                break;
               
                case QLCChannel::Intensity:
                { QLCChannel::PrimaryColour col = channel->colour();
                    switch (col)
                    {
                        case QLCChannel::Red: {
                            lastRGB[0]=ch;
                            checkRGB(frame, fxGrpItem, lastRGB);
                        }
                        break;
                        case QLCChannel::Green: 
                        {
                            lastRGB[1]=ch;
                            checkRGB(frame, fxGrpItem, lastRGB);
                        }
                        break;
                        case QLCChannel::Blue: {
                            lastRGB[2]=ch;
                            checkRGB(frame, fxGrpItem, lastRGB);
                        }
                        break;
                        default:{                            
                            addWidgetItem(frame, channel->name() + " - Intensity", VCWidget::SliderWidget, fxGrpItem, &ch);
                        } break;
                    }
                }
                break; 
                case QLCChannel::Speed: addWidgetItem(frame, channel->name() + " - " + QLCChannel::groupToString(channel->group()), VCWidget::SliderWidget, fxGrpItem, &ch); break;
                break;
                default: addWidgetItem(frame, channel->name() + " - " + QLCChannel::groupToString(channel->group()), VCWidget::SliderWidget, fxGrpItem, &ch); break;
                break;

            }
        }           
    }
}

VCWidget *FunctionWizard::createWidget(int type, VCWidget *parent, int xpos, int ypos,
                                       Function *func, int pType, QTreeWidgetItem *fxGrpItem,
                                       quint32 chan)
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
            // Click & Go Slider in Pallette Frames
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
            else
            {
                // get channel of first Fixture
                int fxID = fxGrpItem->child(0)->data(KFixtureColumnName, Qt::UserRole).toInt();
                Fixture *fixture = m_doc->fixture(fxID);
                const QLCChannel *channel = fixture->channel(chan);

                bool isRGB = channel->colour()==QLCChannel::Red;
                
                for (int c = 0; c < fxGrpItem->childCount(); c++)
                {   
                    QTreeWidgetItem *fxItem = fxGrpItem->child(c);
                    int fxi = fxItem->data(KFixtureColumnName, Qt::UserRole).toInt();
                    
                    slider->addLevelChannel(fxi, chan);
                    if(isRGB)
                    {
                        slider->addLevelChannel(fxi, chan + 1);
                        slider->addLevelChannel(fxi, chan + 2);
                    }
                }
                
                if(isRGB){
                    slider->setClickAndGoType(ClickAndGoWidget::RGB);
                }
                else if(channel->group() == QLCChannel::Intensity)
                {
                    slider->setClickAndGoType(ClickAndGoWidget::None);
                }
                else
                {
                    slider->setClickAndGoType(ClickAndGoWidget::Preset);
                }
                
                if(channel->group()==QLCChannel::Speed)
                    slider->setWidgetStyle(VCSlider::WKnob);

               
                slider->setSliderMode(VCSlider::Level); 
            }

            widget = slider;
        }
        break;
        case VCWidget::XYPadWidget:
        {
            VCXYPad* XYPad = new VCXYPad(parent, m_doc);
            vc->setupWidget(XYPad, parent);
            XYPad->move(QPoint(xpos, ypos));

            for (int c = 0; c < fxGrpItem->childCount(); c++)
            {
                QTreeWidgetItem *fxItem = fxGrpItem->child(c);
                int fxID = fxItem->data(KFixtureColumnName, Qt::UserRole).toInt();
                VCXYPadFixture fxi = VCXYPadFixture(m_doc);
                fxi.setHead(GroupHead(fxID,0));
                XYPad->appendFixture(fxi);
            }
                      
            widget = XYPad;
        }
        break;
        default:
        break;
    }

    // Set Function
    if (widget != NULL && func != NULL)
    {
        if (func->type() == Function::SceneType && type == VCWidget::ButtonWidget)
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
                foreach (SceneValue scv, scene->values())
                {
                    Fixture *fixture = m_doc->fixture(scv.fxi);
                    if (fixture == NULL)
                        continue;

                    const QLCChannel* channel(fixture->channel(scv.channel));
                    if (channel->group() == QLCChannel::Gobo)
                    {
                        QLCCapability *cap = channel->searchCapability(scv.value);
                        if (cap->resource(0).isValid())
                        {
                            widget->setBackgroundImage(cap->resource(0).toString());
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
            Function *func = NULL;
            QTreeWidgetItem *fxGrpItem  = NULL;
            quint32 channel = 0;
            if(type)
                func = (Function *) childItem->data(KWidgetName, Qt::UserRole + 1).value<void *>();
            else
            {
                fxGrpItem = (QTreeWidgetItem *) childItem->data(KWidgetName, Qt::UserRole + 1).value<void *>();
                channel = childItem->data(KWidgetName, Qt::UserRole + 2).toUInt();

            }   
                

            VCWidget *childWidget = createWidget(cType, parent, subX, subY, func, type, fxGrpItem, channel);
            if (childWidget != NULL)
            {
                if(childWidget->type()==VCWidget::SliderWidget)
                    childWidget->setCaption(childItem->text(KWidgetName).split(" - ")[0]);
                else
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

            widget->resize(QSize(2000, 1000));

            int pType = 0;
            if(!wItem->text(KWidgetName).contains("Channels"))
            {
                PaletteGenerator *pal = (PaletteGenerator *) wItem->data(KWidgetName, Qt::UserRole + 1).value<void *>();
                pType = pal->type();
            }

            widget->setCaption(wItem->text(KWidgetName));

            QSize size = recursiveCreateWidget(wItem, widget, pType);

            widget->resize(size);
            xPos += widget->width() + 10;
        }
    }
}
