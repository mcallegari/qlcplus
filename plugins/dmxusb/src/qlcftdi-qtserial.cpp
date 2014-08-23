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
#include <QDebug>
#include <QMap>

#include "dmxusbwidget.h"
#include "enttecdmxusbprotx.h"
#include "enttecdmxusbprorx.h"
#include "enttecdmxusbopen.h"
#include "ultradmxusbprotx.h"
#include "dmx4all.h"
#include "qlcftdi.h"

QT_USE_NAMESPACE

static QList<QSerialPortInfo> m_serialPortInfoList;

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

QString QLCFTDI::readLabel(const QSerialPortInfo &info, uchar label, int *ESTA_code)
{
    QSerialPort serial;
    serial.setPort(info);

    if (serial.open(QIODevice::ReadWrite) == false)
        return QString();

    serial.setBaudRate(250000);
    serial.setDataBits(QSerialPort::Data8);
    serial.setStopBits(QSerialPort::TwoStop);
    serial.setParity(QSerialPort::NoParity);
    serial.setFlowControl(QSerialPort::NoFlowControl);

    QByteArray request;
    request.append(ENTTEC_PRO_START_OF_MSG);
    request.append(label);
    request.append(ENTTEC_PRO_DMX_ZERO); // data length LSB
    request.append(ENTTEC_PRO_DMX_ZERO); // data length MSB
    request.append(ENTTEC_PRO_END_OF_MSG);

    if (serial.write(request) < 0)
    {
        qDebug() << Q_FUNC_INFO << "Cannot write data to device";
        return QString();
    }

    char *buffer = (char*) malloc(sizeof(char) * 40);
    Q_ASSERT(buffer != NULL);

    QByteArray array;
    //usleep(300000); // give some time to the device to respond
    int read = serial.read(buffer, 40);
    //qDebug() << Q_FUNC_INFO << "Data read: " << read;
    array = QByteArray::fromRawData((char*) buffer, read);

    if (array[0] != ENTTEC_PRO_START_OF_MSG)
        qDebug() << Q_FUNC_INFO << "Reply message wrong start code: " << QString::number(array[0], 16);
    *ESTA_code = (array[5] << 8) | array[4];
    array.remove(0, 6); // 4 bytes of Enttec protocol + 2 of ESTA ID
    array.replace(ENTTEC_PRO_END_OF_MSG, '\0'); // replace Enttec termination with string termination

    //for (int i = 0; i < array.size(); i++)
    //    qDebug() << "-Data: " << array[i];
    serial.close();

    return QString(array);
}

QList <DMXUSBWidget*> QLCFTDI::widgets()
{
    QList <DMXUSBWidget*> widgetList;
    quint32 id = 0;
    quint32 input_id = 0;

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QString serial(info.portName());
        QString name(info.description());
        QString vendor(info.manufacturer());

        QMap <QString,QVariant> types(typeMap());

        qDebug() << "serial: " << serial << "name:" << name << "vendor:" << vendor;

        if (info.vendorIdentifier() != QLCFTDI::FTDIVID)
            continue;
        if (info.productIdentifier() != QLCFTDI::FTDIPID &&
            info.productIdentifier() != QLCFTDI::DMX4ALLPID)
            continue;

        m_serialPortInfoList.append(info);

        if (types.contains(serial) == true)
        {
            // Force a widget with a specific serial to either type
            DMXUSBWidget::Type type = (DMXUSBWidget::Type) types[serial].toInt();
            switch (type)
            {
            case DMXUSBWidget::OpenTX:
                widgetList << new EnttecDMXUSBOpen(serial, name, vendor, id++);
                break;
            case DMXUSBWidget::ProRX:
            {
                EnttecDMXUSBProRX* prorx = new EnttecDMXUSBProRX(serial, name, vendor, input_id++);
                widgetList << prorx;
                break;
            }
            case DMXUSBWidget::ProMk2:
            {
                EnttecDMXUSBProTX* protx = new EnttecDMXUSBProTX(serial, name, vendor, 1, NULL, id);
                widgetList << protx;
                widgetList << new EnttecDMXUSBProTX(serial, name, vendor, 2, protx->ftdi(), id);
                EnttecDMXUSBProRX* prorx = new EnttecDMXUSBProRX(serial, name, vendor, input_id++, protx->ftdi(), id++);
                widgetList << prorx;
                break;
            }
            case DMXUSBWidget::UltraPro:
            {
                UltraDMXUSBProTx* protx = new UltraDMXUSBProTx(serial, name, vendor, 1, NULL, id);
                widgetList << protx;
                widgetList << new UltraDMXUSBProTx(serial, name, vendor, 2, protx->ftdi(), id);
                EnttecDMXUSBProRX* prorx = new EnttecDMXUSBProRX(serial, name, vendor, input_id++, protx->ftdi(), id++);
                widgetList << prorx;
                break;
            }
            default:
            case DMXUSBWidget::ProTX:
                widgetList << new EnttecDMXUSBProTX(serial, name, vendor, 0, NULL, id++);
                break;
            }
        }
        else if (name.toUpper().contains("PRO MK2") == true)
        {
            EnttecDMXUSBProTX* protx = new EnttecDMXUSBProTX(serial, name, vendor, 1, NULL, id);
            widgetList << protx;
            widgetList << new EnttecDMXUSBProTX(serial, name, vendor, 2, protx->ftdi(), id);
            EnttecDMXUSBProRX* prorx = new EnttecDMXUSBProRX(serial, name, vendor, input_id++, protx->ftdi(), id++);
            widgetList << prorx;
        }
        else if (name.toUpper().contains("DMX USB PRO"))
        {
            /** Check if the device responds to label 77 and 78, so it might be a DMXking adapter */
            int ESTAID = 0;
            int DEVID = 0;
            QString manName = readLabel(info, USB_DEVICE_MANUFACTURER, &ESTAID);
            qDebug() << "--------> Device Manufacturer: " << manName;
            QString devName = readLabel(info, USB_DEVICE_NAME, &DEVID);
            qDebug() << "--------> Device Name: " << devName;
            qDebug() << "--------> ESTA Code: " << QString::number(ESTAID, 16) << ", Device ID: " << QString::number(DEVID, 16);
            if (ESTAID == DMXKING_ESTA_ID)
            {
                if (DEVID == ULTRADMX_PRO_DEV_ID)
                {
                    UltraDMXUSBProTx* protxP1 = new UltraDMXUSBProTx(serial, name, vendor, 1, NULL, id);
                    protxP1->setRealName(devName);
                    widgetList << protxP1;
                    UltraDMXUSBProTx* protxP2 = new UltraDMXUSBProTx(serial, name, vendor, 2, protxP1->ftdi(), id);
                    protxP2->setRealName(devName);
                    widgetList << protxP2;
                    EnttecDMXUSBProRX* prorx = new EnttecDMXUSBProRX(serial, name, vendor, input_id++, protxP1->ftdi(), id++);
                    prorx->setRealName(devName);
                    widgetList << prorx;
                }
                else
                {
                    EnttecDMXUSBProTX* protx = new EnttecDMXUSBProTX(serial, name, vendor, 0, NULL, id++);
                    protx->setRealName(devName);
                    widgetList << protx;
                }
            }
            else
            {
                /* This is probably a Enttec DMX USB Pro widget in TX mode */
                EnttecDMXUSBProTX* protx = new EnttecDMXUSBProTX(serial, name, vendor, 0, NULL, id);
                widgetList << protx;
                EnttecDMXUSBProRX* prorx = new EnttecDMXUSBProRX(serial, name, vendor, input_id++, protx->ftdi(), id++);
                widgetList << prorx;
            }
        }
        else if (info.productIdentifier() == QLCFTDI::DMX4ALLPID)
        {
            widgetList << new Stageprofi(serial, name, vendor, NULL, id++);
        }
        else if (name.toUpper().contains("USB-DMX512 CONVERTER") == true)
        {
            qDebug() << Q_FUNC_INFO << name << "not supported with QtSerialPort yet";
        }
        else
        {
            /* This is probably an Open DMX USB widget */
            widgetList << new EnttecDMXUSBOpen(serial, name, vendor, id++);
        }
    }

    return widgetList;
}

