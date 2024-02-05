/*
  Q Light Controller
  peperonidevice.cpp

  Copyright (c) Heikki Junnila
                Massimo Callegari

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

#include "peperonidevice.h"
#include "peperonidefs.h"
#include "peperoni.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

PeperoniDevice::PeperoniDevice(Peperoni* parent, struct libusb_device* device,
                               struct libusb_device_descriptor* desc,
                               quint32 line)
    : QThread(parent)
    , m_baseLine(line)
    , m_device(device)
    , m_handle(NULL)
    , m_descriptor(desc)
{
    Q_ASSERT(device != NULL);

    /* Store fw version so we don't need to rely on libusb's volatile data */
    m_firmwareVersion = desc->bcdDevice;
    qDebug() << "[Peperoni] detected device firmware version:" << QString::number(m_firmwareVersion, 16);

    m_operatingModes[line] = CloseMode;
    if (desc->idProduct == PEPERONI_PID_USBDMX21)
        m_operatingModes[line + 1] = CloseMode;

    extractName();
}

PeperoniDevice::~PeperoniDevice()
{
    closeAll();
}

/****************************************************************************
 * Device information
 ****************************************************************************/

bool PeperoniDevice::isPeperoniDevice(const struct libusb_device_descriptor* desc)
{
    /* If there's nothing to inspect, it can't be what we're looking for */
    if (desc == NULL)
        return false;

    /* If it's not manufactured by them, we're not interested in it */
    if (!isPeperoniDevice(desc->idVendor, desc->idProduct))
        return false;

    /* We need one interface */
    if (desc->bNumConfigurations < 1)
        return false;

    return true;
}

bool PeperoniDevice::isPeperoniDevice(int vid, int pid)
{
    if (vid != PEPERONI_VID)
        return false;
    if (pid == PEPERONI_PID_RODIN1     ||
        pid == PEPERONI_PID_RODIN1_MK3 ||
        pid == PEPERONI_PID_RODIN2     ||
        pid == PEPERONI_PID_RODINT     ||
        pid == PEPERONI_PID_XSWITCH    ||
        pid == PEPERONI_PID_USBDMX21)
            return true;
    return false;
}

int PeperoniDevice::outputsNumber(libusb_device_descriptor *desc)
{
    /* If there's nothing to inspect, it can't be what we're looking for */
    if (desc == NULL)
        return 0;

    /* If it's not manufactured by them, we're not interested in it */
    if (desc->idVendor != PEPERONI_VID)
        return 0;

    if (desc->idProduct == PEPERONI_PID_USBDMX21)
    {
        return 2;
    }
    else if (desc->idProduct == PEPERONI_PID_RODIN1     ||
             desc->idProduct == PEPERONI_PID_RODIN1_MK3 ||
             desc->idProduct == PEPERONI_PID_RODIN2     ||
             desc->idProduct == PEPERONI_PID_RODINT     ||
             desc->idProduct == PEPERONI_PID_XSWITCH)
    {
        return 1;
    }

    return 0;
}

void PeperoniDevice::extractName()
{
    Q_ASSERT(m_device != NULL);

    libusb_device_handle *handle = NULL;
    int r = libusb_open(m_device, &handle);
    if (r == 0)
    {
        char buf[256];
        int len = 0;

        /* Extract the name */
        len = libusb_get_string_descriptor_ascii(handle, m_descriptor->iProduct,
                                               (uchar*) &buf, sizeof(buf));
        if (len > 0)
        {
            m_name = QString(QByteArray(buf, len));
        }
        else
        {
            m_name = tr("Unknown");
            qWarning() << "Unable to get product name:" << len;
        }
/*
        // UID
        len = libusb_get_string_descriptor_ascii(handle, m_descriptor->iSerialNumber,
                                               (uchar*) &buf, sizeof(buf));
        if (len > 0)
            m_serial = QString(QByteArray(buf, len));
        else
            qWarning() << "Unable to get device serial:" << len;
*/
    }
    libusb_close(handle);
}

QString PeperoniDevice::name(quint32 line) const
{
    if (m_descriptor->idProduct == PEPERONI_PID_USBDMX21)
        return QString("%1 - %2 %3").arg(m_name).arg(tr("Universe")).arg((line - m_baseLine) + 1);

    return m_name;
}

