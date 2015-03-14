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
{
    Q_ASSERT(doc != NULL);

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

SoftpatchEditor::~SoftpatchEditor() {}

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
        connect(spin, SIGNAL(valueChanged(int)), this, SLOT(slotChannelPatched(int)));
    }
    m_tree->resizeColumnToContents(KColumnName);
    m_tree->resizeColumnToContents(KColumnUniverse);
    m_tree->resizeColumnToContents(KColumnAddress);
    m_tree->resizeColumnToContents(KColumnPatch);
}

bool SoftpatchEditor::hasOverlappingChannels()
{
    int count;

    const QList<int> keys = m_overlappingChannels.keys();
    for (int i = 0; i < keys.size(); i++)
    {
        QList<QTreeWidgetItem*> values = m_overlappingChannels.values(keys[i]);
        count += values.size();
    }
    return count;
}

void SoftpatchEditor::slotTestButtonPressed()
{
    // switches on (pressed) and off (released) channels
}

void SoftpatchEditor::slotChannelPatched(int address)
{
    QSpinBox* senderSpin = qobject_cast<QSpinBox*>(QObject::sender());
    QVariant var = senderSpin->property("treeItem");
    QTreeWidgetItem *item = (QTreeWidgetItem *) var.value<void *>();
    quint32 senderFxID = item->data(KColumnName, PROP_ID).toUInt();
    Fixture* senderFxi = m_doc->fixture(senderFxID);
    quint32 numChannels = senderFxi->channels();

    //FIXME: limit to first universe now
    if (senderFxi->universe() > 0)
        return;

    // search overlapping map for current item and reset it to white
    const QList<int> keys = m_overlappingChannels.keys();
    for (int i = 0; i < keys.size(); i++)
    {
        QList<QTreeWidgetItem*> values = m_overlappingChannels.values(keys[i]);
        foreach (QTreeWidgetItem* fItem, values)
        {
            if (fItem == item)
            {
                // unmark current item and remove it from map
                QSpinBox *spin = (QSpinBox *)m_tree->itemWidget(fItem, KColumnPatch);
                spin->setStyleSheet("QWidget {background-color:white}");
                m_overlappingChannels.remove(keys[i], fItem);
            }
        }
    }

    // loop through map keys (channels) for single entries and remove them
    for (int i = 0; i < keys.size(); i++)
    {
        QList<QTreeWidgetItem*> values = m_overlappingChannels.values(keys[i]);
        // leftover single map entries (umark them)
        if (values.size() == 1)
        {
            QTreeWidgetItem *fItem = values.at(0);
            QSpinBox *spin = (QSpinBox *)m_tree->itemWidget(fItem, KColumnPatch);
            spin->setStyleSheet("QWidget {background-color:white}");
            m_overlappingChannels.remove(keys[i], fItem);
        }
    }

    // test fixture tree on overlapping channels
    for (int t = 0; t < m_tree->topLevelItemCount(); t++)
    {
        QTreeWidgetItem *uniItem = m_tree->topLevelItem(t);
        for (int f = 0; f < uniItem->childCount(); f++)
        {
            QTreeWidgetItem *fItem = uniItem->child(f);
            quint32 fxID = fItem->data(KColumnName, PROP_ID).toUInt();
            Fixture *testFixture = m_doc->fixture(fxID);
            if (senderFxID != fxID)
            {
                quint32 testNumChannels = testFixture->channels();
                QSpinBox *spin = (QSpinBox *)m_tree->itemWidget(fItem, KColumnPatch);

                // check on overlapping channel
                quint32 testAddress = spin->value();

                // check first and last channel of current fixture
                if ((quint32(address) >= testAddress && quint32(address) < testAddress + testNumChannels)
                || (quint32(address + numChannels -1) >= testAddress && quint32(address + numChannels -1) < testAddress + testNumChannels))
                {
                    // insert overlapping channels into map
                    if (!m_overlappingChannels.contains(address, fItem))
                        m_overlappingChannels.insertMulti(address, fItem);
                    if (!m_overlappingChannels.contains(address, item))
                        m_overlappingChannels.insertMulti(address, item);
                }
            }
        }

        // all overlapping channels marked red now
        QList<QTreeWidgetItem*> values = m_overlappingChannels.values(address);
        for (int i = 0; i < values.size(); ++i)
        {
            QTreeWidgetItem *fItem = values.at(i);
            QSpinBox *spin = (QSpinBox *)m_tree->itemWidget(fItem, KColumnPatch);
            spin->setStyleSheet("QWidget {background-color:red}");
        }
    }
}

void SoftpatchEditor::accept()
{
    if(hasOverlappingChannels())
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
            QSpinBox *spin = (QSpinBox *)m_tree->itemWidget(fItem, KColumnPatch);
            Fixture *fixture = m_doc->fixture(fxID);

            //FIXME: limit to first universe for now
            if (fixture != NULL && fixture->universe() == 0)
            {
                //quint32 universe = fixture->universe();
                //unis[universe]->reset(fixture->address(), fixture->channels());
                m_doc->moveFixture(fixture->id(), spin->value()-1);
                //fixture->setUniverse(universe);
                fixture->setAddress(spin->value()-1);
                emit m_doc->fixtureChanged(fixture->id());
            }
        }
    }
    m_fixture_manager->updateView();
    m_doc->inputOutputMap()->releaseUniverses(true);
    QDialog::accept();
}
