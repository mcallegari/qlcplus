/*
  Q Light Controller Plus
  dmxusbopenrx.cpp

  Copyright (C) Massimo Callegari
                Emmanuel Coirier

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

#include "dmxusbopenrx.h"
#include "qlcmacros.h"

#define DEFAULT_OPEN_DMX_FREQUENCY    30  // crap
#define RX_BUFFER_SIZE              1024
#define RECEIVE_START_THRESHOLD      300  // number of missed packets before entering the receiving state
#define LARGE_PAYLOAD_SIZE           600
#define MAX_READ_ATTEMPTS             10
#define SETTINGS_CHANNELS "dmxusbopenrx/channels"

/****************************************************************************
 * Initialization
 ****************************************************************************/

DMXUSBOpenRx::DMXUSBOpenRx(DMXInterface *iface,
                                   quint32 inputLine, QObject* parent)
    : QThread(parent)
    , DMXUSBWidget(iface, 0, DEFAULT_OPEN_DMX_FREQUENCY)
    , m_running(false)
    , m_granularity(Unknown)
    , m_reader_state(Calibrating)
{
    qDebug() << "Open RX constructor, line" << inputLine;

    m_inputBaseLine = inputLine;
    setOutputsNumber(0);
    setInputsNumber(1);

    m_inputLines[0].m_universeData = QByteArray();
    m_inputLines[0].m_compareData = QByteArray();

// on macOS, QtSerialPort cannot handle an OpenDMX device
// so, unfortunately, we need to switch back to libftdi
#if defined(Q_OS_MACOS) && defined(QTSERIAL) && (defined(LIBFTDI1) || defined(LIBFTDI))
    if (iface->type() == DMXInterface::QtSerial)
        forceInterfaceDriver(DMXInterface::libFTDI);
#endif
    qDebug() << "Open RX constructor end";
}

DMXUSBOpenRx::~DMXUSBOpenRx()
{
    qDebug() << "Open RX destructor";
    stop();
    qDebug() << "Open RX stopped in destructor";
}

DMXUSBWidget::Type DMXUSBOpenRx::type() const
{
    return DMXUSBWidget::OpenRX;
}

/****************************************************************************
 * Open & Close
 ****************************************************************************/

bool DMXUSBOpenRx::open(quint32 line, bool input)
{
    if (input == false)
    {
        qWarning() << "DMX USB Open RX opened for output, giving up.";
        return false;
    }

    qDebug() << "DMX USB Open RX, opening line" << line;

    if (iface()->type() != DMXInterface::QtSerial)
    {
        if (DMXUSBWidget::open(line, input) == false)
            return close(line);

        if (iface()->clearRts() == false)
            return close(line);
    }
    qDebug() << "Starting Open RX";
    start(QThread::TimeCriticalPriority);
    qDebug() << "Open RX started";
    return true;
}

bool DMXUSBOpenRx::close(quint32 line, bool input)
{
    qDebug() << "Open RX close" << line << "input:" << input;
    stop();
    return DMXUSBWidget::close(line, input);
}

/****************************************************************************
 * Name & Serial
 ****************************************************************************/

