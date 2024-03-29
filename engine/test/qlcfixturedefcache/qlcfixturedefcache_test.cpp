/*
  Q Light Controller - Unit tests
  qlcfixturedefcache_test.cpp

  Copyright (C) Heikki Junnila

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

#define private public
#include "qlcfixturedefcache.h"
#undef private

#include "qlcfixturedefcache_test.h"
#include "qlcfixturedef.h"
#include "qlcconfig.h"
#include "qlcfile.h"

#include "../common/resource_paths.h"

void QLCFixtureDefCache_Test::init()
{
    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(cache.load(QDir("/just/kidding/stoopid")) == false);
    QVERIFY(cache.loadMap(dir) == true);
}

void QLCFixtureDefCache_Test::cleanup()
{
    cache.clear();
}

void QLCFixtureDefCache_Test::duplicates()
{
    // Check that duplicates are discarded
    int num = cache.m_defs.size();
    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    cache.load(dir);
    QCOMPARE(cache.m_defs.size(), num);
}

void QLCFixtureDefCache_Test::add()
{
    QVERIFY(cache.fixtureCache().isEmpty() == false);
    QVERIFY(cache.fixtureCache()["Martin"]["MAC250"] == false);

    QVERIFY(cache.addFixtureDef(NULL) == false);

    QVERIFY(cache.manufacturers().count() != 0);
    cache.clear();
    QVERIFY(cache.manufacturers().count() == 0);

    /* Add the first fixtureDef */
    QLCFixtureDef* def = new QLCFixtureDef();
    def->setManufacturer("Martin");
    def->setModel("MAC250");

    QVERIFY(cache.addFixtureDef(def) == true);
    QVERIFY(cache.manufacturers().count() == 1);
    QVERIFY(cache.manufacturers().contains("Martin") == true);
    QVERIFY(cache.manufacturers().contains("MAC250") == false);

    QVERIFY(cache.models("Martin").count() == 1);
    QVERIFY(cache.models("Martin").contains("MAC250") == true);
    QVERIFY(cache.models("Foo").count() == 0);

    /* Another fixtureDef, same manufacturer & model. Should be ignored. */
    QLCFixtureDef* def2 = new QLCFixtureDef();
    def2->setManufacturer("Martin");
    def2->setModel("MAC250");

    QVERIFY(cache.addFixtureDef(def2) == false);
    QVERIFY(cache.manufacturers().count() == 1);
    QVERIFY(cache.manufacturers().contains("Martin") == true);

    delete def2;
    def2 = NULL;

    /* Another fixtureDef, same manufacturer, different model */
    def2 = new QLCFixtureDef();
    def2->setManufacturer("Martin");
    def2->setModel("MAC500");

    QVERIFY(cache.addFixtureDef(def2) == true);
    QVERIFY(cache.manufacturers().count() == 1);
    QVERIFY(cache.manufacturers().contains("Martin") == true);
    QVERIFY(cache.manufacturers().contains("MAC500") == false);

    QVERIFY(cache.models("Martin").count() == 2);
    QVERIFY(cache.models("Martin").contains("MAC250") == true);
    QVERIFY(cache.models("Martin").contains("MAC500") == true);

    /* Another fixtureDef, different manufacturer, different model */
    QLCFixtureDef* def3 = new QLCFixtureDef();
    def3->setManufacturer("Futurelight");
    def3->setModel("PHS700");

    QVERIFY(cache.addFixtureDef(def3) == true);
    QVERIFY(cache.manufacturers().count() == 2);
    QVERIFY(cache.manufacturers().contains("Martin") == true);
    QVERIFY(cache.manufacturers().contains("Futurelight") == true);
    QVERIFY(cache.manufacturers().contains("PHS700") == false);

    /* Another fixtureDef, different manufacturer, same model */
    QLCFixtureDef* def4 = new QLCFixtureDef();
    def4->setManufacturer("Yoyodyne");
    def4->setModel("MAC250");

    QVERIFY(cache.addFixtureDef(def4) == true);
    QVERIFY(cache.manufacturers().count() == 3);
    QVERIFY(cache.manufacturers().contains("Martin") == true);
    QVERIFY(cache.manufacturers().contains("Futurelight") == true);
    QVERIFY(cache.manufacturers().contains("Yoyodyne") == true);
    QVERIFY(cache.manufacturers().contains("MAC250") == false);

    QVERIFY(cache.models("Yoyodyne").count() == 1);
    QVERIFY(cache.models("Yoyodyne").contains("MAC250") == true);
}

void QLCFixtureDefCache_Test::reload()
{
    QLCFixtureDef *def = cache.fixtureDef("Botex", "SP-1500");
    QLCChannel *channel = def->channel("Control");

    QVERIFY(def->channels().count() == 5);
    def->removeChannel(channel);
    QVERIFY(def->channels().count() == 4);

    cache.reloadFixtureDef(def);
    QVERIFY(def->channels().count() == 5);
}

