/*
  Q Light Controller
  peperonidevice.cpp

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
#include <usb.h>

#include "peperonidevice.h"
#include "peperonidefs.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

PeperoniDevice::PeperoniDevice(QObject* parent, struct usb_device* device)
    : QObject(parent)
    , m_device(device)
    , m_handle(NULL)
{
    Q_ASSERT(device != NULL);

    /* Store fw version so we don't need to rely on libusb's volatile data */
    m_firmwareVersion = m_device->descriptor.bcdDevice;
    qDebug() << "[Peperoni] detected device firmware version:" << QString::number(m_firmwareVersion, 16);

    if (m_firmwareVersion < PEPERONI_FW_BLOCKING_WRITE_SUPPORT)
        m_blockingControlWrite = PEPERONI_TX_MEM_NONBLOCK;
    else
        m_blockingControlWrite = PEPERONI_TX_MEM_BLOCK;

    extractName();
}

PeperoniDevice::~PeperoniDevice()
{
    close();
}

/****************************************************************************
 * Device information
 ****************************************************************************/

bool PeperoniDevice::isPeperoniDevice(const struct usb_device* device)
{
    /* If there's nothing to inspect, it can't be what we're looking for */
    if (device == NULL)
        return false;

    /* If it's not manufactured by them, we're not interested in it */
    if (device->descriptor.idVendor != PEPERONI_VID)
        return false;

    if (device->descriptor.idProduct == PEPERONI_PID_RODIN1 ||
        device->descriptor.idProduct == PEPERONI_PID_RODIN2 ||
        device->descriptor.idProduct == PEPERONI_PID_RODINT ||
        device->descriptor.idProduct == PEPERONI_PID_XSWITCH ||
        device->descriptor.idProduct == PEPERONI_PID_USBDMX21)
    {
        /* We need one interface */
        if (device->config->bNumInterfaces < 1)
            return false;

        return true;
    }
    else
    {
        return false;
    }
}

void PeperoniDevice::extractName()
{
    bool needToClose = false;
    char name[256];
    int len;

    Q_ASSERT(m_device != NULL);

    if (m_handle == NULL)
    {
        needToClose = true;
        open();
    }

    /* Check, whether open() was successful */
    if (m_handle == NULL)
        return;

    /* Extract the name */
    len = usb_get_string_simple(m_handle, m_device->descriptor.iProduct,
                                name, sizeof(name));
    if (len > 0)
        m_name = QString(name);
    else
        m_name = tr("Unknown");

    /* Close the device if it was opened for this function only. */
    if (needToClose == true)
        close();
}

QString PeperoniDevice::name() const
{
    return m_name;
}

QString PeperoniDevice::infoText() const
{
    QString info;

    if (m_device != NULL)
    {
        info += QString("<B>%1</B>").arg(name());
        info += QString("<P>");
        info += tr("Device is working correctly.");
        info += QString("<BR/>");
        info += tr("Firmware version: %1").arg(m_firmwareVersion, 4, 16, QChar('0'));
        info += QString("</P>");
    }
    else
    {
        info += QString("<B>");
        info += tr("Unknown device");
        info += QString("</B>");
        info += QString("<P>");
        info += tr("Cannot connect to USB device.");
        info += QString("</P>");
    }

    return info;
}

/****************************************************************************
 * Open & close
 ****************************************************************************/

