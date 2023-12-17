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
#include "vcxypad.h"
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

void VCWizard::addWidgetItem(QTreeWidgetItem *grpItem, QString name, int type,
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
    item->setIcon(KWidgetName, VCWidget::typeToIcon(type));
    if(name.toLower().contains("speed")) item->setIcon(KWidgetName, QIcon(":/knob.png"));
    
}

void VCWizard::checkPanTilt(QTreeWidgetItem *grpItem, 
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

void VCWizard::checkRGB(QTreeWidgetItem *grpItem, 
                            QTreeWidgetItem *fxGrpItem, qint32* channels){
    
    //Check if all required Channels are set
    for (size_t i = 0; i < 3; i++) if(channels[i] < 0) return;

    quint32 RGB[3] = {};
    for (size_t i = 0; i < 3; i++) RGB[i] = channels[i];

    addWidgetItem(grpItem, "RGB Click & Go", VCWidget::SliderWidget, fxGrpItem, RGB);

    RGB[0] = -1;
    RGB[1] = -1;
    RGB[2] = -1;

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
                
                case QLCChannel::Gobo: addWidgetItem(frame, "Gobo Click & Go", VCWidget::SliderWidget, fxGrpItem, &ch); break;                
                case QLCChannel::Shutter: addWidgetItem(frame, "Shutter Click & Go", VCWidget::SliderWidget, fxGrpItem, &ch); break;
                case QLCChannel::Colour: addWidgetItem(frame, "Colour Click & Go", VCWidget::SliderWidget, fxGrpItem, &ch); break;
               
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
                            addWidgetItem(frame, channel->name() + " Intensity", VCWidget::SliderWidget, fxGrpItem, &ch);
                        } break;
                    }
                }
                break; 
                case QLCChannel::Speed: addWidgetItem(frame, channel->name(), VCWidget::SliderWidget, fxGrpItem, &ch); break;
                break;
                default: addWidgetItem(frame, channel->name() + " " + channel->group(), VCWidget::SliderWidget, fxGrpItem, &ch); break;
                break;

            }
        }           
    }

    m_allWidgetsTree->resizeColumnToContents(KWidgetName);

}

VCWidget *VCWizard::createWidget(int type, VCWidget *parent, int xpos, int ypos, 
                                    QTreeWidgetItem *fxGrpItem, QString str)
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
            
            size_t start = str.indexOf('(') + 1;
            size_t length = str.indexOf(')') - start;
            QStringList numbers = str.mid(start, length).split(", ");
            
            slider->setCaption(str.left(str.indexOf(' ')));

            for (int c = 0; c < fxGrpItem->childCount(); c++)
            {   
                QTreeWidgetItem *fxItem = fxGrpItem->child(c);
                int fxi = fxItem->data(KFixtureColumnName, Qt::UserRole).toInt();
                for(const auto &n : std::as_const(numbers))
                {
                    slider->addLevelChannel(fxi, n.toInt() - 1);
                }
            }
            if(str.contains("RGB")){
                slider->setClickAndGoType(ClickAndGoWidget::RGB);
            }
            else if(str.contains("Intensity"))
            {
                slider->setClickAndGoType(ClickAndGoWidget::None);
            }
            else
            {
                slider->setClickAndGoType(ClickAndGoWidget::Preset);
            }
            
            slider->setSliderMode(VCSlider::Level);
            if(str.toLower().contains("speed")) slider->setWidgetStyle(VCSlider::WKnob);

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
                QTextStream cout(stdout, QIODevice::WriteOnly);
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

    return widget;
}

QSize VCWizard::recursiveCreateWidget(QTreeWidgetItem *item, VCWidget *parent)
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
            QTreeWidgetItem *fxGrpItem = (QTreeWidgetItem *) childItem->data(KWidgetName, Qt::UserRole + 1).value<void *>();

            VCWidget *childWidget = createWidget(cType, parent, subX, subY, fxGrpItem, childItem->text(KWidgetName));
            if (childWidget != NULL)
            {
                if (childItem->childCount() > 0)
                {
                    childWidget->resize(QSize(1000, 1000));

                    QSize size = recursiveCreateWidget(childItem, childWidget);

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

            VCWidget *widget = createWidget(wType, mainFrame, xPos, yPos, nullptr, wItem->text(KWidgetName));
            if (widget == NULL)
                continue;

            widget->resize(QSize(1000, 1000));
            
            QSize size = recursiveCreateWidget(wItem, widget);

            widget->resize(size);
            xPos += widget->width() + 10;
        }
    }
}
