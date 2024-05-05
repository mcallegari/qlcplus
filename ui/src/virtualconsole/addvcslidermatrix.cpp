/*
  Q Light Controller
  addvcslidermatrix.cpp

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

#include <QSettings>
#include <QSpinBox>
#include <QAction>

#include "addvcslidermatrix.h"
#include "vcpropertieseditor.h"

#define SETTINGS_GEOMETRY "addvcslidermatrix/geometry"
#define SETTINGS_SLIDER_MATRIX_SIZE "slidermatrix/defaultSize"

AddVCSliderMatrix::AddVCSliderMatrix(QWidget* parent)
    : QDialog(parent)
    , m_amount(1)
    , m_height(100)
    , m_width(60)
{
    setupUi(this);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    QSettings settings;
    QVariant userVar = settings.value(SETTINGS_SLIDER_SIZE);
    if (userVar.isValid() == true)
    {
        QSize userSize = userVar.toSize();
        m_height = userSize.height();
        m_width = userSize.width();
    }

    QVariant var = settings.value(SETTINGS_SLIDER_MATRIX_SIZE);
    if (var.isValid() == true)
    {
        QSize size = var.toSize();
        m_amount = size.width();
        m_height = size.height();
    }

    QVariant geometrySettings = settings.value(SETTINGS_GEOMETRY);
    if (geometrySettings.isValid() == true)
        restoreGeometry(geometrySettings.toByteArray());

    m_amountSpin->setValue(m_amount);
    m_heightSpin->setValue(m_height);
    m_widthSpin->setValue(m_width);
}

AddVCSliderMatrix::~AddVCSliderMatrix()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

int AddVCSliderMatrix::amount() const
{
    return m_amount;
}

int AddVCSliderMatrix::height() const
{
    return m_height;
}

int AddVCSliderMatrix::width() const
{
    return m_width;
}

void AddVCSliderMatrix::accept()
{
    m_amount = m_amountSpin->value();
    m_height = m_heightSpin->value();
    m_width = m_widthSpin->value();

    QSettings settings;
    QSize size(m_amount, m_height);
    settings.setValue(SETTINGS_SLIDER_MATRIX_SIZE, size);

    QDialog::accept();
}
