/*
  Q Light Controller
  addfixture_test.cpp

  Copyright (c) Heikki Junnila

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

#include <QtTest>
#include <QList>

#include "qlcfixturedefcache.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "outputpatch.h"
#include "qlcfile.h"
#include "fixture.h"
#include "doc.h"

#include "addfixture_test.h"
#define protected public
#include "addfixture.h"
#undef protected

#include "../../../engine/test/common/resource_paths.h"

void AddFixture_Test::initTestCase()
{
    m_doc = new Doc(this);

    QSettings settings;
    m_expanded = settings.value(SETTINGS_EXPANDED);

    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_doc->fixtureDefCache()->load(dir) == true);
}

void AddFixture_Test::cleanupTestCase()
{
    QSettings settings;
    if(m_expanded.isValid())
    {
        settings.setValue(SETTINGS_EXPANDED, m_expanded);
    }
    else
    {
        settings.remove(SETTINGS_EXPANDED);
    }

    delete m_doc;
}

void AddFixture_Test::findAddress()
{
    QList <Fixture*> fixtures;

    /* All addresses are available (except for fixtures taking more than
       one complete universe). */
    QVERIFY(AddFixture::findAddress(15, fixtures, 4) == 0);
    QVERIFY(AddFixture::findAddress(0, fixtures, 4) == QLCChannel::invalid());
    QVERIFY(AddFixture::findAddress(512, fixtures, 4) == 0);
    QVERIFY(AddFixture::findAddress(513, fixtures, 4) == QLCChannel::invalid());

    Fixture* f1 = new Fixture(m_doc);
    f1->setChannels(15);
    f1->setAddress(10);
    fixtures << f1;

    /* There's a fixture taking 15 channels (10-24) */
    QVERIFY(AddFixture::findAddress(10, fixtures, 4) == 0);
    QVERIFY(AddFixture::findAddress(11, fixtures, 4) == 25);

    Fixture* f2 = new Fixture(m_doc);
    f2->setChannels(15);
    f2->setAddress(10);
    fixtures << f2;

    /* Now there are two fixtures at the same address, with all channels
       overlapping. */
    QVERIFY(AddFixture::findAddress(10, fixtures, 4) == 0);
    QVERIFY(AddFixture::findAddress(11, fixtures, 4) == 25);

    /* Now only some channels overlap (f2: 0-14, f1: 10-24) */
    f2->setAddress(0);
    QVERIFY(AddFixture::findAddress(1, fixtures, 4) == 25);
    QVERIFY(AddFixture::findAddress(10, fixtures, 4) == 25);
    QVERIFY(AddFixture::findAddress(11, fixtures, 4) == 25);

    Fixture* f3 = new Fixture(m_doc);
    f3->setChannels(5);
    f3->setAddress(30);
    fixtures << f3;

    /* Next free slot for max 5 channels is between 25 and 30 */
    QVERIFY(AddFixture::findAddress(1, fixtures, 4) == 25);
    QVERIFY(AddFixture::findAddress(5, fixtures, 4) == 25);
    QVERIFY(AddFixture::findAddress(6, fixtures, 4) == 35);
    QVERIFY(AddFixture::findAddress(11, fixtures, 4) == 35);

    /* Next free slot is found only from the next universe */
    QVERIFY(AddFixture::findAddress(500, fixtures, 4) == 512);

    while (fixtures.isEmpty() == false)
        delete fixtures.takeFirst();
}

