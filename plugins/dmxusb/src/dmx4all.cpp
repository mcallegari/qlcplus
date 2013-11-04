/*
  Q Light Controller Plus
  dmx4all.cpp

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

#include "dmx4all.h"

#include <QDebug>

DMX4ALL::DMX4ALL(const QString& serial, const QString& name,
                       const QString &vendor, QLCFTDI *ftdi, quint32 id)
    : DMXUSBWidget(serial, name, vendor, ftdi, id)
{
}

DMX4ALL::~DMX4ALL()
{
}

DMXUSBWidget::Type DMX4ALL::type() const
{
    return DMXUSBWidget::DMX4ALL;
}

bool DMX4ALL::checkReply()
{
    uchar reply = ftdi()->readByte();
    //qDebug() << Q_FUNC_INFO << "Reply: " << QString::number(reply[0], 16);
    if (reply == 'G')
    {
        qDebug() << Q_FUNC_INFO << name() << "Good connection.";
        return true;
    }

    qWarning() << Q_FUNC_INFO << name() << "Response failed (got: " << reply << ")";
    return false;
}

bool DMX4ALL::sendChannelValue(int channel, uchar value)
{
    QByteArray chanMsg;
    QString msg;
    chanMsg.append(msg.sprintf("C%03dL%03d", channel, value));
    return ftdi()->write(chanMsg);
}

/****************************************************************************
 * Open & Close
 ****************************************************************************/

bool DMX4ALL::open()
{
    if (isOpen() == true)
        close();

    if (ftdi()->openByPID(QLCFTDI::DMX4ALLPID) == false)
        return close();

    if (ftdi()->reset() == false)
        return close();

    if (ftdi()->setBaudRate() == false)
        return close();

    if (ftdi()->setLineProperties() == false)
        return close();

    if (ftdi()->setFlowControl() == false)
        return close();

    if (ftdi()->purgeBuffers() == false)
        return close();

    if (ftdi()->clearRts() == false)
       return close();

    QByteArray initSequence;

    /* Check connection */
    initSequence.append("C?");
    if (ftdi()->write(initSequence) == true)
    {
        if (checkReply() == false)
            return false;
    }
    else
        qWarning() << Q_FUNC_INFO << name() << "Initialization failed";

    /* set the DMX OUT channels number */
    initSequence.clear();
    initSequence.append("N511");
    if (ftdi()->write(initSequence) == true)
    {
        if (checkReply() == false)
            return false;
    }

    return true;
}

QString DMX4ALL::uniqueName() const
{
    return QString("%1").arg(name());
}

/****************************************************************************
 * Name & Serial
 ****************************************************************************/

QString DMX4ALL::additionalInfo() const
{
    QString info;

    info += QString("<P>");
    info += QString("<B>%1:</B> %2 (%3)").arg(QObject::tr("Protocol"))
                                         .arg("DMX4ALL DMX-USB")
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

bool DMX4ALL::writeUniverse(const QByteArray& universe)
{
    if (isOpen() == false)
        return false;

    /* Since the DMX4ALL array transfer protocol can handle bulk transfer of
     * a maximum of 256 channels, I need to split a 512 universe into 2 */

    QByteArray arrayTransfer(universe);
    arrayTransfer.prepend(char(0xFF));
    arrayTransfer.prepend(char(0x00));        // Start channel low byte
    arrayTransfer.prepend(char(0x00));        // Start channel high byte
    if (universe.size() < 256)
    {
        arrayTransfer.prepend(universe.size());   // Number of channels
    }
    else
    {
        arrayTransfer.prepend(char(0xFF));   // First 256 channels
        arrayTransfer.insert(259, char(0xFF));
        arrayTransfer.insert(260, char(0x00));
        arrayTransfer.insert(261, char(0x01));
    }

    if (ftdi()->write(arrayTransfer) == false)
    {
        qWarning() << Q_FUNC_INFO << name() << "will not accept DMX data";
        return false;
    }
    else
    {
        return true;
    }
}


