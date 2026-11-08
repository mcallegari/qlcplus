/*
  Q Light Controller Plus - Test Unit
  vcslider_test.cpp

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

#include "vcslider_test.h"
#include "inputoutputmap.h"
#include "qlcinputsource.h"
#include "vctestenv.h"
#include "vcbutton.h"
#include "vcslider.h"
#include "vcframe.h"
#include "scene.h"

/* External control IDs, as defined in vcslider.cpp */
#define INPUT_SLIDER_CONTROL_ID     0
#define INPUT_SLIDER_RESET_ID       1
#define INPUT_SLIDER_FLASH_ID       2

void VCSlider_Test::init()
{
    m_env = new VCTestEnv();
}

void VCSlider_Test::cleanup()
{
    delete m_env;
}

void VCSlider_Test::initial()
{
    VCSlider slider(m_env->doc());

    QCOMPARE(slider.type(), int(VCWidget::SliderWidget));
    QCOMPARE(slider.widgetStyle(), VCSlider::WSlider);
    QCOMPARE(slider.valueDisplayStyle(), VCSlider::DMXValue);
    QCOMPARE(slider.invertedAppearance(), false);
    QCOMPARE(slider.catchValues(), false);
    QCOMPARE(slider.sliderMode(), VCSlider::Adjust);
    QCOMPARE(slider.value(), 0);
    QCOMPARE(slider.rangeLowLimit(), qreal(0));
    QCOMPARE(slider.rangeHighLimit(), qreal(255));
    QCOMPARE(slider.monitorEnabled(), false);
    QCOMPARE(slider.isOverriding(), false);
    QCOMPARE(slider.controlledFunction(), Function::invalidId());
    QCOMPARE(slider.adjustFlashEnabled(), false);
    QCOMPARE(slider.clickAndGoType(), VCSlider::CnGNone);
    QVERIFY(slider.levelChannels().isEmpty());
    QVERIFY(slider.availableAttributes().isEmpty());
    QCOMPARE(slider.externalControlsCount(), 3);
    QCOMPARE(slider.propertiesResource(), QString("qrc:/VCSliderProperties.qml"));

    slider.setID(0);
    QCOMPARE(slider.caption(), QString("Slider 1"));
    slider.setWidgetStyle(VCSlider::WKnob);
    QCOMPARE(slider.defaultCaption(), QString("Knob 1"));

    slider.setupLookAndFeel(10, 1);
    QCOMPARE(slider.page(), 1);
    QCOMPARE(slider.backgroundColor(), QColor("#444"));
}

