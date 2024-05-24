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

VinceUSBDMX512::VinceUSBDMX512(DMXInterface *iface, quint32 outputLine)
    : QThread(NULL)
    , DMXUSBWidget(iface, outputLine, DEFAULT_OUTPUT_FREQUENCY)
    , m_running(false)
{
    // TODO: Check if DMX IN is available
}

VinceUSBDMX512::~VinceUSBDMX512()
{
    stopOutputThread();
}

DMXUSBWidget::Type VinceUSBDMX512::type() const
{
    return DMXUSBWidget::VinceTX;
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

    if (iface()->clearRts() == false)
        return false;

    // Write two null bytes
    if (iface()->write(QByteArray(2, 0x00)) == false)
        return false;

    QByteArray startSequence;

    startSequence.append(QByteArray(2, VINCE_START_OF_MSG));
    startSequence.append(VINCE_CMD_START_DMX);
    startSequence.append(QByteArray(2, 0x00));
    startSequence.append(VINCE_END_OF_MSG);

    if (iface()->write(startSequence) == false)
        qWarning() << Q_FUNC_INFO << name() << "START command failed";

    start();

    return true;
}

bool VinceUSBDMX512::close(quint32 line, bool input)
{
    Q_UNUSED(input)

    stopOutputThread();

    QByteArray stopSequence;

    stopSequence.append(QByteArray(2, VINCE_START_OF_MSG));
    stopSequence.append(VINCE_CMD_STOP_DMX);
    stopSequence.append(QByteArray(2, 0x00));
    stopSequence.append(VINCE_END_OF_MSG);

    if (iface()->write(stopSequence) == false)
        qWarning() << Q_FUNC_INFO << name() << "STOP command failed";

    return DMXUSBWidget::close(line);
}

/****************************************************************************
 * Inputs
 ****************************************************************************/

int readData(DMXInterface *iface, QByteArray &payload)
{
    bool ok;
    char byte;
    ushort dataLength = 0;

    // Read headers
    for (int i = 0; i < 6; i++)
    {
        byte = iface->readByte(&ok);

        if (ok == false)
            return 0;

        // Retrieve response (4th byte)
        if (i == 3)
        {
            if (byte != VINCE_RESP_OK)
            {
                qWarning() << Q_FUNC_INFO << "Unable to find start of next message";
                return 0;
            }
        }
        // Retrieve data length (5th & 6th bytes)
        else if (i == 4)
            dataLength = ushort(byte) * 256;
        else if (i == 5)
            dataLength += ushort(byte);
    }

    if (dataLength > 0)
    {
        qDebug() << Q_FUNC_INFO << "Attempt to read" << dataLength << "bytes";

        // Read the whole payload
        payload.clear();
        payload = iface->read(dataLength);
    }

    // Read end of message
    if ((byte = iface->readByte()) != VINCE_END_OF_MSG)
        qWarning() << Q_FUNC_INFO << "Incorrect end of message received:" << byte;

    return dataLength;
}

/****************************************************************************
 * Outputs
 ****************************************************************************/

bool VinceUSBDMX512::writeUniverse(quint32 universe, quint32 output, const QByteArray& data, bool dataChanged)
{
    Q_UNUSED(universe)
    Q_UNUSED(output)

    if (isOpen() == false)
        return false;

    if (m_outputLines[0].m_universeData.size() == 0)
    {
        m_outputLines[0].m_universeData.append(data);
        m_outputLines[0].m_universeData.append(DMX_CHANNELS - data.size(), 0);
    }

    if (dataChanged)
        m_outputLines[0].m_universeData.replace(0, data.size(), data);

    return true;
}

void VinceUSBDMX512::stopOutputThread()
{
    if (isRunning() == true)
    {
        m_running = false;
        wait();
    }
}

void VinceUSBDMX512::run()
{
    qDebug() << "OUTPUT thread started";

    QElapsedTimer timer;

    m_running = true;

    while (m_running == true)
    {
        timer.restart();

        int dataLen = m_outputLines[0].m_universeData.length();

        if (dataLen > 0)
        {
            QByteArray request;
            request.append(QByteArray(2, VINCE_START_OF_MSG));  // Start byte
            request.append(VINCE_CMD_UPDATE_DMX);               // Command
            request.append(int((dataLen + 2) / 256));           // Data length
            request.append(int((dataLen + 2) % 256));
            request.append(QByteArray(2, 0x00));                // Gap with data
            request.append(m_outputLines[0].m_universeData);
            request.append(VINCE_END_OF_MSG);                   // Stop byte

            if (iface()->write(request) == false)
                qWarning() << Q_FUNC_INFO << name() << "Will not accept DMX data";
            else
            {
                QByteArray reply;

                if (readData(iface(), reply) > 0)
                    qWarning() << Q_FUNC_INFO << name() << "Invalid response";
            }
        }

        int timetoSleep = m_frameTimeUs - (timer.nsecsElapsed() / 1000);
        if (timetoSleep < 0)
            qWarning() << "DMX output is running late !";
        else
            usleep(timetoSleep);
    }

    qDebug() << "OUTPUT thread terminated";
}

/****************************************************************************
 * Serial & name
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
