/*
  Q Light Controller
  qlcftdi-ftd2xx.cpp

  Copyright (C) Heikki Junnila

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <QSettings>
#include <QVariant>
#include <QDebug>
#include <QMap>

#include "dmxusbwidget.h"
#include "enttecdmxusbprotx.h"
#include "enttecdmxusbprorx.h"
#include "enttecdmxusbopen.h"
#include "ultradmxusbprotx.h"
#include "dmx4all.h"
#include "qlcftdi.h"

/**
 * Get some interesting strings from the device.
 *
 * @param deviceIndex The device index, whose strings to get
 * @param vendor Returned vendor string
 * @param description Returned description string
 * @param serial Returned serial string
 * @return FT_OK if strings were extracted successfully
 */
static FT_STATUS qlcftdi_get_strings(DWORD deviceIndex,
                                     QString& vendor,
                                     QString& description,
                                     QString& serial)
{
    char cVendor[256];
    char cVendorId[256];
    char cDescription[256];
    char cSerial[256];

    FT_HANDLE handle;

    FT_STATUS status = FT_Open(deviceIndex, &handle);
    if (status != FT_OK)
        return status;

    FT_PROGRAM_DATA pData;
    pData.Signature1 = 0;
    pData.Signature2 = 0xFFFFFFFF;
    pData.Version = 0x00000005;
    pData.Manufacturer = cVendor;
    pData.ManufacturerId = cVendorId;
    pData.Description = cDescription;
    pData.SerialNumber = cSerial;
    status = FT_EE_Read(handle, &pData);
    if (status == FT_OK)
    {
        if (pData.ProductId == QLCFTDI::DMX4ALLPID)
            vendor = QString("DMX4ALL");
        else
            vendor = QString(cVendor);
        description = QString(cDescription);
        serial = QString(cSerial);
    }

    FT_Close(handle);

    return status;
}

QLCFTDI::QLCFTDI(const QString& serial, const QString& name, const QString& vendor, quint32 id)
    : m_serial(serial)
    , m_name(name)
    , m_vendor(vendor)
    , m_id(id)
    , m_refCount(1)
    , m_openCount(0)
    , m_handle(NULL)
{
}

QLCFTDI::~QLCFTDI()
{
    if (isOpen() == true)
        close();
}

QString QLCFTDI::readLabel(quint32 id, uchar label, int *ESTA_code)
{
    FT_HANDLE ftdi = NULL;

    if (FT_Open(id, &ftdi) != FT_OK)
        return QString();

    if(FT_ResetDevice(ftdi) != FT_OK)
        return QString();

    if(FT_SetBaudRate(ftdi, 250000) != FT_OK)
        return QString();

    if(FT_SetDataCharacteristics(ftdi, FT_BITS_8, FT_STOP_BITS_2, FT_PARITY_NONE) != FT_OK)
        return QString();

    if(FT_SetFlowControl(ftdi, 0, 0, 0) != FT_OK)
        return QString();

    QByteArray request;
    request.append(ENTTEC_PRO_START_OF_MSG);
    request.append(label);
    request.append(ENTTEC_PRO_DMX_ZERO); // data length LSB
    request.append(ENTTEC_PRO_DMX_ZERO); // data length MSB
    request.append(ENTTEC_PRO_END_OF_MSG);

    DWORD written = 0;
    if (FT_Write(ftdi, (char*) request.data(), request.size(), &written) != FT_OK)
        return QString();

    if (written == 0)
    {
        qDebug() << Q_FUNC_INFO << "Cannot write data to device";
        return QString();
    }

    uchar* buffer = (uchar*) malloc(sizeof(uchar) * 40);
    Q_ASSERT(buffer != NULL);

    int read = 0;
    QByteArray array;
    FT_SetTimeouts(ftdi, 500,0);
    FT_Read(ftdi, buffer, 40, (LPDWORD) &read);
    qDebug() << Q_FUNC_INFO << "----- Read: " << read << " ------";
    for (int i = 0; i < read; i++)
        array.append((char) buffer[i]);

    if (array[0] != ENTTEC_PRO_START_OF_MSG)
        qDebug() << Q_FUNC_INFO << "Reply message wrong start code: " << QString::number(array[0], 16);
    *ESTA_code = (array[5] << 8) | array[4];
    array.remove(0, 6); // 4 bytes of Enttec protocol + 2 of ESTA ID
    array.replace(ENTTEC_PRO_END_OF_MSG, '\0'); // replace Enttec termination with string termination

    FT_Close(ftdi);
    return QString(array);
}

