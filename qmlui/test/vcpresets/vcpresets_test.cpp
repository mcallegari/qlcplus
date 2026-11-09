/*
  Q Light Controller Plus - Test Unit
  vcpresets_test.cpp

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

#include "vcspeeddialpreset.h"
#include "vcanimationpreset.h"
#include "vcxypadpreset.h"
#include "vcpresets_test.h"
#include "qlcinputsource.h"
#include "fixturegroup.h"
#include "vctestenv.h"
#include "vcwidget.h"
#include "function.h"

void VCPresets_Test::init()
{
    m_env = new VCTestEnv();
}

void VCPresets_Test::cleanup()
{
    delete m_env;
}

/*****************************************************************************
 * VCSpeedDialPreset
 *****************************************************************************/

void VCPresets_Test::speedDialPresetCopy()
{
    VCSpeedDialPreset preset(20);
    QCOMPARE(preset.m_id, quint8(20));
    QCOMPARE(preset.m_value, 1000);
    QVERIFY(preset.m_inputSource.isNull());

    preset.m_name = "A";
    preset.m_value = 500;
    preset.m_keySequence = QKeySequence("X");
    preset.m_inputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(1, 2));
    preset.m_inputSource->setFeedbackValue(QLCInputFeedback::LowerValue, 5);

    VCSpeedDialPreset copy(preset);
    QCOMPARE(copy.m_id, quint8(20));
    QCOMPARE(copy.m_name, QString("A"));
    QCOMPARE(copy.m_value, 500);
    QCOMPARE(copy.m_keySequence, QKeySequence("X"));

    /* The input source is duplicated, not shared */
    QVERIFY(copy.m_inputSource.isNull() == false);
    QVERIFY(copy.m_inputSource != preset.m_inputSource);
    QCOMPARE(copy.m_inputSource->universe(), quint32(1));
    QCOMPARE(copy.m_inputSource->channel(), quint32(2));
    QCOMPARE(copy.m_inputSource->feedbackValue(QLCInputFeedback::LowerValue), uchar(5));

    VCSpeedDialPreset other(21);
    QVERIFY(preset < other);
    QVERIFY((other < preset) == false);
    QVERIFY(VCSpeedDialPreset::compare(&preset, &other));
}

void VCPresets_Test::speedDialPresetSaveLoad()
{
    VCSpeedDialPreset preset(20);
    preset.m_name = "Slow";
    preset.m_value = 2500;
    preset.m_keySequence = QKeySequence("Ctrl+S");
    preset.m_inputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(1, 2));
    preset.m_inputSource->setFeedbackValue(QLCInputFeedback::LowerValue, 5);
    preset.m_inputSource->setFeedbackValue(QLCInputFeedback::UpperValue, 200);

    QByteArray data = vcSaveXML(&preset);
    VCXMLReader xml(data);
    QCOMPARE(xml.reader().name().toString(), QString("Preset"));

    VCSpeedDialPreset preset2(0);
    QVERIFY(preset2.loadXML(xml.reader()));
    QCOMPARE(preset2.m_id, quint8(20));
    QCOMPARE(preset2.m_name, QString("Slow"));
    QCOMPARE(preset2.m_value, 2500);
    QCOMPARE(preset2.m_keySequence, QKeySequence("Ctrl+S"));
    QVERIFY(preset2.m_inputSource.isNull() == false);
    QCOMPARE(preset2.m_inputSource->universe(), quint32(1));
    QCOMPARE(preset2.m_inputSource->channel(), quint32(2));
    QCOMPARE(preset2.m_inputSource->feedbackValue(QLCInputFeedback::LowerValue), uchar(5));
    QCOMPARE(preset2.m_inputSource->feedbackValue(QLCInputFeedback::UpperValue), uchar(200));

    /* No input source and no key: nothing is written */
    VCSpeedDialPreset plain(21);
    QByteArray plainData = vcSaveXML(&plain);
    QVERIFY(plainData.contains("<Input") == false);
    QVERIFY(plainData.contains("<Key>") == false);
}

void VCPresets_Test::speedDialPresetLoadErrors()
{
    VCSpeedDialPreset preset(0);

    VCXMLReader wrongTag(QByteArray("<Foo ID=\"1\"/>"));
    QCOMPARE(preset.loadXML(wrongTag.reader()), false);

    VCXMLReader noID(QByteArray("<Preset><Name>A</Name></Preset>"));
    QCOMPARE(preset.loadXML(noID.reader()), false);
    QVERIFY(preset.m_name.isEmpty());
}

/*****************************************************************************
 * VCXYPadPreset
 *****************************************************************************/

