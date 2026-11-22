/*
  Q Light Controller Plus
  android-ftdi-interface.cpp

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

#include "android-ftdi-interface.h"

/* FTDI vendor requests (see libftdi / FTDI AN232B-05) */
#define FTDI_REQTYPE_OUT        (USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_DIR_OUT)
#define FTDI_REQTYPE_IN         (USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_DIR_IN)

#define SIO_RESET               0
#define SIO_SET_MODEM_CTRL      1
#define SIO_SET_FLOW_CTRL       2
#define SIO_SET_BAUD_RATE       3
#define SIO_SET_DATA            4
#define SIO_SET_LATENCY_TIMER   9
#define SIO_GET_LATENCY_TIMER   10

#define SIO_RESET_SIO           0
#define SIO_RESET_PURGE_RX      1
#define SIO_RESET_PURGE_TX      2

#define SIO_SET_RTS_LOW         0x0200
#define SIO_DISABLE_FLOW_CTRL   0x0000

/* 8 data bits, no parity, 2 stop bits */
#define FTDI_LINE_8N2           (8 | (0 << 8) | (2 << 11))
#define FTDI_BREAK_ON           (1 << 14)

/* 250000 baud = 3MHz / 12, exact on all chip families */
#define FTDI_DMX_BAUD_DIVISOR   12

#define FTDI_IO_TIMEOUT_MS      1000
#define FTDI_READ_CHUNK_SIZE    4096
#define FTDI_STATUS_BYTES       2

static const QVector<int> s_ftdiIds =
{
    DMXInterface::FTDIVID, DMXInterface::FTDIPID,
    DMXInterface::FTDIVID, DMXInterface::FTDI2PID,
    DMXInterface::FTDIVID, DMXInterface::FTDI4PID,
    DMXInterface::FTDIVID, DMXInterface::DMX4ALLPID
};

AndroidFTDIInterface::AndroidFTDIInterface(const QString& serial, const QString& name, const QString& vendor,
                                           quint16 VID, quint16 PID, quint32 id, const QString& path)
    : DMXInterface(serial, name, vendor, VID, PID, id)
    , m_path(path)
    , m_isOpen(false)
{
    m_defaultLatency = 16;
}

AndroidFTDIInterface::~AndroidFTDIInterface()
{
    if (isOpen() == true)
        close();
}

DMXInterface::Type AndroidFTDIInterface::type() const
{
    return DMXInterface::AndroidFTDI;
}

QString AndroidFTDIInterface::typeString() const
{
    return "Android FTDI";
}

QList<DMXInterface *> AndroidFTDIInterface::interfaces(QList<DMXInterface *> discoveredList)
{
    QList<DMXInterface *> interfacesList;

    /* FTDI chips expose vendor specific interfaces, one per port */
    const QList<AndroidUSB::DeviceInfo> devices = AndroidUSB::enumerate(USB_CLASS_VENDOR_SPEC, s_ftdiIds);
    for (const AndroidUSB::DeviceInfo &dev : devices)
    {
        qDebug() << Q_FUNC_INFO << "DMX USB VID:" << QString::number(dev.vendorID, 16) <<
                    "PID:" << QString::number(dev.productID, 16) << "interface:" << dev.interfaceNumber;
        qDebug() << Q_FUNC_INFO << "DMX USB serial: " << dev.serial << "name:" << dev.product <<
                    "vendor:" << dev.manufacturer;

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
            interfacesList << new AndroidFTDIInterface(dev.serial, dev.product, dev.manufacturer,
                                                       dev.vendorID, dev.productID,
                                                       dev.interfaceNumber, dev.path);
    }

    return interfacesList;
}

void AndroidFTDIInterface::setHotplugListener(QObject *listener)
{
    AndroidUSB::addListener(listener, s_ftdiIds);
}

bool AndroidFTDIInterface::vendorRequest(quint8 request, quint16 value, quint16 index)
{
    if (AndroidUSB::controlTransfer(m_handle, FTDI_REQTYPE_OUT, request, value, index,
                                    NULL, 0, FTDI_IO_TIMEOUT_MS) < 0)
    {
        qWarning() << Q_FUNC_INFO << name() << "request" << request << "failed, errno:" << errno;
        return false;
    }

    return true;
}

/* FTDI port index: 1 = A, 2 = B, ... */
#define FTDI_PORT (m_handle.interfaceNumber + 1)

bool AndroidFTDIInterface::open()
{
    if (isOpen() == true)
        return true;

    if (AndroidUSB::open(m_path, m_handle) == false)
        return false;

    if (m_handle.epIn == 0 || m_handle.epOut == 0)
    {
        qWarning() << Q_FUNC_INFO << name() << "missing bulk endpoints";
        AndroidUSB::close(m_path);
        return false;
    }

    if (m_handle.inMaxPacket <= FTDI_STATUS_BYTES)
        m_handle.inMaxPacket = 64;

    m_isOpen = true;
    m_readBuffer.clear();

    unsigned char latency = 0;
    if (AndroidUSB::controlTransfer(m_handle, FTDI_REQTYPE_IN, SIO_GET_LATENCY_TIMER, 0, FTDI_PORT,
                                    &latency, 1, FTDI_IO_TIMEOUT_MS) == 1)
        m_defaultLatency = latency;
    else
        m_defaultLatency = 16;

    qDebug() << Q_FUNC_INFO << serial() << "Default latency is" << m_defaultLatency;
    return true;
}

