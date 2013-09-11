/*
  Q Light Controller Plus
  fixtureremap.cpp

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

#include <QProgressDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QDir>

#include "audiotriggerfactory.h"
#include "virtualconsole.h"
#include "qlcfixturedef.h"
#include "channelsgroup.h"
#include "fixtureremap.h"
#include "remapwidget.h"
#include "qlcchannel.h"
#include "addfixture.h"
#include "efxfixture.h"
#include "scenevalue.h"
#include "chaserstep.h"
#include "vcslider.h"
#include "vcframe.h"
#include "chaser.h"
#include "scene.h"
#include "efx.h"
#include "doc.h"
#include "app.h"

#define KColumnName         0
#define KColumnAddress      1
#define KColumnUniverse     2
#define KColumnID           3
#define KColumnChIdx        4

FixtureRemap::FixtureRemap(Doc *doc, QWidget *parent)
    : QDialog(parent)
    , m_doc(doc)
{
    Q_ASSERT(doc != NULL);

    setupUi(this);

    connect(m_addButton, SIGNAL(clicked()),
            this, SLOT(slotAddTargetFixture()));
    connect(m_removeButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveTargetFixture()));
    connect(m_remapButton, SIGNAL(clicked()),
            this, SLOT(slotAddRemap()));
    connect(m_unmapButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveRemap()));

    remapWidget = new RemapWidget(m_sourceTree, m_targetTree, this);
    remapWidget->show();
    m_remapLayout->addWidget(remapWidget);

    m_targetDoc = new Doc(this);
    /* Load user fixtures first so that they override system fixtures */
    m_targetDoc->fixtureDefCache()->load(QLCFixtureDefCache::userDefinitionDirectory());
    m_targetDoc->fixtureDefCache()->load(QLCFixtureDefCache::systemDefinitionDirectory());

    m_sourceTree->setIconSize(QSize(24, 24));
    m_sourceTree->setAllColumnsShowFocus(true);
    fillFixturesTree(m_doc, m_sourceTree);

    m_targetTree->setIconSize(QSize(24, 24));
    m_targetTree->setAllColumnsShowFocus(true);

    connect(m_sourceTree->verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(slotUpdateConnections()));
    connect(m_sourceTree, SIGNAL(clicked(QModelIndex)),
            this, SLOT(slotUpdateConnections()));
    connect(m_sourceTree, SIGNAL(expanded(QModelIndex)),
            this, SLOT(slotUpdateConnections()));
    connect(m_sourceTree, SIGNAL(collapsed(QModelIndex)),
            this, SLOT(slotUpdateConnections()));

    connect(m_targetTree->verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(slotUpdateConnections()));
    connect(m_targetTree, SIGNAL(clicked(QModelIndex)),
            this, SLOT(slotUpdateConnections()));
    connect(m_targetTree, SIGNAL(expanded(QModelIndex)),
            this, SLOT(slotUpdateConnections()));
    connect(m_targetTree, SIGNAL(collapsed(QModelIndex)),
            this, SLOT(slotUpdateConnections()));

    // retrieve the original project name for QLC+ main class
    App *mainApp = (App *)m_doc->parent();
    QString prjName = mainApp->fileName();

    if (prjName.lastIndexOf(".") > 0)
        prjName.insert(prjName.lastIndexOf("."), tr(" (remapped)"));
    else
        prjName.append(tr(" (remapped)"));

    m_targetProjectLabel->setText(prjName);
}

FixtureRemap::~FixtureRemap()
{
    delete m_targetDoc;
}

