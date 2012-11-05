/*
  Q Light Controller
  aboutbox.h

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

#ifndef ABOUTBOX_H
#define ABOUTBOX_H

#include <QDialog>
#include "ui_aboutbox.h"

class QTimer;

class AboutBox : public QDialog, public Ui_AboutBox
{
    Q_OBJECT
    Q_DISABLE_COPY(AboutBox)

public:
    AboutBox(QWidget* parent);
    ~AboutBox();

private slots:
    void slotTimeout();
    void slotItemClicked();

private:
    QTimer* m_timer;
    int m_row;
    int m_increment;
};

#endif
