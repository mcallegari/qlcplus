/*
  Q Light Controller Plus - Unit test
  fixtureremap_test.cpp

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

#include <QtTest>

#include "fixtureremap_test.h"
#include "qlcfixturedefcache.h"
#include "qlcfixturedef.h"
#include "qlcfixturemode.h"
#include "fixtureremap.h"
#include "qlcfile.h"
#include "fixture.h"
#include "scene.h"
#include "doc.h"

#include "../common/resource_paths.h"

void FixtureRemap_Test::initTestCase()
{
    m_doc = new Doc(this);

    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_doc->fixtureDefCache()->loadMap(dir) == true);
}

void FixtureRemap_Test::cleanupTestCase()
{
    delete m_doc;
}

void FixtureRemap_Test::replaceProfiles()
{
    // 1. Create a source fixture (Generic RGB)
    QLCFixtureDef *rgbDef = m_doc->fixtureDefCache()->fixtureDef("Generic", "Generic RGB");
    QVERIFY(rgbDef != nullptr);
    QLCFixtureMode *rgbMode = rgbDef->modes().first();
    QVERIFY(rgbMode != nullptr);

    Fixture *fxi = new Fixture(m_doc);
    fxi->setFixtureDefinition(rgbDef, rgbMode);
    m_doc->addFixture(fxi);
    quint32 fid = fxi->id();

    // 2. Create a scene using this fixture
    Scene *s = new Scene(m_doc);
    s->setValue(fid, 0, 255); // Red
    s->setValue(fid, 1, 128); // Green
    s->setValue(fid, 2, 64);  // Blue
    m_doc->addFunction(s);

    // 3. Remap to Generic RGBW
    QLCFixtureDef *rgbwDef = m_doc->fixtureDefCache()->fixtureDef("Generic", "Generic RGBW");
    QVERIFY(rgbwDef != nullptr);
    QLCFixtureMode *rgbwMode = rgbwDef->modes().first();

    FixtureRemap remap(m_doc);
    QMap<SceneValue, SceneValue> res = remap.replaceProfiles(QList<quint32>() << fid, "Generic", "Generic RGBW", rgbwMode->name());

    // Generic RGB has Red(0), Green(1), Blue(2).
    // Generic RGBW has Red(0), Green(1), Blue(2), White(3).
    // Our logic matches by Group AND Color.
    // So Red(0)->Red(0), Green(1)->Green(1), Blue(2)->Blue(2).

    // Let's verify results.
    QCOMPARE(fxi->fixtureDef()->model(), QString("Generic RGBW"));

    // Scene values for RGB should be preserved and remapped
    QCOMPARE(s->values().count(), 3);
    QCOMPARE(s->values().at(0).value, (uchar)255);
    QCOMPARE(s->values().at(0).channel, (quint32)0); // Red
    QCOMPARE(s->values().at(1).value, (uchar)128);
    QCOMPARE(s->values().at(1).channel, (quint32)1); // Green
    QCOMPARE(s->values().at(2).value, (uchar)64);
    QCOMPARE(s->values().at(2).channel, (quint32)2); // Blue

    // Now let's try RGB to RGB (same definition)
    fxi->setFixtureDefinition(rgbDef, rgbMode);
    s->clear();
    s->setValue(fid, 0, 200);
    QCOMPARE(s->values().count(), 1);

    res = remap.replaceProfiles(QList<quint32>() << fid, "Generic", "Generic RGB", rgbMode->name());
    QCOMPARE(s->values().count(), 1);
    QCOMPARE(s->values().first().value, (uchar)200);
    QCOMPARE(s->values().first().channel, (quint32)0);
}

QTEST_MAIN(FixtureRemap_Test)
