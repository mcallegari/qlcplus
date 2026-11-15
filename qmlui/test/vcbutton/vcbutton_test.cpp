/*
  Q Light Controller Plus - Test Unit
  vcbutton_test.cpp

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

#include "vcbutton_test.h"
#include "inputoutputmap.h"
#include "qlcinputsource.h"
#include "vctestenv.h"
#include "vcbutton.h"
#include "scene.h"

void VCButton_Test::init()
{
    m_env = new VCTestEnv();
}

void VCButton_Test::cleanup()
{
    delete m_env;
}

Scene *VCButton_Test::createScene(QString name)
{
    Scene *scene = new Scene(m_env->doc());
    scene->setName(name);
    m_env->doc()->addFunction(scene);
    return scene;
}

void VCButton_Test::initial()
{
    VCButton btn(m_env->doc());

    QCOMPARE(btn.type(), int(VCWidget::ButtonWidget));
    QCOMPARE(btn.functionID(), Function::invalidId());
    QCOMPARE(btn.state(), VCButton::Inactive);
    QCOMPARE(btn.actionType(), VCButton::Toggle);
    QCOMPARE(btn.stopAllFadeOutTime(), 0);
    QCOMPARE(btn.startupIntensityEnabled(), false);
    QCOMPARE(btn.startupIntensity(), qreal(1.0));
    QCOMPARE(btn.flashOverrides(), false);
    QCOMPARE(btn.flashForceLTP(), false);
    QCOMPARE(btn.externalControlsCount(), 1);
    QCOMPARE(btn.propertiesResource(), QString("qrc:/VCButtonProperties.qml"));

    btn.setID(0);
    QCOMPARE(btn.caption(), QString("Button 1"));

    /* Buttons have a custom background color by default */
    btn.setupLookAndFeel(10, 1);
    QCOMPARE(btn.page(), 1);
    QCOMPARE(btn.backgroundColor(), QColor("#444"));
    QCOMPARE(btn.hasCustomBackgroundColor(), true);
    QCOMPARE(btn.font().pixelSize(), 27);
}

void VCButton_Test::actionStrings()
{
    QCOMPARE(VCButton::actionToString(VCButton::Toggle), QString("Toggle"));
    QCOMPARE(VCButton::actionToString(VCButton::Flash), QString("Flash"));
    QCOMPARE(VCButton::actionToString(VCButton::Blackout), QString("Blackout"));
    QCOMPARE(VCButton::actionToString(VCButton::StopAll), QString("StopAll"));

    QCOMPARE(VCButton::stringToAction("Toggle"), VCButton::Toggle);
    QCOMPARE(VCButton::stringToAction("Flash"), VCButton::Flash);
    QCOMPARE(VCButton::stringToAction("Blackout"), VCButton::Blackout);
    QCOMPARE(VCButton::stringToAction("StopAll"), VCButton::StopAll);
    QCOMPARE(VCButton::stringToAction("Foobar"), VCButton::Toggle);
}

