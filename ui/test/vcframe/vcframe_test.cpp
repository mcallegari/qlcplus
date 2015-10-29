/*
  Q Light Controller
  vcframe_test.cpp

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
#include <QMenu>
#include <QtXml>
#include <QSet>

#define protected public

#include "qlcfixturedefcache.h"
#include "virtualconsole.h"
#include "vcframe_test.h"
#include "mastertimer.h"
#include "vcbutton.h"
#include "vcwidget.h"
#include "vcframe.h"
#include "vcframe.h"
#include "doc.h"

#undef protected

void VCFrame_Test::initTestCase()
{
    m_doc = NULL;
}

void VCFrame_Test::init()
{
    m_doc = new Doc(this);
    new VirtualConsole(NULL, m_doc);
}

void VCFrame_Test::cleanup()
{
    delete VirtualConsole::instance();
    delete m_doc;
}

void VCFrame_Test::initial()
{
    QWidget w;

    VCFrame frame(&w, m_doc);
    QCOMPARE(frame.objectName(), QString("VCFrame"));
    QCOMPARE(frame.frameStyle(), QFrame::Panel | QFrame::Sunken);
}

void VCFrame_Test::copy()
{
    QWidget w;

    VCFrame parent(&w, m_doc);
    VCFrame frame(&parent, m_doc);
    VCButton* btn = new VCButton(&frame, m_doc);
    btn->setCaption("Foobar");
    VCWidget* frame2 = frame.createCopy(&parent);
    QVERIFY(frame2 != NULL && frame2 != &frame);
    QCOMPARE(frame2->objectName(), QString("VCFrame"));
    QCOMPARE(frame2->parentWidget(), &parent);

    // Also children should get copied
    QList <VCButton*> list = frame2->findChildren<VCButton*>();
    QCOMPARE(list.size(), 1);
    QCOMPARE(list[0]->caption(), QString("Foobar"));

    QVERIFY(frame.copyFrom(NULL) == false);
}

void VCFrame_Test::loadXML()
{
    QWidget w;

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("Frame");
    xmldoc.appendChild(root);

    QDomElement wstate = xmldoc.createElement("WindowState");
    wstate.setAttribute("Width", "42");
    wstate.setAttribute("Height", "69");
    wstate.setAttribute("X", "3");
    wstate.setAttribute("Y", "4");
    wstate.setAttribute("Visible", "True");
    root.appendChild(wstate);

    QDomElement allowChildren = xmldoc.createElement("AllowChildren");
    QDomText allowChildrenText = xmldoc.createTextNode("False");
    allowChildren.appendChild(allowChildrenText);
    root.appendChild(allowChildren);

    QDomElement allowResize = xmldoc.createElement("AllowResize");
    QDomText allowResizeText = xmldoc.createTextNode("False");
    allowResize.appendChild(allowResizeText);
    root.appendChild(allowResize);

    QDomElement frame = xmldoc.createElement("Frame");
    root.appendChild(frame);

    QDomElement label = xmldoc.createElement("Label");
    root.appendChild(label);

    QDomElement button = xmldoc.createElement("Button");
    root.appendChild(button);

    QDomElement xypad = xmldoc.createElement("XYPad");
    root.appendChild(xypad);

    QDomElement slider = xmldoc.createElement("Slider");
    root.appendChild(slider);

    QDomElement soloframe = xmldoc.createElement("SoloFrame");
    root.appendChild(soloframe);

    QDomElement cuelist = xmldoc.createElement("CueList");
    root.appendChild(cuelist);

    QDomElement foobar = xmldoc.createElement("Foobar");
    root.appendChild(foobar);

    VCFrame parent(&w, m_doc);
    QVERIFY(parent.loadXML(&root) == true);
    parent.postLoad();
    QCOMPARE(parent.geometry().width(), 42);
    QCOMPARE(parent.geometry().height(), 69);
    QCOMPARE(parent.geometry().x(), 3);
    QCOMPARE(parent.geometry().y(), 4);
    QVERIFY(parent.allowChildren() == false);
    QVERIFY(parent.allowResize() == false);

    QSet <QString> childSet;
    QCOMPARE(parent.children().size(), 7);
    foreach (QObject* child, parent.children())
        childSet << child->metaObject()->className();
    QVERIFY(childSet.contains("VCFrame"));
    QVERIFY(childSet.contains("VCLabel"));
    QVERIFY(childSet.contains("VCButton"));
    QVERIFY(childSet.contains("VCXYPad"));
    QVERIFY(childSet.contains("VCSlider"));
    QVERIFY(childSet.contains("VCSoloFrame"));
    QVERIFY(childSet.contains("VCCueList"));

    root.setTagName("Farme");
    QVERIFY(parent.loadXML(&root) == false);
}

void VCFrame_Test::saveXML()
{
    QWidget w;

    VCFrame parent(&w, m_doc);
    parent.setCaption("Parent");
    VCFrame child(&parent, m_doc);
    child.setCaption("Child");
    VCFrame grandChild(&child, m_doc);
    grandChild.setCaption("Grandchild");
    child.setAllowChildren(false);
    child.setAllowResize(false);

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("TestRoot");
    xmldoc.appendChild(root);

    QVERIFY(parent.saveXML(&xmldoc, &root) == true);

    QDomNode node = root.firstChild();
    QVERIFY(node.nextSibling().isNull() == true);
    QCOMPARE(node.toElement().tagName(), QString("Frame"));
    QVERIFY(node.firstChild().isNull() == false);

    QDomNode subFrame;
    int appearance = 0, windowstate = 0, frame = 0, allowChildren = 0, allowResize = 0;
    int collapsed = 0, showheader = 0, disabled = 0, enableInput = 0, showEnableButton = 0;

    // Parent
    node = node.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == QString("Appearance"))
        {
            appearance++;
        }
        else if (tag.tagName() == QString("WindowState"))
        {
            windowstate++;
        }
        else if (tag.tagName() == QString("Frame"))
        {
            frame++;
            QVERIFY(subFrame.isNull() == true);
            subFrame = node;
        }
        else if (tag.tagName() == QString("Collapsed"))
        {
            collapsed++;
        }
        else if (tag.tagName() == QString("ShowHeader"))
        {
            showheader++;
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(tag.tagName()).toUtf8().constData());
        }
        node = node.nextSibling();
    }

    QCOMPARE(appearance, 1);
    QCOMPARE(windowstate, 0);
    QCOMPARE(collapsed, 0);
    QCOMPARE(frame, 1);
    QVERIFY(subFrame.isNull() == false);

    // Child
    node = subFrame.firstChild();
    subFrame = QDomNode(); // Don't use .clear() since that would clear the whole tree
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == QString("Appearance"))
        {
            appearance++;
        }
        else if (tag.tagName() == QString("WindowState"))
        {
            windowstate++;
        }
        else if (tag.tagName() == QString("AllowChildren"))
        {
            allowChildren++;
            QCOMPARE(tag.text(), QString("False"));
        }
        else if (tag.tagName() == QString("AllowResize"))
        {
            allowResize++;
            QCOMPARE(tag.text(), QString("False"));
        }
        else if (tag.tagName() == QString("Collapsed"))
        {
            collapsed++;
            QCOMPARE(tag.text(), QString("False"));
        }
        else if (tag.tagName() == QString("ShowHeader"))
        {
            showheader++;
            QCOMPARE(tag.text(), QString("True"));
        }
        else if (tag.tagName() == QString("Disabled"))
        {
            disabled++;
            QCOMPARE(tag.text(), QString("False"));
        }
        else if (tag.tagName() == QString("Frame"))
        {
            frame++;
            QVERIFY(subFrame.isNull() == true);
            subFrame = node;
        }
        node = node.nextSibling();
    }

    QCOMPARE(appearance, 2);
    QCOMPARE(windowstate, 1);
    QCOMPARE(allowChildren, 1);
    QCOMPARE(allowResize, 1);
    QCOMPARE(collapsed, 1);
    QCOMPARE(showheader, 1);
    QCOMPARE(disabled, 1);
    QCOMPARE(frame, 2);
    QVERIFY(subFrame.isNull() == false);

    // Grandchild
    node = subFrame.firstChild();
    subFrame = QDomNode(); // Don't use .clear() since that would clear the whole tree
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == QString("Appearance"))
        {
            appearance++;
        }
        else if (tag.tagName() == QString("WindowState"))
        {
            windowstate++;
        }
        else if (tag.tagName() == QString("AllowChildren"))
        {
            allowChildren++;
            QCOMPARE(tag.text(), QString("True"));
        }
        else if (tag.tagName() == QString("AllowResize"))
        {
            allowResize++;
            QCOMPARE(tag.text(), QString("True"));
        }
        else if (tag.tagName() == QString("Collapsed"))
        {
            collapsed++;
            QCOMPARE(tag.text(), QString("False"));
        }
        else if (tag.tagName() == QString("ShowHeader"))
        {
            showheader++;
            QCOMPARE(tag.text(), QString("True"));
        }
        else if (tag.tagName() == QString("Disabled"))
        {
            disabled++;
            QCOMPARE(tag.text(), QString("False"));
        }
        else if (tag.tagName() == QString("Enable"))
        {
            enableInput++;
        }
        else if (tag.tagName() == QString("ShowEnableButton"))
        {
            showEnableButton++;
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(tag.tagName()).toUtf8().constData());
        }
        node = node.nextSibling();
    }

    QCOMPARE(appearance, 3);
    QCOMPARE(windowstate, 2);
    QCOMPARE(collapsed, 2);
    QCOMPARE(showheader, 2);
    QCOMPARE(disabled, 2);
    QCOMPARE(enableInput, 1);
    QCOMPARE(showEnableButton, 1);
    QCOMPARE(frame, 2);
    QVERIFY(subFrame.isNull() == true);
}

void VCFrame_Test::customMenu()
{
    VCFrame* frame = VirtualConsole::instance()->contents();
    QVERIFY(frame != NULL);

    QMenu menu;
    QMenu* customMenu = frame->customMenu(&menu);
    QVERIFY(customMenu != NULL);
    QCOMPARE(customMenu->title(), tr("Add"));
    delete customMenu;
}

void VCFrame_Test::handleWidgetSelection()
{
    VCFrame* frame = VirtualConsole::instance()->contents();
    QVERIFY(frame->isBottomFrame() == true);

    QMouseEvent ev(QEvent::MouseButtonPress, QPoint(0, 0), Qt::LeftButton,
                   Qt::LeftButton, Qt::NoModifier);
    frame->handleWidgetSelection(&ev);

    // Select bottom frame (->no selection)
    frame->handleWidgetSelection(&ev);
    QCOMPARE(VirtualConsole::instance()->selectedWidgets().size(), 0);

    // Select a child frame
    VCFrame* child = new VCFrame(frame, m_doc);
    QVERIFY(child->isBottomFrame() == false);
    child->handleWidgetSelection(&ev);
    QCOMPARE(VirtualConsole::instance()->selectedWidgets().size(), 1);

    // Again select bottom frame
    frame->handleWidgetSelection(&ev);
    QCOMPARE(VirtualConsole::instance()->selectedWidgets().size(), 0);
}

void VCFrame_Test::mouseMoveEvent()
{
    // Well, there isn't much that can be checked here... Actual moving happens in VCWidget.
    QMouseEvent ev(QEvent::MouseButtonPress, QPoint(0, 0), Qt::LeftButton,
                   Qt::LeftButton, Qt::NoModifier);

    QWidget w;
    VCFrame frame(&w, m_doc);
    QVERIFY(frame.isBottomFrame() == true);
    frame.move(QPoint(0, 0));
    frame.mouseMoveEvent(&ev);
    QCOMPARE(frame.pos(), QPoint(0, 0));

    VCFrame child(&frame, m_doc);
    QVERIFY(child.isBottomFrame() == false);
    child.mouseMoveEvent(&ev);
    QCOMPARE(child.pos(), QPoint(0, 0));
}

QTEST_MAIN(VCFrame_Test)
