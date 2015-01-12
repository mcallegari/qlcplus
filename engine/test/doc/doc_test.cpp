/*
  Q Light Controller - Unit test
  doc_test.cpp

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

#include <QPointer>
#include <QtTest>
#include <QtXml>

#define protected public
#define private public

#include "qlcfixturedefcache.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "qlcphysical.h"
#include "collection.h"
#include "qlcchannel.h"
#include "qlcfile.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "efx.h"
#include "bus.h"
#include "doc.h"

#include "doc_test.h"

#undef protected
#undef private

#include "../common/resource_paths.h"

void Doc_Test::initTestCase()
{
    Bus::init(this);

    m_doc = new Doc(this);

    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_doc->fixtureDefCache()->load(dir) == true);
}

void Doc_Test::init()
{
}

void Doc_Test::cleanup()
{
    QSignalSpy spy1(m_doc, SIGNAL(clearing()));
    QSignalSpy spy2(m_doc, SIGNAL(cleared()));
    m_doc->clearContents();
    QCOMPARE(spy1.size(), 1);
    QCOMPARE(spy2.size(), 1);
    QCOMPARE(m_doc->functions().size(), 0);
    QCOMPARE(m_doc->fixtures().size(), 0);
    QCOMPARE(m_doc->fixtureGroups().size(), 0);
    QCOMPARE(m_doc->m_latestFunctionId, quint32(0));
    QCOMPARE(m_doc->m_latestFixtureId, quint32(0));
    QCOMPARE(m_doc->m_latestFixtureGroupId, quint32(0));
    QCOMPARE(m_doc->m_addresses.size(), 0);
}

void Doc_Test::normalizeComponentPath()
{
     m_doc->setWorkspacePath(QDir("../../../resources/icons/png").absolutePath());

     QCOMPARE(m_doc->normalizeComponentPath(QString()), QString());
     QCOMPARE(m_doc->normalizeComponentPath("qlcplus.png"), QFileInfo("qlcplus.png").absoluteFilePath());
     QCOMPARE(m_doc->normalizeComponentPath("../../../resources/icons/png/qlcplus.png"), QString("qlcplus.png"));
     QCOMPARE(m_doc->normalizeComponentPath("../../../resources/icons/png/sub/qlcplus.png"), QString("sub/qlcplus.png"));
     QCOMPARE(m_doc->normalizeComponentPath("/home/user/test.png"), QString("/home/user/test.png"));
}

void Doc_Test::denormalizeComponentPath()
{
     m_doc->setWorkspacePath(QDir("../../../resources/icons/png").absolutePath());

     QCOMPARE(m_doc->denormalizeComponentPath(QString()), QString());
     QCOMPARE(m_doc->denormalizeComponentPath("qlcplus.png"), QFileInfo("../../../resources/icons/png/qlcplus.png").absoluteFilePath());
     QCOMPARE(m_doc->denormalizeComponentPath("sub/qlcplus.png"), QFileInfo("../../../resources/icons/png/sub/qlcplus.png").absoluteFilePath());
     QCOMPARE(m_doc->denormalizeComponentPath("/home/user/test.png"), QString("/home/user/test.png"));
}

void Doc_Test::defaults()
{
    QVERIFY(m_doc->m_fixtureDefCache != NULL);
    QVERIFY(m_doc->m_ioMap != NULL);
    QVERIFY(m_doc->m_masterTimer != NULL);

    QVERIFY(m_doc->m_modified == false);
    QVERIFY(m_doc->m_latestFixtureId == 0);
    QVERIFY(m_doc->m_fixtures.size() == 0);
    QVERIFY(m_doc->m_latestFunctionId == 0);
    QVERIFY(m_doc->m_functions.size() == 0);
    QVERIFY(m_doc->isKiosk() == false);
}

void Doc_Test::mode()
{
    QSignalSpy spy(m_doc, SIGNAL(modeChanged(Doc::Mode)));
    QCOMPARE(m_doc->mode(), Doc::Design);

    m_doc->setMode(Doc::Operate);
    QCOMPARE(spy.size(), 1);
    m_doc->setMode(Doc::Operate);
    QCOMPARE(spy.size(), 1);
    m_doc->setMode(Doc::Design);
    QCOMPARE(spy.size(), 2);
    m_doc->setMode(Doc::Design);
    QCOMPARE(spy.size(), 2);

    m_doc->setKiosk(true);
    QVERIFY(m_doc->isKiosk() == true);
}

void Doc_Test::createFixtureId()
{
    for (quint32 i = 0; i < 16384; i++)
    {
        quint32 id = m_doc->createFixtureId();
        m_doc->m_fixtures[i] = new Fixture(m_doc);
        m_doc->m_fixtures[i]->setID(id);
        QCOMPARE(id, i);
    }
}

void Doc_Test::addFixture()
{
    QVERIFY(m_doc->isModified() == false);
    QSignalSpy spy(m_doc, SIGNAL(fixtureAdded(quint32)));

    /* Add a completely new fixture */
    Fixture* f1 = new Fixture(m_doc);
    f1->setName("One");
    f1->setChannels(5);
    f1->setAddress(0);
    f1->setUniverse(0);

    QVERIFY(m_doc->addFixture(f1) == true);
    QVERIFY(f1->id() == 0);
    QVERIFY(m_doc->isModified() == true);
    QVERIFY(spy.size() == 1);
    QVERIFY(spy.at(0).at(0) == f1->id());

    m_doc->resetModified();

    /* Add another fixture but attempt to put assign it an already-assigned
       fixture ID. */
    Fixture* f2 = new Fixture(m_doc);
    f2->setName("Two");
    f2->setChannels(5);
    f2->setAddress(0);
    f2->setUniverse(0);
    QVERIFY(m_doc->addFixture(f2, f1->id()) == false);
    QVERIFY(m_doc->isModified() == false);
    QVERIFY(spy.size() == 1);
    QVERIFY(spy.at(0).at(0) == f1->id());

    /* But, the fixture can be added if we give it an unassigned ID. */
    QVERIFY(m_doc->addFixture(f2, f1->id() + 1) == true);
    QVERIFY(f1->id() == 0);
    QVERIFY(f2->id() == 1);
    QVERIFY(m_doc->isModified() == true);
    QVERIFY(spy.size() == 2);
    QVERIFY(spy.at(1).at(0) == f2->id());

    m_doc->resetModified();

    /* Add again a completely new fixture, with automatic ID assignment */
    Fixture* f3 = new Fixture(m_doc);
    f3->setName("Three");
    f3->setChannels(5);
    f3->setAddress(0);
    f3->setUniverse(0);
    QVERIFY(m_doc->addFixture(f3) == true);
    QVERIFY(f1->id() == 0);
    QVERIFY(f2->id() == 1);
    QVERIFY(f3->id() == 2);
    QVERIFY(m_doc->isModified() == true);
    QVERIFY(spy.size() == 3);
    QVERIFY(spy.at(2).at(0) == f3->id());
}

