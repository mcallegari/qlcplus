/*
  Q Light Controller Plus - Test Unit
  qlcclipboard_test.cpp

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
#define private public
#include "qlcclipboard.h"
#undef private
#include "scene.h"
#include "doc.h"
#include "qlcclipboard_test.h"

void QLCClipboard_Test::initTestCase()
{
    m_doc = new Doc(this);
    m_scene = new Scene(m_doc);
    m_doc->addFunction(m_scene);
}

void QLCClipboard_Test::cleanupTestCase()
{
    delete m_doc;
    m_doc = nullptr;
}

void QLCClipboard_Test::defaults()
{
    QLCClipboard clip(m_doc);
    QCOMPARE(clip.hasChaserSteps(), false);
    QCOMPARE(clip.hasSceneValues(), false);
    QCOMPARE(clip.hasFunction(), false);
}

void QLCClipboard_Test::copySteps()
{
    QLCClipboard clip(m_doc);
    QList<ChaserStep> steps;
    ChaserStep st(1); st.duration = 1000; steps << st;
    clip.copyContent(0, steps);
    QCOMPARE(clip.hasChaserSteps(), true);
    QCOMPARE(clip.getChaserSteps().count(), 1);
}

void QLCClipboard_Test::copyValues()
{
    QLCClipboard clip(m_doc);
    QList<SceneValue> values;
    values << SceneValue(1,2,100);
    clip.copyContent(0, values);
    QCOMPARE(clip.hasSceneValues(), true);
    QCOMPARE(clip.getSceneValues().count(), 1);
}

void QLCClipboard_Test::copyFunction()
{
    QLCClipboard clip(m_doc);
    clip.copyContent(0, m_scene);
    QCOMPARE(clip.hasFunction(), true);
    QVERIFY(clip.getFunction() != m_scene);
    QCOMPARE(clip.getFunction()->name(), QString("Copy of %1").arg(m_scene->name()));
}

void QLCClipboard_Test::reset()
{
    QLCClipboard clip(m_doc);
    QList<ChaserStep> steps; steps << ChaserStep(1);
    clip.copyContent(0, steps);
    clip.resetContents();
    QCOMPARE(clip.hasChaserSteps(), false);
    QCOMPARE(clip.hasSceneValues(), false);
    QCOMPARE(clip.hasFunction(), false);
}

QTEST_APPLESS_MAIN(QLCClipboard_Test)