void VCPresets_Test::xyPadPresetTypes()
{
    QCOMPARE(VCXYPadPreset::typeToString(VCXYPadPreset::Position), QString("Position"));
    QCOMPARE(VCXYPadPreset::typeToString(VCXYPadPreset::EFX), QString("EFX"));
    QCOMPARE(VCXYPadPreset::typeToString(VCXYPadPreset::Scene), QString("Scene"));
    QCOMPARE(VCXYPadPreset::typeToString(VCXYPadPreset::FixtureGroup), QString("FixtureGroup"));

    QCOMPARE(VCXYPadPreset::stringToType("Position"), VCXYPadPreset::Position);
    QCOMPARE(VCXYPadPreset::stringToType("EFX"), VCXYPadPreset::EFX);
    QCOMPARE(VCXYPadPreset::stringToType("Scene"), VCXYPadPreset::Scene);
    QCOMPARE(VCXYPadPreset::stringToType("FixtureGroup"), VCXYPadPreset::FixtureGroup);
    QCOMPARE(VCXYPadPreset::stringToType("Foo"), VCXYPadPreset::Position);

    VCXYPadPreset preset(1);
    QCOMPARE(preset.m_type, VCXYPadPreset::Position);
    QCOMPARE(preset.m_funcID, Function::invalidId());
    QCOMPARE(preset.m_fxGroupID, ::FixtureGroup::invalidId());
    QCOMPARE(preset.m_dmxPos, QPointF(0, 0));
    QCOMPARE(preset.color(), QString("#BBBBBB"));
    preset.m_type = VCXYPadPreset::EFX;
    QCOMPARE(preset.color(), QString("#BBBB8D"));
    preset.m_type = VCXYPadPreset::Scene;
    QCOMPARE(preset.color(), QString("#BB8E8E"));
    preset.m_type = VCXYPadPreset::FixtureGroup;
    QCOMPARE(preset.color(), QString("#95BB95"));
}

void VCPresets_Test::xyPadPresetCopy()
{
    VCXYPadPreset preset(5);
    preset.m_type = VCXYPadPreset::FixtureGroup;
    preset.m_name = "Group";
    preset.m_dmxPos = QPointF(1, 2);
    preset.m_funcID = 3;
    preset.m_fxGroupID = 4;
    preset.m_fxGroup << GroupHead(1, 0) << GroupHead(2, 3);
    preset.m_keySequence = QKeySequence("G");
    preset.m_inputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(0, 8));

    VCXYPadPreset copy(preset);
    QCOMPARE(copy.m_id, quint8(5));
    QCOMPARE(copy.m_type, VCXYPadPreset::FixtureGroup);
    QCOMPARE(copy.m_name, QString("Group"));
    QCOMPARE(copy.m_dmxPos, QPointF(1, 2));
    QCOMPARE(copy.m_funcID, quint32(3));
    QCOMPARE(copy.m_fxGroupID, quint32(4));
    QCOMPARE(copy.m_fxGroup.count(), 2);
    QVERIFY(copy.m_fxGroup.at(1) == GroupHead(2, 3));
    QCOMPARE(copy.m_keySequence, QKeySequence("G"));
    QVERIFY(copy.m_inputSource.isNull() == false);
    QVERIFY(copy.m_inputSource != preset.m_inputSource);
    QCOMPARE(copy.m_inputSource->channel(), quint32(8));

    /* Assigning a preset without input source clears it */
    VCXYPadPreset empty(6);
    copy = empty;
    QCOMPARE(copy.m_id, quint8(6));
    QVERIFY(copy.m_inputSource.isNull());
    QVERIFY(copy.m_fxGroup.isEmpty());

    QVERIFY(preset < empty);
    QVERIFY(VCXYPadPreset::compare(&preset, &empty));
}

void VCPresets_Test::xyPadPresetSaveLoadPosition()
{
    VCXYPadPreset preset(16);
    preset.m_name = "Home";
    preset.m_dmxPos = QPointF(12.5, 200);
    preset.m_keySequence = QKeySequence("H");

    QByteArray data = vcSaveXML(&preset);
    QVERIFY(data.contains("<FuncID>") == false);

    VCXMLReader xml(data);
    VCXYPadPreset preset2(0);
    QVERIFY(preset2.loadXML(xml.reader()));
    QCOMPARE(preset2.m_id, quint8(16));
    QCOMPARE(preset2.m_type, VCXYPadPreset::Position);
    QCOMPARE(preset2.m_name, QString("Home"));
    QCOMPARE(preset2.m_dmxPos, QPointF(12.5, 200));
    QCOMPARE(preset2.m_keySequence, QKeySequence("H"));
    QVERIFY(preset2.m_inputSource.isNull());
}

