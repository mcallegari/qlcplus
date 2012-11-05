/*
  Q Light Controller - Unit test
  qlcpoint_test.cpp

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

#include <QtTest>

#include "qlcpoint_test.h"
#include "qlcpoint.h"

void QLCPoint_Test::initial()
{
    QLCPoint pt;
    QCOMPARE(pt.isNull(), true);

    pt = QLCPoint(1, 2);
    QCOMPARE(pt.isNull(), false);
    QCOMPARE(pt.x(), 1);
    QCOMPARE(pt.y(), 2);
}

void QLCPoint_Test::equals()
{
    QLCPoint pt1(1, 1);
    QLCPoint pt2(2, 2);
    QCOMPARE(pt1, pt1);
    QCOMPARE(pt2, pt2);
    QVERIFY(pt1 != pt2);
}

void QLCPoint_Test::hash()
{
    QLCPoint pt(10, 10);
    qDebug() << qHash(pt);
}

QTEST_APPLESS_MAIN(QLCPoint_Test)
