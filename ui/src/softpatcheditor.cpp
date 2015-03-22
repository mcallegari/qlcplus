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
#include <QSpinBox>
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
    , testChannel(0)
    , testValue(0)
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
        QSpinBox *spin = new QSpinBox();
        spin->setRange(1, 255);
        spin->setProperty("treeItem", qVariantFromValue((void *)fItem));
        spin->setValue(fxi->address()+1);
        m_tree->setItemWidget(fItem, KColumnPatch ,spin);
        connect(spin, SIGNAL(valueChanged(int)), this, SLOT(slotChannelPatched()));
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
        QSpinBox *spin = (QSpinBox *)m_tree->itemWidget(item, KColumnPatch);

        if(fxi->isDimmer())
        {
            m_mutex.lock();
            testChannel = spin->value() - 1;
            testUniverse = fxi->universe();
            testValue = uchar(255);
            runTest = true;
            resetTest = false;
            m_mutex.unlock();
        }
    }
    else
    {
        m_mutex.lock();
        testValue = uchar(0);
        resetTest = true;
        m_mutex.unlock();
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
                QSpinBox *spin = (QSpinBox *)m_tree->itemWidget(pItem, KColumnPatch);
                spin->setStyleSheet("QWidget {background-color:white}");
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
            QSpinBox *spin = (QSpinBox *)m_tree->itemWidget(mItem, KColumnPatch);
            spin->setStyleSheet("QWidget {background-color:white}");

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
            QSpinBox *spin = (QSpinBox *)m_tree->itemWidget(mItem, KColumnPatch);
            spin->setStyleSheet("QWidget {background-color:red}");
        }
    }
}

QSet<quint32> SoftpatchEditor::getChannelSet(QTreeWidgetItem* item)
{
    quint32 fxID = item->data(KColumnName, PROP_ID).toUInt();
    Fixture *fxi = m_doc->fixture(fxID);
    QSpinBox *spin = (QSpinBox *)m_tree->itemWidget(item, KColumnPatch);
    QSet<quint32> set = QSet<quint32>();
    for (uint i = 0; i < fxi->channels(); i++)
    {
        // universe starts with 0 ( channel = spin.value -1 )
        set.insert((spin->value() - 1) + i + (fxi->universe() << 9));
    }
    return set;
}

QSet<quint32> SoftpatchEditor::duplicateChannelsSet(QTreeWidgetItem* changed, QTreeWidgetItem* other)
{
    QSet<quint32> set = getChannelSet(changed);
    return set.intersect(getChannelSet(other));
}

void SoftpatchEditor::slotChannelPatched()
{
    QSpinBox* senderSpin = qobject_cast<QSpinBox*>(QObject::sender());
    senderSpin->blockSignals(true);

    QVariant var = senderSpin->property("treeItem");
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
    senderSpin->blockSignals(false);
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

    m_doc->inputOutputMap()->claimUniverses();

    for (int t = 0; t < m_tree->topLevelItemCount(); t++)
    {
        QTreeWidgetItem *uniItem = m_tree->topLevelItem(t);
        for (int f = 0; f < uniItem->childCount(); f++)
        {
            QTreeWidgetItem *fItem = uniItem->child(f);
            quint32 fxID = fItem->data(KColumnName, PROP_ID).toUInt();
            QSpinBox *spin = (QSpinBox *)m_tree->itemWidget(fItem, KColumnPatch);
            Fixture *fixture = m_doc->fixture(fxID);
            if (fixture->address() != quint32(spin->value()-1))
                fixture->setAddress(spin->value()-1);
        }
    }
    m_fixture_manager->updateView();
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
        ua[testUniverse]->write(testChannel, testValue);
        if (resetTest)
            runTest = false;
    }
    m_mutex.unlock();
}
