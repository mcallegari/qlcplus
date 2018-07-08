/*
  Q Light Controller
  udmxdevice.cpp

  Copyright (c) Heikki Junnila
                Lutz Hillebrand

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

#if defined(WIN32) || defined(Q_OS_WIN)
#   include <Windows.h>
#   include "libusb_dyn.h"
#else
#   include <usb.h>
#endif

#include <QSettings>
#include <QDebug>
#include <QTime>
#include <cmath>

#include "udmxdevice.h"
#include "qlcmacros.h"

#define DMX_CHANNELS 512

#define UDMX_SHARED_VENDOR     0x16C0 /* VOTI */
#define UDMX_SHARED_PRODUCT    0x05DC /* Obdev's free shared PID */

#define UDMX_AVLDIY_D512_CLONE_VENDOR     0x03EB /* Atmel Corp. (Clone VID)*/
#define UDMX_AVLDIY_D512_CLONE_PRODUCT    0x8888 /* Clone PID */

#define UDMX_SET_CHANNEL_RANGE 0x0002 /* Command to set n channel values */

#define SETTINGS_FREQUENCY "udmx/frequency"
#define SETTINGS_CHANNELS "udmx/channels"

/****************************************************************************
 * Initialization
 ****************************************************************************/

UDMXDevice::UDMXDevice(struct usb_device* device, QObject* parent)
    : QThread(parent)
    , m_device(device)
    , m_handle(NULL)
    , m_running(false)
    , m_universe(QByteArray(DMX_CHANNELS, 0))
    , m_frequency(30)
    , m_granularity(Unknown)
{
    Q_ASSERT(device != NULL);

    QSettings settings;
    QVariant var = settings.value(SETTINGS_FREQUENCY);
    if (var.isValid() == true)
        m_frequency = var.toDouble();

    QVariant var2 = settings.value(SETTINGS_CHANNELS);
    if (var2.isValid() == true)
    {
        int channels = var2.toInt();
        if (channels > DMX_CHANNELS || channels <= 0)
            channels = DMX_CHANNELS;
        m_universe = QByteArray(channels, 0);
    }

    extractName();
}

UDMXDevice::~UDMXDevice()
{
    close();
}

/****************************************************************************
 * Device information
 ****************************************************************************/

bool UDMXDevice::isUDMXDevice(const struct usb_device* device)
{
    if (device == NULL)
        return false;

    if (device->descriptor.idVendor != UDMX_SHARED_VENDOR &&
        device->descriptor.idVendor != UDMX_AVLDIY_D512_CLONE_VENDOR)
            return false;

    if (device->descriptor.idProduct != UDMX_SHARED_PRODUCT &&
        device->descriptor.idProduct != UDMX_AVLDIY_D512_CLONE_PRODUCT)
            return false;

    return true;
}

void UDMXDevice::extractName()
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

QString UDMXDevice::name() const
{
    return m_name;
}

QString UDMXDevice::infoText() const
{
    QString info;
    QString gran;

    if (m_device != NULL && m_handle != NULL)
    {
        info += QString("<B>%1</B>").arg(name());
        info += QString("<P>");
        info += QString("<B>%1:</B> %2").arg(tr("DMX Channels")).arg(m_universe.size());
        info += QString("<BR>");
        info += QString("<B>%1:</B> %2Hz").arg(tr("DMX Frame Frequency")).arg(m_frequency);
        info += QString("<BR>");
        if (m_granularity == Bad)
            gran = QString("<FONT COLOR=\"#aa0000\">%1</FONT>").arg(tr("Bad"));
        else if (m_granularity == Good)
            gran = QString("<FONT COLOR=\"#00aa00\">%1</FONT>").arg(tr("Good"));
        else
            gran = tr("Patch this device to a universe to find out.");
        info += QString("<B>%1:</B> %2").arg(tr("System Timer Accuracy")).arg(gran);
        info += QString("</P>");
    }
    else
    {
        info += QString("<B>%1</B>").arg(tr("Unknown device"));
        info += QString("<P>");
        info += tr("Cannot connect to USB device.");
        info += QString("</P>");
    }

    return info;
}

/****************************************************************************
 * Open & close
 ****************************************************************************/

bool UDMXDevice::open()
{
    if (m_device != NULL && m_handle == NULL)
        m_handle = usb_open(m_device);

    if (m_handle == NULL)
        return false;

    start();

    return true;
}

void UDMXDevice::close()
{
    stop();

    if (m_device != NULL && m_handle != NULL)
        usb_close(m_handle);
    m_handle = NULL;
}

const struct usb_device* UDMXDevice::device() const
{
    return m_device;
}

const usb_dev_handle* UDMXDevice::handle() const
{
    return m_handle;
}

/****************************************************************************
 * Thread
 ****************************************************************************/

void UDMXDevice::outputDMX(const QByteArray& universe)
{
    m_universe.replace(0, MIN(universe.size(), m_universe.size()),
                       universe.constData(), MIN(universe.size(), m_universe.size()));
}

void UDMXDevice::stop()
{
    while (isRunning() == true)
    {
	// This may occur before the thread sets m_running,
	// so timeout and try again if necessary
        m_running = false;
        wait(100);
    }
}

void UDMXDevice::run()
{
    // One "official" DMX frame can take (1s/44Hz) = 23ms
    int frameTime = (int) floor(((double)1000 / m_frequency) + (double)0.5);
    int r = 0;

    // Wait for device to settle in case the device was opened just recently
    // Also measure, whether timer granularity is OK
    QTime time;
    time.start();
    usleep(1000);
    if (time.elapsed() > 3)
        m_granularity = Bad;
    else
        m_granularity = Good;

    m_running = true;
    while (m_running == true)
    {
        if (m_handle == NULL)
            goto framesleep;

        time.restart();

        /* Write all 512 channels */
        r = usb_control_msg(m_handle,
                            USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
                            UDMX_SET_CHANNEL_RANGE,   /* Command */
                            m_universe.size(),        /* Number of channels to set */
                            0,                        /* Starting index */
                            (char*)m_universe.data(), /* Values to set */
                            m_universe.size(),        /* Size of values */
                            500);                     /* Timeout 0.5s */
        if (r < 0)
            qWarning() << "uDMX: unable to write universe:" << usb_strerror();

framesleep:
        // Sleep for the remainder of the DMX frame time
        if (m_granularity == Good)
            while (time.elapsed() < frameTime) { usleep(1000); }
        else
            while (time.elapsed() < frameTime) { /* Busy sleep */ }
    }
}