void VCButton_Test::properties()
{
    VCButton btn(m_env->doc());

    QSignalSpy actionSpy(&btn, &VCButton::actionTypeChanged);
    btn.setActionType(VCButton::Flash);
    QCOMPARE(btn.actionType(), VCButton::Flash);
    QCOMPARE(actionSpy.count(), 1);
    btn.setActionType(VCButton::Flash);
    QCOMPARE(actionSpy.count(), 1);

    QSignalSpy fadeSpy(&btn, &VCButton::stopAllFadeOutTimeChanged);
    btn.setStopAllFadeOutTime(1500);
    QCOMPARE(btn.stopAllFadeOutTime(), 1500);
    QCOMPARE(fadeSpy.count(), 1);
    btn.setStopAllFadeOutTime(1500);
    QCOMPARE(fadeSpy.count(), 1);

    QSignalSpy enableSpy(&btn, &VCButton::startupIntensityEnabledChanged);
    btn.setStartupIntensityEnabled(true);
    QCOMPARE(btn.startupIntensityEnabled(), true);
    QCOMPARE(enableSpy.count(), 1);
    btn.setStartupIntensityEnabled(true);
    QCOMPARE(enableSpy.count(), 1);

    QSignalSpy intSpy(&btn, &VCButton::startupIntensityChanged);
    btn.setStartupIntensity(0.3);
    QCOMPARE(btn.startupIntensity(), qreal(0.3));
    QCOMPARE(intSpy.count(), 1);
    btn.setStartupIntensity(0.3);
    QCOMPARE(intSpy.count(), 1);

    /* The value is clamped to 0.0 - 1.0 */
    btn.setStartupIntensity(1.5);
    QCOMPARE(btn.startupIntensity(), qreal(1.0));
    btn.setStartupIntensity(-1.0);
    QCOMPARE(btn.startupIntensity(), qreal(0.0));

    QSignalSpy overrideSpy(&btn, &VCButton::flashOverrideChanged);
    btn.setFlashOverride(true);
    QCOMPARE(btn.flashOverrides(), true);
    QCOMPARE(overrideSpy.count(), 1);
    btn.setFlashOverride(true);
    QCOMPARE(overrideSpy.count(), 1);

    QSignalSpy ltpSpy(&btn, &VCButton::flashForceLTPChanged);
    btn.setFlashForceLTP(true);
    QCOMPARE(btn.flashForceLTP(), true);
    QCOMPARE(ltpSpy.count(), 1);
    btn.setFlashForceLTP(true);
    QCOMPARE(ltpSpy.count(), 1);

    QSignalSpy stateSpy(&btn, &VCButton::stateChanged);
    btn.setState(VCButton::Monitoring);
    QCOMPARE(btn.state(), VCButton::Monitoring);
    QCOMPARE(stateSpy.count(), 1);
    btn.setState(VCButton::Monitoring);
    QCOMPARE(stateSpy.count(), 1);
}

void VCButton_Test::functionAttachment()
{
    Scene *scene = createScene("MyScene");

    VCButton btn(m_env->doc());
    btn.setID(0);
    QCOMPARE(btn.caption(), QString("Button 1"));

    QSignalSpy spy(&btn, &VCButton::functionIDChanged);
    btn.setFunctionID(scene->id());
    QCOMPARE(btn.functionID(), scene->id());
    QCOMPARE(spy.count(), 1);

    /* The default caption is replaced by the function name */
    QCOMPARE(btn.caption(), QString("MyScene"));

    btn.setFunctionID(scene->id());
    QCOMPARE(spy.count(), 1);

    /* A custom caption is not replaced */
    Scene *scene2 = createScene("Other");
    btn.setCaption("Custom");
    btn.setFunctionID(scene2->id());
    QCOMPARE(btn.functionID(), scene2->id());
    QCOMPARE(btn.caption(), QString("Custom"));

    /* A non existing function detaches the current one */
    btn.setFunctionID(12345);
    QCOMPARE(btn.functionID(), Function::invalidId());
    QCOMPARE(spy.count(), 3);
}

void VCButton_Test::toggle()
{
    VCButton btn(m_env->doc());
    QSignalSpy startSpy(&btn, &VCWidget::functionStarting);

    /* Nothing happens without a function */
    btn.requestStateChange(true);
    QCOMPARE(btn.state(), VCButton::Inactive);
    QCOMPARE(startSpy.count(), 0);

    Scene *scene = createScene("S");
    btn.setFunctionID(scene->id());

    btn.requestStateChange(true);
    QCOMPARE(btn.state(), VCButton::Active);
    QCOMPARE(startSpy.count(), 1);
    QCOMPARE(startSpy.at(0).at(1).toUInt(), scene->id());

    btn.requestStateChange(true);
    QCOMPARE(btn.state(), VCButton::Inactive);
    QCOMPARE(startSpy.count(), 1);
}

