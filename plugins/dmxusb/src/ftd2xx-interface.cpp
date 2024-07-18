/*
  Q Light Controller
  ftd2xx-interface.cpp

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

#include "ftd2xx-interface.h"
#include "enttecdmxusbpro.h"

#include <ftd2xx.h>

/**
 * Get some interesting strings from the device.
 *
 * @param deviceIndex The device index, whose strings to get
 * @param vendor Returned vendor string
 * @param description Returned description string
 * @param serial Returned serial string
 * @return FT_OK if strings were extracted successfully
 */
static FT_STATUS get_interface_info(DWORD deviceIndex,
                                     QString& vendor, QString& description,
                                     QString& serial, quint16 &VID, quint16 &PID)
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
        VID = pData.VendorId;
        PID = pData.ProductId;

        if (pData.ProductId == DMXInterface::DMX4ALLPID)
            vendor = QString("DMX4ALL");
        else
            vendor = QString(cVendor);
        description = QString(cDescription);
        serial = QString(cSerial);
    }

    FT_Close(handle);

    return status;
}

FTD2XXInterface::FTD2XXInterface(const QString& serial, const QString& name, const QString& vendor,
                                 quint16 VID, quint16 PID, quint32 id)
    : DMXInterface(serial, name, vendor, VID, PID , id)
    , m_handle(NULL)
{
}

FTD2XXInterface::~FTD2XXInterface()
{
    if (isOpen() == true)
        close();
}

bool FTD2XXInterface::readLabel(uchar label, int &intParam, QString &strParam)
{
    FT_HANDLE ftdi = NULL;

    if (FT_Open(id(), &ftdi) != FT_OK)
        return false;

    if (FT_ResetDevice(ftdi) != FT_OK)
        return false;

    if (FT_SetBaudRate(ftdi, 250000) != FT_OK)
        return false;

    if (FT_SetDataCharacteristics(ftdi, FT_BITS_8, FT_STOP_BITS_2, FT_PARITY_NONE) != FT_OK)
        return false;

    if (FT_SetFlowControl(ftdi, 0, 0, 0) != FT_OK)
        return false;

    QByteArray request;
    request.append(ENTTEC_PRO_START_OF_MSG);
    request.append(label);
    request.append(ENTTEC_PRO_DMX_ZERO); // data length LSB
    request.append(ENTTEC_PRO_DMX_ZERO); // data length MSB
    request.append(ENTTEC_PRO_END_OF_MSG);

    DWORD written = 0;
    if (FT_Write(ftdi, (char*) request.data(), request.size(), &written) != FT_OK)
    {
        qDebug() << Q_FUNC_INFO << "Cannot write data to device" << id();
        FT_Close(ftdi);
        return false;
    }

    if (written == 0)
    {
        qDebug() << Q_FUNC_INFO << "Cannot write data to device" << id();
        FT_Close(ftdi);
        return false;
    }

    uchar buffer[40];
    int read = 0;
    QByteArray array;

    FT_SetTimeouts(ftdi, 500,0);
    FT_Read(ftdi, buffer, 40, (LPDWORD) &read);
    array = QByteArray::fromRawData((char*) buffer, read);

    if (array.size() == 0)
    {
        FT_Close(ftdi);
        return false;
    }

    if (array[0] != ENTTEC_PRO_START_OF_MSG)
    {
        qDebug() << Q_FUNC_INFO << "Reply message wrong start code: " << QString::number(array[0], 16);
        FT_Close(ftdi);
        return false;
    }

    // start | label | data length
    if (array.size() < 4)
    {
        FT_Close(ftdi);
        return false;
    }

    int dataLen = (array[3] << 8) | array[2];
    if (dataLen == 1)
    {
        intParam = array[4];
        FT_Close(ftdi);
        return true;
    }

    intParam = (array[5] << 8) | array[4];
    array.remove(0, 6); // 4 bytes of Enttec protocol + 2 of ESTA ID
    array.replace(ENTTEC_PRO_END_OF_MSG, '\0'); // replace Enttec termination with string termination
    strParam = QString(array);

    FT_Close(ftdi);
    return true;
}

DMXInterface::Type FTD2XXInterface::type()
{
    return DMXInterface::FTD2xx;
}

QString FTD2XXInterface::typeString()
{
    return "FTD2xx";
}

