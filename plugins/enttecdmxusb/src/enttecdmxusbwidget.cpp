/*
  Q Light Controller
  enttecdmxusbwidget.cpp

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <QDebug>
#include "enttecdmxusbwidget.h"

EnttecDMXUSBWidget::EnttecDMXUSBWidget(const QString& serial, const QString& name,
                                       quint32 id)
    : m_ftdi(new QLCFTDI(serial, name, id))
{
}

EnttecDMXUSBWidget::~EnttecDMXUSBWidget()
{
    delete m_ftdi;
    m_ftdi = NULL;
}

QLCFTDI* EnttecDMXUSBWidget::ftdi() const
{
    return m_ftdi;
}

/****************************************************************************
 * Open & Close
 ****************************************************************************/

bool EnttecDMXUSBWidget::open()
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

bool EnttecDMXUSBWidget::close()
{
    if (isOpen() == false)
        return true;

    return m_ftdi->close();
}

bool EnttecDMXUSBWidget::isOpen()
{
    return m_ftdi->isOpen();
}

/****************************************************************************
 * Name & Serial
 ****************************************************************************/

QString EnttecDMXUSBWidget::name() const
{
    return m_ftdi->name();
}

QString EnttecDMXUSBWidget::serial() const
{
    return m_ftdi->serial();
}

QString EnttecDMXUSBWidget::uniqueName() const
{
    return QString("%1 (S/N: %2)").arg(name()).arg(serial());
}

/****************************************************************************
 * Write universe
 ****************************************************************************/

bool EnttecDMXUSBWidget::writeUniverse(const QByteArray& universe)
{
    Q_UNUSED(universe);
    return false;
}
