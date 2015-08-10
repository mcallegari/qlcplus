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

    QPolygonF poly;
    poly << QPointF(0, 0);
    poly << QPointF(10, 10);
    poly << QPointF(20, 20);
    poly << QPointF(128, 128);
    poly << QPointF(255, 255);

    area.setPolygon(poly);

    QSize size = QSize(200, 200);
    area.resize(size);
    QCOMPARE(area.m_original, poly);
    QCOMPARE(area.m_scaled[0].toPoint(), QPoint(0, 0));
    QCOMPARE(area.m_scaled[1].toPoint(), QPoint(8, 8));
    QCOMPARE(area.m_scaled[2].toPoint(), QPoint(16, 16));
    QCOMPARE(area.m_scaled[3].toPoint(), QPoint(100, 100));
    QCOMPARE(area.m_scaled[4].toPoint(), QPoint(200, 200));
}

void EFXPreviewArea_Test::draw()
{
    EFXPreviewArea area(NULL);
    area.show();

    QPolygonF poly;
    poly << QPointF(0, 0);
    poly << QPointF(10, 10);
    poly << QPointF(20, 20);
    poly << QPointF(128, 128);
    poly << QPointF(255, 255);
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
