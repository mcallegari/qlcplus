/*
  Q Light Controller Plus - Unit test
  fixture_test.cpp

  Copyright (c) Heikki Junnila
                Massimo Callegari

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
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "qlcfixturedefcache.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "qlccapability.h"
#include "qlcconfig.h"
#include "qlcfile.h"

#include "fixture_test.h"
#include "fixture.h"
#include "doc.h"

#include "../common/resource_paths.h"

void Fixture_Test::initTestCase()
{
    m_doc = new Doc(this);

    QDir dir(INTERNAL_FIXTUREDIR);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtFixture));
    QVERIFY(m_doc->fixtureDefCache()->loadMap(dir) == true);
}

void Fixture_Test::cleanupTestCase()
{
    delete m_doc;
}

void Fixture_Test::id()
{
    QVERIFY(Fixture::invalidId() == UINT_MAX);

    Fixture fxi(this);
    QVERIFY(fxi.id() == Fixture::invalidId());

    fxi.setID(50);
    QVERIFY(fxi.id() == 50);

    fxi.setID(INT_MAX);
    QVERIFY(fxi.id() == INT_MAX);
}

void Fixture_Test::name()
{
    Fixture fxi(this);
    QVERIFY(fxi.name().isEmpty());

    fxi.setName("MyFixture");
    QVERIFY(fxi.name() == "MyFixture");
}

void Fixture_Test::address()
{
    Fixture fxi(this);
    fxi.setChannels(5);

    QVERIFY(fxi.address() == 0);
    QVERIFY(fxi.universe() == 0);
    QVERIFY(fxi.universeAddress() == 0);

    fxi.setUniverse(1);
    QVERIFY(fxi.address() == 0);
    QVERIFY(fxi.universe() == 1);
    QVERIFY(fxi.universeAddress() == (1 << 9));

    fxi.setUniverse(2);
    QVERIFY(fxi.address() == 0);
    QVERIFY(fxi.universe() == 2);
    QVERIFY(fxi.universeAddress() == (2 << 9));

    fxi.setUniverse(3);
    QVERIFY(fxi.address() == 0);
    QVERIFY(fxi.universe() == 3);
    QVERIFY(fxi.universeAddress() == (3 << 9));

    /* The application might support only 4 universes, but there's no
       reason why Fixture itself couldn't support a million universes,
       as long as it fits into a uint minus 9 bits. */
    fxi.setUniverse(100);
    QVERIFY(fxi.address() == 0);
    QVERIFY(fxi.universe() == 100);
    QVERIFY(fxi.universeAddress() == (100 << 9));

    fxi.setAddress(15);
    fxi.setUniverse(0);
    QVERIFY(fxi.address() == 15);
    QVERIFY(fxi.universe() == 0);
    QVERIFY(fxi.universeAddress() == 15);

    /* Fixture should allow address overflow; maybe the first two channels
       that still fit to the universe here are enough for some fixture,
       who knows? */
    fxi.setAddress(510);
    QVERIFY(fxi.address() == 510);
    QVERIFY(fxi.universe() == 0);
    QVERIFY(fxi.universeAddress() == 510);

    /* Invalid addresses should not be allowed */
    fxi.setAddress(600);
    QVERIFY(fxi.address() == 510);
    QVERIFY(fxi.universe() == 0);
    QVERIFY(fxi.universeAddress() == 510);

    fxi.setAddress(100);
    QVERIFY(fxi.channelAddress(0) == 100);
    QVERIFY(fxi.channelAddress(1) == 101);
    QVERIFY(fxi.channelAddress(2) == 102);
    QVERIFY(fxi.channelAddress(3) == 103);
    QVERIFY(fxi.channelAddress(4) == 104);
    QVERIFY(fxi.channelAddress(5) == QLCChannel::invalid());
    QVERIFY(fxi.channelAddress(20) == QLCChannel::invalid());
}

void Fixture_Test::lessThan()
{
    Fixture fxi1(this);
    Fixture fxi2(this);

    QVERIFY(!(fxi1 < fxi2));
    QVERIFY(!(fxi2 < fxi1));

    fxi1.setAddress(0);
    fxi2.setAddress(1);
    QVERIFY(fxi1 < fxi2);
    QVERIFY(!(fxi2 < fxi1));

    fxi1.setAddress(511);
    fxi2.setAddress(42);
    QVERIFY(fxi2 < fxi1);
    QVERIFY(!(fxi1 < fxi2));
}