void Doc_Test::deleteFixture()
{
    QSignalSpy spy(m_doc, SIGNAL(fixtureRemoved(quint32)));

    QVERIFY(m_doc->fixtures().size() == 0);
    QVERIFY(m_doc->deleteFixture(0) == false);
    QVERIFY(m_doc->fixtures().size() == 0);
    QVERIFY(m_doc->deleteFixture(1) == false);
    QVERIFY(m_doc->fixtures().size() == 0);
    QVERIFY(m_doc->deleteFixture(Fixture::invalidId()) == false);
    QVERIFY(m_doc->fixtures().size() == 0);

    Fixture* f1 = new Fixture(m_doc);
    f1->setName("One");
    f1->setChannels(5);
    f1->setAddress(0);
    f1->setUniverse(0);
    m_doc->addFixture(f1);

    Fixture* f2 = new Fixture(m_doc);
    f2->setName("Two");
    f2->setChannels(5);
    f2->setAddress(0);
    f2->setUniverse(0);
    m_doc->addFixture(f2);

    Fixture* f3 = new Fixture(m_doc);
    f3->setName("Three");
    f3->setChannels(5);
    f3->setAddress(0);
    f3->setUniverse(0);
    m_doc->addFixture(f3);

    QVERIFY(m_doc->isModified() == true);
    m_doc->resetModified();

    // Nonexistent ID
    QVERIFY(m_doc->deleteFixture(42) == false);
    QVERIFY(m_doc->fixtures().size() == 3);
    QVERIFY(m_doc->isModified() == false);

    // Invalid ID
    QVERIFY(m_doc->deleteFixture(Fixture::invalidId()) == false);
    QVERIFY(m_doc->fixtures().size() == 3);
    QVERIFY(m_doc->isModified() == false);

    // Existing ID
    quint32 id = f2->id();
    QPointer <Fixture> f2ptr(f2);
    QVERIFY(f2ptr != NULL);
    QVERIFY(m_doc->deleteFixture(id) == true);
    QVERIFY(m_doc->fixtures().size() == 2);
    QVERIFY(m_doc->isModified() == true);
    QVERIFY(spy.size() == 1);
    QVERIFY(spy.at(0).at(0) == id);
    QVERIFY(f2ptr == NULL);

    // The same ID we just removed
    QVERIFY(m_doc->deleteFixture(id) == false);
    QVERIFY(m_doc->fixtures().size() == 2);
    QVERIFY(m_doc->isModified() == true);
    QVERIFY(spy.size() == 1);
    QVERIFY(spy.at(0).at(0) == id);

    m_doc->resetModified();

    // Another ID just for repetition
    id = f1->id();
    QPointer <Fixture> f1ptr(f1);
    QVERIFY(f1ptr != NULL);
    QVERIFY(m_doc->deleteFixture(id) == true);
    QVERIFY(m_doc->fixtures().size() == 1);
    QVERIFY(m_doc->isModified() == true);
    QVERIFY(spy.size() == 2);
    QVERIFY(spy.at(1).at(0) == id);
    QVERIFY(f1ptr == NULL);

    // And the last one...
    id = f3->id();
    QPointer <Fixture> f3ptr(f3);
    QVERIFY(f3ptr != NULL);
    QVERIFY(m_doc->deleteFixture(id) == true);
    QVERIFY(m_doc->fixtures().size() == 0);
    QVERIFY(m_doc->isModified() == true);
    QVERIFY(spy.size() == 3);
    QVERIFY(spy.at(2).at(0) == id);
    QVERIFY(f3ptr == NULL);
}

