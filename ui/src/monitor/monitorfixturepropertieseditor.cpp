/*
  Q Light Controller Plus
  monitorfixturepropertieseditor.cpp

  Copyright (C) Massimo Callegari

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

#include <QColorDialog>

#include "monitorfixturepropertieseditor.h"
#include "monitorgraphicsview.h"
#include "monitorfixtureitem.h"
#include "monitorproperties.h"

MonitorFixturePropertiesEditor::MonitorFixturePropertiesEditor(
        MonitorFixtureItem *fxItem, MonitorGraphicsView *gfxView,
        MonitorProperties *props, QWidget *parent) :
    QWidget(parent)
  , m_fxItem(fxItem)
  , m_gfxView(gfxView)
  , m_props(props)
{
    Q_ASSERT(fxItem != NULL);
    Q_ASSERT(gfxView != NULL);
    Q_ASSERT(props != NULL);

    setupUi(this);

    m_xPosSpin->setMaximum(m_gfxView->gridSize().width());
    m_yPosSpin->setMaximum(m_gfxView->gridSize().height());

    if (m_props->gridUnits() == MonitorProperties::Feet)
    {
        m_xPosSpin->setSuffix("ft");
        m_yPosSpin->setSuffix("ft");
    }
    else
    {
        m_xPosSpin->setSuffix("m");
        m_yPosSpin->setSuffix("m");
    }

    m_fxName->setText(m_fxItem->name());
    m_xPosSpin->setValue(m_fxItem->realPosition().x() / 1000);
    m_yPosSpin->setValue(m_fxItem->realPosition().y() / 1000);
    m_rotationSpin->setValue(m_fxItem->rotation());

    QPixmap pm(28, 28);
    if (m_fxItem->getColor().isValid())
    {
        pm.fill(m_fxItem->getColor());
        m_gelColorButton->setIcon(QIcon(pm));
    }

    connect(m_xPosSpin, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetPosition()));
    connect(m_yPosSpin, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetPosition()));
    connect(m_rotationSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotRotationChanged(int)));
    connect(m_gelColorButton, SIGNAL(clicked()),
            this, SLOT(slotGelColorClicked()));
    connect(m_gelResetButton, SIGNAL(clicked()),
            this, SLOT(slotGelResetClicked()));
}

MonitorFixturePropertiesEditor::~MonitorFixturePropertiesEditor()
{

}

void MonitorFixturePropertiesEditor::slotSetPosition()
{
    QPointF itemPos(m_xPosSpin->value() * 1000, m_yPosSpin->value() * 1000);
    m_fxItem->setPos(m_gfxView->realPositionToPixels(itemPos.x(), itemPos.y()));
    m_fxItem->setRealPosition(itemPos);
    m_props->setFixturePosition(m_fxItem->fixtureID(), 0, 0,QVector3D(itemPos.x(), itemPos.y(), 0));
}

void MonitorFixturePropertiesEditor::slotRotationChanged(int value)
{
    m_fxItem->setRotation(value);
    m_props->setFixtureRotation(m_fxItem->fixtureID(), 0, 0, QVector3D(0, value, 0));
}

void MonitorFixturePropertiesEditor::slotGelColorClicked()
{
    QColor color = m_fxItem->getColor();
    QColor newColor = QColorDialog::getColor(color);

    if (newColor.isValid())
    {
        m_fxItem->setGelColor(newColor);
        m_props->setFixtureGelColor(m_fxItem->fixtureID(), 0, 0, newColor);
        QPixmap pm(28, 28);
        pm.fill(newColor);
        m_gelColorButton->setIcon(QIcon(pm));
    }
}

void MonitorFixturePropertiesEditor::slotGelResetClicked()
{
    m_gelColorButton->setIcon(QIcon());
    m_fxItem->setGelColor(QColor());
    m_props->setFixtureGelColor(m_fxItem->fixtureID(), 0, 0, QColor());
}

