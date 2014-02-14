/*
  Q Light Controller
  vinceusbdmx512tx.cpp

  Copyright (C) Heikki Junnila
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

#include <QByteArray>
#include <QDebug>

#include "vinceusbdmx512tx.h"

VinceUSBDMX512TX::VinceUSBDMX512TX(const QString& serial, const QString& name, const QString &vendor,
                                   QLCFTDI *ftdi, quint32 id)
    : VinceUSBDMX512(serial, name, vendor, ftdi, id)
{
}

VinceUSBDMX512TX::~VinceUSBDMX512TX()
{
}

DMXUSBWidget::Type VinceUSBDMX512TX::type() const
{
    return DMXUSBWidget::VinceTX;
}

/****************************************************************************
 * Name & Serial
 ****************************************************************************/

QString VinceUSBDMX512TX::additionalInfo() const
{
    QString info;

    info += QString("<P>");
    info += QString("<B>%1:</B> %2 (%3)").arg(QObject::tr("Protocol"))
                                         .arg("Vince USB-DMX512")
                                         .arg(QObject::tr("Output"));
    info += QString("<BR>");
    info += QString("<B>%1:</B> %2").arg(QObject::tr("Serial number"))
                                    .arg(serial());
    info += QString("</P>");

    return info;
}

/****************************************************************************
 * Write universe
 ****************************************************************************/

bool VinceUSBDMX512TX::writeUniverse(const QByteArray& universe)
{
    if (isOpen() == false)
        return false;

    // Write only if universe has changed
    if (universe == m_universe)
        return true;

    if (this->writeData(VinceUSBDMX512::UpdateDMX, universe) == false)
    {
        qWarning() << Q_FUNC_INFO << name() << "will not accept DMX data";
        return false;
    }
    else
    {
        bool ok = false;
        QByteArray resp = this->readData(&ok);

        // Check the interface reponse
        if (ok == false || resp.size() > 0)
        {
            qWarning() << Q_FUNC_INFO << name() << "doesn't respond properly";
            return false;
        }

        m_universe = universe;
        return true;
    }
}
