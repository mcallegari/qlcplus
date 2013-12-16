/*
  Q Light Controller Plus
  dmxdumpfactory.cpp

  Copyright (c) Massimo Callegari

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

#include "dmxdumpfactoryproperties.h"
#include "virtualconsole.h"
#include "dmxdumpfactory.h"
#include "chaserstep.h"
#include "function.h"
#include "vcwidget.h"
#include "vcbutton.h"
#include "vcslider.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"

#define KColumnName  0
#define KColumnType  1
#define KColumnID    2

#define KColumnTargetName 0
#define KColumnTargetID   1

DmxDumpFactory::DmxDumpFactory(Doc *doc, DmxDumpFactoryProperties *props, QWidget *parent)
    : QDialog(parent)
    , m_doc(doc)
    , m_properties(props)
{
    Q_ASSERT(doc != NULL);

    setupUi(this);

    m_universesCount = 0;
    m_fixturesCount = 0;
    m_channelsCount = 0;

    connect(m_fixturesTree, SIGNAL(expanded(QModelIndex)),
            this, SLOT(slotItemExpanded()));
    connect(m_fixturesTree, SIGNAL(collapsed(QModelIndex)),
            this, SLOT(slotItemExpanded()));

    updateFixturesTree();

    if (m_properties->selectedTarget() == 1)
        m_buttonRadio->setChecked(true);
    else if (m_properties->selectedTarget() == 2)
        m_sliderRadio->setChecked(true);
    else
        slotUpdateChasersTree();

    m_dumpAllRadio->setText(tr("Dump all channels (%1 Universes, %2 Fixtures, %3 Channels)")
                            .arg(m_universesCount).arg(m_fixturesCount).arg(m_channelsCount));

    m_sceneName->setText(tr("New Scene From Live %1").arg(m_doc->nextFunctionID()));
    if (m_properties->dumpChannelsMode() == true)
        m_dumpAllRadio->setChecked(true);
    else
        m_dumpSelectedRadio->setChecked(true);

    if(m_properties->nonZeroValuesMode() == true)
        m_nonZeroCheck->setChecked(true);
}

DmxDumpFactory::~DmxDumpFactory()
{
}

void DmxDumpFactory::updateFixturesTree()
{
    QByteArray chMask = m_properties->channelsMask();
    m_fixturesTree->clear();
    m_fixturesTree->setIconSize(QSize(24, 24));

    foreach(Fixture *fxi, m_doc->fixtures())
    {
        QTreeWidgetItem *topItem = NULL;
        quint32 uni = fxi->universe();
        for (int i = 0; i < m_fixturesTree->topLevelItemCount(); i++)
        {
            QTreeWidgetItem* tItem = m_fixturesTree->topLevelItem(i);
            quint32 tUni = tItem->text(KColumnID).toUInt();
            if (tUni == uni)
            {
                topItem = tItem;
                break;
            }
        }
        // Haven't found this universe node ? Create it.
        if (topItem == NULL)
        {
            topItem = new QTreeWidgetItem(m_fixturesTree);
            topItem->setText(KColumnName, tr("Universe %1").arg(uni + 1));
            topItem->setText(KColumnID, QString::number(uni));
            topItem->setFlags(topItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsTristate);
            topItem->setCheckState(KColumnName, Qt::Unchecked);
            m_universesCount++;
        }

        QTreeWidgetItem *fItem = new QTreeWidgetItem(topItem);
        fItem->setText(KColumnName, fxi->name());
        fItem->setIcon(KColumnName, fxi->getIconFromType(fxi->type()));
        fItem->setText(KColumnID, QString::number(fxi->id()));
        fItem->setFlags(fItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsTristate);
        fItem->setCheckState(KColumnName, Qt::Unchecked);

        quint32 baseAddress = fxi->universeAddress();
        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            const QLCChannel* channel = fxi->channel(c);
            QTreeWidgetItem *item = new QTreeWidgetItem(fItem);
            item->setText(KColumnName, QString("%1:%2").arg(c + 1)
                          .arg(channel->name()));
            item->setIcon(KColumnName, channel->getIconFromGroup(channel->group()));
            if (channel->group() == QLCChannel::Intensity &&
                channel->colour() != QLCChannel::NoColour)
                item->setText(KColumnType, QLCChannel::colourToString(channel->colour()));
            else
                item->setText(KColumnType, QLCChannel::groupToString(channel->group()));

            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            if (chMask.at(baseAddress + c) == 1)
                item->setCheckState(KColumnName, Qt::Checked);
            else
                item->setCheckState(KColumnName, Qt::Unchecked);
            m_channelsCount++;
        }
        m_fixturesCount++;
    }
    m_fixturesTree->resizeColumnToContents(KColumnName);
    m_fixturesTree->resizeColumnToContents(KColumnType);
}

void DmxDumpFactory::slotUpdateChasersTree()
{
    m_addtoTree->clear();
    foreach(Function *f, m_doc->functionsByType(Function::Chaser))
    {
        Chaser *chaser = qobject_cast<Chaser*>(f);
        if (chaser->isSequence() == false)
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(m_addtoTree);
            item->setText(KColumnTargetName, chaser->name());
            item->setText(KColumnTargetID, QString::number(chaser->id()));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            if (m_properties->isChaserSelected(chaser->id()))
                item->setCheckState(KColumnName, Qt::Checked);
            else
                item->setCheckState(KColumnName, Qt::Unchecked);
        }
    }
}

void DmxDumpFactory::slotUpdateButtons()
{
    updateWidgetsTree(VCWidget::ButtonWidget);
}

void DmxDumpFactory::slotUpdateSliders()
{
    updateWidgetsTree(VCWidget::SliderWidget);
}

QList<VCWidget *> DmxDumpFactory::getChildren(VCWidget *obj, int type)
{
    QList<VCWidget *> list;
    if (obj == NULL)
        return list;
    QListIterator <VCWidget*> it(obj->findChildren<VCWidget*>());
    while (it.hasNext() == true)
    {
        VCWidget* child = it.next();
        qDebug() << Q_FUNC_INFO << "append: " << child->caption();
        if (type == child->type())
            list.append(child);
    }
    return list;
}

void DmxDumpFactory::updateWidgetsTree(int type)
{
    m_addtoTree->clear();
    VCFrame* contents = VirtualConsole::instance()->contents();
    QList<VCWidget *> widgetsList = getChildren((VCWidget *)contents, type);

    foreach (QObject *object, widgetsList)
    {
        VCWidget *widget = (VCWidget *)object;

        QTreeWidgetItem *item = new QTreeWidgetItem(m_addtoTree);
        item->setText(KColumnTargetName, widget->caption());
        item->setIcon(KColumnTargetName, VCWidget::typeToIcon(widget->type()));
        item->setText(KColumnTargetID, QString::number(widget->id()));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(KColumnName, Qt::Unchecked);
    }
}

void DmxDumpFactory::slotDumpModeChanged(bool mode)
{
    if (mode == true)
        m_fixturesTree->setEnabled(false);
    else
        m_fixturesTree->setEnabled(true);
    m_properties->setDumpChannelsMode(mode);
}

void DmxDumpFactory::slotDumpNonZeroChanged(bool active)
{
    m_properties->setNonZeroValuesMode(active);
}

void DmxDumpFactory::slotItemExpanded()
{
    m_fixturesTree->resizeColumnToContents(KColumnName);
    m_fixturesTree->resizeColumnToContents(KColumnType);
}

void DmxDumpFactory::accept()
{
    QByteArray dumpMask = m_properties->channelsMask();
    UniverseArray *ua = m_doc->outputMap()->claimUniverses();
    const QByteArray preGMValues = ua->preGMValues();
    m_doc->outputMap()->releaseUniverses(false);
    Scene *newScene = NULL;
    for (int t = 0; t < m_fixturesTree->topLevelItemCount(); t++)
    {
        QTreeWidgetItem *uniItem = m_fixturesTree->topLevelItem(t);
        if (newScene == NULL && (m_dumpAllRadio->isChecked() ||
             uniItem->checkState(KColumnName) != Qt::Unchecked))
                newScene = new Scene(m_doc);
        //int uni = uniItem->text(KColumnID).toInt();
        for (int f = 0; f < uniItem->childCount(); f++)
        {
            QTreeWidgetItem *fixItem = uniItem->child(f);
            quint32 fxID = fixItem->text(KColumnID).toUInt();
            Fixture *fxi = m_doc->fixture(fxID);
            if (fxi != NULL)
            {
                quint32 baseAddress = fxi->universeAddress();
                for (int c = 0; c < fixItem->childCount(); c++)
                {
                    QTreeWidgetItem *chanItem = fixItem->child(c);
                    SceneValue sv;
                    if (m_dumpAllRadio->isChecked())
                    {
                        dumpMask[baseAddress + c] = 1;
                        uchar value = preGMValues.at(baseAddress + c);
                        if (m_nonZeroCheck->isChecked() == false ||
                           (m_nonZeroCheck->isChecked() == true && value > 0))
                        {
                            SceneValue sv = SceneValue(fxID, c, value);
                            newScene->setValue(sv);
                        }
                    }
                    else
                    {
                        //qDebug() << "Fix: " << fxID << "chan:" << c << "addr:" << (baseAddress + c);
                        if (chanItem->checkState(KColumnName) == Qt::Checked)
                        {
                            dumpMask[baseAddress + c] = 1;
                            uchar value = preGMValues.at(baseAddress + c);
                            if (m_nonZeroCheck->isChecked() == false ||
                               (m_nonZeroCheck->isChecked() == true && value > 0))
                            {
                                SceneValue sv = SceneValue(fxID, c, value);
                                newScene->setValue(sv);
                            }
                        }
                        else
                            dumpMask[baseAddress + c] = 0;
                    }
                }
            }
        }
    }
    /** If the Scene is valid, add it to QLC+ functions */
    if (newScene != NULL)
    {
        newScene->setName(m_sceneName->text());
        if (m_doc->addFunction(newScene) == true)
        {
            quint32 sceneID = newScene->id();
            /** Now add the Scene to the selected Chasers */
            for (int tc = 0; tc < m_addtoTree->topLevelItemCount(); tc++)
            {
                QTreeWidgetItem *targetItem = m_addtoTree->topLevelItem(tc);
                quint32 targetID = targetItem->text(KColumnTargetID).toUInt();
                if (targetItem->checkState(KColumnTargetName) == Qt::Checked)
                {
                    if (m_chaserRadio->isChecked())
                    {
                        Chaser *chaser = qobject_cast<Chaser*>(m_doc->function(targetID));
                        if (chaser != NULL)
                        {
                            ChaserStep chsStep(sceneID);
                            chaser->addStep(chsStep);
                            m_properties->addChaserID(targetID);
                        }
                    }
                    else if (m_buttonRadio->isChecked())
                    {
                        VCButton *button = qobject_cast<VCButton*>(VirtualConsole::instance()->widget(targetID));
                        if (button != NULL)
                        {
                            button->setFunction(newScene->id());
                            button->setCaption(newScene->name());
                        }
                    }
                    else if (m_sliderRadio->isChecked())
                    {
                        VCSlider *slider = qobject_cast<VCSlider*>(VirtualConsole::instance()->widget(targetID));
                        if (slider != NULL)
                        {
                            slider->setPlaybackFunction(newScene->id());
                            slider->setCaption(newScene->name());
                        }
                    }
                }
                else
                    m_properties->removeChaserID(targetID);
            }
        }
    }

    m_properties->setChannelsMask(dumpMask);
    if (m_chaserRadio->isChecked())
        m_properties->setSelectedTarget(0);
    else if (m_buttonRadio->isChecked())
        m_properties->setSelectedTarget(1);
    else if (m_sliderRadio->isChecked())
        m_properties->setSelectedTarget(2);

    /* Close dialog */
    QDialog::accept();
}