void FixtureRemap::fillFixturesTree(Doc *doc, QTreeWidget *tree)
{
    foreach(Fixture *fxi, doc->fixtures())
    {
        QTreeWidgetItem *topItem = NULL;
        quint32 uni = fxi->universe();
        for (int i = 0; i < tree->topLevelItemCount(); i++)
        {
            QTreeWidgetItem* tItem = tree->topLevelItem(i);
            quint32 tUni = tItem->text(KColumnUniverse).toUInt();
            if (tUni == uni)
            {
                topItem = tItem;
                break;
            }
        }

        // Haven't found this universe node ? Create it.
        if (topItem == NULL)
        {
            topItem = new QTreeWidgetItem(tree);
            topItem->setText(KColumnName, tr("Universe %1").arg(uni + 1));
            topItem->setText(KColumnUniverse, QString::number(uni));
            topItem->setExpanded(true);
        }

        quint32 baseAddr = fxi->address();
        QTreeWidgetItem *fItem = new QTreeWidgetItem(topItem);
        fItem->setText(KColumnName, fxi->name());
        fItem->setIcon(KColumnName, fxi->getIconFromType(fxi->type()));
        fItem->setText(KColumnAddress, QString("%1 - %2").arg(baseAddr + 1).arg(baseAddr + fxi->channels()));
        fItem->setText(KColumnUniverse, QString::number(uni));
        fItem->setText(KColumnID, QString::number(fxi->id()));

        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            const QLCChannel* channel = fxi->channel(c);
            QTreeWidgetItem *item = new QTreeWidgetItem(fItem);
            item->setText(KColumnName, QString("%1:%2").arg(c + 1)
                          .arg(channel->name()));
            item->setIcon(KColumnName, channel->getIconFromGroup(channel->group()));
            item->setText(KColumnUniverse, QString::number(uni));
            item->setText(KColumnID, QString::number(fxi->id()));
            item->setText(KColumnChIdx, QString::number(c));
        }
    }

    tree->resizeColumnToContents(KColumnName);
}

void FixtureRemap::slotAddTargetFixture()
{
    AddFixture af(this, m_targetDoc);
    if (af.exec() == QDialog::Rejected)
        return;

    QString name = af.name();
    quint32 address = af.address();
    quint32 universe = af.universe();
    quint32 channels = af.channels();
    const QLCFixtureDef* fixtureDef = af.fixtureDef();
    const QLCFixtureMode* mode = af.mode();
    int gap = af.gap();

    for(int i = 0; i < af.amount(); i++)
    {
        QString modname;

        /* If an empty name was given use the model instead */
        if (name.simplified().isEmpty())
        {
            if (fixtureDef != NULL)
                name = fixtureDef->model();
            else
                name = tr("Generic Dimmer");
        }

        /* If we're adding more than one fixture,
           append a number to the end of the name */
        if (af.amount() > 1)
            modname = QString("%1 #%2").arg(name).arg(i+1);
        else
            modname = name;

        /* Create the target fixture */
        Fixture* fxi = new Fixture(m_targetDoc);

        /* Add the first fixture without gap, at the given address */
        fxi->setAddress(address + (i * channels) + (i * gap));
        fxi->setUniverse(universe);
        fxi->setName(modname);

        /* Set a fixture definition & mode if they were selected.
           Otherwise assign channels to a generic dimmer. */
        if (fixtureDef != NULL && mode != NULL)
            fxi->setFixtureDefinition(fixtureDef, mode);
        else
            fxi->setChannels(channels);

        m_targetDoc->addFixture(fxi);

        QTreeWidgetItem *topItem = NULL;
        for (int i = 0; i < m_targetTree->topLevelItemCount(); i++)
        {
            QTreeWidgetItem* tItem = m_targetTree->topLevelItem(i);
            quint32 tUni = tItem->text(KColumnUniverse).toUInt();
            if (tUni == universe)
            {
                topItem = tItem;
                break;
            }
        }
        // Haven't found this universe node ? Create it.
        if (topItem == NULL)
        {
            topItem = new QTreeWidgetItem(m_targetTree);
            topItem->setText(KColumnName, tr("Universe %1").arg(universe + 1));
            topItem->setText(KColumnUniverse, QString::number(universe));
            topItem->setExpanded(true);
        }

        quint32 baseAddr = fxi->address();
        QTreeWidgetItem *fItem = new QTreeWidgetItem(topItem);
        fItem->setText(KColumnName, fxi->name());
        fItem->setIcon(KColumnName, fxi->getIconFromType(fxi->type()));
        fItem->setText(KColumnAddress, QString("%1 - %2").arg(baseAddr + 1).arg(baseAddr + fxi->channels()));
        fItem->setText(KColumnUniverse, QString::number(universe));
        fItem->setText(KColumnID, QString::number(fxi->id()));

        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            const QLCChannel* channel = fxi->channel(c);
            QTreeWidgetItem *item = new QTreeWidgetItem(fItem);
            item->setText(KColumnName, QString("%1:%2").arg(c + 1)
                          .arg(channel->name()));
            item->setIcon(KColumnName, channel->getIconFromGroup(channel->group()));
            item->setText(KColumnUniverse, QString::number(universe));
            item->setText(KColumnID, QString::number(fxi->id()));
            item->setText(KColumnChIdx, QString::number(c));
        }
    }
    m_targetTree->resizeColumnToContents(KColumnName);

    qDebug() << "Fixtures in target doc:" << m_targetDoc->fixtures().count();
}