void VCSlider_Test::stringConversions()
{
    VCSlider slider(m_env->doc());

    QCOMPARE(slider.widgetStyleToString(VCSlider::WSlider), QString("Slider"));
    QCOMPARE(slider.widgetStyleToString(VCSlider::WKnob), QString("Knob"));
    QCOMPARE(slider.stringToWidgetStyle("Slider"), VCSlider::WSlider);
    QCOMPARE(slider.stringToWidgetStyle("Knob"), VCSlider::WKnob);
    QCOMPARE(slider.stringToWidgetStyle("Foo"), VCSlider::WSlider);

    QCOMPARE(VCSlider::valueDisplayStyleToString(VCSlider::DMXValue), QString("Exact"));
    QCOMPARE(VCSlider::valueDisplayStyleToString(VCSlider::PercentageValue), QString("Percentage"));
    QCOMPARE(VCSlider::stringToValueDisplayStyle("Exact"), VCSlider::DMXValue);
    QCOMPARE(VCSlider::stringToValueDisplayStyle("Percentage"), VCSlider::PercentageValue);
    QCOMPARE(VCSlider::stringToValueDisplayStyle("Foo"), VCSlider::DMXValue);

    QCOMPARE(VCSlider::sliderModeToString(VCSlider::Level), QString("Level"));
    QCOMPARE(VCSlider::sliderModeToString(VCSlider::Adjust), QString("Adjust"));
    QCOMPARE(VCSlider::sliderModeToString(VCSlider::Submaster), QString("Submaster"));
    QCOMPARE(VCSlider::sliderModeToString(VCSlider::GrandMaster), QString("GrandMaster"));
    QCOMPARE(VCSlider::stringToSliderMode("Level"), VCSlider::Level);
    QCOMPARE(VCSlider::stringToSliderMode("Submaster"), VCSlider::Submaster);
    QCOMPARE(VCSlider::stringToSliderMode("GrandMaster"), VCSlider::GrandMaster);
    QCOMPARE(VCSlider::stringToSliderMode("Adjust"), VCSlider::Adjust);
    /* Legacy "Playback" and unknown modes become Adjust */
    QCOMPARE(VCSlider::stringToSliderMode("Playback"), VCSlider::Adjust);

    QCOMPARE(VCSlider::clickAndGoTypeToString(VCSlider::CnGNone), QString("None"));
    QCOMPARE(VCSlider::clickAndGoTypeToString(VCSlider::CnGColors), QString("Colors"));
    QCOMPARE(VCSlider::clickAndGoTypeToString(VCSlider::CnGPreset), QString("Preset"));
    QCOMPARE(VCSlider::stringToClickAndGoType("Colors"), VCSlider::CnGColors);
    QCOMPARE(VCSlider::stringToClickAndGoType("RGB"), VCSlider::CnGColors);
    QCOMPARE(VCSlider::stringToClickAndGoType("CMY"), VCSlider::CnGColors);
    QCOMPARE(VCSlider::stringToClickAndGoType("Preset"), VCSlider::CnGPreset);
    QCOMPARE(VCSlider::stringToClickAndGoType("Foo"), VCSlider::CnGNone);
}

void VCSlider_Test::sliderMode()
{
    VCSlider slider(m_env->doc());
    QSignalSpy spy(&slider, &VCSlider::sliderModeChanged);

    slider.setSliderMode(VCSlider::Level);
    QCOMPARE(slider.sliderMode(), VCSlider::Level);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(slider.value(), 0);

    /* Changing mode resets the range */
    slider.setRangeLowLimit(10);
    slider.setRangeHighLimit(100);
    slider.setMonitorEnabled(true);
    slider.setSliderMode(VCSlider::Submaster);
    QCOMPARE(slider.sliderMode(), VCSlider::Submaster);
    QCOMPARE(slider.rangeLowLimit(), qreal(0));
    QCOMPARE(slider.rangeHighLimit(), qreal(255));
    QCOMPARE(slider.value(), 255);
    QCOMPARE(slider.valueDisplayStyle(), VCSlider::PercentageValue);
    QCOMPARE(slider.monitorEnabled(), false);

    slider.setSliderMode(VCSlider::Adjust);
    QCOMPARE(slider.sliderMode(), VCSlider::Adjust);
    QCOMPARE(slider.value(), 0);
}

void VCSlider_Test::value()
{
    VCSlider slider(m_env->doc());
    slider.setSliderMode(VCSlider::Level);

    QSignalSpy spy(&slider, &VCSlider::valueChanged);
    slider.setValue(100);
    QCOMPARE(slider.value(), 100);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), 100);
    slider.setValue(100);
    QCOMPARE(spy.count(), 1);

    QSignalSpy lowSpy(&slider, &VCSlider::rangeLowLimitChanged);
    QSignalSpy highSpy(&slider, &VCSlider::rangeHighLimitChanged);
    slider.setRangeLowLimit(20);
    slider.setRangeHighLimit(200);
    QCOMPARE(slider.rangeLowLimit(), qreal(20));
    QCOMPARE(slider.rangeHighLimit(), qreal(200));
    QCOMPARE(lowSpy.count(), 1);
    QCOMPARE(highSpy.count(), 1);
    slider.setRangeLowLimit(20);
    slider.setRangeHighLimit(200);
    QCOMPARE(lowSpy.count(), 1);
    QCOMPARE(highSpy.count(), 1);

    /* A manual change while monitoring overrides the channels */
    slider.setMonitorEnabled(true);
    QCOMPARE(slider.monitorEnabled(), true);
    slider.setValue(50);
    QCOMPARE(slider.isOverriding(), true);

    /* Overriding is meaningful only while monitoring */
    slider.setMonitorEnabled(false);
    QCOMPARE(slider.isOverriding(), false);

    slider.setInvertedAppearance(true);
    QCOMPARE(slider.invertedAppearance(), true);
    slider.setValueDisplayStyle(VCSlider::PercentageValue);
    QCOMPARE(slider.valueDisplayStyle(), VCSlider::PercentageValue);
    slider.setClickAndGoType(VCSlider::CnGColors);
    QCOMPARE(slider.clickAndGoType(), VCSlider::CnGColors);
}

