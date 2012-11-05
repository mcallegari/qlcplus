/*
  Q Light Controller
  efxpreviewarea_test.cpp

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

#include <QtTest>
#include <QtCore>
#include <QtGui>

#include "efxpreviewarea_test.h"
#define private public
#include "efxpreviewarea.h"
#undef private

void EFXPreviewArea_Test::initial()
{
    EFXPreviewArea area(NULL);
    QCOMPARE(area.m_iter, 0);
    QCOMPARE(area.m_timer.parent(), &area);
    QCOMPARE(area.m_timer.isActive(), false);
}

void EFXPreviewArea_Test::setPoints()
{
    EFXPreviewArea area(NULL);
    area.show();

    QPolygon poly(5);
    poly.setPoint(0, QPoint(0, 0));
    poly.setPoint(1, QPoint(10, 10));
    poly.setPoint(2, QPoint(20, 20));
    poly.setPoint(3, QPoint(128, 128));
    poly.setPoint(4, QPoint(255, 255));

    area.setPoints(poly);

    QSize size = QSize(200, 200);
    area.resize(size);
    QCOMPARE(area.m_original, poly);
    QCOMPARE(area.m_points[0], QPoint(0, 0));
    QCOMPARE(area.m_points[1], QPoint(7, 7));
    QCOMPARE(area.m_points[2], QPoint(15, 15));
    QCOMPARE(area.m_points[3], QPoint(100, 100));
    QCOMPARE(area.m_points[4], QPoint(200, 200));
}

void EFXPreviewArea_Test::draw()
{
    EFXPreviewArea area(NULL);
    area.show();

    QPolygon poly(5);
    poly.setPoint(0, QPoint(0, 0));
    poly.setPoint(1, QPoint(10, 10));
    poly.setPoint(2, QPoint(20, 20));
    poly.setPoint(3, QPoint(128, 128));
    poly.setPoint(4, QPoint(255, 255));
    area.setPoints(poly);

    area.draw();
    QCOMPARE(area.m_iter, 0);
    QCOMPARE(area.m_timer.isActive(), true);
    QCOMPARE(area.m_timer.interval(), 20);

    QTest::qWait(300);
    QVERIFY(area.m_iter >= 5);
    QCOMPARE(area.m_timer.isActive(), false);
}

QTEST_MAIN(EFXPreviewArea_Test)
