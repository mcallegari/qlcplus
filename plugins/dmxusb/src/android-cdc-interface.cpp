/*
  Q Light Controller Plus
  android-cdc-interface.cpp

  Copyright (c) Massimo Callegari

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

#include <linux/usb/ch9.h>
#include <errno.h>

#include "android-cdc-interface.h"

/* CDC PSTN subclass requests */
#define CDC_REQTYPE_OUT             (USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_OUT)
#define CDC_SET_LINE_CODING         0x20
#define CDC_SET_CONTROL_LINE_STATE  0x22
#define CDC_SEND_BREAK              0x23

#define CDC_CONTROL_LINE_DTR        0x01
#define CDC_CONTROL_LINE_RTS        0x02
#define CDC_BREAK_ON                0xFFFF
#define CDC_BREAK_OFF               0x0000

#define CDC_STOP_BITS_2             2
#define CDC_PARITY_NONE             0
#define CDC_DMX_BAUD_RATE           250000

#define CDC_IO_TIMEOUT_MS           1000
/* Same wait as QtSerialInterface::read */
#define CDC_READ_TIMEOUT_MS         10

static const QVector<int> s_cdcIds =
{
    DMXInterface::NXPVID,       DMXInterface::DMXKINGMAXPID,
    DMXInterface::MICROCHIPVID, DMXInterface::EUROLITEPID,
    DMXInterface::ATMELVID,     DMXInterface::NANODMXPID
};

AndroidCDCInterface::AndroidCDCInterface(const QString& serial, const QString& name, const QString& vendor,
                                         quint16 VID, quint16 PID, quint32 id, const QString& path)
    : DMXInterface(serial, name, vendor, VID, PID, id)
    , m_path(path)
    , m_isOpen(false)
{
}

AndroidCDCInterface::~AndroidCDCInterface()
{
    if (isOpen() == true)
        close();
}

DMXInterface::Type AndroidCDCInterface::type() const
{
    return DMXInterface::AndroidCDC;
}

QString AndroidCDCInterface::typeString() const
{
    return "Android CDC";
}

QList<DMXInterface *> AndroidCDCInterface::interfaces(QList<DMXInterface *> discoveredList)
{
    QList<DMXInterface *> interfacesList;
    int id = 0;

    /* One serial port per CDC data interface */
    const QList<AndroidUSB::DeviceInfo> devices = AndroidUSB::enumerate(USB_CLASS_CDC_DATA, s_cdcIds);
    for (const AndroidUSB::DeviceInfo &dev : devices)
    {
        qDebug() << "[AndroidCDCInterface] Serial: " << dev.serial << "name:" << dev.product <<
                    "vendor:" << dev.manufacturer << "VID:" << QString::number(dev.vendorID, 16) <<
                    "PID:" << QString::number(dev.productID, 16) << "interface:" << dev.interfaceNumber;

        bool found = false;
        for (int c = 0; c < discoveredList.count(); c++)
        {
            if (discoveredList.at(c)->checkInfo(dev.serial, dev.product, dev.manufacturer) == true)
            {
                found = true;
                break;
            }
        }

        if (found == false)
            interfacesList << new AndroidCDCInterface(dev.serial, dev.product, dev.manufacturer,
                                                      dev.vendorID, dev.productID, id++, dev.path);
    }

    return interfacesList;
}

void AndroidCDCInterface::setHotplugListener(QObject *listener)
{
    AndroidUSB::addListener(listener, s_cdcIds);
}

bool AndroidCDCInterface::classRequest(quint8 request, quint16 value, void *data, quint16 length)
{
    if (AndroidUSB::controlTransfer(m_handle, CDC_REQTYPE_OUT, request, value, m_handle.commInterface,
                                    data, length, CDC_IO_TIMEOUT_MS) < 0)
    {
        qWarning() << Q_FUNC_INFO << name() << "request" << request << "failed, errno:" << errno;
        return false;
    }

    return true;
}

