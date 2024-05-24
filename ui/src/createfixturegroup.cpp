/*
  Q Light Controller
  createfixturegroup.cpp

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

#include <QSettings>

#include "createfixturegroup.h"

#define SETTINGS_GEOMETRY "createfixturegroup/geometry"

CreateFixtureGroup::CreateFixtureGroup(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

    QSettings settings;
    QVariant geometrySettings = settings.value(SETTINGS_GEOMETRY);
    if (geometrySettings.isValid() == true)
        restoreGeometry(geometrySettings.toByteArray());
}

CreateFixtureGroup::~CreateFixtureGroup()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

QString CreateFixtureGroup::name() const
{
    Q_ASSERT(m_nameEdit != NULL);
    return m_nameEdit->text();
}

void CreateFixtureGroup::setSize(const QSize& size)
{
    Q_ASSERT(m_widthSpin != NULL);
    Q_ASSERT(m_heightSpin != NULL);
    m_widthSpin->setValue(size.width());
    m_heightSpin->setValue(size.height());
}

QSize CreateFixtureGroup::size() const
{
    Q_ASSERT(m_widthSpin != NULL);
    Q_ASSERT(m_heightSpin != NULL);
    return QSize(m_widthSpin->value(), m_heightSpin->value());
}
