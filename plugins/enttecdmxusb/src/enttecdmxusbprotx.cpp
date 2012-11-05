/*
  Q Light Controller
  enttecdmxusbprotx.cpp

  Copyright (C) Heikki Junnila

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

EnttecDMXUSBProTX::EnttecDMXUSBProTX(const QString& serial, const QString& name, quint32 id)
    : EnttecDMXUSBPro(serial, name, id)
{
}

EnttecDMXUSBProTX::~EnttecDMXUSBProTX()
{
}

EnttecDMXUSBWidget::Type EnttecDMXUSBProTX::type() const
{
    return EnttecDMXUSBWidget::ProTX;
}

/****************************************************************************
 * Open & Close
 ****************************************************************************/

bool EnttecDMXUSBProTX::open()
{
    if (EnttecDMXUSBWidget::open() == false)
        return close();

    if (ftdi()->clearRts() == false)
        return close();

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
                                         .arg("Enttec DMX USB Pro")
                                         .arg(QObject::tr("Output"));
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
    request.prepend(char(ENTTEC_PRO_DMX_STARTCODE)); // DMX start code (Which constitutes the + 1 below)
    request.prepend(((universe.size() + 1) >> 8) & 0xff); // Data length MSB
    request.prepend((universe.size() + 1) & 0xff); // Data length LSB
    request.prepend(ENTTEC_PRO_SEND_DMX_RQ); // Command
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
