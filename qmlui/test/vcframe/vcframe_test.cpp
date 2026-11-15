/*
  Q Light Controller Plus - Test Unit
  vcframe_test.cpp

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

#include "vcframe_test.h"
#include "qlcinputsource.h"
#include "vctestenv.h"
#include "vcslider.h"
#include "vcframe.h"

void VCFrame_Test::init()
{
    m_env = new VCTestEnv();
}

void VCFrame_Test::cleanup()
{
    delete m_env;
}

void VCFrame_Test::initial()
{
    VCFrame frame(m_env->doc(), m_env->vc());

    QCOMPARE(frame.type(), int(VCWidget::FrameWidget));
    QCOMPARE(frame.showHeader(), true);
    QCOMPARE(frame.showEnable(), true);
    QCOMPARE(frame.isCollapsed(), false);
    QCOMPARE(frame.multiPageMode(), false);
    QCOMPARE(frame.currentPage(), 0);
    QCOMPARE(frame.totalPagesNumber(), 1);
    QCOMPARE(frame.usedPagesNumber(), 1);
    QCOMPARE(frame.pagesLoop(), false);
    QVERIFY(frame.pageLabels().isEmpty());
    QCOMPARE(frame.PIN(), 0);
    QCOMPARE(frame.requirePIN(), false);
    QCOMPARE(frame.hasChildren(), false);
    QVERIFY(frame.children().isEmpty());
    QVERIFY(frame.childrenItem() == nullptr);
    /* Next/Previous page, enable and collapse */
    QCOMPARE(frame.externalControlsCount(), 4);
    QCOMPARE(frame.propertiesResource(), QString("qrc:/VCFrameProperties.qml"));

    frame.setID(1);
    QCOMPARE(frame.caption(), QString("Frame 2"));

    /* A top level frame is a VC page */
    VCFrame topFrame(m_env->doc(), m_env->vc(), m_env->vc());
    QCOMPARE(topFrame.propertiesResource(), QString("qrc:/VCPageProperties.qml"));
}

void VCFrame_Test::addWidgets()
{
    VirtualConsole *vc = m_env->vc();
    VCFrame frame(m_env->doc(), vc);

    QStringList types;
    types << "Frame" << "Solo frame" << "Button" << "Label" << "Slider" << "Knob"
          << "Animation" << "Audio Triggers" << "XYPad" << "Speed" << "Clock" << "Cue list";

    QList<int> expected;
    expected << VCWidget::FrameWidget << VCWidget::SoloFrameWidget << VCWidget::ButtonWidget
             << VCWidget::LabelWidget << VCWidget::SliderWidget << VCWidget::SliderWidget
             << VCWidget::AnimationWidget << VCWidget::AudioTriggersWidget << VCWidget::XYPadWidget
             << VCWidget::SpeedWidget << VCWidget::ClockWidget << VCWidget::CueListWidget;

    QSignalSpy spy(&frame, &VCFrame::usedPagesNumberChanged);

    for (int i = 0; i < types.count(); i++)
    {
        VCWidget *widget = frame.addWidget(nullptr, types.at(i), QPoint(10, 20));
        QVERIFY2(widget != nullptr, qPrintable(types.at(i)));
        QCOMPARE(widget->type(), expected.at(i));
        QVERIFY(widget->parent() == &frame);
        QVERIFY(widget->id() != VCWidget::invalidId());
        QVERIFY(vc->widget(widget->id()) == widget);
        QCOMPARE(widget->page(), 0);
        QCOMPARE(widget->geometry().topLeft(), QPointF(10, 20));
    }

    QCOMPARE(spy.count(), types.count());
    QCOMPARE(frame.hasChildren(), true);
    QCOMPARE(frame.children().count(), types.count());

    /* "Knob" is a slider with the knob style */
    int knobs = 0;
    for (VCWidget *widget : frame.children())
    {
        VCSlider *slider = qobject_cast<VCSlider *>(widget);
        if (slider != nullptr && slider->widgetStyle() == VCSlider::WKnob)
            knobs++;
    }
    QCOMPARE(knobs, 1);

    /* Unknown types are refused */
    QVERIFY(frame.addWidget(nullptr, "Foobar", QPoint()) == nullptr);
    QCOMPARE(frame.children().count(), types.count());

    /* Recursive listing includes the widgets of the child frames */
    VCFrame *child = nullptr;
    for (VCWidget *widget : frame.children())
    {
        if (widget->type() == VCWidget::FrameWidget)
            child = qobject_cast<VCFrame *>(widget);
    }
    QVERIFY(child != nullptr);
    VCWidget *grandChild = child->addWidget(nullptr, "Button", QPoint());
    QVERIFY(grandChild != nullptr);
    QCOMPARE(frame.children(false).count(), types.count());
    QCOMPARE(frame.children(true).count(), types.count() + 1);
    QVERIFY(frame.children(true).contains(grandChild));
}

