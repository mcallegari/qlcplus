/*
  Q Light Controller Plus - Test Unit
  vcaudiotriggers_test.cpp

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

#include "vcaudiotriggers_test.h"
#include "vcaudiotriggers.h"
#include "vctestenv.h"
#include "vcframe.h"
#include "scene.h"

/* External control IDs, as defined in vcaudiotriggers.cpp */
#define INPUT_VOLUME_CONTROL    1

/* The default number of spectrum bars, plus the volume bar */
#define DEFAULT_BARS_NUMBER     (16 + 1)

static QVariantMap barInfo(const VCAudioTriggers &triggers, int index)
{
    return triggers.barsInfo().at(index).toMap();
}

void VCAudioTriggers_Test::init()
{
    m_env = new VCTestEnv();
}

void VCAudioTriggers_Test::cleanup()
{
    delete m_env;
}

void VCAudioTriggers_Test::initial()
{
    VCAudioTriggers triggers(m_env->doc(), m_env->vc());

    QCOMPARE(triggers.type(), int(VCWidget::AudioTriggersWidget));
    QCOMPARE(triggers.captureEnabled(), false);
    QCOMPARE(triggers.volumeLevel(), uchar(100));
    QCOMPARE(triggers.barsNumber(), DEFAULT_BARS_NUMBER);
    QCOMPARE(triggers.selectedBar(), -1);
    QCOMPARE(triggers.audioLevels().count(), DEFAULT_BARS_NUMBER);
    QCOMPARE(triggers.externalControlsCount(), 2);
    QCOMPARE(triggers.propertiesResource(), QString("qrc:/VCAudioTriggersProperties.qml"));

    QVariantList info = triggers.barsInfo();
    QCOMPARE(info.count(), DEFAULT_BARS_NUMBER);
    QCOMPARE(info.at(0).toMap().value("bLabel").toString(), QString("Volume Bar"));
    for (int i = 0; i < info.count(); i++)
    {
        QVariantMap map = info.at(i).toMap();
        QCOMPARE(map.value("index").toInt(), i);
        QCOMPARE(map.value("type").toInt(), int(VCAudioTriggers::None));
        QCOMPARE(map.value("minThreshold").toInt(), 20);
        QCOMPARE(map.value("maxThreshold").toInt(), 80);
    }

    triggers.setID(0);
    QCOMPARE(triggers.caption(), QString("Audio Trigger 1"));
}

void VCAudioTriggers_Test::barsNumber()
{
    VCAudioTriggers triggers(m_env->doc(), m_env->vc());

    QSignalSpy spy(&triggers, &VCAudioTriggers::barsNumberChanged);
    triggers.setBarsNumber(5);
    QCOMPARE(triggers.barsNumber(), 5);
    QCOMPARE(triggers.barsInfo().count(), 5);
    QCOMPARE(triggers.audioLevels().count(), 5);
    QCOMPARE(spy.count(), 1);
    triggers.setBarsNumber(5);
    QCOMPARE(spy.count(), 1);

    QSignalSpy selSpy(&triggers, &VCAudioTriggers::selectedBarChanged);
    triggers.setSelectedBar(4);
    QCOMPARE(triggers.selectedBar(), 4);
    QCOMPARE(selSpy.count(), 1);

    /* Removing the selected bar resets the selection */
    triggers.setBarsNumber(3);
    QCOMPARE(triggers.selectedBar(), -1);
    QCOMPARE(selSpy.count(), 2);

    /* Out of range selections reset the selection */
    triggers.setSelectedBar(1);
    triggers.setSelectedBar(10);
    QCOMPARE(triggers.selectedBar(), -1);

    triggers.setBarsNumber(8);
    QCOMPARE(triggers.barsNumber(), 8);
}