void VCPresets_Test::xyPadPresetSaveLoadFunction()
{
    VCXYPadPreset preset(17);
    preset.m_type = VCXYPadPreset::EFX;
    preset.m_name = "Circle";
    preset.m_funcID = 7;
    preset.m_inputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(2, 9));

    QByteArray data = vcSaveXML(&preset);
    /* The position is saved only for position presets */
    QVERIFY(data.contains("<X>") == false);

    VCXMLReader xml(data);
    VCXYPadPreset preset2(0);
    QVERIFY(preset2.loadXML(xml.reader()));
    QCOMPARE(preset2.m_type, VCXYPadPreset::EFX);
    QCOMPARE(preset2.m_name, QString("Circle"));
    QCOMPARE(preset2.m_funcID, quint32(7));
    QCOMPARE(preset2.m_dmxPos, QPointF(0, 0));
    QVERIFY(preset2.m_inputSource.isNull() == false);
    QCOMPARE(preset2.m_inputSource->universe(), quint32(2));
    QCOMPARE(preset2.m_inputSource->channel(), quint32(9));

    VCXYPadPreset wrong(0);
    VCXMLReader wrongTag(QByteArray("<Foo ID=\"1\"/>"));
    QCOMPARE(wrong.loadXML(wrongTag.reader()), false);
    VCXMLReader noID(QByteArray("<Preset/>"));
    QCOMPARE(wrong.loadXML(noID.reader()), false);
}

void VCPresets_Test::xyPadPresetSaveLoadFixtures()
{
    /* Without a group ID, the single heads are saved */
    VCXYPadPreset preset(18);
    preset.m_type = VCXYPadPreset::FixtureGroup;
    preset.m_fxGroup << GroupHead(1, 0) << GroupHead(2, 3);

    QByteArray data = vcSaveXML(&preset);
    VCXMLReader xml(data);
    VCXYPadPreset preset2(0);
    QVERIFY(preset2.loadXML(xml.reader()));
    QCOMPARE(preset2.m_type, VCXYPadPreset::FixtureGroup);
    QCOMPARE(preset2.m_fxGroupID, ::FixtureGroup::invalidId());
    QCOMPARE(preset2.m_fxGroup.count(), 2);
    QVERIFY(preset2.m_fxGroup.at(0) == GroupHead(1, 0));
    QVERIFY(preset2.m_fxGroup.at(1) == GroupHead(2, 3));

    /* With a group ID, only the group is referenced */
    preset.m_fxGroupID = 4;
    data = vcSaveXML(&preset);
    QVERIFY(data.contains("<Fixture") == false);

    VCXMLReader xml2(data);
    VCXYPadPreset preset3(0);
    QVERIFY(preset3.loadXML(xml2.reader()));
    QCOMPARE(preset3.m_fxGroupID, quint32(4));
    QVERIFY(preset3.m_fxGroup.isEmpty());
}

/*****************************************************************************
 * VCAnimationPreset
 *****************************************************************************/

void VCPresets_Test::animationPresetTypes()
{
    QList<VCAnimationPreset::ControlType> types;
    types << VCAnimationPreset::Color1 << VCAnimationPreset::Color2 << VCAnimationPreset::Color3
          << VCAnimationPreset::Color4 << VCAnimationPreset::Color5
          << VCAnimationPreset::Color1Knob << VCAnimationPreset::Color2Knob << VCAnimationPreset::Color3Knob
          << VCAnimationPreset::Color4Knob << VCAnimationPreset::Color5Knob
          << VCAnimationPreset::Color1Reset << VCAnimationPreset::Color2Reset << VCAnimationPreset::Color3Reset
          << VCAnimationPreset::Color4Reset << VCAnimationPreset::Color5Reset
          << VCAnimationPreset::Animation << VCAnimationPreset::Text;

    /* Every type survives a round trip through its string */
    for (VCAnimationPreset::ControlType type : types)
    {
        QString str = VCAnimationPreset::typeToString(type);
        QVERIFY(str.isEmpty() == false);
        QCOMPARE(int(VCAnimationPreset::stringToType(str)), int(type));
    }

    QCOMPARE(VCAnimationPreset::typeToString(VCAnimationPreset::Color1Reset), QString("ResetColor1"));

    /* Legacy names */
    QCOMPARE(int(VCAnimationPreset::stringToType("StartColor")), int(VCAnimationPreset::Color1));
    QCOMPARE(int(VCAnimationPreset::stringToType("EndColor")), int(VCAnimationPreset::Color2));
    QCOMPARE(int(VCAnimationPreset::stringToType("StartColorKnob")), int(VCAnimationPreset::Color1Knob));
    QCOMPARE(int(VCAnimationPreset::stringToType("EndColorKnob")), int(VCAnimationPreset::Color2Knob));
    QCOMPARE(int(VCAnimationPreset::stringToType("Foo")), int(VCAnimationPreset::Color1));

    VCAnimationPreset preset(30);
    QCOMPARE(int(preset.m_type), int(VCAnimationPreset::Color1));
    QCOMPARE(int(preset.widgetType()), int(VCAnimationPreset::Button));
    QCOMPARE(preset.colorIndex(), 0);

    preset.m_type = VCAnimationPreset::Color4Knob;
    QCOMPARE(int(preset.widgetType()), int(VCAnimationPreset::Knob));
    QCOMPARE(preset.colorIndex(), 3);

    preset.m_type = VCAnimationPreset::Color5Reset;
    QCOMPARE(int(preset.widgetType()), int(VCAnimationPreset::Button));
    QCOMPARE(preset.colorIndex(), 4);

    preset.m_type = VCAnimationPreset::Text;
    QCOMPARE(preset.colorIndex(), -1);
    preset.m_type = VCAnimationPreset::Animation;
    QCOMPARE(preset.colorIndex(), -1);

    VCAnimationPreset other(31);
    QVERIFY(preset < other);
    QVERIFY(VCAnimationPreset::compare(&preset, &other));
}