void VCFrame_Test::deleteChildren()
{
    VirtualConsole *vc = m_env->vc();
    VCFrame frame(m_env->doc(), vc);

    VCFrame *child = qobject_cast<VCFrame *>(frame.addWidget(nullptr, "Frame", QPoint()));
    QVERIFY(child != nullptr);
    VCWidget *btn = frame.addWidget(nullptr, "Button", QPoint());
    VCWidget *grandChild = child->addWidget(nullptr, "Label", QPoint());

    quint32 childId = child->id();
    quint32 btnId = btn->id();
    quint32 grandChildId = grandChild->id();
    QVERIFY(vc->widget(childId) != nullptr);
    QVERIFY(vc->widget(btnId) != nullptr);
    QVERIFY(vc->widget(grandChildId) != nullptr);

    frame.deleteChildren();
    QCOMPARE(frame.hasChildren(), false);
    QVERIFY(vc->widget(childId) == nullptr);
    QVERIFY(vc->widget(btnId) == nullptr);
    QVERIFY(vc->widget(grandChildId) == nullptr);
}

void VCFrame_Test::multiPage()
{
    VCFrame frame(m_env->doc(), m_env->vc());

    QSignalSpy modeSpy(&frame, &VCFrame::multiPageModeChanged);
    frame.setMultiPageMode(true);
    QCOMPARE(frame.multiPageMode(), true);
    QCOMPARE(modeSpy.count(), 1);
    frame.setMultiPageMode(true);
    QCOMPARE(modeSpy.count(), 1);

    /* Enabling multipage registers the first page */
    QCOMPARE(frame.pageLabels(), QStringList() << "Page 1");
    QCOMPARE(frame.externalControlsCount(), 5);

    QSignalSpy totalSpy(&frame, &VCFrame::totalPagesNumberChanged);
    frame.setTotalPagesNumber(3);
    QCOMPARE(frame.totalPagesNumber(), 3);
    QCOMPARE(totalSpy.count(), 1);
    QCOMPARE(frame.pageLabels(), QStringList() << "Page 1" << "Page 2" << "Page 3");
    QCOMPARE(frame.externalControlsCount(), 7);

    VCWidget *btn = frame.addWidget(nullptr, "Button", QPoint());
    QCOMPARE(btn->page(), 0);
    QCOMPARE(btn->isVisible(), true);

    QSignalSpy pageSpy(&frame, &VCFrame::currentPageChanged);
    frame.gotoNextPage();
    QCOMPARE(frame.currentPage(), 1);
    QCOMPARE(btn->isVisible(), false);
    QCOMPARE(pageSpy.count(), 1);

    frame.gotoNextPage();
    QCOMPARE(frame.currentPage(), 2);

    /* No loop: the last page is the limit */
    frame.gotoNextPage();
    QCOMPARE(frame.currentPage(), 2);

    QSignalSpy loopSpy(&frame, &VCFrame::pagesLoopChanged);
    frame.setPagesLoop(true);
    QCOMPARE(frame.pagesLoop(), true);
    QCOMPARE(loopSpy.count(), 1);

    frame.gotoNextPage();
    QCOMPARE(frame.currentPage(), 0);
    QCOMPARE(btn->isVisible(), true);

    frame.gotoPreviousPage();
    QCOMPARE(frame.currentPage(), 2);

    /* Out of range pages are ignored */
    frame.gotoPage(5);
    QCOMPARE(frame.currentPage(), 2);
    frame.gotoPage(-1);
    QCOMPARE(frame.currentPage(), 2);
    frame.gotoPage(1);
    QCOMPARE(frame.currentPage(), 1);

    frame.setShortcutName(1, "Foo");
    QCOMPARE(frame.pageLabels().at(1), QString("Foo"));
    QCOMPARE(frame.externalControlsCount(), 7);

    /* Without multipage the current page is always 0 */
    frame.setMultiPageMode(false);
    QCOMPARE(frame.currentPage(), 0);
}

