/*
  Q Light Controller
  enttecdmxusbpro.cpp

  Copyright (C) Heikki Junnila

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

EnttecDMXUSBPro::EnttecDMXUSBPro(DMXInterface *interface,
                                 quint32 outputLine, quint32 inputLine)
    : QThread(NULL)
    , DMXUSBWidget(interface, outputLine)
    , m_dmxKingMode(false)
    , m_running(false)
{
    m_inputBaseLine = inputLine;

    setInputsNumber(1);
}

EnttecDMXUSBPro::~EnttecDMXUSBPro()
{
    qDebug() << Q_FUNC_INFO;
    stopThread();
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
    for (int i = 0; i < inputs; i++)
        m_midiInputsMap[m_inputBaseLine + inputsNumber() + i] = ushort(inputsNumber() + i);

    for (int o = 0; o < outputs; o++)
        m_mididOutputsMap[m_outputBaseLine + outputsNumber() + o] = ushort(outputsNumber() + o);

    setInputsNumber(inputsNumber() + inputs);
    setOutputsNumber(outputsNumber() + outputs);
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

bool EnttecDMXUSBPro::configureLine(ushort dmxLine, ushort midiLine)
{
    qDebug() << "EnttecDMXUSBPro: Configuring line: " << dmxLine << "midi line:" << midiLine;

    if (dmxLine == 1)
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
        if (midiLine != USHRT_MAX)
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

    // specific port configuration are needed
    // only by ENTTEC
    if (m_dmxKingMode == false)
    {
        if (input == false)
        {
            if (m_mididOutputsMap.contains(line))
                configureLine(m_mididOutputsMap[line], line);
            else
                configureLine(m_outputsMap[line], USHRT_MAX);
        }
        else if(m_midiInputsMap.contains(line))
            configureLine(m_midiInputsMap[line], line);
        else
            qDebug() << Q_FUNC_INFO << "No specific port configuration is needed";
    }

    if (input == true && m_running == false)
    {
        m_universe.fill(0, 512);
        start();
    }

    return true;
}

bool EnttecDMXUSBPro::close(quint32 line, bool input)
{
    if (input == true)
        stopThread();
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

    if (input == true)
    {
        if (m_midiInputsMap.values().contains(line))
            return QString("%1 - %2").arg(devName).arg(QObject::tr("MIDI Input"));
        else
            return QString("%1 - %2").arg(devName).arg(QObject::tr("DMX Input"));
    }
    else
    {
        if (m_mididOutputsMap.values().contains(line))
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
            m_proSerial.sprintf("%x%.2x%.2x%.2x", uchar(reply[7]),
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

/************************************************************************
 * DMX reception
 ************************************************************************/

void EnttecDMXUSBPro::stopThread()
{
    qDebug() << Q_FUNC_INFO;

    if (m_running == true)
    {
        m_running = false;
        wait();
    }
}

