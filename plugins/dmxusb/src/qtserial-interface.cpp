/*
  Q Light Controller Plus
  qtserial-interface.cpp

  Copyright (C) Massimo Callegari

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

#include <QCoreApplication>
#include <QSettings>
#include <QVariant>
#include <QDebug>
#include <QMap>

#include "qtserial-interface.h"
#include "enttecdmxusbpro.h"

QT_USE_NAMESPACE

QtSerialInterface::QtSerialInterface(const QString& serial, const QString& name, const QString& vendor,
                                     quint16 VID, quint16 PID, quint32 id)
    : DMXInterface(serial, name, vendor, VID, PID , id)
    , m_handle(NULL)
{

}

QtSerialInterface::~QtSerialInterface()
{
    if (isOpen() == true)
        close();
}

bool QtSerialInterface::readLabel(uchar label, int &intParam, QString &strParam)
{
    QSerialPort serial;
    serial.setPort(m_info);

    if (serial.open(QIODevice::ReadWrite) == false)
        return false;

    serial.setReadBufferSize(1024);

    serial.setDataBits(QSerialPort::Data8);
    serial.setStopBits(QSerialPort::TwoStop);
    serial.setParity(QSerialPort::NoParity);
    serial.setFlowControl(QSerialPort::NoFlowControl);
    serial.setBaudRate(250000);

    QByteArray request;
    request.append(ENTTEC_PRO_START_OF_MSG);
    request.append(label);
    request.append(ENTTEC_PRO_DMX_ZERO); // data length LSB
    request.append(ENTTEC_PRO_DMX_ZERO); // data length MSB
    request.append(ENTTEC_PRO_END_OF_MSG);

    if (serial.write(request) < 0)
    {
        qDebug() << Q_FUNC_INFO << "Cannot write data to device";
        return false;
    }
    serial.waitForBytesWritten(20);

    char buffer[40];
    Q_ASSERT(buffer != NULL);

    QByteArray array;
    // wait 100ms maximum for the device to respond
    serial.waitForReadyRead(100);
    int read = serial.read(buffer, 40);
    //qDebug() << Q_FUNC_INFO << "Data read: " << read;
    array = QByteArray::fromRawData((char*) buffer, read);

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
    array.remove(0, 6); // 4 bytes of Enttec protocol + 2 byte of ESTA ID
    array.replace(ENTTEC_PRO_END_OF_MSG, '\0'); // replace Enttec termination with string termination
    strParam = QString(array);

    //for (int i = 0; i < array.size(); i++)
    //    qDebug() << "-Data: " << array[i];
    serial.close();

    return true;
}

DMXInterface::Type QtSerialInterface::type()
{
    return DMXInterface::QtSerial;
}

QString QtSerialInterface::typeString()
{
    return "QtSerialPort";
}

QList<DMXInterface *> QtSerialInterface::interfaces(QList<DMXInterface *> discoveredList)
{
    QList <DMXInterface*> interfacesList;
    int id = 0;

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QString serial(info.serialNumber());
        QString name(info.description());
        QString vendor(info.manufacturer());

        qDebug() << "[QtSerialInterface] Serial: " << serial << "name:" << name << "vendor:" << vendor;

        // Skip non wanted devices
        if (validInterface(info.vendorIdentifier(), info.productIdentifier()) == false)
            continue;

        if (info.vendorIdentifier() == DMXInterface::FTDIVID)
            continue;

#if defined(Q_OS_MACOS)
        /* Qt 5.6+ reports the same device as "cu" and "tty". Only the first will be considered */
        if (info.portName().startsWith("tty"))
            continue;
#endif

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
            QtSerialInterface *iface = new QtSerialInterface(serial, name, vendor, info.vendorIdentifier(),
                                                             info.productIdentifier(), id++);
            iface->setInfo(info);
            interfacesList << iface;
        }
    }

    return interfacesList;
}

void QtSerialInterface::setInfo(QSerialPortInfo info)
{
    m_info = info;
}

bool QtSerialInterface::open()
{
    if (isOpen() == true)
        return true;

    qDebug() << Q_FUNC_INFO << "Open device ID: " << id() << "(" << m_info.description() << ")";

    m_handle = new QSerialPort(m_info);
    if (m_handle == NULL)
    {
        qWarning() << Q_FUNC_INFO << name() << "cannot create serial driver";
        return false;
    }
    else
    {
        if (m_handle->open(QIODevice::ReadWrite) == false)
        {
            qWarning() << Q_FUNC_INFO << name() << "cannot open serial driver";
            delete m_handle;
            m_handle = NULL;
            return false;
        }

        m_handle->setReadBufferSize(1024);
        qDebug() << "Read buffer size:" << m_handle->readBufferSize() << m_handle->errorString();

        return true;
    }
}

