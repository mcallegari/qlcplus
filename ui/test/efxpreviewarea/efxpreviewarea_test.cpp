/*
  Q Light Controller
  efxpreviewarea_test.cpp

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

    area.setPolygon(poly);

    QSize size = QSize(200, 200);
    area.resize(size);
    QCOMPARE(area.m_original, poly);
    QCOMPARE(area.m_scaled[0], QPoint(0, 0));
    QCOMPARE(area.m_scaled[1], QPoint(7, 7));
    QCOMPARE(area.m_scaled[2], QPoint(15, 15));
    QCOMPARE(area.m_scaled[3], QPoint(100, 100));
    QCOMPARE(area.m_scaled[4], QPoint(200, 200));
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
    area.setPolygon(poly);

    area.draw();
    QCOMPARE(area.m_iter, 0);
    QCOMPARE(area.m_timer.isActive(), true);
    QCOMPARE(area.m_timer.interval(), 20);

    //QTest::qWait(300);
    //QVERIFY(area.m_iter >= 5);
    //QCOMPARE(area.m_timer.isActive(), false);
}

QTEST_MAIN(EFXPreviewArea_Test)