QList <DMXUSBWidget*> QLCFTDI::widgets()
{
    QList <DMXUSBWidget*> widgetList;
    quint32 input_id = 0;

    /* Find out the number of FTDI devices present */
    DWORD num = 0;
    FT_STATUS status = FT_CreateDeviceInfoList(&num);
    if (status != FT_OK)
    {
        qWarning() << Q_FUNC_INFO << "CreateDeviceInfoList:" << status;
        return widgetList;
    }
    else if (num <= 0)
    {
        return widgetList;
    }

    // Allocate storage for list based on numDevices
    FT_DEVICE_LIST_INFO_NODE* devInfo = new FT_DEVICE_LIST_INFO_NODE[num];

	// Get a map of user-forced serials and their types
    QMap <QString,QVariant> types(typeMap());

    // Get the device information list
    if (FT_GetDeviceInfoList(devInfo, &num) == FT_OK)
    {
        for (DWORD i = 0; i < num; i++)
        {
            QString vendor, name, serial;
            FT_STATUS s = qlcftdi_get_strings(i, vendor, name, serial);
            if (s != FT_OK || name.isEmpty() || serial.isEmpty())
            {
				// Seems that some otherwise working devices don't provide
				// FT_PROGRAM_DATA struct used by qlcftdi_get_strings().
                name = QString(devInfo[i].Description);
				serial = QString(devInfo[i].SerialNumber);
				vendor = QString();
			}

            qDebug() << "serial: " << serial << "name:" << name << "vendor:" << vendor;

            if (types.contains(serial) == true)
            {
                // Force a widget with a specific serial to either type
                DMXUSBWidget::Type type = (DMXUSBWidget::Type)
                                                    types[serial].toInt();
                switch (type)
                {
                case DMXUSBWidget::OpenTX:
                    widgetList << new EnttecDMXUSBOpen(serial, name, vendor, i);
                    break;
                case DMXUSBWidget::ProRX:
                    widgetList << new EnttecDMXUSBProRX(serial, name, vendor, input_id++);
                    break;
                case DMXUSBWidget::ProMk2:
                {
                    EnttecDMXUSBProTX* protx = new EnttecDMXUSBProTX(serial, name, vendor, 1);
                    widgetList << protx;
                    widgetList << new EnttecDMXUSBProTX(serial, name, vendor, 2, protx->ftdi(), i);
                    EnttecDMXUSBProRX* prorx = new EnttecDMXUSBProRX(serial, name, vendor, input_id++, protx->ftdi(), i);
                    widgetList << prorx;
                    break;
                }
                case DMXUSBWidget::UltraProTx:
                {
                    UltraDMXUSBProTx* protx = new UltraDMXUSBProTx(serial, name, vendor, 1, NULL, i);
                    widgetList << protx;
                    widgetList << new UltraDMXUSBProTx(serial, name, vendor, 2, protx->ftdi(), i);
                    EnttecDMXUSBProRX* prorx = new EnttecDMXUSBProRX(serial, name, vendor, input_id++, protx->ftdi(), i);
                    widgetList << prorx;
                    break;
                }
                default:
                case DMXUSBWidget::ProTX:
                    widgetList << new EnttecDMXUSBProTX(serial, name, vendor, i);
                    break;
                }
            }
            else if (name.toUpper().contains("PRO MK2") == true)
            {
                EnttecDMXUSBProTX* protx = new EnttecDMXUSBProTX(serial, name, vendor, 1, NULL, i);
                widgetList << protx;
                widgetList << new EnttecDMXUSBProTX(serial, name, vendor, 2, protx->ftdi(), i);
                EnttecDMXUSBProRX* prorx = new EnttecDMXUSBProRX(serial, name, vendor, input_id++, protx->ftdi(), i);
                widgetList << prorx;
            }
            else if (name.toUpper().contains("DMX USB PRO"))
            {
                /** Check if the device responds to label 77 and 78, so it might be a DMXking adapter */
                int ESTAID = 0;
                int DEVID = 0;
                QString manName = readLabel(i, USB_DEVICE_MANUFACTURER, &ESTAID);
                qDebug() << "--------> Device Manufacturer: " << manName;
                QString devName = readLabel(i, USB_DEVICE_NAME, &DEVID);
                qDebug() << "--------> Device Name: " << devName;
                qDebug() << "--------> ESTA Code: " << QString::number(ESTAID, 16) << ", Device ID: " << QString::number(DEVID, 16);
                if (ESTAID == DMXKING_ESTA_ID)
                {
                    if (DEVID == ULTRADMX_PRO_DEV_ID)
                    {
                        UltraDMXUSBProTx* protxP1 = new UltraDMXUSBProTx(serial, name, vendor, 1, NULL, i);
                        protxP1->setRealName(devName);
                        widgetList << protxP1;
                        UltraDMXUSBProTx* protxP2 = new UltraDMXUSBProTx(serial, name, vendor, 2, protxP1->ftdi(), i);
                        protxP2->setRealName(devName);
                        widgetList << protxP2;
                        EnttecDMXUSBProRX* prorx = new EnttecDMXUSBProRX(serial, name, vendor, input_id++, protxP1->ftdi(), i);
                        prorx->setRealName(devName);
                        widgetList << prorx;
                    }
                    else
                    {
                        EnttecDMXUSBProTX* protx = new EnttecDMXUSBProTX(serial, name, vendor, 1, NULL, i);
                        protx->setRealName(devName);
                        widgetList << protx;
                    }
                }
                else
                {
                    /* This is probably a Enttec DMX USB Pro widget in TX mode */
                    widgetList << new EnttecDMXUSBProTX(serial, name, vendor, 1, NULL, i);
                }
            }
            else if (vendor.toUpper().contains("DMX4ALL") == true)
            {
                widgetList << new DMX4ALL(serial, name, vendor, NULL, i);
            }
            else
            {
                /* This is probably an Open DMX USB widget */
                widgetList << new EnttecDMXUSBOpen(serial, name, vendor, i);
            }
        }
    }

    delete [] devInfo;
    return widgetList;
}

