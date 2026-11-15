/*
  Q Light Controller Plus - Test Unit
  vcspeeddial_test.cpp

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

#include "vcspeeddial_test.h"
#include "qlcinputsource.h"
#include "vcspeeddial.h"
#include "vctestenv.h"
#include "scene.h"

/* External control IDs, as defined in vcspeeddial.cpp */
#define INPUT_DIAL_ID   0
#define INPUT_TAP_ID    1

void VCSpeedDial_Test::init()
{
    m_env = new VCTestEnv();
}

void VCSpeedDial_Test::cleanup()
{
    delete m_env;
}

void VCSpeedDial_Test::initial()
{
    VCSpeedDial dial(m_env->doc());

    QCOMPARE(dial.type(), int(VCWidget::SpeedWidget));
    QCOMPARE(dial.visibilityMask(), quint32(VCSpeedDial::Tap | VCSpeedDial::Beats | VCSpeedDial::Multipliers));
    QCOMPARE(dial.timeMinimumValue(), uint(0));
    QCOMPARE(dial.timeMaximumValue(), uint(10000));
    QCOMPARE(dial.currentTime(), uint(0));
    QCOMPARE(dial.resetOnDialChange(), false);
    QCOMPARE(dial.controlBPM(), false);
    QCOMPARE(dial.currentFactor(), VCSpeedDial::One);
    QCOMPARE(dial.tapTimeValue(), 0);
    QVERIFY(dial.functions().isEmpty());
    QVERIFY(dial.presetsList().isEmpty());
    QCOMPARE(dial.supportsPresets(), true);
    QCOMPARE(dial.presetsResource(), QString("qrc:/VCSpeedDialPresets.qml"));
    QCOMPARE(dial.propertiesResource(), QString("qrc:/VCSpeedDialProperties.qml"));
    QCOMPARE(dial.externalControlsCount(), 14);

    dial.setID(2);
    QCOMPARE(dial.caption(), QString("Speed 3"));
}

void VCSpeedDial_Test::properties()
{
    VCSpeedDial dial(m_env->doc());

    QSignalSpy maskSpy(&dial, &VCSpeedDial::visibilityMaskChanged);
    dial.setVisibilityMask(VCSpeedDial::Dial | VCSpeedDial::Apply);
    QCOMPARE(dial.visibilityMask(), quint32(VCSpeedDial::Dial | VCSpeedDial::Apply));
    QCOMPARE(maskSpy.count(), 1);
    dial.setVisibilityMask(VCSpeedDial::Dial | VCSpeedDial::Apply);
    QCOMPARE(maskSpy.count(), 1);

    QSignalSpy minSpy(&dial, &VCSpeedDial::timeMinimumValueChanged);
    dial.setTimeMinimumValue(100);
    QCOMPARE(dial.timeMinimumValue(), uint(100));
    QCOMPARE(minSpy.count(), 1);
    dial.setTimeMinimumValue(100);
    QCOMPARE(minSpy.count(), 1);

    QSignalSpy maxSpy(&dial, &VCSpeedDial::timeMaximumValueChanged);
    dial.setTimeMaximumValue(5000);
    QCOMPARE(dial.timeMaximumValue(), uint(5000));
    QCOMPARE(maxSpy.count(), 1);
    dial.setTimeMaximumValue(5000);
    QCOMPARE(maxSpy.count(), 1);

    QSignalSpy timeSpy(&dial, &VCSpeedDial::currentTimeChanged);
    dial.setCurrentTime(1234);
    QCOMPARE(dial.currentTime(), uint(1234));
    QCOMPARE(timeSpy.count(), 1);
    dial.setCurrentTime(1234);
    QCOMPARE(timeSpy.count(), 1);

    QSignalSpy resetSpy(&dial, &VCSpeedDial::resetOnDialChangeChanged);
    dial.setResetOnDialChange(true);
    QCOMPARE(dial.resetOnDialChange(), true);
    QCOMPARE(resetSpy.count(), 1);
    dial.setResetOnDialChange(true);
    QCOMPARE(resetSpy.count(), 1);

    QSignalSpy bpmSpy(&dial, &VCSpeedDial::controlBPMChanged);
    dial.setControlBPM(true);
    QCOMPARE(dial.controlBPM(), true);
    QCOMPARE(bpmSpy.count(), 1);
    dial.setControlBPM(true);
    QCOMPARE(bpmSpy.count(), 1);
}

