/*
  Q Light Controller
  vcxypadfixture.cpp

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

#include <QCheckBox>
#include <QSpinBox>
#include <QDialog>
#include <QString>
#include <cmath>

#include "vcxypadfixtureeditor.h"
#include "vcxypadfixture.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

VCXYPadFixtureEditor::VCXYPadFixtureEditor(QWidget* parent,
        QList <VCXYPadFixture> fixtures) : QDialog(parent)
{
    setupUi(this);

    m_fixtures = fixtures;

    /* Take initial values from the first fixture */
    if (fixtures.count() > 0)
    {
        VCXYPadFixture fxi = fixtures.first();

        m_xMin->setValue(int(floor((fxi.xMin() * qreal(100)) + qreal(0.5))));
        m_xMax->setValue(int(floor((fxi.xMax() * qreal(100)) + qreal(0.5))));
        m_xReverse->setChecked(fxi.xReverse());

        m_yMin->setValue(int(floor((fxi.yMin() * qreal(100)) + qreal(0.5))));
        m_yMax->setValue(int(floor((fxi.yMax() * qreal(100)) + qreal(0.5))));
        m_yReverse->setChecked(fxi.yReverse());
    }
}

VCXYPadFixtureEditor::~VCXYPadFixtureEditor()
{
}

void VCXYPadFixtureEditor::slotXMinChanged(int value)
{
    if (value >= m_xMax->value())
        m_xMax->setValue(value + 1);
}

void VCXYPadFixtureEditor::slotXMaxChanged(int value)
{
    if (value <= m_xMin->value())
        m_xMin->setValue(value - 1);
}

void VCXYPadFixtureEditor::slotYMinChanged(int value)
{
    if (value >= m_yMax->value())
        m_yMax->setValue(value + 1);
}

void VCXYPadFixtureEditor::slotYMaxChanged(int value)
{
    if (value <= m_yMin->value())
        m_yMin->setValue(value - 1);
}

void VCXYPadFixtureEditor::accept()
{
    /* Put dialog values to all fixtures */
    QMutableListIterator <VCXYPadFixture> it(m_fixtures);
    while (it.hasNext() == true)
    {
        VCXYPadFixture fxi(it.next());

        fxi.setX(qreal(m_xMin->value()) / qreal(100), qreal(m_xMax->value()) / qreal(100),
                 m_xReverse->isChecked());
        fxi.setY(qreal(m_yMin->value()) / qreal(100), qreal(m_yMax->value()) / qreal(100),
                 m_yReverse->isChecked());

        it.setValue(fxi);
    }

    QDialog::accept();
}

/****************************************************************************
 * Fixtures
 ****************************************************************************/

QList <VCXYPadFixture> VCXYPadFixtureEditor::fixtures() const
{
    return m_fixtures;
}

