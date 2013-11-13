/*
  Q Light Controller
  ultradmxusbprotx.cpp

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

#include <QDebug>

#include "ultradmxusbprotx.h"

UltraDMXUSBProTx::UltraDMXUSBProTx(const QString& serial, const QString& name, const QString &vendor,
                                   int port, QLCFTDI *ftdi, quint32 id)
    : EnttecDMXUSBPro(serial, name, vendor, ftdi, id)
    , m_port(port)
{
}

UltraDMXUSBProTx::~UltraDMXUSBProTx()
{
}

DMXUSBWidget::Type UltraDMXUSBProTx::type() const
{
    return DMXUSBWidget::UltraProTx;
}

/****************************************************************************
 * Open & Close
 ****************************************************************************/

bool UltraDMXUSBProTx::open()
{
    qDebug() << Q_FUNC_INFO << "port: " << m_port;
    if (DMXUSBWidget::open() == false)
        return close();

    if (ftdi()->clearRts() == false)
        return close();

    return true;
}

QString UltraDMXUSBProTx::uniqueName() const
{
    if (realName().isEmpty())
        return QString("%1 - %2 %3").arg(name()).arg(QObject::tr("Output")).arg(m_port);
    else
        return QString("%1 - %2 %3").arg(realName()).arg(QObject::tr("Output")).arg(m_port);
}

/****************************************************************************
 * Name & Serial
 ****************************************************************************/

QString UltraDMXUSBProTx::additionalInfo() const
{
    QString info;

    info += QString("<P>");
    info += QString("<B>%1:</B> %2 (%3)").arg(QObject::tr("Protocol"))
                                         .arg("ultraDMX USB Pro Tx")
                                         .arg(QObject::tr("Output"));
    info += QString("<BR>");
    info += QString("<B>%1:</B> %2").arg(QObject::tr("Manufacturer"))
                                         .arg(vendor());
    info += QString("<BR>");
    info += QString("<B>%1:</B> %2").arg(QObject::tr("Serial number"))
                                                 .arg(serial());
    info += QString("</P>");

    return info;
}

/****************************************************************************
 * Write universe data
 ****************************************************************************/

bool UltraDMXUSBProTx::writeUniverse(const QByteArray& universe)
{
    if (isOpen() == false)
        return false;

    QByteArray request(universe);
    request.prepend(char(ENTTEC_PRO_DMX_ZERO)); // DMX start code (Which constitutes the + 1 below)
    request.prepend(((universe.size() + 1) >> 8) & 0xff); // Data length MSB
    request.prepend((universe.size() + 1) & 0xff); // Data length LSB

    if (m_port == 2)
        request.prepend(SEND_DMX_PORT2); // Command - second port
    else
        request.prepend(SEND_DMX_PORT1); // Command - first port

    request.prepend(ENTTEC_PRO_START_OF_MSG); // Start byte
    request.append(ENTTEC_PRO_END_OF_MSG); // Stop byte

    /* Write "Output Only Send DMX Packet Request" message */
    if (ftdi()->write(request) == false)
    {
        qWarning() << Q_FUNC_INFO << name() << "will not accept DMX data";
        return false;
    }
    else
    {
        return true;
    }
}
