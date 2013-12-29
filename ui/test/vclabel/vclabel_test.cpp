/*
  Q Light Controller
  vclabel_test.cpp

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
#define private public

#include "virtualconsole.h"
#include "vcwidget.h"
#include "vcframe.h"
#include "vclabel.h"
#include "qlcfixturedefcache.h"
#include "vclabel_test.h"
#include "mastertimer.h"
#include "doc.h"

#undef private
#undef protected

void VCLabel_Test::initTestCase()
{
    m_doc = NULL;
}

void VCLabel_Test::init()
{
    m_doc = new Doc(this);
    new VirtualConsole(NULL, m_doc);
}

void VCLabel_Test::cleanup()
{
    delete VirtualConsole::instance();
    delete m_doc;
}

void VCLabel_Test::initial()
{
    QWidget w;

    VCLabel label(&w, m_doc);
    QCOMPARE(label.objectName(), QString("VCLabel"));
    QCOMPARE(label.frameStyle(), 0);
    QCOMPARE(label.caption(), tr("Label"));
    QCOMPARE(label.size(), QSize(100, 30));
}

void VCLabel_Test::copy()
{
    QWidget w;

    VCFrame parent(&w, m_doc);
    VCLabel label(&parent, m_doc);
    label.setCaption("Foobar");
    VCLabel* label2 = qobject_cast<VCLabel*> (label.createCopy(&parent));
    QVERIFY(label2 != NULL && label2 != &label);
    QCOMPARE(label2->objectName(), QString("VCLabel"));
    QCOMPARE(label2->parentWidget(), &parent);
    QCOMPARE(label2->caption(), QString("Foobar"));

    QVERIFY(label.copyFrom(NULL) == false);
}

void VCLabel_Test::loadXML()
{
    QWidget w;

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("Label");
    xmldoc.appendChild(root);

    QDomElement wstate = xmldoc.createElement("WindowState");
    wstate.setAttribute("Width", "42");
    wstate.setAttribute("Height", "69");
    wstate.setAttribute("X", "3");
    wstate.setAttribute("Y", "4");
    wstate.setAttribute("Visible", "True");
    root.appendChild(wstate);

    QDomElement appearance = xmldoc.createElement("Appearance");
    QFont f(w.font());
    f.setPointSize(f.pointSize() + 3);
    QDomElement font = xmldoc.createElement("Font");
    QDomText fontText = xmldoc.createTextNode(f.toString());
    font.appendChild(fontText);
    appearance.appendChild(font);
    root.appendChild(appearance);

    QDomElement foobar = xmldoc.createElement("Foobar");
    root.appendChild(foobar);

    VCLabel label(&w, m_doc);
    QVERIFY(label.loadXML(&root) == true);
    QCOMPARE(label.geometry().width(), 42);
    QCOMPARE(label.geometry().height(), 69);
    QCOMPARE(label.geometry().x(), 3);
    QCOMPARE(label.geometry().y(), 4);
    QCOMPARE(label.font(), f);

    root.setTagName("Lable");
    QVERIFY(label.loadXML(&root) == false);
}

void VCLabel_Test::saveXML()
{
    QWidget w;

    VCLabel label(&w, m_doc);
    label.setCaption("Simo Kuassimo");

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("TestRoot");
    xmldoc.appendChild(root);

    QVERIFY(label.saveXML(&xmldoc, &root) == true);

    QDomNode node = root.firstChild();
    QVERIFY(node.nextSibling().isNull() == true);
    QCOMPARE(node.toElement().tagName(), QString("Label"));
    QCOMPARE(node.toElement().attribute("Caption"), QString("Simo Kuassimo"));
    QVERIFY(node.firstChild().isNull() == false);

    int appearance = 0, windowstate = 0;

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
        else
        {
            QFAIL(QString("Unexpected XML tag: %1").arg(tag.tagName()).toUtf8().constData());
        }
        node = node.nextSibling();
    }

    QCOMPARE(appearance, 1);
    QCOMPARE(windowstate, 1);
}

void VCLabel_Test::paintEvent()
{
    QPaintEvent ev(QRect(0, 0, 5, 5));

    // Checking the result of a paint event would have to compare individual pixels, which
    // I'm not gonna do. Just call all branches to try to find any crashes and that's it...
    QWidget w;
    VCLabel label(&w, m_doc);
    label.paintEvent(&ev);

    m_doc->setMode(Doc::Operate);
    label.paintEvent(&ev);
}

QTEST_MAIN(VCLabel_Test)
