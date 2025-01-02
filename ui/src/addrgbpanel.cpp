/*
  Q Light Controller Plus
  addrgbpanel.cpp

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

#include <QPushButton>
#include <QDebug>
#include <QSettings>

#include "addrgbpanel.h"
#include "doc.h"

#define SETTINGS_GEOMETRY "addrgbpanel/geometry"

AddRGBPanel::AddRGBPanel(QWidget *parent, const Doc *doc)
    : QDialog(parent)
    , m_doc(doc)
{
    setupUi(this);

    /* Fill universe combo with available universes */
    m_uniCombo->addItems(m_doc->inputOutputMap()->universeNames());

    m_compCombo->addItem("RGB");
    m_compCombo->addItem("BGR");
    m_compCombo->addItem("BRG");
    m_compCombo->addItem("GBR");
    m_compCombo->addItem("GRB");
    m_compCombo->addItem("RBG");
    m_compCombo->addItem("RGBW");

    checkAddressAvailability();

    QSettings settings;
    QVariant geometrySettings = settings.value(SETTINGS_GEOMETRY);
    if (geometrySettings.isValid() == true)
        restoreGeometry(geometrySettings.toByteArray());

    connect(m_uniCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotUniverseChanged()));
    connect(m_compCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotComponentsChanged(int)));
    connect(m_addressSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotAddressChanged()));
    connect(m_columnSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotSizeChanged(int)));
    connect(m_rowSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotSizeChanged(int)));
}

AddRGBPanel::~AddRGBPanel()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}
bool AddRGBPanel::checkAddressAvailability()
{
    int uniAddr = m_doc->inputOutputMap()->getUniverseID(m_uniCombo->currentIndex());
    int startAddress = ((m_addressSpin->value() - 1) & 0x01FF) | (uniAddr << 9);
    int channels = m_columnSpin->value() * m_rowSpin->value() * 3;
    QPushButton *okBtn = buttonBox->button(QDialogButtonBox::Ok);

    qDebug() << "Check availability for address: " << startAddress;

    for (int i = 0; i < channels; i++)
    {
        quint32 fid = m_doc->fixtureForAddress(startAddress + i);
        if (fid != Fixture::invalidId())
        {
            m_addrErrorLabel->show();
            okBtn->setEnabled(false);
            return false;
        }
    }
    m_addrErrorLabel->hide();
    okBtn->setEnabled(true);
    return true;
}

void AddRGBPanel::slotUniverseChanged()
{
    checkAddressAvailability();
}

void AddRGBPanel::slotComponentsChanged(int index)
{
    if (index == 6) // RGBW
        m_columnSpin->setMaximum(128);
    else
        m_columnSpin->setMaximum(170);
}

void AddRGBPanel::slotAddressChanged()
{
    checkAddressAvailability();
}

QString AddRGBPanel::name()
{
    return m_nameEdit->text();
}

int AddRGBPanel::universeIndex()
{
    return m_uniCombo->currentIndex();
}

int AddRGBPanel::address()
{
    return m_addressSpin->value() - 1;
}

int AddRGBPanel::columns()
{
    return m_columnSpin->value();
}

int AddRGBPanel::rows()
{
    return m_rowSpin->value();
}

quint32 AddRGBPanel::physicalWidth()
{
    return m_phyWidthSpin->value();
}

quint32 AddRGBPanel::physicalHeight()
{
    return m_phyHeightSpin->value();
}

AddRGBPanel::Orientation AddRGBPanel::orientation()
{
    if (m_oriTopLeftRadio->isChecked())
        return TopLeft;
    else if (m_oriTopRightRadio->isChecked())
        return TopRight;
    else if (m_oriBottomLeftRadio->isChecked())
        return BottomLeft;
    else if (m_oriBottomRightRadio->isChecked())
        return BottomRight;

    return None;
}

AddRGBPanel::Type AddRGBPanel::type()
{
    if (m_snakeRadio->isChecked())
        return Snake;
    else if (m_zigzagRadio->isChecked())
        return ZigZag;

    return Unknown;
}

AddRGBPanel::Direction AddRGBPanel::direction()
{
	if (m_verticalRadio->isChecked())
		return Vertical;
	else if (m_horizontalRadio->isChecked())
		return Horizontal;

	return Undefined;
}

Fixture::Components AddRGBPanel::components()
{
    if (m_compCombo->currentIndex() == 1)
        return Fixture::BGR;
    else if (m_compCombo->currentIndex() == 2)
        return Fixture::BRG;
    else if (m_compCombo->currentIndex() == 3)
        return Fixture::GBR;
    else if (m_compCombo->currentIndex() == 4)
        return Fixture::GRB;
    else if (m_compCombo->currentIndex() == 5)
        return Fixture::RBG;
    else if (m_compCombo->currentIndex() == 6)
        return Fixture::RGBW;

    return Fixture::RGB;
}

bool AddRGBPanel::is16Bit()
{
    return m_16bitCheck->isChecked();
}

bool AddRGBPanel::crossUniverse()
{
    return m_crossUniverseCheck->isChecked();
}

void AddRGBPanel::slotSizeChanged(int)
{
    checkAddressAvailability();
    m_totalLabel->setText(QString::number(m_columnSpin->value() * m_rowSpin->value()));
}
