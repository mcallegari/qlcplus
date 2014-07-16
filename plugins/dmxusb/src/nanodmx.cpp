/*
  Q Light Controller Plus
  nanodmx.cpp

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

#include "nanodmx.h"

#include <QDebug>

NanoDMX::NanoDMX(const QString& serial, const QString& name,
                       const QString &vendor, QLCFTDI *ftdi, quint32 id)
    : DMXUSBWidget(serial, name, vendor, ftdi, id)
{
}

NanoDMX::~NanoDMX()
{
    if (m_file.isOpen() == true)
        m_file.close();
}

DMXUSBWidget::Type NanoDMX::type() const
{
    return DMXUSBWidget::DMX4ALL;
}

bool NanoDMX::checkReply()
{
    QByteArray reply = m_file.readAll();
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

bool NanoDMX::sendChannelValue(int channel, uchar value)
{
    QByteArray chanMsg;
    QString msg;
    chanMsg.append(msg.sprintf("C%03dL%03d", channel, value));
    return ftdi()->write(chanMsg);
}

/****************************************************************************
 * Open & Close
 ****************************************************************************/

bool NanoDMX::open()
{
    // !!! This is buggy !!! Need to autodetect the
    // right device name for this device
    m_file.setFileName("/dev/ttyACM0");

    m_file.unsetError();
    if (m_file.open(QIODevice::ReadWrite | QIODevice::Unbuffered) == false)
    {
        qWarning() << "NanoDMX output cannot be opened:"
                   << m_file.errorString();
        return false;
    }

    QByteArray initSequence;

    /* Check connection */
    initSequence.append("C?");
    if (m_file.write(initSequence) == true)
    {
        if (checkReply() == false)
            return false;
    }
    else
        qWarning() << Q_FUNC_INFO << name() << "Initialization failed";

    /* set the DMX OUT channels number */
    initSequence.clear();
    initSequence.append("N511");
    if (m_file.write(initSequence) == true)
    {
        if (checkReply() == false)
            return false;
    }

    return true;
}

bool NanoDMX::close()
{
    if (m_file.isOpen() == true)
        m_file.close();

    return true;
}

QString NanoDMX::uniqueName() const
{
    return QString("%1").arg(name());
}

/****************************************************************************
 * Name & Serial
 ****************************************************************************/

QString NanoDMX::additionalInfo() const
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

bool NanoDMX::writeUniverse(const QByteArray& universe)
{
    if (m_file.isOpen() == false)
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

    if (m_file.write(arrayTransfer) == false)
    {
        qWarning() << Q_FUNC_INFO << name() << "will not accept DMX data";
        return false;
    }
    else
    {
        return true;
    }
}