void Doc_Test::fixture()
{
    Fixture* f1 = new Fixture(m_doc);
    f1->setName("One");
    f1->setChannels(5);
    f1->setAddress(0);
    f1->setUniverse(0);
    m_doc->addFixture(f1);

    Fixture* f2 = new Fixture(m_doc);
    f2->setName("Two");
    f2->setChannels(5);
    f2->setAddress(0);
    f2->setUniverse(0);
    m_doc->addFixture(f2);

    Fixture* f3 = new Fixture(m_doc);
    f3->setName("Three");
    f3->setChannels(5);
    f3->setAddress(0);
    f3->setUniverse(0);
    m_doc->addFixture(f3);

    QVERIFY(m_doc->fixture(f1->id()) == f1);
    QVERIFY(m_doc->fixture(f2->id()) == f2);
    QVERIFY(m_doc->fixture(f3->id()) == f3);
    QVERIFY(m_doc->fixture(f3->id() + 1) == NULL);
    QVERIFY(m_doc->fixture(42) == NULL);
    QVERIFY(m_doc->fixture(Fixture::invalidId()) == NULL);
}

void Doc_Test::totalPowerConsumption()
{
    int fuzzy = 0;

    /* Load Showtec - MiniMax 250 with 250W power consumption */
    QLCFixtureDef* fixtureDef;
    fixtureDef = m_doc->fixtureDefCache()->fixtureDef("Showtec", "MiniMax 250");
    Q_ASSERT(fixtureDef != NULL);
    QLCFixtureMode* fixtureMode;
    fixtureMode = fixtureDef->modes().at(0);
    Q_ASSERT(fixtureMode != NULL);

    /* Add a new fixture */
    Fixture* f1 = new Fixture(m_doc);
    f1->setName("250W (total 250W)");
    f1->setChannels(6);
    f1->setAddress(0);
    f1->setUniverse(0);
    f1->setFixtureDefinition(fixtureDef, fixtureMode);
    QVERIFY(f1->fixtureDef() == fixtureDef);
    QVERIFY(f1->fixtureMode() == fixtureMode);
    QVERIFY(f1->fixtureMode()->physical().powerConsumption() == 250);
    QVERIFY(m_doc->addFixture(f1) == true);
    QVERIFY(m_doc->totalPowerConsumption(fuzzy) == 250);
    QVERIFY(fuzzy == 0);

    /* Add the same fixture once more */
    Fixture* f2 = new Fixture(m_doc);
    f2->setName("250W (total 500W)");
    f2->setChannels(6);
    f2->setAddress(10);
    f2->setUniverse(0);
    f2->setFixtureDefinition(fixtureDef, fixtureMode);
    QVERIFY(f2->fixtureDef() == fixtureDef);
    QVERIFY(f2->fixtureMode() == fixtureMode);
    QVERIFY(f2->fixtureMode()->physical().powerConsumption() == 250);
    QVERIFY(m_doc->addFixture(f2) == true);
    QVERIFY(m_doc->totalPowerConsumption(fuzzy) == 500);
    QVERIFY(fuzzy == 0);

    /* Test generic dimmer and fuzzy */
    Fixture* f3 = new Fixture(m_doc);
    f3->setName("Generic Dimmer");
    f3->setChannels(6);
    f3->setAddress(20);
    f3->setUniverse(0);
    QVERIFY(m_doc->addFixture(f3) == true);
    QVERIFY(m_doc->totalPowerConsumption(fuzzy) == 500);
    QVERIFY(fuzzy == 1);
    // reset fuzzy count
    fuzzy = 0;

    /* Test fuzzy count */
    Fixture* f4 = new Fixture(m_doc);
    f4->setName("Generic Dimmer 2");
    f4->setChannels(6);
    f4->setAddress(30);
    f4->setUniverse(0);
    QVERIFY(m_doc->addFixture(f4) == true);
    QVERIFY(m_doc->totalPowerConsumption(fuzzy) == 500);
    QVERIFY(fuzzy == 2);
}

