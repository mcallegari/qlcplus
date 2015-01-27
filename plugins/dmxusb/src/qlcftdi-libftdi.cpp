/*
  Q Light Controller
  qlcftdi-libftdi.cpp

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

#include <QSettings>
#include <QVariant>
#include <ftdi.h>
#include <QDebug>
#include <QMap>

#include "dmxusbwidget.h"
#include "enttecdmxusbpro.h"
#include "enttecdmxusbopen.h"
#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
#include "nanodmx.h"
#endif
#include "stageprofi.h"
#include "vinceusbdmx512.h"
#include "qlcftdi.h"

QLCFTDI::QLCFTDI(const QString& serial, const QString& name, const QString& vendor, quint32 id)
    : m_serial(serial)
    , m_name(name)
    , m_vendor(vendor)
    , m_id(id)
{
    bzero(&m_handle, sizeof(struct ftdi_context));
    ftdi_init(&m_handle);
}

QLCFTDI::~QLCFTDI()
{
    if (isOpen() == true)
        close();
    ftdi_deinit(&m_handle);
}

QString QLCFTDI::readLabel(ftdi_context *ftdi, char* name, char* serial, uchar label, int *ESTA_code)
{
    if (ftdi_usb_open_desc(ftdi, QLCFTDI::FTDIVID, QLCFTDI::FTDIPID, name, serial) < 0)
        return QString();

    if (ftdi_usb_reset(ftdi) < 0)
        return QString();

    if (ftdi_set_baudrate(ftdi, 250000) < 0)
        return QString();

    if (ftdi_set_line_property(ftdi, BITS_8, STOP_BIT_2, NONE) < 0)
        return QString();

    if (ftdi_setflowctrl(ftdi, SIO_DISABLE_FLOW_CTRL) < 0)
        return QString();

    QByteArray request;
    request.append(ENTTEC_PRO_START_OF_MSG);
    request.append(label);
    request.append(ENTTEC_PRO_DMX_ZERO); // data length LSB
    request.append(ENTTEC_PRO_DMX_ZERO); // data length MSB
    request.append(ENTTEC_PRO_END_OF_MSG);

    if (ftdi_write_data(ftdi, (uchar*) request.data(), request.size()) < 0)
    {
        qDebug() << Q_FUNC_INFO << "Cannot write data to device";
        return QString();
    }

    uchar *buffer = (uchar*) malloc(sizeof(uchar) * 40);
    Q_ASSERT(buffer != NULL);

    QByteArray array;
    usleep(300000); // give some time to the device to respond
    int read = ftdi_read_data(ftdi, buffer, 40);
    //qDebug() << Q_FUNC_INFO << "Data read: " << read;
    array = QByteArray::fromRawData((char*) buffer, read);

    if (array[0] != ENTTEC_PRO_START_OF_MSG)
        qDebug() << Q_FUNC_INFO << "Reply message wrong start code: " << QString::number(array[0], 16);
    *ESTA_code = (array[5] << 8) | array[4];
    array.remove(0, 6); // 4 bytes of Enttec protocol + 2 of ESTA ID
    array.replace(ENTTEC_PRO_END_OF_MSG, '\0'); // replace Enttec termination with string termination

    //for (int i = 0; i < array.size(); i++)
    //    qDebug() << "-Data: " << array[i];
    ftdi_usb_close(ftdi);

    return QString(array);
}

QList <DMXUSBWidget*> QLCFTDI::widgets()
{
    QList <DMXUSBWidget*> widgetList;
    quint32 input_id = 0;
    quint32 output_id = 0;

    struct ftdi_context ftdi;

    ftdi_init(&ftdi);

#ifdef LIBFTDI1
    libusb_device *dev;
    libusb_device **devs;
    struct libusb_device_descriptor dev_descriptor;
    int i = 0;

    if (libusb_get_device_list(ftdi.usb_ctx, &devs) < 0)
    {
        qDebug() << "usb_find_devices() failed";
        return widgetList;
    }

    while ((dev = devs[i++]) != NULL)
    {
        libusb_get_device_descriptor(dev, &dev_descriptor);
#else
    struct usb_bus *bus;
    struct usb_device *dev;
    struct usb_device_descriptor dev_descriptor;

    usb_init();

    if (usb_find_busses() < 0)
    {
        qDebug() << "usb_find_busses() failed";
        return widgetList;
    }
    if (usb_find_devices() < 0)
    {
        qDebug() << "usb_find_devices() failed";
        return widgetList;
    }

    for (bus = usb_get_busses(); bus; bus = bus->next)
    {
      for (dev = bus->devices; dev; dev = dev->next)
      {
        dev_descriptor = dev->descriptor;
#endif
        Q_ASSERT(dev != NULL);

        // Skip non wanted devices
        if (dev_descriptor.idVendor != QLCFTDI::FTDIVID &&
            dev_descriptor.idVendor != QLCFTDI::ATMELVID)
                continue;

        if (dev_descriptor.idProduct != QLCFTDI::FTDIPID &&
            dev_descriptor.idProduct != QLCFTDI::DMX4ALLPID &&
            dev_descriptor.idProduct != QLCFTDI::NANODMXPID)
                continue;

        char ser[256];
        char nme[256];
        char vend[256];

        ftdi_usb_get_strings(&ftdi, dev, vend, 256, nme, 256, ser, 256);

        QString serial(ser);
        QString name(nme);
        QString vendor(vend);

        QMap <QString,QVariant> types(typeMap());

        qDebug() << Q_FUNC_INFO << "DMX USB VID:" << QString::number(dev_descriptor.idVendor, 16) <<
                    "PID:" << QString::number(dev_descriptor.idProduct, 16);
        qDebug() << Q_FUNC_INFO << "DMX USB serial: " << serial << "name:" << name << "vendor:" << vendor;

        if (types.contains(serial) == true)
        {
            // Force a widget with a specific serial to either type
            DMXUSBWidget::Type type = (DMXUSBWidget::Type) types[serial].toInt();
            switch (type)
            {
            case DMXUSBWidget::OpenTX:
                widgetList << new EnttecDMXUSBOpen(serial, name, vendor, output_id++);
                break;
            case DMXUSBWidget::ProMk2:
            {
                EnttecDMXUSBPro *promkii = new EnttecDMXUSBPro(serial, name, vendor, output_id, input_id);
                promkii->setOutputsNumber(2);
                promkii->setMidiPortsNumber(1, 1);
                output_id += 3;
                input_id += 2;
                widgetList << promkii;
                break;
            }
            case DMXUSBWidget::UltraPro:
            {
                EnttecDMXUSBPro *ultra = new EnttecDMXUSBPro(serial, name, vendor, output_id, input_id++);
                ultra->setOutputsNumber(2);
                ultra->setDMXKingMode();
                output_id += 2;
                widgetList << ultra;
                break;
            }
            case DMXUSBWidget::VinceTX:
                widgetList << new VinceUSBDMX512(serial, name, vendor, output_id++);
                break;
            default:
            case DMXUSBWidget::ProRXTX:
                widgetList << new EnttecDMXUSBPro(serial, name, vendor, output_id++, input_id++);
                break;
            }
        }
        else if (name.toUpper().contains("PRO MK2") == true)
        {
            EnttecDMXUSBPro *promkii = new EnttecDMXUSBPro(serial, name, vendor, output_id, input_id);
            promkii->setOutputsNumber(2);
            promkii->setMidiPortsNumber(1, 1);
            output_id += 3;
            input_id += 2;
            widgetList << promkii;
        }
        else if (name.toUpper().contains("DMX USB PRO"))
        {
            /** Check if the device responds to label 77 and 78, so it might be a DMXking adapter */
            int ESTAID = 0;
            int DEVID = 0;
            QString manName = readLabel(&ftdi, name.toLatin1().data(), serial.toLatin1().data(),
                                        DMXKING_USB_DEVICE_MANUFACTURER, &ESTAID);
            qDebug() << "--------> Device Manufacturer: " << manName;
            QString devName = readLabel(&ftdi, name.toLatin1().data(), serial.toLatin1().data(),
                                        DMXKING_USB_DEVICE_NAME, &DEVID);
            qDebug() << "--------> Device Name: " << devName;
            qDebug() << "--------> ESTA Code: " << QString::number(ESTAID, 16) << ", Device ID: " << QString::number(DEVID, 16);
            if (ESTAID == DMXKING_ESTA_ID)
            {
                if (DEVID == ULTRADMX_PRO_DEV_ID)
                {
                    EnttecDMXUSBPro *ultra = new EnttecDMXUSBPro(serial, name, vendor, output_id, input_id++);
                    ultra->setOutputsNumber(2);
                    ultra->setDMXKingMode();
                    ultra->setRealName(devName);
                    output_id += 2;
                    widgetList << ultra;
                }
                else
                {
                    EnttecDMXUSBPro *pro = new EnttecDMXUSBPro(serial, name, vendor, output_id++);
                    pro->setInputsNumber(0);
                    pro->setRealName(devName);
                    widgetList << pro;
                }
            }
            else
            {
                /* This is probably a Enttec DMX USB Pro widget */
                EnttecDMXUSBPro *pro = new EnttecDMXUSBPro(serial, name, vendor, output_id++, input_id++);
                pro->setRealName(devName);
                widgetList << pro;
            }
        }
        else if (name.toUpper().contains("USB-DMX512 CONVERTER") == true)
        {
            widgetList << new VinceUSBDMX512(serial, name, vendor, output_id++);
        }
        else if (dev_descriptor.idVendor == QLCFTDI::FTDIVID &&
                 dev_descriptor.idProduct == QLCFTDI::DMX4ALLPID)
        {
            widgetList << new Stageprofi(serial, name, vendor, output_id++);
        }