void VCSpeedDial_Test::speedFactor()
{
    VCSpeedDial dial(m_env->doc());
    QSignalSpy spy(&dial, &VCSpeedDial::currentFactorChanged);

    dial.increaseSpeedFactor();
    QCOMPARE(dial.currentFactor(), VCSpeedDial::Two);
    QCOMPARE(spy.count(), 1);

    /* Sixteen is the upper limit */
    for (int i = 0; i < 10; i++)
        dial.increaseSpeedFactor();
    QCOMPARE(dial.currentFactor(), VCSpeedDial::Sixteen);

    /* One sixteenth is the lower limit */
    for (int i = 0; i < 20; i++)
        dial.decreaseSpeedFactor();
    QCOMPARE(dial.currentFactor(), VCSpeedDial::OneSixteenth);

    dial.setCurrentFactor(VCSpeedDial::Half);
    QCOMPARE(dial.currentFactor(), VCSpeedDial::Half);
}

void VCSpeedDial_Test::functions()
{
    Scene *scene = new Scene(m_env->doc());
    m_env->doc()->addFunction(scene);
    scene->setFadeInSpeed(0);
    scene->setDuration(0);

    VCSpeedDial dial(m_env->doc());
    QSignalSpy spy(&dial, &VCSpeedDial::functionsListChanged);

    dial.addFunction(scene->id());
    QCOMPARE(dial.functions().count(), 1);
    QCOMPARE(spy.count(), 1);

    /* Duplicates are ignored */
    dial.addFunction(scene->id());
    QCOMPARE(dial.functions().count(), 1);
    QCOMPARE(spy.count(), 1);

    /* By default only the duration is controlled */
    VCSpeedDial::VCSpeedDialFunction func = dial.functions().value(scene->id());
    QCOMPARE(func.m_fId, scene->id());
    QCOMPARE(func.m_fadeInFactor, VCSpeedDial::None);
    QCOMPARE(func.m_fadeOutFactor, VCSpeedDial::None);
    QCOMPARE(func.m_durationFactor, VCSpeedDial::One);

    QVariantList list = dial.functionsList().toList();
    QCOMPARE(list.count(), 1);
    QCOMPARE(list.first().toMap().value("funcID").toUInt(), scene->id());

    /* The time is applied to the attached functions */
    dial.setCurrentTime(2000);
    QCOMPARE(scene->duration(), uint(2000));
    QCOMPARE(scene->fadeInSpeed(), uint(0));

    dial.setCurrentFactor(VCSpeedDial::Two);
    QCOMPARE(scene->duration(), uint(4000));

    dial.setFunctionSpeed(scene->id(), Function::FadeIn, VCSpeedDial::Half);
    QCOMPARE(dial.functions().value(scene->id()).m_fadeInFactor, VCSpeedDial::Half);
    dial.applyFunctionsTime();
    QCOMPARE(scene->fadeInSpeed(), uint(2000));

    dial.removeFunction(scene->id());
    QVERIFY(dial.functions().isEmpty());
}

void VCSpeedDial_Test::presets()
{
    VCSpeedDial dial(m_env->doc());
    QSignalSpy spy(&dial, &VCSpeedDial::presetsListChanged);

    int id1 = dial.addPreset("Slow", 2000);
    int id2 = dial.addPreset("Fast", 250);
    QVERIFY(id1 != id2);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(dial.externalControlsCount(), 16);

    QVariantList list = dial.presetsList();
    QCOMPARE(list.count(), 2);
    QCOMPARE(list.at(0).toMap().value("id").toInt(), id1);
    QCOMPARE(list.at(0).toMap().value("name").toString(), QString("Slow"));
    QCOMPARE(list.at(0).toMap().value("value").toInt(), 2000);
    QCOMPARE(list.at(1).toMap().value("id").toInt(), id2);
    QCOMPARE(list.at(1).toMap().value("name").toString(), QString("Fast"));
    QCOMPARE(list.at(1).toMap().value("value").toInt(), 250);

    dial.setPresetName(id1, "Slower");
    QCOMPARE(dial.presetsList().at(0).toMap().value("name").toString(), QString("Slower"));
    dial.setPresetValue(id2, 300);
    QCOMPARE(dial.presetsList().at(1).toMap().value("value").toInt(), 300);
    QCOMPARE(dial.externalControlsCount(), 16);

    /* Unknown presets are ignored */
    dial.setPresetName(200, "Foo");
    dial.setPresetValue(200, 1);
    dial.removePreset(200);
    QCOMPARE(dial.presetsList().count(), 2);

    dial.removePreset(id1);
    QCOMPARE(dial.presetsList().count(), 1);
    QCOMPARE(dial.presetsList().first().toMap().value("id").toInt(), id2);
    QCOMPARE(dial.externalControlsCount(), 15);

    /* IDs are never reused */
    int id3 = dial.addPreset("New", 100);
    QVERIFY(id3 != id1);
    QVERIFY(id3 != id2);
}