void VCSlider_Test::levelChannels()
{
    VCSlider slider(m_env->doc());

    slider.addLevelChannel(1, 2);
    slider.addLevelChannel(1, 2);
    slider.addLevelChannel(0, 5);
    QCOMPARE(slider.levelChannels().count(), 2);
    QVERIFY(slider.levelChannels().contains(SceneValue(1, 2)));
    QVERIFY(slider.levelChannels().contains(SceneValue(0, 5)));

    slider.removeLevelChannel(1, 2);
    QCOMPARE(slider.levelChannels().count(), 1);
    slider.removeLevelChannel(7, 7);
    QCOMPARE(slider.levelChannels().count(), 1);

    slider.clearLevelChannels();
    QVERIFY(slider.levelChannels().isEmpty());
}

void VCSlider_Test::controlledFunction()
{
    Scene *scene = new Scene(m_env->doc());
    scene->setName("MyScene");
    m_env->doc()->addFunction(scene);

    VCSlider slider(m_env->doc());
    slider.setID(0);
    slider.setSliderMode(VCSlider::Level);

    QSignalSpy spy(&slider, &VCSlider::controlledFunctionChanged);

    /* Attaching a function switches to Adjust mode */
    slider.setControlledFunction(scene->id());
    QCOMPARE(slider.controlledFunction(), scene->id());
    QCOMPARE(slider.sliderMode(), VCSlider::Adjust);
    QCOMPARE(slider.controlledAttribute(), int(Function::Intensity));
    QCOMPARE(slider.caption(), QString("MyScene"));
    QCOMPARE(slider.availableAttributes().count(), scene->attributes().count());
    QCOMPARE(spy.count(), 1);

    slider.setControlledFunction(scene->id());
    QCOMPARE(spy.count(), 1);

    /* Out of range attributes are refused */
    slider.setControlledAttribute(scene->attributes().count());
    QCOMPARE(slider.controlledAttribute(), int(Function::Intensity));

    slider.setAdjustFlashEnabled(true);
    QCOMPARE(slider.adjustFlashEnabled(), true);

    /* Submaster mode detaches the function and disables flashing */
    slider.setSliderMode(VCSlider::Submaster);
    QCOMPARE(slider.controlledFunction(), Function::invalidId());
    QCOMPARE(slider.adjustFlashEnabled(), false);

    slider.setSliderMode(VCSlider::Adjust);
    slider.setControlledFunction(scene->id());
    slider.setControlledFunction(12345);
    QCOMPARE(slider.controlledFunction(), Function::invalidId());
}

void VCSlider_Test::submaster()
{
    VCFrame frame(m_env->doc(), m_env->vc());
    VCSlider *submaster = qobject_cast<VCSlider *>(frame.addWidget(nullptr, "Slider", QPoint()));
    VCWidget *btn = frame.addWidget(nullptr, "Button", QPoint());
    QVERIFY(submaster != nullptr && btn != nullptr);

    QSignalSpy spy(submaster, &VCSlider::submasterValueChanged);
    submaster->setSliderMode(VCSlider::Submaster);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toReal(), qreal(1.0));
    QCOMPARE(btn->intensity(), qreal(1.0));

    /* The submaster value scales the intensity of the frame siblings */
    submaster->setValue(51);
    QCOMPARE(spy.count(), 2);
    QVERIFY(qAbs(spy.at(1).at(0).toReal() - 0.2) < 0.0001);
    QVERIFY(qAbs(btn->intensity() - 0.2) < 0.0001);

    /* ...but not its own */
    QCOMPARE(submaster->intensity(), qreal(1.0));

    submaster->setValue(255);
    QVERIFY(qAbs(btn->intensity() - 1.0) < 0.0001);
}