void Fixture_Test::type()
{
    Fixture fxi(this);
    QCOMPARE(fxi.typeString(), QString(KXMLFixtureDimmer));
    QCOMPARE(fxi.type(), QLCFixtureDef::Dimmer);
    QCOMPARE(fxi.iconResource(), QString(":/dimmer.png"));

    QLCFixtureDef* fixtureDef;
    fixtureDef = m_doc->fixtureDefCache()->fixtureDef("Martin", "MAC250+");
    QVERIFY(fixtureDef != NULL);

    QLCFixtureMode* fixtureMode;
    fixtureMode = fixtureDef->modes().at(0);
    QVERIFY(fixtureMode != NULL);

    fxi.setFixtureDefinition(fixtureDef, fixtureMode);
    QCOMPARE(fxi.typeString(), fixtureDef->typeToString(fixtureDef->type()));
    QCOMPARE(fxi.type(), QLCFixtureDef::MovingHead);
    QCOMPARE(fxi.iconResource(), QString(":/movinghead.png"));

    fixtureDef = m_doc->fixtureDefCache()->fixtureDef("SGM", "Colorlab 250");
    QVERIFY(fixtureDef != NULL);
    fixtureMode = fixtureDef->modes().at(0);
    QVERIFY(fixtureMode != NULL);

    fxi.setFixtureDefinition(fixtureDef, fixtureMode);
    QCOMPARE(fxi.typeString(), fixtureDef->typeToString(fixtureDef->type()));
    QCOMPARE(fxi.type(), QLCFixtureDef::ColorChanger);
    QCOMPARE(fxi.iconResource(), QString(":/fixture.png"));

    fixtureDef = m_doc->fixtureDefCache()->fixtureDef("Chauvet", "Vue 3.1");
    QVERIFY(fixtureDef != NULL);
    fixtureMode = fixtureDef->modes().at(0);
    QVERIFY(fixtureMode != NULL);

    fxi.setFixtureDefinition(fixtureDef, fixtureMode);
    QCOMPARE(fxi.typeString(), fixtureDef->typeToString(fixtureDef->type()));
    QCOMPARE(fxi.type(), QLCFixtureDef::Effect);
    QCOMPARE(fxi.iconResource(), QString(":/effect.png"));

    fixtureDef = m_doc->fixtureDefCache()->fixtureDef("Cameo", "Storm");
    QVERIFY(fixtureDef != NULL);
    fixtureMode = fixtureDef->modes().at(0);
    QVERIFY(fixtureMode != NULL);

    fxi.setFixtureDefinition(fixtureDef, fixtureMode);
    QCOMPARE(fxi.typeString(), fixtureDef->typeToString(fixtureDef->type()));
    QCOMPARE(fxi.type(), QLCFixtureDef::Flower);
    QCOMPARE(fxi.iconResource(), QString(":/flower.png"));

    fixtureDef = m_doc->fixtureDefCache()->fixtureDef("Showtec", "Dragon F-350");
    QVERIFY(fixtureDef != NULL);
    fixtureMode = fixtureDef->modes().at(0);
    QVERIFY(fixtureMode != NULL);

    fxi.setFixtureDefinition(fixtureDef, fixtureMode);
    QCOMPARE(fxi.typeString(), fixtureDef->typeToString(fixtureDef->type()));
    QCOMPARE(fxi.type(), QLCFixtureDef::Hazer);
    QCOMPARE(fxi.iconResource(true), QString("qrc:/hazer.svg"));

    fixtureDef = m_doc->fixtureDefCache()->fixtureDef("beamZ", "LS-3DRG");
    QVERIFY(fixtureDef != NULL);
    fixtureMode = fixtureDef->modes().at(0);
    QVERIFY(fixtureMode != NULL);

    fxi.setFixtureDefinition(fixtureDef, fixtureMode);
    QCOMPARE(fxi.typeString(), fixtureDef->typeToString(fixtureDef->type()));
    QCOMPARE(fxi.type(), QLCFixtureDef::Laser);
    QCOMPARE(fxi.iconResource(), QString(":/laser.png"));

    fixtureDef = m_doc->fixtureDefCache()->fixtureDef("GLP", "PocketScan");
    QVERIFY(fixtureDef != NULL);
    fixtureMode = fixtureDef->modes().at(0);
    QVERIFY(fixtureMode != NULL);

    fxi.setFixtureDefinition(fixtureDef, fixtureMode);
    QCOMPARE(fxi.typeString(), fixtureDef->typeToString(fixtureDef->type()));
    QCOMPARE(fxi.type(), QLCFixtureDef::Scanner);
    QCOMPARE(fxi.iconResource(), QString(":/scanner.png"));

    fixtureDef = m_doc->fixtureDefCache()->fixtureDef("Robe", "Fog 1500 FT");
    QVERIFY(fixtureDef != NULL);
    fixtureMode = fixtureDef->modes().at(0);
    QVERIFY(fixtureMode != NULL);

    fxi.setFixtureDefinition(fixtureDef, fixtureMode);
    QCOMPARE(fxi.typeString(), fixtureDef->typeToString(fixtureDef->type()));
    QCOMPARE(fxi.type(), QLCFixtureDef::Smoke);
    QCOMPARE(fxi.iconResource(true), QString("qrc:/smoke.svg"));

    fixtureDef = m_doc->fixtureDefCache()->fixtureDef("Chauvet", "LED Shadow");
    QVERIFY(fixtureDef != NULL);
    fixtureMode = fixtureDef->modes().at(0);
    QVERIFY(fixtureMode != NULL);

    fxi.setFixtureDefinition(fixtureDef, fixtureMode);
    QCOMPARE(fxi.typeString(), fixtureDef->typeToString(fixtureDef->type()));
    QCOMPARE(fxi.type(), QLCFixtureDef::Strobe);
    QCOMPARE(fxi.iconResource(), QString(":/strobe.png"));

    fixtureDef = m_doc->fixtureDefCache()->fixtureDef("Varytec", "Gigabar II");
    QVERIFY(fixtureDef != NULL);
    fixtureMode = fixtureDef->modes().at(0);
    QVERIFY(fixtureMode != NULL);

    fxi.setFixtureDefinition(fixtureDef, fixtureMode);
    QCOMPARE(fxi.typeString(), fixtureDef->typeToString(fixtureDef->type()));
    QCOMPARE(fxi.type(), QLCFixtureDef::LEDBarPixels);
    QCOMPARE(fxi.iconResource(), QString(":/ledbar_pixels.png"));

    fixtureDef = m_doc->fixtureDefCache()->fixtureDef("Clay Paky", "Show Batten 100");
    QVERIFY(fixtureDef != NULL);
    fixtureMode = fixtureDef->modes().at(0);
    QVERIFY(fixtureMode != NULL);

    fxi.setFixtureDefinition(fixtureDef, fixtureMode);
    QCOMPARE(fxi.typeString(), fixtureDef->typeToString(fixtureDef->type()));
    QCOMPARE(fxi.type(), QLCFixtureDef::LEDBarBeams);
    QCOMPARE(fxi.iconResource(true), QString("qrc:/ledbar_beams.svg"));
}