void PeperoniDevice::open()
{
    if (m_device != NULL && m_handle == NULL)
    {
        int r = -1;
        int configuration = PEPERONI_CONF_TXONLY;

        m_handle = usb_open(m_device);
        if (m_handle == NULL)
        {
            qWarning() << "Unable to open PeperoniDevice with idProduct:" << m_device->descriptor.idProduct;
            return;
        }

        /* Use configuration #2 on X-Switch */
        if (m_device->descriptor.idProduct == PEPERONI_PID_XSWITCH)
            configuration = PEPERONI_CONF_TXRX;
        else
            configuration = PEPERONI_CONF_TXONLY;

        /* Set selected configuration */
        r = usb_set_configuration(m_handle, configuration);
        if (r < 0)
            qWarning() << "PeperoniDevice is unable to set configuration #" << configuration;

        /* We must claim the interface before doing anything */
        r = usb_claim_interface(m_handle, PEPERONI_IFACE_EP0);
        if (r < 0)
            qWarning() << "PeperoniDevice is unable to claim interface EP0!";

        /* Set DMX startcode */
        r = usb_control_msg(m_handle,
                            USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
                            PEPERONI_TX_STARTCODE,   // Set DMX startcode
                            0,                       // Standard startcode is 0
                            0,                       // No index
                            NULL,                    // No data
                            0,                       // Zero data length
                            50);                     // Timeout (ms)
        if (r < 0)
            qWarning() << "PeperoniDevice is unable to set 0 as the DMX startcode!";

        if (m_firmwareVersion >= PEPERONI_FW_OLD_BULK_SUPPORT)
        {
            /* Sometimes you need a little jolt to get the device on its feet. */
            r = usb_clear_halt(m_handle, PEPERONI_BULK_OUT_ENDPOINT);
            if (r < 0)
                qWarning() << "PeperoniDevice" << name() << "is unable to reset bulk endpoint.";
        }
    }
}

void PeperoniDevice::close()
{
    if (m_device != NULL && m_handle != NULL)
    {
        /* Release the interface in case we claimed it */
        int r = usb_release_interface(m_handle, PEPERONI_IFACE_EP0);
        if (r < 0)
        {
            qWarning() << "PeperoniDevice" << name()
                       << "is unable to release interface EP0!";
        }

        usb_close(m_handle);
    }

    m_handle = NULL;
}

const struct usb_device* PeperoniDevice::device() const
{
    return m_device;
}

const usb_dev_handle* PeperoniDevice::handle() const
{
    return m_handle;
}

/****************************************************************************
 * Write
 ****************************************************************************/

void PeperoniDevice::outputDMX(const QByteArray& universe)
{
    int r = -1;

    if (m_handle == NULL)
        return;

    /* Choose write method based on firmware version. One has to unplug
       and then re-plug the dongle in apple for bulk write to work,
       so disable it for apple, since control msg should work for all. */
#if !defined(__APPLE__) && !defined(Q_OS_MAC)
    if (m_firmwareVersion < PEPERONI_FW_OLD_BULK_SUPPORT)
    {
#endif
        qDebug() << "[Peperoni] control pipe write. Mode:" << m_blockingControlWrite << ", size:" << universe.size();
        r = usb_control_msg(m_handle,
                            USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
                            PEPERONI_TX_MEM_REQUEST, // We are WRITING DMX data
                            m_blockingControlWrite,  // Block during frame send?
                            0,                       // Start at DMX address 0
                            (char*) universe.data(), // The DMX universe data
                            universe.size(),         // Size of DMX universe
                            50);                     // Timeout (ms)

        if (r < 0)
            qWarning() << "PeperoniDevice" << name() << "failed control write:" << usb_strerror();
#if !defined(__APPLE__) && !defined(Q_OS_MAC)
    }
    else
    {
        qDebug() << "Old bulk pipe write. Size:" << universe.size();
        /* Construct a bulk header first */
        m_bulkBuffer.clear();
        m_bulkBuffer.append(char(PEPERONI_OLD_BULK_HEADER_ID));
        m_bulkBuffer.append(char(PEPERONI_OLD_BULK_HEADER_REQUEST_TX_SET));
        m_bulkBuffer.append(char(universe.size() & 0xFF));
        m_bulkBuffer.append(char((universe.size() >> 8) & 0xFF));

        /* Append universe data to the bulk buffer */
        m_bulkBuffer.append(universe);

        /* Perform a bulk write */
        r = usb_bulk_write(m_handle,
                           PEPERONI_BULK_OUT_ENDPOINT,
                           m_bulkBuffer.data(),
                           m_bulkBuffer.size(),
                           50);

        if (r < 0)
        {
            qWarning() << "PeperoniDevice" << name() << "failed 'old' bulk write:" << usb_strerror();
            qWarning() << "Resetting bulk endpoint.";
            r = usb_clear_halt(m_handle, PEPERONI_BULK_OUT_ENDPOINT);
            if (r < 0)
                qWarning() << "PeperoniDevice" << name() << "is unable to reset bulk endpoint.";
        }
    }
#endif
}
