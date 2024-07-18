/*
  Q Light Controller Plus
  libftdi-interface.cpp

  Copyright (C) Heikki Junnila
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

#include <QSettings>
#include <QVariant>
#include <QDebug>
#include <QMap>

#if defined(LIBFTDI1)
  #include <unistd.h>
  #include <libusb.h>
#endif

#include "libftdi-interface.h"
#include "enttecdmxusbpro.h"

LibFTDIInterface::LibFTDIInterface(const QString& serial, const QString& name, const QString& vendor,
                                   quint16 VID, quint16 PID, quint32 id)
    : DMXInterface(serial, name, vendor, VID, PID , id)
{
    bzero(&m_handle, sizeof(struct ftdi_context));
    ftdi_init(&m_handle);
#ifdef LIBFTDI1_5
    m_handle.module_detach_mode = AUTO_DETACH_REATACH_SIO_MODULE;
#endif
}

LibFTDIInterface::~LibFTDIInterface()
{
    if (isOpen() == true)
        close();
    ftdi_deinit(&m_handle);
}

bool LibFTDIInterface::readLabel(uchar label, int &intParam, QString &strParam)
{
    if (ftdi_usb_open_desc(&m_handle, DMXInterface::FTDIVID, DMXInterface::FTDIPID,
                           name().toLatin1().data(), serial().toLatin1().data()) < 0)

        return false;

    if (ftdi_usb_reset(&m_handle) < 0)
        return false;

    if (ftdi_set_baudrate(&m_handle, 250000) < 0)
        return false;

    if (ftdi_set_line_property(&m_handle, BITS_8, STOP_BIT_2, NONE) < 0)
        return false;

    if (ftdi_setflowctrl(&m_handle, SIO_DISABLE_FLOW_CTRL) < 0)
        return false;

    QByteArray request;
    request.append(ENTTEC_PRO_START_OF_MSG);
    request.append(label);
    request.append(ENTTEC_PRO_DMX_ZERO); // data length LSB
    request.append(ENTTEC_PRO_DMX_ZERO); // data length MSB
    request.append(ENTTEC_PRO_END_OF_MSG);

    if (ftdi_write_data(&m_handle, (uchar*) request.data(), request.size()) < 0)
    {
        qDebug() << Q_FUNC_INFO << "Cannot write data to device";
        return false;
    }

    uchar buffer[40];

    for (int i = 0; i < 3; i++)
    {
        QByteArray array = read(40, buffer);
        if (array.size() == 0)
            return false;

        if (array[0] != ENTTEC_PRO_START_OF_MSG)
        {
            qDebug() << Q_FUNC_INFO << "Reply message wrong start code: " << QString::number(array[0], 16);
            return false;
        }

        // start | label | data length
        if (array.size() < 4)
            return false;

        int dataLen = (array[3] << 8) | array[2];
        if (dataLen == 1)
        {
            intParam = array[4];
            return true;
        }

        intParam = (array[5] << 8) | array[4];
        array.remove(0, 6); // 4 bytes of Enttec protocol + 2 of ESTA ID
        array.replace(ENTTEC_PRO_END_OF_MSG, '\0'); // replace Enttec termination with string termination
        strParam = QString(array);

        ftdi_usb_close(&m_handle);

        return true;

        // retry in case no data is read immediately
        usleep(100000);
    }

    ftdi_usb_close(&m_handle);

    return false;
}

void LibFTDIInterface::setBusLocation(quint8 location)
{
    m_busLocation = location;
}

quint8 LibFTDIInterface::busLocation()
{
    return m_busLocation;
}

DMXInterface::Type LibFTDIInterface::type()
{
    return DMXInterface::libFTDI;
}

QString LibFTDIInterface::typeString()
{
    return "libFTDI";
}

QList<DMXInterface *> LibFTDIInterface::interfaces(QList<DMXInterface *> discoveredList)
{
    QList <DMXInterface*> interfacesList;
    int id = 0;

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
        return interfacesList;
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
        return interfacesList;
    }
    if (usb_find_devices() < 0)
    {
        qDebug() << "usb_find_devices() failed";
        return interfacesList;
    }

    for (bus = usb_get_busses(); bus; bus = bus->next)
    {
      for (dev = bus->devices; dev; dev = dev->next)
      {
        dev_descriptor = dev->descriptor;
#endif
        Q_ASSERT(dev != NULL);

        // Skip non wanted devices
        if (validInterface(dev_descriptor.idVendor, dev_descriptor.idProduct) == false)
            continue;

        if (dev_descriptor.idVendor != DMXInterface::FTDIVID)
            continue;

        char ser[256];
        memset(ser, 0, 256);
        char nme[256];
        char vend[256];

        ftdi_usb_get_strings(&ftdi, dev, vend, 256, nme, 256, ser, 256);

        QString serial(ser);
        QString name(nme);
        QString vendor(vend);

        qDebug() << Q_FUNC_INFO << "DMX USB VID:" << QString::number(dev_descriptor.idVendor, 16) <<
                    "PID:" << QString::number(dev_descriptor.idProduct, 16);
        qDebug() << Q_FUNC_INFO << "DMX USB serial: " << serial << "name:" << name << "vendor:" << vendor;

        bool found = false;
        for (int c = 0; c < discoveredList.count(); c++)
        {
            if (discoveredList.at(c)->checkInfo(serial, name, vendor) == true)
            {
                found = true;
                break;
            }
        }
        if (found == false)
        {
            LibFTDIInterface *iface = new LibFTDIInterface(serial, name, vendor, dev_descriptor.idVendor,
                                                           dev_descriptor.idProduct, id++);
#ifdef LIBFTDI1
            iface->setBusLocation(libusb_get_port_number(dev));
#else
            iface->setBusLocation(dev->bus->location);
#endif
            interfacesList << iface;
        }

#ifndef LIBFTDI1
      }
#endif
    }

#ifdef LIBFTDI1
    libusb_free_device_list(devs, 1);
#endif

    ftdi_deinit(&ftdi);

    return interfacesList;
}

bool LibFTDIInterface::open()
{
    if (isOpen() == true)
        return true;

    QByteArray sba = serial().toLatin1();
    const char *ser = NULL;
    if (serial().isEmpty() == false)
        ser = (const char *)sba.data();

    if (ftdi_usb_open_desc(&m_handle, vendorID(), productID(),
                           name().toLatin1(), ser) < 0)
    {
        qWarning() << Q_FUNC_INFO << name() << ftdi_get_error_string(&m_handle);
        return false;
    }

    if (ftdi_get_latency_timer(&m_handle, &m_defaultLatency))
    {
        qWarning() << Q_FUNC_INFO << serial() << ftdi_get_error_string(&m_handle) << "while querying latency";
        m_defaultLatency = 16;
    }

    qDebug() << Q_FUNC_INFO << serial() << "Default latency is" << m_defaultLatency;
    return true;
}

bool LibFTDIInterface::openByPID(const int PID)
{
    if (isOpen() == true)
        return true;

    if (ftdi_usb_open(&m_handle, DMXInterface::FTDIVID, PID) < 0)
    {
        qWarning() << Q_FUNC_INFO << name() << ftdi_get_error_string(&m_handle);
        return false;
    } else
    {
        return true;
    }
}

bool LibFTDIInterface::close()
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

bool LibFTDIInterface::isOpen() const
{
    return (m_handle.usb_dev != NULL) ? true : false;
}

bool LibFTDIInterface::reset()
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

bool LibFTDIInterface::setLineProperties()
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

bool LibFTDIInterface::setBaudRate()
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

bool LibFTDIInterface::setFlowControl()
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

bool LibFTDIInterface::setLowLatency(bool lowLatency)
{
    unsigned char latency;
    if (lowLatency)
    {
        latency = 1;
    }
    else
    {
        latency = m_defaultLatency;
    }

    if (ftdi_set_latency_timer(&m_handle, latency))
    {
        qWarning() << Q_FUNC_INFO << serial() << ftdi_get_error_string(&m_handle);
        return false;
    }
    else
    {
        qDebug() << Q_FUNC_INFO << serial() << "Latency set to" << latency;
    }

    return true;
}

bool LibFTDIInterface::clearRts()
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

bool LibFTDIInterface::purgeBuffers()
{
#if defined(LIBFTDI1_5)
    if (ftdi_tcioflush(&m_handle) < 0)
#else
    if (ftdi_usb_purge_buffers(&m_handle) < 0)
#endif
    {
        qWarning() << Q_FUNC_INFO << name() << ftdi_get_error_string(&m_handle);
        return false;
    }
    else
    {
        return true;
    }
}

bool LibFTDIInterface::setBreak(bool on)
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

bool LibFTDIInterface::write(const QByteArray& data)
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

QByteArray LibFTDIInterface::read(int size, uchar* userBuffer)
{
    uchar* buffer = NULL;

    if (userBuffer == NULL)
        buffer = (uchar*) malloc(sizeof(uchar) * size);
    else
        buffer = userBuffer;
    Q_ASSERT(buffer != NULL);

    QByteArray array;
    int read = ftdi_read_data(&m_handle, buffer, size);
    array = QByteArray((char*)buffer, read);

    if (userBuffer == NULL)
        free(buffer);

    return array;
}

uchar LibFTDIInterface::readByte(bool* ok)
{
    if (ok) *ok = false;

    uchar byte = 0;
    int read = ftdi_read_data(&m_handle, &byte, 1);
    if (read == 1)
    {
        if (ok) *ok = true;
        return byte;
    }

    return 0;
}
