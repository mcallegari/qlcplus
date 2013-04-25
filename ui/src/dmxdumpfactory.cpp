/*
  Q Light Controller Plus
  dmxdumpfactory.cpp

  Copyright (c) Massimo Callegari

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

#include "dmxdumpfactoryproperties.h"
#include "dmxdumpfactory.h"
#include "chaserstep.h"
#include "function.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"

#define KColumnName  0
#define KColumnType  1
#define KColumnID    2

#define KColumnChaserName 0
#define KColumnChaserID   1

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

    updateFixturesTree();
    updateChasersTree();

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
    m_fixturesTree->header()->setResizeMode(QHeaderView::ResizeToContents);
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
}

void DmxDumpFactory::updateChasersTree()
{
    m_chasersTree->clear();
    foreach(Function *f, m_doc->functionsByType(Function::Chaser))
    {
        Chaser *chaser = qobject_cast<Chaser*>(f);
        if (chaser->isSequence() == false)
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(m_chasersTree);
            item->setText(KColumnChaserName, chaser->name());
            item->setText(KColumnChaserID, QString::number(chaser->id()));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            if (m_properties->isChaserSelected(chaser->id()))
                item->setCheckState(KColumnName, Qt::Checked);
            else
                item->setCheckState(KColumnName, Qt::Unchecked);
        }
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
                    SceneValue sv();
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
            for (int tc = 0; tc < m_chasersTree->topLevelItemCount(); tc++)
            {
                QTreeWidgetItem *chsItem = m_chasersTree->topLevelItem(tc);
                quint32 chsID = chsItem->text(KColumnChaserID).toUInt();
                if (chsItem->checkState(KColumnChaserName) == Qt::Checked)
                {
                    Chaser *chaser = qobject_cast<Chaser*>(m_doc->function(chsID));
                    if (chaser != NULL)
                    {
                        ChaserStep chsStep(sceneID);
                        chaser->addStep(chsStep);
                        m_properties->addChaserID(chsID);
                    }
                }
                else
                    m_properties->removeChaserID(chsID);
            }
        }
    }

    m_properties->setChannelsMask(dumpMask);

    /* Close dialog */
    QDialog::accept();
}