QString DMXUSBOpenRx::additionalInfo() const
{
    QString info;
    QString gran;
    QString state;

    info += QString("<P>");
    info += QString("<B>%1:</B> %2").arg(tr("Protocol")).arg("Open DMX USB (Receiving mode (RX))");
    info += QString("<BR>");
    info += QString("<B>%1:</B> %2").arg(QObject::tr("Manufacturer")).arg(vendor());
    info += QString("<BR>");

    if (!m_running)
        state = QString("<FONT COLOR=\"#000000\">%1</FONT>").arg(tr("Stopped"));
    else if (m_reader_state == Idling)
        state = QString("<FONT COLOR=\"#aa0000\">%1</FONT>").arg(tr("Idling"));
    else if (m_reader_state == Calibrating)
        state = QString("<FONT COLOR=\"#aa5500\">%1</FONT>").arg(tr("Calibrating"));
    else
        state = QString("<FONT COLOR=\"#00aa00\">%1</FONT>").arg(tr("Receiving"));

    info += QString("<B>%1:</B> %2").arg(tr("Receiver state")).arg(state);
    info += QString("<BR>");

    if (m_reader_state == Receiving)
    {
        info += QString("<B>%1:</B> %2").arg(tr("Received DMX Channels"))
                                        .arg(m_inputLines[0].m_compareData.length() - 2);

        info += QString("<BR>");
        if (m_frameTimeUs > 0)
            info += QString("<B>%1:</B> %2 Hz").arg(tr("DMX Frame Frequency"))
                                               .arg(1000 / m_frameTimeUs);
    }
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

bool DMXUSBOpenRx::writeUniverse(quint32 universe, quint32 output, const QByteArray& data, bool dataChanged)
{
    Q_UNUSED(universe)
    Q_UNUSED(output)
    Q_UNUSED(data)
    Q_UNUSED(dataChanged)
    // not used at runtime but needed for compilation
    return true;
}

void DMXUSBOpenRx::stop()
{
    if (isRunning() == true)
    {
        qDebug() << "Waiting for receiving thread to stop";
        m_running = false;
        wait();
        qDebug() << "Receiving thread stopped";
    } 
    else 
    {
        qDebug() << "Already stopped";
    }
}

void DMXUSBOpenRx::compareAndEmit(const QByteArray& last_payload, const QByteArray& current_payload)
{
    int max_bound = qMax(last_payload.length(), current_payload.length());

    // bytes 0 and 1 are not in use
    for (int i = 2; i < max_bound; i++)
    {
        if (i < last_payload.length() && i < current_payload.length())
        {
            // value exists in both, just compare
            if (last_payload[i] != current_payload[i])
            {
                emit valueChanged(UINT_MAX, m_inputBaseLine, i - 2, current_payload[i]);
                qDebug() << "Channel" << i - 2 << "changed to" << QString::number((uchar) current_payload[i], 10);
            }
        } 
        else if (i < last_payload.length() && i >= current_payload.length())
        {
            // This frame is shorter. So put a 0 instead.
            emit valueChanged(UINT_MAX, m_inputBaseLine, i - 2, 0);
            qDebug() << "Channel" << i - 2 << "changed to \"0\"";
        } 
        else if (i < current_payload.length() && i >= last_payload.length())
        {
            // Last frame was shorter, just put the current value
            emit valueChanged(UINT_MAX, m_inputBaseLine, i - 2, current_payload[i]);
            qDebug() << "Channel" << i - 2 << "changed to" << QString::number((uchar) current_payload[i], 10);
        }
    }
}

void DMXUSBOpenRx::run()
{
    // Wait for device to settle in case the device was opened just recently
    // Also measure, whether timer granularity is OK
    QElapsedTimer time;
    time.start();
    usleep(1000);
    if (time.elapsed() > 3)
        m_granularity = Bad;
    else
        m_granularity = Good;

    if (iface()->type() == DMXInterface::QtSerial)
    {
        if (DMXUSBWidget::open(0) == false)
        {
            close(0);
            return;
        }

        if (iface()->clearRts() == false)
        {
            close(0);
            return;
        }
    }

    m_running = true;

    QByteArray payload;
    QByteArray& last_payload = m_inputLines[0].m_compareData;
    QByteArray& current_payload = m_inputLines[0].m_universeData;

    quint32 missed_frames = 0;
    quint32 erroneous_frames = 0;
    quint32 erroneous_reads = 0;

    m_frameTimeUs = 0;

    while (m_running == true)
    {
        payload = iface()->read(RX_BUFFER_SIZE);

        if (payload.length() == 0)
        {
            usleep(1000); // nothing to read, don't waste CPU
            missed_frames += 1;
        } 
        else if (payload.length() == 1)
        {
            // a new frame has begun, the chip returns us the first byte
            current_payload.append(payload);
            usleep(500); // wait a little for the other bytes to be read
        } 
        else
        {
            current_payload.append(payload);

            if (payload.length() > LARGE_PAYLOAD_SIZE)
            {
                // we are not reading only a single frame but several of them in one chunk.
                erroneous_reads += 1;
                current_payload.clear();
                qDebug() << iface()->serial() << "Erroneous read" << payload.length() << "bytes";

                if (erroneous_reads > MAX_READ_ATTEMPTS)
                {
                    // set low latency in order to try to fetch frames one by one.
                    iface()->setLowLatency(true);
                    erroneous_reads = 0;
                }
                continue;
            }

            if (current_payload.length() != last_payload.length() && erroneous_frames < 5)
            {
                qDebug() << iface()->serial() << "Bogus frame" << current_payload.length() << "bytes instead of" << last_payload.length();
                current_payload.clear();
                erroneous_frames += 1;
                continue;
            }

            // a frame has been received
            if (missed_frames > RECEIVE_START_THRESHOLD) // only to emit the debug message once, not at each frame
                qDebug() << iface()->serial() << "Receiving";

            m_reader_state = Receiving;
            missed_frames = 0;
            erroneous_frames = 0;
            erroneous_reads = 0;

            m_frameTimeUs = time.elapsed();
            time.restart();

            compareAndEmit(last_payload, current_payload);

            last_payload.clear();
            last_payload.append(current_payload);
            current_payload.clear();
        }

        if (missed_frames == RECEIVE_START_THRESHOLD)
        {
            m_reader_state = Idling;
            qDebug() << iface()->serial() << "Idling";
        } 
        else if (missed_frames == UINT_MAX)
        {
            missed_frames = RECEIVE_START_THRESHOLD;
        }
    }
    qDebug() << iface()->serial() << "Requested to stop";
    iface()->setLowLatency(false);
    m_reader_state = Calibrating;
}
