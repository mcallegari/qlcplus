/*
  Q Light Controller
  enttecdmxusbprotx.cpp

  Copyright (C) Heikki Junnila
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

#include <QByteArray>
#include <QDebug>

#include "enttecdmxusbprotx.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

EnttecDMXUSBProTX::EnttecDMXUSBProTX(const QString& serial, const QString& name, const QString &vendor,
                                     int port, QLCFTDI *ftdi, quint32 id)
    : EnttecDMXUSBPro(serial, name, vendor, ftdi, id)
    , m_port(port)
{
}

EnttecDMXUSBProTX::~EnttecDMXUSBProTX()
{
}

DMXUSBWidget::Type EnttecDMXUSBProTX::type() const
{
    if (EnttecDMXUSBPro::name().toUpper().contains("PRO MK2") == true)
        return DMXUSBWidget::ProMk2;
    else
    return DMXUSBWidget::ProTX;
}



/****************************************************************************
 * Open & Close
 ****************************************************************************/

bool EnttecDMXUSBProTX::open()
{
    if (DMXUSBWidget::open() == false)
        return close();

    if (ftdi()->clearRts() == false)
        return close();

    if (configurePort(m_port) == false)
        return close();

    return true;
}

QString EnttecDMXUSBProTX::uniqueName() const
{
    return QString("%1 - %2 %3").arg(name()).arg(QObject::tr("Output")).arg(m_port);
}

bool EnttecDMXUSBProTX::configurePort(int port)
{
    qDebug() << "EnttecDMXUSBProTX: Configuring port: " << port;
    if (port == 2)
    {
        QByteArray request;
        request.append(ENTTEC_PRO_START_OF_MSG); // DMX start code (Which constitutes the + 1 below)
        request.append(ENTTEC_PRO_ENABLE_API2); // Enable API2
        request.append(char(0x04)); // data length LSB
        request.append(ENTTEC_PRO_DMX_ZERO); // data length MSB
        request.append(char(0xAD)); // Magic number
        request.append(char(0x88)); // Magic number
        request.append(char(0xD0)); // Magic number
        request.append(char(0xC8)); // Magic number
        request.append(ENTTEC_PRO_END_OF_MSG); // Stop byte

        /* Write "Set API Key Request" message */
        if (ftdi()->write(request) == false)
        {
            qWarning() << Q_FUNC_INFO << name() << "FTDI write filed (second port)";
            return false;
        }

        request.clear();
        request.append(ENTTEC_PRO_START_OF_MSG);
        request.append(ENTTEC_PRO_PORT_ASS_REQ);
        request.append(char(0x02)); // data length LSB - 2 bytes
        request.append(ENTTEC_PRO_DMX_ZERO); // data length MSB
        request.append(char(0x01)); // Port 1 enabled for DMX and RDM
        request.append(char(0x01)); // Port 2 enabled for DMX and RDM
        request.append(ENTTEC_PRO_END_OF_MSG); // Stop byte

        /* Write "Set Port Assignment Request" message */
        if (ftdi()->write(request) == false)
        {
            qWarning() << Q_FUNC_INFO << name() << "FTDI write filed (port config)";
            return false;
        }
    }

    return true;
}

/****************************************************************************
 * Name & Serial
 ****************************************************************************/

QString EnttecDMXUSBProTX::additionalInfo() const
{
    QString info;

    info += QString("<P>");
    info += QString("<B>%1:</B> %2 (%3)").arg(QObject::tr("Protocol"))
                                         .arg("Enttec DMX USB Pro Tx")
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

bool EnttecDMXUSBProTX::writeUniverse(const QByteArray& universe)
{
    if (isOpen() == false)
        return false;

    QByteArray request(universe);
    request.prepend(char(ENTTEC_PRO_DMX_ZERO)); // DMX start code (Which constitutes the + 1 below)
    request.prepend(((universe.size() + 1) >> 8) & 0xff); // Data length MSB
    request.prepend((universe.size() + 1) & 0xff); // Data length LSB

    if (m_port == 2)
        request.prepend(ENTTEC_PRO_SEND_DMX_RQ2); // Command - second port
    else
        request.prepend(ENTTEC_PRO_SEND_DMX_RQ); // Command - first port

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