void VCPresets_Test::animationPresetColors()
{
    VCAnimationPreset preset(30);
    QRgb rgb = qRgb(10, 20, 30);

    preset.m_color = QColor(Qt::red);
    QCOMPARE(preset.rgbToValue(rgb), quint8(10));
    QCOMPARE(preset.valueToRgb(100), qRgb(100, 0, 0));

    preset.m_color = QColor(Qt::green);
    QCOMPARE(preset.rgbToValue(rgb), quint8(20));
    QCOMPARE(preset.valueToRgb(100), qRgb(0, 100, 0));

    preset.m_color = QColor(Qt::blue);
    QCOMPARE(preset.rgbToValue(rgb), quint8(30));
    QCOMPARE(preset.valueToRgb(100), qRgb(0, 0, 100));

    /* Only the primary colors map to a component */
    preset.m_color = QColor(Qt::yellow);
    QCOMPARE(preset.rgbToValue(rgb), quint8(0));
    QCOMPARE(preset.valueToRgb(100), QRgb(0));

    /* Copies are deep */
    preset.m_properties["foo"] = "bar";
    VCAnimationPreset copy(preset);
    QCOMPARE(copy.m_id, quint8(30));
    QCOMPARE(copy.m_color, QColor(Qt::yellow));
    QCOMPARE(copy.m_properties.value("foo"), QString("bar"));
}

void VCPresets_Test::animationPresetSaveLoad()
{
    VCWidget widget(m_env->doc());
    widget.registerExternalControl(30, "Preset", true);
    widget.addKeySequence(QKeySequence("K"), 30);

    VCAnimationPreset preset(30);
    preset.m_type = VCAnimationPreset::Color2;
    preset.m_color = QColor(Qt::green);
    preset.m_properties["foo"] = "bar";

    VCAnimationPreset text(31);
    text.m_type = VCAnimationPreset::Text;
    text.m_resource = "Hello";

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartElement("Root");
    QVERIFY(preset.saveXML(&xmlWriter, &widget));
    QVERIFY(text.saveXML(&xmlWriter, &widget));
    xmlWriter.writeEndElement();
    xmlWriter.writeEndDocument();
    buffer.close();

    /* Color presets carry a color, the others a resource */
    QVERIFY(buffer.data().contains("<Color>#00ff00</Color>"));
    QVERIFY(buffer.data().contains("<Resource>Hello</Resource>"));

    VCXMLReader xml(buffer.data());
    VCWidget widget2(m_env->doc());
    widget2.registerExternalControl(30, "Preset", true);

    QVERIFY(xml.reader().readNextStartElement());
    VCAnimationPreset preset2(0xff);
    QVERIFY(preset2.loadXML(xml.reader(), &widget2));
    QCOMPARE(preset2.m_id, quint8(30));
    QCOMPARE(int(preset2.m_type), int(VCAnimationPreset::Color2));
    QCOMPARE(preset2.m_color, QColor(Qt::green));
    QCOMPARE(preset2.m_properties.value("foo"), QString("bar"));
    QCOMPARE(widget2.keySequenceMap().value(QKeySequence("K")), quint32(30));

    QVERIFY(xml.reader().readNextStartElement());
    VCAnimationPreset text2(0xff);
    QVERIFY(text2.loadXML(xml.reader(), &widget2));
    QCOMPARE(text2.m_id, quint8(31));
    QCOMPARE(int(text2.m_type), int(VCAnimationPreset::Text));
    QCOMPARE(text2.m_resource, QString("Hello"));

    VCAnimationPreset wrong(0);
    VCXMLReader wrongTag(QByteArray("<Preset ID=\"1\"/>"));
    QCOMPARE(wrong.loadXML(wrongTag.reader(), &widget2), false);
    VCXMLReader noID(QByteArray("<Control/>"));
    QCOMPARE(wrong.loadXML(noID.reader(), &widget2), false);
}

QTEST_MAIN(VCPresets_Test)