void Doc_Test::addFixtureGroup()
{
    QSignalSpy spy(m_doc, SIGNAL(fixtureGroupAdded(quint32)));

    QCOMPARE(m_doc->fixtureGroups().size(), 0);
    QCOMPARE(m_doc->m_latestFixtureGroupId, quint32(0));

    FixtureGroup* grp = new FixtureGroup(m_doc);
    QCOMPARE(m_doc->addFixtureGroup(grp), true);
    QCOMPARE(grp->id(), quint32(0));
    QCOMPARE(m_doc->m_latestFixtureGroupId, quint32(0));
    QCOMPARE(m_doc->fixtureGroups().size(), 1);
    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy[0].size(), 1);
    QCOMPARE(spy[0][0].toUInt(), quint32(0));

    QCOMPARE(m_doc->addFixtureGroup(grp, 0), false);
    QCOMPARE(m_doc->fixtureGroups().size(), 1);

    grp = new FixtureGroup(m_doc);
    QCOMPARE(m_doc->addFixtureGroup(grp, 0), false);
    QCOMPARE(m_doc->addFixtureGroup(grp, 15), true);
    QCOMPARE(m_doc->m_latestFixtureGroupId, quint32(0));
    QCOMPARE(m_doc->fixtureGroups().size(), 2);
    QCOMPARE(spy.size(), 2);
    QCOMPARE(spy[1].size(), 1);
    QCOMPARE(spy[1][0].toUInt(), quint32(15));

    grp = new FixtureGroup(m_doc);
    QCOMPARE(m_doc->addFixtureGroup(grp), true);
    QCOMPARE(grp->id(), quint32(1));
    QCOMPARE(m_doc->m_latestFixtureGroupId, quint32(1));
    QCOMPARE(m_doc->fixtureGroups().size(), 3);
    QCOMPARE(spy.size(), 3);
    QCOMPARE(spy[2].size(), 1);
    QCOMPARE(spy[2][0].toUInt(), quint32(1));
}