void VCButton_Test::flash()
{
    Scene *scene = createScene("S");

    VCButton btn(m_env->doc());
    btn.setActionType(VCButton::Flash);
    btn.setFunctionID(scene->id());

    btn.requestStateChange(true);
    QCOMPARE(btn.state(), VCButton::Active);
    QCOMPARE(scene->flashing(), true);

    /* Pressing again while active does nothing */
    btn.requestStateChange(true);
    QCOMPARE(btn.state(), VCButton::Active);

    btn.requestStateChange(false);
    QCOMPARE(btn.state(), VCButton::Inactive);
    QCOMPARE(scene->flashing(), false);
}

void VCButton_Test::flashReleasedWhenHidden()
{
    Scene *scene = createScene("S");

    VCButton btn(m_env->doc());
    btn.setActionType(VCButton::Flash);
    btn.setFunctionID(scene->id());

    btn.requestStateChange(true);
    QCOMPARE(btn.state(), VCButton::Active);

    /* Hiding a pressed Flash button (e.g. page switch) releases it */
    btn.setVisible(false);
    QCOMPARE(btn.isVisible(), false);
    QCOMPARE(btn.state(), VCButton::Inactive);
    QCOMPARE(scene->flashing(), false);
}

void VCButton_Test::blackout()
{
    InputOutputMap *ioMap = m_env->doc()->inputOutputMap();
    bool initialBlackout = ioMap->blackout();

    VCButton btn(m_env->doc());
    btn.setActionType(VCButton::Blackout);

    btn.requestStateChange(true);
    QCOMPARE(btn.state(), VCButton::Active);
    QCOMPARE(ioMap->blackout(), !initialBlackout);

    btn.requestStateChange(false);
    QCOMPARE(btn.state(), VCButton::Inactive);
    QCOMPARE(ioMap->blackout(), initialBlackout);
}

void VCButton_Test::inputValue()
{
    Scene *scene = createScene("S");

    VCButton btn(m_env->doc());
    btn.setFunctionID(scene->id());

    /* Toggle: every non-zero value toggles the state */
    btn.slotInputValueChanged(0, 255);
    QCOMPARE(btn.state(), VCButton::Active);
    btn.slotInputValueChanged(0, 0);
    QCOMPARE(btn.state(), VCButton::Active);
    btn.slotInputValueChanged(0, 255);
    QCOMPARE(btn.state(), VCButton::Inactive);

    /* Unknown control IDs are ignored */
    btn.slotInputValueChanged(1, 255);
    QCOMPARE(btn.state(), VCButton::Inactive);

    /* Flash: active while the value is non-zero */
    btn.setActionType(VCButton::Flash);
    btn.slotInputValueChanged(0, 127);
    QCOMPARE(btn.state(), VCButton::Active);
    btn.slotInputValueChanged(0, 255);
    QCOMPARE(btn.state(), VCButton::Active);
    btn.slotInputValueChanged(0, 0);
    QCOMPARE(btn.state(), VCButton::Inactive);
}

void VCButton_Test::copy()
{
    Scene *scene = createScene("S");

    VCWidget parent(m_env->doc());
    VCButton btn(m_env->doc());
    btn.setCaption("Copied");
    btn.setFunctionID(scene->id());
    btn.setActionType(VCButton::Flash);
    btn.setStartupIntensityEnabled(true);
    btn.setStartupIntensity(0.5);
    btn.setStopAllFadeOutTime(1000);
    btn.setFlashOverride(true);
    btn.setFlashForceLTP(true);
    btn.setGeometry(QRectF(5, 5, 50, 50));

    QSharedPointer<QLCInputSource> src(new QLCInputSource(0, 3));
    src->setID(0);
    btn.addInputSource(src);
    btn.addKeySequence(QKeySequence("K"), 0);

    VCButton *copy = qobject_cast<VCButton *>(btn.createCopy(&parent));
    QVERIFY(copy != nullptr);
    QVERIFY(copy->parent() == &parent);
    QCOMPARE(copy->caption(), QString("Copied"));
    QCOMPARE(copy->functionID(), scene->id());
    QCOMPARE(copy->actionType(), VCButton::Flash);
    QCOMPARE(copy->startupIntensityEnabled(), true);
    QCOMPARE(copy->startupIntensity(), qreal(0.5));
    QCOMPARE(copy->stopAllFadeOutTime(), 1000);
    QCOMPARE(copy->flashOverrides(), true);
    QCOMPARE(copy->flashForceLTP(), true);
    QCOMPARE(copy->geometry(), QRectF(5, 5, 50, 50));

    /* Input sources are duplicated, not shared */
    QCOMPARE(copy->inputSources().count(), 1);
    QVERIFY(copy->inputSources().first() != src);
    QCOMPARE(copy->inputSources().first()->universe(), quint32(0));
    QCOMPARE(copy->inputSources().first()->channel(), quint32(3));
    QCOMPARE(copy->keySequenceMap().value(QKeySequence("K")), quint32(0));
}

