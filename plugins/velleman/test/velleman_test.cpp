/*
  Q Light Controller
  velleman_test.cpp

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

#define private public
#include "velleman_test.h"
#include "velleman.h"
#undef private

/****************************************************************************
 * Velleman mock
 ****************************************************************************/

extern int _StartDeviceCalled;
extern int _StopDeviceCalled;
extern int _ChannelCount;
extern int* _SetAllData;

/****************************************************************************
 * Velleman tests
 ****************************************************************************/

void Velleman_Test::initial()
{
    Velleman vo;
    vo.init();
    QVERIFY(vo.m_currentlyOpen == false);
    QVERIFY(vo.m_values != NULL);
    QCOMPARE(vo.name(), QString("Velleman"));
    QCOMPARE(vo.outputs(), QStringList() << "1: Velleman Device");
    QVERIFY(vo.canConfigure() == false);
    vo.configure(); // Merely a crash test
}

void Velleman_Test::openClose()
{
    Velleman vo;
    vo.init();

    vo.openOutput(3);
    QVERIFY(vo.m_currentlyOpen == false);
    vo.openOutput(2);
    QVERIFY(vo.m_currentlyOpen == false);
    vo.openOutput(1);
    QVERIFY(vo.m_currentlyOpen == false);

    vo.openOutput(0);
    QVERIFY(vo.m_currentlyOpen == true);
    QCOMPARE(_StartDeviceCalled, 1);
    QCOMPARE(_StopDeviceCalled, 0);

    vo.openOutput(0);
    QVERIFY(vo.m_currentlyOpen == true);
    QCOMPARE(_StartDeviceCalled, 1);
    QCOMPARE(_StopDeviceCalled, 0);

    vo.closeOutput(3);
    QVERIFY(vo.m_currentlyOpen == true);
    vo.closeOutput(2);
    QVERIFY(vo.m_currentlyOpen == true);
    vo.closeOutput(1);
    QVERIFY(vo.m_currentlyOpen == true);

    vo.closeOutput(0);
    QVERIFY(vo.m_currentlyOpen == false);
    QCOMPARE(_StartDeviceCalled, 1);
    QCOMPARE(_StopDeviceCalled, 1);

    vo.closeOutput(0);
    QVERIFY(vo.m_currentlyOpen == false);
    QCOMPARE(_StartDeviceCalled, 1);
    QCOMPARE(_StopDeviceCalled, 1);
}

void Velleman_Test::outputInfo()
{
    Velleman vo;
    vo.init();

// is this seriously needed ??
/*
    QString info;
    info = vo.outputInfo(42);
    QVERIFY(info.startsWith(QString("<HTML><HEAD><TITLE>%1</TITLE></HEAD><BODY>").arg(vo.name())));
    QVERIFY(info.contains(tr("This plugin provides DMX output support")) == false);
    QVERIFY(info.contains("<H3>") == false);
    QVERIFY(info.endsWith("</BODY></HTML>"));

    info = vo.outputInfo(QLCIOPlugin::invalidLine());
    QVERIFY(info.startsWith(QString("<HTML><HEAD><TITLE>%1</TITLE></HEAD><BODY>").arg(vo.name())));
    QVERIFY(info.contains(tr("This plugin provides DMX output support")));
    QVERIFY(info.endsWith("</BODY></HTML>"));

    info = vo.outputInfo(0);
    QVERIFY(info.startsWith(QString("<HTML><HEAD><TITLE>%1</TITLE></HEAD><BODY>").arg(vo.name())));
    QVERIFY(info.contains(vo.outputs()[0]));
    QVERIFY(info.endsWith("</BODY></HTML>"));
*/
}

void Velleman_Test::writeUniverse()
{
    Velleman vo;
    vo.init();

    _StartDeviceCalled = 0;
    _StopDeviceCalled = 0;

    QByteArray data(512, (char) 0);
    data[1] = 63;
    data[15] = 112;
    data[127] = 42;
    data[511] = 96;

    vo.writeUniverse(0, data);
    QVERIFY(_ChannelCount == 0);
    QVERIFY(_SetAllData == NULL);

    vo.openOutput(0);
    vo.writeUniverse(1, data);
    QVERIFY(_ChannelCount == 0);
    QVERIFY(_SetAllData == NULL);

    vo.writeUniverse(0, data);
    QVERIFY(_ChannelCount == data.size());
    QVERIFY(_SetAllData != NULL);
    QCOMPARE(_SetAllData[0], 0);
    QCOMPARE(_SetAllData[1], 63);
    QCOMPARE(_SetAllData[2], 0);
    QCOMPARE(_SetAllData[14], 0);
    QCOMPARE(_SetAllData[15],112);
    QCOMPARE(_SetAllData[16], 0);
    QCOMPARE(_SetAllData[126], 0);
    QCOMPARE(_SetAllData[127], 42);
    QCOMPARE(_SetAllData[128], 0);
    QCOMPARE(_SetAllData[510], 0);
    QCOMPARE(_SetAllData[511], 96);
}

QTEST_MAIN(Velleman_Test)