void Doc_Test::removeFixtureGroup()
{
    QSignalSpy spy(m_doc, SIGNAL(fixtureGroupRemoved(quint32)));
    QCOMPARE(m_doc->fixtureGroups().size(), 0);
    QCOMPARE(m_doc->m_latestFixtureGroupId, quint32(0));

    QCOMPARE(m_doc->deleteFixtureGroup(0), false);
    QCOMPARE(spy.size(), 0);

    FixtureGroup* grp = new FixtureGroup(m_doc);
    QCOMPARE(m_doc->addFixtureGroup(grp), true);
    grp = new FixtureGroup(m_doc);
    QCOMPARE(m_doc->addFixtureGroup(grp), true);
    grp = new FixtureGroup(m_doc);
    QCOMPARE(m_doc->addFixtureGroup(grp), true);
    QCOMPARE(m_doc->fixtureGroups().size(), 3);
    QVERIFY(m_doc->fixtureGroup(0) != NULL);
    QVERIFY(m_doc->fixtureGroup(1) != NULL);
    QVERIFY(m_doc->fixtureGroup(2) != NULL);

    QCOMPARE(m_doc->deleteFixtureGroup(0), true);
    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy[0].size(), 1);
    QCOMPARE(spy[0][0].toUInt(), quint32(0));
    QCOMPARE(m_doc->fixtureGroups().size(), 2);
    QVERIFY(m_doc->fixtureGroup(0) == NULL);
    QVERIFY(m_doc->fixtureGroup(1) != NULL);
    QVERIFY(m_doc->fixtureGroup(2) != NULL);

    QCOMPARE(m_doc->deleteFixtureGroup(0), false);
    QCOMPARE(spy.size(), 1);

    QCOMPARE(m_doc->deleteFixtureGroup(1), true);
    QCOMPARE(spy.size(), 2);
    QCOMPARE(spy[1].size(), 1);
    QCOMPARE(spy[1][0].toUInt(), quint32(1));
    QCOMPARE(m_doc->fixtureGroups().size(), 1);
    QVERIFY(m_doc->fixtureGroup(0) == NULL);
    QVERIFY(m_doc->fixtureGroup(1) == NULL);
    QVERIFY(m_doc->fixtureGroup(2) != NULL);
}

void Doc_Test::addFunction()
{
    QVERIFY(m_doc->functions().size() == 0);

    Scene* s = new Scene(m_doc);
    QVERIFY(s->id() == Function::invalidId());
    QVERIFY(m_doc->addFunction(s) == true);
    QVERIFY(s->id() == 0);
    QVERIFY(m_doc->functions().size() == 1);
    QVERIFY(m_doc->isModified() == true);

    m_doc->resetModified();

    Chaser* c = new Chaser(m_doc);
    QVERIFY(c->id() == Function::invalidId());
    QVERIFY(m_doc->addFunction(c) == true);
    QVERIFY(c->id() == 1);
    QVERIFY(m_doc->functions().size() == 2);
    QVERIFY(m_doc->isModified() == true);

    m_doc->resetModified();

    Collection* o = new Collection(m_doc);
    QVERIFY(o->id() == Function::invalidId());
    QVERIFY(m_doc->addFunction(o, 0) == false);
    QVERIFY(m_doc->isModified() == false);
    QVERIFY(o->id() == Function::invalidId());
    QVERIFY(m_doc->functions().size() == 2);
    QVERIFY(m_doc->addFunction(o, 2) == true);
    QVERIFY(o->id() == 2);
    QVERIFY(m_doc->functions().size() == 3);
    QVERIFY(m_doc->isModified() == true);

    m_doc->resetModified();

    EFX* e = new EFX(m_doc);
    QVERIFY(e->id() == Function::invalidId());
    QVERIFY(m_doc->addFunction(e, 1) == false);
    QVERIFY(e->id() == Function::invalidId());
    QVERIFY(m_doc->addFunction(e) == true);
    QVERIFY(e->id() == 3);
    QVERIFY(m_doc->functions().size() == 4);
    QVERIFY(m_doc->isModified() == true);
}

