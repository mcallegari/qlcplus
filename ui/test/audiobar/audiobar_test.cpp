/*
  Q Light Controller Plus - Test Unit
  audiobar_test.cpp

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
#include "audiobar.h"
#include "fixture.h"
#include "chaser.h"
#include "doc.h"
#undef private
#undef protected

#include <QBuffer>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include "audiobar_test.h"

void AudioBar_Test::init()
{
    m_doc = new Doc(this);
}

void AudioBar_Test::cleanup()
{
    delete m_doc;
}

void AudioBar_Test::defaults()
{
    AudioBar bar;
    QCOMPARE(bar.m_type, 0);
    QCOMPARE(bar.m_value, uchar(0));
    QCOMPARE(bar.m_divisor, 1);
}

void AudioBar_Test::copy()
{
    AudioBar bar(1, 42, 100);
    bar.setName("foo");
    AudioBar* cp = bar.createCopy();
    QVERIFY(cp != NULL);
    QCOMPARE(cp->m_type, bar.m_type);
    QCOMPARE(cp->m_value, bar.m_value);
    QCOMPARE(cp->m_parentId, bar.m_parentId);
    delete cp;
}

void AudioBar_Test::setType()
{
    AudioBar bar;
    bar.setType(AudioBar::FunctionBar);
    QCOMPARE(bar.m_type, int(AudioBar::FunctionBar));

    bar.setType(AudioBar::None);
    QCOMPARE(bar.m_type, int(AudioBar::None));
    QCOMPARE(bar.m_value, uchar(0));
    QCOMPARE(bar.m_divisor, 1);
    QCOMPARE(bar.m_dmxChannels.count(), 0);
    QCOMPARE(bar.m_widgetID, VCWidget::invalidId());
}

void AudioBar_Test::thresholdsDivisor()
{
    AudioBar bar;
    bar.m_skippedBeats = 5;
    bar.setMinThreshold(60);
    bar.setDivisor(4);
    QCOMPARE(bar.m_minThreshold, uchar(60));
    QCOMPARE(bar.m_divisor, 4);
    QCOMPARE(bar.m_skippedBeats, 0);
}

void AudioBar_Test::attachDmxChannels()
{
    Fixture* fxi = new Fixture(m_doc);
    fxi->setAddress(10);
    fxi->setChannels(3);
    m_doc->addFixture(fxi);

    QList<SceneValue> list;
    list << SceneValue(fxi->id(), 1, 0);

    AudioBar bar;
    bar.attachDmxChannels(m_doc, list);
    QCOMPARE(bar.m_dmxChannels.count(), 1);
    QCOMPARE(bar.m_absDmxChannels.at(0), int(fxi->universeAddress() + 1));
}

void AudioBar_Test::attachFunction()
{
    Chaser* ch = new Chaser(m_doc);
    m_doc->addFunction(ch);

    AudioBar bar;
    bar.attachFunction(ch);
    QCOMPARE(bar.m_function, ch);
}

void AudioBar_Test::xml()
{
    Fixture* fxi = new Fixture(m_doc);
    fxi->setAddress(0);
    fxi->setChannels(1);
    m_doc->addFixture(fxi);

    AudioBar bar(AudioBar::DMXBar, 0, 0);
    bar.setName("bar");
    bar.setMinThreshold(10);
    bar.setMaxThreshold(200);
    bar.setDivisor(2);
    QList<SceneValue> list;
    list << SceneValue(fxi->id(), 0, 0);
    bar.attachDmxChannels(m_doc, list);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);
    bar.saveXML(&xmlWriter, "VolumeBar", 0);
    xmlWriter.setDevice(nullptr);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    AudioBar bar2;
    QVERIFY(bar2.loadXML(xmlReader, m_doc) == true);
    QCOMPARE(bar2.m_name, QString("bar"));
    QCOMPARE(bar2.m_type, int(AudioBar::DMXBar));
    QCOMPARE(bar2.m_minThreshold, uchar(10));
    QCOMPARE(bar2.m_maxThreshold, uchar(200));
    QCOMPARE(bar2.m_divisor, 2);
    QCOMPARE(bar2.m_absDmxChannels.count(), 1);
    QCOMPARE(bar2.m_absDmxChannels.at(0), int(fxi->universeAddress() + 0));
}

QTEST_MAIN(AudioBar_Test)
