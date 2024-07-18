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
#include <QSettings>

#include "vcxypadfixtureeditor.h"
#include "vcxypadfixture.h"

#define SETTINGS_GEOMETRY "vcxypadfixtureeditor/geometry"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

VCXYPadFixtureEditor::VCXYPadFixtureEditor(QWidget* parent, QList <VCXYPadFixture> fixtures)
    : QDialog(parent)
{
    setupUi(this);

    m_fixtures = fixtures;
    m_maxXVal = 100, m_maxYVal = 100;
    QString units = "%";

    /* Take initial values from the first fixture */
    if (fixtures.count() > 0)
    {
        VCXYPadFixture fxi = fixtures.first();

        if (fxi.displayMode() == VCXYPadFixture::DMX)
        {
            m_maxXVal = m_maxYVal = 255;
            units = "";
        }
        else if (fxi.displayMode() == VCXYPadFixture::Degrees)
        {
            m_maxXVal = fxi.degreesRange().width();
            m_maxYVal = fxi.degreesRange().height();
            units = "°";
        }

        m_xMax->setMaximum(m_maxXVal);
        m_xMin->setMaximum(m_maxXVal);
        m_yMax->setMaximum(m_maxYVal);
        m_yMin->setMaximum(m_maxYVal);

        m_xMin->setSuffix(units);
        m_xMax->setSuffix(units);
        m_yMin->setSuffix(units);
        m_yMax->setSuffix(units);

        m_xMin->setValue(int(floor((fxi.xMin() * qreal(m_maxXVal)) + qreal(0.5))));
        m_xMax->setValue(int(floor((fxi.xMax() * qreal(m_maxXVal)) + qreal(0.5))));
        m_xReverse->setChecked(fxi.xReverse());

        m_yMin->setValue(int(floor((fxi.yMin() * qreal(m_maxYVal)) + qreal(0.5))));
        m_yMax->setValue(int(floor((fxi.yMax() * qreal(m_maxYVal)) + qreal(0.5))));
        m_yReverse->setChecked(fxi.yReverse());
    }

    QSettings settings;
    QVariant geometrySettings = settings.value(SETTINGS_GEOMETRY);
    if (geometrySettings.isValid() == true)
        restoreGeometry(geometrySettings.toByteArray());
}

VCXYPadFixtureEditor::~VCXYPadFixtureEditor()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
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

        fxi.setX(qreal(m_xMin->value()) / qreal(m_maxXVal), qreal(m_xMax->value()) / qreal(m_maxXVal),
                 m_xReverse->isChecked());
        fxi.setY(qreal(m_yMin->value()) / qreal(m_maxYVal), qreal(m_yMax->value()) / qreal(m_maxYVal),
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

