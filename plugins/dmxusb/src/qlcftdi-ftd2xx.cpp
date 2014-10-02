/*
  Q Light Controller
  qlcftdi-ftd2xx.cpp

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
#include <QDebug>
#include <QMap>

#include "dmxusbwidget.h"
#include "enttecdmxusbpro.h"
#include "enttecdmxusbopen.h"
#include "stageprofi.h"
#include "vinceusbdmx512.h"
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
    quint32 output_id = 0;

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
                    widgetList << new EnttecDMXUSBOpen(serial, name, vendor, output_id++, i);
                    break;
                case DMXUSBWidget::ProMk2:
                {
                    EnttecDMXUSBPro *promkii = new EnttecDMXUSBPro(serial, name, vendor, output_id, input_id, i);
                    promkii->setOutputsNumber(2);
                    promkii->setMidiPortsNumber(1, 1);
                    output_id += 3;
                    input_id += 2;
                    widgetList << promkii;
                    break;
                }
                case DMXUSBWidget::UltraPro:
                {
                    EnttecDMXUSBPro *ultra = new EnttecDMXUSBPro(serial, name, vendor, output_id, input_id++, i);
                    ultra->setOutputsNumber(2);
                    ultra->setDMXKingMode();
                    output_id += 2;
                    widgetList << ultra;
                    break;
                }
                case DMXUSBWidget::VinceTX:
                    widgetList << new VinceUSBDMX512(serial, name, vendor, output_id++, i);
                    break;
                default:
                case DMXUSBWidget::ProRXTX:
                    widgetList << new EnttecDMXUSBPro(serial, name, vendor, output_id++, input_id++, i);
                    break;
                }
            }
            else if (name.toUpper().contains("PRO MK2") == true)
            {
                EnttecDMXUSBPro *promkii = new EnttecDMXUSBPro(serial, name, vendor, output_id, input_id, i);
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
                QString manName = readLabel(i, DMXKING_USB_DEVICE_MANUFACTURER, &ESTAID);
                qDebug() << "--------> Device Manufacturer: " << manName;
                QString devName = readLabel(i, DMXKING_USB_DEVICE_NAME, &DEVID);
                qDebug() << "--------> Device Name: " << devName;
                qDebug() << "--------> ESTA Code: " << QString::number(ESTAID, 16) << ", Device ID: " << QString::number(DEVID, 16);
                if (ESTAID == DMXKING_ESTA_ID)
                {
                    if (DEVID == ULTRADMX_PRO_DEV_ID)
                    {
                        EnttecDMXUSBPro *ultra = new EnttecDMXUSBPro(serial, name, vendor, output_id, input_id++, i);
                        ultra->setOutputsNumber(2);
                        ultra->setDMXKingMode();
                        ultra->setRealName(devName);
                        output_id += 2;
                        widgetList << ultra;
                    }
                    else
                    {
                        EnttecDMXUSBPro *pro = new EnttecDMXUSBPro(serial, name, vendor, output_id++, 0, i);
                        pro->setInputsNumber(0);
                        pro->setRealName(devName);
                        widgetList << pro;
                    }
                }
                else
                {
                    /* This is probably a Enttec DMX USB Pro widget */
                    EnttecDMXUSBPro *pro = new EnttecDMXUSBPro(serial, name, vendor, output_id++, input_id++, i);
                    pro->setRealName(devName);
                    widgetList << pro;
                }
            }
            else if (name.toUpper().contains("USB-DMX512 CONVERTER") == true)
            {
                widgetList << new VinceUSBDMX512(serial, name, vendor, output_id++, i);
            }
            else if (vendor.toUpper().contains("DMX4ALL") == true)
            {
                widgetList << new Stageprofi(serial, name, vendor, output_id++, i);
            }
            else
            {
                /* This is probably an Open DMX USB widget */
                widgetList << new EnttecDMXUSBOpen(serial, name, vendor, output_id++, i);
            }
        }
    }

    delete [] devInfo;
    return widgetList;
}

bool QLCFTDI::open()
{
    if (isOpen() == true)
        return true;

    FT_STATUS status = FT_Open(m_id, &m_handle);
    if (status != FT_OK)
    {
        qWarning() << Q_FUNC_INFO << "Error opening" << name() << status;
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



