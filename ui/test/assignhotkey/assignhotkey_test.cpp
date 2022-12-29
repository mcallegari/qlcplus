/*
  Q Light Controller
  assignhotkey_test.cpp

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

#define protected public
#define private public
#include "assignhotkey.h"
#undef private
#undef protected

#include "assignhotkey_test.h"

void AssignHotKey_Test::initial()
{
    QKeySequence seq(Qt::Key_A | Qt::SHIFT);
    AssignHotKey ahk(NULL, seq);
    QCOMPARE(ahk.keySequence(), seq);
    QCOMPARE(ahk.m_previewEdit->text(), seq.toString(QKeySequence::NativeText));
    QCOMPARE(ahk.m_previewEdit->isReadOnly(), true);
}

void AssignHotKey_Test::keyPressEventAuto()
{
    AssignHotKey ahk(NULL);
    bool autoclose = ahk.m_autoCloseCheckBox->isChecked();

    // Autoclose on
    ahk.m_autoCloseCheckBox->setChecked(true);
    QKeyEvent ev(QEvent::KeyPress, Qt::Key_B, Qt::ShiftModifier);
    QCOMPARE(ahk.result(), (int) QDialog::Rejected);
    ahk.keyPressEvent(&ev);
    QCOMPARE(ahk.result(), (int) QDialog::Accepted);
    QCOMPARE(ahk.keySequence(), QKeySequence(Qt::Key_B | Qt::SHIFT));

    // Reset autoclose
    ahk.m_autoCloseCheckBox->setChecked(autoclose);
}

void AssignHotKey_Test::keyPressEventNoAuto()
{
    AssignHotKey ahk(NULL);
    bool autoclose = ahk.m_autoCloseCheckBox->isChecked();

    // Autoclose off
    ahk.m_autoCloseCheckBox->setChecked(false);
    QKeyEvent ev(QEvent::KeyPress, Qt::Key_B, Qt::ShiftModifier);
    QCOMPARE(ahk.result(), (int) QDialog::Rejected);
    ahk.keyPressEvent(&ev);
    QCOMPARE(ahk.result(), (int) QDialog::Rejected);
    QCOMPARE(ahk.keySequence(), QKeySequence(Qt::Key_B | Qt::SHIFT));

    // Reset autoclose
    ahk.m_autoCloseCheckBox->setChecked(autoclose);
}

void AssignHotKey_Test::keyPressEventOnlyModifiers()
{
    AssignHotKey ahk(NULL);
    bool autoclose = ahk.m_autoCloseCheckBox->isChecked();

    ahk.m_autoCloseCheckBox->setChecked(false);
    QKeyEvent ev(QEvent::KeyPress, Qt::Key_Shift, Qt::ShiftModifier);
    ahk.keyPressEvent(&ev);
    QCOMPARE(ahk.keySequence(), QKeySequence());
    QCOMPARE(ahk.result(), (int) QDialog::Rejected);

    QKeyEvent ev2(QEvent::KeyPress, Qt::Key_Control, Qt::ControlModifier);
    ahk.keyPressEvent(&ev2);
    QCOMPARE(ahk.keySequence(), QKeySequence());
    QCOMPARE(ahk.result(), (int) QDialog::Rejected);

    QKeyEvent ev3(QEvent::KeyPress, Qt::Key_Alt, Qt::AltModifier);
    ahk.keyPressEvent(&ev3);
    QCOMPARE(ahk.keySequence(), QKeySequence());
    QCOMPARE(ahk.result(), (int) QDialog::Rejected);

    QKeyEvent ev4(QEvent::KeyPress, Qt::Key_Meta, Qt::MetaModifier);
    ahk.keyPressEvent(&ev4);
    QCOMPARE(ahk.keySequence(), QKeySequence());
    QCOMPARE(ahk.result(), (int) QDialog::Rejected);

    // Reset autoclose
    ahk.m_autoCloseCheckBox->setChecked(autoclose);
}

QTEST_MAIN(AssignHotKey_Test)
