/*
  Q Light Controller - Unit test
  vcwidgetproperties_test.cpp

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QWidget>
#include <QtTest>

#include "vcwidgetproperties_test.h"
#define protected public
#include "vcwidgetproperties.h"
#undef private

void VCWidgetProperties_Test::stateAndVisibility()
{
    VCWidgetProperties p;
    QVERIFY(p.state() == Qt::WindowNoState);
    QVERIFY(p.visible() == false);

    QWidget w(NULL);
    p.store(&w);
    QVERIFY(p.state() == Qt::WindowNoState);
    QVERIFY(p.visible() == false);

    w.showMinimized();
    p.store(&w);
    QVERIFY(p.state() & Qt::WindowMinimized);
    QVERIFY(p.visible() == true);

    w.showMaximized();
    p.store(&w);
    QVERIFY(p.state() & Qt::WindowMaximized);
    QVERIFY(p.visible() == true);

    w.showFullScreen();
    p.store(&w);
    QVERIFY(p.state() & Qt::WindowFullScreen);
    QVERIFY(p.visible() == true);

    w.hide();
    p.store(&w);
    QVERIFY(p.state() & Qt::WindowFullScreen);
    QVERIFY(p.visible() == false);
}

void VCWidgetProperties_Test::xy()
{
    VCWidgetProperties p;

    QWidget w(NULL);
    p.store(&w);

    QVERIFY(p.x() == 0);
    QVERIFY(p.y() == 0);

    w.move(50, 10);
    p.store(&w);

    QVERIFY(p.x() == 50);
    QVERIFY(p.y() == 10);
}

void VCWidgetProperties_Test::wh()
{
    VCWidgetProperties p;

    QWidget w(NULL);
    p.store(&w);

    QVERIFY(p.x() == 0);
    QVERIFY(p.y() == 0);

    w.resize(20, 30);
    p.store(&w);

    QVERIFY(p.width() == 20);
    QVERIFY(p.height() == 30);
}

void VCWidgetProperties_Test::load()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("WidgetProperties");
    doc.appendChild(root);

    QDomElement x = doc.createElement("X");
    QDomText xText = doc.createTextNode("50");
    x.appendChild(xText);
    root.appendChild(x);

    QDomElement y = doc.createElement("Y");
    QDomText yText = doc.createTextNode("70");
    y.appendChild(yText);
    root.appendChild(y);

    QDomElement w = doc.createElement("Width");
    QDomText wText = doc.createTextNode("40");
    w.appendChild(wText);
    root.appendChild(w);

    QDomElement h = doc.createElement("Height");
    QDomText hText = doc.createTextNode("60");
    h.appendChild(hText);
    root.appendChild(h);

    QDomElement v = doc.createElement("Visible");
    QDomText vText = doc.createTextNode("1");
    v.appendChild(vText);
    root.appendChild(v);

    QDomElement s = doc.createElement("State");
    QDomText sText = doc.createTextNode(QString("%1").arg(Qt::WindowMinimized));
    s.appendChild(sText);
    root.appendChild(s);

    QDomElement foo = doc.createElement("Foobar");
    QDomText fooText = doc.createTextNode("1");
    foo.appendChild(fooText);
    root.appendChild(foo);

    VCWidgetProperties p;
    p.loadXML(root);
    QVERIFY(p.x() == 50);
    QVERIFY(p.y() == 70);
    QVERIFY(p.width() == 40);
    QVERIFY(p.height() == 60);
    QVERIFY(p.state() == Qt::WindowMinimized);
    QVERIFY(p.visible() == true);
}

void VCWidgetProperties_Test::loadWrongRoot()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("WidgetPropertiez");
    doc.appendChild(root);

    VCWidgetProperties p;
    QVERIFY(p.loadXML(root) == false);
    QVERIFY(p.x() == 100);
    QVERIFY(p.y() == 100);
    QVERIFY(p.width() == 0);
    QVERIFY(p.height() == 0);
    QVERIFY(p.state() == Qt::WindowNoState);
    QVERIFY(p.visible() == false);
}

void VCWidgetProperties_Test::save()
{
    VCWidgetProperties p;
    p.m_state = Qt::WindowMinimized;
    p.m_visible = true;
    p.m_x = 10;
    p.m_y = 20;
    p.m_width = 30;
    p.m_height = 40;

    QDomDocument doc;
    QDomElement root = doc.createElement("Root");
    doc.appendChild(root);

    QVERIFY(p.saveXML(&doc, &root) == true);
    QVERIFY(root.firstChild().toElement().tagName() == "WidgetProperties");

    bool s = false, v = false, x = false, y = false, w = false, h = false;
    QDomNode node = root.firstChild().firstChild();
    while (node.isNull() == false)
    {
        QDomElement e = node.toElement();
        if (e.tagName() == "State")
        {
            s = true;
            QCOMPARE(e.text(), QString::number(p.m_state));
        }
        else if (e.tagName() == "Visible")
        {
            v = true;
            QCOMPARE(e.text(), QString::number(p.m_visible));
        }
        else if (e.tagName() == "X")
        {
            x = true;
            QCOMPARE(e.text(), QString::number(p.m_x));
        }
        else if (e.tagName() == "Y")
        {
            y = true;
            QCOMPARE(e.text(), QString::number(p.m_y));
        }
        else if (e.tagName() == "Width")
        {
            w = true;
            QCOMPARE(e.text(), QString::number(p.m_width));
        }
        else if (e.tagName() == "Height")
        {
            h = true;
            QCOMPARE(e.text(), QString::number(p.m_height));
        }
        else
        {
            QFAIL(QString("Unexpected widget property tag: %1").arg(e.tagName()).toUtf8().constData());
        }

        node = node.nextSibling();
    }

    QVERIFY(s && v && x && y && w && h);
}

void VCWidgetProperties_Test::copy()
{
    VCWidgetProperties p;
    p.m_state = Qt::WindowMinimized;
    p.m_visible = true;
    p.m_x = 10;
    p.m_y = 20;
    p.m_width = 30;
    p.m_height = 40;

    VCWidgetProperties p2(p);
    QCOMPARE(p2.state(), p.state());
    QCOMPARE(p2.visible(), p.visible());
    QCOMPARE(p2.x(), p.x());
    QCOMPARE(p2.y(), p.y());
    QCOMPARE(p2.width(), p.width());
    QCOMPARE(p2.height(), p.height());
}

QTEST_MAIN(VCWidgetProperties_Test)
