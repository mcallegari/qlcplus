/*
  Q Light Controller
  dmxaddresstool.cpp

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


#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>

#include "dmxaddresstool.h"

DMXAddressTool::DMXAddressTool(QWidget* parent) : QDialog(parent)
{
    m_address = 1;
    m_updateValue = true;
}

DMXAddressTool::~DMXAddressTool()
{
}

void DMXAddressTool::slotDecimalChanged(const QString &text)
{
    int number;

    if (m_updateValue == false)
        return;

    m_updateValue = false;

    number = m_decimalSpin->value();
    m_address = number;

    number -= 256;
    if (number >= 0)
    {
        m_256->setCheckState(Qt::Checked);
    }
    else
    {
        number += 256;
        m_256->setCheckState(Qt::Unchecked);
    }

    number -= 128;
    if (number >= 0)
    {
        m_128->setCheckState(Qt::Checked);
    }
    else
    {
        number += 128;
        m_128->setCheckState(Qt::Unchecked);
    }

    number -= 64;
    if (number >= 0)
    {
        m_64->setCheckState(Qt::Checked);
    }
    else
    {
        number += 64;
        m_64->setCheckState(Qt::Unchecked);
    }

    number -= 32;
    if (number >= 0)
    {
        m_32->setCheckState(Qt::Checked);
    }
    else
    {
        number += 32;
        m_32->setCheckState(Qt::Unchecked);
    }

    number -= 16;
    if (number >= 0)
    {
        m_16->setCheckState(Qt::Checked);
    }
    else
    {
        number += 16;
        m_16->setCheckState(Qt::Unchecked);
    }

    number -= 8;
    if (number >= 0)
    {
        m_8->setCheckState(Qt::Checked);
    }
    else
    {
        number += 8;
        m_8->setCheckState(Qt::Unchecked);
    }

    number -= 4;
    if (number >= 0)
    {
        m_4->setCheckState(Qt::Checked);
    }
    else
    {
        number += 4;
        m_4->setCheckState(Qt::Unchecked);
    }

    number -= 2;
    if (number >= 0)
    {
        m_2->setCheckState(Qt::Checked);
    }
    else
    {
        number += 2;
        m_2->setCheckState(Qt::Unchecked);
    }

    number -= 1;
    if (number >= 0)
    {
        m_1->setCheckState(Qt::Checked);
    }
    else
    {
        number += 1;
        m_1->setCheckState(Qt::Unchecked);
    }

    m_updateValue = true;
}

void DMXAddressTool::slotSliderValueChanged(int value)
{
    int number = 0;
    QString str;

    if (m_updateValue == false)
        return;

    m_updateValue = false;

    if (m_256->checkState() == Qt::Checked) number += 256;
    if (m_128->checkState() == Qt::Checked) number += 128;
    if (m_64->checkState() == Qt::Checked) number += 64;
    if (m_32->checkState() == Qt::Checked) number += 32;
    if (m_16->checkState() == Qt::Checked) number += 16;
    if (m_8->checkState() == Qt::Checked) number += 8;
    if (m_4->checkState() == Qt::Checked) number += 4;
    if (m_2->checkState() == Qt::Checked) number += 2;
    if (m_1->checkState() == Qt::Checked) number += 1;

    m_address = number;
    m_decimalSpin->setValue(number);

    m_updateValue = true;
}

void DMXAddressTool::setAddress(int address)
{
    m_address = address;
    m_decimalSpin->setValue(address);
}