QString PeperoniDevice::baseInfoText(quint32 line) const
{
    QString info;

    if (m_device != NULL)
    {
        info += QString("<B>%1</B>").arg(name(line));
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

QString PeperoniDevice::inputInfoText(quint32 line) const
{
    QString info;

    if (m_device != NULL)
    {
        info += QString("<B>%1:</B> ").arg(tr("Input line"));
        if (m_operatingModes[line] & InputMode)
            info += QString("%1").arg(tr("Open"));
        else
            info += QString("%1").arg(tr("Close"));
        info += QString("<BR>");
    }

    return info;
}

QString PeperoniDevice::outputInfoText(quint32 line) const
{
    QString info;

    if (m_device != NULL)
    {
        info += QString("<B>%1:</B> ").arg(tr("Output line"));
        if (m_operatingModes[line] & OutputMode)
            info += QString("%1").arg(tr("Open"));
        else
            info += QString("%1").arg(tr("Close"));
        info += QString("<BR>");
    }

    return info;
}

/****************************************************************************
 * Open & close
 ****************************************************************************/

bool PeperoniDevice::open(quint32 line, OperatingMode mode)
{
    m_operatingModes[line] |= mode;

    if (m_device != NULL && m_handle == NULL)
    {
        int r = -1;
        int configuration = PEPERONI_CONF_TXRX;

        r = libusb_open(m_device, &m_handle);
        if (r < 0)
        {
            qWarning() << "Unable to open PeperoniDevice with idProduct:" << m_descriptor->idProduct;
            m_handle = NULL;
            return false;
        }

        /* Use configuration #2 on X-Switch */
        if (m_descriptor->idProduct == PEPERONI_PID_XSWITCH)
            configuration = PEPERONI_CONF_TXRX;
        else
            configuration = PEPERONI_CONF_TXONLY;

        /* Set selected configuration */
        r = libusb_set_configuration(m_handle, configuration);
        if (r < 0)
            qWarning() << "PeperoniDevice is unable to set configuration #" << configuration;

        /* We must claim the interface before doing anything */
        r = libusb_claim_interface(m_handle, PEPERONI_IFACE_EP0);
        if (r < 0)
            qWarning() << "PeperoniDevice is unable to claim interface EP0!";

        /* Set TX DMX startcode */
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
            qWarning() << "PeperoniDevice is unable to set 0 as the DMX output startcode!";

        /* Set RX DMX startcode */
        r = libusb_control_transfer(m_handle,
                            LIBUSB_REQUEST_TYPE_VENDOR |
                            LIBUSB_RECIPIENT_INTERFACE |
                            LIBUSB_ENDPOINT_IN,
                            PEPERONI_RX_STARTCODE,   // Set DMX startcode
                            0,                       // Standard startcode is 0
                            0,                       // No index
                            NULL,                    // No data
                            0,                       // Zero data length
                            50);                     // Timeout (ms)
        if (r < 0)
            qWarning() << "PeperoniDevice is unable to set 0 as the DMX input startcode!";

        if (m_firmwareVersion >= PEPERONI_FW_OLD_BULK_SUPPORT)
        {
            /* Sometimes you need a little jolt to get the device on its feet. */
            r = libusb_clear_halt(m_handle, PEPERONI_BULK_OUT_ENDPOINT);
            if (r < 0)
                qWarning() << "PeperoniDevice" << name(m_baseLine) << "is unable to reset bulk OUT endpoint.";
            r = libusb_clear_halt(m_handle, PEPERONI_BULK_IN_ENDPOINT);
            if (r < 0)
                qWarning() << "PeperoniDevice" << name(m_baseLine) << "is unable to reset bulk IN endpoint.";
        }
    }

    if (m_operatingModes[line] & InputMode && m_running == false)
    {
        qDebug() << "[Peperoni] open input line:" << m_baseLine;
        m_dmxInputBuffer.clear();
        m_dmxInputBuffer.fill(0, 512);
        m_running = true;
        start();
    }
    return true;
}

void PeperoniDevice::close(quint32 line, OperatingMode mode)
{
    m_operatingModes[line] &= ~mode;

    if (mode == InputMode && m_running == true)
    {
        m_running = false;
        wait();
    }

    if (m_operatingModes[line] != CloseMode)
        return;

    QMutexLocker lock(&m_ioMutex);

    if (m_device != NULL && m_handle != NULL)
    {
        /* Release the interface in case we claimed it */
        int r = libusb_release_interface(m_handle, PEPERONI_IFACE_EP0);
        if (r < 0)
        {
            qWarning() << "PeperoniDevice" << name(m_baseLine)
                       << "is unable to release interface EP0!";
        }

        libusb_close(m_handle);
    }

    m_handle = NULL;
}

void PeperoniDevice::closeAll()
{
    qDebug() << "[Peperoni] close input...";
    close(m_baseLine, InputMode);
    qDebug() << "[Peperoni] close output...";
    close(m_baseLine, OutputMode);
}

const struct libusb_device* PeperoniDevice::device() const
{
    return m_device;
}


/****************************************************************************
 * Input Thread
 ****************************************************************************/

void PeperoniDevice::run()
{
    if (m_device == NULL)
        return;

    qDebug() << "[Peperoni] input thread started correctly";

    while (m_running == true)
    {
        QByteArray tmpBuffer(512, 0);

        /* Choose write method based on firmware version. One has to unplug
           and then re-plug the dongle in apple for bulk write to work,
           so disable it for apple, since control msg should work for all. */
#if !defined(__APPLE__) && !defined(Q_OS_MAC)
        if (1 || m_firmwareVersion < PEPERONI_FW_NEW_BULK_SUPPORT)
        {
#endif
            unsigned short rxslots;
            unsigned char  rxstartcode;
            unsigned int   block;

            // read memory blocking if firmware is >= 0x0500
            if (m_firmwareVersion >= PEPERONI_FW_NEW_BULK_SUPPORT)
            {
                block = 1;
            }
            else
            {
                block = 0;
                // if we don't block sleep for 10ms
                usleep(10000);
            }

            {
            int r = -1;
            QMutexLocker lock(&m_ioMutex);

            r = libusb_control_transfer(m_handle,
                                LIBUSB_REQUEST_TYPE_VENDOR |
                                LIBUSB_RECIPIENT_INTERFACE |
                                LIBUSB_ENDPOINT_IN,
                                PEPERONI_RX_MEM_REQUEST,   // We are READING DMX data
                                block,                     // Blocking does not work on all firmware versions -> don't block
                                0,                         // Start at DMX address 0
                                (uchar *)tmpBuffer.data(), // The DMX universe data
                                tmpBuffer.size(),          // Size of DMX universe
                                100);                      // Timeout (ms)

            if (r < 0)
            {
                qWarning() << "PeperoniDevice" << name(m_baseLine) << "failed control_msg:" << libusb_strerror(libusb_error(r));
                r = libusb_clear_halt(m_handle, PEPERONI_BULK_IN_ENDPOINT);
                if (r < 0)
                    qWarning() << "PeperoniDevice" << name(m_baseLine) << "is unable to reset bulk IN endpoint.";
            }
            else
            {
                /* read received startcode */
                r = libusb_control_transfer(m_handle,
                                    LIBUSB_REQUEST_TYPE_VENDOR |
                                    LIBUSB_RECIPIENT_INTERFACE |
                                    LIBUSB_ENDPOINT_IN,
                                    PEPERONI_RX_STARTCODE,
                                    0,
                                    0,
                                    (uchar *)&rxstartcode,
                                    sizeof(rxstartcode),
                                    10);
                if (r < 0)
                    qWarning() << "PeperoniDevice" << name(m_baseLine) << "failed to read receiver startcode:" << libusb_strerror(libusb_error(r));
                else
                {
                    /* read number of received slots */
                    r = libusb_control_transfer(m_handle,
                                        LIBUSB_REQUEST_TYPE_VENDOR |
                                        LIBUSB_RECIPIENT_INTERFACE |
                                        LIBUSB_ENDPOINT_IN,
                                        PEPERONI_RX_SLOTS,
                                        0,
                                        0,
                                        (uchar *)&rxslots,
                                        sizeof(rxslots),
                                        10);
                    if (r < 0)
                        qWarning() << "PeperoniDevice" << name(m_baseLine) << "failed to read receiver slot counter:" << libusb_strerror(libusb_error(r));
                    else
                    {
                        if (rxslots > m_dmxInputBuffer.size())
                        {
                            rxslots = m_dmxInputBuffer.size();
                            qWarning() << "PeperoniDevice" << name(m_baseLine) << "input frame too long, truncated";
                        }

                        //qDebug() << "[Peperoni] input frame has startcode" << rxstartcode << "and is" << rxslots << "slots long";

                        /* only accept DMX512 data */
                        if (rxstartcode == 0)
                        {
                            for (int i = 0; i < rxslots; i++)
                            {
                                if (tmpBuffer.at(i) != m_dmxInputBuffer.at(i))
                                {
                                    emit valueChanged(UINT_MAX, m_baseLine, i, tmpBuffer.at(i));
                                    m_dmxInputBuffer[i] = tmpBuffer.at(i);
                                }
                            }
                        }
                    }
                }
            }
            }
#if !defined(__APPLE__) && !defined(Q_OS_MAC)
        }
        else
        {
            //TODO
        }
#endif
    }
}

/****************************************************************************
 * Write
 ****************************************************************************/

void PeperoniDevice::outputDMX(quint32 line, const QByteArray& universe)
{
    Q_UNUSED(line)
    int r = -1;

    if (m_handle == NULL)
        return;

    {
    QMutexLocker lock(&m_ioMutex);
    /* Choose write method based on firmware version. One has to unplug
       and then re-plug the dongle in apple for bulk write to work,
       so disable it for apple, since control msg should work for all. */
#if !defined(__APPLE__) && !defined(Q_OS_MAC)
    if (m_firmwareVersion < PEPERONI_FW_OLD_BULK_SUPPORT)
    {
#endif
        //qDebug() << "[Peperoni] control pipe write. size:" << universe.size();
        r = libusb_control_transfer(m_handle,
                            LIBUSB_REQUEST_TYPE_VENDOR |
                            LIBUSB_RECIPIENT_INTERFACE |
                            LIBUSB_ENDPOINT_OUT,
                            PEPERONI_TX_MEM_REQUEST,  // We are WRITING DMX data
                            0,                        // Blocking does not work on all firmware versions -> don't block
                            0,                        // Start at DMX address 0
                            (uchar*) universe.data(), // The DMX universe data
                            universe.size(),          // Size of DMX universe
                            50);                      // Timeout (ms)

        if (r < 0)
            qWarning() << "PeperoniDevice" << name(m_baseLine) << "failed control write:" << libusb_strerror(libusb_error(r));

#if !defined(__APPLE__) && !defined(Q_OS_MAC)
    }
    else if (m_firmwareVersion < PEPERONI_FW_NEW_BULK_SUPPORT)
    {
        char requestType = char(PEPERONI_OLD_BULK_HEADER_REQUEST_TX_SET);
        if (line - m_baseLine == 1)
            requestType = PEPERONI_OLD_BULK_HEADER_REQUEST_TX2_SET;

        //qDebug() << "Old bulk pipe write. Size:" << universe.size();
        /* Construct a bulk header first */
        m_bulkBuffer.clear();
        m_bulkBuffer.append(char(PEPERONI_OLD_BULK_HEADER_ID));
        m_bulkBuffer.append(requestType);
        m_bulkBuffer.append(char(0x00)); // 512 - LSB
        m_bulkBuffer.append(char(0x02)); // 512 - MSB

        /* Append universe data to the bulk buffer */
        m_bulkBuffer.append(universe);
        /* Append trailing zeros to reach size of 512 bytes */
        m_bulkBuffer.append(QByteArray(int(512 - universe.size()), char(0)));

        /* Perform a bulk write */
        int written = 0;
        r = libusb_bulk_transfer(m_handle,
                                 PEPERONI_BULK_OUT_ENDPOINT,
                                 (uchar *)m_bulkBuffer.data(),
                                 m_bulkBuffer.size(),
                                 &written,
                                 50);

        if (r < 0)
        {
            qWarning() << "PeperoniDevice" << name(m_baseLine) << "failed 'old' bulk write:" << libusb_strerror(libusb_error(r));
            qWarning() << "Resetting bulk endpoint.";
            r = libusb_clear_halt(m_handle, PEPERONI_BULK_OUT_ENDPOINT);
            if (r < 0)
                qWarning() << "PeperoniDevice" << name(m_baseLine) << "is unable to reset bulk endpoint.";
        }
    }
    else
    {
        //qDebug() << "New bulk pipe write. Size:" << universe.size();

        m_bulkBuffer.clear();
        /* Construct a bulk command header first */
        m_bulkBuffer.append(char(PEPERONI_NEW_BULK_HEADER_ID1));
        m_bulkBuffer.append(char(PEPERONI_NEW_BULK_HEADER_ID2));
        m_bulkBuffer.append(char(PEPERONI_NEW_BULK_HEADER_ID3));
        m_bulkBuffer.append(char(PEPERONI_NEW_BULK_HEADER_ID4));
        m_bulkBuffer.append(char(PEPERONI_NEW_BULK_HEADER_REQUEST_SET));
        /** universe number: Rodins only support index 0, DMX21 supports 0 and 1 */
        m_bulkBuffer.append(char(line - m_baseLine));

        m_bulkBuffer.append(char(0x07)); /** lenght of data stage (512 + 7 bytes), LSB */
        m_bulkBuffer.append(char(0x02)); /** length of data state (512 + 7 bytes), MSB */
        m_bulkBuffer.append(char(PEPERONI_NEW_BULK_CONFIG_BLOCK      /** transmitter configuration: block while transmitter is busy */
                               | PEPERONI_NEW_BULK_CONFIG_FORCETX)); /** transmitter configuration: force transmission */
        m_bulkBuffer.append(char(50));  /** timeout LSB, 50ms */
        m_bulkBuffer.append(char(0));   /** timeout MSB, 50ms */
        m_bulkBuffer.append(char(181)); /** length of break, 200 us */
        m_bulkBuffer.append(char(250)); /** length of Mark after Break, 20us */

        /* Construct a bulk data state header */
        m_bulkBuffer.append(char(PEPERONI_NEW_BULK_HEADER_ID1));
        m_bulkBuffer.append(char(PEPERONI_NEW_BULK_HEADER_ID2));
        m_bulkBuffer.append(char(PEPERONI_NEW_BULK_HEADER_ID3));
        m_bulkBuffer.append(char(PEPERONI_NEW_BULK_HEADER_ID4));
        m_bulkBuffer.append(char(0x01));  /** number of bytes to send, incl. startcode, LSB */
        m_bulkBuffer.append(char(0x02));  /** number of bytes to send, incl. startcode, MSB */
        m_bulkBuffer.append(char(0));     /** internal buffer comes without startcode -> insert it */

        /* Append universe data to the bulk buffer */
        m_bulkBuffer.append(universe);

        /* Append trailing zeros to reach size of 512 bytes */
        m_bulkBuffer.append(QByteArray(int(512 - universe.size()), char(0)));

        /* Perform a bulk write */
        int written = 0;
        r = libusb_bulk_transfer(m_handle,
                                 PEPERONI_BULK_OUT_ENDPOINT,
                                 (uchar *)m_bulkBuffer.data(),
                                 m_bulkBuffer.size(),
                                 &written,
                                 100); /* use larger timeout then specified in the command header above */
        if (r < 0)
        {
            qWarning() << "PeperoniDevice" << name(m_baseLine) << "failed 'new' bulk write:" << libusb_strerror(libusb_error(r));
        }
        else
        {
            char status[8];
            int read = 0;
            r = libusb_bulk_transfer(m_handle,
                                     PEPERONI_BULK_IN_ENDPOINT,
                                     (uchar *)status,
                                     sizeof(status),
                                     &read,
                                     100); /* use larger timeout then specified in the command header above */
            if (r < 0)
                qWarning() << "PeperoniDevice" << name(m_baseLine) << "failed 'new' bulk read:" << libusb_strerror(libusb_error(r));
        }

        if (r < 0)
        {
            qWarning() << "Resetting bulk endpoints.";
            r = libusb_clear_halt(m_handle, PEPERONI_BULK_OUT_ENDPOINT);
            if (r < 0)
                qWarning() << "PeperoniDevice" << name(m_baseLine) << "is unable to reset bulk OUT endpoint.";

            r = libusb_clear_halt(m_handle, PEPERONI_BULK_IN_ENDPOINT);
            if (r < 0)
                qWarning() << "PeperoniDevice" << name(m_baseLine) << "is unable to reset bulk IN endpoint.";
        }
        else
        {
            /** optionally analyse status buffer: ID (4 bytes), Timestamp (2 bytes, ms), status (1 byte), unused (1 byte) */
        }
    }
#endif
    }
}
