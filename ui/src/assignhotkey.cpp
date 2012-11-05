/*
  Q Light Controller
  assignhotkey.cpp

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

#include <QKeySequence>
#include <QTextBrowser>
#include <QSettings>
#include <QLineEdit>
#include <QKeyEvent>
#include <QDebug>

#include "assignhotkey.h"

#define SETTINGS_AUTOCLOSE "assignhotkey/autoclose"

/*****************************************************************************
 * Initialization
 *****************************************************************************/
AssignHotKey::AssignHotKey(QWidget* parent, const QKeySequence& keySequence)
    : QDialog(parent)
{
    setupUi(this);

#if defined(__APPLE__) || defined(WIN32)
    QString shift(QKeySequence(Qt::Key_Shift).toString(QKeySequence::NativeText));
    QString alt(QKeySequence(Qt::Key_Alt).toString(QKeySequence::NativeText));
    QString meta(QKeySequence(Qt::Key_Meta).toString(QKeySequence::NativeText));
#else
    QString shift("Shift");
    QString alt("Alt");
    QString meta("Meta");
#endif

    QString str("<HTML><HEAD><TITLE></TITLE></HEAD><BODY><CENTER>");
    str += QString("<H1>") + tr("Assign Key") + QString("</H1>");
    str += tr("Hit the key combination that you wish to assign. "
              "You may hit either a single key or a combination "
              "using %1, %2, and %3.").arg(shift).arg(alt).arg(meta);
    str += QString("</CENTER></BODY></HTML>");

    m_infoText->setText(str);
    m_infoText->setFocusPolicy(Qt::NoFocus);
    m_buttonBox->setFocusPolicy(Qt::NoFocus);

    m_previewEdit->setReadOnly(true);
    m_previewEdit->setAlignment(Qt::AlignCenter);

    m_keySequence = QKeySequence(keySequence);
    m_previewEdit->setText(m_keySequence.toString(QKeySequence::NativeText));

    QSettings settings;
    m_autoCloseCheckBox->setChecked(settings.value(SETTINGS_AUTOCLOSE).toBool());
}

AssignHotKey::~AssignHotKey()
{
    QSettings settings;
    settings.setValue(SETTINGS_AUTOCLOSE, m_autoCloseCheckBox->isChecked());
}

QKeySequence AssignHotKey::keySequence() const
{
    return m_keySequence;
}

void AssignHotKey::keyPressEvent(QKeyEvent* event)
{
    int key = event->key();
    if (event->key() == Qt::Key_Control || event->key() == Qt::Key_Alt ||
        event->key() == Qt::Key_Shift || event->key() == Qt::Key_Meta)
    {
        key = 0;
    }

    m_keySequence = QKeySequence(key | (event->modifiers() & ~Qt::ControlModifier));
    m_previewEdit->setText(m_keySequence.toString(QKeySequence::NativeText));

    if (m_autoCloseCheckBox->isChecked() == true && key != 0)
        accept();
}
