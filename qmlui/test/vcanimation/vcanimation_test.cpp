/*
  Q Light Controller Plus - Test Unit
  vcanimation_test.cpp

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

#include "vcanimation_test.h"
#include "vcanimationpreset.h"
#include "qlcinputsource.h"
#include "vcanimation.h"
#include "vctestenv.h"
#include "rgbmatrix.h"
#include "scene.h"

/* External control IDs, as defined in vcanimation.cpp */
#define INPUT_FADER_ID  0

void VCAnimation_Test::init()
{
    m_env = new VCTestEnv();
}

void VCAnimation_Test::cleanup()
{
    delete m_env;
}

void VCAnimation_Test::initial()
{
    VCAnimation anim(m_env->doc());

    QCOMPARE(anim.type(), int(VCWidget::AnimationWidget));
    QCOMPARE(anim.functionID(), Function::invalidId());
    QCOMPARE(anim.faderLevel(), 0);
    QCOMPARE(anim.instantChanges(), true);
    QCOMPARE(anim.visibilityMask(), anim.defaultVisibilityMask());
    QCOMPARE(anim.defaultVisibilityMask(),
             quint32(VCAnimation::Fader | VCAnimation::Label | VCAnimation::Color1 |
                     VCAnimation::Color2 | VCAnimation::PresetCombo));
    QVERIFY(anim.presetsList().isEmpty());
    QCOMPARE(anim.activePresetId(), -1);
    QCOMPARE(anim.supportsPresets(), true);
    QCOMPARE(anim.presetsResource(), QString("qrc:/VCAnimationPresets.qml"));
    QCOMPARE(anim.propertiesResource(), QString("qrc:/VCAnimationProperties.qml"));
    QCOMPARE(anim.externalControlsCount(), 1);
    QVERIFY(anim.colorAt(0).isValid() == false);
    QVERIFY(anim.colorAt(-1).isValid() == false);

    anim.setID(0);
    QCOMPARE(anim.caption(), QString("Animation 1"));
}

void VCAnimation_Test::properties()
{
    VCAnimation anim(m_env->doc());

    QSignalSpy maskSpy(&anim, &VCAnimation::visibilityMaskChanged);
    anim.setVisibilityMask(VCAnimation::Fader | VCAnimation::Color3);
    QCOMPARE(anim.visibilityMask(), quint32(VCAnimation::Fader | VCAnimation::Color3));
    QCOMPARE(maskSpy.count(), 1);
    anim.setVisibilityMask(VCAnimation::Fader | VCAnimation::Color3);
    QCOMPARE(maskSpy.count(), 1);

    QSignalSpy instantSpy(&anim, &VCAnimation::instantChangesChanged);
    anim.setInstantChanges(false);
    QCOMPARE(anim.instantChanges(), false);
    QCOMPARE(instantSpy.count(), 1);
    anim.setInstantChanges(false);
    QCOMPARE(instantSpy.count(), 1);

    /* Without a matrix, the fader does nothing */
    anim.setFaderLevel(100);
    QCOMPARE(anim.faderLevel(), 0);
}

void VCAnimation_Test::functionAttachment()
{
    Doc *doc = m_env->doc();

    RGBMatrix *matrix = new RGBMatrix(doc);
    doc->addFunction(matrix);
    Scene *scene = new Scene(doc);
    doc->addFunction(scene);

    VCAnimation anim(doc);
    QSignalSpy spy(&anim, &VCAnimation::functionIDChanged);

    anim.setFunctionID(matrix->id());
    QCOMPARE(anim.functionID(), matrix->id());
    QCOMPARE(spy.count(), 1);

    /* The local colors are taken from the matrix */
    QCOMPARE(anim.getColor1(), matrix->getColor(0));
    QCOMPARE(anim.colorAt(0), matrix->getColor(0));
    QCOMPARE(anim.algorithmIndex(), matrix->algorithmIndex());

    anim.setFunctionID(matrix->id());
    QCOMPARE(spy.count(), 1);

    /* Only RGB matrices can be attached */
    anim.setFunctionID(scene->id());
    QCOMPARE(anim.functionID(), Function::invalidId());
    QVERIFY(anim.getColor1().isValid() == false);
    QCOMPARE(spy.count(), 2);
}

