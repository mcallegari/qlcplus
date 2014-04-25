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
#include "enttecdmxusbprotx.h"
#include "enttecdmxusbprorx.h"
#include "enttecdmxusbopen.h"
#include "ultradmxusbprotx.h"
#include "dmx4all.h"
#include "vinceusbdmx512tx.h"
#include "qlcftdi.h"

QLCFTDI::QLCFTDI(const QString& serial, const QString& name, const QString& vendor, quint32 id)
    : m_serial(serial)
    , m_name(name)
    , m_vendor(vendor)
    , m_id(id)
    , m_refCount(1)
    , m_openCount(0)
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
    if (ftdi_usb_open_desc(ftdi, QLCFTDI::VID, QLCFTDI::PID, name, serial) < 0)
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

    struct ftdi_device_list* list = 0;
    struct ftdi_context ftdi;
    ftdi_init(&ftdi);
    ftdi_usb_find_all(&ftdi, &list, QLCFTDI::VID, QLCFTDI::PID);
    while (list != NULL)
    {
#ifdef LIBFTDI1
        struct libusb_device* dev = list->dev;
#else
        struct usb_device* dev = list->dev;
#endif
        Q_ASSERT(dev != NULL);

        char serial[256];
        char name[256];
        char vendor[256];

        ftdi_usb_get_strings(&ftdi, dev,
                             vendor, sizeof(vendor),
                             name, sizeof(name),
                             serial, sizeof(serial));

        QString ser(serial);
        QString nme(name);
        QString ven(vendor);
        QMap <QString,QVariant> types(typeMap());

        qDebug() << "serial: " << ser << "name:" << nme << "vendor:" << ven;

        if (types.contains(ser) == true)
        {
            // Force a widget with a specific serial to either type
            DMXUSBWidget::Type type = (DMXUSBWidget::Type) types[ser].toInt();
            switch (type)
            {
            case DMXUSBWidget::OpenTX:
                widgetList << new EnttecDMXUSBOpen(serial, name, vendor);
                break;
            case DMXUSBWidget::ProRX:
            {
                EnttecDMXUSBProRX* prorx = new EnttecDMXUSBProRX(serial, name, vendor, input_id++);
                widgetList << prorx;
                break;
            }
            case DMXUSBWidget::ProMk2:
            {
                EnttecDMXUSBProTX* protx = new EnttecDMXUSBProTX(serial, name, vendor, 1);
                widgetList << protx;
                widgetList << new EnttecDMXUSBProTX(serial, name, vendor, 2, protx->ftdi());
                EnttecDMXUSBProRX* prorx = new EnttecDMXUSBProRX(serial, name, vendor, input_id++, protx->ftdi());
                widgetList << prorx;
                break;
            }
            case DMXUSBWidget::UltraProTx:
            {
                UltraDMXUSBProTx* protx = new UltraDMXUSBProTx(serial, name, vendor, 1);
                widgetList << protx;
                widgetList << new UltraDMXUSBProTx(serial, name, vendor, 2, protx->ftdi());
                EnttecDMXUSBProRX* prorx = new EnttecDMXUSBProRX(serial, name, vendor, input_id++, protx->ftdi());
                widgetList << prorx;
                break;
            }
            case DMXUSBWidget::VinceTX:
                widgetList << new VinceUSBDMX512TX(serial, name, vendor);
                break;
            default:
            case DMXUSBWidget::ProTX:
                widgetList << new EnttecDMXUSBProTX(serial, name, vendor);
                break;
            }
        }
        else if (nme.toUpper().contains("PRO MK2") == true)
        {
            EnttecDMXUSBProTX* protx = new EnttecDMXUSBProTX(serial, name, vendor, 1);
            widgetList << protx;
            widgetList << new EnttecDMXUSBProTX(serial, name, vendor, 2, protx->ftdi());
            EnttecDMXUSBProRX* prorx = new EnttecDMXUSBProRX(serial, name, vendor, input_id++, protx->ftdi());
            widgetList << prorx;
        }
        else if (nme.toUpper().contains("DMX USB PRO"))
        {
            /** Check if the device responds to label 77 and 78, so it might be a DMXking adapter */
            int ESTAID = 0;
            int DEVID = 0;
            QString manName = readLabel(&ftdi, name, serial, USB_DEVICE_MANUFACTURER, &ESTAID);
            qDebug() << "--------> Device Manufacturer: " << manName;
            QString devName = readLabel(&ftdi, name, serial, USB_DEVICE_NAME, &DEVID);
            qDebug() << "--------> Device Name: " << devName;
            qDebug() << "--------> ESTA Code: " << QString::number(ESTAID, 16) << ", Device ID: " << QString::number(DEVID, 16);
            if (ESTAID == DMXKING_ESTA_ID)
            {
                if (DEVID == ULTRADMX_PRO_DEV_ID)
                {
                    UltraDMXUSBProTx* protxP1 = new UltraDMXUSBProTx(serial, name, vendor, 1);
                    protxP1->setRealName(devName);
                    widgetList << protxP1;
                    UltraDMXUSBProTx* protxP2 = new UltraDMXUSBProTx(serial, name, vendor, 2, protxP1->ftdi());
                    protxP2->setRealName(devName);
                    widgetList << protxP2;
                    EnttecDMXUSBProRX* prorx = new EnttecDMXUSBProRX(serial, name, vendor, input_id++, protxP1->ftdi());
                    prorx->setRealName(devName);
                    widgetList << prorx;
                }
                else
                {
                    EnttecDMXUSBProTX* protx = new EnttecDMXUSBProTX(serial, name, vendor);
                    protx->setRealName(devName);
                    widgetList << protx;
                }
            }
            else
            {
                /* This is probably a Enttec DMX USB Pro widget */
                EnttecDMXUSBProTX* protx = new EnttecDMXUSBProTX(serial, name, vendor);
                widgetList << protx;
                EnttecDMXUSBProRX* prorx = new EnttecDMXUSBProRX(serial, name, vendor, input_id++, protx->ftdi());
                widgetList << prorx;
            }
        }
        else if (nme.toUpper().contains("USB-DMX512 CONVERTER") == true)
        {
            widgetList << new VinceUSBDMX512TX(serial, name, vendor);
        }
        else
        {
            /* This is probably an Open DMX USB widget */
            widgetList << new EnttecDMXUSBOpen(serial, name, vendor, 0);
        }

        list = list->next;
    }

    /* Search for DMX4ALL devices now */
    ftdi_usb_find_all(&ftdi, &list, QLCFTDI::VID, QLCFTDI::DMX4ALLPID);
    while (list != NULL)
    {
#ifdef LIBFTDI1
        struct libusb_device* dev = list->dev;
#else
        struct usb_device* dev = list->dev;
#endif
        Q_ASSERT(dev != NULL);

        char serial[256];
        char name[256];
        char vendor[256];

        ftdi_usb_get_strings(&ftdi, dev,
                             vendor, sizeof(vendor),
                             name, sizeof(name),
                             serial, sizeof(serial));

        QString ser(serial);
        QString nme(name);
        QString ven(vendor);

        qDebug() << "serial: " << ser << "name:" << nme << "vendor:" << ven;
        widgetList << new DMX4ALL(ser, nme, ven);

        list = list->next;
    }

    ftdi_deinit(&ftdi);
    return widgetList;
}

bool QLCFTDI::open()
{
    if (m_openCount < m_refCount)
        m_openCount++;

    if (isOpen() == true)
        return true;

    if (ftdi_usb_open_desc(&m_handle, QLCFTDI::VID, QLCFTDI::PID,
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
    if (m_openCount < m_refCount)
        m_openCount++;

    if (isOpen() == true)
        return true;

    if (ftdi_usb_open(&m_handle, QLCFTDI::VID, PID) < 0)
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
    if (m_openCount > 1)
    {
        m_openCount--;
        return true;
    }

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

void QLCFTDI::modifyRefCount(int amount)
{
    m_refCount += amount;
}

int QLCFTDI::refCount()
{
    return m_refCount;
}