void Doc_Test::deleteFunction()
{
    Scene* s1 = new Scene(m_doc);
    m_doc->addFunction(s1);

    Scene* s2 = new Scene(m_doc);
    m_doc->addFunction(s2);

    Scene* s3 = new Scene(m_doc);
    m_doc->addFunction(s3);

    m_doc->resetModified();

    QPointer <Scene> ptr(s2);
    QVERIFY(ptr != NULL);
    quint32 id = s2->id();
    QVERIFY(m_doc->deleteFunction(id) == true);
    QVERIFY(m_doc->isModified() == true);

    m_doc->resetModified();

    QVERIFY(m_doc->deleteFunction(id) == false);
    QVERIFY(m_doc->deleteFunction(42) == false);
    QVERIFY(m_doc->isModified() == false);
    QVERIFY(ptr == NULL); // m_doc->deleteFunction() should also delete
    QVERIFY(m_doc->m_fixtures.contains(id) == false);

    id = s1->id();
    QVERIFY(m_doc->deleteFunction(id) == true);
    QVERIFY(m_doc->m_fixtures.contains(id) == false);
    QVERIFY(m_doc->isModified() == true);

    id = s3->id();
    QVERIFY(m_doc->deleteFunction(id) == true);
    QVERIFY(m_doc->m_fixtures.contains(id) == false);
    QVERIFY(m_doc->isModified() == true);

    QVERIFY(m_doc->functions().size() == 0);
}

void Doc_Test::function()
{
    Scene* s1 = new Scene(m_doc);
    m_doc->addFunction(s1);

    Scene* s2 = new Scene(m_doc);
    m_doc->addFunction(s2);

    Scene* s3 = new Scene(m_doc);
    m_doc->addFunction(s3);

    QVERIFY(m_doc->function(s1->id()) == s1);
    QVERIFY(m_doc->function(s2->id()) == s2);
    QVERIFY(m_doc->function(s3->id()) == s3);

    quint32 id = s2->id();
    m_doc->deleteFunction(id);
    QVERIFY(m_doc->function(id) == NULL);
}

void Doc_Test::load()
{
    QDomDocument document;
    QDomElement root = document.createElement("Engine");

    root.appendChild(createFixtureNode(document, 0));
    root.appendChild(createFixtureNode(document, 72));
    root.appendChild(createFixtureNode(document, 15));

    root.appendChild(createFixtureGroupNode(document, 0));
    root.appendChild(createFixtureGroupNode(document, 42));
    root.appendChild(createFixtureGroupNode(document, 72));

    root.appendChild(createCollectionNode(document, 5));
    root.appendChild(createCollectionNode(document, 9));
    root.appendChild(createCollectionNode(document, 1));
    root.appendChild(createCollectionNode(document, 7));

    root.appendChild(createBusNode(document, 0, 1));
    root.appendChild(createBusNode(document, 7, 2));
    root.appendChild(createBusNode(document, 12, 3));
    root.appendChild(createBusNode(document, 29, 4));
    root.appendChild(createBusNode(document, 31, 500));

    root.appendChild(document.createElement("ExtraTag"));

    QVERIFY(m_doc->fixtures().size() == 0);
    QVERIFY(m_doc->functions().size() == 0);
    QVERIFY(m_doc->loadXML(root) == true);
    QVERIFY(m_doc->fixtures().size() == 3);
    QVERIFY(m_doc->functions().size() == 4);
    QVERIFY(m_doc->fixtureGroups().size() == 3);
    QVERIFY(Bus::instance()->value(0) == 1);
    QVERIFY(Bus::instance()->value(7) == 2);
    QVERIFY(Bus::instance()->value(12) == 3);
    QVERIFY(Bus::instance()->value(29) == 4);
    QVERIFY(Bus::instance()->value(31) == 500);
}

