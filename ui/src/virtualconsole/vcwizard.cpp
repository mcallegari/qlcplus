/*
  Q Light Controller
  VCWizard.cpp

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
#include "vcwizard.h"
#include "virtualconsole.h"
#include "vcsoloframe.h"
#include "vcwidget.h"
#include "vcbutton.h"
#include "vcslider.h"
#include "vcframe.h"
#include "fixture.h"
#include "doc.h"

#include "qlcfixturedef.h"
#include "qlccapability.h"
#include "qlcchannel.h"

#define KFixtureColumnName          0
#define KFixtureColumnCaps          1
#define KFixtureColumnManufacturer  2
#define KFixtureColumnModel         3

#define KWidgetName                 0

VCWizard::VCWizard(QWidget* parent, Doc* doc)
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

VCWizard::~VCWizard()
{

}

void VCWizard::slotNextPageClicked()
{
    int newIdx = m_tabWidget->currentIndex() + 1;
    if (newIdx == 4)
        return;

    m_tabWidget->setCurrentIndex(newIdx);
    checkTabsAndButtons();
}

void VCWizard::slotTabClicked()
{
    checkTabsAndButtons();
}

void VCWizard::accept()
{

    addWidgetsToVirtualConsole();

    m_doc->setModified();

    QDialog::accept();
}

void VCWizard::checkTabsAndButtons()
{
    switch(m_tabWidget->currentIndex())
    {
        case 0:
        {
            m_nextButton->setEnabled(true);
            m_tabWidget->setTabEnabled(2, false);
        }
        break;
        case 1:
        {
            if (m_allWidgetsTree->topLevelItemCount() == 0)
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
          m_nextButton->setEnabled(false);
        }
        break;
    }
}

/****************************************************************************
 * Fixtures
 ****************************************************************************/

QTreeWidgetItem *VCWizard::getFixtureGroupItem(QString manufacturer, QString model)
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

void VCWizard::addFixture(quint32 fxi_id)
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

void VCWizard::slotAddClicked()
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
            updateAvailableWidgetsTree();
    }
    checkTabsAndButtons();
}

void VCWizard::slotRemoveClicked()
{
    QListIterator <QTreeWidgetItem*> it(m_fixtureTree->selectedItems());
    while (it.hasNext() == true)
        delete it.next();

    checkTabsAndButtons();
}

QList <quint32> VCWizard::fixtureIds() const
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
 * Widgets
 ********************************************************************/

void VCWizard::addWidgetItem(QTreeWidgetItem *grpItem,
                                       QString name, int type)
{
    if (grpItem == NULL)
        return;

    QTreeWidgetItem *item = new QTreeWidgetItem(grpItem);
    item->setText(KWidgetName, name);
    item->setCheckState(KWidgetName, Qt::Unchecked);
    item->setData(KWidgetName, Qt::UserRole, type);
    item->setIcon(KWidgetName, VCWidget::typeToIcon(type));
    
}

void VCWizard::updateAvailableWidgetsTree()
{
    m_allWidgetsTree->clear();
    
    for (int i = 0; i < m_fixtureTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *fxGrpItem = m_fixtureTree->topLevelItem(i);
        Q_ASSERT(fxGrpItem != NULL);

        if (fxGrpItem->childCount() == 0)
            continue;

        QTreeWidgetItem *frame = new QTreeWidgetItem(m_allWidgetsTree);
        frame->setText(KWidgetName, fxGrpItem->text(KFixtureColumnName));
        frame->setIcon(KWidgetName, VCWidget::typeToIcon(VCWidget::FrameWidget));
        frame->setData(KWidgetName, Qt::UserRole, VCWidget::FrameWidget);
        frame->setFlags(frame->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
        frame->setCheckState(KWidgetName, Qt::Unchecked);
        frame->setExpanded(true);

        // since groups contain fixture of the same type, get the first
        // child and create categories on its capabilities
        QTreeWidgetItem *fxItem = fxGrpItem->child(0);
        quint32 fxID = fxItem->data(KFixtureColumnName, Qt::UserRole).toUInt();
        Fixture* fxi = m_doc->fixture(fxID);
        Q_ASSERT(fxi != NULL);
        
        QStringList caps = PaletteGenerator::getCapabilities(fxi);

        foreach(QString cap, caps)
        {   
            cout << "caps:" << cap << Qt::endl;

            if (cap == KXMLQLCChannelDefault)
            {
                addWidgetItem(frame, "Dimmer", VCWidget::SliderWidget); // Dimmer Slider
            }
            else if (cap == KQLCChannelMovement)
            {
                addWidgetItem(frame, "XY Pad", VCWidget::XYPadWidget); // Movement XY Pad
            }            
            else if (cap == KQLCChannelRGB || cap == KQLCChannelCMY)
            {
                addWidgetItem(frame, "Click & Go RGB", VCWidget::SliderWidget); // RGB Click & GO
            }
            else if (cap == KQLCChannelWhite)
            {
                addWidgetItem(frame, "White", VCWidget::SliderWidget);  // White Slider
            }
            else if (cap == QLCChannel::groupToString(QLCChannel::Gobo))
            {
                addWidgetItem(frame, "Click & Go Gobo", VCWidget::SliderWidget); // Gobo Click & Go
            }
            else if (cap == QLCChannel::groupToString(QLCChannel::Shutter))
            {
                addWidgetItem(frame, "Click & Go Shutter", VCWidget::SliderWidget); // Shutter Click & Go
            }    
            else if (cap == QLCChannel::groupToString(QLCChannel::Colour))
            {
                addWidgetItem(frame, "Click & Go Color", VCWidget::SliderWidget); // Colour Click & Go
            }  
        }
    }

    m_allWidgetsTree->resizeColumnToContents(KWidgetName);

}

VCWidget *VCWizard::createWidget(int type, VCWidget *parent, int xpos, int ypos)
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
        case VCWidget::SliderWidget:
        {
            VCSlider* slider = new VCSlider(parent, m_doc);
            vc->setupWidget(slider, parent);
            slider->move(QPoint(xpos, ypos));
            
            slider->setSliderMode(VCSlider::Level);
            
            widget = slider;
        }
        break;
        default:
        break;
    }

    return widget;
}

QSize VCWizard::recursiveCreateWidget(QTreeWidgetItem *item, VCWidget *parent, int type)
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

            VCWidget *childWidget = createWidget(cType, parent, subX, subY);
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

void VCWizard::addWidgetsToVirtualConsole()
{
    int xPos = 10;
    int yPos = 10;

    VirtualConsole *vc = VirtualConsole::instance();
    VCFrame *mainFrame = vc->contents();
    Q_ASSERT(mainFrame != NULL);

    for (int i = 0; i < m_allWidgetsTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *wItem = m_allWidgetsTree->topLevelItem(i);

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
