/*
  Q Light Controller
  win32peperonidevice.cpp

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

#include <QDebug>

#include "win32peperonidevice.h"
#include "usbdmx-dynamic.h"
#include "peperonidefs.h"

Win32PeperoniDevice::Win32PeperoniDevice(const QVariant& uid, const QString& name, USHORT id,
                                         struct usbdmx_functions* usbdmx,
                                         QObject* parent)
    : OutputDevice(uid, name, parent)
    , m_usbdmx(usbdmx)
    , m_id(id)
    , m_handle(NULL)
{
    qDebug() << Q_FUNC_INFO;

    Q_ASSERT(usbdmx != NULL);
}

Win32PeperoniDevice::~Win32PeperoniDevice()
{
    qDebug() << Q_FUNC_INFO;

    close();
}

void Win32PeperoniDevice::open()
{
    qDebug() << Q_FUNC_INFO;

    if (isOpen() == true)
        return;

    if (m_usbdmx->open(m_id, &m_handle) == TRUE)
    {
        /* DMX512 specifies 0 as the official startcode */
        if (m_usbdmx->tx_startcode_set(m_handle, 0) == FALSE)
            qWarning() << "Unable to set DMX startcode on device with UID:" << uid();
    }
    else
    {
        qWarning() << "Unable to open device with UID:" << uid();
        m_handle = NULL;
    }
}

void Win32PeperoniDevice::close()
{
    qDebug() << Q_FUNC_INFO;

    if (isOpen() == false)
        return;

    m_usbdmx->close(m_handle);
    m_handle = NULL;
}

bool Win32PeperoniDevice::isOpen() const
{
    if (m_handle != NULL)
        return true;
    else
        return false;
}

void Win32PeperoniDevice::writeChannel(ushort channel, uchar value)
{
    Q_UNUSED(channel);
    Q_UNUSED(value);
}

void Win32PeperoniDevice::writeUniverse(const QByteArray& universe)
{
    if (isOpen() == true)
        m_usbdmx->tx_set(m_handle, (uchar*) universe.constData(), universe.size());
}
