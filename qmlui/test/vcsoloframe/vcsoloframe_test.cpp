/*
  Q Light Controller Plus - Test Unit
  vcsoloframe_test.cpp

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

#include "vcsoloframe_test.h"
#include "vcsoloframe.h"
#include "vctestenv.h"
#include "vcbutton.h"
#include "scene.h"

void VCSoloFrame_Test::init()
{
    m_env = new VCTestEnv();
}

void VCSoloFrame_Test::cleanup()
{
    delete m_env;
}

void VCSoloFrame_Test::initial()
{
    VCSoloFrame solo(m_env->doc(), m_env->vc());

    QCOMPARE(solo.type(), int(VCWidget::SoloFrameWidget));
    QCOMPARE(solo.soloframeMixing(), false);
    QCOMPARE(solo.excludeMonitoredFunctions(), false);
    QCOMPARE(solo.showHeader(), true);
    QCOMPARE(solo.hasChildren(), false);

    solo.setID(2);
    QCOMPARE(solo.caption(), QString("Solo Frame 3"));
}

void VCSoloFrame_Test::properties()
{
    VCSoloFrame solo(m_env->doc(), m_env->vc());

    m_env->doc()->resetModified();
    QSignalSpy mixSpy(&solo, &VCSoloFrame::soloframeMixingChanged);
    solo.setSoloframeMixing(true);
    QCOMPARE(solo.soloframeMixing(), true);
    QCOMPARE(mixSpy.count(), 1);
    QCOMPARE(m_env->doc()->isModified(), true);
    solo.setSoloframeMixing(true);
    QCOMPARE(mixSpy.count(), 1);

    QSignalSpy exSpy(&solo, &VCSoloFrame::excludeMonitoredFunctionsChanged);
    solo.setExcludeMonitoredFunctions(true);
    QCOMPARE(solo.excludeMonitoredFunctions(), true);
    QCOMPARE(exSpy.count(), 1);
    solo.setExcludeMonitoredFunctions(true);
    QCOMPARE(exSpy.count(), 1);
}

void VCSoloFrame_Test::soloParent()
{
    VCSoloFrame solo(m_env->doc(), m_env->vc());
    VCFrame frame(m_env->doc(), m_env->vc());

    VCWidget *soloChild = solo.addWidget(nullptr, "Button", QPoint());
    QCOMPARE(soloChild->hasSoloParent(), true);

    /* A frame inside a solo frame passes the solo behavior through */
    VCFrame *inner = qobject_cast<VCFrame *>(solo.addWidget(nullptr, "Frame", QPoint()));
    QVERIFY(inner != nullptr);
    QCOMPARE(inner->hasSoloParent(), true);
    VCWidget *nested = inner->addWidget(nullptr, "Button", QPoint());
    QCOMPARE(nested->hasSoloParent(), true);

    VCWidget *frameChild = frame.addWidget(nullptr, "Button", QPoint());
    QCOMPARE(frameChild->hasSoloParent(), false);
    QCOMPARE(solo.hasSoloParent(), false);
}

void VCSoloFrame_Test::functionStarting()
{
    Doc *doc = m_env->doc();

    Scene *s1 = new Scene(doc);
    doc->addFunction(s1);
    Scene *s2 = new Scene(doc);
    doc->addFunction(s2);
    Scene *s3 = new Scene(doc);
    doc->addFunction(s3);

    VCSoloFrame solo(doc, m_env->vc());
    VCButton *b1 = qobject_cast<VCButton *>(solo.addWidget(nullptr, "Button", QPoint()));
    VCButton *b2 = qobject_cast<VCButton *>(solo.addWidget(nullptr, "Button", QPoint()));
    VCFrame *inner = qobject_cast<VCFrame *>(solo.addWidget(nullptr, "Frame", QPoint()));
    VCButton *b3 = qobject_cast<VCButton *>(inner->addWidget(nullptr, "Button", QPoint()));
    QVERIFY(b1 != nullptr && b2 != nullptr && b3 != nullptr);

    b1->setFunctionID(s1->id());
    b2->setFunctionID(s2->id());
    b3->setFunctionID(s3->id());

    b1->requestStateChange(true);
    QCOMPARE(s1->stopped(), false);

    /* Starting another function in the solo frame stops the others */
    b2->requestStateChange(true);
    QCOMPARE(s2->stopped(), false);
    QCOMPARE(s1->stopped(), true);

    /* The request is passed through nested frames as well */
    b3->requestStateChange(true);
    QCOMPARE(s3->stopped(), false);
    QCOMPARE(s2->stopped(), true);
}

void VCSoloFrame_Test::copy()
{
    VCFrame parent(m_env->doc(), m_env->vc());

    VCSoloFrame solo(m_env->doc(), m_env->vc());
    solo.setCaption("Solo");
    solo.setSoloframeMixing(true);
    solo.setExcludeMonitoredFunctions(true);
    VCWidget *child = solo.addWidget(nullptr, "Button", QPoint());
    child->setCaption("Child");

    VCSoloFrame *copy = qobject_cast<VCSoloFrame *>(solo.createCopy(&parent));
    QVERIFY(copy != nullptr);
    QCOMPARE(copy->type(), int(VCWidget::SoloFrameWidget));
    QCOMPARE(copy->caption(), QString("Solo"));
    QCOMPARE(copy->soloframeMixing(), true);
    QCOMPARE(copy->excludeMonitoredFunctions(), true);
    QCOMPARE(copy->children().count(), 1);
    QCOMPARE(copy->children().first()->caption(), QString("Child"));
    QCOMPARE(copy->children().first()->hasSoloParent(), true);
}

void VCSoloFrame_Test::saveLoad()
{
    VCSoloFrame solo(m_env->doc(), m_env->vc());
    solo.setCaption("Solo");
    solo.setSoloframeMixing(true);
    solo.setExcludeMonitoredFunctions(true);
    VCWidget *child = solo.addWidget(nullptr, "Button", QPoint());
    child->setCaption("Child");

    QByteArray data = vcSaveXML(&solo);
    QVERIFY(data.contains("<Mixing>True</Mixing>"));
    QVERIFY(data.contains("<ExcludeMonitored>True</ExcludeMonitored>"));

    VCXMLReader xml(data);
    QCOMPARE(xml.reader().name().toString(), QString("SoloFrame"));

    /* A normal frame refuses a solo frame tag */
    VCFrame frame(m_env->doc(), m_env->vc());
    VCXMLReader xml2(data);
    QCOMPARE(frame.loadXML(xml2.reader()), false);

    VCSoloFrame solo2(m_env->doc(), m_env->vc());
    QVERIFY(solo2.loadXML(xml.reader()));
    QCOMPARE(solo2.caption(), QString("Solo"));
    QCOMPARE(solo2.soloframeMixing(), true);
    QCOMPARE(solo2.excludeMonitoredFunctions(), true);
    QCOMPARE(solo2.children().count(), 1);
    QCOMPARE(solo2.children().first()->caption(), QString("Child"));
    QCOMPARE(solo2.children().first()->hasSoloParent(), true);

    /* Defaults are not written */
    VCSoloFrame plain(m_env->doc(), m_env->vc());
    QByteArray plainData = vcSaveXML(&plain);
    QVERIFY(plainData.contains("<Mixing>") == false);
    QVERIFY(plainData.contains("<ExcludeMonitored>") == false);
}

QTEST_MAIN(VCSoloFrame_Test)