void VCButton_Test::saveLoad()
{
    Scene *scene = createScene("S");

    VCButton btn(m_env->doc());
    btn.setID(3);
    btn.setCaption("B");
    btn.setFunctionID(scene->id());
    btn.setActionType(VCButton::Flash);
    btn.setFlashOverride(true);
    btn.setFlashForceLTP(true);
    btn.setStartupIntensity(0.5);
    btn.setStartupIntensityEnabled(true);

    QSharedPointer<QLCInputSource> src(new QLCInputSource(0, 12));
    src->setID(0);
    btn.addInputSource(src);
    btn.addKeySequence(QKeySequence("Q"), 0);

    QByteArray data = vcSaveXML(&btn);
    VCXMLReader xml(data);
    QCOMPARE(xml.reader().name().toString(), QString("Button"));

    VCButton btn2(m_env->doc());
    QVERIFY(btn2.loadXML(xml.reader()));
    QCOMPARE(btn2.id(), quint32(3));
    QCOMPARE(btn2.caption(), QString("B"));
    QCOMPARE(btn2.functionID(), scene->id());
    QCOMPARE(btn2.actionType(), VCButton::Flash);
    QCOMPARE(btn2.flashOverrides(), true);
    QCOMPARE(btn2.flashForceLTP(), true);
    QCOMPARE(btn2.startupIntensity(), qreal(0.5));

    /* saveXML writes the <Intensity> value but not the Adjust attribute
     * loadXML relies on, so the flag is lost on reload */
    QEXPECT_FAIL("", "The Intensity Adjust attribute is not saved", Continue);
    QCOMPARE(btn2.startupIntensityEnabled(), true);

    QCOMPARE(btn2.inputSources().count(), 1);
    QCOMPARE(btn2.inputSources().first()->universe(), quint32(0));
    QCOMPARE(btn2.inputSources().first()->channel(), quint32(12));
    QCOMPARE(btn2.keySequenceMap().value(QKeySequence("Q")), quint32(0));

    /* Wrong tag */
    VCXMLReader wrongXml(QByteArray("<Label/>"));
    VCButton btn3(m_env->doc());
    QCOMPARE(btn3.loadXML(wrongXml.reader()), false);
}

void VCButton_Test::saveLoadStopAll()
{
    VCButton btn(m_env->doc());
    btn.setActionType(VCButton::StopAll);
    btn.setStopAllFadeOutTime(2000);
    btn.setFlashOverride(true);

    QByteArray data = vcSaveXML(&btn);
    /* Flash attributes are saved only for Flash buttons */
    QVERIFY(data.contains("FadeOut=\"2000\""));
    QVERIFY(data.contains("Override=") == false);

    VCXMLReader xml(data);
    VCButton btn2(m_env->doc());
    QVERIFY(btn2.loadXML(xml.reader()));
    QCOMPARE(btn2.actionType(), VCButton::StopAll);
    QCOMPARE(btn2.stopAllFadeOutTime(), 2000);
    QCOMPARE(btn2.flashOverrides(), false);
    QCOMPARE(btn2.functionID(), Function::invalidId());
}

QTEST_MAIN(VCButton_Test)