QList<DMXInterface *> FTD2XXInterface::interfaces(QList<DMXInterface *> discoveredList)
{
    QList <DMXInterface*> interfacesList;

    /* Find out the number of FTDI devices present */
    DWORD num = 0;
    FT_STATUS status = FT_CreateDeviceInfoList(&num);
    if (status != FT_OK || num <= 0)
    {
        qWarning() << Q_FUNC_INFO << "[FTD2XXInterface] Error in FT_CreateDeviceInfoList:" << status;
        return interfacesList;
    }

    // Allocate storage for list based on numDevices
    FT_DEVICE_LIST_INFO_NODE* devInfo = new FT_DEVICE_LIST_INFO_NODE[num];

    // Get the device information list
    if (FT_GetDeviceInfoList(devInfo, &num) == FT_OK)
    {
        int id = 0;

        for (DWORD i = 0; i < num; i++)
        {
            QString vendor, name, serial;
            quint16 VID = 0, PID = 0;
            FT_STATUS s = get_interface_info(i, vendor, name, serial, VID, PID);
            if (s != FT_OK || name.isEmpty() || serial.isEmpty())
            {
                // Seems that some otherwise working devices don't provide
                // FT_PROGRAM_DATA struct used by get_interface_info().
                name = QString(devInfo[i].Description);
                serial = QString(devInfo[i].SerialNumber);
                vendor = QString();
            }

            qDebug() << "serial: " << serial << "name:" << name << "vendor:" << vendor;

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
                FTD2XXInterface *iface = new FTD2XXInterface(serial, name, vendor, VID, PID, id);
                interfacesList << iface;
            }

            id++;
        }
    }

    delete [] devInfo;
    return interfacesList;
}

bool FTD2XXInterface::open()
{
    if (isOpen() == true)
        return true;

    FT_STATUS status = FT_Open(id(), &m_handle);
    if (status != FT_OK)
    {
        qWarning() << Q_FUNC_INFO << "Error opening" << name() << "id:" << id() << "status:" << status;
        return false;
    }

    status = FT_GetLatencyTimer(m_handle, &m_defaultLatency);
    if (status != FT_OK)
    {
        qWarning() << Q_FUNC_INFO << name() << status;
        m_defaultLatency = 16;
    }

    qDebug() << Q_FUNC_INFO << serial() << "Default latency is" << m_defaultLatency;
    return true;
}

bool FTD2XXInterface::openByPID(const int PID)
{
    Q_UNUSED(PID)
    return open();
}

bool FTD2XXInterface::close()
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

bool FTD2XXInterface::isOpen() const
{
    return (m_handle != NULL) ? true : false;
}

bool FTD2XXInterface::reset()
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

bool FTD2XXInterface::setLineProperties()
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

bool FTD2XXInterface::setBaudRate()
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

bool FTD2XXInterface::setFlowControl()
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

bool FTD2XXInterface::setLowLatency(bool lowLatency)
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

    FT_STATUS status = FT_SetLatencyTimer(m_handle, latency);
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

bool FTD2XXInterface::clearRts()
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

bool FTD2XXInterface::purgeBuffers()
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

bool FTD2XXInterface::setBreak(bool on)
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

bool FTD2XXInterface::write(const QByteArray& data)
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

QByteArray FTD2XXInterface::read(int size, uchar* userBuffer)
{
    if (m_handle == NULL)
        return QByteArray();

    DWORD RxBytes, TxBytes, event;
    FT_GetStatus(m_handle, &RxBytes, &TxBytes, &event);

    if (RxBytes < (DWORD)size)
        return QByteArray();

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
        array = QByteArray((char*) buffer, read);
    }

    if (userBuffer == NULL)
        free(buffer);

    return array;
}

uchar FTD2XXInterface::readByte(bool* ok)
{
    if (ok) *ok = false;

    if (m_handle == NULL)
        return 0;

    DWORD RxBytes, TxBytes, event;
    FT_GetStatus(m_handle, &RxBytes, &TxBytes, &event);

    if (RxBytes < 1)
        return 0;

    uchar byte = 0;
    int read = 0;
    FT_Read(m_handle, &byte, 1, (LPDWORD) &read);
    if (read == 1)
    {
        if (ok) *ok = true;
        return byte;
    }

    return 0;
}



