/*
  Q Light Controller Plus - Test Unit
  vclabel_test.cpp

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

#include "vclabel_test.h"
#include "vctestenv.h"
#include "vclabel.h"

void VCLabel_Test::init()
{
    m_env = new VCTestEnv();
}

void VCLabel_Test::cleanup()
{
    delete m_env;
}

void VCLabel_Test::initial()
{
    VCLabel label(m_env->doc());

    QCOMPARE(label.type(), int(VCWidget::LabelWidget));
    QVERIFY(label.caption().isEmpty());
    QVERIFY(label.propertiesResource().isEmpty());
    QCOMPARE(label.externalControlsCount(), 0);

    /* Setting an ID assigns the default caption */
    label.setID(3);
    QCOMPARE(label.caption(), QString("Label 4"));
    QCOMPARE(label.defaultCaption(), QString("Label 4"));

    /* A custom caption is not overwritten */
    label.setCaption("Foo");
    label.setID(4);
    QCOMPARE(label.caption(), QString("Foo"));

    label.setupLookAndFeel(10, 2);
    QCOMPARE(label.page(), 2);
    QCOMPARE(label.font().pixelSize(), 35);
}

void VCLabel_Test::copy()
{
    VCWidget parent(m_env->doc());

    VCLabel label(m_env->doc());
    label.setCaption("Foobar");
    label.setGeometry(QRectF(1, 2, 30, 40));
    label.setZIndex(2);
    label.setPage(1);
    label.setBackgroundColor(QColor(Qt::red));
    label.setForegroundColor(QColor(Qt::blue));
    label.setFont(QFont("Arial", 20));
    label.setAllowResize(false);

    VCLabel *copy = qobject_cast<VCLabel *>(label.createCopy(&parent));
    QVERIFY(copy != nullptr);
    QVERIFY(copy != &label);
    QVERIFY(copy->parent() == &parent);
    QCOMPARE(copy->type(), int(VCWidget::LabelWidget));
    QCOMPARE(copy->caption(), QString("Foobar"));
    QCOMPARE(copy->geometry(), QRectF(1, 2, 30, 40));
    QCOMPARE(copy->zIndex(), 2);
    QCOMPARE(copy->page(), 1);
    QCOMPARE(copy->allowResize(), false);
    QCOMPARE(copy->hasCustomBackgroundColor(), true);
    QCOMPARE(copy->backgroundColor(), QColor(Qt::red));
    QCOMPARE(copy->hasCustomForegroundColor(), true);
    QCOMPARE(copy->foregroundColor(), QColor(Qt::blue));
    QCOMPARE(copy->hasCustomFont(), true);
    QCOMPARE(copy->font().family(), QString("Arial"));
    QCOMPARE(copy->font().pointSize(), 20);
}

void VCLabel_Test::loadWrongTag()
{
    VCLabel label(m_env->doc());
    VCXMLReader xml(QByteArray("<Button Caption=\"Foo\"/>"));
    QCOMPARE(label.loadXML(xml.reader()), false);
    QVERIFY(label.caption().isEmpty());
}

void VCLabel_Test::saveLoad()
{
    VCLabel label(m_env->doc());
    label.setID(7);
    label.setCaption("Hello");
    label.setGeometry(QRectF(10, 20, 100, 50));
    label.setPage(1);
    label.setZIndex(3);
    label.setBackgroundColor(QColor(Qt::red));
    label.setForegroundColor(QColor(Qt::green));
    label.setFont(QFont("Arial", 14));

    QByteArray data = vcSaveXML(&label);
    VCXMLReader xml(data);
    QCOMPARE(xml.reader().name().toString(), QString("Label"));

    VCLabel label2(m_env->doc());
    QVERIFY(label2.loadXML(xml.reader()));

    QCOMPARE(label2.id(), quint32(7));
    QCOMPARE(label2.caption(), QString("Hello"));
    QCOMPARE(label2.geometry(), QRectF(10, 20, 100, 50));
    QCOMPARE(label2.page(), 1);
    QCOMPARE(label2.zIndex(), 3);
    QCOMPARE(label2.hasCustomBackgroundColor(), true);
    QCOMPARE(label2.backgroundColor(), QColor(Qt::red));
    QCOMPARE(label2.hasCustomForegroundColor(), true);
    QCOMPARE(label2.foregroundColor(), QColor(Qt::green));
    QCOMPARE(label2.hasCustomFont(), true);
    QCOMPARE(label2.font().family(), QString("Arial"));
    QCOMPARE(label2.font().pointSize(), 14);
}

void VCLabel_Test::saveDefaultAppearance()
{
    VCLabel label(m_env->doc());
    label.setCaption("Plain");

    /* No custom appearance: no <Appearance> tag, no ID and no page */
    QByteArray data = vcSaveXML(&label);
    QVERIFY(data.contains("<Label"));
    QVERIFY(data.contains("Caption=\"Plain\""));
    QVERIFY(data.contains("<WindowState"));
    QVERIFY(data.contains("<Appearance") == false);
    QVERIFY(data.contains("ID=") == false);
    QVERIFY(data.contains("Page=") == false);

    VCXMLReader xml(data);
    VCLabel label2(m_env->doc());
    QVERIFY(label2.loadXML(xml.reader()));
    QCOMPARE(label2.caption(), QString("Plain"));
    QCOMPARE(label2.id(), VCWidget::invalidId());
    QCOMPARE(label2.hasCustomBackgroundColor(), false);
    QCOMPARE(label2.hasCustomForegroundColor(), false);
    QCOMPARE(label2.hasCustomFont(), false);
}

QTEST_MAIN(VCLabel_Test)
