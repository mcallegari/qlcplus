/*
  Q Light Controller
  assignhotkey.h

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

#ifndef ASSIGNHOTKEY_H
#define ASSIGNHOTKEY_H

#include <QKeySequence>
#include <QDialog>

#include "ui_assignhotkey.h"

class QKeyEvent;

class AssignHotKey : public QDialog, public Ui_AssignHotKey
{
    Q_OBJECT
    Q_DISABLE_COPY(AssignHotKey)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    AssignHotKey(QWidget* parent, const QKeySequence& keySequence = QKeySequence());
    ~AssignHotKey();

    /*********************************************************************
     * Key sequence
     *********************************************************************/
public:
    /** Get the key sequence */
    QKeySequence keySequence() const;

private:
    QKeySequence m_keySequence;

protected:
    /** @reimp */
    void keyPressEvent(QKeyEvent* event);
};

#endif
