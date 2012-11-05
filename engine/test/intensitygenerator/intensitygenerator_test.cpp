/*
  Q Light Controller
  intensitygenerator_test.cpp

  Copyright (C) Heikki Junnila

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

#include <QtTest>

#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "qlcfile.h"
#include "fixture.h"
#include "scene.h"
#include "doc.h"

#include "intensitygenerator_test.h"
#include "intensitygenerator.h"

#define INTERNAL_FIXTUREDIR "../../../fixtures/"

void IntensityGenerator_Test::initTestCase()
{
    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_fixtureDefCache.load(dir) == true);
}

void IntensityGenerator_Test::randomScenes()
{
    Doc doc(this);
    QList <Fixture*> fixtures;

    Fixture* fxi = new Fixture(&doc);
    fxi->setChannels(4);
    fixtures << fxi;

    Fixture* fxi2 = new Fixture(&doc);
    const QLCFixtureDef* def = m_fixtureDefCache.fixtureDef("Futurelight", "DJScan250");
    Q_ASSERT(def != NULL);
    Q_ASSERT(def->modes().first() != NULL);
    Q_ASSERT(def->modes().first()->channels().size() == 6);
    fxi2->setFixtureDefinition(def, def->modes().first());
    fixtures << fxi2;

    QList <Scene*> scenes = IntensityGenerator::randomScenes(fixtures, &doc, 31337);
    QCOMPARE(scenes.size(), 5);

    // Even though injecting the random seed to randomScenes() allows deterministic behaviour,
    // the rand() algorithm implementation varies between platforms and platform versions.
    // So let's not check the randomized contents at all.
}

void IntensityGenerator_Test::sequenceScenes()
{
    Doc doc(this);
    QList <Fixture*> fixtures;

    Fixture* fxi = new Fixture(&doc);
    fxi->setChannels(4);
    fixtures << fxi;

    Fixture* fxi2 = new Fixture(&doc);
    const QLCFixtureDef* def = m_fixtureDefCache.fixtureDef("Futurelight", "DJScan250");
    Q_ASSERT(def != NULL);
    Q_ASSERT(def->modes().first() != NULL);
    Q_ASSERT(def->modes().first()->channels().size() == 6);
    fxi2->setFixtureDefinition(def, def->modes().first());
    fixtures << fxi2;

    QList <Scene*> scenes = IntensityGenerator::sequenceScenes(fixtures, &doc);
    QCOMPARE(scenes.size(), 5);

    QCOMPARE(scenes[0]->values().size(), 1);
    QCOMPARE(scenes[0]->values()[0].fxi, fxi->id());
    QCOMPARE(scenes[0]->values()[0].channel, quint32(0));
    QCOMPARE(scenes[0]->values()[0].value, uchar(UCHAR_MAX));

    QCOMPARE(scenes[1]->values().size(), 1);
    QCOMPARE(scenes[1]->values()[0].fxi, fxi->id());
    QCOMPARE(scenes[1]->values()[0].channel, quint32(1));
    QCOMPARE(scenes[1]->values()[0].value, uchar(UCHAR_MAX));

    QCOMPARE(scenes[2]->values().size(), 1);
    QCOMPARE(scenes[2]->values()[0].fxi, fxi->id());
    QCOMPARE(scenes[2]->values()[0].channel, quint32(2));
    QCOMPARE(scenes[2]->values()[0].value, uchar(UCHAR_MAX));

    QCOMPARE(scenes[3]->values().size(), 1);
    QCOMPARE(scenes[3]->values()[0].fxi, fxi->id());
    QCOMPARE(scenes[3]->values()[0].channel, quint32(3));
    QCOMPARE(scenes[3]->values()[0].value, uchar(UCHAR_MAX));

    QCOMPARE(scenes[4]->values().size(), 1);
    QCOMPARE(scenes[4]->values()[0].fxi, fxi->id());
    QCOMPARE(scenes[4]->values()[0].channel, quint32(5));
    QCOMPARE(scenes[4]->values()[0].value, uchar(128));
}

void IntensityGenerator_Test::fullZeroScenes()
{
    Doc doc(this);

    QList <Fixture*> fixtures;

    Fixture* fxi = new Fixture(&doc);
    fxi->setChannels(4);
    fixtures << fxi;

    Fixture* fxi2 = new Fixture(&doc);
    const QLCFixtureDef* def = m_fixtureDefCache.fixtureDef("Futurelight", "DJScan250");
    Q_ASSERT(def != NULL);
    Q_ASSERT(def->modes().first() != NULL);
    Q_ASSERT(def->modes().first()->channels().size() == 6);
    fxi2->setFixtureDefinition(def, def->modes().first());
    fixtures << fxi2;

    QList <Scene*> scenes = IntensityGenerator::fullZeroScenes(fixtures, &doc);
    QCOMPARE(scenes.size(), 2);

    QCOMPARE(scenes[1]->values().size(), 0);
    QCOMPARE(scenes[0]->values().size(), 5);
    QCOMPARE(scenes[0]->values()[0].fxi, fxi->id());
    QCOMPARE(scenes[0]->values()[0].channel, quint32(0));
    QCOMPARE(scenes[0]->values()[0].value, uchar(UCHAR_MAX));
    QCOMPARE(scenes[0]->values()[1].fxi, fxi->id());
    QCOMPARE(scenes[0]->values()[1].channel, quint32(1));
    QCOMPARE(scenes[0]->values()[1].value, uchar(UCHAR_MAX));
    QCOMPARE(scenes[0]->values()[2].fxi, fxi->id());
    QCOMPARE(scenes[0]->values()[2].channel, quint32(2));
    QCOMPARE(scenes[0]->values()[2].value, uchar(UCHAR_MAX));
    QCOMPARE(scenes[0]->values()[3].fxi, fxi->id());
    QCOMPARE(scenes[0]->values()[3].channel, quint32(3));
    QCOMPARE(scenes[0]->values()[3].value, uchar(UCHAR_MAX));
    QCOMPARE(scenes[0]->values()[4].fxi, fxi2->id());
    QCOMPARE(scenes[0]->values()[4].channel, quint32(5));
    QCOMPARE(scenes[0]->values()[4].value, uchar(128));

    QCOMPARE(scenes[1]->values().size(), 0);
    // Zero scene needs no values because non-participating HTP channels are faded
    // automatically to zero
}

void IntensityGenerator_Test::evenOddScenes()
{
    Doc doc(this);

    QList <Fixture*> fixtures;

    Fixture* fxi = new Fixture(&doc);
    fxi->setChannels(4);
    fixtures << fxi;

    Fixture* fxi2 = new Fixture(&doc);
    const QLCFixtureDef* def = m_fixtureDefCache.fixtureDef("Futurelight", "DJScan250");
    Q_ASSERT(def != NULL);
    Q_ASSERT(def->modes().first() != NULL);
    Q_ASSERT(def->modes().first()->channels().size() == 6);
    fxi2->setFixtureDefinition(def, def->modes().first());
    fixtures << fxi2;

    QList <Scene*> scenes = IntensityGenerator::evenOddScenes(fixtures, &doc);
    QCOMPARE(scenes.size(), 2);

    QCOMPARE(scenes[0]->values().size(), 2);
    QCOMPARE(scenes[0]->values()[0].fxi, fxi->id());
    QCOMPARE(scenes[0]->values()[0].channel, quint32(1));
    QCOMPARE(scenes[0]->values()[0].value, uchar(UCHAR_MAX));
    QCOMPARE(scenes[0]->values()[1].fxi, fxi->id());
    QCOMPARE(scenes[0]->values()[1].channel, quint32(3));
    QCOMPARE(scenes[0]->values()[1].value, uchar(UCHAR_MAX));

    QCOMPARE(scenes[1]->values().size(), 3);
    QCOMPARE(scenes[1]->values()[0].fxi, fxi->id());
    QCOMPARE(scenes[1]->values()[0].channel, quint32(0));
    QCOMPARE(scenes[1]->values()[0].value, uchar(UCHAR_MAX));
    QCOMPARE(scenes[1]->values()[1].fxi, fxi->id());
    QCOMPARE(scenes[1]->values()[1].channel, quint32(2));
    QCOMPARE(scenes[1]->values()[1].value, uchar(UCHAR_MAX));
    QCOMPARE(scenes[1]->values()[2].fxi, fxi2->id());
    QCOMPARE(scenes[1]->values()[2].channel, quint32(5));
    QCOMPARE(scenes[1]->values()[2].value, uchar(128));
}

QTEST_APPLESS_MAIN(IntensityGenerator_Test)
