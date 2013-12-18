/*
  Q Light Controller
  dmxusbwidget.cpp

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

#include <QDebug>
#include "dmxusbwidget.h"

DMXUSBWidget::DMXUSBWidget(const QString& serial, const QString& name, const QString& vendor,
                                       QLCFTDI *ftdi, quint32 id)
{
    if (ftdi != NULL)
    {
        m_ftdi = ftdi;
        m_ftdi->modifyRefCount(1);
    }
    else
        m_ftdi = new QLCFTDI(serial, name, vendor, id);
}

DMXUSBWidget::~DMXUSBWidget()
{
    if (m_ftdi->refCount() == 1)
        delete m_ftdi;
    else
    {
        m_ftdi->modifyRefCount(-1);
        m_ftdi = NULL;
    }
}

QLCFTDI* DMXUSBWidget::ftdi() const
{
    return m_ftdi;
}

/****************************************************************************
 * Open & Close
 ****************************************************************************/

bool DMXUSBWidget::open()
{
    if (isOpen() == true)
        close();

    if (m_ftdi->open() == false)
        return close();

    if (m_ftdi->reset() == false)
        return close();

    if (m_ftdi->setBaudRate() == false)
        return close();

    if (m_ftdi->setLineProperties() == false)
        return close();

    if (m_ftdi->setFlowControl() == false)
        return close();

    if (m_ftdi->purgeBuffers() == false)
        return close();

    return true;
}

bool DMXUSBWidget::close()
{
    if (isOpen() == false)
        return true;

    return m_ftdi->close();
}

bool DMXUSBWidget::isOpen()
{
    return m_ftdi->isOpen();
}

/****************************************************************************
 * Name & Serial
 ****************************************************************************/

QString DMXUSBWidget::name() const
{
    return m_ftdi->name();
}

QString DMXUSBWidget::serial() const
{
    return m_ftdi->serial();
}

QString DMXUSBWidget::uniqueName() const
{
    return QString("%1 (S/N: %2)").arg(name()).arg(serial());
}

void DMXUSBWidget::setRealName(QString devName)
{
    m_realName = devName;
}

QString DMXUSBWidget::realName() const
{
    return m_realName;
}

QString DMXUSBWidget::vendor() const
{
    return m_ftdi->vendor();
}

/****************************************************************************
 * Write universe
 ****************************************************************************/

bool DMXUSBWidget::writeUniverse(const QByteArray& universe)
{
    Q_UNUSED(universe);
    return false;
}