#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
        else if (dev_descriptor.idVendor == QLCFTDI::ATMELVID &&
                 dev_descriptor.idProduct == QLCFTDI::NANODMXPID)
        {
            widgetList << new NanoDMX(serial, name, vendor, (void *)dev, output_id++);
        }
#endif
        else
        {
            /* This is probably an Open DMX USB widget */
            widgetList << new EnttecDMXUSBOpen(serial, name, vendor, output_id++);
        }
#ifndef LIBFTDI1
      }
#endif
    }

#ifdef LIBFTDI1
    libusb_free_device_list(devs, 1);
#endif

    ftdi_deinit(&ftdi);
    return widgetList;
}

bool QLCFTDI::open()
{
    if (isOpen() == true)
        return true;

    if (ftdi_usb_open_desc(&m_handle, QLCFTDI::FTDIVID, QLCFTDI::FTDIPID,
                           name().toLatin1(), serial().toLatin1()) < 0)
    {
        qWarning() << Q_FUNC_INFO << name() << ftdi_get_error_string(&m_handle);
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::openByPID(const int PID)
{
    if (isOpen() == true)
        return true;

    if (ftdi_usb_open(&m_handle, QLCFTDI::FTDIVID, PID) < 0)
    {
        qWarning() << Q_FUNC_INFO << name() << ftdi_get_error_string(&m_handle);
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::close()
{
    if (ftdi_usb_close(&m_handle) < 0)
    {
        qWarning() << Q_FUNC_INFO << name() << ftdi_get_error_string(&m_handle);
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::isOpen() const
{
    return (m_handle.usb_dev != NULL) ? true : false;
}

bool QLCFTDI::reset()
{
    if (ftdi_usb_reset(&m_handle) < 0)
    {
        qWarning() << Q_FUNC_INFO << name() << ftdi_get_error_string(&m_handle);
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::setLineProperties()
{
    if (ftdi_set_line_property(&m_handle, BITS_8, STOP_BIT_2, NONE) < 0)
    {
        qWarning() << Q_FUNC_INFO << name() << ftdi_get_error_string(&m_handle);
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::setBaudRate()
{
    if (ftdi_set_baudrate(&m_handle, 250000) < 0)
    {
        qWarning() << Q_FUNC_INFO << name() << ftdi_get_error_string(&m_handle);
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::setFlowControl()
{
    if (ftdi_setflowctrl(&m_handle, SIO_DISABLE_FLOW_CTRL) < 0)
    {
        qWarning() << Q_FUNC_INFO << name() << ftdi_get_error_string(&m_handle);
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::clearRts()
{
    if (ftdi_setrts(&m_handle, 0) < 0)
    {
        qWarning() << Q_FUNC_INFO << name() << ftdi_get_error_string(&m_handle);
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::purgeBuffers()
{
    if (ftdi_usb_purge_buffers(&m_handle) < 0)
    {
        qWarning() << Q_FUNC_INFO << name() << ftdi_get_error_string(&m_handle);
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::setBreak(bool on)
{
    ftdi_break_type type;
    if (on == true)
        type = BREAK_ON;
    else
        type = BREAK_OFF;

    if (ftdi_set_line_property2(&m_handle, BITS_8, STOP_BIT_2, NONE, type) < 0)
    {
        qWarning() << Q_FUNC_INFO << name() << ftdi_get_error_string(&m_handle);
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::write(const QByteArray& data)
{
    if (ftdi_write_data(&m_handle, (uchar*) data.data(), data.size()) < 0)
    {
        qWarning() << Q_FUNC_INFO << name() << ftdi_get_error_string(&m_handle);
        return false;
    }
    else
    {
        return true;
    }
}

QByteArray QLCFTDI::read(int size, uchar* userBuffer)
{
    uchar* buffer = NULL;

    if (userBuffer == NULL)
        buffer = (uchar*) malloc(sizeof(uchar) * size);
    else
        buffer = userBuffer;
    Q_ASSERT(buffer != NULL);

    QByteArray array;
    int read = ftdi_read_data(&m_handle, buffer, size);
    if (userBuffer != NULL)
    {
        for (int i = 0; i < read; i++)
            array.append((char) buffer[i]);
    }
    else
    {
        array = QByteArray::fromRawData((char*) buffer, read);
    }

    if (userBuffer == NULL)
        free(buffer);

    return array;
}

uchar QLCFTDI::readByte(bool* ok)
{
    uchar byte = 0;
    int read = ftdi_read_data(&m_handle, &byte, 1);
    if (read == 1)
    {
        if (ok)
            *ok = true;
        return byte;
    }
    else
    {
        if (ok)
            *ok = false;
        return 0;
    }
}