void Fixture_Test::dimmer()
{
    Fixture fxi(this);

    QVERIFY(fxi.fixtureDef() == NULL);
    QVERIFY(fxi.fixtureMode() == NULL);
    QVERIFY(fxi.channels() == 0);
    QVERIFY(fxi.channel(0) == NULL);
    QVERIFY(fxi.channel(42) == NULL);

    /* All channels point to the same generic channel instance */
    fxi.setChannels(5);
    QVERIFY(fxi.channels() == 5);
    QVERIFY(fxi.channel(0) != NULL);
    const QLCChannel* ch = fxi.channel(0);
    QVERIFY(fxi.channel(1) != fxi.channel(0));
    QVERIFY(fxi.channel(2) != fxi.channel(1));
    QVERIFY(fxi.channel(3) != fxi.channel(2));
    QVERIFY(fxi.channel(4) != fxi.channel(3));
    QVERIFY(fxi.channel(5) == NULL);
    QVERIFY(fxi.channel(42) == NULL);
    QVERIFY(fxi.channel(QLCChannel::invalid()) == NULL);

    QVERIFY(ch->capabilities().count() == 1);
    QVERIFY(ch->capabilities().at(0)->min() == 0);
    QVERIFY(ch->capabilities().at(0)->max() == UCHAR_MAX);
    QVERIFY(ch->capabilities().at(0)->name() == "Intensity");

    /* Although the dimmer fixture HAS a channel with this name, it is
       not returned, because all channels have the same name. */
    QVERIFY(fxi.channel(QLCChannel::Intensity) == 0);
}

