/*
  Q Light Controller
  enttecdmxusbpro.cpp

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
#include "enttecdmxusbpro.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

EnttecDMXUSBPro::EnttecDMXUSBPro(const QString& serial, const QString& name, const QString &vendor, QLCFTDI *ftdi, quint32 id)
    : DMXUSBWidget(serial, name, vendor, ftdi, id)
{
    // Bypass rts setting by calling parent class' open method
    if (DMXUSBWidget::open() == true)
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
    if (DMXUSBWidget::open() == false)
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
    request.append(ENTTEC_PRO_START_OF_MSG);
    request.append(ENTTEC_PRO_READ_SERIAL);
    request.append(ENTTEC_PRO_DMX_ZERO); // data length LSB
    request.append(ENTTEC_PRO_DMX_ZERO); // data length MSB
    request.append(ENTTEC_PRO_END_OF_MSG);

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
            qDebug() << Q_FUNC_INFO << "Serial number OK: " << m_proSerial;
            return true;
        }
        else
        {
            qWarning() << Q_FUNC_INFO << name() << "gave malformed serial reply:"
                       << QString::number(reply[0], 16) << QString::number(reply[1], 16)
                       << QString::number(reply[2], 16) << QString::number(reply[3], 16)
                       << QString::number(reply[4], 16) << QString::number(reply[5], 16)
                       << QString::number(reply[6], 16) << QString::number(reply[7], 16)
                       << QString::number(reply[8], 16);
            return false;
        }
    }
    else
    {
        qWarning() << Q_FUNC_INFO << name() << "will not accept serial request";
        return false;
    }
}


