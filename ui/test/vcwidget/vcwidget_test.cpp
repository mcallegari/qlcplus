/*
  Q Light Controller
  vcwidget_test.cpp

  Copyright (C) Heikki Junnila

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

#include <QFrame>
#include <QtTest>
#include <QtXml>

#define protected public
#define private public
#include "qlcfixturedefcache.h"
#include "virtualconsole.h"
#include "qlcinputsource.h"
#include "vcwidget_test.h"
#include "mastertimer.h"
#include "stubwidget.h"
#include "vcwidget.h"
#include "vcframe.h"
#include "doc.h"
#undef private
#undef protected

void VCWidget_Test::initTestCase()
{
    m_doc = NULL;
}

void VCWidget_Test::init()
{
    m_doc = new Doc(this);
    new VirtualConsole(NULL, m_doc);
}

void VCWidget_Test::cleanup()
{
    delete VirtualConsole::instance();
    delete m_doc;
}

void VCWidget_Test::initial()
{
    QWidget w;

    StubWidget stub(&w, m_doc);
    QCOMPARE(stub.objectName(), QString("VCWidget"));
    QCOMPARE(stub.hasCustomBackgroundColor(), false);
    QCOMPARE(stub.hasCustomForegroundColor(), false);
    QCOMPARE(stub.hasCustomFont(), false);
    QCOMPARE(stub.frameStyle(), 0);
    QCOMPARE(stub.allowChildren(), false);
    QCOMPARE(stub.customMenu(0), (QMenu*) 0);
    QCOMPARE(stub.lastClickPoint(), QPoint(0, 0));

    for (quint8 i = 0; i < 255; i++)
        QVERIFY(stub.inputSource(i) == NULL);
}

void VCWidget_Test::bgImage()
{
    QWidget w;

    QSignalSpy spy(m_doc, SIGNAL(modified(bool)));

    StubWidget stub(&w, m_doc);
    QCOMPARE(stub.backgroundImage(), QString());
    QCOMPARE(stub.hasCustomBackgroundColor(), false);

    stub.setBackgroundImage("../../../resources/icons/png/qlcplus.png");
    QCOMPARE(stub.backgroundImage(), QString("../../../resources/icons/png/qlcplus.png"));
    QCOMPARE(stub.palette().brush(QPalette::Window).texture().isNull(), false);
    QCOMPARE(stub.hasCustomBackgroundColor(), false);
    QCOMPARE(spy.size(), 1);

    stub.setBackgroundColor(QColor(Qt::red));
    QCOMPARE(spy.size(), 2);

    stub.setBackgroundImage("../../../resources/icons/png/qlcplus.png");
    QCOMPARE(stub.backgroundImage(), QString("../../../resources/icons/png/qlcplus.png"));
    QCOMPARE(stub.palette().brush(QPalette::Window).texture().isNull(), false);
    QCOMPARE(stub.hasCustomBackgroundColor(), false);
    QCOMPARE(spy.size(), 3);
}

void VCWidget_Test::bgColor()
{
    QWidget w;

    QSignalSpy spy(m_doc, SIGNAL(modified(bool)));

    StubWidget stub(&w, m_doc);
    QCOMPARE(stub.backgroundColor(), w.palette().color(QPalette::Window));
    QCOMPARE(stub.hasCustomBackgroundColor(), false);

    stub.setBackgroundColor(QColor(Qt::red));
    QCOMPARE(stub.backgroundImage(), QString());
    QCOMPARE(stub.backgroundColor(), QColor(Qt::red));
    QCOMPARE(stub.hasCustomBackgroundColor(), true);
    QCOMPARE(stub.palette().brush(QPalette::Window).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::Window).color(), QColor(Qt::red));
    QCOMPARE(spy.size(), 1);

    stub.setBackgroundImage("../../../resources/icons/png/qlcplus.png");
    QCOMPARE(spy.size(), 2);

    stub.setBackgroundColor(QColor(Qt::red));
    QCOMPARE(stub.backgroundImage(), QString());
    QCOMPARE(stub.backgroundColor(), QColor(Qt::red));
    QCOMPARE(stub.palette().brush(QPalette::Window).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::Window).color(), QColor(Qt::red));
    QCOMPARE(spy.size(), 3);
}

void VCWidget_Test::fgColor()
{
    QWidget w;

    QSignalSpy spy(m_doc, SIGNAL(modified(bool)));

    StubWidget stub(&w, m_doc);
    QCOMPARE(stub.backgroundColor(), w.palette().color(QPalette::Window));
    QCOMPARE(stub.hasCustomBackgroundColor(), false);

    stub.setForegroundColor(QColor(Qt::blue));
    QCOMPARE(stub.foregroundColor(), QColor(Qt::blue));
    QCOMPARE(stub.hasCustomForegroundColor(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).color(), QColor(Qt::blue));
    QCOMPARE(spy.size(), 1);
}

void VCWidget_Test::resetBg()
{
    QWidget w;

    QSignalSpy spy(m_doc, SIGNAL(modified(bool)));

    StubWidget stub(&w, m_doc);
    stub.setBackgroundColor(QColor(Qt::red));
    stub.setForegroundColor(QColor(Qt::cyan));
    QCOMPARE(spy.size(), 2);

    stub.resetBackgroundColor();
    QCOMPARE(stub.backgroundImage(), QString());
    QCOMPARE(stub.backgroundColor(), w.palette().color(QPalette::Window));
    QCOMPARE(stub.hasCustomBackgroundColor(), false);
    QCOMPARE(stub.palette().brush(QPalette::Window).color(), w.palette().color(QPalette::Window));
    QCOMPARE(stub.foregroundColor(), QColor(Qt::cyan));
    QCOMPARE(stub.hasCustomForegroundColor(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).color(), QColor(Qt::cyan));
    QCOMPARE(spy.size(), 3);

    stub.setBackgroundImage("../../../resources/icons/png/qlcplus.png");
    QCOMPARE(spy.size(), 4);

    stub.resetBackgroundColor();
    QCOMPARE(stub.backgroundImage(), QString());
    QCOMPARE(stub.backgroundColor(), w.palette().color(QPalette::Window));
    QCOMPARE(stub.hasCustomBackgroundColor(), false);
    QCOMPARE(stub.palette().brush(QPalette::Window).color(), w.palette().color(QPalette::Window));
    QCOMPARE(stub.foregroundColor(), QColor(Qt::cyan));
    QCOMPARE(stub.hasCustomForegroundColor(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).color(), QColor(Qt::cyan));
    QCOMPARE(spy.size(), 5);
}

void VCWidget_Test::resetFg()
{
    QWidget w;

    QSignalSpy spy(m_doc, SIGNAL(modified(bool)));

    StubWidget stub(&w, m_doc);
    stub.setBackgroundColor(QColor(Qt::red));
    stub.setForegroundColor(QColor(Qt::cyan));
    QCOMPARE(spy.size(), 2);

    stub.resetForegroundColor();
    QCOMPARE(stub.backgroundImage(), QString());
    QCOMPARE(stub.backgroundColor(), QColor(Qt::red));
    QCOMPARE(stub.hasCustomBackgroundColor(), true);
    QCOMPARE(stub.palette().brush(QPalette::Window).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::Window).color(), QColor(Qt::red));
    QCOMPARE(stub.foregroundColor(), w.palette().color(QPalette::WindowText));
    QCOMPARE(stub.hasCustomForegroundColor(), false);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).color(), w.palette().color(QPalette::WindowText));
    QCOMPARE(spy.size(), 3);

    stub.setBackgroundImage("../../../resources/icons/png/qlcplus.png");
    QCOMPARE(spy.size(), 4);

    stub.resetForegroundColor();
    QCOMPARE(stub.backgroundImage(), QString("../../../resources/icons/png/qlcplus.png"));
    QCOMPARE(stub.hasCustomBackgroundColor(), false);
    QCOMPARE(stub.palette().brush(QPalette::Window).texture().isNull(), false);
    QCOMPARE(stub.foregroundColor(), w.palette().color(QPalette::WindowText));
    QCOMPARE(stub.hasCustomForegroundColor(), false);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).color(), w.palette().color(QPalette::WindowText));
    QCOMPARE(spy.size(), 5);

    stub.resetBackgroundColor();
    QCOMPARE(spy.size(), 6);

    stub.resetForegroundColor();
    QCOMPARE(stub.backgroundImage(), QString());
    QCOMPARE(stub.hasCustomBackgroundColor(), false);
    QCOMPARE(stub.foregroundColor(), w.palette().color(QPalette::WindowText));
    QCOMPARE(stub.hasCustomForegroundColor(), false);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).color(), w.palette().color(QPalette::WindowText));
    QCOMPARE(spy.size(), 7);
}

void VCWidget_Test::font()
{
    QWidget w;

    QSignalSpy spy(m_doc, SIGNAL(modified(bool)));

    QFont font(w.font());
    font.setBold(!font.bold());
    QVERIFY(font != w.font());

    StubWidget stub(&w, m_doc);
    stub.setFont(font);
    QCOMPARE(stub.font(), font);
    QCOMPARE(stub.hasCustomFont(), true);
    QCOMPARE(spy.size(), 1);

    stub.resetFont();
    QCOMPARE(stub.font(), w.font());
    QCOMPARE(stub.hasCustomFont(), false);
    QCOMPARE(spy.size(), 2);
}

void VCWidget_Test::caption()
{
    QWidget w;

    QSignalSpy spy(m_doc, SIGNAL(modified(bool)));

    StubWidget stub(&w, m_doc);
    stub.setCaption("Foobar");
    QCOMPARE(stub.caption(), QString("Foobar"));
    QCOMPARE(spy.size(), 1);
}

void VCWidget_Test::frame()
{
    QWidget w;

    QSignalSpy spy(m_doc, SIGNAL(modified(bool)));

    StubWidget stub(&w, m_doc);
    stub.setFrameStyle(KVCFrameStyleSunken);
    QCOMPARE(stub.frameStyle(), (int) KVCFrameStyleSunken);
    QCOMPARE(spy.size(), 1);

    stub.setFrameStyle(KVCFrameStyleRaised);
    QCOMPARE(stub.frameStyle(), (int) KVCFrameStyleRaised);
    QCOMPARE(spy.size(), 2);

    stub.resetFrameStyle();
    QCOMPARE(stub.frameStyle(), (int) KVCFrameStyleNone);
    QCOMPARE(spy.size(), 3);

    QCOMPARE(stub.frameStyleToString(KVCFrameStyleSunken), QString("Sunken"));
    QCOMPARE(stub.frameStyleToString(KVCFrameStyleRaised), QString("Raised"));
    QCOMPARE(stub.frameStyleToString(KVCFrameStyleNone), QString("None"));
    QCOMPARE(stub.frameStyleToString(QFrame::Plain), QString("None"));

    QCOMPARE(stub.stringToFrameStyle("Sunken"), (int) KVCFrameStyleSunken);
    QCOMPARE(stub.stringToFrameStyle("Raised"), (int) KVCFrameStyleRaised);
    QCOMPARE(stub.stringToFrameStyle("None"), (int) KVCFrameStyleNone);
    QCOMPARE(stub.stringToFrameStyle("Foo"), (int) KVCFrameStyleNone);
}

void VCWidget_Test::inputSource()
{
    QLCInputSource *src;
    QWidget w;

    StubWidget stub(&w, m_doc);
    stub.setInputSource(new QLCInputSource(1, 2));
    src = stub.inputSource();
    QVERIFY(src->isValid() == true);
    QCOMPARE(src->universe(), quint32(1));
    QCOMPARE(src->channel(), quint32(2));

    src = stub.inputSource(0);
    QVERIFY(src->isValid() == true);
    QCOMPARE(src->universe(), quint32(1));
    QCOMPARE(src->channel(), quint32(2));

    src = stub.inputSource(1);
    QVERIFY(src == NULL);
    src = stub.inputSource(2);
    QVERIFY(src == NULL);
    src = stub.inputSource(42);
    QVERIFY(src == NULL);

    stub.setInputSource(new QLCInputSource(3, 4), 0);
    src = stub.inputSource();
    QVERIFY(src->isValid() == true);
    QCOMPARE(src->universe(), quint32(3));
    QCOMPARE(src->channel(), quint32(4));

    stub.setInputSource(new QLCInputSource());
    src = stub.inputSource();
    QVERIFY(src == NULL);

    // Just for coverage - the implementation does nothing
    stub.slotInputValueChanged(0, 1, 2);
}

void VCWidget_Test::copy()
{
    QWidget w;

    StubWidget stub(&w, m_doc);
    stub.setCaption("Pertti Pasanen");
    stub.setBackgroundColor(QColor(Qt::red));
    stub.setForegroundColor(QColor(Qt::green));
    QFont font(w.font());
    font.setBold(!font.bold());
    stub.setFont(font);
    stub.setFrameStyle(KVCFrameStyleRaised);
    stub.move(QPoint(10, 20));
    stub.resize(QSize(20, 30));
    stub.setInputSource(new QLCInputSource(0, 12));
    stub.setInputSource(new QLCInputSource(1, 2), 15);
    stub.setInputSource(new QLCInputSource(3, 4), 1);

    StubWidget copy(&w, m_doc);
    copy.copyFrom(&stub);
    QCOMPARE(copy.caption(), QString("Pertti Pasanen"));
    QCOMPARE(copy.hasCustomBackgroundColor(), true);
    QCOMPARE(copy.backgroundColor(), QColor(Qt::red));
    QCOMPARE(copy.hasCustomForegroundColor(), true);
    QCOMPARE(copy.foregroundColor(), QColor(Qt::green));
    QCOMPARE(copy.font(), font);
    QCOMPARE(copy.frameStyle(), (int) KVCFrameStyleRaised);
    QCOMPARE(copy.pos(), QPoint(10, 20));
    QCOMPARE(copy.size(), QSize(20, 30));

    QLCInputSource *src1 = new QLCInputSource(0, 12);
    QCOMPARE(copy.inputSource()->universe(), src1->universe());
    QCOMPARE(copy.inputSource()->channel(), src1->channel());

    QLCInputSource *src2 = new QLCInputSource(1, 2);
    QCOMPARE(copy.inputSource(15)->universe(), src2->universe());
    QCOMPARE(copy.inputSource(15)->channel(), src2->channel());

    QLCInputSource *src3 = new QLCInputSource(3, 4);
    QCOMPARE(copy.inputSource(1)->universe(), src3->universe());
    QCOMPARE(copy.inputSource(1)->channel(), src3->channel());

    QVERIFY(copy.inputSource(2) == NULL);
}

void VCWidget_Test::stripKeySequence()
{
    QCOMPARE(VCWidget::stripKeySequence(QKeySequence("P")), QKeySequence("P"));
    QCOMPARE(VCWidget::stripKeySequence(QKeySequence("CTRL+P")), QKeySequence("P"));
    QCOMPARE(VCWidget::stripKeySequence(QKeySequence("ALT+P")), QKeySequence("ALT+P"));
    QCOMPARE(VCWidget::stripKeySequence(QKeySequence("CTRL+ALT+P")), QKeySequence("ALT+P"));
    QCOMPARE(VCWidget::stripKeySequence(QKeySequence("CTRL+ALT")), QKeySequence("ALT"));
    QCOMPARE(VCWidget::stripKeySequence(QKeySequence("SHIFT+CTRL+ALT+P")), QKeySequence("SHIFT+ALT+P"));
}

void VCWidget_Test::keyPress()
{
    QWidget w;

    StubWidget stub(&w, m_doc);
    QSignalSpy pspy(&stub, SIGNAL(keyPressed(QKeySequence)));
    QSignalSpy rspy(&stub, SIGNAL(keyReleased(QKeySequence)));

    stub.slotKeyPressed(QKeySequence(QKeySequence::Copy));
    QCOMPARE(pspy.size(), 1);
    QCOMPARE(pspy[0].size(), 1);
    QCOMPARE(pspy[0][0].value<QKeySequence>(), QKeySequence(QKeySequence::Copy));

    stub.slotKeyReleased(QKeySequence(QKeySequence::Copy));
    QCOMPARE(rspy.size(), 1);
    QCOMPARE(rspy[0].size(), 1);
    QCOMPARE(rspy[0][0].value<QKeySequence>(), QKeySequence(QKeySequence::Copy));
}

void VCWidget_Test::loadInput()
{
    QWidget w;

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("Input");
    root.setAttribute("Universe", "12");
    root.setAttribute("Channel", "34");
    xmldoc.appendChild(root);

    StubWidget stub(&w, m_doc);
    QCOMPARE(stub.loadXMLInput(&root), true);

    QLCInputSource *src = new QLCInputSource(12, 34);
    QCOMPARE(stub.inputSource()->universe(), src->universe());
    QCOMPARE(stub.inputSource()->channel(), src->channel());

    root.setTagName("Output");
    QCOMPARE(stub.loadXMLInput(&root), false);
    QCOMPARE(stub.inputSource()->universe(), src->universe());
    QCOMPARE(stub.inputSource()->channel(), src->channel());
}

void VCWidget_Test::loadAppearance()
{
    QWidget w;

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("Appearance");
    xmldoc.appendChild(root);

    QDomElement frame = xmldoc.createElement("FrameStyle");
    QDomText frameText = xmldoc.createTextNode("Sunken");
    frame.appendChild(frameText);
    root.appendChild(frame);

    QDomElement fg = xmldoc.createElement("ForegroundColor");
    QDomText fgText = xmldoc.createTextNode(QString("%1").arg(QColor(Qt::red).rgb()));
    fg.appendChild(fgText);
    root.appendChild(fg);

    QDomElement bg = xmldoc.createElement("BackgroundColor");
    QDomText bgText = xmldoc.createTextNode(QString("%1").arg(QColor(Qt::blue).rgb()));
    bg.appendChild(bgText);
    root.appendChild(bg);

    QDomElement bgImage = xmldoc.createElement("BackgroundImage");
    QDomText bgImageText = xmldoc.createTextNode("None");
    bgImage.appendChild(bgImageText);
    root.appendChild(bgImage);

    QDomElement foo = xmldoc.createElement("Foo");
    root.appendChild(foo);

    QFont font(w.font());
    font.setItalic(true);
    QDomElement fn = xmldoc.createElement("Font");
    QDomText fnText = xmldoc.createTextNode(font.toString());
    fn.appendChild(fnText);
    root.appendChild(fn);

    StubWidget stub(&w, m_doc);
    QVERIFY(stub.loadXMLAppearance(&root) == true);
    QCOMPARE(stub.frameStyle(), (int) KVCFrameStyleSunken);
    QCOMPARE(stub.hasCustomForegroundColor(), true);
    QCOMPARE(stub.foregroundColor(), QColor(Qt::red));
    QCOMPARE(stub.hasCustomBackgroundColor(), true);
    QCOMPARE(stub.backgroundColor(), QColor(Qt::blue));
    QCOMPARE(stub.font(), font);

    fgText.setData("Default");
    bgText.setData("Default");
    bgImageText.setData("../../../resources/icons/png/qlcplus.png");
    QVERIFY(stub.loadXMLAppearance(&root) == true);
    QCOMPARE(stub.frameStyle(), (int) KVCFrameStyleSunken);
    QCOMPARE(stub.hasCustomForegroundColor(), false);
    QCOMPARE(stub.hasCustomBackgroundColor(), false);
    QCOMPARE(stub.backgroundImage(), QFileInfo("../../../resources/icons/png/qlcplus.png").absoluteFilePath());
    QCOMPARE(stub.font(), font);

    root.setTagName("Appiarenz");
    QVERIFY(stub.loadXMLAppearance(&root) == false);
    QCOMPARE(stub.frameStyle(), (int) KVCFrameStyleSunken);
    QCOMPARE(stub.hasCustomForegroundColor(), false);
    QCOMPARE(stub.hasCustomBackgroundColor(), false);
    QCOMPARE(stub.backgroundImage(), QFileInfo("../../../resources/icons/png/qlcplus.png").absoluteFilePath());
    QCOMPARE(stub.font(), font);
}

void VCWidget_Test::saveInput()
{
    QWidget w;

    StubWidget stub(&w, m_doc);

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("Root");
    xmldoc.appendChild(root);

    QVERIFY(stub.saveXMLInput(&xmldoc, &root) == false);
    QCOMPARE(root.childNodes().count(), 0);

    stub.setInputSource(new QLCInputSource(34, 56));
    QVERIFY(stub.saveXMLInput(&xmldoc, &root) == true);
    QCOMPARE(root.childNodes().count(), 1);
    QCOMPARE(root.firstChild().toElement().tagName(), QString("Input"));
    QCOMPARE(root.firstChild().toElement().attribute("Universe"), QString("34"));
    QCOMPARE(root.firstChild().toElement().attribute("Channel"), QString("56"));

    root.clear();

    stub.setInputSource(new QLCInputSource(34, 56), 1);
    QVERIFY(stub.saveXMLInput(&xmldoc, &root) == true);
    QCOMPARE(root.childNodes().count(), 0);
}

void VCWidget_Test::saveAppearance()
{
    QWidget w;

    StubWidget stub(&w, m_doc);
    stub.setBackgroundColor(QColor(Qt::red));
    stub.setForegroundColor(QColor(Qt::green));
    QFont fn(w.font());
    fn.setBold(!fn.bold());
    stub.setFont(fn);
    stub.setFrameStyle(KVCFrameStyleRaised);

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("Root");
    xmldoc.appendChild(root);

    int bgcolor = 0, bgimage = 0, fgcolor = 0, font = 0, frame = 0;

    QCOMPARE(stub.saveXMLAppearance(&xmldoc, &root), true);
    QDomNode node = root.firstChild();
    QCOMPARE(node.toElement().tagName(), QString("Appearance"));
    node = node.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == "BackgroundColor")
        {
            bgcolor++;
            QCOMPARE(tag.text(), QString::number(QColor(Qt::red).rgb()));
        }
        else if (tag.tagName() == "BackgroundImage")
        {
            bgimage++;
            QCOMPARE(tag.text(), QString("None"));
        }
        else if (tag.tagName() == "ForegroundColor")
        {
            fgcolor++;
            QCOMPARE(tag.text(), QString::number(QColor(Qt::green).rgb()));
        }
        else if (tag.tagName() == "Font")
        {
            font++;
            QCOMPARE(tag.text(), fn.toString());
        }
        else if (tag.tagName() == "FrameStyle")
        {
            frame++;
            QCOMPARE(tag.text(), QString("Raised"));
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(tag.tagName()).toUtf8().constData());
        }

        node = node.nextSibling();
    }

    QCOMPARE(bgcolor, 1);
    QCOMPARE(bgimage, 1);
    QCOMPARE(fgcolor, 1);
    QCOMPARE(font, 1);
    QCOMPARE(frame, 1);
}

void VCWidget_Test::saveAppearanceDefaultsImage()
{
    QWidget w;

    StubWidget stub(&w, m_doc);
    stub.setBackgroundImage("../../../resources/icons/png/qlcplus.png");

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("Root");
    xmldoc.appendChild(root);

    int bgcolor = 0, bgimage = 0, fgcolor = 0, font = 0, frame = 0;

    QCOMPARE(stub.saveXMLAppearance(&xmldoc, &root), true);
    QDomNode node = root.firstChild();
    QCOMPARE(node.toElement().tagName(), QString("Appearance"));
    node = node.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == "BackgroundColor")
        {
            bgcolor++;
            QCOMPARE(tag.text(), QString("Default"));
        }
        else if (tag.tagName() == "BackgroundImage")
        {
            bgimage++;
            QCOMPARE(tag.text(), QString("../../../resources/icons/png/qlcplus.png"));
        }
        else if (tag.tagName() == "ForegroundColor")
        {
            fgcolor++;
            QCOMPARE(tag.text(), QString("Default"));
        }
        else if (tag.tagName() == "Font")
        {
            font++;
            QCOMPARE(tag.text(), QString("Default"));
        }
        else if (tag.tagName() == "FrameStyle")
        {
            frame++;
            QCOMPARE(tag.text(), QString("None"));
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(tag.tagName()).toUtf8().constData());
        }

        node = node.nextSibling();
    }

    QCOMPARE(bgcolor, 1);
    QCOMPARE(bgimage, 1);
    QCOMPARE(fgcolor, 1);
    QCOMPARE(font, 1);
    QCOMPARE(frame, 1);
}

void VCWidget_Test::saveWindowState()
{
    QWidget w;

    StubWidget stub(&w, m_doc);
    w.show();
    w.resize(QSize(100, 100));
    stub.resize(QSize(30, 40));
    stub.move(QPoint(10, 20));
    stub.show();

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("Root");
    xmldoc.appendChild(root);

    QCOMPARE(stub.saveXMLWindowState(&xmldoc, &root), true);
    QDomElement tag = root.firstChild().toElement();
    QCOMPARE(tag.tagName(), QString("WindowState"));
    QCOMPARE(tag.attribute("X"), QString("10"));
    QCOMPARE(tag.attribute("Y"), QString("20"));
    QCOMPARE(tag.attribute("Width"), QString("30"));
    QCOMPARE(tag.attribute("Height"), QString("40"));
    QCOMPARE(tag.attribute("Visible"), QString("True"));

    root.removeChild(tag);

    w.hide();
    QCOMPARE(stub.saveXMLWindowState(&xmldoc, &root), true);
    tag = root.firstChild().toElement();
    QCOMPARE(tag.tagName(), QString("WindowState"));
    QCOMPARE(tag.attribute("X"), QString("10"));
    QCOMPARE(tag.attribute("Y"), QString("20"));
    QCOMPARE(tag.attribute("Width"), QString("30"));
    QCOMPARE(tag.attribute("Height"), QString("40"));
    QCOMPARE(tag.attribute("Visible"), QString("False"));

    QCOMPARE(stub.saveXMLWindowState(&xmldoc, NULL), false);
    QCOMPARE(stub.saveXMLWindowState(NULL, &root), false);
    QCOMPARE(stub.saveXMLWindowState(NULL, NULL), false);
}

void VCWidget_Test::loadWindowState()
{
    QWidget parent;

    StubWidget stub(&parent, m_doc);

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("WindowState");
    root.setAttribute("X", "20");
    root.setAttribute("Y", "10");
    root.setAttribute("Width", "40");
    root.setAttribute("Height", "30");
    root.setAttribute("Visible", "True");
    xmldoc.appendChild(root);

    int x = 0, y = 0, w = 0, h = 0;
    bool v = false;
    QCOMPARE(stub.loadXMLWindowState(&root, &x, &y, &w, &h, NULL), false);
    QCOMPARE(stub.loadXMLWindowState(&root, &x, &y, &w, NULL, &v), false);
    QCOMPARE(stub.loadXMLWindowState(&root, &x, &y, NULL, &h, &v), false);
    QCOMPARE(stub.loadXMLWindowState(&root, &x, NULL, &w, &h, &v), false);
    QCOMPARE(stub.loadXMLWindowState(&root, NULL, &y, &w, &h, &v), false);
    QCOMPARE(stub.loadXMLWindowState(NULL, &x, &y, &w, &h, &v), false);

    QCOMPARE(stub.loadXMLWindowState(&root, &x, &y, &w, &h, &v), true);
    QCOMPARE(x, 20);
    QCOMPARE(y, 10);
    QCOMPARE(w, 40);
    QCOMPARE(h, 30);
    QCOMPARE(v, true);

    root.setAttribute("Visible", "False");
    QCOMPARE(stub.loadXMLWindowState(&root, &x, &y, &w, &h, &v), true);
    QCOMPARE(x, 20);
    QCOMPARE(y, 10);
    QCOMPARE(w, 40);
    QCOMPARE(h, 30);
    QCOMPARE(v, false);

    root.setTagName("WinduhState");
    QCOMPARE(stub.loadXMLWindowState(&root, &x, &y, &w, &h, &v), false);
}

void VCWidget_Test::resize()
{
    QWidget parent;

    StubWidget stub(&parent, m_doc);
    parent.show();
    stub.show();
    parent.resize(QSize(200, 200));

    VCProperties prop = VirtualConsole::instance()->properties();
    VirtualConsole::instance()->m_properties = prop;

    stub.resize(QSize(25, 25));
    QCOMPARE(stub.size(), QSize(25, 25));

    stub.resize(QSize(26, 26));
    QCOMPARE(stub.size(), QSize(25, 25));

    stub.resize(QSize(31, 30));
    QCOMPARE(stub.size(), QSize(30, 30));

    // Allow resizing beyond parent's area
    stub.resize(QSize(250, 250));
    QCOMPARE(stub.size(), QSize(250, 250));

    VirtualConsole::instance()->m_properties = prop;

    stub.resize(QSize(25, 25));
    QCOMPARE(stub.size(), QSize(25, 25));

    stub.resize(QSize(26, 26));
    QCOMPARE(stub.size(), QSize(25, 25));

    stub.resize(QSize(31, 30));
    QCOMPARE(stub.size(), QSize(30, 30));

    // Allow resizing beyond parent's area
    stub.resize(QSize(251, 252));
    QCOMPARE(stub.size(), QSize(250, 250));
}

void VCWidget_Test::move()
{
    QWidget parent;

    StubWidget stub(&parent, m_doc);
    parent.show();
    stub.show();
    parent.resize(QSize(200, 200));
    stub.resize(QSize(50, 50));

    VCProperties prop = VirtualConsole::instance()->properties();
    VirtualConsole::instance()->m_properties = prop;

    stub.move(QPoint(25, 25));
    QCOMPARE(stub.geometry(), QRect(25, 25, 50, 50));

    stub.move(QPoint(-5, -5));
    QCOMPARE(stub.geometry(), QRect(0, 0, 50, 50));

    stub.move(QPoint(190, 190));
    QCOMPARE(stub.geometry(), QRect(150, 150, 50, 50));

    VirtualConsole::instance()->m_properties = prop;

    stub.move(QPoint(25, 25));
    QCOMPARE(stub.geometry(), QRect(25, 25, 50, 50));

    stub.move(QPoint(26, 26));
    QCOMPARE(stub.geometry(), QRect(25, 25, 50, 50));

    stub.move(QPoint(30, 30));
    QCOMPARE(stub.geometry(), QRect(30, 30, 50, 50));

    stub.move(QPoint(31, 31));
    QCOMPARE(stub.geometry(), QRect(30, 30, 50, 50));
}

void VCWidget_Test::paint()
{
    VirtualConsole* vc = VirtualConsole::instance();
    QVERIFY(vc != NULL);

    // Just try to cover all local branches with this test
    StubWidget* stub = new StubWidget(vc->contents(), m_doc);
    VirtualConsole::instance()->show();
    stub->show();
    QTest::qWait(10);

    stub->setFrameStyle(KVCFrameStyleSunken);
    stub->update();
    QTest::qWait(10);

    stub->setFrameStyle(KVCFrameStyleRaised);
    stub->update();
    QTest::qWait(10);

    vc->setWidgetSelected(stub, true);
    stub->update();
    QTest::qWait(10);

    stub->setAllowResize(false);
    stub->update();
    QTest::qWait(10);

    m_doc->setMode(Doc::Operate);
    stub->update();
    QTest::qWait(10);
}

void VCWidget_Test::mousePress()
{
    QCOMPARE(m_doc->mode(), Doc::Design);

    VirtualConsole* vc = VirtualConsole::instance();
    QVERIFY(vc != NULL);

    vc->show();

    StubWidget* stub = new StubWidget(vc->contents(), m_doc);
    stub->show();
    stub->resize(QSize(20, 20));
    QCOMPARE(stub->pos(), QPoint(0, 0));

    QMouseEvent e(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, 0, 0);

    stub->mousePressEvent(&e);
    QCOMPARE(vc->selectedWidgets().size(), 1);
    QCOMPARE(vc->selectedWidgets()[0], stub);
    QCOMPARE(stub->lastClickPoint(), QPoint(10, 10));
    QTest::qWait(10);

    e = QMouseEvent(QEvent::MouseMove, QPoint(20, 20), Qt::NoButton, Qt::LeftButton, 0);
    stub->mouseMoveEvent(&e);
    QTest::qWait(10);
    QCOMPARE(stub->pos(), QPoint(10, 10));
}

QTEST_MAIN(VCWidget_Test)