void Fixture_Test::rgbPanel()
{
    Fixture fxi(this);
    fxi.setName("RGB Panel");
    QLCFixtureDef *rowDef = fxi.genericRGBPanelDef(10, Fixture::RGBW, false);
    QLCFixtureMode *rowMode = fxi.genericRGBPanelMode(rowDef, Fixture::RGBW, false, 1000, 100);
    fxi.setFixtureDefinition(rowDef, rowMode);

    QVERIFY(fxi.channels() == 40);

    QVERIFY(fxi.channel(0)->group() == QLCChannel::Intensity);
    QVERIFY(fxi.channel(0)->colour() == QLCChannel::Red);
    QVERIFY(fxi.channel(0)->name() == "Red 1");
    QVERIFY(fxi.channel(1)->group() == QLCChannel::Intensity);
    QVERIFY(fxi.channel(1)->colour() == QLCChannel::Green);
    QVERIFY(fxi.channel(1)->name() == "Green 1");
    QVERIFY(fxi.channel(2)->group() == QLCChannel::Intensity);
    QVERIFY(fxi.channel(2)->colour() == QLCChannel::Blue);
    QVERIFY(fxi.channel(2)->name() == "Blue 1");
    QVERIFY(fxi.channel(3)->group() == QLCChannel::Intensity);
    QVERIFY(fxi.channel(3)->colour() == QLCChannel::White);
    QVERIFY(fxi.channel(3)->name() == "White 1");

    QVERIFY(fxi.fixtureMode()->name() == "RGBW");
    QVERIFY(fxi.fixtureMode()->channels().count() == 40);
    QVERIFY(fxi.fixtureMode()->physical().width() == 1000);
    QVERIFY(fxi.fixtureMode()->physical().height() == 100);
    QVERIFY(fxi.fixtureMode()->physical().depth() == 100);
    QVERIFY(fxi.fixtureMode()->heads().count() == 10);

    rowDef = fxi.genericRGBPanelDef(10, Fixture::RGB, false);
    rowMode = fxi.genericRGBPanelMode(rowDef, Fixture::RGB, false, 1000, 100);
    fxi.setFixtureDefinition(rowDef, rowMode);

    QVERIFY(fxi.channels() == 30);
    QVERIFY(fxi.fixtureMode()->name() == "RGB");
    QVERIFY(fxi.fixtureMode()->channels().count() == 30);

    QVERIFY(fxi.channel(0)->group() == QLCChannel::Intensity);
    QVERIFY(fxi.channel(0)->colour() == QLCChannel::Red);
    QVERIFY(fxi.channel(0)->name() == "Red 1");
    QVERIFY(fxi.channel(1)->group() == QLCChannel::Intensity);
    QVERIFY(fxi.channel(1)->colour() == QLCChannel::Green);
    QVERIFY(fxi.channel(1)->name() == "Green 1");
    QVERIFY(fxi.channel(2)->group() == QLCChannel::Intensity);
    QVERIFY(fxi.channel(2)->colour() == QLCChannel::Blue);
    QVERIFY(fxi.channel(2)->name() == "Blue 1");

    rowDef = fxi.genericRGBPanelDef(10, Fixture::RBG, false);
    rowMode = fxi.genericRGBPanelMode(rowDef, Fixture::RBG, false, 1000, 100);
    fxi.setFixtureDefinition(rowDef, rowMode);

    QVERIFY(fxi.channels() == 30);
    QVERIFY(fxi.fixtureMode()->name() == "RBG");
    QVERIFY(fxi.fixtureMode()->channels().count() == 30);

    QVERIFY(fxi.channel(0)->group() == QLCChannel::Intensity);
    QVERIFY(fxi.channel(0)->colour() == QLCChannel::Red);
    QVERIFY(fxi.channel(0)->name() == "Red 1");
    QVERIFY(fxi.channel(1)->group() == QLCChannel::Intensity);
    QVERIFY(fxi.channel(1)->colour() == QLCChannel::Blue);
    QVERIFY(fxi.channel(1)->name() == "Blue 1");
    QVERIFY(fxi.channel(2)->group() == QLCChannel::Intensity);
    QVERIFY(fxi.channel(2)->colour() == QLCChannel::Green);
    QVERIFY(fxi.channel(2)->name() == "Green 1");

    rowDef = fxi.genericRGBPanelDef(10, Fixture::BGR, false);
    rowMode = fxi.genericRGBPanelMode(rowDef, Fixture::BGR, false, 1000, 100);
    fxi.setFixtureDefinition(rowDef, rowMode);

    QVERIFY(fxi.channels() == 30);
    QVERIFY(fxi.fixtureMode()->name() == "BGR");
    QVERIFY(fxi.fixtureMode()->channels().count() == 30);

    QVERIFY(fxi.channel(0)->group() == QLCChannel::Intensity);
    QVERIFY(fxi.channel(0)->colour() == QLCChannel::Blue);
    QVERIFY(fxi.channel(0)->name() == "Blue 1");
    QVERIFY(fxi.channel(1)->group() == QLCChannel::Intensity);
    QVERIFY(fxi.channel(1)->colour() == QLCChannel::Green);
    QVERIFY(fxi.channel(1)->name() == "Green 1");
    QVERIFY(fxi.channel(2)->group() == QLCChannel::Intensity);
    QVERIFY(fxi.channel(2)->colour() == QLCChannel::Red);
    QVERIFY(fxi.channel(2)->name() == "Red 1");

    rowDef = fxi.genericRGBPanelDef(10, Fixture::BRG, false);
    rowMode = fxi.genericRGBPanelMode(rowDef, Fixture::BRG, false, 1000, 100);
    fxi.setFixtureDefinition(rowDef, rowMode);

    QVERIFY(fxi.channels() == 30);
    QVERIFY(fxi.fixtureMode()->name() == "BRG");
    QVERIFY(fxi.fixtureMode()->channels().count() == 30);

    QVERIFY(fxi.channel(0)->group() == QLCChannel::Intensity);
    QVERIFY(fxi.channel(0)->colour() == QLCChannel::Blue);
    QVERIFY(fxi.channel(0)->name() == "Blue 1");
    QVERIFY(fxi.channel(1)->group() == QLCChannel::Intensity);
    QVERIFY(fxi.channel(1)->colour() == QLCChannel::Red);
    QVERIFY(fxi.channel(1)->name() == "Red 1");
    QVERIFY(fxi.channel(2)->group() == QLCChannel::Intensity);
    QVERIFY(fxi.channel(2)->colour() == QLCChannel::Green);
    QVERIFY(fxi.channel(2)->name() == "Green 1");

    rowDef = fxi.genericRGBPanelDef(10, Fixture::GBR, false);
    rowMode = fxi.genericRGBPanelMode(rowDef, Fixture::GBR, false, 1000, 100);
    fxi.setFixtureDefinition(rowDef, rowMode);

    QVERIFY(fxi.channels() == 30);
    QVERIFY(fxi.fixtureMode()->name() == "GBR");
    QVERIFY(fxi.fixtureMode()->channels().count() == 30);

    QVERIFY(fxi.channel(0)->group() == QLCChannel::Intensity);
    QVERIFY(fxi.channel(0)->colour() == QLCChannel::Green);
    QVERIFY(fxi.channel(0)->name() == "Green 1");
    QVERIFY(fxi.channel(1)->group() == QLCChannel::Intensity);
    QVERIFY(fxi.channel(1)->colour() == QLCChannel::Blue);
    QVERIFY(fxi.channel(1)->name() == "Blue 1");
    QVERIFY(fxi.channel(2)->group() == QLCChannel::Intensity);
    QVERIFY(fxi.channel(2)->colour() == QLCChannel::Red);
    QVERIFY(fxi.channel(2)->name() == "Red 1");

    rowDef = fxi.genericRGBPanelDef(10, Fixture::GRB, false);
    rowMode = fxi.genericRGBPanelMode(rowDef, Fixture::GRB, false, 1000, 100);
    fxi.setFixtureDefinition(rowDef, rowMode);

    QVERIFY(fxi.channels() == 30);
    QVERIFY(fxi.fixtureMode()->name() == "GRB");
    QVERIFY(fxi.fixtureMode()->channels().count() == 30);

    QVERIFY(fxi.channel(0)->group() == QLCChannel::Intensity);
    QVERIFY(fxi.channel(0)->colour() == QLCChannel::Green);
    QVERIFY(fxi.channel(0)->name() == "Green 1");
    QVERIFY(fxi.channel(1)->group() == QLCChannel::Intensity);
    QVERIFY(fxi.channel(1)->colour() == QLCChannel::Red);
    QVERIFY(fxi.channel(1)->name() == "Red 1");
    QVERIFY(fxi.channel(2)->group() == QLCChannel::Intensity);
    QVERIFY(fxi.channel(2)->colour() == QLCChannel::Blue);
    QVERIFY(fxi.channel(2)->name() == "Blue 1");
}