bool AndroidCDCInterface::setLineCoding()
{
    unsigned char coding[7] =
    {
        CDC_DMX_BAUD_RATE & 0xFF, (CDC_DMX_BAUD_RATE >> 8) & 0xFF,
        (CDC_DMX_BAUD_RATE >> 16) & 0xFF, (CDC_DMX_BAUD_RATE >> 24) & 0xFF,
        CDC_STOP_BITS_2, CDC_PARITY_NONE, 8
    };

    return classRequest(CDC_SET_LINE_CODING, 0, coding, sizeof(coding));
}

bool AndroidCDCInterface::open()
{
    if (isOpen() == true)
        return true;

    if (AndroidUSB::open(m_path, m_handle) == false)
        return false;

    if (m_handle.epIn == 0 || m_handle.epOut == 0 || m_handle.commInterface < 0)
    {
        qWarning() << Q_FUNC_INFO << name() << "not a CDC-ACM data interface";
        AndroidUSB::close(m_path);
        return false;
    }

    if (m_handle.inMaxPacket <= 0)
        m_handle.inMaxPacket = 64;

    m_isOpen = true;
    m_readBuffer.clear();

    /* Like a tty open: assert DTR and RTS */
    classRequest(CDC_SET_CONTROL_LINE_STATE, CDC_CONTROL_LINE_DTR | CDC_CONTROL_LINE_RTS);

    return true;
}

bool AndroidCDCInterface::openByPID(const int FTDIPID)
{
    Q_UNUSED(FTDIPID)
    return open();
}

bool AndroidCDCInterface::close()
{
    if (m_isOpen)
        AndroidUSB::close(m_path);

    m_isOpen = false;
    m_handle = AndroidUSB::Handle();
    m_readBuffer.clear();

    return true;
}

bool AndroidCDCInterface::isOpen() const
{
    return m_isOpen;
}

bool AndroidCDCInterface::reset()
{
    m_readBuffer.clear();
    return isOpen();
}

bool AndroidCDCInterface::setLineProperties()
{
    return setLineCoding();
}

bool AndroidCDCInterface::setBaudRate()
{
    return setLineCoding();
}

bool AndroidCDCInterface::setFlowControl()
{
    return isOpen();
}

bool AndroidCDCInterface::setLowLatency(bool lowLatency)
{
    Q_UNUSED(lowLatency)
    return true;
}

bool AndroidCDCInterface::clearRts()
{
    return classRequest(CDC_SET_CONTROL_LINE_STATE, CDC_CONTROL_LINE_DTR);
}

bool AndroidCDCInterface::purgeBuffers()
{
    m_readBuffer.clear();
    return isOpen();
}

bool AndroidCDCInterface::setBreak(bool on)
{
    return classRequest(CDC_SEND_BREAK, on ? CDC_BREAK_ON : CDC_BREAK_OFF);
}

bool AndroidCDCInterface::write(const QByteArray& data)
{
    if (isOpen() == false)
        return false;

    int res = AndroidUSB::bulkTransfer(m_handle, m_handle.epOut, (void *)data.constData(),
                                       data.size(), CDC_IO_TIMEOUT_MS);
    if (res != data.size())
    {
        qWarning() << Q_FUNC_INFO << name() << "write failed:" << res << "errno:" << errno;
        return false;
    }

    return true;
}

QByteArray AndroidCDCInterface::read(int size)
{
    if (isOpen() == false)
        return QByteArray();

    /* Like QtSerialInterface: wait a little for data and return what's available.
       Read one packet at a time: a timed out transfer loses its partial data */
    if (m_readBuffer.isEmpty())
    {
        QByteArray packet(m_handle.inMaxPacket, 0);
        int res = AndroidUSB::bulkTransfer(m_handle, m_handle.epIn, packet.data(),
                                           packet.size(), CDC_READ_TIMEOUT_MS);
        if (res > 0)
            m_readBuffer.append(packet.constData(), res);
    }

    QByteArray data = m_readBuffer.left(size);
    m_readBuffer.remove(0, data.size());

    return data;
}

uchar AndroidCDCInterface::readByte(bool* ok)
{
    QByteArray data = read(1);

    if (ok)
        *ok = (data.size() == 1);

    return data.isEmpty() ? 0 : uchar(data.at(0));
}
