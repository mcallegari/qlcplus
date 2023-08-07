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

#include <QString>
#include <QDebug>

Stageprofi::Stageprofi(DMXInterface *interface, quint32 outputLine)
    : DMXUSBWidget(interface, outputLine, DEFAULT_OUTPUT_FREQUENCY)
    , m_running(false)
{
}

Stageprofi::~Stageprofi()
{
    stop();
}

DMXUSBWidget::Type Stageprofi::type() const
{
    return DMXUSBWidget::DMX4ALL;
}

bool Stageprofi::checkReply()
{
    bool ok = false;
    uchar res;

    res = iface()->readByte(&ok);
    if (ok == false || res != 0x47)
        return false;

    return true;
/*
    for (int i = 0; i < 16; i++)
    {
        QByteArray reply = ftdi()->read(1);
        //qDebug() << Q_FUNC_INFO << "Reply: " << QString::number(reply[0], 16);

        if (reply.count() > 0 && reply[0] == 'G')
        {
            qDebug() << Q_FUNC_INFO << name() << "Good connection.";
            return true;
        }
    }

    qWarning() << Q_FUNC_INFO << name() << "got no reply";

    return false;
*/
}

bool Stageprofi::sendChannelValue(int channel, uchar value)
{
    QByteArray chanMsg;
    QString msg;
    chanMsg.append(msg.asprintf("C%03dL%03d", channel, value).toUtf8());
    return iface()->write(chanMsg);
}

/****************************************************************************
 * Open & Close
 ****************************************************************************/

bool Stageprofi::open(quint32 line, bool input)
{
    Q_UNUSED(line)
    Q_UNUSED(input)

    if (DMXUSBWidget::open() == false)
        return false;

    QByteArray initSequence;

    /* Check connection */
    initSequence.append("C?");
    if (iface()->write(initSequence) == true)
    {
        if (checkReply() == false)
            qWarning() << Q_FUNC_INFO << name() << "Initialization failed";
    }
    else
        qWarning() << Q_FUNC_INFO << name() << "Initialization failed";

    /* set the DMX OUT channels number */
    initSequence.clear();
    initSequence.append("N511");
    if (iface()->write(initSequence) == true)
    {
        if (checkReply() == false)
            qWarning() << Q_FUNC_INFO << name() << "Channels initialization failed";
    }

    // start the output thread
    start();

    return true;
}

bool Stageprofi::close(quint32 line, bool input)
{
    Q_UNUSED(input)

    stop();
    return DMXUSBWidget::close(line);
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

bool Stageprofi::writeUniverse(quint32 universe, quint32 output, const QByteArray& data, bool dataChanged)
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

void Stageprofi::stop()
{
    if (isRunning() == true)
    {
        m_running = false;
        wait();
    }
}

void Stageprofi::run()
{
    qDebug() << "OUTPUT thread started";

    QElapsedTimer timer;

    m_running = true;

    if (m_outputLines[0].m_compareData.size() == 0)
        m_outputLines[0].m_compareData.fill(0, 512);

    // Wait for device to settle in case the device was opened just recently
    usleep(1000);

    while (m_running == true)
    {
        timer.restart();

        for (int i = 0; i < m_outputLines[0].m_universeData.length(); i++)
        {
            char val = m_outputLines[0].m_universeData[i];

            if (val == m_outputLines[0].m_compareData[i])
                continue;

            QByteArray fastTrans;
            if (i < 256)
            {
                fastTrans.append((char)0xE2);
                fastTrans.append((char)i);
            }
            else
            {
                fastTrans.append((char)0xE3);
                fastTrans.append((char)(i - 256));
            }
            fastTrans.append(val);

            if (iface()->write(fastTrans) == false)
            {
                qWarning() << Q_FUNC_INFO << name() << "will not accept DMX data";
                iface()->purgeBuffers();
                continue;
            }
            else
            {
                m_outputLines[0].m_compareData[i] = val;
                if (checkReply() == false)
                    iface()->purgeBuffers();
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



