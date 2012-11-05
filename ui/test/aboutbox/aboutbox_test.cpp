/*
  Q Light Controller
  aboutbox_test.cpp

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

#include "qlcconfig.h"
#include "aboutbox_test.h"
#define private public
#include "aboutbox.h"
#undef protected

#define CONTRIBCOUNT 11

void AboutBox_Test::initial()
{
    QWidget w;
    AboutBox ab(&w);
    QCOMPARE(ab.parentWidget(), &w);

    QVERIFY(ab.m_titleLabel != NULL);
    QCOMPARE(ab.m_titleLabel->text(), QString(APPNAME));

    QVERIFY(ab.m_versionLabel != NULL);
    QCOMPARE(ab.m_versionLabel->text(), QString(APPVERSION));

    QVERIFY(ab.m_copyrightLabel != NULL);
    QVERIFY(ab.m_copyrightLabel->text().contains("Copyright &copy; <B>Heikki Junnila</B>"));

    QVERIFY(ab.m_websiteLabel != NULL);
    QVERIFY(ab.m_websiteLabel->text().contains("<A HREF=\"http://qlc.sourceforge.net\">http://qlc.sourceforge.net</A>"));

    QVERIFY(ab.m_contributors != NULL);
    QCOMPARE(ab.m_contributors->count(), CONTRIBCOUNT);

    QVERIFY(ab.m_timer != NULL);
    QCOMPARE(ab.m_row, -1);
    QCOMPARE(ab.m_increment, 1);
    QCOMPARE(ab.m_timer->interval(), 500);
    QCOMPARE(ab.m_timer->isActive(), true);
}

void AboutBox_Test::slotTimeout()
{
    AboutBox ab(NULL);
    ab.m_timer->stop();

    QCOMPARE(ab.m_row, -1);
    QCOMPARE(ab.m_increment, 1);

    for (int i = 0; i <= CONTRIBCOUNT; i++)
    {
        ab.slotTimeout();
        QCOMPARE(ab.m_row, i);
        QCOMPARE(ab.m_increment, 1);
    }

    for (int i = CONTRIBCOUNT - 1; i > 0; i--)
    {
        ab.slotTimeout();
        QCOMPARE(ab.m_row, i);
        QCOMPARE(ab.m_increment, -1);
    }
}

void AboutBox_Test::itemClick()
{
    AboutBox ab(NULL);
    QVERIFY(ab.m_timer != NULL);
    ab.slotItemClicked();
    QVERIFY(ab.m_timer == NULL);
    ab.slotItemClicked();
    QVERIFY(ab.m_timer == NULL);
}

QTEST_MAIN(AboutBox_Test)
