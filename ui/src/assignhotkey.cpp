/*
  Q Light Controller
  assignhotkey.cpp

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

#include <QKeySequence>
#include <QTextBrowser>
#include <QSettings>
#include <QLineEdit>
#include <QKeyEvent>
#include <QDebug>
#include <QSettings>

#include "assignhotkey.h"

#define SETTINGS_GEOMETRY "assignhotkey/geometry"
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

    QVariant geometrySettings = settings.value(SETTINGS_GEOMETRY);
    if (geometrySettings.isValid() == true)
        restoreGeometry(geometrySettings.toByteArray());
}

AssignHotKey::~AssignHotKey()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
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