void VCAudioTriggers_Test::bars()
{
    Scene *scene = new Scene(m_env->doc());
    m_env->doc()->addFunction(scene);

    VCFrame frame(m_env->doc(), m_env->vc());
    VCWidget *btn = frame.addWidget(nullptr, "Button", QPoint());
    btn->setCaption("Target");

    VCAudioTriggers triggers(m_env->doc(), m_env->vc());

    /* Nothing happens without a selected bar */
    triggers.setBarType(VCAudioTriggers::FunctionBar);
    triggers.setBarFunction(scene->id());
    for (const QVariant &bar : triggers.barsInfo())
        QCOMPARE(bar.toMap().value("type").toInt(), int(VCAudioTriggers::None));

    QSignalSpy spy(&triggers, &VCAudioTriggers::barsInfoChanged);

    /* Function bar */
    triggers.setSelectedBar(1);
    triggers.setBarType(VCAudioTriggers::FunctionBar);
    triggers.setBarFunction(scene->id());
    triggers.setBarThresholds(10, 90);
    QCOMPARE(spy.count(), 3);
    QVariantMap info = barInfo(triggers, 1);
    QCOMPARE(info.value("type").toInt(), int(VCAudioTriggers::FunctionBar));
    QCOMPARE(info.value("intVal").toUInt(), scene->id());
    QCOMPARE(info.value("minThreshold").toInt(), 10);
    QCOMPARE(info.value("maxThreshold").toInt(), 90);

    /* VC widget bar */
    triggers.setSelectedBar(2);
    triggers.setBarType(VCAudioTriggers::VCWidgetBar);
    info = barInfo(triggers, 2);
    QCOMPARE(info.value("intVal").toInt(), -1);
    triggers.setBarWidget(btn->id());
    info = barInfo(triggers, 2);
    QCOMPARE(info.value("type").toInt(), int(VCAudioTriggers::VCWidgetBar));
    QCOMPARE(info.value("intVal").toUInt(), btn->id());
    QCOMPARE(info.value("strVal").toString(), QString("Target"));
    QCOMPARE(info.value("iconVal").toString(), VCWidget::typeToIcon(VCWidget::ButtonWidget));

    /* DMX bar */
    triggers.setSelectedBar(3);
    triggers.setBarType(VCAudioTriggers::DMXBar);
    triggers.setBarDmxChannels(QList<SceneValue>() << SceneValue(1, 2) << SceneValue(3, 4));
    info = barInfo(triggers, 3);
    QCOMPARE(info.value("type").toInt(), int(VCAudioTriggers::DMXBar));
    QCOMPARE(info.value("intVal").toInt(), 2);

    /* Changing type resets the bar */
    triggers.setSelectedBar(1);
    triggers.setBarType(VCAudioTriggers::DMXBar);
    info = barInfo(triggers, 1);
    QCOMPARE(info.value("type").toInt(), int(VCAudioTriggers::DMXBar));
    QCOMPARE(info.value("intVal").toInt(), 0);
    QCOMPARE(info.value("minThreshold").toInt(), 20);
    QCOMPARE(info.value("maxThreshold").toInt(), 80);
}

void VCAudioTriggers_Test::volume()
{
    VCAudioTriggers triggers(m_env->doc(), m_env->vc());

    QSignalSpy spy(&triggers, &VCAudioTriggers::volumeLevelChanged);
    triggers.setVolumeLevel(50);
    QCOMPARE(triggers.volumeLevel(), uchar(50));
    QCOMPARE(spy.count(), 1);
    triggers.setVolumeLevel(50);
    QCOMPARE(spy.count(), 1);

    /* External volume control, scaled to 0-100 */
    triggers.slotInputValueChanged(INPUT_VOLUME_CONTROL, 255);
    QCOMPARE(triggers.volumeLevel(), uchar(100));
    triggers.slotInputValueChanged(INPUT_VOLUME_CONTROL, 0);
    QCOMPARE(triggers.volumeLevel(), uchar(0));
}

void VCAudioTriggers_Test::copy()
{
    Scene *scene = new Scene(m_env->doc());
    m_env->doc()->addFunction(scene);

    VCWidget parent(m_env->doc());

    VCAudioTriggers triggers(m_env->doc(), m_env->vc());
    triggers.setCaption("Copied");
    triggers.setGeometry(QRectF(1, 2, 3, 4));
    triggers.setSelectedBar(1);
    triggers.setBarType(VCAudioTriggers::FunctionBar);
    triggers.setBarFunction(scene->id());

    VCAudioTriggers *copy = qobject_cast<VCAudioTriggers *>(triggers.createCopy(&parent));
    QVERIFY(copy != nullptr);
    QCOMPARE(copy->caption(), QString("Copied"));
    QCOMPARE(copy->geometry(), QRectF(1, 2, 3, 4));
    QCOMPARE(copy->barsNumber(), triggers.barsNumber());

    QEXPECT_FAIL("", "copyFrom() doesn't copy the bars configuration", Continue);
    QCOMPARE(barInfo(*copy, 1).value("type").toInt(), int(VCAudioTriggers::FunctionBar));
}