void VCSlider_Test::grandMaster()
{
    InputOutputMap *ioMap = m_env->doc()->inputOutputMap();

    VCSlider slider(m_env->doc());
    slider.setSliderMode(VCSlider::GrandMaster);
    QCOMPARE(slider.value(), 255);
    QCOMPARE(ioMap->grandMasterValue(), uchar(255));

    slider.setValue(100);
    QCOMPARE(ioMap->grandMasterValue(), uchar(100));

    slider.setGrandMasterValueMode(GrandMaster::Limit);
    QCOMPARE(ioMap->grandMasterValueMode(), GrandMaster::Limit);
    QCOMPARE(slider.grandMasterValueMode(), GrandMaster::Limit);
    slider.setGrandMasterValueMode(GrandMaster::Reduce);
    QCOMPARE(slider.grandMasterValueMode(), GrandMaster::Reduce);

    slider.setGrandMasterChannelMode(GrandMaster::AllChannels);
    QCOMPARE(slider.grandMasterChannelMode(), GrandMaster::AllChannels);
    slider.setGrandMasterChannelMode(GrandMaster::Intensity);
    QCOMPARE(slider.grandMasterChannelMode(), GrandMaster::Intensity);
}

void VCSlider_Test::inputValue()
{
    VCSlider slider(m_env->doc());
    slider.setSliderMode(VCSlider::Level);

    slider.slotInputValueChanged(INPUT_SLIDER_CONTROL_ID, 255);
    QCOMPARE(slider.value(), 255);
    slider.slotInputValueChanged(INPUT_SLIDER_CONTROL_ID, 0);
    QCOMPARE(slider.value(), 0);

    /* Input values are scaled to the slider range */
    slider.setRangeLowLimit(100);
    slider.setRangeHighLimit(200);
    slider.slotInputValueChanged(INPUT_SLIDER_CONTROL_ID, 0);
    QCOMPARE(slider.value(), 100);
    slider.slotInputValueChanged(INPUT_SLIDER_CONTROL_ID, 255);
    QCOMPARE(slider.value(), 200);

    /* The reset control stops overriding the monitored channels */
    slider.setMonitorEnabled(true);
    slider.setValue(150);
    QCOMPARE(slider.isOverriding(), true);
    slider.slotInputValueChanged(INPUT_SLIDER_RESET_ID, 0);
    QCOMPARE(slider.isOverriding(), true);
    slider.slotInputValueChanged(INPUT_SLIDER_RESET_ID, 255);
    QCOMPARE(slider.isOverriding(), false);
}

void VCSlider_Test::catchValues()
{
    VCSlider slider(m_env->doc());
    slider.setSliderMode(VCSlider::Level);
    slider.setCatchValues(true);
    QCOMPARE(slider.catchValues(), true);
    slider.setValue(200);

    /* The first input value is just stored */
    slider.slotInputValueChanged(INPUT_SLIDER_CONTROL_ID, 0);
    QCOMPARE(slider.value(), 200);

    /* Values far from the current one, on the same side, are not caught */
    slider.slotInputValueChanged(INPUT_SLIDER_CONTROL_ID, 10);
    QCOMPARE(slider.value(), 200);

    /* Reaching the slider value catches it */
    slider.slotInputValueChanged(INPUT_SLIDER_CONTROL_ID, 199);
    QCOMPARE(slider.value(), 199);
    slider.slotInputValueChanged(INPUT_SLIDER_CONTROL_ID, 100);
    QCOMPARE(slider.value(), 100);

    /* Hiding the slider resets the catching state */
    slider.setVisible(false);
    slider.setVisible(true);
    slider.slotInputValueChanged(INPUT_SLIDER_CONTROL_ID, 0);
    QCOMPARE(slider.value(), 100);

    /* ...and so does disabling it */
    slider.slotInputValueChanged(INPUT_SLIDER_CONTROL_ID, 98);
    QCOMPARE(slider.value(), 98);
    slider.setDisabled(true);
    slider.setDisabled(false);
    slider.slotInputValueChanged(INPUT_SLIDER_CONTROL_ID, 0);
    QCOMPARE(slider.value(), 98);
}

