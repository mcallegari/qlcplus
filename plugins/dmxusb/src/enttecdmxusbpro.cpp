/*
  Q Light Controller Plus
  enttecdmxusbpro.cpp

  Copyright (C) Heikki Junnila, Massimo Callegari

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
#include "midiprotocol.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

EnttecDMXUSBPro::EnttecDMXUSBPro(DMXInterface *interface, quint32 outputLine, quint32 inputLine)
    : QThread(NULL)
    , DMXUSBWidget(interface, outputLine, DEFAULT_OUTPUT_FREQUENCY)
    , m_dmxKingMode(false)
    , m_inputThread(NULL)
    , m_outputRunning(false)
{
    m_inputBaseLine = inputLine;

    setInputsNumber(1);
}

EnttecDMXUSBPro::~EnttecDMXUSBPro()
{
    qDebug() << Q_FUNC_INFO;
    close(m_inputBaseLine, true);
    close(m_outputBaseLine, false);
}

DMXUSBWidget::Type EnttecDMXUSBPro::type() const
{
    if (name().toUpper().contains("PRO MK2") == true)
        return ProMk2;
    else if(m_dmxKingMode)
        return UltraPro;
    else
        return ProRXTX;
}

void EnttecDMXUSBPro::setMidiPortsNumber(int inputs, int outputs)
{
    // place MIDI I/O after the DMX I/O
    // and create a local midi map
    if (inputs)
    {
        m_inputLines.resize(m_inputLines.count() + inputs);
        for (int i = m_inputLines.count() - inputs; i < m_inputLines.count(); i++)
        {
            m_inputLines[i].m_isOpen = false;
            m_inputLines[i].m_lineType = MIDI;
        }
    }

    if (outputs)
    {
        m_outputLines.resize(m_outputLines.count() + inputs);
        for (int o = m_outputLines.count() - outputs; o < m_outputLines.count(); o++)
        {
            m_outputLines[o].m_isOpen = false;
            m_outputLines[o].m_lineType = MIDI;
        }
    }
}

void EnttecDMXUSBPro::setDMXKingMode()
{
    m_dmxKingMode = true;
}

QString EnttecDMXUSBPro::additionalInfo() const
{
    QString info;

    info += QString("<P>");

    if (m_dmxKingMode == true)
        info += QString("<B>%1:</B> %2").arg(tr("Protocol")).arg("ultraDMX USB Pro");
    else
        info += QString("<B>%1:</B> %2").arg(tr("Protocol")).arg("ENTTEC");

    info += QString("<BR>");
    info += QString("<B>%1:</B> %2").arg(tr("Manufacturer")).arg(vendor());
    info += QString("<BR>");
    info += QString("<B>%1:</B> %2").arg(tr("Serial number")).arg(m_proSerial.isEmpty() ? serial() : m_proSerial);
    info += QString("</P>");

    return info;
}

/****************************************************************************
 * Open & Close
 ****************************************************************************/

bool EnttecDMXUSBPro::configureLine(ushort dmxLine, bool isMidi)
{
    qDebug() << "EnttecDMXUSBPro: Configuring line: " << dmxLine << "MIDI line:" << isMidi;

    if (dmxLine >= 1)
    {
        QByteArray request;
        request.append(ENTTEC_PRO_START_OF_MSG); // DMX start code (Which constitutes the + 1 below)
        request.append(ENTTEC_PRO_ENABLE_API2); // Enable API2
        request.append(char(0x04)); // data length LSB
        request.append(ENTTEC_PRO_DMX_ZERO); // data length MSB
        request.append(char(0xAD)); // Magic number. WTF ??
        request.append(char(0x88)); // Magic number. WFT ??
        request.append(char(0xD0)); // Magic number. WTF ??
        request.append(char(0xC8)); // Magic number. WTF ??
        request.append(ENTTEC_PRO_END_OF_MSG); // Stop byte

        /* Write "Set API Key Request" message */
        if (interface()->write(request) == false)
        {
            qWarning() << Q_FUNC_INFO << name() << "FTDI write filed (DMX2 port config)";
            return false;
        }

        request.clear();
        request.append(ENTTEC_PRO_START_OF_MSG);
        request.append(ENTTEC_PRO_PORT_ASS_REQ);
        request.append(char(0x02)); // data length LSB - 2 bytes
        request.append(ENTTEC_PRO_DMX_ZERO); // data length MSB
        request.append(char(0x01)); // Port 1 enabled for DMX and RDM
        if (isMidi)
            request.append(char(0x02)); // Port 2 enabled for MIDI IN and MIDI OUT
        else
            request.append(char(0x01)); // Port 2 enabled for DMX and RDM
        request.append(ENTTEC_PRO_END_OF_MSG); // Stop byte

        /* Write "Set Port Assignment Request" message */
        if (interface()->write(request) == false)
        {
            qWarning() << Q_FUNC_INFO << name() << "FTDI write filed (DMX1 port config)";
            return false;
        }
    }

    return true;
}