void Doc_Test::loadWrongRoot()
{
    QDomDocument document;
    QDomElement root = document.createElement("Enjine");

    root.appendChild(createFixtureNode(document, 0));
    root.appendChild(createFixtureNode(document, 72));
    root.appendChild(createFixtureNode(document, 15));

    root.appendChild(createFixtureGroupNode(document, 0));
    root.appendChild(createFixtureGroupNode(document, 42));
    root.appendChild(createFixtureGroupNode(document, 72));

    root.appendChild(createCollectionNode(document, 5));
    root.appendChild(createCollectionNode(document, 9));
    root.appendChild(createCollectionNode(document, 1));
    root.appendChild(createCollectionNode(document, 7));

    root.appendChild(createBusNode(document, 0, 1));
    root.appendChild(createBusNode(document, 7, 2));
    root.appendChild(createBusNode(document, 12, 3));
    root.appendChild(createBusNode(document, 29, 4));
    root.appendChild(createBusNode(document, 31, 500));

    root.appendChild(document.createElement("ExtraTag"));

    QVERIFY(m_doc->loadXML(root) == false);
}

void Doc_Test::save()
{
    Scene* s = new Scene(m_doc);
    m_doc->addFunction(s);

    Fixture* f1 = new Fixture(m_doc);
    f1->setName("One");
    f1->setChannels(5);
    f1->setAddress(0);
    f1->setUniverse(0);
    m_doc->addFixture(f1);

    Chaser* c = new Chaser(m_doc);
    m_doc->addFunction(c);

    Fixture* f2 = new Fixture(m_doc);
    f2->setName("Two");
    f2->setChannels(10);
    f2->setAddress(20);
    f2->setUniverse(1);
    m_doc->addFixture(f2);

    Collection* o = new Collection(m_doc);
    m_doc->addFunction(o);

    Fixture* f3 = new Fixture(m_doc);
    f3->setName("Three");
    f3->setChannels(15);
    f3->setAddress(40);
    f3->setUniverse(2);
    m_doc->addFixture(f3);

    EFX* e = new EFX(m_doc);
    m_doc->addFunction(e);

    FixtureGroup* grp = new FixtureGroup(m_doc);
    grp->setName("Group 1");
    m_doc->addFixtureGroup(grp);

    grp = new FixtureGroup(m_doc);
    grp->setName("Group 2");
    m_doc->addFixtureGroup(grp);

    QVERIFY(m_doc->isModified() == true);

    QDomDocument document;
    QDomElement root = document.createElement("TestRoot");

    QVERIFY(m_doc->saveXML(&document, &root) == true);

    uint fixtures = 0, groups = 0, functions = 0, ioMap = 0;
    QDomNode node = root.firstChild();
    QVERIFY(node.toElement().tagName() == "Engine");

    // Merely tests that the start of each hierarchy is found from the XML document.
    // Their contents are tested individually in their own separate tests.
    node = node.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == "Fixture")
            fixtures++;
        else if (tag.tagName() == "Function")
            functions++;
        else if (tag.tagName() == "FixtureGroup")
            groups++;
        else if (tag.tagName() == "InputOutputMap")
            ioMap++;
        else if (tag.tagName() == "Bus")
            QFAIL("Bus tags should not be saved anymore!");
        else
            QFAIL(QString("Unexpected tag: %1")
                  .arg(tag.tagName()).toLatin1());

        node = node.nextSibling();
    }

    QVERIFY(fixtures == 3);
    QVERIFY(groups == 2);
    QVERIFY(functions == 4);
    QVERIFY(ioMap == 1);

    /* Saving doesn't implicitly reset modified status */
    QVERIFY(m_doc->isModified() == true);
}