void AddFixture_Test::initialNoFixture()
{
    AddFixture af(NULL, m_doc);
    QVERIFY(m_doc == af.m_doc);
    QVERIFY(af.fixtureDef() == NULL);
    QVERIFY(af.mode() == NULL);
    QCOMPARE(af.name(), tr("Dimmers"));
    QVERIFY(af.address() == 0);
    QVERIFY(af.universe() == 0);
    QVERIFY(af.amount() == 1);
    QVERIFY(af.gap() == 0);
    QVERIFY(af.channels() == 1);
    QVERIFY(af.m_tree->columnCount() == 1);

    // Check that all makes & models are put to the tree
    QStringList makers(m_doc->fixtureDefCache()->manufacturers());
    QVERIFY(makers.isEmpty() == false);
    for (int i = 0; i < af.m_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* top = af.m_tree->topLevelItem(i);

        if (top->text(0) != KXMLFixtureGeneric)
        {
            QStringList models(m_doc->fixtureDefCache()->models(top->text(0)));
            for (int j = 0; j < top->childCount(); j++)
            {
                QTreeWidgetItem* child = top->child(j);
                QCOMPARE(child->childCount(), 0);
                QCOMPARE(models.removeAll(child->text(0)), 1);
            }

            QCOMPARE(makers.removeAll(top->text(0)), 1);
        }
        else
        {
            QCOMPARE(i, af.m_tree->topLevelItemCount() - 1); // Generic should be last
            QCOMPARE(top->childCount(), 3);
            QCOMPARE(top->child(0)->text(0), QString(KXMLFixtureGeneric));

            QStringList models(m_doc->fixtureDefCache()->models(top->text(0)));
            for (int j = 0; j < top->childCount(); j++)
            {
                QTreeWidgetItem* child = top->child(j);
                QCOMPARE(child->childCount(), 0);
                QCOMPARE(models.removeAll(child->text(0)), child->text(0) == KXMLFixtureGeneric ? 0 : 1);
            }

            QCOMPARE(makers.removeAll(top->text(0)), 1);
        }
    }
    QVERIFY(makers.isEmpty() == true);

    // Generic / Generic should be selected by default
    QVERIFY(af.m_tree->currentItem() != NULL);
    QCOMPARE(af.m_tree->currentItem()->text(0), QString(KXMLFixtureGeneric));
    QVERIFY(af.m_tree->currentItem()->parent() != NULL);
    QCOMPARE(af.m_tree->currentItem()->parent()->text(0), QString(KXMLFixtureGeneric));

    QVERIFY(af.m_modeCombo->isEnabled() == false);
    QCOMPARE(af.m_modeCombo->count(), 1);
    QCOMPARE(af.m_modeCombo->itemText(0), QString(KXMLFixtureGeneric));

    QVERIFY(af.m_universeCombo->isEnabled() == true);
    QCOMPARE(af.m_universeCombo->currentIndex(), 0);
    QCOMPARE(af.m_universeCombo->count(), 4);

    QVERIFY(af.m_addressSpin->isEnabled() == true);
    QCOMPARE(af.m_addressSpin->value(), 1);
    QCOMPARE(af.m_addressSpin->minimum(), 1);
    QCOMPARE(af.m_addressSpin->maximum(), 512);

    QVERIFY(af.m_channelsSpin->isEnabled() == true);
    QCOMPARE(af.m_channelsSpin->value(), 1);

    QVERIFY(af.m_nameEdit->isEnabled() == true);
    QCOMPARE(af.m_nameEdit->text(), QString(KXMLFixtureDimmer + QString("s")));
    QVERIFY(af.m_nameEdit->isModified() == false);

    QVERIFY(af.m_multipleGroup->isEnabled() == true);
    QVERIFY(af.m_gapSpin->isEnabled() == true);
    QCOMPARE(af.m_gapSpin->value(), 0);
    QVERIFY(af.m_amountSpin->isEnabled() == true);
    QCOMPARE(af.m_amountSpin->value(), 1);
}