bool AndroidFTDIInterface::openByPID(const int FTDIPID)
{
    Q_UNUSED(FTDIPID)
    return open();
}

bool AndroidFTDIInterface::close()
{
    if (m_isOpen)
        AndroidUSB::close(m_path);

    m_isOpen = false;
    m_handle = AndroidUSB::Handle();
    m_readBuffer.clear();

    return true;
}

bool AndroidFTDIInterface::isOpen() const
{
    return m_isOpen;
}

bool AndroidFTDIInterface::reset()
{
    m_readBuffer.clear();
    return vendorRequest(SIO_RESET, SIO_RESET_SIO, FTDI_PORT);
}

bool AndroidFTDIInterface::setLineProperties()
{
    return vendorRequest(SIO_SET_DATA, FTDI_LINE_8N2, FTDI_PORT);
}

bool AndroidFTDIInterface::setBaudRate()
{
    /* Like libftdi: on 2232C/D and H chips (bcdDevice 0x500, 0x700, 0x800, 0x900)
       the divisor high bits go in the wIndex high byte and the low byte is the port.
       On the other chips (BM/232R/X) wIndex only carries the divisor high bits (0 here) */
    quint16 index = 0;
    if (m_handle.bcdDevice == 0x0500 || m_handle.bcdDevice == 0x0700 ||
        m_handle.bcdDevice == 0x0800 || m_handle.bcdDevice == 0x0900)
        index = FTDI_PORT;
    return vendorRequest(SIO_SET_BAUD_RATE, FTDI_DMX_BAUD_DIVISOR, index);
}

bool AndroidFTDIInterface::setFlowControl()
{
    return vendorRequest(SIO_SET_FLOW_CTRL, 0, SIO_DISABLE_FLOW_CTRL | FTDI_PORT);
}

bool AndroidFTDIInterface::setLowLatency(bool lowLatency)
{
    unsigned char latency = lowLatency ? 1 : m_defaultLatency;

    if (vendorRequest(SIO_SET_LATENCY_TIMER, latency, FTDI_PORT) == false)
        return false;

    qDebug() << Q_FUNC_INFO << serial() << "Latency set to" << latency;
    return true;
}

bool AndroidFTDIInterface::clearRts()
{
    return vendorRequest(SIO_SET_MODEM_CTRL, SIO_SET_RTS_LOW, FTDI_PORT);
}

bool AndroidFTDIInterface::purgeBuffers()
{
    m_readBuffer.clear();

    if (vendorRequest(SIO_RESET, SIO_RESET_PURGE_RX, FTDI_PORT) == false)
        return false;

    return vendorRequest(SIO_RESET, SIO_RESET_PURGE_TX, FTDI_PORT);
}

bool AndroidFTDIInterface::setBreak(bool on)
{
    return vendorRequest(SIO_SET_DATA, FTDI_LINE_8N2 | (on ? FTDI_BREAK_ON : 0), FTDI_PORT);
}

bool AndroidFTDIInterface::write(const QByteArray& data)
{
    if (isOpen() == false)
        return false;

    int res = AndroidUSB::bulkTransfer(m_handle, m_handle.epOut, (void *)data.constData(),
                                       data.size(), FTDI_IO_TIMEOUT_MS);
    if (res != data.size())
    {
        qWarning() << Q_FUNC_INFO << name() << "write failed:" << res << "errno:" << errno;
        return false;
    }

    return true;
}

int AndroidFTDIInterface::fillReadBuffer()
{
    const int packetSize = m_handle.inMaxPacket;
    const int chunkSize = (FTDI_READ_CHUNK_SIZE / packetSize) * packetSize;
    QByteArray chunk(chunkSize, 0);

    int res = AndroidUSB::bulkTransfer(m_handle, m_handle.epIn, chunk.data(), chunkSize, FTDI_IO_TIMEOUT_MS);
    if (res < 0)
        return -1;

    /* Each USB packet starts with 2 modem status bytes */
    int payload = 0;
    for (int offset = 0; offset < res; offset += packetSize)
    {
        int len = qMin(packetSize, res - offset) - FTDI_STATUS_BYTES;
        if (len <= 0)
            continue;

        m_readBuffer.append(chunk.constData() + offset + FTDI_STATUS_BYTES, len);
        payload += len;
    }

    return payload;
}

QByteArray AndroidFTDIInterface::read(int size)
{
    if (isOpen() == false)
        return QByteArray();

    /* Like libftdi: keep reading until @size bytes are available
       or the chip has nothing more to send */
    while (m_readBuffer.size() < size)
    {
        if (fillReadBuffer() <= 0)
            break;
    }

    QByteArray data = m_readBuffer.left(size);
    m_readBuffer.remove(0, data.size());

    return data;
}

uchar AndroidFTDIInterface::readByte(bool* ok)
{
    QByteArray data = read(1);

    if (ok)
        *ok = (data.size() == 1);

    return data.isEmpty() ? 0 : uchar(data.at(0));
}
