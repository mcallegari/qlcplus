/*
  Q Light Controller
  vinceusbdmx512.cpp

  Copyright (C) Jérôme Lebleu

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
#include "vinceusbdmx512.h"

VinceUSBDMX512::VinceUSBDMX512(DMXInterface *interface, quint32 outputLine)
    : DMXUSBWidget(interface, outputLine)
{
    // TODO: Check if DMX IN is available
}

VinceUSBDMX512::~VinceUSBDMX512()
{
}

DMXUSBWidget::Type VinceUSBDMX512::type() const
{
    return DMXUSBWidget::VinceTX;
}

/****************************************************************************
 * Name & Serial
 ****************************************************************************/

QString VinceUSBDMX512::additionalInfo() const
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
 * Open & Close
 ****************************************************************************/

bool VinceUSBDMX512::open(quint32 line, bool input)
{
    Q_UNUSED(line)
    Q_UNUSED(input)

    if (DMXUSBWidget::open() == false)
        return false;

    if (interface()->clearRts() == false)
        return false;

    // Write two null bytes
    if (interface()->write(QByteArray(2, 0x00)) == false)
        return false;

    // Request start DMX command
    return this->writeData(VinceUSBDMX512::StartDMX);
}

bool VinceUSBDMX512::close(quint32 line, bool input)
{
    Q_UNUSED(line)
    Q_UNUSED(input)

    if (isOpen() == false)
        return true;

    // Reqest stop DMX command
    if (this->writeData(VinceUSBDMX512::StopDMX) == true)
        return DMXUSBWidget::close();

    return false;
}

/****************************************************************************
 * Write & Read
 ****************************************************************************/

bool VinceUSBDMX512::writeData(Command command, const QByteArray &data)
{
    QByteArray message(1, command);                     // Command
    message.prepend(QByteArray(2, VINCE_START_OF_MSG)); // Start condition
    if (data.size() == 0)
        message.append(QByteArray(2, 0x00));            // Data length
    else
    {
        message.append(int((data.size() + 2) / 256));   // Data length
        message.append(int((data.size() + 2) % 256));
        message.append(QByteArray(2, 0x00));            // Gap with data
        message.append(data);                           // Data
    }
    message.append(VINCE_END_OF_MSG);                   // Stop condition

    return interface()->write(message);
}

QByteArray VinceUSBDMX512::readData(bool* ok)
{
    uchar byte = 0;
    ushort dataLength = 0;
    QByteArray data = QByteArray();

    // Read headers
    for (int i = 0; i < 6; i++)
    {
        *ok = false;
        // Attempt to read byte
        byte = interface()->readByte(ok);
        if (*ok == false)
            return data;

        // Retrieve response (4th byte)
        if (i == 3 && byte != VINCE_RESP_OK)
        {
            qWarning() << Q_FUNC_INFO << "Error" << byte << "in readed message";
            *ok = false;
        }
        // Retrieve length (5th & 6th bytes)
        else if (i == 4)
            dataLength = ushort(byte) * 256;
        else if (i == 5)
            dataLength += ushort(byte);
    }

    // Read data
    if (dataLength > 0)
    {
        qDebug() << Q_FUNC_INFO << "Attempt to read" << dataLength << "bytes";

        ushort i;
        for (i = 0; i < dataLength; i++)
        {
            byte = interface()->readByte(ok);
            if (*ok == false)
            {
                qWarning() << Q_FUNC_INFO << "No available byte to read (" << (dataLength - i) << "missing bytes)";
                return data;
            }
            data.append(byte);
        }
    }

    // Read end of message
    byte = interface()->readByte();
    if (byte != VINCE_END_OF_MSG)
    {
        qWarning() << Q_FUNC_INFO << "Incorrect end of message received:" << byte;
        *ok = false;
    }

    return data;
}

bool VinceUSBDMX512::writeUniverse(quint32 universe, quint32 output, const QByteArray& data)
{
    Q_UNUSED(universe)
    Q_UNUSED(output)

    if (isOpen() == false)
        return false;

    // Write only if universe has changed
    if (data == m_universe)
        return true;

    if (writeData(VinceUSBDMX512::UpdateDMX, data) == false)
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

        m_universe = data;
        return true;
    }
}
