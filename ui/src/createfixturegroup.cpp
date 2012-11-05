/*
  Q Light Controller
  createfixturegroup.cpp

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

#include "createfixturegroup.h"

CreateFixtureGroup::CreateFixtureGroup(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
}

CreateFixtureGroup::~CreateFixtureGroup()
{
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