void VCFrame_Test::totalPagesKeepsUsedPages()
{
    VCFrame frame(m_env->doc(), m_env->vc());
    frame.setMultiPageMode(true);
    frame.setTotalPagesNumber(3);
    frame.setCurrentPage(2);

    /* New widgets are placed on the current page */
    VCWidget *label = frame.addWidget(nullptr, "Label", QPoint());
    QCOMPARE(label->page(), 2);
    QCOMPARE(frame.usedPagesNumber(), 3);

    /* Pages with widgets are never dropped */
    frame.setTotalPagesNumber(1);
    QCOMPARE(frame.totalPagesNumber(), 3);

    frame.setTotalPagesNumber(5);
    QCOMPARE(frame.totalPagesNumber(), 5);
    QCOMPARE(frame.pageLabels().count(), 5);

    frame.setTotalPagesNumber(4);
    QCOMPARE(frame.totalPagesNumber(), 4);
    QCOMPARE(frame.pageLabels().count(), 4);
    QCOMPARE(frame.currentPage(), 2);
}

void VCFrame_Test::cloneFirstPage()
{
    VCFrame frame(m_env->doc(), m_env->vc());

    /* Nothing to do with a single page */
    VCWidget *btn = frame.addWidget(nullptr, "Button", QPoint());
    btn->setCaption("Clone me");
    frame.cloneFirstPage();
    QCOMPARE(frame.children().count(), 1);

    QSharedPointer<QLCInputSource> src(new QLCInputSource(0, 5));
    src->setID(0);
    btn->addInputSource(src);

    frame.setMultiPageMode(true);
    frame.setTotalPagesNumber(3);
    frame.cloneFirstPage();

    QList<VCWidget *> children = frame.children();
    QCOMPARE(children.count(), 3);

    QList<int> pages;
    for (VCWidget *widget : children)
    {
        QCOMPARE(widget->type(), int(VCWidget::ButtonWidget));
        QCOMPARE(widget->caption(), QString("Clone me"));
        pages.append(widget->page());

        /* The input sources follow the page of the clone */
        QCOMPARE(widget->inputSources().count(), 1);
        QCOMPARE(int(widget->inputSources().first()->page()), widget->page());
        QCOMPARE(widget->inputSources().first()->channel() & 0xFFFF, quint32(5));
    }
    std::sort(pages.begin(), pages.end());
    QCOMPARE(pages, QList<int>() << 0 << 1 << 2);
}

void VCFrame_Test::disable()
{
    VCFrame frame(m_env->doc(), m_env->vc());
    VCFrame *child = qobject_cast<VCFrame *>(frame.addWidget(nullptr, "Frame", QPoint()));
    VCWidget *btn = frame.addWidget(nullptr, "Button", QPoint());
    VCWidget *grandChild = child->addWidget(nullptr, "Button", QPoint());

    frame.setDisabled(true);
    QCOMPARE(frame.isDisabled(), true);
    QCOMPARE(child->isDisabled(), true);
    QCOMPARE(btn->isDisabled(), true);
    QCOMPARE(grandChild->isDisabled(), true);

    frame.setDisabled(false);
    QCOMPARE(frame.isDisabled(), false);
    QCOMPARE(child->isDisabled(), false);
    QCOMPARE(btn->isDisabled(), false);
    QCOMPARE(grandChild->isDisabled(), false);
}

void VCFrame_Test::appearanceFlags()
{
    VCFrame frame(m_env->doc(), m_env->vc());

    QSignalSpy headerSpy(&frame, &VCFrame::showHeaderChanged);
    frame.setShowHeader(false);
    QCOMPARE(frame.showHeader(), false);
    QCOMPARE(headerSpy.count(), 1);
    frame.setShowHeader(false);
    QCOMPARE(headerSpy.count(), 1);

    QSignalSpy enableSpy(&frame, &VCFrame::showEnableChanged);
    frame.setShowEnable(false);
    QCOMPARE(frame.showEnable(), false);
    QCOMPARE(enableSpy.count(), 1);
    frame.setShowEnable(false);
    QCOMPARE(enableSpy.count(), 1);

    m_env->doc()->resetModified();
    QSignalSpy collapseSpy(&frame, &VCFrame::collapsedChanged);
    frame.setCollapsed(true);
    QCOMPARE(frame.isCollapsed(), true);
    QCOMPARE(collapseSpy.count(), 1);
    QCOMPARE(m_env->doc()->isModified(), true);
    frame.setCollapsed(true);
    QCOMPARE(collapseSpy.count(), 1);
}

