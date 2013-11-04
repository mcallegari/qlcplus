/*
  Q Light Controller
  win32peperonidevice.cpp

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