void AddFixture_Test::initialDimmer()
{
    Fixture* fxi = new Fixture(m_doc);
    fxi->setChannels(6);
    fxi->setName("My dimmer");
    fxi->setUniverse(2);
    fxi->setAddress(484);
    m_doc->addFixture(fxi);

    AddFixture af(NULL, m_doc, fxi);
    QVERIFY(m_doc == af.m_doc);
    QVERIFY(af.fixtureDef() == NULL);
    QVERIFY(af.mode() == NULL);
    QVERIFY(af.name() == QString("My dimmer"));
    QVERIFY(af.address() == 484);
    QVERIFY(af.universe() == 2);
    QVERIFY(af.amount() == 1);
    QVERIFY(af.gap() == 0);
    QVERIFY(af.channels() == 6);

    // Check that all makes & models are put to the tree
    QStringList makers(m_doc->fixtureDefCache()->manufacturers());
    QVERIFY(makers.isEmpty() == false);
    for (int i = 0; i < af.m_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* top = af.m_tree->topLevelItem(i);

        if (top->text(0) != KXMLFixtureGeneric)
        {
            QStringList models(m_doc->fixtureDefCache()->models(top->text(0)));
            for (int j = 0; j < top->childCount(); j++)
            {
                QTreeWidgetItem* child = top->child(j);
                QCOMPARE(child->childCount(), 0);
                QCOMPARE(models.removeAll(child->text(0)), 1);
            }

            QCOMPARE(makers.removeAll(top->text(0)), 1);
        }
        else
        {
            QCOMPARE(i, af.m_tree->topLevelItemCount() - 1); // Generic should be last
            QCOMPARE(top->childCount(), 3);
            QCOMPARE(top->child(0)->text(0), QString(KXMLFixtureGeneric));

            QStringList models(m_doc->fixtureDefCache()->models(top->text(0)));
            for (int j = 0; j < top->childCount(); j++)
            {
                QTreeWidgetItem* child = top->child(j);
                QCOMPARE(child->childCount(), 0);
                QCOMPARE(models.removeAll(child->text(0)), child->text(0) == KXMLFixtureGeneric ? 0 : 1);
            }

            QCOMPARE(makers.removeAll(top->text(0)), 1);
        }
    }
    QVERIFY(makers.isEmpty() == true);

    // Generic / Generic should be selected for dimmers
    QVERIFY(af.m_tree->currentItem() != NULL);
    QCOMPARE(af.m_tree->currentItem()->text(0), QString(KXMLFixtureGeneric));
    QVERIFY(af.m_tree->currentItem()->parent() != NULL);
    QCOMPARE(af.m_tree->currentItem()->parent()->text(0), QString(KXMLFixtureGeneric));

    QVERIFY(af.m_modeCombo->isEnabled() == false);
    QCOMPARE(af.m_modeCombo->count(), 1);
    QCOMPARE(af.m_modeCombo->itemText(0), QString(KXMLFixtureGeneric));

    QVERIFY(af.m_universeCombo->isEnabled() == true);
    QCOMPARE(af.m_universeCombo->currentIndex(), 2);
    QCOMPARE(af.m_universeCombo->count(), 4);

    QVERIFY(af.m_addressSpin->isEnabled() == true);
    QCOMPARE(af.m_addressSpin->value(), 485);
    QCOMPARE(af.m_addressSpin->minimum(), 1);
    QCOMPARE(af.m_addressSpin->maximum(), int(513 - fxi->channels()));

    QVERIFY(af.m_channelsSpin->isEnabled() == true);
    QCOMPARE(af.m_channelsSpin->value(), 6);

    QVERIFY(af.m_nameEdit->isEnabled() == true);
    QCOMPARE(af.m_nameEdit->text(), QString("My dimmer"));
    QVERIFY(af.m_nameEdit->isModified() == true);

    QVERIFY(af.m_multipleGroup->isEnabled() == false);
    QVERIFY(af.m_gapSpin->isEnabled() == false);
    QCOMPARE(af.m_gapSpin->value(), 0);

    QVERIFY(af.m_amountSpin->isEnabled() == false);
    QCOMPARE(af.m_amountSpin->value(), 1);
}

