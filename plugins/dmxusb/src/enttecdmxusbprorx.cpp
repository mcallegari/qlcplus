/*
  Q Light Controller
  enttecdmxusbprorx.cpp

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

#include <QDebug>
#include "enttecdmxusbprorx.h"

EnttecDMXUSBProRX::EnttecDMXUSBProRX(const QString& serial, const QString& name, const QString &vendor,
                                     quint32 input, QLCFTDI *ftdi, quint32 id)
    : QThread(NULL)
    , EnttecDMXUSBPro(serial, name, vendor, ftdi, id)
    , m_input(input)
    , m_running(false)
    , m_universe(QByteArray(512, char(0)))
{
    qDebug() << Q_FUNC_INFO;
}

EnttecDMXUSBProRX::~EnttecDMXUSBProRX()
{
    qDebug() << Q_FUNC_INFO;
    stop();
}

DMXUSBWidget::Type EnttecDMXUSBProRX::type() const
{
    return DMXUSBWidget::ProRX;
}

QString EnttecDMXUSBProRX::uniqueName() const
{
    if (realName().isEmpty())
        return QString("%1 - %2").arg(name()).arg(QObject::tr("Input"));
    else
        return QString("%1 - %2").arg(realName()).arg(QObject::tr("Input"));
}

/****************************************************************************
 * Open & Close
 ****************************************************************************/

bool EnttecDMXUSBProRX::open()
{
    qDebug() << Q_FUNC_INFO;

    if (EnttecDMXUSBPro::open() == false)
        return close();

    start();
    return true;
}

bool EnttecDMXUSBProRX::close()
{
    qDebug() << Q_FUNC_INFO;
    stop();
    return DMXUSBWidget::close();
}

/****************************************************************************
 * Name & Serial
 ****************************************************************************/

QString EnttecDMXUSBProRX::additionalInfo() const
{
    QString info;

    info += QString("<P>");
    info += QString("<B>%1:</B> %2 (%3)").arg(QObject::tr("Protocol"))
                                         .arg("DMX USB Pro Rx")
                                         .arg(QObject::tr("Input"));
    info += QString("<BR>");
    info += QString("<B>%1:</B> %2").arg(QObject::tr("Manufacturer"))
                                         .arg(vendor());
    info += QString("<BR>");
    info += QString("<B>%1:</B> %2").arg(QObject::tr("Serial number"))
                                                 .arg(serial());
    info += QString("</P>");

    return info;
}

void EnttecDMXUSBProRX::stop()
{
    qDebug() << Q_FUNC_INFO;

    m_running = false;
    wait();
}

void EnttecDMXUSBProRX::run()
{
    qDebug() << Q_FUNC_INFO << "begin";

    uchar byte = 0;
    ushort dataLength = 0;

    m_running = true;
    while (m_running == true)
    {
        bool ok = false;
        // Skip bytes until we find the start of the next message
        if ( (byte = ftdi()->readByte(&ok)) != ENTTEC_PRO_START_OF_MSG)
        {
            // If nothing was read, sleep for a while
            if (ok == false)
                msleep(10);
            continue;
        }

        // Check that the message is a "DMX receive packet"
        byte = ftdi()->readByte();
        if (byte != ENTTEC_PRO_RECV_DMX_PKT)
        {
            qWarning() << Q_FUNC_INFO << "Got label:" << (uchar) byte
                       << "but expected:" << (uchar) ENTTEC_PRO_RECV_DMX_PKT;
            continue;
        }

        // Get payload length
        dataLength = (ushort) ftdi()->readByte() | ((ushort) ftdi()->readByte() << 8);

        // Check status bytes
        byte = ftdi()->readByte();
        if (byte & char(0x01))
            qWarning() << Q_FUNC_INFO << "Widget receive queue overflowed";
        else if (byte & char(0x02))
            qWarning() << Q_FUNC_INFO << "Widget receive overrun occurred";

        // Check DMX startcode
        byte = ftdi()->readByte();
        if (byte != char(0))
            qWarning() << Q_FUNC_INFO << "Non-standard DMX startcode received:" << (uchar) byte;

        // Read payload bytes
        ushort i = 0;
        for (i = 0; i < dataLength; i++)
        {
            byte = ftdi()->readByte();
            if (byte == (uchar) ENTTEC_PRO_END_OF_MSG)
            {
                // Stop when the end of message is received
                break;
            }
            else if (byte != (uchar) m_universe[i])
            {
                // Store and emit changed values
                m_universe[i] = byte;
                emit valueChanged(m_input, i, byte);
            }
        }
    }

    qDebug() << Q_FUNC_INFO << "end";
}
