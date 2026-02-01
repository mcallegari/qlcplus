/*
  Q Light Controller Plus
  physicaledit.cpp

  Copyright (c) Massimo Callegari

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


#include "physicaledit.h"

PhysicalEdit::PhysicalEdit(QLCPhysical phy, QObject *parent)
    : QObject(parent)
    , m_phy(phy)
{

}

PhysicalEdit::~PhysicalEdit()
{

}

QLCPhysical PhysicalEdit::physical()
{
    return m_phy;
}

QString PhysicalEdit::bulbType() const
{
    return m_phy.bulbType();
}

void PhysicalEdit::setBulbType(const QString type)
{
    if (m_phy.bulbType() == type)
        return;

    m_phy.setBulbType(type);
    emit bulbTypeChanged();
    emit changed();
}

int PhysicalEdit::bulbLumens() const
{
    return m_phy.bulbLumens();
}

void PhysicalEdit::setBulbLumens(int lumens)
{
    if (m_phy.bulbLumens() == lumens)
        return;

    m_phy.setBulbLumens(lumens);
    emit bulbLumensChanged();
    emit changed();
}

int PhysicalEdit::bulbColorTemperature() const
{
    return m_phy.bulbColourTemperature();
}

void PhysicalEdit::setBulbColorTemperature(int temp)
{
    if (m_phy.bulbColourTemperature() == temp)
        return;

    m_phy.setBulbColourTemperature(temp);
    emit bulbColorTemperatureChanged();
    emit changed();
}

double PhysicalEdit::weight() const
{
    return m_phy.weight();
}

void PhysicalEdit::setWeight(double weight)
{
    if (m_phy.weight() == weight)
        return;

    m_phy.setWeight(weight);
    emit weightChanged();
    emit changed();
}

int PhysicalEdit::width() const
{
    return m_phy.width();
}

void PhysicalEdit::setWidth(int width)
{
    if (m_phy.width() == width)
        return;

    m_phy.setWidth(width);
    emit widthChanged();
    emit changed();
}

int PhysicalEdit::height() const
{
    return m_phy.height();
}

void PhysicalEdit::setHeight(int height)
{
    if (m_phy.height() == height)
        return;

    m_phy.setHeight(height);
    emit heightChanged();
    emit changed();
}

int PhysicalEdit::depth() const
{
    return m_phy.depth();
}

void PhysicalEdit::setDepth(int depth)
{
    if (m_phy.depth() == depth)
        return;

    m_phy.setDepth(depth);
    emit depthChanged();
    emit changed();
}

QString PhysicalEdit::lensType() const
{
    return m_phy.lensName();
}

void PhysicalEdit::setLensType(const QString type)
{
    if (m_phy.lensName() == type)
        return;

    m_phy.setLensName(type);
    emit lensTypeChanged();
    emit changed();
}

double PhysicalEdit::lensDegreesMin() const
{
    return m_phy.lensDegreesMin();
}

void PhysicalEdit::setLensDegreesMin(double degrees)
{
    if (m_phy.lensDegreesMin() == degrees)
        return;

    m_phy.setLensDegreesMin(degrees);
    emit lensDegreesMinChanged();
    emit changed();
}

double PhysicalEdit::lensDegreesMax() const
{
    return m_phy.lensDegreesMax();
}

void PhysicalEdit::setLensDegreesMax(double degrees)
{
    if (m_phy.lensDegreesMax() == degrees)
        return;

    m_phy.setLensDegreesMax(degrees);
    emit lensDegreesMaxChanged();
    emit changed();
}

QString PhysicalEdit::focusType() const
{
    return m_phy.focusType();
}

void PhysicalEdit::setFocusType(const QString type)
{
    if (m_phy.focusType() == type)
        return;

    m_phy.setFocusType(type);
    emit focusTypeChanged();
    emit changed();
}

int PhysicalEdit::focusPanMax() const
{
    return m_phy.focusPanMax();
}

void PhysicalEdit::setFocusPanMax(int pan)
{
    if (m_phy.focusPanMax() == pan)
        return;

    m_phy.setFocusPanMax(pan);
    emit focusPanMaxChanged();
    emit changed();
}

int PhysicalEdit::focusTiltMax() const
{
    return m_phy.focusTiltMax();
}

void PhysicalEdit::setFocusTiltMax(int tilt)
{
    if (m_phy.focusTiltMax() == tilt)
        return;

    m_phy.setFocusTiltMax(tilt);
    emit focusTiltMaxChanged();
    emit changed();
}

QSize PhysicalEdit::layoutSize() const
{
    return m_phy.layoutSize();
}

void PhysicalEdit::setLayoutSize(QSize size)
{
    if (m_phy.layoutSize() == size)
        return;

    m_phy.setLayoutSize(size);
    emit layoutSizeChanged();
    emit changed();
}

int PhysicalEdit::powerConsumption() const
{
    return m_phy.powerConsumption();
}

void PhysicalEdit::setPowerConsumption(int watt)
{
    if (m_phy.powerConsumption() == watt)
        return;

    m_phy.setPowerConsumption(watt);
    emit powerConsumptionChanged();
    emit changed();
}

QString PhysicalEdit::dmxConnector() const
{
    return m_phy.dmxConnector();
}

void PhysicalEdit::setDmxConnector(const QString type)
{
    if (m_phy.dmxConnector() == type)
        return;

    m_phy.setDmxConnector(type);
    emit dmxConnectorChanged();
    emit changed();
}