void Fixture_Test::rgbPanel16bit()
{
    Fixture fxi(this);
    fxi.setName("RGB Panel 16bit");
    QLCFixtureDef *rowDef = fxi.genericRGBPanelDef(10, Fixture::RGB, true);
    QLCFixtureMode *rowMode = fxi.genericRGBPanelMode(rowDef, Fixture::RGB, true, 1000, 100);
    fxi.setFixtureDefinition(rowDef, rowMode);

    QVERIFY(fxi.channels() == 60);
    QVERIFY(fxi.fixtureMode()->name() == "RGB 16bit");
    QVERIFY(fxi.fixtureMode()->heads().at(0).channels().count() == 6);

    QVERIFY(fxi.channel(0)->group() == QLCChannel::Intensity);
    QVERIFY(fxi.channel(0)->colour() == QLCChannel::Red);
    QVERIFY(fxi.channel(0)->name() == "Red 1");
    QVERIFY(fxi.channel(0)->controlByte() == QLCChannel::MSB);

    QVERIFY(fxi.channel(1)->group() == QLCChannel::Intensity);
    QVERIFY(fxi.channel(1)->colour() == QLCChannel::Red);
    QVERIFY(fxi.channel(1)->name() == "Red Fine 1");
    QVERIFY(fxi.channel(1)->controlByte() == QLCChannel::LSB);
}

void Fixture_Test::fixtureDef()
{
    Fixture fxi(this);

    QVERIFY(fxi.fixtureDef() == NULL);
    QVERIFY(fxi.fixtureMode() == NULL);
    QVERIFY(fxi.channels() == 0);
    QVERIFY(fxi.channel(0) == NULL);
    QCOMPARE(fxi.channelNumber(QLCChannel::Pan, QLCChannel::MSB), QLCChannel::invalid());
    QCOMPARE(fxi.channelNumber(QLCChannel::Tilt, QLCChannel::MSB), QLCChannel::invalid());
    QCOMPARE(fxi.channelNumber(QLCChannel::Pan, QLCChannel::LSB), QLCChannel::invalid());
    QCOMPARE(fxi.channelNumber(QLCChannel::Tilt, QLCChannel::LSB), QLCChannel::invalid());
    QCOMPARE(fxi.masterIntensityChannel(), QLCChannel::invalid());

    QLCFixtureDef* fixtureDef;
    fixtureDef = m_doc->fixtureDefCache()->fixtureDef("Martin", "MAC300");
    Q_ASSERT(fixtureDef != NULL);

    fxi.setFixtureDefinition(fixtureDef, NULL);
    QVERIFY(fxi.fixtureDef() == NULL);
    QVERIFY(fxi.fixtureMode() == NULL);

    QLCFixtureMode* fixtureMode;
    fixtureMode = fixtureDef->modes().last();
    Q_ASSERT(fixtureMode != NULL);

    fxi.setFixtureDefinition(NULL, fixtureMode);
    QVERIFY(fxi.fixtureDef() == NULL);
    QVERIFY(fxi.fixtureMode() == NULL);

    fxi.setFixtureDefinition(fixtureDef, fixtureMode);
    QVERIFY(fxi.fixtureDef() != NULL);
    QVERIFY(fxi.fixtureMode() != NULL);
    QVERIFY(fxi.fixtureDef() == fixtureDef);
    QVERIFY(fxi.fixtureMode() == fixtureMode);

    QVERIFY(fxi.channels() == quint32(fixtureMode->channels().count()));
    QVERIFY(fxi.channel(fxi.channels() - 1) != NULL);
    QVERIFY(fxi.channel(fxi.channels()) == NULL);

    QVERIFY(fxi.channel(QLCChannel::Pan) != QLCChannel::invalid());
    const QLCChannel* ch = fxi.channel(fxi.channel(QLCChannel::Pan));
    QVERIFY(ch != NULL);

    QCOMPARE(fxi.channelNumber(QLCChannel::Pan, QLCChannel::MSB), quint32(7));
    QCOMPARE(fxi.channelNumber(QLCChannel::Tilt, QLCChannel::MSB), quint32(9));
    QCOMPARE(fxi.channelNumber(QLCChannel::Pan, QLCChannel::LSB), quint32(8));
    QCOMPARE(fxi.channelNumber(QLCChannel::Tilt, QLCChannel::LSB), quint32(10));
    QCOMPARE(fxi.masterIntensityChannel(), quint32(1));
    QCOMPARE(fxi.rgbChannels(), QVector <quint32> ());
    QCOMPARE(fxi.cmyChannels(), QVector <quint32> () << 2 << 3 << 4);
}

