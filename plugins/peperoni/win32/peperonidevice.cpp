/*
  Q Light Controller
  peperonidevice.cpp

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

#include <QObject>
#include <QDebug>

#include "usbdmx-dynamic.h"
#include "peperonidevice.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

PeperoniDevice::PeperoniDevice(QObject* parent, struct usbdmx_functions* usbdmx,
                               int output)
    : QObject(parent)
{
    Q_ASSERT(usbdmx != NULL);

    m_handle = NULL;
    m_output = output;
    m_usbdmx = usbdmx;
    m_deviceOK = false;

    extractName();
}

PeperoniDevice::~PeperoniDevice()
{
    close();
}

/****************************************************************************
 * Properties
 ****************************************************************************/

QString PeperoniDevice::name() const
{
    return m_name;
}

int PeperoniDevice::output() const
{
    return m_output;
}

QString PeperoniDevice::infoText() const
{
    QString str;

    str += QString("<H3>%1</H3>").arg(m_name);
    str += QString("<P>");
    if (m_deviceOK == true)
        str += tr("Device is working correctly.");
    else
        str += tr("Device might not work correctly.");
    str += QString("</P>");

    return str;
}

void PeperoniDevice::extractName()
{
    bool needToClose = false;

    if (m_handle == NULL)
    {
        /* If the device was closed, we need to close it after name
           extraction. But if it was already open, we leave it that
           way, too. */
        needToClose = true;
        open();
    }

    if (m_handle == NULL)
    {
        /* Opening the device failed */
        m_name = tr("Nothing");
        m_deviceOK = false;
    }
    else
    {
        /* Check the device type and name it accordingly */
        if (m_usbdmx->is_xswitch(m_handle) == TRUE)
            m_name = QString("X-Switch");
        else if (m_usbdmx->is_rodin1(m_handle) == TRUE)
            m_name = QString("Rodin 1");
        else if (m_usbdmx->is_rodin2(m_handle) == TRUE)
            m_name = QString("Rodin 2");
        else if (m_usbdmx->is_rodint(m_handle) == TRUE)
            m_name = QString("Rodin T");
        else if (m_usbdmx->is_usbdmx21(m_handle) == TRUE)
            m_name = QString("USBDMX21");
        else
            m_name = tr("Unknown");

        m_deviceOK = true;
    }

    /* Close the device if it was opened only for name extraction */
    if (needToClose == true)
        close();
}

/****************************************************************************
 * Open & close
 ****************************************************************************/

void PeperoniDevice::open()
{
    if (m_handle != NULL)
        return;

    /* Open the device */
    if (m_usbdmx->open(m_output, &m_handle) == TRUE)
    {
        USHORT version;

        /* Check the device version against driver version */
        m_usbdmx->device_version(m_handle, &version);
        if (USBDMX_DLL_VERSION_CHECK(m_usbdmx) == FALSE)
            return;

        /* DMX512 specifies 0 as the official startcode */
        if (m_usbdmx->tx_startcode_set(m_handle, 0) == FALSE)
            return;
    }
    else
    {
        qWarning() << QString("Unable to open Peperoni %1").arg(m_output + 1);
    }
}

void PeperoniDevice::close()
{
    if (m_handle == NULL)
        return;

    m_usbdmx->close(m_handle);
    m_handle = NULL;
}

void PeperoniDevice::rehash()
{
    if (m_handle != NULL)
    {
        close();
        open();
    }

    extractName();
}

/****************************************************************************
 * Write
 ****************************************************************************/

void PeperoniDevice::outputDMX(const QByteArray& universe)
{
    if (m_handle != NULL)
        m_usbdmx->tx_set(m_handle, (unsigned char*) universe.data(),
                         universe.size());
}
