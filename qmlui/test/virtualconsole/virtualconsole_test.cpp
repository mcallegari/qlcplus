/*
  Q Light Controller Plus - Test Unit
  virtualconsole_test.cpp

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

#include "virtualconsole_test.h"
#include "qlcinputsource.h"
#include "virtualconsole.h"
#include "vctestenv.h"
#include "vclabel.h"
#include "vcpage.h"

/* The pages created by the VirtualConsole constructor */
#define DEFAULT_PAGES_NUMBER 4

void VirtualConsole_Test::init()
{
    m_env = new VCTestEnv();
}

void VirtualConsole_Test::cleanup()
{
    delete m_env;
}

void VirtualConsole_Test::pages()
{
    VirtualConsole *vc = m_env->vc();

    QCOMPARE(vc->pagesCount(), DEFAULT_PAGES_NUMBER);
    QVERIFY(vc->page(DEFAULT_PAGES_NUMBER) == nullptr);
    QVERIFY(vc->page(-1) == nullptr);
    QCOMPARE(vc->loadStatus(), VirtualConsole::Cleared);

    QList<quint32> ids;
    for (int i = 0; i < DEFAULT_PAGES_NUMBER; i++)
    {
        VCPage *page = vc->page(i);
        QVERIFY(page != nullptr);
        QVERIFY(page->parent() == vc);
        QCOMPARE(page->type(), int(VCWidget::FrameWidget));
        QCOMPARE(page->caption(), QString("Page %1").arg(i + 1));
        QCOMPARE(page->geometry(), QRectF(0, 0, 1920, 1080));
        QCOMPARE(page->allowResize(), false);
        QCOMPARE(page->showHeader(), false);
        QCOMPARE(page->pageScale(), qreal(1.0));
        QVERIFY(page->previewContext() != nullptr);
        QCOMPARE(page->propertiesResource(), QString("qrc:/VCPageProperties.qml"));

        /* Pages only keep the enable control */
        QCOMPARE(page->externalControlsCount(), 1);

        /* Pages have unique IDs, reachable through the widget lookup */
        QVERIFY(page->id() != VCWidget::invalidId());
        QVERIFY(ids.contains(page->id()) == false);
        ids.append(page->id());
        QVERIFY(vc->widget(page->id()) == page);
    }

    QVERIFY(vc->widget(VCWidget::invalidId()) == nullptr);
}

void VirtualConsole_Test::widgetsMap()
{
    VirtualConsole *vc = m_env->vc();
    VCPage *page = vc->page(0);

    VCWidget *btn = page->addWidget(nullptr, "Button", QPoint());
    QVERIFY(btn != nullptr);
    QVERIFY(vc->widget(btn->id()) == btn);

    /* Widget IDs never clash with page IDs */
    for (int i = 0; i < vc->pagesCount(); i++)
        QVERIFY(btn->id() != vc->page(i)->id());

    /* A widget with a conflicting ID gets a new one */
    VCLabel label(m_env->doc());
    label.setID(btn->id());
    vc->addWidgetToMap(&label);
    QVERIFY(label.id() != btn->id());
    QVERIFY(vc->widget(label.id()) == &label);
    QVERIFY(vc->widget(btn->id()) == btn);

    /* Adding twice is harmless */
    quint32 labelId = label.id();
    vc->addWidgetToMap(&label);
    QCOMPARE(label.id(), labelId);

    /* A widget can't take the ID of a page */
    VCLabel label2(m_env->doc());
    label2.setID(page->id());
    vc->addWidgetToMap(&label2);
    QVERIFY(label2.id() != page->id());
    QVERIFY(vc->widget(page->id()) == page);

    vc->removeWidgetFromMap(&label);
    vc->removeWidgetFromMap(&label2);
    QVERIFY(vc->widget(labelId) == nullptr);
    vc->removeWidgetFromMap(nullptr);
}