bool QLCFTDI::open()
{
    if (m_openCount < m_refCount)
        m_openCount++;

    if (isOpen() == true)
        return true;

    FT_STATUS status = FT_Open(m_id, &m_handle);
    if (status != FT_OK)
    {
        qWarning() << Q_FUNC_INFO << name() << status;
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::openByPID(const int PID)
{
    Q_UNUSED(PID)
    return open();
}

bool QLCFTDI::close()
{
    if (m_openCount > 1)
    {
        m_openCount--;
        return true;
    }

    FT_STATUS status = FT_Close(m_handle);
    m_handle = NULL;
    if (status != FT_OK)
    {
        qWarning() << Q_FUNC_INFO << name() << status;
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::isOpen() const
{
    return (m_handle != NULL) ? true : false;
}

bool QLCFTDI::reset()
{
    FT_STATUS status = FT_ResetDevice(m_handle);
    if (status != FT_OK)
    {
        qWarning() << Q_FUNC_INFO << name() << status;
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::setLineProperties()
{
    FT_STATUS status = FT_SetDataCharacteristics(m_handle, FT_BITS_8, FT_STOP_BITS_2, FT_PARITY_NONE);
    if (status != FT_OK)
    {
        qWarning() << Q_FUNC_INFO << name() << status;
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::setBaudRate()
{
    FT_STATUS status = FT_SetBaudRate(m_handle, 250000);
    if (status != FT_OK)
    {
        qWarning() << Q_FUNC_INFO << name() << status;
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::setFlowControl()
{
    FT_STATUS status = FT_SetFlowControl(m_handle, 0, 0, 0);
    if (status != FT_OK)
    {
        qWarning() << Q_FUNC_INFO << name() << status;
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::clearRts()
{
    FT_STATUS status = FT_ClrRts(m_handle);
    if (status != FT_OK)
    {
        qWarning() << Q_FUNC_INFO << name() << status;
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::purgeBuffers()
{
    FT_STATUS status = FT_Purge(m_handle, FT_PURGE_RX | FT_PURGE_TX);
    if (status != FT_OK)
    {
        qWarning() << Q_FUNC_INFO << name() << status;
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::setBreak(bool on)
{
    FT_STATUS status;
    if (on == true)
        status = FT_SetBreakOn(m_handle);
    else
        status = FT_SetBreakOff(m_handle);

    if (status != FT_OK)
    {
        qWarning() << Q_FUNC_INFO << name() << status;
        return false;
    }
    else
    {
        return true;
    }
}

bool QLCFTDI::write(const QByteArray& data)
{
    DWORD written = 0;
    FT_STATUS status = FT_Write(m_handle, (char*) data.data(), data.size(), &written);
    if (status != FT_OK)
    {
        qWarning() << Q_FUNC_INFO << name() << status;
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

    int read = 0;
    QByteArray array;
    FT_Read(m_handle, buffer, size, (LPDWORD) &read);
    if (userBuffer == NULL)
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
    int read = 0;
    FT_Read(m_handle, &byte, 1, (LPDWORD) &read);
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


