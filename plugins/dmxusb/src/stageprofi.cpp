/*
  Q Light Controller Plus
  stageprofi.cpp

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

#include "stageprofi.h"

#include <QDebug>

Stageprofi::Stageprofi(const QString& serial, const QString& name,
                       const QString &vendor, quint32 id)
    : DMXUSBWidget(serial, name, vendor, id)
{
}

Stageprofi::~Stageprofi()
{
}

DMXUSBWidget::Type Stageprofi::type() const
{
    return DMXUSBWidget::DMX4ALL;
}

bool Stageprofi::checkReply()
{
    QByteArray reply = ftdi()->read(16);
    //qDebug() << Q_FUNC_INFO << "Reply: " << QString::number(reply[0], 16);
    for (int i = 0; i < reply.count(); i++)
    {
        if (reply[i] == 'G')
        {
            qDebug() << Q_FUNC_INFO << name() << "Good connection.";
            return true;
        }
    }

    qWarning() << Q_FUNC_INFO << name() << "Response failed (got: " << reply << ")";
    return false;
}

bool Stageprofi::sendChannelValue(int channel, uchar value)
{
    QByteArray chanMsg;
    QString msg;
    chanMsg.append(msg.sprintf("C%03dL%03d", channel, value));
    return ftdi()->write(chanMsg);
}

/****************************************************************************
 * Open & Close
 ****************************************************************************/

bool Stageprofi::open(quint32 line, bool input)
{
    Q_UNUSED(line)
    Q_UNUSED(input)

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

QString Stageprofi::uniqueName(ushort line, bool input) const
{
    Q_UNUSED(line)
    Q_UNUSED(input)
    return QString("%1").arg(name());
}

/****************************************************************************
 * Name & Serial
 ****************************************************************************/

QString Stageprofi::additionalInfo() const
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

bool Stageprofi::writeUniverse(quint32 universe, quint32 output, const QByteArray& data)
{
    Q_UNUSED(universe)
    Q_UNUSED(output)

    if (isOpen() == false)
        return false;

    /* Since the DMX4ALL array transfer protocol can handle bulk transfer of
     * a maximum of 256 channels, I need to split a 512 universe into 2 */

    QByteArray arrayTransfer(data);
    arrayTransfer.prepend(char(0xFF));
    arrayTransfer.prepend(char(0x00));        // Start channel low byte
    arrayTransfer.prepend(char(0x00));        // Start channel high byte
    if (data.size() < 256)
    {
        arrayTransfer.prepend(data.size());   // Number of channels
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
        ftdi()->purgeBuffers();
        return true;
    }
}


