/*
  Q Light Controller Plus - Test Unit
  vcxypad_test.cpp

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

#include "vcxypad_test.h"
#include "qlcinputsource.h"
#include "vcxypadpreset.h"
#include "vctestenv.h"
#include "vcxypad.h"
#include "scene.h"
#include "efx.h"

/* External control IDs, as defined in vcxypad.cpp */
#define INPUT_PAN_ID    0

void VCXYPad_Test::init()
{
    m_env = new VCTestEnv();
}

void VCXYPad_Test::cleanup()
{
    delete m_env;
}

void VCXYPad_Test::initial()
{
    VCXYPad pad(m_env->doc());

    QCOMPARE(pad.type(), int(VCWidget::XYPadWidget));
    QCOMPARE(pad.invertedAppearance(), false);
    QCOMPARE(pad.displayMode(), VCXYPad::Degrees);
    QCOMPARE(pad.currentPosition(), QPointF(0, 0));
    QCOMPARE(pad.horizontalRange(), QPointF(0, 255));
    QCOMPARE(pad.verticalRange(), QPointF(0, 255));
    QCOMPARE(pad.floorControl(), false);
    QVERIFY(pad.presetsList().isEmpty());
    QCOMPARE(pad.activePresetId(), -1);
    QCOMPARE(pad.supportsPresets(), true);
    QCOMPARE(pad.presetsResource(), QString("qrc:/VCXYPadPresets.qml"));
    QCOMPARE(pad.propertiesResource(), QString("qrc:/VCXYPadProperties.qml"));
    QCOMPARE(pad.externalControlsCount(), 7);

    pad.setID(0);
    QCOMPARE(pad.caption(), QString("XY Pad 1"));
}

void VCXYPad_Test::properties()
{
    VCXYPad pad(m_env->doc());

    QSignalSpy invSpy(&pad, &VCXYPad::invertedAppearanceChanged);
    pad.setInvertedAppearance(true);
    QCOMPARE(pad.invertedAppearance(), true);
    QCOMPARE(invSpy.count(), 1);
    pad.setInvertedAppearance(true);
    QCOMPARE(invSpy.count(), 1);

    QSignalSpy modeSpy(&pad, &VCXYPad::displayModeChanged);
    pad.setDisplayMode(VCXYPad::Percentage);
    QCOMPARE(pad.displayMode(), VCXYPad::Percentage);
    QCOMPARE(modeSpy.count(), 1);
    pad.setDisplayMode(VCXYPad::Percentage);
    QCOMPARE(modeSpy.count(), 1);

    QSignalSpy posSpy(&pad, &VCXYPad::currentPositionChanged);
    pad.setCurrentPosition(QPointF(100, 50));
    QCOMPARE(pad.currentPosition(), QPointF(100, 50));
    QCOMPARE(posSpy.count(), 1);
    pad.setCurrentPosition(QPointF(100, 50));
    QCOMPARE(posSpy.count(), 1);

    /* The position is clamped to the 16 bit DMX range */
    pad.setCurrentPosition(QPointF(300, -10));
    QVERIFY(pad.currentPosition().x() > 255.0);
    QVERIFY(pad.currentPosition().x() < 256.0);
    QCOMPARE(pad.currentPosition().y(), qreal(0));

    QSignalSpy hSpy(&pad, &VCXYPad::horizontalRangeChanged);
    pad.setHorizontalRange(QPointF(10, 200));
    QCOMPARE(pad.horizontalRange(), QPointF(10, 200));
    QCOMPARE(hSpy.count(), 1);
    pad.setHorizontalRange(QPointF(10, 200));
    QCOMPARE(hSpy.count(), 1);

    QSignalSpy vSpy(&pad, &VCXYPad::verticalRangeChanged);
    pad.setVerticalRange(QPointF(20, 100));
    QCOMPARE(pad.verticalRange(), QPointF(20, 100));
    QCOMPARE(vSpy.count(), 1);
    pad.setVerticalRange(QPointF(20, 100));
    QCOMPARE(vSpy.count(), 1);
}

