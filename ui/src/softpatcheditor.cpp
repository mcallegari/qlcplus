/*
  Q Light Controller Plus
  channelsselection.cpp

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
#include <QPushButton>
#include <QMessageBox>
#include <QLineEdit>
#include <QDebug>

#include "softpatcheditor.h"
#include "fixturemanager.h"
#include "universe.h"
#include "doc.h"

#define KColumnName         0
#define KColumnUniverse     1
#define KColumnAddress      2
#define KColumnPatch        3

SoftpatchEditor::SoftpatchEditor(Doc *doc, FixtureManager *mgr, QWidget *parent)
    : QDialog(parent)
    , m_doc(doc)
    , m_fixture_manager(mgr)
    , runTest(false)
    , resetTest(false)
    , testUniverse(0)
    , testSwitch(false)
{
    Q_ASSERT(doc != NULL);

    m_doc->masterTimer()->registerDMXSource(this, "SoftpatchTest");

    setupUi(this);

    setWindowTitle(tr("Patch start addresses of Fixtures"));
    setWindowIcon(QIcon(":/input_output.png"));

    QStringList hdrLabels;
    hdrLabels << tr("Name") << tr("Universe") << tr("Address") << tr("New Address");
    m_tree->setHeaderLabels(hdrLabels);

    updateFixturesTree();

    connect(m_testButton, SIGNAL(pressed()), this, SLOT(slotTestButtonPressed()));
    connect(m_testButton, SIGNAL(released()), this, SLOT(slotTestButtonPressed()));
    connect(m_oneToOneButton, SIGNAL(pressed()), this, SLOT(slotOneToOnePressed()));
}

SoftpatchEditor::~SoftpatchEditor()
{
    m_duplicateChannels.clear();
    m_mutex.lock();
    m_doc->masterTimer()->unregisterDMXSource(this);
    m_mutex.unlock();
}

void SoftpatchEditor::updateFixturesTree()
{
    m_tree->clear();
    m_tree->setIconSize(QSize(24, 24));
    m_tree->setAllColumnsShowFocus(true);

    foreach(Fixture *fxi, m_doc->fixtures())
    {
        QTreeWidgetItem *topItem = NULL;
        quint32 uni = fxi->universe();
        for (int i = 0; i < m_tree->topLevelItemCount(); i++)
        {
            QTreeWidgetItem* tItem = m_tree->topLevelItem(i);
            quint32 tUni = tItem->text(KColumnUniverse).toUInt();
            if ((tUni-1) == uni)
            {
                topItem = tItem;
                break;
            }
        }
        // Haven't found this universe node ? Create it.
        if (topItem == NULL)
        {
            topItem = new QTreeWidgetItem(m_tree);
            topItem->setText(KColumnName, tr("Universe %1").arg(uni + 1));
            topItem->setText(KColumnUniverse, QString::number(uni + 1));
            topItem->setExpanded(true);
        }

        QTreeWidgetItem *fItem = new QTreeWidgetItem(topItem);

        // Column Name
        fItem->setText(KColumnName, fxi->name());
        fItem->setIcon(KColumnName, fxi->getIconFromType(fxi->type()));
        fItem->setData(KColumnName, PROP_ID, fxi->id());

        // Column Universe
        fItem->setText(KColumnUniverse, QString::number(fxi->universe() + 1));

        // Column Address
        QString s;
        s.sprintf("%.3d - %.3d", fxi->address() + 1, fxi->address() + fxi->channels());
        fItem->setText(KColumnAddress, s);
        fItem->setData(KColumnAddress, PROP_ADDRESS, fxi->address());

        // Column Softpatch
        QList<Universe*> unis = m_doc->inputOutputMap()->claimUniverses();
        QLineEdit *ledit = new QLineEdit();
        // TODO
        //ledit->setRange(0, 512);
        ledit->setProperty("treeItem", qVariantFromValue((void *)fItem));
        ledit->setStyleSheet("QWidget {background-color:white}");
        const QList<uint> channels = unis[fxi->universe()]->getPatchedChannels(fxi->address());
        QString tc = channelsToText(channels);
        ledit->setText(tc);
        m_tree->setItemWidget(fItem, KColumnPatch ,ledit);
        connect(ledit, SIGNAL(editingFinished()), this, SLOT(slotChannelPatched()));
        m_doc->inputOutputMap()->releaseUniverses();
    }
    m_tree->resizeColumnToContents(KColumnName);
    m_tree->resizeColumnToContents(KColumnUniverse);
    m_tree->resizeColumnToContents(KColumnAddress);
    m_tree->resizeColumnToContents(KColumnPatch);
}

bool SoftpatchEditor::hasDupliateChannels()
{
    int count = 0;
    foreach(uint key, m_duplicateChannels.uniqueKeys())
    {
        QList<QTreeWidgetItem*> values = m_duplicateChannels.values(key);
        count += values.size();
    }
    return count;
}

void SoftpatchEditor::slotTestButtonPressed()
{

    if (m_testButton->isDown())
    {
        QTreeWidgetItem* item = m_tree->currentItem();
        quint32 FxID = item->data(KColumnName, PROP_ID).toUInt();
        Fixture* fxi = m_doc->fixture(FxID);
        QLineEdit *ledit = (QLineEdit *)m_tree->itemWidget(item, KColumnPatch);

        if(fxi->isDimmer())
        {
            m_mutex.lock();
            foreach (uint ch, textToChannels(ledit->text()))
                testChannels.append(ch - 1);
            testUniverse = fxi->universe();
            testSwitch = true;
            runTest = true;
            resetTest = false;
            m_mutex.unlock();
        }
    }
    else
    {
        m_mutex.lock();
        testSwitch = false;
        resetTest = true;
        m_mutex.unlock();
    }
}

void SoftpatchEditor::slotOneToOnePressed()
{
    QMessageBox msg(QMessageBox::Warning, tr("Reset Patch"),
                    tr("Pressing OK will reset your patch to a one to one patch"), QMessageBox::Ok|QMessageBox::Cancel);

    int ret = msg.exec();

    switch (ret) {
        case QMessageBox::Ok:
        {
            QList<Universe*> unis = m_doc->inputOutputMap()->claimUniverses();
            foreach (Universe* universe, unis)
                universe->patchOneToOne();
            m_doc->inputOutputMap()->releaseUniverses();
            m_duplicateChannels.clear();
            updateFixturesTree();
            break;
        }
        case QMessageBox::Cancel:
            break;
        default:
            break;
    }

}

void SoftpatchEditor::initChannelSearch(QTreeWidgetItem* item)
{
    // reset prev entry
    foreach (const uint &key, m_duplicateChannels.uniqueKeys())
    {
        foreach (QTreeWidgetItem* pItem, m_duplicateChannels.values(key))
        {
            if (pItem == item)
            {
                QLineEdit *ledit = (QLineEdit *)m_tree->itemWidget(pItem, KColumnPatch);
                ledit->setStyleSheet("QWidget {background-color:white}");
                m_duplicateChannels.remove(key, pItem);
            }
        }
    }

    // remove single leftover entries
    foreach (const uint &key, m_duplicateChannels.uniqueKeys())
    {
        if (m_duplicateChannels.values(key).size() == 1)
        {
            QTreeWidgetItem* mItem = m_duplicateChannels.values(key).at(0);
            QLineEdit *ledit = (QLineEdit *)m_tree->itemWidget(mItem, KColumnPatch);
            ledit->setStyleSheet("QWidget {background-color:white}");

            m_duplicateChannels.remove(key);
        }
    }
}

void SoftpatchEditor::markFixtures()
{
    foreach (const uint &key, m_duplicateChannels.uniqueKeys())
    {
        foreach (QTreeWidgetItem* mItem, m_duplicateChannels.values(key))
        {
            QLineEdit *ledit = (QLineEdit *)m_tree->itemWidget(mItem, KColumnPatch);
            ledit->setStyleSheet("QWidget {background-color:red}");
        }
    }
}

QSet<quint32> SoftpatchEditor::getChannelSet(QTreeWidgetItem* item)
{
    quint32 fxID = item->data(KColumnName, PROP_ID).toUInt();
    Fixture *fxi = m_doc->fixture(fxID);
    QLineEdit *ledit = (QLineEdit *)m_tree->itemWidget(item, KColumnPatch);
    QSet<quint32> set = QSet<quint32>();

    foreach (uint ch, textToChannels(ledit->text()))
    {
        for (uint i = 0; i < fxi->channels(); i++)
        {
            // value == 0 means unpatched
            if (ch > 0)
                // universe starts with 0 ( channel = gui_entry -1 )
                set.insert((ch - 1) + i + (fxi->universe() << 9));
        }
    }
    qDebug() << Q_FUNC_INFO << " Set: " << set;
    return set;
}

QSet<quint32> SoftpatchEditor::duplicateChannelsSet(QTreeWidgetItem* changed, QTreeWidgetItem* other)
{
    QSet<quint32> set = getChannelSet(changed);
    return set.intersect(getChannelSet(other));
}

void SoftpatchEditor::slotChannelPatched()
{
    QLineEdit* senderLedit = qobject_cast<QLineEdit*>(QObject::sender());
    senderLedit->blockSignals(true);

    QVariant var = senderLedit->property("treeItem");
    QTreeWidgetItem *item = (QTreeWidgetItem *) var.value<void *>();
    quint32 senderFxID = item->data(KColumnName, PROP_ID).toUInt();

    initChannelSearch(item);

    // test fixture tree on overlapping channels
    for (int t = 0; t < m_tree->topLevelItemCount(); t++)
    {
        QTreeWidgetItem *uniItem = m_tree->topLevelItem(t);
        for (int f = 0; f < uniItem->childCount(); f++)
        {
            QTreeWidgetItem *fItem = uniItem->child(f);
            quint32 fxID = fItem->data(KColumnName, PROP_ID).toUInt();
            if (senderFxID != fxID)
            {
                foreach (quint32 dSet, duplicateChannelsSet(item, fItem))
                {
                    // insert overlapping channels into map
                    if (!m_duplicateChannels.contains(dSet, fItem))
                        m_duplicateChannels.insertMulti(dSet, fItem);
                    if (!m_duplicateChannels.contains(dSet, item))
                        m_duplicateChannels.insertMulti(dSet, item);
                }
            }
        }
    }
    markFixtures();
    senderLedit->blockSignals(false);
}

void SoftpatchEditor::accept()
{
    if(hasDupliateChannels())
    {
        QMessageBox msg(QMessageBox::Critical, tr("Error"),
                        tr("Please solve overlapping channels first"), QMessageBox::Ok);
        msg.exec();
        return;
    }

    QList<Universe*> unis = m_doc->inputOutputMap()->claimUniverses();

    for (int t = 0; t < m_tree->topLevelItemCount(); t++)
    {
        QTreeWidgetItem *uniItem = m_tree->topLevelItem(t);
        for (int f = 0; f < uniItem->childCount(); f++)
        {
            QTreeWidgetItem *fItem = uniItem->child(f);
            quint32 fxID = fItem->data(KColumnName, PROP_ID).toUInt();
            QLineEdit *ledit = (QLineEdit *)m_tree->itemWidget(fItem, KColumnPatch);
            Fixture *fixture = m_doc->fixture(fxID);
            Universe *universe = unis[fixture->universe()];


            // remove old patch
            for (uint i = 0; i < fixture->channels(); i++)
            {
                uint dimmer = fixture->address() + i;
                QList<uint> channels = universe->getPatchedChannels(dimmer);
                foreach (uint channel, channels)
                    universe->unPatchChannel(channel);
            }

            foreach (uint ch, textToChannels(ledit->text()))
            {
                if (ch > 0)
                {
                    for (uint i = 0; i < fixture->channels(); i++)
                    {
                        // new patch
                        uint dimmer = fixture->address() + i;
                        uint channel = ch - 1 + i;
                        unis[fixture->universe()]->patchDimmer(dimmer, channel);
                    }
                }
                // ledit value == 0 is unpatch
                else
                {
                    // unpatch each patched channel for each channel in fixture (if Fixture (1,2) is patched to (10,12) and (11,13), unpatch 10,11,12,13)
                    for (uint i = 0; i < fixture->channels(); i++)
                    {
                        // 1. get current patch of the dimmer
                        uint dimmer = fixture->address();
                        QList<uint> channels = universe->getPatchedChannels(dimmer);
                        // 2. unpatch each channel found in patch for this dimmer
                        foreach (uint channel, channels)
                            universe->unPatchChannel(channel);
                    }
                }
            }
        }
    }
    m_doc->inputOutputMap()->releaseUniverses();

    m_duplicateChannels.clear();

    QDialog::accept();
}

void SoftpatchEditor::writeDMX(MasterTimer* timer, QList<Universe *> ua)
{
    Q_UNUSED(timer);

    m_mutex.lock();
    if (runTest)
    {
        ua[testUniverse]->testDimmer(testChannels, testSwitch);
        if (resetTest)
        {
            testChannels.clear();
            runTest = false;
        }
    }
    m_mutex.unlock();
}

QList<quint32> SoftpatchEditor::textToChannels(QString text)
{
    QString s = text.trimmed();
    QStringList slist = s.split(",", QString::SkipEmptyParts);
    QList<uint> channels;
    foreach (QString ch, slist) {
        bool ok;
        uint v = ch.toUInt(&ok);
        if (ok == true && v <= 512)
            channels.append(v);
        else
        {
            QMessageBox msg(QMessageBox::Warning, tr("Patch Error"),
                            tr("separate channels by \",\""), QMessageBox::Ok);
        }
    }
    return channels;
}

QString SoftpatchEditor::channelsToText(QList<uint> channels)
{
    QListIterator <uint> it(channels);
    QString chanstr;
    while (it.hasNext())
    {
        chanstr.append(QString("%1").arg(it.next()+1));
        if(it.hasNext())
            chanstr.append(",");
    }

    return chanstr;
}