bool EnttecDMXUSBPro::open(quint32 line, bool input)
{
    if (DMXUSBWidget::open(line, input) == false)
        return close(line, input);

    if (interface()->clearRts() == false)
        return close(line, input);

    if (m_proSerial.isEmpty())
        extractSerial();

    // specific port configuration are needed only by ENTTEC
    if (m_dmxKingMode == false)
    {
        if (input == false)
        {
            quint32 devLine = line - m_outputBaseLine;
            if (m_outputLines[devLine].m_lineType == MIDI)
                configureLine(devLine, true);
            else
                configureLine(devLine, false);
        }
        else
        {
            quint32 devLine = line - m_inputBaseLine;
            if (m_inputLines[devLine].m_lineType == MIDI)
                configureLine(devLine, true);
        }
    }

    if (input == false && m_outputRunning == false)
    {
        // start the output thread
        start();
    }
    else if (input == true && m_inputThread == NULL)
    {
        // create (therefore start) the input thread
        m_inputThread = new EnttecDMXUSBProInput(interface());
        connect(m_inputThread, SIGNAL(dataReady(QByteArray,bool)), this, SLOT(slotDataReceived(QByteArray,bool)));
    }

    return true;
}

bool EnttecDMXUSBPro::close(quint32 line, bool input)
{
    if (input)
    {
        if (m_inputThread)
        {
            disconnect(m_inputThread, SIGNAL(dataReady(QByteArray,bool)), this, SLOT(slotDataReceived(QByteArray,bool)));
            delete m_inputThread;
            m_inputThread = NULL;
        }
    }
    else
    {
        stopOutputThread();
    }

    return DMXUSBWidget::close(line, input);
}

/****************************************************************************
 * Name & Serial
 ****************************************************************************/

QString EnttecDMXUSBPro::uniqueName(ushort line, bool input) const
{
    QString devName;
    if (realName().isEmpty() == false)
        devName = realName();
    else
        devName = name();

    if (input)
    {
        if (m_inputLines[line].m_lineType == MIDI)
            return QString("%1 - %2").arg(devName).arg(QObject::tr("MIDI Input"));
        else
            return QString("%1 - %2").arg(devName).arg(QObject::tr("DMX Input"));
    }
    else
    {
        if (m_outputLines[line].m_lineType == MIDI)
            return QString("%1 - %2").arg(devName).arg(QObject::tr("MIDI Output"));
        else
            return QString("%1 - %2 %3").arg(devName).arg(QObject::tr("DMX Output")).arg(line + 1);
    }
/*
    if (m_proSerial.isEmpty() == true)
        return QString("%1 (S/N: %2)").arg(name()).arg(serial());
    else
        return QString("%1 (S/N: %2)").arg(name()).arg(m_proSerial);
*/
}