void VCSpeedDial_Test::copy()
{
    Scene *scene = new Scene(m_env->doc());
    m_env->doc()->addFunction(scene);

    VCWidget parent(m_env->doc());

    VCSpeedDial dial(m_env->doc());
    dial.setCaption("Copied");
    dial.setVisibilityMask(VCSpeedDial::Dial);
    dial.setTimeMinimumValue(10);
    dial.setTimeMaximumValue(9000);
    dial.setResetOnDialChange(true);
    dial.setControlBPM(true);
    dial.addFunction(scene->id());
    dial.addPreset("P1", 500);
    dial.addPreset("P2", 1500);

    VCSpeedDial *copy = qobject_cast<VCSpeedDial *>(dial.createCopy(&parent));
    QVERIFY(copy != nullptr);
    QCOMPARE(copy->caption(), QString("Copied"));
    QCOMPARE(copy->visibilityMask(), quint32(VCSpeedDial::Dial));
    QCOMPARE(copy->timeMinimumValue(), uint(10));
    QCOMPARE(copy->timeMaximumValue(), uint(9000));
    QCOMPARE(copy->resetOnDialChange(), true);
    QCOMPARE(copy->controlBPM(), true);
    QCOMPARE(copy->functions().count(), 1);
    QVERIFY(copy->functions().contains(scene->id()));
    QCOMPARE(copy->presetsList(), dial.presetsList());
    QCOMPARE(copy->externalControlsCount(), dial.externalControlsCount());

    /* Presets added to the copy don't clash with the copied ones */
    int newId = copy->addPreset("P3", 2500);
    for (const QVariant &preset : dial.presetsList())
        QVERIFY(preset.toMap().value("id").toInt() != newId);
}

void VCSpeedDial_Test::saveLoad()
{
    Scene *scene = new Scene(m_env->doc());
    m_env->doc()->addFunction(scene);

    VCSpeedDial dial(m_env->doc());
    dial.setID(6);
    dial.setCaption("Speed");
    dial.setVisibilityMask(VCSpeedDial::Dial | VCSpeedDial::Tap);
    dial.setTimeMinimumValue(100);
    dial.setTimeMaximumValue(8000);
    dial.setCurrentTime(1500);
    dial.setResetOnDialChange(true);
    dial.setControlBPM(true);
    dial.addFunction(scene->id());
    dial.setFunctionSpeed(scene->id(), Function::FadeIn, VCSpeedDial::Half);
    dial.setFunctionSpeed(scene->id(), Function::Duration, VCSpeedDial::Two);
    int presetId = dial.addPreset("P", 500);

    QSharedPointer<QLCInputSource> src(new QLCInputSource(0, 7));
    src->setID(INPUT_DIAL_ID);
    dial.addInputSource(src);
    dial.addKeySequence(QKeySequence("T"), INPUT_TAP_ID);

    QByteArray data = vcSaveXML(&dial);
    VCXMLReader xml(data);
    QCOMPARE(xml.reader().name().toString(), QString("SpeedDial"));

    VCSpeedDial dial2(m_env->doc());
    QVERIFY(dial2.loadXML(xml.reader()));
    QCOMPARE(dial2.id(), quint32(6));
    QCOMPARE(dial2.caption(), QString("Speed"));
    QCOMPARE(dial2.visibilityMask(), quint32(VCSpeedDial::Dial | VCSpeedDial::Tap));
    QCOMPARE(dial2.timeMinimumValue(), uint(100));
    QCOMPARE(dial2.timeMaximumValue(), uint(8000));
    QCOMPARE(dial2.currentTime(), uint(1500));
    QCOMPARE(dial2.resetOnDialChange(), true);
    QCOMPARE(dial2.controlBPM(), true);

    QCOMPARE(dial2.functions().count(), 1);
    VCSpeedDial::VCSpeedDialFunction func = dial2.functions().value(scene->id());
    QCOMPARE(func.m_fId, scene->id());
    QCOMPARE(func.m_fadeInFactor, VCSpeedDial::Half);
    QCOMPARE(func.m_fadeOutFactor, VCSpeedDial::None);
    QCOMPARE(func.m_durationFactor, VCSpeedDial::Two);

    QVariantList presets = dial2.presetsList();
    QCOMPARE(presets.count(), 1);
    QCOMPARE(presets.first().toMap().value("id").toInt(), presetId);
    QCOMPARE(presets.first().toMap().value("name").toString(), QString("P"));
    QCOMPARE(presets.first().toMap().value("value").toInt(), 500);

    QCOMPARE(dial2.inputSources().count(), 1);
    QCOMPARE(dial2.inputSources().first()->id(), quint32(INPUT_DIAL_ID));
    QCOMPARE(dial2.inputSources().first()->channel(), quint32(7));
    QCOMPARE(dial2.keySequenceMap().value(QKeySequence("T")), quint32(INPUT_TAP_ID));

    /* Wrong tag */
    VCXMLReader wrongXml(QByteArray("<Button/>"));
    VCSpeedDial dial3(m_env->doc());
    QCOMPARE(dial3.loadXML(wrongXml.reader()), false);
}

QTEST_MAIN(VCSpeedDial_Test)