void VCAnimation_Test::presets()
{
    VCAnimation anim(m_env->doc());
    QSignalSpy spy(&anim, &VCAnimation::presetsListChanged);

    /* Invalid presets are refused */
    QCOMPARE(anim.addColorPreset(-1, QColor(Qt::red)), -1);
    QCOMPARE(anim.addColorPreset(RGBAlgorithmColorDisplayCount, QColor(Qt::red)), -1);
    QCOMPARE(anim.addColorPreset(0, QColor()), -1);
    QCOMPARE(anim.addTextPreset(QString()), -1);
    QCOMPARE(anim.addColorKnobsPreset(-1), -1);
    QCOMPARE(spy.count(), 0);

    int colorId = anim.addColorPreset(0, QColor(Qt::red));
    int textId = anim.addTextPreset("Hello");
    int lastKnobId = anim.addColorKnobsPreset(1);
    QVERIFY(colorId != -1);
    QVERIFY(textId != -1);
    QCOMPARE(lastKnobId, textId + 3);
    QCOMPARE(spy.count(), 3);

    /* Fader + one control per preset */
    QCOMPARE(anim.externalControlsCount(), 6);

    QVariantList list = anim.presetsList();
    QCOMPARE(list.count(), 5);

    QVariantMap color = list.at(0).toMap();
    QCOMPARE(color.value("id").toInt(), colorId);
    QCOMPARE(color.value("type").toInt(), int(VCAnimationPreset::Color1));
    QCOMPARE(color.value("typeString").toString(), QString("Color1"));
    QCOMPARE(color.value("isKnob").toBool(), false);
    QCOMPARE(color.value("colorIndex").toInt(), 0);
    QCOMPARE(color.value("color").value<QColor>(), QColor(Qt::red));

    QVariantMap text = list.at(1).toMap();
    QCOMPARE(text.value("id").toInt(), textId);
    QCOMPARE(text.value("typeString").toString(), QString("Text"));
    QCOMPARE(text.value("resource").toString(), QString("Hello"));
    QCOMPARE(text.value("colorIndex").toInt(), -1);

    /* One knob per RGB component */
    QList<QColor> knobColors;
    for (int i = 2; i < 5; i++)
    {
        QVariantMap knob = list.at(i).toMap();
        QCOMPARE(knob.value("type").toInt(), int(VCAnimationPreset::Color2Knob));
        QCOMPARE(knob.value("isKnob").toBool(), true);
        QCOMPARE(knob.value("colorIndex").toInt(), 1);
        QVERIFY(knob.value("tooltip").toString().isEmpty() == false);
        knobColors.append(knob.value("color").value<QColor>());
    }
    QCOMPARE(knobColors, QList<QColor>() << QColor(Qt::red) << QColor(Qt::green) << QColor(Qt::blue));

    /* Moving up swaps the IDs */
    int movedId = anim.movePresetUp(textId);
    list = anim.presetsList();
    QCOMPARE(list.at(0).toMap().value("typeString").toString(), QString("Text"));
    QCOMPARE(list.at(1).toMap().value("typeString").toString(), QString("Color1"));
    QCOMPARE(anim.movePresetUp(colorId), colorId);

    /* The QML presets page selects the returned ID, which should be the new
     * ID of the moved preset (as VCXYPad does) */
    QEXPECT_FAIL("", "movePresetUp() returns the old ID of the moved preset", Continue);
    QCOMPARE(movedId, colorId);

    /* colorId is now the text preset */
    anim.removePreset(colorId);
    list = anim.presetsList();
    QCOMPARE(list.count(), 4);
    QCOMPARE(list.at(0).toMap().value("typeString").toString(), QString("Color1"));
    QCOMPARE(anim.externalControlsCount(), 5);

    anim.removePreset(200);
    QCOMPARE(anim.presetsList().count(), 4);
}