bool EnttecDMXUSBPro::extractSerial()
{
    QByteArray request;
    request.append(ENTTEC_PRO_START_OF_MSG);
    request.append(ENTTEC_PRO_READ_SERIAL);
    request.append(ENTTEC_PRO_DMX_ZERO); // data length LSB
    request.append(ENTTEC_PRO_DMX_ZERO); // data length MSB
    request.append(ENTTEC_PRO_END_OF_MSG);

    if (interface()->write(request) == true)
    {
        QByteArray reply = interface()->read(9);

        /* Reply message is:
           { 0x7E 0x0A 0x04 0x00 0xNN, 0xNN, 0xNN, 0xNN 0xE7 }
           Where 0xNN represent widget's unique serial number in BCD */
        if (uchar(reply[0]) == 0x7e && uchar(reply[1]) == 0x0a &&
            uchar(reply[2]) == 0x04 && uchar(reply[3]) == 0x00 &&
            uchar(reply[8]) == 0xe7)
        {
            m_proSerial.asprintf("%x%.2x%.2x%.2x", uchar(reply[7]),
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

void EnttecDMXUSBPro::slotDataReceived(QByteArray data, bool isMidi)
{
    // count the received MIDI packets.
    // When reaching 3 (cmd + data1 + data2) a complete MIDI packet is ready to be sent
    int midiCounter = 0;
    uchar midiCmd = 0;
    uchar midiData1 = 0;
    uchar midiData2 = 0;

    int devLine = isMidi ? m_inputLines.count() - 1 : 0;
    int emitLine = m_inputBaseLine + devLine;

    for (int i = 0; i < data.length(); i++)
    {
        uchar byte = uchar(data.at(i));

        if (isMidi == false)
        {
            if (m_inputLines[devLine].m_universeData.size() == 0)
                m_inputLines[devLine].m_universeData.fill(0, 512);

            if (i < 512 && byte != (uchar) m_inputLines[devLine].m_universeData[i])
            {
                qDebug() << "Value at" << i << "changed to" << QString::number(byte);
                // Store and emit changed values
                m_inputLines[devLine].m_universeData[i] = byte;
                emit valueChanged(UINT_MAX, emitLine, i, byte);
            }
        }
        else // MIDI message parsing
        {
            //qDebug() << "MIDI byte:" << byte;
            if (midiCounter == 0)
            {
                if(MIDI_IS_CMD(byte))
                {
                    midiCmd = byte;
                    midiCounter++;
                }
            }
            else if (midiCounter == 1)
            {
                midiData1 = byte;
                midiCounter++;
            }
            else if (midiCounter == 2)
            {
                midiData2 = byte;
                uint channel = 0;
                uchar value = 0;
                if (QLCMIDIProtocol::midiToInput(midiCmd, midiData1, midiData2,
                                                 MAX_MIDI_CHANNELS, // always listen in OMNI mode
                                                 &channel, &value) == true)
                {
                    emit valueChanged(UINT_MAX, emitLine, channel, value);
                    // for MIDI beat clock signals,
                    // generate a synthetic release event
                    if (midiCmd >= MIDI_BEAT_CLOCK && midiCmd <= MIDI_BEAT_STOP)
                        emit valueChanged(UINT_MAX, emitLine, channel, 0);
                }
                midiCounter = 0;
            }
        }
    }
}

/************************************************************************
 * Output
 ************************************************************************/

void EnttecDMXUSBPro::stopOutputThread()
{
    qDebug() << Q_FUNC_INFO;

    if (m_outputRunning == true)
    {
        m_outputRunning = false;
        wait();
    }
}

bool EnttecDMXUSBPro::writeUniverse(quint32 universe, quint32 output, const QByteArray& data)
{
    Q_UNUSED(universe)

    if (isOpen() == false)
    {
        qDebug() << "[DMXUSB] writeUniverse: device is not open!";
        return false;
    }

    quint32 devLine = output - m_outputBaseLine;
    if (devLine >= (quint32)outputsNumber())
        return false;

    if (m_outputLines[devLine].m_universeData.size() == 0)
        m_outputLines[devLine].m_universeData.append(data);
    else
        m_outputLines[devLine].m_universeData.replace(0, data.size(), data);

    return true;
}

void EnttecDMXUSBPro::run()
{
    qDebug() << "OUTPUT thread started";
    QElapsedTimer timer;

    m_outputRunning = true;
    while (m_outputRunning == true)
    {
        timer.restart();

        // no open output lines: do nothing
        if (openOutputLines() == 0)
            goto framesleep;

        for (int i = 0; i < m_outputLines.count(); i++)
        {
            int dataLen = m_outputLines[i].m_universeData.length();
            if (dataLen == 0)
                continue;

            if (m_outputLines[i].m_lineType == MIDI)
            {
                QByteArray request;

                if (m_outputLines[i].m_compareData.size() == 0)
                    m_outputLines[i].m_compareData.fill(0, 512);

                // send only values that changed
                for (int j = 0; j < m_outputLines[i].m_universeData.length(); j++)
                {
                    uchar val = uchar(m_outputLines[i].m_universeData[j]);

                    if (val == m_outputLines[i].m_compareData[j])
                        continue;

                    m_outputLines[i].m_compareData[j] = val;

                    request.clear();
                    request.prepend(ENTTEC_PRO_START_OF_MSG); // Start byte
                    request.append(ENTTEC_PRO_MIDI_OUT_MSG);
                    request.append(char(0x03)); // size LSB: 3 bytes
                    request.append(char(0x00)); // size MSB

                    uchar cmd = 0;
                    uchar data1 = 0, data2 = 0;

                    if (QLCMIDIProtocol::feedbackToMidi(i + 1, val,
                                                        MAX_MIDI_CHANNELS, // MIDI output channel is always OMNI
                                                        true, // send Note OFF
                                                        &cmd, &data1, &data2) == true)
                    {
                        qDebug() << Q_FUNC_INFO << "cmd:" << cmd << "data1:" << data1 << "data2:" << data2;
                        request.append(cmd);
                        request.append(data1);
                        request.append(data2);
                        request.append(ENTTEC_PRO_END_OF_MSG); // Stop byte
                        if (interface()->write(request) == false)
                        {
                            qWarning() << Q_FUNC_INFO << name() << "will not accept MIDI data";
                            continue;
                        }
                    }
                }
            }
            else
            {
                QByteArray request;
                request.append(ENTTEC_PRO_START_OF_MSG); // Start byte

                if (i == 1)
                {
                    if (m_dmxKingMode)
                        request.append(DMXKING_SEND_DMX_PORT2); // Command - second port
                    else
                        request.append(ENTTEC_PRO_SEND_DMX_RQ2); // Command - second port
                }
                else
                {
                    if (m_dmxKingMode)
                        request.append(DMXKING_SEND_DMX_PORT1); // Command - first port
                    else
                        request.append(ENTTEC_PRO_SEND_DMX_RQ); // Command - first port
                }

                request.append((dataLen + 1) & 0xff); // Data length LSB
                request.append(((dataLen + 1) >> 8) & 0xff); // Data length MSB
                request.append(char(ENTTEC_PRO_DMX_ZERO)); // DMX start code (Which constitutes the + 1 below)
                request.append(m_outputLines[i].m_universeData);
                request.append(ENTTEC_PRO_END_OF_MSG); // Stop byte

                //qDebug() << "OUTPUT" << request.length() << "bytes on line" << i;
                //qDebug() << "DATA" << QString::number(request.at(5)) << QString::number(request.at(6)) << QString::number(request.at(7));

                /* Write "Output Only Send DMX Packet Request" message */
                if (interface()->write(request) == false)
                {
                    qWarning() << Q_FUNC_INFO << name() << "will not accept DMX data";
                    continue;
                }
            }
        }
framesleep:
        int timetoSleep = m_frameTimeUs - (timer.nsecsElapsed() / 1000);
        if (timetoSleep < 0)
            qWarning() << "DMX output is running late !";
        else
            usleep(timetoSleep);
    }

    qDebug() << "OUTPUT thread terminated";
}

/************************************************************************
 * Input thread implementation
 ************************************************************************/

EnttecDMXUSBProInput::EnttecDMXUSBProInput(DMXInterface *interface)
    : m_interface(interface)
    , m_running(false)
{
    Q_ASSERT(interface != NULL);

    // start the event loop immediately
    start();
}

EnttecDMXUSBProInput::~EnttecDMXUSBProInput()
{
    qDebug() << Q_FUNC_INFO;
    stopInputThread();
}

void EnttecDMXUSBProInput::stopInputThread()
{
    qDebug() << Q_FUNC_INFO;

    if (m_running == true)
    {
        m_running = false;
        wait();
    }
}

void EnttecDMXUSBProInput::run()
{
    qDebug() << "INPUT thread started";

    uchar byte = 0;
    ushort dataLength = 0;

    m_running = true;
    while (m_running == true)
    {
        bool ok = false;
        bool midiMessage = false;
        // Skip bytes until we find the start of the next message
        if ((byte = m_interface->readByte(&ok)) != ENTTEC_PRO_START_OF_MSG)
        {
            // If nothing was read, sleep for a while
            if (ok == false)
                msleep(10);
            continue;
        }

        // Check that the message is a "DMX receive packet"
        byte = m_interface->readByte();
        if (byte == ENTTEC_PRO_MIDI_IN_MSG)
        {
            midiMessage = true;
        }
        else if (byte != ENTTEC_PRO_RECV_DMX_PKT)
        {
            qWarning() << Q_FUNC_INFO << "Got unrecognized label:" << (uchar) byte;
            continue;
        }

        // Get payload length
        dataLength = (ushort) m_interface->readByte() | ((ushort) m_interface->readByte() << 8);
        //qDebug() << "Packet data length:" << dataLength;

        if (midiMessage == false)
        {
            // Check status bytes
            byte = m_interface->readByte();
            if (byte & char(0x01))
                qWarning() << Q_FUNC_INFO << "Widget receive queue overflowed";
            else if (byte & char(0x02))
                qWarning() << Q_FUNC_INFO << "Widget receive overrun occurred";

            // Check DMX startcode
            byte = m_interface->readByte();
            if (byte != char(0))
                qWarning() << Q_FUNC_INFO << "Non-standard DMX startcode received:" << (uchar) byte;
            dataLength -= 2;
        }

        // Read the whole payload
        QByteArray payload = m_interface->read(dataLength);
        emit dataReady(payload, midiMessage);
    }

    qDebug() << "INPUT thread terminated";
}
