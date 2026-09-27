/*
  Q Light Controller Plus - Unit test
  fixtureremapper_test.cpp

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

#define protected public
#define private public
#include "mastertimer_stub.h"
#include "fixtureremapper.h"
#include "monitorproperties.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "channelsgroup.h"
#include "fixturegroup.h"
#include "qlcchannel.h"
#include "scenevalue.h"
#include "chaserstep.h"
#include "grouphead.h"
#include "sequence.h"
#include "fixture.h"
#include "scene.h"
#include "doc.h"
#undef private
#undef protected

#include "fixtureremapper_test.h"

QTEST_MAIN(FixtureRemapper_Test)

/* -------------------------------------------------------------------------
 * Helpers
 * ------------------------------------------------------------------------- */

static QLCFixtureDef *makeDef(const QString &name)
{
    QLCFixtureDef *def = new QLCFixtureDef();
    def->setManufacturer("Test");
    def->setModel(name);
    return def;
}

static QLCChannel *makeChannel(const QString &name,
                               QLCChannel::Group group,
                               QLCChannel::ControlByte cb = QLCChannel::MSB,
                               QLCChannel::PrimaryColour colour = QLCChannel::NoColour)
{
    QLCChannel *ch = new QLCChannel();
    ch->setName(name);
    ch->setGroup(group);
    ch->setControlByte(cb);
    ch->setColour(colour);
    return ch;
}

Fixture *FixtureRemapper_Test::buildMovingHead(Doc *doc, quint32 address,
                                               const QString &defName,
                                               const QString &modeName)
{
    QLCFixtureDef *def = makeDef(defName);
    def->addChannel(makeChannel("Dimmer", QLCChannel::Intensity));
    def->addChannel(makeChannel("Pan",    QLCChannel::Pan));
    def->addChannel(makeChannel("Tilt",   QLCChannel::Tilt));

    QLCFixtureMode *mode = new QLCFixtureMode(def);
    mode->setName(modeName);
    mode->insertChannel(def->channel("Dimmer"), 0);
    mode->insertChannel(def->channel("Pan"),    1);
    mode->insertChannel(def->channel("Tilt"),   2);
    def->addMode(mode);

    Fixture *fxi = new Fixture(doc);
    fxi->setAddress(address);
    fxi->setUniverse(0);
    fxi->setFixtureDefinition(def, mode);
    doc->addFixture(fxi);
    return fxi;
}

Fixture *FixtureRemapper_Test::buildDimmer(Doc *doc, quint32 address, quint32 channels)
{
    Fixture *fxi = new Fixture(doc);
    fxi->setAddress(address);
    fxi->setUniverse(0);
    QLCFixtureDef *def = fxi->genericDimmerDef(channels);
    QLCFixtureMode *mode = fxi->genericDimmerMode(def, channels);
    fxi->setFixtureDefinition(def, mode);
    doc->addFixture(fxi);
    return fxi;
}

Fixture *FixtureRemapper_Test::buildRGB(Doc *doc, quint32 address, const QString &defName)
{
    QLCFixtureDef *def = makeDef(defName);
    def->addChannel(makeChannel("Red",   QLCChannel::Intensity, QLCChannel::MSB, QLCChannel::Red));
    def->addChannel(makeChannel("Green", QLCChannel::Intensity, QLCChannel::MSB, QLCChannel::Green));
    def->addChannel(makeChannel("Blue",  QLCChannel::Intensity, QLCChannel::MSB, QLCChannel::Blue));

    QLCFixtureMode *mode = new QLCFixtureMode(def);
    mode->setName("RGB");
    mode->insertChannel(def->channel("Red"),   0);
    mode->insertChannel(def->channel("Green"), 1);
    mode->insertChannel(def->channel("Blue"),  2);
    def->addMode(mode);

    Fixture *fxi = new Fixture(doc);
    fxi->setAddress(address);
    fxi->setUniverse(0);
    fxi->setFixtureDefinition(def, mode);
    doc->addFixture(fxi);
    return fxi;
}

/* -------------------------------------------------------------------------
 * Test lifecycle
 * ------------------------------------------------------------------------- */

void FixtureRemapper_Test::initTestCase()
{
    m_doc = new Doc(this);
}

