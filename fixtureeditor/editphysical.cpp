/*
  Q Light Controller Plus - Fixture Editor
  editphysical.cpp

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

#include "editphysical.h"
#include "ui_editphysical.h"

EditPhysical::EditPhysical(QLCPhysical physical, QWidget *parent)
    : QWidget(parent)
    , m_physical(physical)

{
    setupUi(this);

    QString str;

    m_bulbTypeCombo->setEditText(m_physical.bulbType());
    m_bulbLumensSpin->setValue(m_physical.bulbLumens());
    m_bulbTempCombo->setEditText(str.setNum(m_physical.bulbColourTemperature()));

    m_weightSpin->setValue(m_physical.weight());
    m_widthSpin->setValue(m_physical.width());
    m_heightSpin->setValue(m_physical.height());
    m_depthSpin->setValue(m_physical.depth());

    m_lensNameCombo->setEditText(m_physical.lensName());
    m_lensDegreesMinSpin->setValue(m_physical.lensDegreesMin());
    m_lensDegreesMaxSpin->setValue(m_physical.lensDegreesMax());

    m_focusTypeCombo->setEditText(m_physical.focusType());
    m_panMaxSpin->setValue(m_physical.focusPanMax());
    m_tiltMaxSpin->setValue(m_physical.focusTiltMax());
    m_layoutColsSpin->setValue(m_physical.layoutSize().width());
    m_layoutRowsSpin->setValue(m_physical.layoutSize().height());

    m_powerConsumptionSpin->setValue(m_physical.powerConsumption());
    m_dmxConnectorCombo->setEditText(m_physical.dmxConnector());

    connect(copyClipboardButton, SIGNAL(clicked()),
            this, SLOT(slotCopyToClipboard()));
    connect(pasteClipboardButton, SIGNAL(clicked()),
            this, SLOT(slotPasteFromClipboard()));
}

EditPhysical::~EditPhysical()
{

}

QLCPhysical EditPhysical::physical()
{
    m_physical.setBulbType(m_bulbTypeCombo->currentText());
    m_physical.setBulbLumens(m_bulbLumensSpin->value());
    m_physical.setBulbColourTemperature(m_bulbTempCombo->currentText().toInt());
    m_physical.setWeight(m_weightSpin->value());
    m_physical.setWidth(m_widthSpin->value());
    m_physical.setHeight(m_heightSpin->value());
    m_physical.setDepth(m_depthSpin->value());
    m_physical.setLensName(m_lensNameCombo->currentText());
    m_physical.setLensDegreesMin(m_lensDegreesMinSpin->value());
    m_physical.setLensDegreesMax(m_lensDegreesMaxSpin->value());
    m_physical.setFocusType(m_focusTypeCombo->currentText());
    m_physical.setFocusPanMax(m_panMaxSpin->value());
    m_physical.setFocusTiltMax(m_tiltMaxSpin->value());
    m_physical.setLayoutSize(QSize(m_layoutColsSpin->value(), m_layoutRowsSpin->value()));
    m_physical.setPowerConsumption(m_powerConsumptionSpin->value());
    m_physical.setDmxConnector(m_dmxConnectorCombo->currentText());

    return m_physical;
}

void EditPhysical::slotCopyToClipboard()
{
    emit copyToClipboard(m_physical);
}

void EditPhysical::slotPasteFromClipboard()
{
    emit requestPasteFromClipboard();
}

void EditPhysical::pasteFromClipboard(QLCPhysical clipboard)
{
    m_bulbLumensSpin->setValue(clipboard.bulbLumens());
    m_bulbTypeCombo->setEditText(clipboard.bulbType());
    m_bulbTempCombo->setEditText(QString::number(clipboard.bulbColourTemperature()));
    m_weightSpin->setValue(clipboard.weight());
    m_widthSpin->setValue(clipboard.width());
    m_heightSpin->setValue(clipboard.height());
    m_depthSpin->setValue(clipboard.depth());
    m_lensNameCombo->setEditText(clipboard.lensName());
    m_lensDegreesMinSpin->setValue(clipboard.lensDegreesMin());
    m_lensDegreesMaxSpin->setValue(clipboard.lensDegreesMax());
    m_focusTypeCombo->setEditText(clipboard.focusType());
    m_panMaxSpin->setValue(clipboard.focusPanMax());
    m_tiltMaxSpin->setValue(clipboard.focusTiltMax());
    m_layoutColsSpin->setValue(clipboard.layoutSize().width());
    m_layoutRowsSpin->setValue(clipboard.layoutSize().height());
    m_powerConsumptionSpin->setValue(clipboard.powerConsumption());
    m_dmxConnectorCombo->setEditText(clipboard.dmxConnector());
}