void Fixture_Test::channels()
{
    Fixture fxi(this);
    QLCFixtureDef* fixtureDef = m_doc->fixtureDefCache()->fixtureDef("i-Pix", "BB4");
    QVERIFY(fixtureDef != NULL);
    QLCFixtureMode* fixtureMode = fixtureDef->modes().last();
    QVERIFY(fixtureMode != NULL);
    fxi.setFixtureDefinition(fixtureDef, fixtureMode);

    QCOMPARE(fxi.channel(QLCChannel::Intensity, QLCChannel::Red), quint32(3));

    QSet <quint32> chs;
    chs << 3 << 4 << 21 << 22 << 12 << 13 << 30 << 31;
    QCOMPARE(chs, fxi.channels(QLCChannel::Intensity, QLCChannel::Red));
    chs.clear();
    chs << 5 << 6 << 23 << 24 << 14 << 15 << 32 << 33;
    QCOMPARE(chs, fxi.channels(QLCChannel::Intensity, QLCChannel::Green));
    chs.clear();
    chs << 7 << 8 << 16 << 17 << 25 << 26 << 34 << 35;
    QCOMPARE(chs, fxi.channels(QLCChannel::Intensity, QLCChannel::Blue));
    chs.clear();
    QCOMPARE(chs, fxi.channels(QLCChannel::Colour, QLCChannel::Blue));
}

void Fixture_Test::degrees()
{
    Fixture fxi(this);

    QLCFixtureDef* fixtureDef;
    fixtureDef = m_doc->fixtureDefCache()->fixtureDef("Martin", "MAC250+");
    QVERIFY(fixtureDef != NULL);

    QLCFixtureMode* fixtureMode;
    fixtureMode = fixtureDef->modes().at(1);
    QVERIFY(fixtureMode != NULL);

    fxi.setFixtureDefinition(fixtureDef, fixtureMode);

    QCOMPARE(fxi.degreesRange(0).width(), 540.0);
    QCOMPARE(fxi.degreesRange(0).height(), 270.0);

    // dummy ID just for testing
    fxi.setID(99);

    QList<SceneValue> pos = fxi.positionToValues(QLCChannel::Pan, 90);
    QCOMPARE(pos.count(), 2);
    // verify coarse Pan
    QCOMPARE(pos.at(0).fxi, quint32(99));
    QCOMPARE(pos.at(0).channel, quint32(7));
    QCOMPARE(pos.at(0).value, uchar(42));
    // verify fine Pan
    QCOMPARE(pos.at(1).fxi, quint32(99));
    QCOMPARE(pos.at(1).channel, quint32(8));
    QCOMPARE(pos.at(1).value, uchar(170));

    pos = fxi.positionToValues(QLCChannel::Tilt, 45);
    QCOMPARE(pos.count(), 2);
    // verify coarse Tilt
    QCOMPARE(pos.at(0).fxi, quint32(99));
    QCOMPARE(pos.at(0).channel, quint32(9));
    QCOMPARE(pos.at(0).value, uchar(42));
    // verify fine Tilt
    QCOMPARE(pos.at(1).fxi, quint32(99));
    QCOMPARE(pos.at(1).channel, quint32(10));
    QCOMPARE(pos.at(1).value, uchar(170));
}

void Fixture_Test::heads()
{
    Fixture fxi(this);

    QLCFixtureDef* fixtureDef;
    fixtureDef = m_doc->fixtureDefCache()->fixtureDef("Equinox", "Photon");
    QVERIFY(fixtureDef != NULL);

    QLCFixtureMode* fixtureMode;
    fixtureMode = fixtureDef->modes().last();
    QVERIFY(fixtureMode != NULL);

    fxi.setFixtureDefinition(fixtureDef, fixtureMode);

    QCOMPARE(fxi.heads(), 6);

    QLCFixtureHead head = fxi.head(0);
    QCOMPARE(head.channels().count(), 4);
    head = fxi.head(1);
    QCOMPARE(head.channels().count(), 4);
    head = fxi.head(2);
    QCOMPARE(head.channels().count(), 4);
    head = fxi.head(3);
    QCOMPARE(head.channels().count(), 4);
    head = fxi.head(4);
    QCOMPARE(head.channels().count(), 4);
    head = fxi.head(5);
    QCOMPARE(head.channels().count(), 4);
}

void Fixture_Test::loadWrongRoot()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Function");
    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    Fixture fxi(this);
    QVERIFY(fxi.loadXML(xmlReader, m_doc, m_doc->fixtureDefCache()) == false);
}