void FixtureRemap::slotRemoveTargetFixture()
{
    if (m_targetTree->selectedItems().count() == 0)
        return;

    QTreeWidgetItem *item = m_targetTree->selectedItems().first();
    bool ok = false;
    quint32 fxid = item->text(KColumnID).toUInt(&ok);
    if (ok == false)
        return;

    // Ask before deletion
    if (QMessageBox::question(this, tr("Delete Fixtures"),
                              tr("Do you want to delete the selected items?"),
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
    {
        return;
    }

    int i = 0;
    QListIterator <RemapInfo> it(m_remapList);
    while (it.hasNext() == true)
    {
        RemapInfo info = it.next();
        quint32 tgtID = info.target->text(KColumnID).toUInt();
        if (tgtID == fxid)
            m_remapList.takeAt(i);
        else
            i++;
    }
    remapWidget->setRemapList(m_remapList);
    m_targetDoc->deleteFixture(fxid);
    for (int i = 0; i < item->childCount(); i++)
    {
        QTreeWidgetItem *child = item->child(i);
        delete child;
    }
    delete item;
    m_targetTree->resizeColumnToContents(KColumnName);

    qDebug() << "Fixtures in target doc:" << m_targetDoc->fixtures().count();
}

void FixtureRemap::slotAddRemap()
{
    if (m_sourceTree->selectedItems().count() == 0 ||
        m_targetTree->selectedItems().count() == 0)
    {
        QMessageBox::warning(this,
                             tr("Invalid selection"),
                             tr("Please select a source and a target fixture or channel to perform this operation."));
        return;
    }

    RemapInfo newRemap;
    newRemap.source = m_sourceTree->selectedItems().first();
    newRemap.target = m_targetTree->selectedItems().first();

    bool srcFxiSelected = false;
    bool tgtFxiSelected = false;

    bool ok = false;
    int chIdx = newRemap.source->text(KColumnChIdx).toInt(&ok);
    if (ok == false)
        srcFxiSelected = true;
    ok = false;
    chIdx = newRemap.target->text(KColumnChIdx).toInt(&ok);
    if (ok == false)
        tgtFxiSelected = true;

    qDebug() << "Idx:" << chIdx << ", src:" << srcFxiSelected << ", tgt:" << tgtFxiSelected;

    if ((srcFxiSelected == true && tgtFxiSelected == false) ||
        (srcFxiSelected == false && tgtFxiSelected == true) )
    {
        QMessageBox::warning(this,
                             tr("Invalid selection"),
                             tr("To perform a fixture remap, please select fixtures on both lists."));
        return;
    }
    else if (srcFxiSelected == true && tgtFxiSelected == true)
    {
        // perform a full fixture remap
        quint32 srcFxiID = newRemap.source->text(KColumnID).toUInt();
        Fixture *srcFxi = m_doc->fixture(srcFxiID);
        Q_ASSERT(srcFxi != NULL);
        quint32 tgtFxiID = newRemap.target->text(KColumnID).toUInt();
        Fixture *tgtFxi = m_targetDoc->fixture(tgtFxiID);
        Q_ASSERT(tgtFxi != NULL);

        if (m_remapNamesCheck->isChecked())
        {
            tgtFxi->setName(srcFxi->name());
            newRemap.target->setText(KColumnName, srcFxi->name());
        }

        for (quint32 s = 0; s < srcFxi->channels(); s++)
        {
            const QLCChannel* srcCh = srcFxi->channel(s);

            for (quint32 t = 0; t < tgtFxi->channels(); t++)
            {
                const QLCChannel* tgtCh = tgtFxi->channel(t);
                if ((tgtCh->group() == srcCh->group()) &&
                    (tgtCh->controlByte() == srcCh->controlByte()))
                {
                    if (tgtCh->group() == QLCChannel::Intensity &&
                        tgtCh->colour() != srcCh->colour())
                            continue;
                    RemapInfo matchInfo;
                    matchInfo.source = newRemap.source->child(s);
                    matchInfo.target = newRemap.target->child(t);
                    m_remapList.append(matchInfo);
                    break;
                }
            }
        }
    }
    else
    {
        // perform a single channel remap
        m_remapList.append(newRemap);
    }

    remapWidget->setRemapList(m_remapList);
}

void FixtureRemap::slotRemoveRemap()
{
    if (m_sourceTree->selectedItems().count() == 0 ||
        m_targetTree->selectedItems().count() == 0)
    {
        QMessageBox::warning(this,
                             tr("Invalid selection"),
                             tr("Please select a source and a target fixture or channel to perform this operation."));
        return;
    }

    RemapInfo delRemap;
    delRemap.source = m_sourceTree->selectedItems().first();
    delRemap.target = m_targetTree->selectedItems().first();

    for (int i = 0; i < m_remapList.count(); i++)
    {
        RemapInfo info = m_remapList.at(i);
        if (info.source == delRemap.source && info.target == delRemap.target)
        {
            m_remapList.takeAt(i);
            i--;
        }
    }
    remapWidget->setRemapList(m_remapList);
}

void FixtureRemap::slotUpdateConnections()
{
    remapWidget->update();
}

QList<SceneValue> FixtureRemap::remapSceneValues(QList<SceneValue> funcList,
                                    QList<SceneValue> &srcList,
                                    QList<SceneValue> &tgtList)
{
    QList <SceneValue> newValuesList;
    foreach(SceneValue val, funcList)
    {
        for( int v = 0; v < srcList.count(); v++)
        {
            if (val == srcList.at(v))
            {
                SceneValue tgtVal = tgtList.at(v);
                qDebug() << "[Scene] Remapping" << val.fxi << val.channel << " to " << tgtVal.fxi << tgtVal.channel;
                newValuesList.append(SceneValue(tgtVal.fxi, tgtVal.channel, val.value));
            }
        }
    }
    return newValuesList;
}

QList<VCWidget *> FixtureRemap::getVCChildren(VCWidget *obj)
{
    QList<VCWidget *> list;
    if (obj == NULL)
        return list;
    QListIterator <VCWidget*> it(obj->findChildren<VCWidget*>());
    while (it.hasNext() == true)
    {
        VCWidget* child = it.next();
        qDebug() << Q_FUNC_INFO << "append: " << child->caption();
        list.append(child);
        list.append(getVCChildren(child));
    }
    return list;
}

void FixtureRemap::accept()
{
    /* **********************************************************************
     * 1 - create a map of SceneValues from the fixtures channel associations
     * ********************************************************************** */
    QList<SceneValue> sourceList;
    QList<SceneValue> targetList;

    foreach (RemapInfo info, m_remapList)
    {
        quint32 srcFxiID = info.source->text(KColumnID).toUInt();
        quint32 srcChIdx = info.source->text(KColumnChIdx).toUInt();

        quint32 tgtFxiID = info.target->text(KColumnID).toUInt();
        quint32 tgtChIdx = info.target->text(KColumnChIdx).toUInt();

        SceneValue srcVal(srcFxiID, srcChIdx);
        SceneValue tgtVal(tgtFxiID, tgtChIdx);
        sourceList.append(srcVal);
        targetList.append(tgtVal);
    }

    /* **********************************************************************
     * 2 - Show a progress dialog, in case the operation takes a while
     * ********************************************************************** */
    QProgressDialog progress(tr("This might take a while..."), tr("Cancel"), 0, 100, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    /* **********************************************************************
     * 3 - replace original project fixtures
     * ********************************************************************** */
    m_doc->replaceFixtures(m_targetDoc->fixtures());

    /* **********************************************************************
     * 4 - remap channel groups
     * ********************************************************************** */
    foreach (ChannelsGroup *grp, m_doc->channelsGroups())
    {
        QList<SceneValue> grpChannels = grp->getChannels();
        // this is crucial: here all the "unmapped" channels will be lost forever !
        grp->resetChannels();
        QList <SceneValue> newList = remapSceneValues(grpChannels, sourceList, targetList);
        foreach (SceneValue val, newList)
            grp->addChannel(val.fxi, val.channel);
    }

    /* **********************************************************************
     * 5 - scan project functions and perform remapping
     * ********************************************************************** */
    int funcNum = m_doc->functions().count();
    int f = 0;
    foreach (Function *func, m_doc->functions())
    {
        switch (func->type())
        {
            case Function::Scene:
            {
                Scene *s = qobject_cast<Scene*>(func);
                qDebug() << "Analyzing Scene #" << s->id();
                QList <SceneValue> newList = remapSceneValues(s->values(), sourceList, targetList);
                // this is crucial: here all the "unmapped" channels will be lost forever !
                s->clear();
                for (int i = 0; i < newList.count(); i++)
                    s->setValue(newList.at(i));
            }
            break;
            case Function::Chaser:
            {
                Chaser *c = qobject_cast<Chaser*>(func);
                if (c->isSequence() == true)
                {
                    for (int idx = 0; idx < c->stepsCount(); idx++)
                    {
                        ChaserStep cs = c->stepAt(idx);
                        QList <SceneValue> newList = remapSceneValues(cs.values, sourceList, targetList);
                        // this is crucial: here all the "unmapped" channels will be lost forever !
                        cs.values.clear();
                        for (int i = 0; i < newList.count(); i++)
                            cs.values.append(newList.at(i));
                        c->replaceStep(cs, idx);
                    }
                }
            }
            break;
            case Function::EFX:
            {
                EFX *e = qobject_cast<EFX*>(func);
                // make a copy of this EFX fixtures list
                QList <EFXFixture*> fixListCopy;
                foreach(EFXFixture *efxFix, e->fixtures())
                {
                    EFXFixture* ef = new EFXFixture(e);
                    ef->copyFrom(efxFix);
                    fixListCopy.append(ef);
                }
                // this is crucial: here all the "unmapped" fixtures will be lost forever !
                e->removeAllFixtures();

                foreach( EFXFixture *efxFix, fixListCopy)
                {
                    quint32 fxID = efxFix->fixture();
                    for (int i = 0; i < sourceList.count(); i++)
                    {
                        SceneValue val = sourceList.at(i);
                        if (val.fxi == fxID)
                        {
                            SceneValue tgtVal = targetList.at(i);
                            Fixture *docFix = m_doc->fixture(tgtVal.fxi);
                            quint32 fxCh = tgtVal.channel;
                            const QLCChannel *chan = docFix->channel(fxCh);
                            if (chan->group() == QLCChannel::Pan ||
                                chan->group() == QLCChannel::Tilt)
                            {
                                EFXFixture* ef = new EFXFixture(e);
                                ef->copyFrom(efxFix);
                                ef->setFixture(tgtVal.fxi);
                                if (e->addFixture(ef) == false)
                                    delete ef;
                                qDebug() << "EFX remap" << val.fxi << "to" << tgtVal.fxi;
                            }
                        }
                    }
                }
                fixListCopy.clear();
            }
            break;
            default:
            break;
        }
        if(progress.wasCanceled())
            break;
        f++;
        progress.setValue((f * 100) / funcNum);
        QApplication::processEvents();
    }

    /* **********************************************************************
     * 6 - remap Virtual Console widgets
     * ********************************************************************** */
    VCFrame* contents = VirtualConsole::instance()->contents();
    QList<VCWidget *> widgetsList = getVCChildren((VCWidget *)contents);

    foreach (QObject *object, widgetsList)
    {
        VCWidget *widget = (VCWidget *)object;
        if (widget->type() == VCWidget::SliderWidget)
        {
            VCSlider *slider = (VCSlider *)object;
            if (slider->sliderMode() == VCSlider::Level)
            {
                QList <VCSlider::LevelChannel> slChannels = slider->levelChannels();
                QList <SceneValue> newChannels;

                foreach (VCSlider::LevelChannel chan, slChannels)
                {
                    for( int v = 0; v < sourceList.count(); v++)
                    {
                        SceneValue val = sourceList.at(v);
                        if (val.fxi == chan.fixture && val.channel == chan.channel)
                        {
                            newChannels.append(SceneValue(targetList.at(v).fxi, targetList.at(v).channel));
                        }
                    }
                }
                // this is crucial: here all the "unmapped" channels will be lost forever !
                slider->clearLevelChannels();
                foreach (SceneValue rmpChan, newChannels)
                    slider->addLevelChannel(rmpChan.fxi, rmpChan.channel);
            }
        }
    }

    /* **********************************************************************
     * 7 - remap Audio Trigger channels
     * ********************************************************************** */
    AudioTriggerFactory *triggers = AudioTriggerFactory::instance();
    foreach (AudioBar *bar, triggers->getAudioBars())
    {
        if (bar->m_type == AudioBar::DMXBar)
        {
            QList <SceneValue> newList = remapSceneValues(bar->m_dmxChannels, sourceList, targetList);
            // this is crucial: here all the "unmapped" channels will be lost forever !
            bar->attachDmxChannels(m_doc, newList);
        }
    }

    /* **********************************************************************
     * 8 - save the remapped project into a new file
     * ********************************************************************** */
    App *mainApp = (App *)m_doc->parent();
    mainApp->setFileName(m_targetProjectLabel->text());
    mainApp->slotFileSave();

    progress.hide();

    /* Close dialog */
    QDialog::accept();
}