void VirtualConsole_Test::widgetsList()
{
    VirtualConsole *vc = m_env->vc();

    /* No widgets */
    QVariantList list = vc->widgetsList();
    QCOMPARE(list.count(), 1);
    QCOMPARE(list.first().toMap().contains("id"), false);

    VCPage *page = vc->page(0);
    VCWidget *btn1 = page->addWidget(nullptr, "Button", QPoint());
    VCWidget *btn2 = page->addWidget(nullptr, "Button", QPoint());
    VCWidget *label = page->addWidget(nullptr, "Label", QPoint());
    btn1->setCaption("B1");

    list = vc->widgetsList();
    QCOMPARE(list.count(), 3);

    /* Sorted by ID */
    QCOMPARE(list.at(0).toMap().value("id").toUInt(), btn1->id());
    QCOMPARE(list.at(0).toMap().value("label").toString(), QString("B1"));
    QCOMPARE(list.at(0).toMap().value("type").toInt(), int(VCWidget::ButtonWidget));

    /* Type filter */
    list = vc->widgetsList(QVariantList() << int(VCWidget::ButtonWidget));
    QCOMPARE(list.count(), 2);

    /* Exclusion */
    list = vc->widgetsList(QVariantList() << int(VCWidget::ButtonWidget), btn2->id());
    QCOMPARE(list.count(), 1);
    QCOMPARE(list.first().toMap().value("id").toUInt(), btn1->id());

    list = vc->widgetsList(QVariantList() << int(VCWidget::LabelWidget));
    QCOMPARE(list.count(), 1);
    QCOMPARE(list.first().toMap().value("id").toUInt(), label->id());
}

void VirtualConsole_Test::pagePIN()
{
    VirtualConsole *vc = m_env->vc();

    QVERIFY(vc->setPagePIN(0, "", "1234"));
    QCOMPARE(vc->page(0)->PIN(), 1234);
    QCOMPARE(vc->page(0)->requirePIN(), true);

    /* The current PIN must match */
    QCOMPARE(vc->setPagePIN(0, "0000", "5678"), false);
    QCOMPARE(vc->setPagePIN(0, "abcd", "5678"), false);
    QCOMPARE(vc->page(0)->PIN(), 1234);

    /* A PIN is either empty or 4 digits */
    QCOMPARE(vc->setPagePIN(0, "1234", "12"), false);
    QCOMPARE(vc->setPagePIN(0, "1234", "abcd"), false);
    QCOMPARE(vc->page(0)->PIN(), 1234);

    /* Invalid pages */
    QCOMPARE(vc->setPagePIN(-1, "", "1234"), false);
    QCOMPARE(vc->setPagePIN(DEFAULT_PAGES_NUMBER, "", "1234"), false);
    QCOMPARE(vc->validatePagePIN(DEFAULT_PAGES_NUMBER, "1234", false), false);

    QCOMPARE(vc->validatePagePIN(0, "4321", true), false);
    QCOMPARE(vc->page(0)->requirePIN(), true);
    QCOMPARE(vc->validatePagePIN(0, "1234", false), true);
    QCOMPARE(vc->page(0)->requirePIN(), true);
    QCOMPARE(vc->validatePagePIN(0, "1234", true), true);
    QCOMPARE(vc->page(0)->requirePIN(), false);

    /* An empty PIN removes it */
    QVERIFY(vc->setPagePIN(0, "1234", ""));
    QCOMPARE(vc->page(0)->PIN(), 0);
}

void VirtualConsole_Test::pageInputMapping()
{
    VCPage *page = m_env->vc()->page(0);

    /* Page own input sources and keys */
    QSharedPointer<QLCInputSource> src(new QLCInputSource(1, 15));
    src->setID(INPUT_ENABLE_ID);
    page->addInputSource(src);
    page->mapInputSource(src, page);
    QCOMPARE(page->pageInputSources(), QList<quint32>() << ((1 << 16) | 15));

    /* Invalid sources are not mapped */
    page->mapInputSource(QSharedPointer<QLCInputSource>(new QLCInputSource()), page);
    QCOMPARE(page->pageInputSources().count(), 1);

    page->mapKeySequence(QKeySequence("F1"), INPUT_ENABLE_ID, page);
    QCOMPARE(page->pageKeySequences(), QList<QKeySequence>() << QKeySequence("F1"));
    page->mapKeySequence(QKeySequence(), INPUT_ENABLE_ID, page);
    QCOMPARE(page->pageKeySequences().count(), 1);

    /* Children sources are not listed as page sources */
    VCWidget *btn = page->addWidget(nullptr, "Button", QPoint());
    QSharedPointer<QLCInputSource> btnSrc(new QLCInputSource(1, 16));
    btnSrc->setID(0);
    btn->addInputSource(btnSrc);
    m_env->vc()->mapWidgetInputs(btn);
    QCOMPARE(page->pageInputSources().count(), 1);

    page->resetInputSourcesMap();
    QVERIFY(page->pageInputSources().isEmpty());
    QVERIFY(page->pageKeySequences().isEmpty());

    /* Keys are rebuilt from the page and the children bindings */
    page->addKeySequence(QKeySequence("F2"), INPUT_ENABLE_ID);
    page->buildKeySequenceMap();
    QCOMPARE(page->pageKeySequences(), QList<QKeySequence>() << QKeySequence("F2"));

    /* Page scale is relative and applied to the children */
    page->setPageScale(0.5);
    QCOMPARE(page->pageScale(), qreal(1.5));
    QCOMPARE(btn->scaleFactor(), qreal(1.5));
}