void VCAnimation_Test::copy()
{
    RGBMatrix *matrix = new RGBMatrix(m_env->doc());
    m_env->doc()->addFunction(matrix);

    VCWidget parent(m_env->doc());

    VCAnimation anim(m_env->doc());
    anim.setCaption("Copied");
    anim.setFunctionID(matrix->id());
    anim.setInstantChanges(false);
    anim.setVisibilityMask(VCAnimation::Fader | VCAnimation::Color5);
    anim.addColorPreset(2, QColor(Qt::blue));
    anim.addTextPreset("Foo");

    VCAnimation *copy = qobject_cast<VCAnimation *>(anim.createCopy(&parent));
    QVERIFY(copy != nullptr);
    QCOMPARE(copy->caption(), QString("Copied"));
    QCOMPARE(copy->functionID(), matrix->id());
    QCOMPARE(copy->instantChanges(), false);
    QCOMPARE(copy->visibilityMask(), quint32(VCAnimation::Fader | VCAnimation::Color5));
    QCOMPARE(copy->presetsList(), anim.presetsList());
    QCOMPARE(copy->externalControlsCount(), anim.externalControlsCount());

    /* New presets don't clash with the copied ones */
    int newId = copy->addTextPreset("Bar");
    for (const QVariant &preset : anim.presetsList())
        QVERIFY(preset.toMap().value("id").toInt() != newId);
}

void VCAnimation_Test::saveLoad()
{
    RGBMatrix *matrix = new RGBMatrix(m_env->doc());
    m_env->doc()->addFunction(matrix);

    VCAnimation anim(m_env->doc());
    anim.setID(12);
    anim.setCaption("Anim");
    anim.setFunctionID(matrix->id());
    anim.setVisibilityMask(VCAnimation::Fader | VCAnimation::Color3);
    int colorId = anim.addColorPreset(1, QColor(Qt::green));
    int textId = anim.addTextPreset("Hi");

    QSharedPointer<QLCInputSource> src(new QLCInputSource(0, 9));
    src->setID(INPUT_FADER_ID);
    anim.addInputSource(src);
    anim.addKeySequence(QKeySequence("P"), colorId);

    QByteArray data = vcSaveXML(&anim);
    VCXMLReader xml(data);
    QCOMPARE(xml.reader().name().toString(), QString("Matrix"));

    VCAnimation anim2(m_env->doc());
    QVERIFY(anim2.loadXML(xml.reader()));
    QCOMPARE(anim2.id(), quint32(12));
    QCOMPARE(anim2.caption(), QString("Anim"));
    QCOMPARE(anim2.functionID(), matrix->id());
    QCOMPARE(anim2.instantChanges(), true);
    QCOMPARE(anim2.visibilityMask(), quint32(VCAnimation::Fader | VCAnimation::Color3));

    QVariantList list = anim2.presetsList();
    QCOMPARE(list.count(), 2);
    QCOMPARE(list.at(0).toMap().value("id").toInt(), colorId);
    QCOMPARE(list.at(0).toMap().value("type").toInt(), int(VCAnimationPreset::Color2));
    QCOMPARE(list.at(0).toMap().value("color").value<QColor>(), QColor(Qt::green));
    QCOMPARE(list.at(1).toMap().value("id").toInt(), textId);
    QCOMPARE(list.at(1).toMap().value("resource").toString(), QString("Hi"));
    QCOMPARE(anim2.externalControlsCount(), 3);

    QCOMPARE(anim2.inputSources().count(), 1);
    QCOMPARE(anim2.inputSources().first()->id(), quint32(INPUT_FADER_ID));
    QCOMPARE(anim2.inputSources().first()->channel(), quint32(9));
    QCOMPARE(anim2.keySequenceMap().value(QKeySequence("P")), quint32(colorId));

    /* Wrong tag */
    VCXMLReader wrongXml(QByteArray("<Animation/>"));
    VCAnimation anim3(m_env->doc());
    QCOMPARE(anim3.loadXML(wrongXml.reader()), false);
}

void VCAnimation_Test::saveLoadInstantChanges()
{
    VCAnimation anim(m_env->doc());
    anim.setInstantChanges(false);

    QByteArray data = vcSaveXML(&anim);
    QVERIFY(data.contains("InstantApply") == false);

    VCXMLReader xml(data);
    VCAnimation anim2(m_env->doc());
    QVERIFY(anim2.loadXML(xml.reader()));

    /* InstantApply is only written when true, and loadXML never resets it */
    QEXPECT_FAIL("", "instantChanges = false is not restored, since it defaults to true", Continue);
    QCOMPARE(anim2.instantChanges(), false);
}

QTEST_MAIN(VCAnimation_Test)
