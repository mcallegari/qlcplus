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
#include <QDebug>
#include <QSettings>

#include "dmxdumpfactoryproperties.h"
#include "fixturetreewidget.h"
#include "functionselection.h"
#include "virtualconsole.h"
#include "dmxdumpfactory.h"
#include "universe.h"
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

#define SETTINGS_GEOMETRY "dmxdumpfactory/geometry"

DmxDumpFactory::DmxDumpFactory(Doc *doc, DmxDumpFactoryProperties *props, QWidget *parent)
    : QDialog(parent)
    , m_doc(doc)
    , m_properties(props)
    , m_selectedSceneID(Function::invalidId())
{
    Q_ASSERT(doc != NULL);

    setupUi(this);

    quint32 treeFlags = FixtureTreeWidget::ChannelType |
                        FixtureTreeWidget::ChannelSelection;

    m_fixturesTree = new FixtureTreeWidget(m_doc, treeFlags, this);
    m_fixturesTree->setIconSize(QSize(24, 24));
    m_fixturesTree->setSortingEnabled(false);

    m_treeLayout->addWidget(m_fixturesTree);
    m_fixturesTree->setChannelsMask(m_properties->channelsMask());

    m_fixturesTree->updateTree();

    if (m_properties->selectedTarget() == DmxDumpFactoryProperties::VCButton)
        m_buttonRadio->setChecked(true);
    else if (m_properties->selectedTarget() == DmxDumpFactoryProperties::VCSlider)
        m_sliderRadio->setChecked(true);
    else
        slotUpdateChasersTree();

    m_dumpAllRadio->setText(tr("Dump all channels (%1 Universes, %2 Fixtures, %3 Channels)")
                            .arg(m_fixturesTree->universeCount()).arg(m_fixturesTree->fixturesCount()).arg(m_fixturesTree->channelsCount()));

    m_sceneName->setText(tr("New Scene From Live %1").arg(m_doc->nextFunctionID()));
    if (m_properties->dumpChannelsMode() == true)
        m_dumpAllRadio->setChecked(true);
    else
        m_dumpSelectedRadio->setChecked(true);

    if (m_properties->nonZeroValuesMode() == true)
        m_nonZeroCheck->setChecked(true);

    QSettings settings;
    QVariant geometrySettings = settings.value(SETTINGS_GEOMETRY);
    if (geometrySettings.isValid() == true)
        restoreGeometry(geometrySettings.toByteArray());

    connect(m_sceneButton, SIGNAL(clicked(bool)),
            this, SLOT(slotSelectSceneButtonClicked()));
}

DmxDumpFactory::~DmxDumpFactory()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