void VCXYPad_Test::floorPosition()
{
    VCXYPad pad(m_env->doc());
    QVector3D size = pad.floorSize();
    QVERIFY(size.x() > 0);
    QVERIFY(size.z() > 0);

    /* The floor target starts at the centre of the stage */
    QVERIFY(qFuzzyCompare(pad.floorPosition().x(), size.x() / 2.0f));
    QVERIFY(qFuzzyCompare(pad.floorPosition().z(), size.z() / 2.0f));

    QSignalSpy spy(&pad, &VCXYPad::floorControlChanged);
    pad.setFloorControl(true);
    QCOMPARE(pad.floorControl(), true);
    QCOMPARE(spy.count(), 1);

    /* The target is clamped to the stage and to the maximum height */
    pad.setFloorPosition(QVector3D(size.x() * 2, 1000, -5));
    QVERIFY(qFuzzyCompare(pad.floorPosition().x(), size.x()));
    QVERIFY(qFuzzyCompare(qreal(pad.floorPosition().y()), pad.floorHeightMax()));
    QVERIFY(qFuzzyIsNull(pad.floorPosition().z()));

    /* The range window limits the reachable area */
    pad.setHorizontalRange(QPointF(0, 127.5));
    QVERIFY(qFuzzyCompare(pad.floorPosition().x(), size.x() / 2.0f));
}

void VCXYPad_Test::positionPresets()
{
    VCXYPad pad(m_env->doc());
    QSignalSpy spy(&pad, &VCXYPad::presetsListChanged);

    pad.setCurrentPosition(QPointF(10, 20));
    int p1 = pad.addPositionPreset();
    pad.setCurrentPosition(QPointF(30, 40));
    int p2 = pad.addPositionPreset();
    QVERIFY(p1 != p2);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(pad.externalControlsCount(), 9);

    QVariantList list = pad.presetsList();
    QCOMPARE(list.count(), 2);
    QVariantMap map = list.at(0).toMap();
    QCOMPARE(map.value("id").toInt(), p1);
    QCOMPARE(map.value("name").toString(), QString("X:10 - Y:20"));
    QCOMPARE(map.value("type").toInt(), int(VCXYPadPreset::Position));
    QCOMPARE(map.value("typeString").toString(), QString("Position"));
    QCOMPARE(map.value("color").toString(), QString("#BBBBBB"));
    QCOMPARE(map.value("active").toBool(), false);
    QCOMPARE(list.at(1).toMap().value("name").toString(), QString("X:30 - Y:40"));

    pad.setPresetName(p1, "Home");
    QCOMPARE(pad.presetsList().at(0).toMap().value("name").toString(), QString("Home"));
    QCOMPARE(pad.externalControlsCount(), 9);

    /* Moving up swaps the IDs, hence the order */
    QCOMPARE(pad.movePresetUp(p2), p1);
    list = pad.presetsList();
    QCOMPARE(list.at(0).toMap().value("name").toString(), QString("X:30 - Y:40"));
    QCOMPARE(list.at(1).toMap().value("name").toString(), QString("Home"));

    /* The first preset can't move up */
    QCOMPARE(pad.movePresetUp(p1), p1);

    pad.removePreset(p1);
    QCOMPARE(pad.presetsList().count(), 1);
    QCOMPARE(pad.presetsList().first().toMap().value("name").toString(), QString("Home"));
    QCOMPARE(pad.externalControlsCount(), 8);

    /* Unknown presets are ignored */
    pad.removePreset(200);
    QCOMPARE(pad.presetsList().count(), 1);
}

void VCXYPad_Test::functionPresets()
{
    Doc *doc = m_env->doc();

    EFX *efx = new EFX(doc);
    efx->setName("Circle");
    doc->addFunction(efx);

    /* A scene without Pan/Tilt channels can't be a preset */
    Scene *scene = new Scene(doc);
    doc->addFunction(scene);

    VCXYPad pad(doc);
    QCOMPARE(pad.addFunctionPreset(12345), -1);
    QCOMPARE(pad.addFunctionPreset(scene->id()), -1);
    QVERIFY(pad.presetsList().isEmpty());

    int id = pad.addFunctionPreset(efx->id());
    QVERIFY(id != -1);

    QVariantList list = pad.presetsList();
    QCOMPARE(list.count(), 1);
    QVariantMap map = list.first().toMap();
    QCOMPARE(map.value("id").toInt(), id);
    QCOMPARE(map.value("name").toString(), QString("Circle"));
    QCOMPARE(map.value("type").toInt(), int(VCXYPadPreset::EFX));
    QCOMPARE(map.value("functionID").toUInt(), efx->id());
    QCOMPARE(map.value("color").toString(), QString("#BBBB8D"));
}

