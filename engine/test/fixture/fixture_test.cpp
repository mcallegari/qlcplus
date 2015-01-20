/*
  Q Light Controller - Unit test
  fixture_test.cpp

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
#include <QtXml>

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
    QVERIFY(m_doc->fixtureDefCache()->load(dir) == true);
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
    QCOMPARE(fxi.type(), QString(KXMLFixtureDimmer));

    QLCFixtureDef* fixtureDef;
    fixtureDef = m_doc->fixtureDefCache()->fixtureDef("Martin", "MAC250+");
    QVERIFY(fixtureDef != NULL);

    QLCFixtureMode* fixtureMode;
    fixtureMode = fixtureDef->modes().at(0);
    QVERIFY(fixtureMode != NULL);

    fxi.setFixtureDefinition(fixtureDef, fixtureMode);
    QCOMPARE(fxi.type(), fixtureDef->type());
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
    QVERIFY(fxi.channel(1) == ch);
    QVERIFY(fxi.channel(2) == ch);
    QVERIFY(fxi.channel(3) == ch);
    QVERIFY(fxi.channel(4) == ch);
    QVERIFY(fxi.channel(5) == NULL);
    QVERIFY(fxi.channel(42) == NULL);
    QVERIFY(fxi.channel(QLCChannel::invalid()) == NULL);

    QVERIFY(ch->capabilities().count() == 1);
    QVERIFY(ch->capabilities().at(0)->min() == 0);
    QVERIFY(ch->capabilities().at(0)->max() == UCHAR_MAX);
    QVERIFY(ch->capabilities().at(0)->name() == "Intensity");

    /* Although the dimmer fixture HAS a channel with this name, it is
       not returned, because all channels have the same name. */
    QVERIFY(fxi.channel(QLCChannel::Intensity) == QLCChannel::invalid());
}

void Fixture_Test::fixtureDef()
{
    Fixture fxi(this);

    QVERIFY(fxi.fixtureDef() == NULL);
    QVERIFY(fxi.fixtureMode() == NULL);
    QVERIFY(fxi.channels() == 0);
    QVERIFY(fxi.channel(0) == NULL);
    QCOMPARE(fxi.panMsbChannel(), QLCChannel::invalid());
    QCOMPARE(fxi.tiltMsbChannel(), QLCChannel::invalid());
    QCOMPARE(fxi.panLsbChannel(), QLCChannel::invalid());
    QCOMPARE(fxi.tiltLsbChannel(), QLCChannel::invalid());
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

    QCOMPARE(fxi.panMsbChannel(), quint32(7));
    QCOMPARE(fxi.tiltMsbChannel(), quint32(9));
    QCOMPARE(fxi.panLsbChannel(), quint32(8));
    QCOMPARE(fxi.tiltLsbChannel(), quint32(10));
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

void Fixture_Test::loadWrongRoot()
{
    QDomDocument doc;
    Fixture fxi(this);

    QDomElement root = doc.createElement("Function");
    doc.appendChild(root);
    QVERIFY(fxi.loadXML(root, m_doc, m_doc->fixtureDefCache()) == false);
}

void Fixture_Test::loadFixtureDef()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Fixture");
    doc.appendChild(root);

    QDomElement chs = doc.createElement("Channels");
    QDomText chsText = doc.createTextNode("9");
    chs.appendChild(chsText);
    root.appendChild(chs);

    QDomElement name = doc.createElement("Name");
    QDomText nameText = doc.createTextNode("Foobar");
    name.appendChild(nameText);
    root.appendChild(name);

    QDomElement uni = doc.createElement("Universe");
    QDomText uniText = doc.createTextNode("0");
    uni.appendChild(uniText);
    root.appendChild(uni);

    QDomElement model = doc.createElement("Model");
    QDomText modelText = doc.createTextNode("MAC250+");
    model.appendChild(modelText);
    root.appendChild(model);

    QDomElement mode = doc.createElement("Mode");
    QDomText modeText = doc.createTextNode("Mode 1");
    mode.appendChild(modeText);
    root.appendChild(mode);

    QDomElement type = doc.createElement("Manufacturer");
    QDomText typeText = doc.createTextNode("Martin");
    type.appendChild(typeText);
    root.appendChild(type);

    QDomElement id = doc.createElement("ID");
    QDomText idText = doc.createTextNode("42");
    id.appendChild(idText);
    root.appendChild(id);

    QDomElement addr = doc.createElement("Address");
    QDomText addrText = doc.createTextNode("21");
    addr.appendChild(addrText);
    root.appendChild(addr);

    Fixture fxi(this);
    QVERIFY(fxi.loadXML(root, m_doc, m_doc->fixtureDefCache()) == true);
    QVERIFY(fxi.name() == "Foobar");
    QVERIFY(fxi.channels() == 9);
    QVERIFY(fxi.address() == 21);
    QVERIFY(fxi.universe() == 0);
    QVERIFY(fxi.fixtureDef() != NULL);
    QVERIFY(fxi.fixtureMode() != NULL);
}

