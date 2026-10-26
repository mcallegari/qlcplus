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
#include <climits>
#include <libusb.h>

#include "peperonidevice.h"
#include "peperonidefs.h"
#include "peperoni.h"

namespace
{
constexpr unsigned char makeVendorInterfaceRequestType(unsigned char endpointDirection)
{
    return static_cast<unsigned char>(static_cast<unsigned char>(LIBUSB_REQUEST_TYPE_VENDOR) |
                                      static_cast<unsigned char>(LIBUSB_RECIPIENT_INTERFACE) |
                                      endpointDirection);
}

constexpr unsigned char kVendorInterfaceOutRequestType =
    makeVendorInterfaceRequestType(static_cast<unsigned char>(LIBUSB_ENDPOINT_OUT));
constexpr unsigned char kVendorInterfaceInRequestType =
    makeVendorInterfaceRequestType(static_cast<unsigned char>(LIBUSB_ENDPOINT_IN));
}

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
    , m_descriptor(*desc)
    , m_running(false)
{
    Q_ASSERT(device != NULL);

    /* Keep the libusb device alive for as long as this object exists,
       so the pointer stays valid after the device list is freed */
    libusb_ref_device(m_device);

    /* Store fw version so we don't need to rely on libusb's volatile data */
    m_firmwareVersion = desc->bcdDevice;
    qDebug() << "[Peperoni] detected device firmware version:" << QString::number(m_firmwareVersion, 16);

    for (int i = 0; i < linesCount(); i++)
        m_operatingModes[i] = CloseMode;

    extractName();
}

PeperoniDevice::~PeperoniDevice()
{
    closeAll();
    libusb_unref_device(m_device);
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

int PeperoniDevice::outputsNumber(const libusb_device_descriptor *desc)
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

int PeperoniDevice::linesCount() const
{
    return outputsNumber(&m_descriptor);
}

quint32 PeperoniDevice::baseLine() const
{
    return m_baseLine;
}

void PeperoniDevice::setBaseLine(quint32 line)
{
    m_baseLine = line;
}

quint32 PeperoniDevice::lineIndex(quint32 line) const
{
    return line - m_baseLine;
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
        len = libusb_get_string_descriptor_ascii(handle, m_descriptor.iProduct,
                                               (uchar*) &buf, sizeof(buf));
        if (len > 0)
        {
            m_name = QString(QByteArray(buf, len));
        }
        else
        {
            m_name = modelName();
            qWarning() << "Unable to get product name:" << len;
        }
/*
        // UID
        len = libusb_get_string_descriptor_ascii(handle, m_descriptor.iSerialNumber,
                                               (uchar*) &buf, sizeof(buf));
        if (len > 0)
            m_serial = QString(QByteArray(buf, len));
        else
            qWarning() << "Unable to get device serial:" << len;
*/
        libusb_close(handle);
    }
    else
    {
        m_name = modelName();
        qWarning() << "[Peperoni] Unable to open" << m_name << ":" << libusb_strerror(libusb_error(r));
#if defined(WIN32) || defined(Q_OS_WIN)
        if (r == LIBUSB_ERROR_NOT_SUPPORTED || r == LIBUSB_ERROR_ACCESS)
            qWarning() << "[Peperoni] The device is not bound to the WinUSB driver."
                       << "Update the device firmware (>= 0x1000) or install WinUSB with Zadig.";
#endif
    }
}

QString PeperoniDevice::modelName() const
{
    switch (m_descriptor.idProduct)
    {
        case PEPERONI_PID_XSWITCH:    return QString("X-Switch");
        case PEPERONI_PID_RODIN1:     return QString("Rodin 1");
        case PEPERONI_PID_RODIN1_MK3: return QString("Rodin 1 MK3");
        case PEPERONI_PID_RODIN2:     return QString("Rodin 2");
        case PEPERONI_PID_RODINT:     return QString("Rodin T");
        case PEPERONI_PID_USBDMX21:   return QString("USBDMX21");
        default:                      return tr("Unknown");
    }
}