void VCFrame_Test::pin()
{
    VCFrame frame(m_env->doc(), m_env->vc());

    QSignalSpy spy(&frame, &VCFrame::PINChanged);
    frame.setPIN(1234);
    QCOMPARE(frame.PIN(), 1234);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(frame.requirePIN(), true);

    frame.setPIN(1234);
    QCOMPARE(spy.count(), 1);

    frame.validatePIN();
    QCOMPARE(frame.requirePIN(), false);
}

void VCFrame_Test::inputValue()
{
    VCFrame frame(m_env->doc(), m_env->vc());

    /* Only full values are considered */
    frame.slotInputValueChanged(INPUT_ENABLE_ID, 127);
    QCOMPARE(frame.isDisabled(), false);

    frame.slotInputValueChanged(INPUT_ENABLE_ID, 255);
    QCOMPARE(frame.isDisabled(), true);
    frame.slotInputValueChanged(INPUT_ENABLE_ID, 255);
    QCOMPARE(frame.isDisabled(), false);

    frame.slotInputValueChanged(INPUT_COLLAPSE_ID, 255);
    QCOMPARE(frame.isCollapsed(), true);
    frame.slotInputValueChanged(INPUT_COLLAPSE_ID, 255);
    QCOMPARE(frame.isCollapsed(), false);

    frame.setMultiPageMode(true);
    frame.setTotalPagesNumber(3);

    frame.slotInputValueChanged(INPUT_NEXT_PAGE_ID, 255);
    QCOMPARE(frame.currentPage(), 1);
    frame.slotInputValueChanged(INPUT_PREVIOUS_PAGE_ID, 255);
    QCOMPARE(frame.currentPage(), 0);

    frame.slotInputValueChanged(INPUT_SHORTCUT_BASE_ID + 2, 255);
    QCOMPARE(frame.currentPage(), 2);
    frame.slotInputValueChanged(INPUT_SHORTCUT_BASE_ID, 255);
    QCOMPARE(frame.currentPage(), 0);

    /* IDs between the fixed controls and the shortcuts do nothing */
    frame.slotInputValueChanged(10, 255);
    QCOMPARE(frame.currentPage(), 0);
}

void VCFrame_Test::copy()
{
    VirtualConsole *vc = m_env->vc();
    VCFrame parentFrame(m_env->doc(), vc);

    VCFrame frame(m_env->doc(), vc);
    frame.setCaption("Source");
    frame.setShowHeader(false);
    frame.setShowEnable(false);
    frame.setMultiPageMode(true);
    frame.setTotalPagesNumber(2);
    frame.setPagesLoop(true);
    frame.setShortcutName(1, "Second");
    VCWidget *child = frame.addWidget(nullptr, "Button", QPoint(1, 1));
    child->setCaption("Child");
    frame.setCurrentPage(1);

    VCFrame *copy = qobject_cast<VCFrame *>(frame.createCopy(&parentFrame));
    QVERIFY(copy != nullptr);
    QVERIFY(copy->parent() == &parentFrame);
    QCOMPARE(copy->type(), int(VCWidget::FrameWidget));
    QCOMPARE(copy->caption(), QString("Source"));
    QCOMPARE(copy->showHeader(), false);
    QCOMPARE(copy->showEnable(), false);
    QCOMPARE(copy->multiPageMode(), true);
    QCOMPARE(copy->totalPagesNumber(), 2);
    QCOMPARE(copy->pagesLoop(), true);
    QCOMPARE(copy->pageLabels().at(1), QString("Second"));
    QCOMPARE(copy->currentPage(), 1);

    /* Children are copied too, with a new ID */
    QList<VCWidget *> children = copy->children();
    QCOMPARE(children.count(), 1);
    VCWidget *childCopy = children.first();
    QVERIFY(childCopy != child);
    QVERIFY(childCopy->parent() == copy);
    QCOMPARE(childCopy->caption(), QString("Child"));
    QVERIFY(childCopy->id() != child->id());
    QVERIFY(vc->widget(childCopy->id()) == childCopy);
    QCOMPARE(childCopy->isVisible(), false);
}