void EnttecDMXUSBPro::run()
{
    qDebug() << Q_FUNC_INFO << "begin";

    uchar byte = 0;
    ushort dataLength = 0;

    // count the received MIDI packets.
    // When reaching 3 (cmd + data1 + data2) a complete MIDI packet is ready to be sent
    int midiCounter = 0;
    uchar midiCmd = 0;
    uchar midiData1 = 0;
    uchar midiData2 = 0;

    m_running = true;
    while (m_running == true)
    {
        bool ok = false;
        bool midiMessage = false;
        // Skip bytes until we find the start of the next message
        if ((byte = interface()->readByte(&ok)) != ENTTEC_PRO_START_OF_MSG)
        {
            // If nothing was read, sleep for a while
            if (ok == false)
                msleep(10);
            continue;
        }

        // Check that the message is a "DMX receive packet"
        byte = interface()->readByte();
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
        dataLength = (ushort) interface()->readByte() | ((ushort) interface()->readByte() << 8);
        //qDebug() << "Packet data length:" << dataLength;

        if (midiMessage == false)
        {
            // Check status bytes
            byte = interface()->readByte();
            if (byte & char(0x01))
                qWarning() << Q_FUNC_INFO << "Widget receive queue overflowed";
            else if (byte & char(0x02))
                qWarning() << Q_FUNC_INFO << "Widget receive overrun occurred";

            // Check DMX startcode
            byte = interface()->readByte();
            if (byte != char(0))
                qWarning() << Q_FUNC_INFO << "Non-standard DMX startcode received:" << (uchar) byte;
            dataLength -= 2;
        }

        // Read the whole payload
        QByteArray payload = interface()->read(dataLength);
        //qDebug() << "Data read:" << payload.length();

        for (ushort i = 0; i < payload.length(); i++)
        {
            byte = uchar(payload.at(i));

            if (midiMessage == false)
            {
                if (i < 512 && byte != (uchar) m_universe[i])
                {
                    //qDebug() << "Value at" << i << "changed to" << QString::number(byte);
                    // Store and emit changed values
                    m_universe[i] = byte;
                    emit valueChanged(UINT_MAX, m_inputBaseLine, i, byte);
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
                        quint32 emitLine = m_inputBaseLine + inputsNumber() - m_midiInputsMap.count();
                        emit valueChanged(UINT_MAX, emitLine, channel, value);
                        // for MIDI beat clock signals,
                        // generate a synthetic release event
                        if (midiCmd >= MIDI_BEAT_CLOCK && midiCmd <= MIDI_BEAT_STOP)
                            emit valueChanged(UINT_MAX, emitLine + inputsNumber(), channel, 0);
                    }
                    midiCounter = 0;
                }
            }
        }
    }

    qDebug() << Q_FUNC_INFO << "end";
}


/****************************************************************************
 * Write universe data
 ****************************************************************************/

bool EnttecDMXUSBPro::writeUniverse(quint32 universe, quint32 output, const QByteArray& data)
{
    if (isOpen() == false)
    {
        qDebug() << "[DMXUSB] writeUniverse: device is not open !";
        return false;
    }

    QByteArray request(data);

    if (m_mididOutputsMap.contains(output))
    {
        if (m_outUniverse.size() == 0)
            m_outUniverse.fill(0, 512);

        // send only values that changed
        for (int i = 0; i < data.count(); i++)
        {
            if (data.at(i) == m_outUniverse.at(i))
                continue;

            m_outUniverse[i] = data.at(i);

            request.clear();
            request.prepend(ENTTEC_PRO_START_OF_MSG); // Start byte
            request.append(ENTTEC_PRO_MIDI_OUT_MSG);
            request.append(char(0x03)); // size LSB: 3 bytes
            request.append(char(0x00)); // size MSB

            uchar cmd = 0;
            uchar data1 = 0, data2 = 0;

            if (QLCMIDIProtocol::feedbackToMidi(i + 1, data.at(i),
                                                universe, // MIDI output channel is QLC+ universe
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
                    return false;
                }
            }
        }
    }
    else
    {
        request.prepend(char(ENTTEC_PRO_DMX_ZERO)); // DMX start code (Which constitutes the + 1 below)
        request.prepend(((data.size() + 1) >> 8) & 0xff); // Data length MSB
        request.prepend((data.size() + 1) & 0xff); // Data length LSB

        if (m_outputsMap[output] == 1)
        {
            if (m_dmxKingMode)
                request.prepend(DMXKING_SEND_DMX_PORT2); // Command - second port
            else
                request.prepend(ENTTEC_PRO_SEND_DMX_RQ2); // Command - second port
        }
        else
        {
            if (m_dmxKingMode)
                request.prepend(DMXKING_SEND_DMX_PORT1); // Command - first port
            else
                request.prepend(ENTTEC_PRO_SEND_DMX_RQ); // Command - first port
        }

        request.prepend(ENTTEC_PRO_START_OF_MSG); // Start byte
        request.append(ENTTEC_PRO_END_OF_MSG); // Stop byte

        /* Write "Output Only Send DMX Packet Request" message */
        if (interface()->write(request) == false)
        {
            qWarning() << Q_FUNC_INFO << name() << "will not accept DMX data";
            return false;
        }
    }
    return true;
}


