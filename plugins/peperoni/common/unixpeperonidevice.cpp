/*
  Q Light Controller
  unixpeperonidevice.cpp

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
#include <libusb.h>

#include "unixpeperonidevice.h"
#include "peperonidefs.h"

UnixPeperoniDevice::UnixPeperoniDevice(const QVariant& uid, const QString& name,
                                       struct libusb_device* device,
                                       const struct libusb_device_descriptor* desc,
                                       QObject* parent)
    : OutputDevice(uid, name, parent)
    , m_device(device)
    , m_handle(NULL)
    , m_firmwareVersion(desc->bcdDevice)
    , m_configuration(PEPERONI_CONF_TXONLY)
{
    Q_ASSERT(device != NULL);
    Q_ASSERT(desc != NULL);

    libusb_ref_device(m_device);

    if (m_firmwareVersion < PEPERONI_FW_BLOCKING_WRITE_SUPPORT)
        m_blockingControlWrite = PEPERONI_TX_MEM_NONBLOCK;
    else
        m_blockingControlWrite = PEPERONI_TX_MEM_BLOCK;

    /* Use configuration #2 on X-Switch */
    if (desc->idProduct == PEPERONI_PID_XSWITCH)
        m_configuration = PEPERONI_CONF_TXRX;

}

UnixPeperoniDevice::~UnixPeperoniDevice()
{
    close();

    libusb_unref_device(m_device);
    m_device = NULL;
}

void UnixPeperoniDevice::open()
{
    qDebug() << Q_FUNC_INFO;

    if (isOpen() == true)
        return;

    int r = libusb_open(m_device, &m_handle);
    if (r < 0)
    {
        qWarning() << "Unable to open device:" << r;
        m_handle = NULL;
        return;
    }

    /* Set selected configuration */
    r = libusb_set_configuration(m_handle, m_configuration);
    if (r < 0)
        qWarning() << "Unable to set configuration #" << m_configuration << ":" << r;

    /* We must claim the interface before doing anything */
    r = libusb_claim_interface(m_handle, PEPERONI_IFACE_EP0);
    if (r < 0)
        qWarning() << "Unable to claim interface EP0:" << r;

    /* Set DMX startcode */
    r = libusb_control_transfer(m_handle,
                        LIBUSB_REQUEST_TYPE_VENDOR |
                            LIBUSB_RECIPIENT_INTERFACE |
                            LIBUSB_ENDPOINT_OUT,
                        PEPERONI_TX_STARTCODE,   // Set DMX startcode
                        0,                       // Standard startcode is 0
                        0,                       // No index
                        NULL,                    // No data
                        0,                       // Zero data length
                        50);                     // Timeout (ms)
    if (r < 0)
        qWarning() << "Unable to set 0 as the DMX startcode:" << r;

    if (m_firmwareVersion >= PEPERONI_FW_BULK_SUPPORT)
    {
        /* Allocate space for bulk buffer */
        m_bulkBuffer = QByteArray(512 + PEPERONI_OLD_BULK_HEADER_SIZE, 0);

        /* Sometimes you need a little jolt to get the device on its feet. */
        r = libusb_clear_halt(m_handle, PEPERONI_BULK_OUT_ENDPOINT);
        if (r < 0)
            qWarning() << "Unable to reset bulk endpoint:" << r;
    }
}

void UnixPeperoniDevice::close()
{
    qDebug() << Q_FUNC_INFO;

    if (isOpen() == false)
        return;

    /* Release the interface in case we claimed it */
    int r = libusb_release_interface(m_handle, PEPERONI_IFACE_EP0);
    if (r < 0)
        qWarning() << "Unable to release interface EP0:" << r;

    libusb_close(m_handle);
    m_handle = NULL;
}

bool UnixPeperoniDevice::isOpen() const
{
    if (m_handle != NULL)
        return true;
    else
        return false;
}

void UnixPeperoniDevice::writeChannel(ushort channel, uchar value)
{
    Q_UNUSED(channel);
    Q_UNUSED(value);
}

void UnixPeperoniDevice::writeUniverse(const QByteArray& universe)
{
    if (m_handle == NULL)
        return;

    int r = -1;

    /* Choose write method based on firmware version. One has to unplug
       and then re-plug the dongle in apple for bulk write to work,
       so disable it for apple, since control msg should work for all. */
#if !defined(__APPLE__) && !defined(Q_OS_MAC)
    if (m_firmwareVersion < PEPERONI_FW_BULK_SUPPORT)
    {
#endif
        r = libusb_control_transfer(m_handle,
                                    LIBUSB_REQUEST_TYPE_VENDOR |
                                        LIBUSB_RECIPIENT_INTERFACE |
                                        LIBUSB_ENDPOINT_OUT,
                                    PEPERONI_TX_MEM_REQUEST, // We are WRITING DMX data
                                    m_blockingControlWrite,  // Block during frame send?
                                    0,                       // Start at DMX address 0
                                    (uchar*) universe.data(), // The DMX universe data
                                    universe.size(),         // Size of DMX universe
                                    50);                     // Timeout (ms)

        if (r < 0)
            qWarning() << Q_FUNC_INFO << "Failed control transfer:" << r;
#if !defined(__APPLE__) && !defined(Q_OS_MAC)
    }
    else
    {
        /* Construct a bulk header first */
        m_bulkBuffer[0] = char(PEPERONI_OLD_BULK_HEADER_ID);
        m_bulkBuffer[1] = char(PEPERONI_OLD_BULK_HEADER_REQUEST_TX);
        m_bulkBuffer[2] = char(universe.size() & 0xFF);
        m_bulkBuffer[3] = char((universe.size() >> 8) & 0xFF);

        /* Append universe data to the bulk buffer */
        m_bulkBuffer.replace(PEPERONI_OLD_BULK_HEADER_SIZE,
                             universe.size(), universe);

        /* Perform a bulk write */
        int written = 0;
        r = libusb_bulk_transfer(m_handle,
                                 PEPERONI_BULK_OUT_ENDPOINT,
                                 (uchar*) m_bulkBuffer.constData(),
                                 m_bulkBuffer.size(),
                                 &written,
                                 50);

        if (r < 0)
        {
            qWarning() << Q_FUNC_INFO << "Failed bulk write:" << r;
            qWarning() << Q_FUNC_INFO << "Resetting bulk endpoint.";
            r = libusb_clear_halt(m_handle, PEPERONI_BULK_OUT_ENDPOINT);
            if (r < 0)
                qWarning() << Q_FUNC_INFO << "Unable to reset bulk endpoint:" << r;
        }
    }
#endif
}
