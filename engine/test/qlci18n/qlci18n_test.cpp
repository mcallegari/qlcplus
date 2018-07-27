/*
  Q Light Controller - Unit test
  qlci18n_test.cpp

  Copyright (c) Heikki Junnila

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
    // so that
    QString dummy = tr("dummy");

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
    QCOMPARE(QLCi18n::loadTranslation("qlci18n"), true);
}

QTEST_MAIN(QLCi18n_Test)