void VCXYPad_Test::copy()
{
    VCWidget parent(m_env->doc());

    VCXYPad pad(m_env->doc());
    pad.setCaption("Copied");
    pad.setInvertedAppearance(true);
    pad.setDisplayMode(VCXYPad::DMX);
    pad.setCurrentPosition(QPointF(100, 50));
    /* Enable floor control first, so the target gets clamped to the ranges */
    pad.setFloorControl(true);
    pad.setHorizontalRange(QPointF(10, 200));
    pad.setVerticalRange(QPointF(20, 100));
    pad.addPositionPreset();

    VCXYPad *copy = qobject_cast<VCXYPad *>(pad.createCopy(&parent));
    QVERIFY(copy != nullptr);
    QCOMPARE(copy->caption(), QString("Copied"));
    QCOMPARE(copy->invertedAppearance(), true);
    QCOMPARE(copy->displayMode(), VCXYPad::DMX);
    QCOMPARE(copy->currentPosition(), QPointF(100, 50));
    QCOMPARE(copy->horizontalRange(), QPointF(10, 200));
    QCOMPARE(copy->verticalRange(), QPointF(20, 100));
    QCOMPARE(copy->floorControl(), true);
    QCOMPARE(copy->floorPosition(), pad.floorPosition());
    QCOMPARE(copy->presetsList(), pad.presetsList());
    QCOMPARE(copy->externalControlsCount(), pad.externalControlsCount());
}

void VCXYPad_Test::saveLoad()
{
    EFX *efx = new EFX(m_env->doc());
    m_env->doc()->addFunction(efx);

    VCXYPad pad(m_env->doc());
    pad.setID(8);
    pad.setCaption("Pad");
    pad.setInvertedAppearance(true);
    /* Enable floor control first, so the target gets clamped to the ranges */
    pad.setFloorControl(true);
    pad.setHorizontalRange(QPointF(10, 200));
    pad.setVerticalRange(QPointF(20, 100));
    pad.setCurrentPosition(QPointF(10, 20));
    int p1 = pad.addPositionPreset();
    pad.setPresetName(p1, "Home");
    int p2 = pad.addFunctionPreset(efx->id());
    pad.setCurrentPosition(QPointF(100, 50));

    QSharedPointer<QLCInputSource> src(new QLCInputSource(0, 3));
    src->setID(INPUT_PAN_ID);
    pad.addInputSource(src);

    /* VCXYPad::saveXML is private: call it through the base class */
    QByteArray data = vcSaveXML(static_cast<const VCWidget *>(&pad));
    VCXMLReader xml(data);
    QCOMPARE(xml.reader().name().toString(), QString("XYPad"));

    VCXYPad pad2(m_env->doc());
    QVERIFY(pad2.loadXML(xml.reader()));
    QCOMPARE(pad2.id(), quint32(8));
    QCOMPARE(pad2.caption(), QString("Pad"));
    QCOMPARE(pad2.invertedAppearance(), true);
    QCOMPARE(pad2.horizontalRange(), QPointF(10, 200));
    QCOMPARE(pad2.verticalRange(), QPointF(20, 100));
    QCOMPARE(pad2.floorControl(), true);
    /* The position is saved with 6 significant digits */
    QVERIFY(qFuzzyCompare(pad2.floorPosition(), pad.floorPosition()));
    QCOMPARE(pad2.currentPosition(), QPointF(100, 50));

    QVariantList list = pad2.presetsList();
    QCOMPARE(list.count(), 2);
    QCOMPARE(list.at(0).toMap().value("id").toInt(), p1);
    QCOMPARE(list.at(0).toMap().value("name").toString(), QString("Home"));
    QCOMPARE(list.at(0).toMap().value("type").toInt(), int(VCXYPadPreset::Position));
    QCOMPARE(list.at(1).toMap().value("id").toInt(), p2);
    QCOMPARE(list.at(1).toMap().value("type").toInt(), int(VCXYPadPreset::EFX));
    QCOMPARE(list.at(1).toMap().value("functionID").toUInt(), efx->id());

    QCOMPARE(pad2.inputSources().count(), 1);
    QCOMPARE(pad2.inputSources().first()->id(), quint32(INPUT_PAN_ID));
    QCOMPARE(pad2.inputSources().first()->channel(), quint32(3));

    /* Wrong tag */
    VCXMLReader wrongXml(QByteArray("<Button/>"));
    VCXYPad pad3(m_env->doc());
    QCOMPARE(pad3.loadXML(wrongXml.reader()), false);
}

QTEST_MAIN(VCXYPad_Test)