void Fixture_Test::loadFixtureDef()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Fixture");

    xmlWriter.writeTextElement("Channels", "9");
    xmlWriter.writeTextElement("Name", "Foobar");
    xmlWriter.writeTextElement("Universe", "0");
    xmlWriter.writeTextElement("Model", "MAC250+");
    xmlWriter.writeTextElement("Mode", "Mode 1");
    xmlWriter.writeTextElement("Manufacturer", "Martin");
    xmlWriter.writeTextElement("ID", "42");
    xmlWriter.writeTextElement("Address", "21");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    Fixture fxi(this);
    QVERIFY(fxi.loadXML(xmlReader, m_doc, m_doc->fixtureDefCache()) == true);
    QVERIFY(fxi.name() == "Foobar");
    QVERIFY(fxi.channels() == 9);
    QVERIFY(fxi.address() == 21);
    QVERIFY(fxi.universe() == 0);
    QVERIFY(fxi.fixtureDef() != NULL);
    QVERIFY(fxi.fixtureMode() != NULL);
}

void Fixture_Test::loadFixtureDefWrongChannels()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Fixture");

    xmlWriter.writeTextElement("Channels", "15");
    xmlWriter.writeTextElement("Name", "Foobar");
    xmlWriter.writeTextElement("Universe", "0");
    xmlWriter.writeTextElement("Model", "MAC250+");
    xmlWriter.writeTextElement("Mode", "Mode 1");
    xmlWriter.writeTextElement("Manufacturer", "Martin");
    xmlWriter.writeTextElement("ID", "42");
    xmlWriter.writeTextElement("Address", "21");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    Fixture fxi(this);
    QVERIFY(fxi.loadXML(xmlReader, m_doc, m_doc->fixtureDefCache()) == true);
    QVERIFY(fxi.name() == "Foobar");
    QVERIFY(fxi.channels() == 9);
    QVERIFY(fxi.address() == 21);
    QVERIFY(fxi.universe() == 0);
    QVERIFY(fxi.fixtureDef() != NULL);
    QVERIFY(fxi.fixtureMode() != NULL);
}

void Fixture_Test::loadDimmer()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Fixture");

    xmlWriter.writeTextElement("Channels", "18");
    xmlWriter.writeTextElement("Name", "Foobar");
    xmlWriter.writeTextElement("Universe", "3");
    xmlWriter.writeTextElement("Model", "Foobar");
    xmlWriter.writeTextElement("Mode", "Foobar");
    xmlWriter.writeTextElement("Manufacturer", "Foobar");
    xmlWriter.writeTextElement("ID", "42");
    xmlWriter.writeTextElement("Address", "21");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    Fixture fxi(this);
    QVERIFY(fxi.loadXML(xmlReader, m_doc, m_doc->fixtureDefCache()) == true);
    QVERIFY(fxi.name() == "Foobar");
    QVERIFY(fxi.channels() == 18);
    QVERIFY(fxi.address() == 21);
    QVERIFY(fxi.universe() == 3);
    QVERIFY(fxi.fixtureDef() != NULL);
    QVERIFY(fxi.fixtureMode() != NULL);
}

void Fixture_Test::loadWrongAddress()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Fixture");

    xmlWriter.writeTextElement("Channels", "18");
    xmlWriter.writeTextElement("Name", "Foobar");
    xmlWriter.writeTextElement("Universe", "0");
    xmlWriter.writeTextElement("Model", "Foobar");
    xmlWriter.writeTextElement("Mode", "Foobar");
    xmlWriter.writeTextElement("Manufacturer", "Foobar");
    xmlWriter.writeTextElement("ID", "42");
    xmlWriter.writeTextElement("Address", "512");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    Fixture fxi(this);
    QVERIFY(fxi.loadXML(xmlReader, m_doc, m_doc->fixtureDefCache()) == true);
    QVERIFY(fxi.name() == "Foobar");
    QVERIFY(fxi.channels() == 18);
    QVERIFY(fxi.address() == 0);
    QVERIFY(fxi.universe() == 0);
}

void Fixture_Test::loadWrongUniverse()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Fixture");

    xmlWriter.writeTextElement("Channels", "18");
    xmlWriter.writeTextElement("Name", "Foobar");
    xmlWriter.writeTextElement("Universe", "4");
    xmlWriter.writeTextElement("Model", "Foobar");
    xmlWriter.writeTextElement("Mode", "Foobar");
    xmlWriter.writeTextElement("Manufacturer", "Foobar");
    xmlWriter.writeTextElement("ID", "42");
    xmlWriter.writeTextElement("Address", "25");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    Fixture fxi(this);
    QVERIFY(fxi.loadXML(xmlReader, m_doc, m_doc->fixtureDefCache()) == true);
    QVERIFY(fxi.name() == "Foobar");
    QVERIFY(fxi.channels() == 18);
    QVERIFY(fxi.address() == 25);
    QVERIFY(fxi.universe() == 4);
}

void Fixture_Test::loadWrongID()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Fixture");

    xmlWriter.writeTextElement("Channels", "9");
    xmlWriter.writeTextElement("Name", "Foobar");
    xmlWriter.writeTextElement("Universe", "0");
    xmlWriter.writeTextElement("Model", "MAC250+");
    xmlWriter.writeTextElement("Mode", "Mode 1");
    xmlWriter.writeTextElement("Manufacturer", "Martin");
    xmlWriter.writeTextElement("ID", QString::number(Fixture::invalidId()));
    xmlWriter.writeTextElement("Address", "21");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    Fixture fxi(this);
    QVERIFY(fxi.loadXML(xmlReader, m_doc, m_doc->fixtureDefCache()) == false);
}