void VCAudioTriggers_Test::saveLoad()
{
    Scene *scene = new Scene(m_env->doc());
    m_env->doc()->addFunction(scene);

    VCAudioTriggers triggers(m_env->doc(), m_env->vc());
    triggers.setID(3);
    triggers.setCaption("Audio");
    triggers.setBarsNumber(9);

    triggers.setSelectedBar(0);
    triggers.setBarType(VCAudioTriggers::FunctionBar);
    triggers.setBarFunction(scene->id());
    triggers.setBarThresholds(30, 70);

    triggers.setSelectedBar(2);
    triggers.setBarType(VCAudioTriggers::VCWidgetBar);
    triggers.setBarWidget(77);

    triggers.setSelectedBar(5);
    triggers.setBarType(VCAudioTriggers::DMXBar);
    triggers.setBarDmxChannels(QList<SceneValue>() << SceneValue(1, 2) << SceneValue(3, 4));

    QByteArray data = vcSaveXML(&triggers);
    VCXMLReader xml(data);
    QCOMPARE(xml.reader().name().toString(), QString("AudioTriggers"));

    VCAudioTriggers triggers2(m_env->doc(), m_env->vc());
    QVERIFY(triggers2.loadXML(xml.reader()));
    QCOMPARE(triggers2.id(), quint32(3));
    QCOMPARE(triggers2.caption(), QString("Audio"));
    QCOMPARE(triggers2.barsNumber(), 9);

    QVariantMap info = barInfo(triggers2, 0);
    QCOMPARE(info.value("type").toInt(), int(VCAudioTriggers::FunctionBar));
    QCOMPARE(info.value("intVal").toUInt(), scene->id());
    QCOMPARE(info.value("minThreshold").toInt(), 30);
    QCOMPARE(info.value("maxThreshold").toInt(), 70);

    info = barInfo(triggers2, 2);
    QCOMPARE(info.value("type").toInt(), int(VCAudioTriggers::VCWidgetBar));
    QCOMPARE(info.value("intVal").toInt(), 77);

    info = barInfo(triggers2, 5);
    QCOMPARE(info.value("type").toInt(), int(VCAudioTriggers::DMXBar));
    QCOMPARE(info.value("intVal").toInt(), 2);

    /* Unconfigured bars stay unconfigured */
    QCOMPARE(barInfo(triggers2, 1).value("type").toInt(), int(VCAudioTriggers::None));

    /* Wrong tag */
    VCXMLReader wrongXml(QByteArray("<Button/>"));
    VCAudioTriggers triggers3(m_env->doc(), m_env->vc());
    QCOMPARE(triggers3.loadXML(wrongXml.reader()), false);
}

void VCAudioTriggers_Test::saveLoadEmptyDMXBar()
{
    Scene *scene = new Scene(m_env->doc());
    m_env->doc()->addFunction(scene);

    VCAudioTriggers triggers(m_env->doc(), m_env->vc());

    /* A DMX bar without channels, followed by another bar */
    triggers.setSelectedBar(1);
    triggers.setBarType(VCAudioTriggers::DMXBar);
    triggers.setSelectedBar(2);
    triggers.setBarType(VCAudioTriggers::FunctionBar);
    triggers.setBarFunction(scene->id());

    QByteArray data = vcSaveXML(&triggers);
    VCXMLReader xml(data);
    VCAudioTriggers triggers2(m_env->doc(), m_env->vc());
    QVERIFY(triggers2.loadXML(xml.reader()));
    QCOMPARE(barInfo(triggers2, 1).value("type").toInt(), int(VCAudioTriggers::DMXBar));

    /* loadBarXML() reads past the end of an empty DMX <Bar/>, then loadXML()
     * skips the rest of the parent element, so the next bar is lost */
    QEXPECT_FAIL("", "The bar following an empty DMX bar is not loaded", Continue);
    QCOMPARE(barInfo(triggers2, 2).value("type").toInt(), int(VCAudioTriggers::FunctionBar));
}

QTEST_MAIN(VCAudioTriggers_Test)