bool QtSerialInterface::openByPID(const int PID)
{
    Q_UNUSED(PID)

    // with QtSerialPort there is no such method,
    // so open a device already discovered
    return open();
}

bool QtSerialInterface::close()
{
    qDebug() << Q_FUNC_INFO;

    if (m_handle)
    {
        m_handle->close();
        delete m_handle;
        m_handle = NULL;
    }

    return true;
}

bool QtSerialInterface::isOpen() const
{
    return (m_handle != NULL) ? true : false;
}

bool QtSerialInterface::reset()
{
    qDebug() << Q_FUNC_INFO;

    if (m_handle->clear() == false)
    {
        qWarning() << Q_FUNC_INFO << name() << "Error in serial reset";
        return false;
    }
    else
    {
        return true;
    }
}

bool QtSerialInterface::setLineProperties()
{
    qDebug() << Q_FUNC_INFO;

    if (m_handle == NULL)
        return false;

    if (m_handle->setDataBits(QSerialPort::Data8) == false)
    {
        qWarning() << Q_FUNC_INFO << name() << "Error in setting data bits property";
        return false;
    }

    if (m_handle->setStopBits(QSerialPort::TwoStop) == false)
    {
        qWarning() << Q_FUNC_INFO << name() << "Error in setting stop bits property";
        return false;
    }

    if (m_handle->setParity(QSerialPort::NoParity) == false)
    {
        qWarning() << Q_FUNC_INFO << name() << "Error in setting parity property";
        return false;
    }

    return true;
}

bool QtSerialInterface::setBaudRate()
{
    qDebug() << Q_FUNC_INFO;

    if (m_handle == NULL)
        return false;

    if (m_handle->setBaudRate(250000) == false)
    {
        qWarning() << Q_FUNC_INFO << name() << "Error in setting line baudrate";
        return false;
    }
    else
    {
        return true;
    }
}

bool QtSerialInterface::setFlowControl()
{
    qDebug() << Q_FUNC_INFO;

    if (m_handle == NULL)
        return false;

    if (m_handle->setFlowControl(QSerialPort::NoFlowControl) == false)
    {
        qWarning() << Q_FUNC_INFO << name() << "Error in setting flow control";
        return false;
    }
    else
    {
        return true;
    }
}

bool QtSerialInterface::setLowLatency(bool lowLatency)
{
    Q_UNUSED(lowLatency)
    return true;
}

bool QtSerialInterface::clearRts()
{
    qDebug() << Q_FUNC_INFO;

    if (m_handle == NULL)
        return false;

    if (m_handle->setRequestToSend(false) == false)
    {
        qWarning() << Q_FUNC_INFO << name() << "Error in setting RTS";
        return false;
    }
    else
    {
        return true;
    }
}

bool QtSerialInterface::purgeBuffers()
{
    qDebug() << Q_FUNC_INFO;

    if (m_handle == NULL)
        return false;

    if (m_handle->clear() == false)
    {
        qWarning() << Q_FUNC_INFO << name() << "Error in flushing buffers";
        return false;
    }
    else
    {
        // kind of a dirty trick ! Without this call no data is transmitted :(
        //write("Start!");
        return true;
    }
}

bool QtSerialInterface::setBreak(bool on)
{
    if (m_handle == NULL)
        return false;

    if (m_handle->setBreakEnabled(on) == false)
    {
        qWarning() << Q_FUNC_INFO << name() << "Error in setting break control";
        return false;
    }
    else
    {
        return true;
    }
}

bool QtSerialInterface::write(const QByteArray& data)
{
    //qDebug() << Q_FUNC_INFO;

    if (m_handle == NULL)
        return false;

    if (m_handle->write(data) == false)
    {
        qWarning() << Q_FUNC_INFO << name() << "Error in writing data !!";
        return false;
    }
    else
    {
        m_handle->waitForBytesWritten(10);
        return true;
    }
}

QByteArray QtSerialInterface::read(int size, uchar* userBuffer)
{
    //qDebug() << Q_FUNC_INFO;

    Q_UNUSED(userBuffer)

    if (m_handle == NULL)
        return QByteArray();

    if (m_handle->waitForReadyRead(10) == true)
    {
        return m_handle->read(size);
    }
    return QByteArray();
}

uchar QtSerialInterface::readByte(bool* ok)
{
    if (ok) *ok = false;

    if (m_handle == NULL)
        return 0;

    //qDebug() << Q_FUNC_INFO;

    if (m_handle->waitForReadyRead(10) == true)
    {
        QByteArray array = m_handle->read(1);
        if (array.size() > 0)
        {
            if (ok) *ok = true;
            return (uchar)array.at(0);
        }
    }

    return 0;
}
