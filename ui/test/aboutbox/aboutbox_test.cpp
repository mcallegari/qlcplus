/*
  Q Light Controller
  aboutbox_test.cpp

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

#include "qlcconfig.h"
#include "aboutbox_test.h"
#define private public
#include "aboutbox.h"
#undef protected

#define CONTRIBCOUNT 34

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
    QVERIFY(ab.m_copyrightLabel->text().contains("Copyright &copy; <B>Heikki Junnila, Massimo Callegari</B>"));

    QVERIFY(ab.m_websiteLabel != NULL);
    QVERIFY(ab.m_websiteLabel->text().contains("<A HREF=\"http://www.qlcplus.org/\">http://www.qlcplus.org/</A>"));

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
