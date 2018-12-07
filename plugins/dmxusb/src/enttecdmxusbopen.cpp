/*
  Q Light Controller
  enttecdmxusbopen.cpp

  Copyright (C) Heikki Junnila
                Christopher Staite

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

#include <QSettings>
#include <QDebug>
#include <math.h>
#include <QTime>

#include "enttecdmxusbopen.h"
#include "qlcmacros.h"

#define DMX_MAB 16
#define DMX_BREAK 110
#define DMX_CHANNELS 512
#define DEFAULT_OPEN_DMX_FREQUENCY    30  // crap
#define SETTINGS_CHANNELS "enttecdmxusbopen/channels"

/****************************************************************************
 * Initialization
 ****************************************************************************/

EnttecDMXUSBOpen::EnttecDMXUSBOpen(DMXInterface *interface,
                                   quint32 outputLine, QObject* parent)
    : QThread(parent)
    , DMXUSBWidget(interface, outputLine, DEFAULT_OPEN_DMX_FREQUENCY)
    , m_running(false)
    , m_granularity(Unknown)
{
    QSettings settings;
    QVariant var = settings.value(SETTINGS_CHANNELS);
    if (var.isValid() == true)
    {
        int channels = var.toInt();
        if (channels > DMX_CHANNELS || channels <= 0)
            channels = DMX_CHANNELS;
        // channels + 1 Because the first byte is always zero
        // to break a full DMX universe transmission
        m_outputLines[0].m_universeData = QByteArray(channels + 1, 0);
    }
    else
    {
        m_outputLines[0].m_universeData = QByteArray(DMX_CHANNELS + 1, 0);
    }

// on macOS, QtSerialPort cannot handle an OpenDMX device
// so, unfortunately, we need to switch back to libftdi
#if defined(Q_OS_OSX) && defined(QTSERIAL) && (defined(LIBFTDI1) || defined(LIBFTDI))
    if (interface->type() == DMXInterface::QtSerial)
        forceInterfaceDriver(DMXInterface::libFTDI);
#endif
}

EnttecDMXUSBOpen::~EnttecDMXUSBOpen()
{
    stop();
}

DMXUSBWidget::Type EnttecDMXUSBOpen::type() const
{
    return DMXUSBWidget::OpenTX;
}

/****************************************************************************
 * Open & Close
 ****************************************************************************/

bool EnttecDMXUSBOpen::open(quint32 line, bool input)
{
    Q_UNUSED(input)

    if (interface()->type() != DMXInterface::QtSerial)
    {
        if (DMXUSBWidget::open(line) == false)
            return close(line);

        if (interface()->clearRts() == false)
            return close(line);
    }
    start(QThread::TimeCriticalPriority);
    return true;
}

bool EnttecDMXUSBOpen::close(quint32 line, bool input)
{
    Q_UNUSED(input)

    stop();
    return DMXUSBWidget::close(line);
}

/****************************************************************************
 * Name & Serial
 ****************************************************************************/

QString EnttecDMXUSBOpen::additionalInfo() const
{
    QString info;
    QString gran;

    info += QString("<P>");
    info += QString("<B>%1:</B> %2").arg(tr("Protocol")).arg("Open DMX USB");
    info += QString("<BR>");
    info += QString("<B>%1:</B> %2").arg(QObject::tr("Manufacturer"))
                                         .arg(vendor());
    info += QString("<BR>");
    info += QString("<B>%1:</B> %2").arg(tr("DMX Channels"))
                                    .arg(m_outputLines[0].m_universeData.size()-1);
    info += QString("<BR>");
    info += QString("<B>%1:</B> %2Hz").arg(tr("DMX Frame Frequency"))
                                      .arg(m_frequency);
    info += QString("<BR>");
    if (m_granularity == Bad)
        gran = QString("<FONT COLOR=\"#aa0000\">%1</FONT>").arg(tr("Bad"));
    else if (m_granularity == Good)
        gran = QString("<FONT COLOR=\"#00aa00\">%1</FONT>").arg(tr("Good"));
    else
        gran = tr("Patch this widget to a universe to find out.");
    info += QString("<B>%1:</B> %2").arg(tr("System Timer Accuracy")).arg(gran);
    info += QString("</P>");

    return info;
}

/****************************************************************************
 * Thread
 ****************************************************************************/

bool EnttecDMXUSBOpen::writeUniverse(quint32 universe, quint32 output, const QByteArray& data)
{
    Q_UNUSED(universe)
    Q_UNUSED(output)

    m_outputLines[0].m_universeData.replace(1, MIN(data.size(), m_outputLines[0].m_universeData.size() - 1),
                                            data.constData(), MIN(data.size(), m_outputLines[0].m_universeData.size() - 1));
    return true;
}

void EnttecDMXUSBOpen::stop()
{
    if (isRunning() == true)
    {
        m_running = false;
        wait();
    }
}

void EnttecDMXUSBOpen::run()
{
    QElapsedTimer timer;

    // Wait for device to settle in case the device was opened just recently
    usleep(1000);

    if (interface()->type() == DMXInterface::QtSerial)
    {
        if (DMXUSBWidget::open(0) == false)
        {
            close(0);
            return;
        }

        if (interface()->clearRts() == false)
        {
            close(0);
            return;
        }
    }

    m_running = true;
    // let's assume this is a good adapter...
    m_granularity = Good;

    while (m_running == true)
    {
        timer.restart();

        if (interface()->setBreak(true) == false)
            goto framesleep;

        if (m_granularity == Good)
            usleep(DMX_BREAK);

        if (interface()->setBreak(false) == false)
            goto framesleep;

        if (m_granularity == Good)
            usleep(DMX_MAB);

        if (interface()->write(m_outputLines[0].m_universeData) == false)
            goto framesleep;

framesleep:
        // Sleep for the remainder of the DMX frame time
        // and set granularity accordingly
        long timetoSleep = m_frameTimeUs - (timer.nsecsElapsed() / 1000);
        if (timetoSleep < 0)
        {
            qWarning() << "DMX output is running late !";
            m_granularity = Bad;
        }
        else
        {
            usleep(timetoSleep);
            m_granularity = Good;
        }
    }
}
