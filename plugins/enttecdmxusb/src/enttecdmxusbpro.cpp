/*
  Q Light Controller
  enttecdmxusbpro.cpp

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

#include <QDebug>
#include "enttecdmxusbpro.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

EnttecDMXUSBPro::EnttecDMXUSBPro(const QString& serial, const QString& name, quint32 id)
    : EnttecDMXUSBWidget(serial, name, id)
{
    // Bypass rts setting by calling parent class' open method
    if (EnttecDMXUSBWidget::open() == true)
        extractSerial();
    close();
}

EnttecDMXUSBPro::~EnttecDMXUSBPro()
{
}

/****************************************************************************
 * Open & Close
 ****************************************************************************/

bool EnttecDMXUSBPro::open()
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

QString EnttecDMXUSBPro::uniqueName() const
{
    if (m_proSerial.isEmpty() == true)
        return QString("%1 (S/N: %2)").arg(name()).arg(serial());
    else
        return QString("%1 (S/N: %2)").arg(name()).arg(m_proSerial);
}

bool EnttecDMXUSBPro::extractSerial()
{
    QByteArray request;
    request.append(char(0x7e));
    request.append(char(0x0a));
    request.append(char(0x00));
    request.append(char(0x00));
    request.append(char(0xe7));
    if (ftdi()->write(request) == true)
    {
        QByteArray reply = ftdi()->read(9);

        /* Reply message is:
           { 0x7E 0x0A 0x04 0x00 0xNN, 0xNN, 0xNN, 0xNN 0xE7 }
           Where 0xNN represent widget's unique serial number in BCD */
        if (uchar(reply[0]) == 0x7e && uchar(reply[1]) == 0x0a &&
            uchar(reply[2]) == 0x04 && uchar(reply[3]) == 0x00 &&
            uchar(reply[8]) == 0xe7)
        {
            m_proSerial.sprintf("%x%.2x%.2x%.2x", uchar(reply[7]),
                                                  uchar(reply[6]),
                                                  uchar(reply[5]),
                                                  uchar(reply[4]));
            return true;
        }
        else
        {
            qWarning() << Q_FUNC_INFO << name() << "gave malformed serial reply:"
                       << int(reply[0]) << int(reply[1]) << int(reply[2])
                       << int(reply[3]) << int(reply[4]) << int(reply[5])
                       << int(reply[6]) << int(reply[7]) << int(reply[8]);
            return false;
        }
    }
    else
    {
        qWarning() << Q_FUNC_INFO << name() << "will not accept serial request";
        return false;
    }
}