void AddFixture_Test::initialScanner()
{
    Fixture* fxi = new Fixture(m_doc);
    fxi->setName("My scanner");

    QLCFixtureDef* def = m_doc->fixtureDefCache()->fixtureDef("Martin", "MAC300");
    Q_ASSERT(def != NULL);
    Q_ASSERT(def != NULL);
    Q_ASSERT(def->channels().size() > 0);
    QLCFixtureMode* mode = def->modes().first();
    Q_ASSERT(def->modes().size() > 1);

    fxi->setFixtureDefinition(def, mode);
    fxi->setUniverse(2);
    fxi->setAddress(484);
    m_doc->addFixture(fxi);

    AddFixture af(NULL, m_doc, fxi);
    QVERIFY(m_doc == af.m_doc);
    QVERIFY(af.fixtureDef() == def);
    QVERIFY(af.mode() == mode);
    QVERIFY(af.name() == QString("My scanner"));
    QVERIFY(af.address() == 484);
    QVERIFY(af.universe() == 2);
    QVERIFY(af.amount() == 1);
    QVERIFY(af.gap() == 0);
    QVERIFY(af.channels() == fxi->channels());

    // Check that all makes & models are put to the tree
    QStringList makers(m_doc->fixtureDefCache()->manufacturers());
    QVERIFY(makers.isEmpty() == false);
    for (int i = 0; i < af.m_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* top = af.m_tree->topLevelItem(i);

        if (top->text(0) != KXMLFixtureGeneric)
        {
            QStringList models(m_doc->fixtureDefCache()->models(top->text(0)));
            for (int j = 0; j < top->childCount(); j++)
            {
                QTreeWidgetItem* child = top->child(j);
                QCOMPARE(child->childCount(), 0);
                QCOMPARE(models.removeAll(child->text(0)), 1);
            }

            QCOMPARE(makers.removeAll(top->text(0)), 1);
        }
        else
        {
            QCOMPARE(i, af.m_tree->topLevelItemCount() - 1); // Generic should be last
            QCOMPARE(top->childCount(), 3);
            QCOMPARE(top->child(0)->text(0), QString(KXMLFixtureGeneric));

            QStringList models(m_doc->fixtureDefCache()->models(top->text(0)));
            for (int j = 0; j < top->childCount(); j++)
            {
                QTreeWidgetItem* child = top->child(j);
                QCOMPARE(child->childCount(), 0);
                QCOMPARE(models.removeAll(child->text(0)), child->text(0) == KXMLFixtureGeneric ? 0 : 1);
            }

            QCOMPARE(makers.removeAll(top->text(0)), 1);
        }
    }
    QVERIFY(makers.isEmpty() == true);

    // Generic / Generic should be selected for dimmers
    QVERIFY(af.m_tree->currentItem() != NULL);
    QCOMPARE(af.m_tree->currentItem()->text(0), def->model());
    QVERIFY(af.m_tree->currentItem()->parent() != NULL);
    QCOMPARE(af.m_tree->currentItem()->parent()->text(0), def->manufacturer());

    QVERIFY(af.m_modeCombo->isEnabled() == true);
    QCOMPARE(af.m_modeCombo->count(), def->modes().size());
    QCOMPARE(af.m_modeCombo->itemText(0), mode->name());

    QVERIFY(af.m_universeCombo->isEnabled() == true);
    QCOMPARE(af.m_universeCombo->currentIndex(), 2);
    QCOMPARE(af.m_universeCombo->count(), 4);

    QVERIFY(af.m_addressSpin->isEnabled() == true);
    QCOMPARE(af.m_addressSpin->value(), 485);

    QVERIFY(af.m_channelsSpin->isEnabled() == false);
    QCOMPARE(af.m_channelsSpin->value(), (int) fxi->channels());

    QVERIFY(af.m_nameEdit->isEnabled() == true);
    QCOMPARE(af.m_nameEdit->text(), QString("My scanner"));
    QVERIFY(af.m_nameEdit->isModified() == true);

    QVERIFY(af.m_multipleGroup->isEnabled() == false);
    QVERIFY(af.m_gapSpin->isEnabled() == false);
    QCOMPARE(af.m_gapSpin->value(), 0);

    QVERIFY(af.m_amountSpin->isEnabled() == false);
    QCOMPARE(af.m_amountSpin->value(), 1);
}