void Fixture_Test::loadFixtureDefWrongChannels()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Fixture");
    doc.appendChild(root);

    QDomElement chs = doc.createElement("Channels");
    QDomText chsText = doc.createTextNode("15");
    chs.appendChild(chsText);
    root.appendChild(chs);

    QDomElement name = doc.createElement("Name");
    QDomText nameText = doc.createTextNode("Foobar");
    name.appendChild(nameText);
    root.appendChild(name);

    QDomElement mode = doc.createElement("Mode");
    QDomText modeText = doc.createTextNode("Mode 1");
    mode.appendChild(modeText);
    root.appendChild(mode);

    QDomElement uni = doc.createElement("Universe");
    QDomText uniText = doc.createTextNode("0");
    uni.appendChild(uniText);
    root.appendChild(uni);

    QDomElement model = doc.createElement("Model");
    QDomText modelText = doc.createTextNode("MAC250+");
    model.appendChild(modelText);
    root.appendChild(model);

    QDomElement type = doc.createElement("Manufacturer");
    QDomText typeText = doc.createTextNode("Martin");
    type.appendChild(typeText);
    root.appendChild(type);

    QDomElement id = doc.createElement("ID");
    QDomText idText = doc.createTextNode("42");
    id.appendChild(idText);
    root.appendChild(id);

    QDomElement addr = doc.createElement("Address");
    QDomText addrText = doc.createTextNode("21");
    addr.appendChild(addrText);
    root.appendChild(addr);

    Fixture fxi(this);
    QVERIFY(fxi.loadXML(root, m_doc, m_doc->fixtureDefCache()) == true);
    QVERIFY(fxi.name() == "Foobar");
    QVERIFY(fxi.channels() == 9);
    QVERIFY(fxi.address() == 21);
    QVERIFY(fxi.universe() == 0);
    QVERIFY(fxi.fixtureDef() != NULL);
    QVERIFY(fxi.fixtureMode() != NULL);
}

void Fixture_Test::loadDimmer()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Fixture");
    doc.appendChild(root);

    QDomElement chs = doc.createElement("Channels");
    QDomText chsText = doc.createTextNode("18");
    chs.appendChild(chsText);
    root.appendChild(chs);

    QDomElement name = doc.createElement("Name");
    QDomText nameText = doc.createTextNode("Foobar");
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

    QDomElement id = doc.createElement("ID");
    QDomText idText = doc.createTextNode("42");
    id.appendChild(idText);
    root.appendChild(id);

    QDomElement addr = doc.createElement("Address");
    QDomText addrText = doc.createTextNode("21");
    addr.appendChild(addrText);
    root.appendChild(addr);

    Fixture fxi(this);
    QVERIFY(fxi.loadXML(root, m_doc, m_doc->fixtureDefCache()) == true);
    QVERIFY(fxi.name() == "Foobar");
    QVERIFY(fxi.channels() == 18);
    QVERIFY(fxi.address() == 21);
    QVERIFY(fxi.universe() == 3);
    QVERIFY(fxi.fixtureDef() == NULL);
    QVERIFY(fxi.fixtureMode() == NULL);
}

void Fixture_Test::loadWrongAddress()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Fixture");
    doc.appendChild(root);

    QDomElement chs = doc.createElement("Channels");
    QDomText chsText = doc.createTextNode("18");
    chs.appendChild(chsText);
    root.appendChild(chs);

    QDomElement name = doc.createElement("Name");
    QDomText nameText = doc.createTextNode("Foobar");
    name.appendChild(nameText);
    root.appendChild(name);

    QDomElement uni = doc.createElement("Universe");
    QDomText uniText = doc.createTextNode("0");
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

    QDomElement id = doc.createElement("ID");
    QDomText idText = doc.createTextNode("42");
    id.appendChild(idText);
    root.appendChild(id);

    QDomElement addr = doc.createElement("Address");
    QDomText addrText = doc.createTextNode("512");
    addr.appendChild(addrText);
    root.appendChild(addr);

    Fixture fxi(this);
    QVERIFY(fxi.loadXML(root, m_doc, m_doc->fixtureDefCache()) == true);
    QVERIFY(fxi.name() == "Foobar");
    QVERIFY(fxi.channels() == 18);
    QVERIFY(fxi.address() == 0);
    QVERIFY(fxi.universe() == 0);
}

void Fixture_Test::loadWrongUniverse()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Fixture");
    doc.appendChild(root);

    QDomElement chs = doc.createElement("Channels");
    QDomText chsText = doc.createTextNode("18");
    chs.appendChild(chsText);
    root.appendChild(chs);

    QDomElement name = doc.createElement("Name");
    QDomText nameText = doc.createTextNode("Foobar");
    name.appendChild(nameText);
    root.appendChild(name);

    QDomElement uni = doc.createElement("Universe");
    QDomText uniText = doc.createTextNode("4");
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

    QDomElement id = doc.createElement("ID");
    QDomText idText = doc.createTextNode("42");
    id.appendChild(idText);
    root.appendChild(id);

    QDomElement addr = doc.createElement("Address");
    QDomText addrText = doc.createTextNode("25");
    addr.appendChild(addrText);
    root.appendChild(addr);

    Fixture fxi(this);
    QVERIFY(fxi.loadXML(root, m_doc, m_doc->fixtureDefCache()) == true);
    QVERIFY(fxi.name() == "Foobar");
    QVERIFY(fxi.channels() == 18);
    QVERIFY(fxi.address() == 25);
    QVERIFY(fxi.universe() == 4);
}