void VirtualConsole_Test::resetContents()
{
    VirtualConsole *vc = m_env->vc();
    VCPage *page = vc->page(0);

    VCWidget *btn = page->addWidget(nullptr, "Button", QPoint());
    quint32 btnId = btn->id();
    page->setCaption("Custom");
    QVERIFY(vc->setPagePIN(0, "", "1234"));
    page->addKeySequence(QKeySequence("F3"), INPUT_ENABLE_ID);

    vc->resetContents();
    QCOMPARE(vc->pagesCount(), DEFAULT_PAGES_NUMBER);
    QCOMPARE(page->hasChildren(), false);
    QVERIFY(vc->widget(btnId) == nullptr);
    QCOMPARE(page->caption(), QString("Page 1"));
    QCOMPARE(page->PIN(), 0);
    QVERIFY(page->keySequenceMap().isEmpty());
    QCOMPARE(vc->loadStatus(), VirtualConsole::Cleared);
    QCOMPARE(vc->selectedPage(), 0);
}

void VirtualConsole_Test::saveLoad()
{
    VirtualConsole *vc = m_env->vc();

    VCWidget *btn = vc->page(0)->addWidget(nullptr, "Button", QPoint(10, 10));
    btn->setCaption("B0");
    VCFrame *frame = qobject_cast<VCFrame *>(vc->page(1)->addWidget(nullptr, "Frame", QPoint()));
    QVERIFY(frame != nullptr);
    VCWidget *label = frame->addWidget(nullptr, "Label", QPoint());
    label->setCaption("L1");
    vc->page(2)->setCaption("Third");

    quint32 btnId = btn->id();
    quint32 frameId = frame->id();
    quint32 labelId = label->id();

    QByteArray data = vcSaveXML(vc);
    VCXMLReader xml(data);
    QCOMPARE(xml.reader().name().toString(), QString("VirtualConsole"));

    vc->resetContents();
    QVERIFY(vc->widget(btnId) == nullptr);
    QCOMPARE(vc->page(2)->caption(), QString("Page 3"));

    QVERIFY(vc->loadXML(xml.reader()));
    QCOMPARE(vc->loadStatus(), VirtualConsole::Loaded);
    vc->postLoad();

    QCOMPARE(vc->pagesCount(), DEFAULT_PAGES_NUMBER);
    QCOMPARE(vc->page(0)->children().count(), 1);
    QCOMPARE(vc->page(1)->children().count(), 1);
    QCOMPARE(vc->page(2)->caption(), QString("Third"));

    /* The saved IDs are restored */
    VCWidget *btn2 = vc->widget(btnId);
    QVERIFY(btn2 != nullptr);
    QVERIFY(btn2->parent() == vc->page(0));
    QCOMPARE(btn2->type(), int(VCWidget::ButtonWidget));
    QCOMPARE(btn2->caption(), QString("B0"));

    VCWidget *frame2 = vc->widget(frameId);
    QVERIFY(frame2 != nullptr);
    QVERIFY(frame2->parent() == vc->page(1));

    VCWidget *label2 = vc->widget(labelId);
    QVERIFY(label2 != nullptr);
    QVERIFY(label2->parent() == frame2);
    QCOMPARE(label2->caption(), QString("L1"));

    /* Wrong tag */
    VCXMLReader wrongXml(QByteArray("<Frame/>"));
    QCOMPARE(vc->loadXML(wrongXml.reader()), false);
}

QTEST_MAIN(VirtualConsole_Test)