void QLCFixtureDefCache_Test::fixtureDef()
{
    // check the content of a cached fixture relative path
    QString firstManufacturer = cache.m_defs.first()->manufacturer();
    QString firstModel = cache.m_defs.first()->model();
    QString relPath = QString("%1/%1-%2.qxf").arg(firstManufacturer).arg(firstModel);
    relPath.replace(" ", "-");
    QVERIFY(cache.m_defs.first()->definitionSourceFile() == relPath);

    // request a fixture cached but not yet loaded
    QLCFixtureDef *def = cache.fixtureDef("Futurelight", "CY-200");

    // check that once loaded, the relative path becomes absolute
    QDir absDir(def->definitionSourceFile());
    QVERIFY(absDir.isAbsolute() == true);

    cache.clear();

    QLCFixtureDef *def1 = new QLCFixtureDef();
    def1->setManufacturer("Martin");
    def1->setModel("MAC250");
    cache.addFixtureDef(def1);

    QLCFixtureDef *def2 = new QLCFixtureDef();
    def2->setManufacturer("Martin");
    def2->setModel("MAC500");
    cache.addFixtureDef(def2);

    QLCFixtureDef *def3 = new QLCFixtureDef();
    def3->setManufacturer("Robe");
    def3->setModel("WL250");
    cache.addFixtureDef(def3);

    QLCFixtureDef *def4 = new QLCFixtureDef();
    def4->setManufacturer("Futurelight");
    def4->setModel("DJ Scan 250");
    cache.addFixtureDef(def4);

    QVERIFY(cache.fixtureDef("Martin", "MAC250") == def1);
    QVERIFY(cache.fixtureDef("Martin", "MAC500") == def2);
    QVERIFY(cache.fixtureDef("Robe", "WL250") == def3);
    QVERIFY(cache.fixtureDef("Futurelight", "DJ Scan 250") == def4);
    QVERIFY(cache.fixtureDef("Martin", "MAC 250") == NULL);
    QVERIFY(cache.fixtureDef("Mar tin", "MAC250") == NULL);
    QVERIFY(cache.fixtureDef("Foobar", "Foobar") == NULL);
    QVERIFY(cache.fixtureDef("", "") == NULL);
}

void QLCFixtureDefCache_Test::load()
{
    /* At least these should be available */
    QVERIFY(cache.manufacturers().contains("Elation") == true);
    QVERIFY(cache.manufacturers().contains("Eurolite") == true);
    QVERIFY(cache.manufacturers().contains("Futurelight") == true);
    QVERIFY(cache.manufacturers().contains("GLP") == true);
    QVERIFY(cache.manufacturers().contains("JB-Lighting") == true);
    QVERIFY(cache.manufacturers().contains("Lite-Works") == true);
    QVERIFY(cache.manufacturers().contains("Martin") == true);
    QVERIFY(cache.manufacturers().contains("Robe") == true);
    QVERIFY(cache.manufacturers().contains("SGM") == true);

    QString loadPath = QString("%1/Futurelight/Futurelight-CY-200.qxf").arg(INTERNAL_FIXTUREDIR);
    QVERIFY(cache.loadQXF(loadPath) == true);
    QVERIFY(cache.loadQXF("Foo/Baz.qxf") == false);
    QVERIFY(cache.loadD4("QLC/Plus.d4") == false);
}

void QLCFixtureDefCache_Test::defDirectories()
{
    QDir dir = QLCFixtureDefCache::systemDefinitionDirectory();

    QVERIFY(dir.filter() & QDir::Files);
    QVERIFY(dir.nameFilters().contains(QString("*%1").arg(KExtFixture)));
    QDir fxDir;
    fxDir.setPath(FIXTUREDIR);
    QCOMPARE(dir.absolutePath(), fxDir.absolutePath());

    dir = QLCFixtureDefCache::userDefinitionDirectory();
#ifndef SKIP_TEST
    QVERIFY(dir.exists() == true);
#endif
    QVERIFY(dir.filter() & QDir::Files);
    QVERIFY(dir.nameFilters().contains(QString("*%1").arg(KExtFixture)));
    QVERIFY(dir.absolutePath().contains(USERFIXTUREDIR));

}

void QLCFixtureDefCache_Test::storeDef()
{
    QLCFixtureDef *def = cache.fixtureDef("Futurelight", "CY-200");
    QFile defFile(def->definitionSourceFile());
    defFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QString defBuffer = defFile.readAll();
    defFile.close();
    QVERIFY(cache.storeFixtureDef("storeTest.qxf", defBuffer) == true);

    QDir dir = QLCFixtureDefCache::userDefinitionDirectory();
    QFile file (dir.absoluteFilePath("storeTest.qxf"));
    file.remove();
}

QTEST_APPLESS_MAIN(QLCFixtureDefCache_Test)
