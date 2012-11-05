/*
  Q Light Controller - Unit test
  qlci18n_test.cpp

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

#include "qlci18n.h"
#include "qlci18n_test.h"

void QLCi18n_Test::defaultLocale()
{
    QCOMPARE(QLCi18n::defaultLocale(), QString());
    QLCi18n::setDefaultLocale("fi_FI");
    QCOMPARE(QLCi18n::defaultLocale(), QString("fi_FI"));
}

void QLCi18n_Test::translationFilePath()
{
    QCOMPARE(QLCi18n::translationFilePath(), QString());
    QLCi18n::setTranslationFilePath("/foo/bar");
    QCOMPARE(QLCi18n::translationFilePath(), QString("/foo/bar"));
}

void QLCi18n_Test::loadTranslation()
{
    QLCi18n::setDefaultLocale(QString());
    QLCi18n::setTranslationFilePath(".");

    // Since QLocale::system() is different for people using such locales that
    // don't have QLC translation yet, we just have to accept a failure on those
    // cases.
    QDir dir(QLCi18n::translationFilePath());
    QStringList entries(dir.entryList(QStringList() << QString("*.qm"), QDir::Files));
    QString qm(QString("%1_%2.qm").arg("qlci18n").arg(QLocale::system().name()));
    if (entries.contains(qm) == false)
        QEXPECT_FAIL("", "No translation for this locale. Fail is OK.", Continue);
    QCOMPARE(QLCi18n::loadTranslation("qlci18n"), true);

    // qlci18n_fi_FI.qm SHOULD be there.
    QLCi18n::setDefaultLocale("fi_FI");
    QVERIFY(QLCi18n::loadTranslation("qlci18n") == true);
}

QTEST_MAIN(QLCi18n_Test)