void Fixture_Test::loader()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Fixture");

    xmlWriter.writeTextElement("Channels", "18");
    xmlWriter.writeTextElement("Name", "Foobar");
    xmlWriter.writeTextElement("Universe", "3");
    xmlWriter.writeTextElement("Model", "Foobar");
    xmlWriter.writeTextElement("Mode", "Foobar");
    xmlWriter.writeTextElement("Manufacturer", "Foobar");
    xmlWriter.writeTextElement("ID", "42");
    xmlWriter.writeTextElement("Address", "21");

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(m_doc != NULL);
    QVERIFY(m_doc->fixtures().size() == 0);

    QVERIFY(Fixture::loader(xmlReader, m_doc) == true);
    QVERIFY(m_doc->fixtures().size() == 1);
    QVERIFY(m_doc->fixture(0) == NULL); // No ID auto-assignment

    Fixture* fxi = m_doc->fixture(42);
    QVERIFY(fxi != NULL);
    QVERIFY(fxi->name() == "Foobar");
    QVERIFY(fxi->channels() == 18);
    QVERIFY(fxi->address() == 21);
    QVERIFY(fxi->universe() == 3);
    QVERIFY(fxi->fixtureDef() != NULL);
    QVERIFY(fxi->fixtureMode() != NULL);
}

void Fixture_Test::save()
{
    QLCFixtureDef* fixtureDef;
    fixtureDef = m_doc->fixtureDefCache()->fixtureDef("Martin", "MAC250+");
    Q_ASSERT(fixtureDef != NULL);

    QLCFixtureMode* fixtureMode;
    fixtureMode = fixtureDef->modes().at(0);
    Q_ASSERT(fixtureMode != NULL);

    Fixture fxi(this);
    fxi.setID(1337);
    fxi.setName("Test Fixture");
    fxi.setUniverse(2);
    fxi.setAddress(438);
    fxi.setFixtureDefinition(fixtureDef, fixtureMode);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("TestRoot");

    QVERIFY(fxi.saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();
    QVERIFY(xmlReader.name().toString() == "TestRoot");
    xmlReader.readNextStartElement();
    QVERIFY(xmlReader.name().toString() == "Fixture");

    bool manufacturer = false, model = false, mode = false, name = false,
                                       channels = false, universe = false, address = false, id = false;

    while (xmlReader.readNextStartElement())
    {
        if (xmlReader.name().toString() == "Manufacturer")
        {
            QVERIFY(xmlReader.readElementText() == "Martin");
            manufacturer = true;
        }
        else if (xmlReader.name().toString() == "Model")
        {
            QVERIFY(xmlReader.readElementText() == "MAC250+");
            model = true;
        }
        else if (xmlReader.name().toString() == "Mode")
        {
            QVERIFY(xmlReader.readElementText() == fixtureMode->name());
            mode = true;
        }
        else if (xmlReader.name().toString() == "ID")
        {
            QVERIFY(xmlReader.readElementText() == "1337");
            id = true;
        }
        else if (xmlReader.name().toString() == "Name")
        {
            QVERIFY(xmlReader.readElementText() == "Test Fixture");
            name = true;
        }
        else if (xmlReader.name().toString() == "Universe")
        {
            QVERIFY(xmlReader.readElementText() == "2");
            universe = true;
        }
        else if (xmlReader.name().toString() == "Address")
        {
            QVERIFY(xmlReader.readElementText() == "438");
            address = true;
        }
        else if (xmlReader.name().toString() == "Channels")
        {
            QVERIFY(xmlReader.readElementText().toInt()
                    == fixtureMode->channels().count());
            channels = true;
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(xmlReader.name().toString())
                  .toLatin1());
            xmlReader.skipCurrentElement();
        }
    }

    QVERIFY(manufacturer == true);
    QVERIFY(model == true);
    QVERIFY(mode == true);
    QVERIFY(id == true);
    QVERIFY(name == true);
    QVERIFY(universe == true);
    QVERIFY(address == true);
    QVERIFY(channels == true);
}

void Fixture_Test::status()
{
    // This test is mostly just a stability check since checking lots of
    // detailed HTML formatting is not that useful.
    QString info;

    Fixture fxi(this);
    info = fxi.status();

    fxi.setID(1337);
    info = fxi.status();

    fxi.setName("Test Fixture");
    info = fxi.status();

    fxi.setUniverse(2);
    info = fxi.status();

    fxi.setAddress(438);
    info = fxi.status();

    fxi.setChannels(12);
    info = fxi.status();

    QLCFixtureDef* fixtureDef;
    fixtureDef = m_doc->fixtureDefCache()->fixtureDef("Martin", "MAC250+");
    Q_ASSERT(fixtureDef != NULL);

    QLCFixtureMode* fixtureMode;
    fixtureMode = fixtureDef->modes().at(0);
    Q_ASSERT(fixtureMode != NULL);

    fxi.setFixtureDefinition(fixtureDef, fixtureMode);
    info = fxi.status();
}

QTEST_APPLESS_MAIN(Fixture_Test)