void FixtureRemapper_Test::cleanupTestCase()
{
    delete m_doc;
}

void FixtureRemapper_Test::init()
{
}

void FixtureRemapper_Test::cleanup()
{
    m_doc->clearContents();
}

/* -------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

void FixtureRemapper_Test::testAddChannelRemap()
{
    FixtureRemapper remapper;
    QVERIFY(remapper.sourceList().isEmpty());
    QVERIFY(remapper.targetList().isEmpty());

    remapper.addChannelRemap(1, 0, 2, 3);
    remapper.addChannelRemap(1, 1, 2, 4);

    QCOMPARE(remapper.sourceList().count(), 2);
    QCOMPARE(remapper.targetList().count(), 2);

    QCOMPARE(remapper.sourceList().at(0).fxi,     quint32(1));
    QCOMPARE(remapper.sourceList().at(0).channel, quint32(0));
    QCOMPARE(remapper.targetList().at(0).fxi,     quint32(2));
    QCOMPARE(remapper.targetList().at(0).channel, quint32(3));

    QCOMPARE(remapper.sourceList().at(1).fxi,     quint32(1));
    QCOMPARE(remapper.sourceList().at(1).channel, quint32(1));
    QCOMPARE(remapper.targetList().at(1).fxi,     quint32(2));
    QCOMPARE(remapper.targetList().at(1).channel, quint32(4));
}

void FixtureRemapper_Test::testReset()
{
    FixtureRemapper remapper;
    remapper.addChannelRemap(1, 0, 2, 0);
    remapper.addChannelRemap(1, 1, 2, 1);
    QVERIFY(!remapper.sourceList().isEmpty());

    remapper.reset();
    QVERIFY(remapper.sourceList().isEmpty());
    QVERIFY(remapper.targetList().isEmpty());
}

void FixtureRemapper_Test::testRemapSceneValues()
{
    QList<SceneValue> srcList;
    srcList << SceneValue(1, 0) << SceneValue(1, 1) << SceneValue(1, 2);

    QList<SceneValue> tgtList;
    tgtList << SceneValue(2, 5) << SceneValue(2, 6) << SceneValue(2, 7);

    QList<SceneValue> funcList;
    funcList << SceneValue(1, 0, 200)   // maps to (2,5,200)
             << SceneValue(1, 2, 100)   // maps to (2,7,100)
             << SceneValue(1, 99, 50);  // no match → dropped

    QList<SceneValue> result = FixtureRemapper::remapSceneValues(funcList, srcList, tgtList);

    QCOMPARE(result.count(), 2);
    // result is sorted by (fxi, channel)
    QCOMPARE(result.at(0).fxi,     quint32(2));
    QCOMPARE(result.at(0).channel, quint32(5));
    QCOMPARE(result.at(0).value,   uchar(200));
    QCOMPARE(result.at(1).fxi,     quint32(2));
    QCOMPARE(result.at(1).channel, quint32(7));
    QCOMPARE(result.at(1).value,   uchar(100));
}

void FixtureRemapper_Test::testAutoConnectOneToOne()
{
    // Two fixtures with the same def name and mode name → 1:1 mapping
    Doc srcDoc(this);
    Doc tgtDoc(this);

    Fixture *src = buildMovingHead(&srcDoc, 0,  "MH1", "Mode1");
    Fixture *tgt = buildMovingHead(&tgtDoc, 10, "MH1", "Mode1");

    // src has id 0, tgt has id 0 in its own doc; for our remapper test
    // the IDs are what matter in the result lists
    FixtureRemapper remapper;
    QList<QPair<quint32, quint32>> pairs = remapper.autoConnectFixtures(src, tgt);

    // 3 channels: Dimmer→Dimmer, Pan→Pan, Tilt→Tilt
    QCOMPARE(pairs.count(), 3);
    QCOMPARE(pairs.at(0).first,  quint32(0));
    QCOMPARE(pairs.at(0).second, quint32(0));
    QCOMPARE(pairs.at(1).first,  quint32(1));
    QCOMPARE(pairs.at(1).second, quint32(1));
    QCOMPARE(pairs.at(2).first,  quint32(2));
    QCOMPARE(pairs.at(2).second, quint32(2));

    QCOMPARE(remapper.sourceList().count(), 3);
    QCOMPARE(remapper.targetList().count(), 3);

    // All source entries belong to src fixture
    for (const SceneValue &sv : remapper.sourceList())
        QCOMPARE(sv.fxi, src->id());
    for (const SceneValue &sv : remapper.targetList())
        QCOMPARE(sv.fxi, tgt->id());
}

void FixtureRemapper_Test::testAutoConnectSemantic()
{
    // Two RGB fixtures with different def names → semantic matching by group+colour
    Doc srcDoc(this);
    Doc tgtDoc(this);

    Fixture *src = buildRGB(&srcDoc, 0,  "LED-A");
    Fixture *tgt = buildRGB(&tgtDoc, 10, "LED-B");

    FixtureRemapper remapper;
    QList<QPair<quint32, quint32>> pairs = remapper.autoConnectFixtures(src, tgt);

    // Red→Red, Green→Green, Blue→Blue
    QCOMPARE(pairs.count(), 3);
    QCOMPARE(pairs.at(0).first,  quint32(0)); // src ch0 Red
    QCOMPARE(pairs.at(0).second, quint32(0)); // tgt ch0 Red
    QCOMPARE(pairs.at(1).first,  quint32(1)); // src ch1 Green
    QCOMPARE(pairs.at(1).second, quint32(1)); // tgt ch1 Green
    QCOMPARE(pairs.at(2).first,  quint32(2)); // src ch2 Blue
    QCOMPARE(pairs.at(2).second, quint32(2)); // tgt ch2 Blue
}

void FixtureRemapper_Test::testAutoConnectGenericDimmer()
{
    Doc srcDoc(this);
    Doc tgtDoc(this);

    Fixture *src = buildDimmer(&srcDoc, 0,  4);
    Fixture *tgt = buildDimmer(&tgtDoc, 10, 4);

    FixtureRemapper remapper;
    QList<QPair<quint32, quint32>> pairs = remapper.autoConnectFixtures(src, tgt);

    QCOMPARE(pairs.count(), 4);
    for (int i = 0; i < 4; i++)
    {
        QCOMPARE(pairs.at(i).first,  quint32(i));
        QCOMPARE(pairs.at(i).second, quint32(i));
    }
}

void FixtureRemapper_Test::testApplyRemapScene()
{
    // src fixture id=0, tgt fixture id=1 (assigned by addFixture)
    Fixture *src = buildMovingHead(m_doc, 0,  "MH", "M1");
    quint32 srcId = src->id();

    // Build the target fixture in a temporary doc, then applyRemap will
    // call replaceFixtures to swap them into m_doc
    Doc targetDoc(this);
    Fixture *tgt = buildMovingHead(&targetDoc, 20, "MH", "M1");
    quint32 tgtId = tgt->id();

    // Create a Scene that references src fixture channels
    Scene *s = new Scene(m_doc);
    s->addFixture(srcId);
    s->setValue(SceneValue(srcId, 0, 255)); // Dimmer full
    s->setValue(SceneValue(srcId, 1, 128)); // Pan mid
    m_doc->addFunction(s);

    // Build remapper: 3 channels 1:1 (same def+mode)
    FixtureRemapper remapper;
    remapper.autoConnectFixtures(src, tgt);

    remapper.applyRemap(m_doc, targetDoc.fixtures());

    // After applyRemap the scene values must point to tgtId
    QList<SceneValue> vals = s->values();
    QCOMPARE(vals.count(), 2);
    for (const SceneValue &sv : vals)
        QCOMPARE(sv.fxi, tgtId);

    // Values preserved
    bool foundDimmer = false, foundPan = false;
    for (const SceneValue &sv : vals)
    {
        if (sv.channel == 0) { QCOMPARE(sv.value, uchar(255)); foundDimmer = true; }
        if (sv.channel == 1) { QCOMPARE(sv.value, uchar(128)); foundPan = true;   }
    }
    QVERIFY(foundDimmer);
    QVERIFY(foundPan);
}

void FixtureRemapper_Test::testApplyRemapSequence()
{
    Fixture *src = buildMovingHead(m_doc, 0,  "MH", "M1");
    quint32 srcId = src->id();

    Doc targetDoc(this);
    Fixture *tgt = buildMovingHead(&targetDoc, 20, "MH", "M1");
    quint32 tgtId = tgt->id();

    // Sequence bound to a scene — add both to doc before adding steps
    Scene *boundScene = new Scene(m_doc);
    m_doc->addFunction(boundScene);

    Sequence *seq = new Sequence(m_doc);
    seq->setBoundSceneID(boundScene->id());
    m_doc->addFunction(seq);  // must be added before addStep so seq->id() is valid

    // Steps reference the bound scene function id, not the sequence id
    ChaserStep step(boundScene->id());
    step.values << SceneValue(srcId, 0, 200) << SceneValue(srcId, 2, 50);
    seq->addStep(step);

    FixtureRemapper remapper;
    remapper.autoConnectFixtures(src, tgt);
    remapper.applyRemap(m_doc, targetDoc.fixtures());

    ChaserStep *cs = seq->stepAt(0);
    QVERIFY(cs != nullptr);
    QCOMPARE(cs->values.count(), 2);
    for (const SceneValue &sv : cs->values)
        QCOMPARE(sv.fxi, tgtId);
}

void FixtureRemapper_Test::testApplyRemapFixtureGroup()
{
    Fixture *src = buildMovingHead(m_doc, 0, "MH", "M1");
    quint32 srcId = src->id();

    Doc targetDoc(this);
    Fixture *tgt = buildMovingHead(&targetDoc, 20, "MH", "M1");
    quint32 tgtId = tgt->id();

    FixtureGroup *group = new FixtureGroup(m_doc);
    group->setName("TestGroup");
    GroupHead head(srcId, 0);
    group->assignHead(QLCPoint(0, 0), head);
    m_doc->addFixtureGroup(group);

    FixtureRemapper remapper;
    remapper.autoConnectFixtures(src, tgt);
    remapper.applyRemap(m_doc, targetDoc.fixtures());

    // The group head at (0,0) should now reference tgtId
    GroupHead remappedHead = group->head(QLCPoint(0, 0));
    QCOMPARE(remappedHead.fxi, tgtId);
}

void FixtureRemapper_Test::testApplyRemapChannelsGroup()
{
    Fixture *src = buildMovingHead(m_doc, 0, "MH", "M1");
    quint32 srcId = src->id();

    Doc targetDoc(this);
    Fixture *tgt = buildMovingHead(&targetDoc, 20, "MH", "M1");
    quint32 tgtId = tgt->id();

    ChannelsGroup *grp = new ChannelsGroup(m_doc);
    grp->addChannel(srcId, 0);
    grp->addChannel(srcId, 1);
    m_doc->addChannelsGroup(grp);

    FixtureRemapper remapper;
    remapper.autoConnectFixtures(src, tgt);
    remapper.applyRemap(m_doc, targetDoc.fixtures());

    QList<SceneValue> channels = grp->getChannels();
    QCOMPARE(channels.count(), 2);
    for (const SceneValue &sv : channels)
        QCOMPARE(sv.fxi, tgtId);
}

void FixtureRemapper_Test::testApplyRemapMonitor()
{
    Fixture *src = buildMovingHead(m_doc, 0, "MH", "M1");
    quint32 srcId = src->id();

    Doc targetDoc(this);
    // Add a placeholder so tgt gets a different ID than src (src has ID 0)
    buildDimmer(&targetDoc, 0, 1);
    Fixture *tgt = buildMovingHead(&targetDoc, 20, "MH", "M1");
    quint32 tgtId = tgt->id();
    QVERIFY(tgtId != srcId);

    MonitorProperties *props = m_doc->monitorProperties();
    FixturePreviewItem item;
    item.m_baseItem.m_position = QVector3D(1.0f, 2.0f, 3.0f);
    props->setFixtureProperties(srcId, item);
    QVERIFY(props->fixtureItemsID().contains(srcId));

    FixtureRemapper remapper;
    remapper.autoConnectFixtures(src, tgt);
    remapper.applyRemap(m_doc, targetDoc.fixtures());

    // srcId entry removed, tgtId entry present with same position
    QVERIFY(!props->fixtureItemsID().contains(srcId));
    QVERIFY(props->fixtureItemsID().contains(tgtId));
    QCOMPARE(props->fixtureProperties(tgtId).m_baseItem.m_position,
             QVector3D(1.0f, 2.0f, 3.0f));
}
