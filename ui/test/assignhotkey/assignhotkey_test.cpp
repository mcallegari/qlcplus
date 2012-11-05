/*
  Q Light Controller
  assignhotkey_test.cpp

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

    ev = QKeyEvent(QEvent::KeyPress, Qt::Key_Control, Qt::ControlModifier);
    ahk.keyPressEvent(&ev);
    QCOMPARE(ahk.keySequence(), QKeySequence());
    QCOMPARE(ahk.result(), (int) QDialog::Rejected);

    ev = QKeyEvent(QEvent::KeyPress, Qt::Key_Alt, Qt::AltModifier);
    ahk.keyPressEvent(&ev);
    QCOMPARE(ahk.keySequence(), QKeySequence());
    QCOMPARE(ahk.result(), (int) QDialog::Rejected);

    ev = QKeyEvent(QEvent::KeyPress, Qt::Key_Meta, Qt::MetaModifier);
    ahk.keyPressEvent(&ev);
    QCOMPARE(ahk.keySequence(), QKeySequence());
    QCOMPARE(ahk.result(), (int) QDialog::Rejected);

    // Reset autoclose
    ahk.m_autoCloseCheckBox->setChecked(autoclose);
}

QTEST_MAIN(AssignHotKey_Test)