void VCFrame_Test::saveLoad()
{
    VirtualConsole *vc = m_env->vc();

    VCFrame frame(m_env->doc(), vc);
    frame.setID(50);
    frame.setCaption("Main");
    frame.setGeometry(QRectF(0, 0, 400, 300));
    frame.setShowHeader(false);
    frame.setShowEnable(false);
    frame.setCollapsed(true);
    frame.setPIN(4321);
    frame.setMultiPageMode(true);
    frame.setTotalPagesNumber(3);
    frame.setPagesLoop(true);
    frame.setShortcutName(2, "Last");

    VCWidget *btn = frame.addWidget(nullptr, "Button", QPoint(10, 10));
    btn->setCaption("Btn");
    frame.setCurrentPage(1);
    VCWidget *label = frame.addWidget(nullptr, "Label", QPoint(20, 20));
    label->setCaption("Lbl");

    QByteArray data = vcSaveXML(&frame);
    /* The PIN is never saved in clear */
    QVERIFY(data.contains("<PIN>"));
    QVERIFY(data.contains("4321") == false);

    VCXMLReader xml(data);
    QCOMPARE(xml.reader().name().toString(), QString("Frame"));

    VCFrame frame2(m_env->doc(), vc);
    QVERIFY(frame2.loadXML(xml.reader()));

    QCOMPARE(frame2.id(), quint32(50));
    QCOMPARE(frame2.caption(), QString("Main"));
    QCOMPARE(frame2.geometry(), QRectF(0, 0, 400, 300));
    QCOMPARE(frame2.showHeader(), false);
    QCOMPARE(frame2.showEnable(), false);
    QCOMPARE(frame2.isCollapsed(), true);
    QCOMPARE(frame2.PIN(), 4321);
    QCOMPARE(frame2.multiPageMode(), true);
    QCOMPARE(frame2.totalPagesNumber(), 3);
    QCOMPARE(frame2.pagesLoop(), true);
    QCOMPARE(frame2.currentPage(), 1);
    QCOMPARE(frame2.pageLabels().count(), 3);
    QCOMPARE(frame2.pageLabels().at(2), QString("Last"));

    QList<VCWidget *> children = frame2.children();
    QCOMPARE(children.count(), 2);

    VCWidget *btn2 = nullptr;
    VCWidget *label2 = nullptr;
    for (VCWidget *widget : children)
    {
        if (widget->type() == VCWidget::ButtonWidget)
            btn2 = widget;
        else if (widget->type() == VCWidget::LabelWidget)
            label2 = widget;
    }
    QVERIFY(btn2 != nullptr);
    QVERIFY(label2 != nullptr);
    QCOMPARE(btn2->caption(), QString("Btn"));
    QCOMPARE(btn2->page(), 0);
    QCOMPARE(btn2->isVisible(), false);
    QCOMPARE(label2->caption(), QString("Lbl"));
    QCOMPARE(label2->page(), 1);
    QCOMPARE(label2->isVisible(), true);

    /* The loaded children are in the VC map, with non conflicting IDs */
    QVERIFY(vc->widget(btn2->id()) == btn2);
    QVERIFY(vc->widget(label2->id()) == label2);
    QVERIFY(btn2->id() != btn->id());
    QVERIFY(label2->id() != label->id());
}

void VCFrame_Test::saveLoadDisabled()
{
    VCFrame frame(m_env->doc(), m_env->vc());
    frame.addWidget(nullptr, "Button", QPoint());
    frame.setDisabled(true);
    frame.setAllowResize(false);

    QByteArray data = vcSaveXML(&frame);
    QVERIFY(data.contains("<Disabled>True</Disabled>"));
    QVERIFY(data.contains("<AllowResize>False</AllowResize>"));

    VCXMLReader xml(data);
    VCFrame frame2(m_env->doc(), m_env->vc());
    QVERIFY(frame2.loadXML(xml.reader()));

    QCOMPARE(frame2.isDisabled(), true);
    QCOMPARE(frame2.allowResize(), false);

    /* The disabled state is propagated to the loaded children */
    QCOMPARE(frame2.children().count(), 1);
    QCOMPARE(frame2.children().first()->isDisabled(), true);
}

void VCFrame_Test::loadWrongTag()
{
    VCFrame frame(m_env->doc(), m_env->vc());

    VCXMLReader xml(QByteArray("<SoloFrame Caption=\"Foo\"/>"));
    QCOMPARE(frame.loadXML(xml.reader()), false);
    QVERIFY(frame.caption().isEmpty());
}

QTEST_MAIN(VCFrame_Test)