QString PeperoniDevice::name(quint32 line) const
{
    if (m_descriptor.idProduct == PEPERONI_PID_USBDMX21)
        return QString("%1 - %2 %3").arg(m_name).arg(tr("Universe")).arg(lineIndex(line) + 1);

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
        if (m_operatingModes.value(lineIndex(line)) & InputMode)
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
        if (m_operatingModes.value(lineIndex(line)) & OutputMode)
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
    quint32 index = lineIndex(line);
    if (m_device == NULL || m_operatingModes.contains(index) == false)
        return false;

    if (m_handle == NULL)
    {
        QMutexLocker lock(&m_ioMutex);

        int r = libusb_open(m_device, &m_handle);
        if (r < 0)
        {
            qWarning() << "Unable to open PeperoniDevice with idProduct:" << m_descriptor.idProduct
                       << libusb_strerror(libusb_error(r));
            m_handle = NULL;
            return false;
        }

        /* Use configuration #2 on X-Switch */
        int configuration = (m_descriptor.idProduct == PEPERONI_PID_XSWITCH) ?
                                PEPERONI_CONF_TXRX : PEPERONI_CONF_TXONLY;

        /* Set selected configuration, only if needed: changing it is not
           supported by the WinUSB driver on Windows (stuck on #1) */
        int current = 0;
        if (libusb_get_configuration(m_handle, &current) < 0 || current != configuration)
        {
            r = libusb_set_configuration(m_handle, configuration);
            if (r < 0)
                qWarning() << "PeperoniDevice is unable to set configuration #" << configuration
                           << libusb_strerror(libusb_error(r));
        }

        /* We must claim the interface before doing anything */
        r = libusb_claim_interface(m_handle, PEPERONI_IFACE_EP0);
        if (r < 0)
            qWarning() << "PeperoniDevice is unable to claim interface EP0!";

        /* Set TX DMX startcode */
        r = libusb_control_transfer(m_handle,
                            kVendorInterfaceOutRequestType,
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
                            kVendorInterfaceInRequestType,
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

    /* Mark the line as open only once the device is actually usable */
    m_operatingModes[index] |= mode;

    if (m_operatingModes[index] & InputMode && m_running == false)
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
    quint32 index = lineIndex(line);
    if (m_operatingModes.contains(index) == false)
        return;

    m_operatingModes[index] &= ~mode;

    if (mode == InputMode && m_running == true)
    {
        m_running = false;
        wait();
    }

    /* The USB handle is shared by all the lines of this device
       (USBDMX21 has two), so keep it open while any line still uses it */
    foreach (int lineMode, m_operatingModes)
    {
        if (lineMode != CloseMode)
            return;
    }

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
    for (int i = 0; i < linesCount(); i++)
    {
        qDebug() << "[Peperoni] close input and output of line" << m_baseLine + i;
        close(m_baseLine + i, InputMode);
        close(m_baseLine + i, OutputMode);
    }
}

const libusb_device* PeperoniDevice::device() const
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
                                kVendorInterfaceInRequestType,
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
                                    kVendorInterfaceInRequestType,
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
                                        kVendorInterfaceInRequestType,
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
    int r = -1;

    {
    QMutexLocker lock(&m_ioMutex);

    /* Check under the lock, since close() may release the handle */
    if (m_handle == NULL)
        return;
    /* Choose write method based on firmware version. One has to unplug
       and then re-plug the dongle in apple for bulk write to work,
       so disable it for apple, since control msg should work for all. */
#if !defined(__APPLE__) && !defined(Q_OS_MAC)
    if (m_firmwareVersion < PEPERONI_FW_OLD_BULK_SUPPORT)
    {
#endif
        //qDebug() << "[Peperoni] control pipe write. size:" << universe.size();
        r = libusb_control_transfer(m_handle,
                            kVendorInterfaceOutRequestType,
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
        if (lineIndex(line) == 1)
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
        m_bulkBuffer.append(char(lineIndex(line)));

        m_bulkBuffer.append(char(0x07)); /** length of data stage (512 + 7 bytes), LSB */
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