void Fixture_Test::loadWrongID()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Fixture");
    doc.appendChild(root);

    QDomElement chs = doc.createElement("Channels");
    QDomText chsText = doc.createTextNode("9");
    chs.appendChild(chsText);
    root.appendChild(chs);

    QDomElement name = doc.createElement("Name");
    QDomText nameText = doc.createTextNode("Foobar");
    name.appendChild(nameText);
    root.appendChild(name);

    QDomElement uni = doc.createElement("Universe");
    QDomText uniText = doc.createTextNode("0");
    uni.appendChild(uniText);
    root.appendChild(uni);

    QDomElement model = doc.createElement("Model");
    QDomText modelText = doc.createTextNode("MAC250+");
    model.appendChild(modelText);
    root.appendChild(model);

    QDomElement mode = doc.createElement("Mode");
    QDomText modeText = doc.createTextNode("Mode 1");
    mode.appendChild(modeText);
    root.appendChild(mode);

    QDomElement type = doc.createElement("Manufacturer");
    QDomText typeText = doc.createTextNode("Martin");
    type.appendChild(typeText);
    root.appendChild(type);

    QDomElement id = doc.createElement("ID");
    QDomText idText = doc.createTextNode(QString::number(Fixture::invalidId()));
    id.appendChild(idText);
    root.appendChild(id);

    QDomElement addr = doc.createElement("Address");
    QDomText addrText = doc.createTextNode("21");
    addr.appendChild(addrText);
    root.appendChild(addr);

    Fixture fxi(this);
    QVERIFY(fxi.loadXML(root, m_doc, m_doc->fixtureDefCache()) == false);
}

void Fixture_Test::loader()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Fixture");
    doc.appendChild(root);

    QDomElement chs = doc.createElement("Channels");
    QDomText chsText = doc.createTextNode("18");
    chs.appendChild(chsText);
    root.appendChild(chs);

    QDomElement name = doc.createElement("Name");
    QDomText nameText = doc.createTextNode("Foobar");
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

    QDomElement id = doc.createElement("ID");
    QDomText idText = doc.createTextNode("42");
    id.appendChild(idText);
    root.appendChild(id);

    QDomElement addr = doc.createElement("Address");
    QDomText addrText = doc.createTextNode("21");
    addr.appendChild(addrText);
    root.appendChild(addr);

    QVERIFY(m_doc != NULL);
    QVERIFY(m_doc->fixtures().size() == 0);

    QVERIFY(Fixture::loader(root, m_doc) == true);
    QVERIFY(m_doc->fixtures().size() == 1);
    QVERIFY(m_doc->fixture(0) == NULL); // No ID auto-assignment

    Fixture* fxi = m_doc->fixture(42);
    QVERIFY(fxi != NULL);
    QVERIFY(fxi->name() == "Foobar");
    QVERIFY(fxi->channels() == 18);
    QVERIFY(fxi->address() == 21);
    QVERIFY(fxi->universe() == 3);
    QVERIFY(fxi->fixtureDef() == NULL);
    QVERIFY(fxi->fixtureMode() == NULL);
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

    QDomDocument doc;
    QDomElement root = doc.createElement("TestRoot");
    QVERIFY(fxi.saveXML(&doc, &root) == true);
    QDomNode node = root.firstChild();
    QVERIFY(node.toElement().tagName() == "Fixture");

    bool manufacturer = false, model = false, mode = false, name = false,
                                       channels = false, universe = false, address = false, id = false;

    node = node.firstChild();
    while (node.isNull() == false)
    {
        QDomElement e = node.toElement();

        if (e.tagName() == "Manufacturer")
        {
            QVERIFY(e.text() == "Martin");
            manufacturer = true;
        }
        else if (e.tagName() == "Model")
        {
            QVERIFY(e.text() == "MAC250+");
            model = true;
        }
        else if (e.tagName() == "Mode")
        {
            QVERIFY(e.text() == fixtureMode->name());
            mode = true;
        }
        else if (e.tagName() == "ID")
        {
            QVERIFY(e.text() == "1337");
            id = true;
        }
        else if (e.tagName() == "Name")
        {
            QVERIFY(e.text() == "Test Fixture");
            name = true;
        }
        else if (e.tagName() == "Universe")
        {
            QVERIFY(e.text() == "2");
            universe = true;
        }
        else if (e.tagName() == "Address")
        {
            QVERIFY(e.text() == "438");
            address = true;
        }
        else if (e.tagName() == "Channels")
        {
            QVERIFY(e.text().toInt()
                    == fixtureMode->channels().count());
            channels = true;
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(e.tagName())
                  .toLatin1());
        }

        node = node.nextSibling();
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
