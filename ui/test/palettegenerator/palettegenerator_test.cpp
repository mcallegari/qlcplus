/*
  Q Light Controller - Unit test
  palettegenerator_test.cpp

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

#define protected public
#define private public

#include "palettegenerator_test.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "fixture.h"
#include "scene.h"
#include "doc.h"

#include "palettegenerator.h"
#include "qlcchannel.h"
#include "qlcfile.h"

#undef private
#undef protected

#include "../../../engine/test/common/resource_paths.h"

void PaletteGenerator_Test::initTestCase()
{
    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_fixtureDefCache.load(dir) == true);
}

void PaletteGenerator_Test::initial()
{
    Doc doc(this);
    QList <Fixture*> list;
    list << new Fixture(&doc);
    list << new Fixture(&doc);
    list << new Fixture(&doc);

    PaletteGenerator pg(&doc, list);
    QCOMPARE(pg.m_doc, &doc);
    QCOMPARE(pg.m_fixtures.size(), 3);
    QCOMPARE(pg.m_scenes.size(), 0);
}

void PaletteGenerator_Test::capabilities()
{
    Doc doc(this);

    QLCFixtureDef* fixtureDef;
    fixtureDef = m_fixtureDefCache.fixtureDef("Showtec", "MiniMax 250");
    Q_ASSERT(fixtureDef != NULL);
    QLCFixtureMode* fixtureMode;
    fixtureMode = fixtureDef->modes().at(0);
    Q_ASSERT(fixtureMode != NULL);

    Fixture* fxi = new Fixture(&doc);
    fxi->setFixtureDefinition(fixtureDef, fixtureMode);

    QStringList caps = PaletteGenerator::getCapabilities(fxi);
    QCOMPARE(caps.count(), 3);
    QCOMPARE(caps.at(0), QLCChannel::groupToString(QLCChannel::Colour));
    QCOMPARE(caps.at(1), QLCChannel::groupToString(QLCChannel::Gobo));
    QCOMPARE(caps.at(2), KQLCChannelMovement);

    fixtureDef = m_fixtureDefCache.fixtureDef("Martin", "MAC300");
    Q_ASSERT(fixtureDef != NULL);
    fixtureMode = fixtureDef->modes().at(0);
    Q_ASSERT(fixtureMode != NULL);

    fxi->setFixtureDefinition(fixtureDef, fixtureMode);

    QStringList caps2 = PaletteGenerator::getCapabilities(fxi);
    QCOMPARE(caps2.count(), 4);
    QCOMPARE(caps2.at(0), QLCChannel::groupToString(QLCChannel::Shutter));
    QCOMPARE(caps2.at(1), QLCChannel::groupToString(QLCChannel::Colour));
    QCOMPARE(caps2.at(2), KQLCChannelMovement);
    QCOMPARE(caps2.at(3), KQLCChannelCMY);
}

void PaletteGenerator_Test::createColours()
{
    Doc doc(this);

    QLCFixtureDef* fixtureDef;
    fixtureDef = m_fixtureDefCache.fixtureDef("Futurelight", "DJScan250");
    Q_ASSERT(fixtureDef != NULL);
    QLCFixtureMode* fixtureMode;
    fixtureMode = fixtureDef->modes().at(0);
    Q_ASSERT(fixtureMode != NULL);

    QList <Fixture*> list;
    Fixture* fxi1 = new Fixture(&doc);
    fxi1->setFixtureDefinition(fixtureDef, fixtureMode);
    list << fxi1;
    doc.addFixture(fxi1);

    Fixture* fxi2 = new Fixture(&doc);
    fxi2->setFixtureDefinition(fixtureDef, fixtureMode);
    list << fxi2;
    doc.addFixture(fxi2);

    PaletteGenerator pg(&doc, list, PaletteGenerator::ColourMacro);
    pg.addToDoc();
    QCOMPARE(doc.functions().size(), 11); // 10 colours
    for (quint32 i = 0; i < 10; i++)
    {
        Scene* s = qobject_cast<Scene*> (doc.function(i));
        QVERIFY(s != NULL);
        QCOMPARE(s->values().size(), 2); // One colour for two fixtures
        QCOMPARE(s->values().at(0).fxi, fxi1->id());
        QCOMPARE(s->values().at(0).channel, quint32(2)); // DJScan colour channel
        QCOMPARE(s->values().at(1).fxi, fxi2->id());
        QCOMPARE(s->values().at(1).channel, quint32(2)); // DJScan colour channel
    }
}

void PaletteGenerator_Test::createGobos()
{
    Doc doc(this);

    QLCFixtureDef* fixtureDef;
    fixtureDef = m_fixtureDefCache.fixtureDef("Futurelight", "DJScan250");
    Q_ASSERT(fixtureDef != NULL);
    QLCFixtureMode* fixtureMode;
    fixtureMode = fixtureDef->modes().at(0);
    Q_ASSERT(fixtureMode != NULL);

    QList <Fixture*> list;
    Fixture* fxi1 = new Fixture(&doc);
    fxi1->setFixtureDefinition(fixtureDef, fixtureMode);
    list << fxi1;
    doc.addFixture(fxi1);

    Fixture* fxi2 = new Fixture(&doc);
    fxi2->setFixtureDefinition(fixtureDef, fixtureMode);
    list << fxi2;
    doc.addFixture(fxi2);

    PaletteGenerator pg(&doc, list, PaletteGenerator::Gobos);
    pg.addToDoc();
    QCOMPARE(doc.functions().size(), 11); // 10 "gobos"
    for (quint32 i = 0; i < 10; i++)
    {
        Scene* s = qobject_cast<Scene*> (doc.function(i));
        QVERIFY(s != NULL);
        QCOMPARE(s->values().size(), 2); // One gobo for two fixtures
        QCOMPARE(s->values().at(0).fxi, fxi1->id());
        QCOMPARE(s->values().at(0).channel, quint32(3)); // DJScan gobo channel
        QCOMPARE(s->values().at(1).fxi, fxi2->id());
        QCOMPARE(s->values().at(1).channel, quint32(3)); // DJScan gobo channel
    }
}

void PaletteGenerator_Test::createShutters()
{
    Doc doc(this);

    QLCFixtureDef* fixtureDef;
    fixtureDef = m_fixtureDefCache.fixtureDef("Martin", "MAC300");
    Q_ASSERT(fixtureDef != NULL);
    QLCFixtureMode* fixtureMode;
    fixtureMode = fixtureDef->modes().at(0);
    Q_ASSERT(fixtureMode != NULL);

    QList <Fixture*> list;
    Fixture* fxi1 = new Fixture(&doc);
    fxi1->setFixtureDefinition(fixtureDef, fixtureMode);
    list << fxi1;
    doc.addFixture(fxi1);

    Fixture* fxi2 = new Fixture(&doc);
    fxi2->setFixtureDefinition(fixtureDef, fixtureMode);
    list << fxi2;
    doc.addFixture(fxi2);

    PaletteGenerator pg(&doc, list, PaletteGenerator::Shutter);
    pg.addToDoc();
    // There are 19 shutter capabilities, but "Shutter open" happens with multiple values
    QCOMPARE(doc.functions().size(), 14);
    for (quint32 i = 0; i < 13; i++)
    {
        Scene* s = qobject_cast<Scene*> (doc.function(i));
        QVERIFY(s != NULL);
        QCOMPARE(s->values().size(), 2); // One cap for two fixtures
        QCOMPARE(s->values().at(0).fxi, fxi1->id());
        QCOMPARE(s->values().at(0).channel, quint32(0)); // MAC300 shutter channel
        QCOMPARE(s->values().at(1).fxi, fxi2->id());
        QCOMPARE(s->values().at(1).channel, quint32(0)); // MAC300 shutter channel
    }
}

QTEST_APPLESS_MAIN(PaletteGenerator_Test)
