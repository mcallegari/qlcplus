/*
  Q Light Controller
  vcxypadarea_test.cpp

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