void VCSlider_Test::copy()
{
    VCWidget parent(m_env->doc());

    VCSlider slider(m_env->doc());
    slider.setCaption("Copied");
    slider.setWidgetStyle(VCSlider::WKnob);
    slider.setSliderMode(VCSlider::Level);
    slider.setRangeLowLimit(10);
    slider.setRangeHighLimit(200);
    slider.addLevelChannel(1, 2);
    slider.addLevelChannel(3, 4);
    slider.setValueDisplayStyle(VCSlider::PercentageValue);
    slider.setInvertedAppearance(true);
    slider.setCatchValues(true);
    slider.setClickAndGoType(VCSlider::CnGColors);
    slider.setMonitorEnabled(true);
    slider.setValue(100, false, false);

    VCSlider *copy = qobject_cast<VCSlider *>(slider.createCopy(&parent));
    QVERIFY(copy != nullptr);
    QCOMPARE(copy->caption(), QString("Copied"));
    QCOMPARE(copy->widgetStyle(), VCSlider::WKnob);
    QCOMPARE(copy->sliderMode(), VCSlider::Level);
    QCOMPARE(copy->rangeLowLimit(), qreal(10));
    QCOMPARE(copy->rangeHighLimit(), qreal(200));
    QCOMPARE(copy->levelChannels().count(), 2);
    QVERIFY(copy->levelChannels().contains(SceneValue(1, 2)));
    QVERIFY(copy->levelChannels().contains(SceneValue(3, 4)));
    QCOMPARE(copy->valueDisplayStyle(), VCSlider::PercentageValue);
    QCOMPARE(copy->invertedAppearance(), true);
    QCOMPARE(copy->catchValues(), true);
    QCOMPARE(copy->clickAndGoType(), VCSlider::CnGColors);
    QCOMPARE(copy->monitorEnabled(), true);
    QCOMPARE(copy->value(), 100);
    QCOMPARE(copy->isOverriding(), false);
}

void VCSlider_Test::saveLoadLevel()
{
    VCSlider slider(m_env->doc());
    slider.setID(4);
    slider.setCaption("Level");
    slider.setWidgetStyle(VCSlider::WKnob);
    slider.setSliderMode(VCSlider::Level);
    slider.setRangeLowLimit(10);
    slider.setRangeHighLimit(200);
    slider.addLevelChannel(3, 4);
    slider.addLevelChannel(1, 2);
    slider.setValueDisplayStyle(VCSlider::PercentageValue);
    slider.setInvertedAppearance(true);
    slider.setCatchValues(true);
    slider.setClickAndGoType(VCSlider::CnGColors);
    slider.setValue(100);

    QSharedPointer<QLCInputSource> src(new QLCInputSource(0, 5));
    src->setID(INPUT_SLIDER_CONTROL_ID);
    slider.addInputSource(src);

    QByteArray data = vcSaveXML(&slider);
    VCXMLReader xml(data);
    QCOMPARE(xml.reader().name().toString(), QString("Slider"));

    VCSlider slider2(m_env->doc());
    QVERIFY(slider2.loadXML(xml.reader()));
    QCOMPARE(slider2.id(), quint32(4));
    QCOMPARE(slider2.caption(), QString("Level"));
    QCOMPARE(slider2.widgetStyle(), VCSlider::WKnob);
    QCOMPARE(slider2.sliderMode(), VCSlider::Level);
    QCOMPARE(slider2.rangeLowLimit(), qreal(10));
    QCOMPARE(slider2.rangeHighLimit(), qreal(200));
    QCOMPARE(slider2.value(), 100);
    QCOMPARE(slider2.valueDisplayStyle(), VCSlider::PercentageValue);
    QCOMPARE(slider2.invertedAppearance(), true);
    QCOMPARE(slider2.catchValues(), true);
    QCOMPARE(slider2.clickAndGoType(), VCSlider::CnGColors);
    QCOMPARE(slider2.monitorEnabled(), false);

    /* Level channels are loaded sorted */
    QCOMPARE(slider2.levelChannels().count(), 2);
    QCOMPARE(slider2.levelChannels().at(0), SceneValue(1, 2));
    QCOMPARE(slider2.levelChannels().at(1), SceneValue(3, 4));

    QCOMPARE(slider2.inputSources().count(), 1);
    QCOMPARE(slider2.inputSources().first()->id(), quint32(INPUT_SLIDER_CONTROL_ID));
    QCOMPARE(slider2.inputSources().first()->channel(), quint32(5));

    /* With monitoring enabled, the value is not saved */
    slider.setMonitorEnabled(true);
    data = vcSaveXML(&slider);
    VCXMLReader xml2(data);
    VCSlider slider3(m_env->doc());
    QVERIFY(slider3.loadXML(xml2.reader()));
    QCOMPARE(slider3.monitorEnabled(), true);
    QCOMPARE(slider3.value(), 0);
}

