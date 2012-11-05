/*
  Q Light Controller
  vcxypadarea_test.cpp

  Copyright (C) Heikki Junnila

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

#include <QFrame>
#include <QtTest>
#include <QtXml>

#define protected public
#define private public
#include "vcxypadarea.h"
#undef private
#undef protected

#include "vcxypadarea_test.h"

void VCXYPadArea_Test::initial()
{
    VCXYPadArea area(NULL);
    QCOMPARE(area.m_mode, Doc::Design);
    QCOMPARE(area.m_changed, false);
    QVERIFY(area.m_pixmap.isNull() == false);
    QCOMPARE(area.frameStyle(), QFrame::Sunken | QFrame::Panel);
    QCOMPARE(area.windowTitle(), QString("XY Pad"));
    QCOMPARE(area.isEnabled(), false);
}

void VCXYPadArea_Test::mode()
{
    VCXYPadArea area(NULL);

    area.setMode(Doc::Operate);
    QCOMPARE(area.isEnabled(), true);
    QCOMPARE(area.m_mode, Doc::Operate);

    area.setMode(Doc::Design);
    QCOMPARE(area.isEnabled(), false);
    QCOMPARE(area.m_mode, Doc::Design);

    area.setMode(Doc::Operate);
    QCOMPARE(area.isEnabled(), true);
    QCOMPARE(area.m_mode, Doc::Operate);

    area.setMode(Doc::Operate);
    QCOMPARE(area.isEnabled(), true);
    QCOMPARE(area.m_mode, Doc::Operate);
}

void VCXYPadArea_Test::position()
{
    VCXYPadArea area(NULL);
    area.resize(QSize(100, 100));

    QCOMPARE(area.hasPositionChanged(), false);
    QCOMPARE(area.position(), QPoint(0, 0));
    QCOMPARE(area.hasPositionChanged(), false);

    area.setPosition(QPoint(50, 50));
    QCOMPARE(area.hasPositionChanged(), true);
    QCOMPARE(area.m_pos, QPoint(50, 50));
    QCOMPARE(area.m_changed, true);
    QCOMPARE(area.position(), QPoint(50, 50));
    QCOMPARE(area.m_changed, false);
    QCOMPARE(area.hasPositionChanged(), false);

    area.setPosition(QPoint(150, 150));
    QCOMPARE(area.hasPositionChanged(), true);
    QCOMPARE(area.position(), QPoint(150, 150));
}

void VCXYPadArea_Test::paint()
{
    VCXYPadArea area(NULL);
    area.resize(100, 100);
    area.show();
    QTest::qWait(10);
    area.setPosition(QPoint(area.width() / 2, area.height() / 2));
    area.update();
    QTest::qWait(10);
}

void VCXYPadArea_Test::mouseEvents()
{
    VCXYPadArea area(NULL);
    area.resize(QSize(100, 100));

    QMouseEvent e(QEvent::MouseButtonPress, QPoint(20, 30), Qt::LeftButton, 0, 0);
    area.mousePressEvent(&e);
    QCOMPARE(area.m_pos, QPoint(0, 0));
    QVERIFY(area.cursor().shape() != Qt::CrossCursor);

    area.setMode(Doc::Operate);
    area.mousePressEvent(&e);
    QCOMPARE(area.m_pos, QPoint(20, 30));
    QCOMPARE(area.cursor().shape(), Qt::CrossCursor);

    QMouseEvent e2(QEvent::MouseButtonPress, QPoint(120, 130), Qt::LeftButton, 0, 0);
    area.mouseMoveEvent(&e2);
    QCOMPARE(area.m_pos, QPoint(100, 100));
    QCOMPARE(area.cursor().shape(), Qt::CrossCursor);

    area.mouseReleaseEvent(&e);
    QVERIFY(area.cursor().shape() != Qt::CrossCursor);
}

QTEST_MAIN(VCXYPadArea_Test)
