/*
  Q Light Controller
  addvcslidermatrix.cpp

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

#include <QSettings>
#include <QSpinBox>

#include "addvcslidermatrix.h"

#define SETTINGS_SLIDER_MATRIX_SIZE "slidermatrix/defaultSize"

AddVCSliderMatrix::AddVCSliderMatrix(QWidget* parent)
    : QDialog(parent)
    , m_amount(1)
    , m_height(100)
{
    setupUi(this);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    QSize size;
    QSettings settings;
    QVariant var = settings.value(SETTINGS_SLIDER_MATRIX_SIZE);
    if (var.isValid() == true)
        size = var.toSize();
    else
        size = QSize(m_amount, m_height);

    m_amountSpin->setValue(size.width());
    m_heightSpin->setValue(size.height());
}

AddVCSliderMatrix::~AddVCSliderMatrix()
{
}

int AddVCSliderMatrix::amount() const
{
    return m_amount;
}

int AddVCSliderMatrix::height() const
{
    return m_height;
}

void AddVCSliderMatrix::accept()
{
    m_amount = m_amountSpin->value();
    m_height = m_heightSpin->value();

    QSettings settings;
    QSize size(m_amount, m_height);
    settings.setValue(SETTINGS_SLIDER_MATRIX_SIZE, size);

    QDialog::accept();
}
