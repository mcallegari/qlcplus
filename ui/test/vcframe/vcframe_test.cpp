/*
  Q Light Controller Plus - Test Unit
  vcframe_test.cpp

  Copyright (C) Heikki Junnila
                Massimo Callegari

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFrame>
#include <QtTest>
#include <QMenu>
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

    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    xmlWriter.writeStartElement("Frame");

    xmlWriter.writeStartElement("WindowState");
    xmlWriter.writeAttribute("Width", "42");
    xmlWriter.writeAttribute("Height", "69");
    xmlWriter.writeAttribute("X", "3");
    xmlWriter.writeAttribute("Y", "4");
    xmlWriter.writeAttribute("Visible", "True");
    xmlWriter.writeEndElement();

    xmlWriter.writeTextElement("AllowChildren", "False");
    xmlWriter.writeTextElement("AllowResize", "False");

    xmlWriter.writeStartElement("Frame");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Label");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Button");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("XYPad");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Slider");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("SoloFrame");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("CueList");
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Foobar");
    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();
    xmlWriter.setDevice(NULL);

    buffer.seek(0);
    QXmlStreamReader xmlReader(&buffer);
    xmlReader.readNextStartElement();

    VCFrame parent(&w, m_doc);
    QVERIFY(parent.loadXML(xmlReader) == true);
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

    buffer.close();
    QByteArray bData = buffer.data();
    bData.replace("<Frame", "<Farme");
    buffer.setData(bData);
    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    buffer.seek(0);
    xmlReader.setDevice(&buffer);
    xmlReader.readNextStartElement();

    QVERIFY(parent.loadXML(xmlReader) == false);
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

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);
    QXmlStreamWriter xmlWriter(&buffer);

    QVERIFY(parent.saveXML(&xmlWriter) == true);

    xmlWriter.setDevice(NULL);
    buffer.close();

    buffer.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xmlReader(&buffer);

    QVERIFY(xmlReader.readNextStartElement() == true);
    QCOMPARE(xmlReader.name().toString(), QString("Frame"));

    int appearance = 0, windowstate = 0, frame = 0, allowChildren = 0, allowResize = 0;
    int collapsed = 0, showheader = 0, disabled = 0, enableInput = 0, showEnableButton = 0;

    //qDebug() << "Buffer:" << buffer.data();
    // Parent
    while (xmlReader.readNextStartElement())
    {
        if (xmlReader.name() == QString("Appearance"))
        {
            appearance++;
            xmlReader.skipCurrentElement();
        }
        else if (xmlReader.name() == QString("WindowState"))
        {
            windowstate++;
            xmlReader.skipCurrentElement();
        }
        else if (xmlReader.name() == QString("Frame"))
        {
            frame++;

            QCOMPARE(appearance, 1);
            QCOMPARE(windowstate, 0);
            QCOMPARE(collapsed, 0);
            QCOMPARE(frame, 1);

            // Child
            while (xmlReader.readNextStartElement())
            {
                if (xmlReader.name() == QString("Appearance"))
                {
                    appearance++;
                    xmlReader.skipCurrentElement();
                }
                else if (xmlReader.name() == QString("WindowState"))
                {
                    windowstate++;
                    xmlReader.skipCurrentElement();
                }
                else if (xmlReader.name() == QString("AllowChildren"))
                {
                    allowChildren++;
                    QCOMPARE(xmlReader.readElementText(), QString("False"));
                }
                else if (xmlReader.name() == QString("AllowResize"))
                {
                    allowResize++;
                    QCOMPARE(xmlReader.readElementText(), QString("False"));
                }
                else if (xmlReader.name() == QString("Collapsed"))
                {
                    collapsed++;
                    QCOMPARE(xmlReader.readElementText(), QString("False"));
                }
                else if (xmlReader.name() == QString("ShowHeader"))
                {
                    showheader++;
                    QCOMPARE(xmlReader.readElementText(), QString("True"));
                }
                else if (xmlReader.name() == QString("ShowEnableButton"))
                {
                    showEnableButton++;
                    xmlReader.skipCurrentElement();
                }
                else if (xmlReader.name() == QString("Disabled"))
                {
                    disabled++;
                    QCOMPARE(xmlReader.readElementText(), QString("False"));
                }
                else if (xmlReader.name() == QString("Enable"))
                {
                    enableInput++;
                    xmlReader.skipCurrentElement();
                }
                else if (xmlReader.name() == QString("Frame"))
                {
                    frame++;

                    QCOMPARE(appearance, 2);
                    QCOMPARE(windowstate, 1);
                    QCOMPARE(allowChildren, 1);
                    QCOMPARE(allowResize, 1);
                    QCOMPARE(collapsed, 1);
                    QCOMPARE(showheader, 1);
                    QCOMPARE(disabled, 1);
                    QCOMPARE(frame, 2);

                    // Grandchild
                    while (xmlReader.readNextStartElement())
                    {
                        if (xmlReader.name() == QString("Appearance"))
                        {
                            appearance++;
                            xmlReader.skipCurrentElement();
                        }
                        else if (xmlReader.name() == QString("WindowState"))
                        {
                            windowstate++;
                            xmlReader.skipCurrentElement();
                        }
                        else if (xmlReader.name() == QString("AllowChildren"))
                        {
                            allowChildren++;
                            QCOMPARE(xmlReader.readElementText(), QString("True"));
                        }
                        else if (xmlReader.name() == QString("AllowResize"))
                        {
                            allowResize++;
                            QCOMPARE(xmlReader.readElementText(), QString("True"));
                        }
                        else if (xmlReader.name() == QString("Collapsed"))
                        {
                            collapsed++;
                            QCOMPARE(xmlReader.readElementText(), QString("False"));
                        }
                        else if (xmlReader.name() == QString("ShowHeader"))
                        {
                            showheader++;
                            QCOMPARE(xmlReader.readElementText(), QString("True"));
                        }
                        else if (xmlReader.name() == QString("Disabled"))
                        {
                            disabled++;
                            QCOMPARE(xmlReader.readElementText(), QString("False"));
                        }
                        else if (xmlReader.name() == QString("Enable"))
                        {
                            enableInput++;
                            xmlReader.skipCurrentElement();
                        }
                        else if (xmlReader.name() == QString("ShowEnableButton"))
                        {
                            showEnableButton++;
                            xmlReader.skipCurrentElement();
                        }
                        else
                        {
                            QFAIL(QString("Unexpected tag: %1").arg(xmlReader.name().toString()).toUtf8().constData());
                        }
                    }
                }
            }
        }
        else if (xmlReader.name() == QString("Collapsed"))
        {
            collapsed++;
            xmlReader.skipCurrentElement();
        }
        else if (xmlReader.name() == QString("Disabled"))
        {
            disabled++;
            QCOMPARE(xmlReader.readElementText(), QString("False"));
        }
        else if (xmlReader.name() == QString("Enable"))
        {
            enableInput++;
            xmlReader.skipCurrentElement();
        }
        else if (xmlReader.name() == QString("ShowHeader"))
        {
            showheader++;
            xmlReader.skipCurrentElement();
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(xmlReader.name().toString()).toUtf8().constData());
        }
    }


    QCOMPARE(appearance, 3);
    QCOMPARE(windowstate, 2);
    QCOMPARE(collapsed, 2);
    QCOMPARE(showheader, 2);
    QCOMPARE(disabled, 2);
    QCOMPARE(enableInput, 0);
    QCOMPARE(showEnableButton, 2);
    QCOMPARE(frame, 2);
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

    QMouseEvent ev(QEvent::MouseButtonPress, QPoint(0, 0), QPoint(0, 0), QPoint(0, 0),
                   Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
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
    QMouseEvent ev(QEvent::MouseButtonPress, QPoint(0, 0), QPoint(0, 0), QPoint(0, 0),
                   Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

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
