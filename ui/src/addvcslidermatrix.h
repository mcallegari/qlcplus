/*
  Q Light Controller
  addvcslidermatrix.h

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

#ifndef ADDVCSLIDERMATRIX_H
#define ADDVCSLIDERMATRIX_H

#include <QDialog>
#include "ui_addvcslidermatrix.h"

class AddVCSliderMatrix : public QDialog, public Ui_AddVCSliderMatrix
{
    Q_OBJECT

public:
    AddVCSliderMatrix(QWidget* parent);
    ~AddVCSliderMatrix();

    int amount() const;
    int height() const;

public slots:
    void accept();

protected:
    int m_amount;
    int m_height;
};

#endif
