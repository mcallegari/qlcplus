/*
  Q Light Controller
  ultradmxusbprotx.cpp

  Copyright (C) Massimo Callegari

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
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
