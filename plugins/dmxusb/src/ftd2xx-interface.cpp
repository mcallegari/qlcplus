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

#define EEPROM_VID_OFFSET       1
#define EEPROM_PID_OFFSET       2
#define EEPROM_VENDOR_OFFSET    13

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
    FT_HANDLE handle;
    if (FT_Open(deviceIndex, &handle) != FT_OK)
        return FT_DEVICE_NOT_OPENED;

    FT_STATUS status = FT_OK;
    WORD value;

#if 0 // uncomment to dump eeprom content
    QByteArray eepromData;
    for (int i = 0; i < 64; i++)
    {
        FT_STATUS ret = FT_ReadEE(handle, i, &value);
        eepromData.append(value >> 8);
        eepromData.append(value & 0xFF);
    }
    qDebug() << "EEPROM DUMP:" << eepromData.toHex(',');
#endif

    auto readWord = [&](int offset, WORD &val) -> FT_STATUS
    {
        return FT_ReadEE(handle, offset, &val);
    };

    auto readStringDescriptor = [&](int &offset, QString &out) -> bool
    {
        WORD val;
        if (readWord(offset, val) != FT_OK || (val >> 8) != 0x03)
            return false;

        int length = ((val & 0xFF) / 2) - 1;
        offset++;
        for (int i = 0; i < length; ++i)
        {
            if (readWord(offset++, val) != FT_OK)
                return false;

            // filter non-visible characters
            if (val < 0x20)
                continue;

            out.append(char(val & 0xFF));
        }
        return true;
    };

    if (readWord(EEPROM_VID_OFFSET, value) == FT_OK)
        VID = value;
    else
        status = FT_EEPROM_READ_FAILED;

    if (status == FT_OK && readWord(EEPROM_PID_OFFSET, value) == FT_OK)
        PID = value;
    else if (status == FT_OK)
        status = FT_EEPROM_READ_FAILED;

    int offset = EEPROM_VENDOR_OFFSET;

    if (status == FT_OK && !readStringDescriptor(offset, vendor))
        status = FT_EEPROM_READ_FAILED;
    if (status == FT_OK && !readStringDescriptor(offset, description))
        status = FT_EEPROM_READ_FAILED;
    if (status == FT_OK && !readStringDescriptor(offset, serial))
        status = FT_EEPROM_READ_FAILED;

    if (status == FT_OK && PID == DMXInterface::DMX4ALLPID)
        vendor = QString("DMX4ALL");

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

QByteArray FTD2XXInterface::read(int size)
{
    if (m_handle == NULL)
        return QByteArray();

    DWORD RxBytes, TxBytes, event;
    FT_GetStatus(m_handle, &RxBytes, &TxBytes, &event);

    if (RxBytes < (DWORD)size)
        return QByteArray();

    uchar* buffer = NULL;

    buffer = (uchar*) malloc(sizeof(uchar) * size);
    Q_ASSERT(buffer != NULL);

    int read = 0;
    QByteArray array;
    FT_Read(m_handle, buffer, size, (LPDWORD) &read);
    array = QByteArray((char*) buffer, read);

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