void VCSlider_Test::saveLoadAdjust()
{
    Scene *scene = new Scene(m_env->doc());
    m_env->doc()->addFunction(scene);

    VCSlider slider(m_env->doc());
    slider.setCaption("Adjust");
    slider.setControlledFunction(scene->id());
    slider.setAdjustFlashEnabled(true);
    slider.addKeySequence(QKeySequence("F"), INPUT_SLIDER_FLASH_ID);

    QByteArray data = vcSaveXML(&slider);
    QVERIFY(data.contains("<Adjust"));
    QVERIFY(data.contains("<Flash>"));

    VCXMLReader xml(data);
    VCSlider slider2(m_env->doc());
    QVERIFY(slider2.loadXML(xml.reader()));
    QCOMPARE(slider2.caption(), QString("Adjust"));
    QCOMPARE(slider2.sliderMode(), VCSlider::Adjust);
    QCOMPARE(slider2.controlledFunction(), scene->id());
    QCOMPARE(slider2.controlledAttribute(), int(Function::Intensity));
    QCOMPARE(slider2.adjustFlashEnabled(), true);
    QCOMPARE(slider2.keySequenceMap().value(QKeySequence("F")), quint32(INPUT_SLIDER_FLASH_ID));

    /* Without a function, the <Adjust> and <Flash> tags are not saved */
    VCSlider plain(m_env->doc());
    plain.setAdjustFlashEnabled(true);
    QByteArray plainData = vcSaveXML(&plain);
    QVERIFY(plainData.contains("<Adjust") == false);
    QVERIFY(plainData.contains("<Flash>") == false);

    /* Wrong tag */
    VCXMLReader wrongXml(QByteArray("<Button/>"));
    VCSlider slider3(m_env->doc());
    QCOMPARE(slider3.loadXML(wrongXml.reader()), false);
}

void VCSlider_Test::loadCompactXML()
{
    Scene *scene = new Scene(m_env->doc());
    m_env->doc()->addFunction(scene);

    VCSlider slider(m_env->doc());
    slider.setControlledFunction(scene->id());

    /* XML without whitespace between the tags, e.g. as serialized
     * for network sync. With no level channels, <Level/> is empty */
    QByteArray data = vcSaveXML(&slider, true);
    QVERIFY(data.contains("/><Adjust"));

    VCXMLReader xml(data);
    VCSlider slider2(m_env->doc());
    QVERIFY(slider2.loadXML(xml.reader()));

    /* loadXMLLevel() reads one token past an empty <Level/>,
     * so the following <Adjust> tag is skipped */
    QEXPECT_FAIL("", "loadXMLLevel() swallows the tag following an empty <Level/>", Continue);
    QCOMPARE(slider2.controlledFunction(), scene->id());
}

QTEST_MAIN(VCSlider_Test)