bool QLCFTDI::open()
{
    if (m_openCount < m_refCount)
        m_openCount++;

    if (isOpen() == true)
        return true;

    qDebug() << Q_FUNC_INFO << "Open device ID: " << m_id << "(" << m_serialPortInfoList.at(m_id).description() << ")";

    m_handle = new QSerialPort(m_serialPortInfoList.at(m_id));
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
        return true;
    }
}

bool QLCFTDI::openByPID(const int PID)
{
    Q_UNUSED(PID)

    if (m_openCount < m_refCount)
        m_openCount++;

    if (isOpen() == true)
        return true;

    qDebug() << Q_FUNC_INFO << "Open device ID: " << m_id << "(" << m_serialPortInfoList.at(m_id).description() << ")";

    m_handle = new QSerialPort();
    if (m_handle == NULL)
    {
        qWarning() << Q_FUNC_INFO << name() << "cannot open serial driver";
        return false;
    }
    else
    {
        m_handle->setPortName(m_serialPortInfoList.at(m_id).portName());
        if (m_handle->open(QIODevice::ReadWrite) == false)
        {
            m_handle = NULL;
            return false;
        }
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

    if (m_handle)
    {
        m_handle->close();
        delete m_handle;
        m_handle = NULL;
    }

    return true;
}

bool QLCFTDI::isOpen() const
{
    return (m_handle != NULL) ? true : false;
}

bool QLCFTDI::reset()
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

bool QLCFTDI::setLineProperties()
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

bool QLCFTDI::setBaudRate()
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

bool QLCFTDI::setFlowControl()
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

bool QLCFTDI::clearRts()
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

bool QLCFTDI::purgeBuffers()
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
        write("Start!");
        return true;
    }
}

bool QLCFTDI::setBreak(bool on)
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

bool QLCFTDI::write(const QByteArray& data)
{
    qDebug() << Q_FUNC_INFO;

    if (m_handle == NULL)
        return false;

    if (m_handle->write(data) == false)
    {
        qWarning() << Q_FUNC_INFO << name() << "Error in writing data !!";
        return false;
    }
    else
    {
        return true;
    }
}

QByteArray QLCFTDI::read(int size, uchar* userBuffer)
{
    qDebug() << Q_FUNC_INFO;

    Q_UNUSED(userBuffer)

    if (m_handle == NULL)
        return QByteArray();

    return m_handle->read(size);
}

uchar QLCFTDI::readByte(bool* ok)
{
    if (ok) *ok = false;

    if (m_handle == NULL)
        return 0;

    QByteArray array = m_handle->read(1);
    if (array.size() > 0)
    {
        if (ok) *ok = true;
        return (uchar)array.at(0);
    }
    else
        return 0;
}

void QLCFTDI::modifyRefCount(int amount)
{
    qDebug() << Q_FUNC_INFO;

    m_refCount += amount;
}

int QLCFTDI::refCount()
{
    qDebug() << Q_FUNC_INFO;

    return m_refCount;
}