void AddFixture_Test::selectionNothing()
{
    AddFixture af(NULL, m_doc);

    af.m_tree->setCurrentItem(NULL);
    QVERIFY(af.m_tree->currentItem() == NULL);

    QVERIFY(af.fixtureDef() == NULL);
    QVERIFY(af.mode() == NULL);
    QVERIFY(af.name() == "");
    QVERIFY(af.address() == 0);
    QVERIFY(af.universe() == 0);
    QVERIFY(af.amount() == 1);
    QVERIFY(af.gap() == 0);
    QVERIFY(af.channels() == 1);

    QVERIFY(af.m_modeCombo->isEnabled() == false);
    QCOMPARE(af.m_modeCombo->count(), 1);
    QCOMPARE(af.m_modeCombo->itemText(0), QString());

    QVERIFY(af.m_universeCombo->isEnabled() == false);
    QCOMPARE(af.m_universeCombo->currentIndex(), 0);
    QCOMPARE(af.m_universeCombo->count(), 4);

    QVERIFY(af.m_addressSpin->isEnabled() == false);
    QCOMPARE(af.m_addressSpin->value(), 1);
    QCOMPARE(af.m_addressSpin->minimum(), 1);
    QCOMPARE(af.m_addressSpin->maximum(), 512);

    QVERIFY(af.m_channelsSpin->isEnabled() == false);
    QCOMPARE(af.m_channelsSpin->value(), 1);

    QVERIFY(af.m_nameEdit->isEnabled() == false);

    QVERIFY(af.m_multipleGroup->isEnabled() == false);
    QVERIFY(af.m_gapSpin->isEnabled() == false);
    QCOMPARE(af.m_gapSpin->value(), 0);
    QVERIFY(af.m_amountSpin->isEnabled() == false);
}

void AddFixture_Test::selectionGeneric()
{
    AddFixture af(NULL, m_doc);

    // Select the last item which should be Generic - Generic
    QTreeWidgetItem* item = af.m_tree->topLevelItem(af.m_tree->topLevelItemCount() - 1);
    QVERIFY(item != NULL);
    // First, select the parent node so that selectionChanged() fires
    QCOMPARE(item->childCount(), 3);
    af.m_tree->setCurrentItem(item);
    // Then, select the child to fire again
    item = item->child(0);
    QVERIFY(item != NULL);
    af.m_tree->setCurrentItem(item);
    QVERIFY(af.m_tree->currentItem() == item);

    QVERIFY(af.m_modeCombo->isEnabled() == false);
    QCOMPARE(af.m_modeCombo->count(), 1);
    QCOMPARE(af.m_modeCombo->itemText(0), QString(KXMLFixtureGeneric));
    QVERIFY(af.fixtureDef() == NULL);
    QVERIFY(af.mode() == NULL);

    QVERIFY(af.m_universeCombo->isEnabled() == true);
    QCOMPARE(af.m_universeCombo->currentIndex(), 0);
    QCOMPARE(af.m_universeCombo->count(), 4);
    QVERIFY(af.universe() == 0);

    QVERIFY(af.m_addressSpin->isEnabled() == true);
    QCOMPARE(af.m_addressSpin->value(), 1);
    QCOMPARE(af.m_addressSpin->minimum(), 1);
    QCOMPARE(af.m_addressSpin->maximum(), 512);
    QVERIFY(af.address() == 0);

    QVERIFY(af.m_channelsSpin->isEnabled() == true);
    QCOMPARE(af.m_channelsSpin->value(), 1);
    QVERIFY(af.channels() == 1);

    QVERIFY(af.m_nameEdit->isEnabled() == true);
    QCOMPARE(af.name(), QString(KXMLFixtureDimmer) + QString("s"));

    QVERIFY(af.m_multipleGroup->isEnabled() == true);
    QVERIFY(af.m_gapSpin->isEnabled() == true);
    QCOMPARE(af.m_gapSpin->value(), 0);
    QVERIFY(af.gap() == 0);

    QVERIFY(af.m_amountSpin->isEnabled() == true);
    QVERIFY(af.m_amountSpin->value() == 1);
    QVERIFY(af.amount() == 1);
}


void AddFixture_Test::rememberExpanded()
{
    {
        AddFixture af(NULL, m_doc);
        af.m_tree->invisibleRootItem()->child(5)->setExpanded(true);
    }

    {
        AddFixture af(NULL, m_doc);
        QVERIFY(af.m_tree->invisibleRootItem()->child(5)->isExpanded());
    }
}

QTEST_MAIN(AddFixture_Test)