void DmxDumpFactory::slotUpdateChasersTree()
{
    m_addtoTree->clear();
    foreach (Function *f, m_doc->functionsByType(Function::ChaserType))
    {
        Chaser *chaser = qobject_cast<Chaser*>(f);
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

void DmxDumpFactory::slotUpdateButtons()
{
    updateWidgetsTree(VCWidget::ButtonWidget);
}

void DmxDumpFactory::slotUpdateSliders()
{
    updateWidgetsTree(VCWidget::SliderWidget);
}

void DmxDumpFactory::slotSelectSceneButtonClicked()
{
    FunctionSelection fs(this, m_doc);
    fs.setMultiSelection(false);
    fs.setFilter(Function::SceneType, true);

    if (fs.exec() == QDialog::Accepted && fs.selection().size() > 0)
    {
        m_selectedSceneID = fs.selection().first();
        Scene *scene = qobject_cast<Scene*>(m_doc->function(m_selectedSceneID));
        if (scene == NULL)
            return;

        m_sceneName->setText(scene->name());
        m_dumpSelectedRadio->setChecked(true);
        QByteArray chMask = m_properties->channelsMask();
        chMask.fill(0);

        foreach (SceneValue scv, scene->values())
        {
            Fixture *fxi = m_doc->fixture(scv.fxi);
            if (fxi == NULL)
                continue;
            quint32 absAddress = fxi->universeAddress() + scv.channel;
            if (chMask.length() > (int)absAddress)
                chMask[absAddress] = 1;
        }
        m_properties->setChannelsMask(chMask);
        m_fixturesTree->setChannelsMask(chMask);
        m_fixturesTree->updateTree();
    }
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
    VCFrame *contents = VirtualConsole::instance()->contents();
    QList<VCWidget *> widgetsList = getChildren((VCWidget *)contents, type);

    foreach (QObject *object, widgetsList)
    {
        VCWidget *widget = qobject_cast<VCWidget *>(object);

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

void DmxDumpFactory::accept()
{
    QByteArray dumpMask = m_properties->channelsMask();
    QList<Universe*> ua = m_doc->inputOutputMap()->claimUniverses();

    QByteArray preGMValues(ua.size() * UNIVERSE_SIZE, 0); //= ua->preGMValues();

    for (int i = 0; i < ua.count(); ++i)
    {
        const int offset = i * UNIVERSE_SIZE;
        preGMValues.replace(offset, UNIVERSE_SIZE, ua.at(i)->preGMValues());
        if (ua.at(i)->passthrough())
        {
            for (int j = 0; j < UNIVERSE_SIZE; ++j)
            {
                const int ofs = offset + j;
                preGMValues[ofs] =
                    static_cast<char>(ua.at(i)->applyPassthrough(j, static_cast<uchar>(preGMValues[ofs])));
            }
        }
    }

    m_doc->inputOutputMap()->releaseUniverses(false);

    Scene *newScene = NULL;
    if (m_selectedSceneID != Function::invalidId())
        newScene = qobject_cast<Scene*>(m_doc->function(m_selectedSceneID));

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
            quint32 fxID = fixItem->data(KColumnName, PROP_ID).toUInt();
            Fixture *fxi = m_doc->fixture(fxID);
            if (fxi != NULL)
            {
                quint32 baseAddress = fxi->universeAddress();
                for (int c = 0; c < fixItem->childCount(); c++)
                {
                    QTreeWidgetItem *chanItem = fixItem->child(c);
                    quint32 channel = chanItem->data(KColumnName, PROP_CHANNEL).toUInt();

                    if (m_dumpAllRadio->isChecked())
                    {
                        dumpMask[baseAddress + channel] = 1;
                        uchar value = preGMValues.at(baseAddress + channel);
                        if (m_nonZeroCheck->isChecked() == false ||
                           (m_nonZeroCheck->isChecked() == true && value > 0))
                        {
                            SceneValue sv = SceneValue(fxID, channel, value);
                            newScene->setValue(sv);
                        }
                    }
                    else
                    {
                        //qDebug() << "Fix: " << fxID << "chan:" << channel << "addr:" << (baseAddress + channel);
                        if (chanItem->checkState(KColumnName) == Qt::Checked)
                        {
                            dumpMask[baseAddress + channel] = 1;
                            uchar value = preGMValues.at(baseAddress + channel);
                            if (m_nonZeroCheck->isChecked() == false ||
                               (m_nonZeroCheck->isChecked() == true && value > 0))
                            {
                                SceneValue sv = SceneValue(fxID, channel, value);
                                newScene->setValue(sv);
                            }
                        }
                        else
                            dumpMask[baseAddress + channel] = 0;
                    }
                }
            }
        }
    }
    /** If the Scene is valid, add it to QLC+ functions */
    if (newScene != NULL)
    {
        bool addedToDoc = false;

        if (m_selectedSceneID != Function::invalidId() &&
            m_doc->function(m_selectedSceneID) != NULL)
        {
            addedToDoc = true;
        }
        else
        {
            newScene->setName(m_sceneName->text());
            addedToDoc = m_doc->addFunction(newScene);
        }
        if (addedToDoc == true)
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
        m_properties->setSelectedTarget(DmxDumpFactoryProperties::Chaser);
    else if (m_buttonRadio->isChecked())
        m_properties->setSelectedTarget(DmxDumpFactoryProperties::VCButton);
    else if (m_sliderRadio->isChecked())
        m_properties->setSelectedTarget(DmxDumpFactoryProperties::VCSlider);

    /* Close dialog */
    QDialog::accept();
}