QDomElement Doc_Test::createFixtureNode(QDomDocument& doc, quint32 id)
{
    QDomElement root = doc.createElement("Fixture");
    doc.appendChild(root);

    QDomElement chs = doc.createElement("Channels");
    QDomText chsText = doc.createTextNode("18");
    chs.appendChild(chsText);
    root.appendChild(chs);

    QDomElement name = doc.createElement("Name");
    QDomText nameText = doc.createTextNode(QString("Fixture %1").arg(id));
    name.appendChild(nameText);
    root.appendChild(name);

    QDomElement uni = doc.createElement("Universe");
    QDomText uniText = doc.createTextNode("3");
    uni.appendChild(uniText);
    root.appendChild(uni);

    QDomElement model = doc.createElement("Model");
    QDomText modelText = doc.createTextNode("Foobar");
    model.appendChild(modelText);
    root.appendChild(model);

    QDomElement mode = doc.createElement("Mode");
    QDomText modeText = doc.createTextNode("Foobar");
    mode.appendChild(modeText);
    root.appendChild(mode);

    QDomElement type = doc.createElement("Manufacturer");
    QDomText typeText = doc.createTextNode("Foobar");
    type.appendChild(typeText);
    root.appendChild(type);

    QDomElement fxi_id = doc.createElement("ID");
    QDomText fxi_idText = doc.createTextNode(QString("%1").arg(id));
    fxi_id.appendChild(fxi_idText);
    root.appendChild(fxi_id);

    QDomElement addr = doc.createElement("Address");
    QDomText addrText = doc.createTextNode("21");
    addr.appendChild(addrText);
    root.appendChild(addr);

    return root;
}

QDomElement Doc_Test::createFixtureGroupNode(QDomDocument& doc, quint32 id)
{
    QDomElement root = doc.createElement("FixtureGroup");
    root.setAttribute("ID", id);
    doc.appendChild(root);

    QDomElement name = doc.createElement("Name");
    QDomText nameText = doc.createTextNode(QString("Group with ID %1").arg(id));
    name.appendChild(nameText);
    root.appendChild(name);

    return root;
}

QDomElement Doc_Test::createCollectionNode(QDomDocument& doc, quint32 id)
{
    QDomElement root = doc.createElement("Function");
    root.setAttribute("Type", "Collection");
    root.setAttribute("ID", QString("%1").arg(id));

    QDomElement s1 = doc.createElement("Step");
    QDomText s1Text = doc.createTextNode("50");
    s1.appendChild(s1Text);
    root.appendChild(s1);

    QDomElement s2 = doc.createElement("Step");
    QDomText s2Text = doc.createTextNode("12");
    s2.appendChild(s2Text);
    root.appendChild(s2);

    QDomElement s3 = doc.createElement("Step");
    QDomText s3Text = doc.createTextNode("87");
    s3.appendChild(s3Text);
    root.appendChild(s3);

    return root;
}

QDomElement Doc_Test::createBusNode(QDomDocument& doc, quint32 id, quint32 val)
{
    // Used to test that loading legacy Bus tags won't screw up Doc
    QDomElement root = doc.createElement("Bus");
    doc.appendChild(root);
    root.setAttribute("ID", id);

    QDomElement name = doc.createElement("Name");
    QDomText nameText = doc.createTextNode(QString("Bus %1").arg(id));
    name.appendChild(nameText);
    root.appendChild(name);

    QDomElement value = doc.createElement("Value");
    QDomText valueText = doc.createTextNode(QString("%1").arg(val));
    value.appendChild(valueText);
    root.appendChild(value);

    return root;
}

QTEST_APPLESS_MAIN(Doc_Test)
