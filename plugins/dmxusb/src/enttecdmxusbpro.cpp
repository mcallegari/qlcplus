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
#include "rdmprotocol.h"

#define ENTTEC_PRO_DMX_ZERO      char(0x00)
#define ENTTEC_PRO_RECV_DMX_PKT  char(0x05)
#define ENTTEC_PRO_SEND_DMX_RQ   char(0x06)
#define ENTTEC_PRO_READ_SERIAL   char(0x0A)
#define ENTTEC_PRO_ENABLE_API2   char(0x0D)
#define ENTTEC_PRO_SEND_DMX_RQ2  char(0xA9)
#define ENTTEC_PRO_PORT_ASS_REQ  char(0xCB)
#define ENTTEC_PRO_START_OF_MSG  char(0x7E)
#define ENTTEC_PRO_END_OF_MSG    char(0xE7)
#define ENTTEC_PRO_MIDI_OUT_MSG  char(0xBE)
#define ENTTEC_PRO_MIDI_IN_MSG   char(0xE8)

// RDM defines
#define ENTTEC_PRO_RDM_SEND             char(0x07)
#define ENTTEC_PRO_RDM_DISCOVERY_REQ    char(0x0B)
#define ENTTEC_PRO_RDM_RECV_TIMEOUT     char(0x0C)
#define ENTTEC_PRO_RDM_RECV_TIMEOUT2    char(0x8E)
#define ENTTEC_PRO_RDM_SEND2            char(0x9D)
#define ENTTEC_PRO_RDM_DISCOVERY_REQ2   char(0xB6)

// DMXking defines
#define DMXKING_ESTA_ID          0x6A6B
#define ULTRADMX_DMX512A_DEV_ID  0x00
#define ULTRADMX_MICRO_DEV_ID    0x03

#define DMXKING_USB_DEVICE_MANUFACTURER 0x4D
#define DMXKING_USB_DEVICE_NAME         0x4E
#define DMXKING_DMX_PORT_COUNT          0x63
#define DMXKING_DMX_PORT_DIRECTION      0x71
#define DMXKING_SEND_DMX_PORT1          char(0x64)
#define DMXKING_SEND_DMX_PORT2          char(0x65)

#define RDM_MAX_RETRY   5

/****************************************************************************
 * Initialization
 ****************************************************************************/

EnttecDMXUSBPro::EnttecDMXUSBPro(DMXInterface *iface, quint32 outputLine, quint32 inputLine)
    : QThread(NULL)
    , DMXUSBWidget(iface, outputLine, DEFAULT_OUTPUT_FREQUENCY)
    , m_dmxKingMode(false)
    , m_isThreadRunning(false)
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    , m_outputMutex()
#else
    , m_outputMutex(QMutex::Recursive)
#endif
    , m_rdm(NULL)
    , m_universe(UINT_MAX)
{
    m_inputBaseLine = inputLine;

    setInputsNumber(1);

    // by default, set the serial number
    // provided by FTDI
    m_proSerial = serial();

    // then attempt to extract
    // the vendor serial number
    extractSerial();
}

EnttecDMXUSBPro::~EnttecDMXUSBPro()
{
    qDebug() << Q_FUNC_INFO;
    close(m_inputBaseLine, true);
    close(m_outputBaseLine, false);

    if (m_rdm)
        free(m_rdm);
}

bool EnttecDMXUSBPro::writeLabelRequest(DMXInterface *iface, int label)
{
    QByteArray request;
    request.append(ENTTEC_PRO_START_OF_MSG);
    request.append(label);
    request.append(char(0)); // data length LSB
    request.append(char(0)); // data length MSB
    request.append(ENTTEC_PRO_END_OF_MSG);

    return iface->write(request);
}

bool EnttecDMXUSBPro::readResponse(DMXInterface *iface, char label, QByteArray &payload)
{
    if (iface == NULL)
        return false;

    int attemptCount = 0;
    QByteArray data;

    while (attemptCount < 10)
    {
        data.append(iface->read(1024));

        if (data.size() == 0)
        {
            qDebug() << "No data read";
            return false;
        }

        if (data[0] != ENTTEC_PRO_START_OF_MSG)
        {
            qDebug() << Q_FUNC_INFO << "Reply message wrong start code: " << QString::number(data[0], 16);
            return false;
        }

        // start | label | data length
        if (data.size() < 4)
            return false;

        char labelRead = data[1];
        int dataLen = (data[3] << 8) | data[2];

        // data is not enough. Request more
        if (data.length() < dataLen + 5)
        {
            qDebug() << "Not enough data. Read more";
            attemptCount++;
            continue;
        }

        // check if we got the expected label
        // otherwise, read more data until
        // reaching maximum attempts
        if (labelRead != label)
        {
            qDebug() << "Mismatching label. Expected" << uchar(label) << "got" << uchar(labelRead);
            data.remove(0, dataLen + 5);
            attemptCount++;
            continue;
        }

        payload = data.mid(4, dataLen);
        break;
    }

    return true;
}

void EnttecDMXUSBPro::parsePortFlags(const QByteArray &inArray, QByteArray &outArray)
{
    if (inArray.size() < 4)
    {
        qWarning() << "Malformed port configuration detected" << inArray;
        return;
    }

    quint8 direction        = static_cast<quint8>(inArray[0]);
    quint8 usbDMX           = static_cast<quint8>(inArray[1]);
    quint8 artnetSACN       = static_cast<quint8>(inArray[2]);
    quint8 artnetSACNSelect = static_cast<quint8>(inArray[3]);

    for (int i = 0; i < outArray.size(); ++i)
    {
        quint8 flags = 0;

        flags |= (direction & (1 << i))        ? Input               : Output;
        if (usbDMX & (1 << i))                 flags |= USB_DMX_Forward;
        if (artnetSACN & (1 << i))             flags |= ArtNet_sACN_Forward;
        if (artnetSACNSelect & (1 << i))       flags |= ArtNet_sACN_Select;

        outArray[i] = static_cast<char>(flags);
    }
}

bool EnttecDMXUSBPro::detectDMXKingDevice(DMXInterface *iface,
                                          QString &manufName, QString &deviceName,
                                          int &ESTA_ID, int &DEV_ID,
                                          QByteArray &portDirection)
{
    bool ret = false;

    if (iface->open() == false)
        return false;

    iface->setLineProperties();
    iface->setFlowControl();
    iface->setBaudRate();
    iface->purgeBuffers();

    // retrieve ESTA ID and Manufacturer name
    if (writeLabelRequest(iface, DMXKING_USB_DEVICE_MANUFACTURER) == false)
    {
        qDebug() << Q_FUNC_INFO << "Cannot write data to device";
        return false;
    }

    QByteArray payload;

    if (readResponse(iface, DMXKING_USB_DEVICE_MANUFACTURER, payload))
    {
        if (payload.length() > 2)
        {
            ESTA_ID = (payload[1] << 8) | payload[0];
            manufName = QString(payload.mid(2, payload.length() - 2));
            qDebug() << "--------> Device Manufacturer: " << manufName;
        }
    }

    // retrieve Device ID and Device name
    if (writeLabelRequest(iface, DMXKING_USB_DEVICE_NAME) == false)
    {
        qDebug() << Q_FUNC_INFO << "Cannot write data to device";
        return false;
    }

    if (readResponse(iface, DMXKING_USB_DEVICE_NAME, payload))
    {
        if (payload.length() > 2)
        {
            DEV_ID = (payload[1] << 8) | payload[0];
            deviceName = QString(payload.mid(2, payload.length() - 2));
            qDebug() << "--------> Device Name: " << deviceName;
        }
    }

    qDebug() << "--------> ESTA Code: " << QString::number(ESTA_ID, 16) << ", Device ID: " << QString::number(DEV_ID, 16);

    // if this is a DMXKing device, gather ports configuration
    if (ESTA_ID == DMXKING_ESTA_ID)
    {
        // retrieve the number of ports
        if (writeLabelRequest(iface, DMXKING_DMX_PORT_COUNT) == false)
        {
            qDebug() << Q_FUNC_INFO << "Cannot write data to device";
            return false;
        }

        uchar portsNumber = 0;

        if (readResponse(iface, DMXKING_DMX_PORT_COUNT, payload))
        {
            if (payload.length())
            {
                portsNumber = uchar(payload[0]);
                qDebug() << "Number of ports detected:" << portsNumber;

                portDirection.fill(0x00, portsNumber);

                // retrieve the ports configuration
                if (writeLabelRequest(iface, DMXKING_DMX_PORT_DIRECTION) == false)
                {
                    qDebug() << Q_FUNC_INFO << "Cannot write data to device";
                    return false;
                }

                if (readResponse(iface, DMXKING_DMX_PORT_DIRECTION, payload))
                {
                    parsePortFlags(payload, portDirection);
                    qDebug() << "Port direction" << portDirection;
                }
            }
        }

        ret = true;
    }

    iface->close();

    return ret;
}

DMXUSBWidget::Type EnttecDMXUSBPro::type() const
{
    if (name().toUpper().contains("PRO MK2") == true)
        return ProMk2;
    else if (m_dmxKingMode)
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
        m_outputLines.resize(m_outputLines.count() + outputs);
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
    info += QString("<B>%1:</B> %2").arg(tr("Serial number")).arg(m_proSerial);
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
        if (iface()->write(request) == false)
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
        if (iface()->write(request) == false)
        {
            qWarning() << Q_FUNC_INFO << name() << "FTDI write filed (DMX1 port config)";
            return false;
        }
    }

    return true;
}

bool EnttecDMXUSBPro::open(quint32 line, bool input)
{
    InterfaceAction cmd;
    cmd.action = OpenLine;
    cmd.param1 = line;
    cmd.param2 = input;
    m_actionsQueue.append(cmd);

    // if needed, start the input/output thread
    if (m_isThreadRunning == false)
        start();

    return true;
}

bool EnttecDMXUSBPro::close(quint32 line, bool input)
{
    InterfaceAction cmd;
    cmd.action = CloseLine;
    cmd.param1 = line;
    cmd.param2 = input;
    m_actionsQueue.append(cmd);

    // if this is the last line to close,
    // stop the input/output thread
    if ((input == false && openOutputLines() == 1) ||
        (input == true && openInputLines() == 1))
        stopThread();

    return DMXUSBWidget::close(line, input);
}

/************************************************************************
 * Input
 ************************************************************************/

int EnttecDMXUSBPro::readData(QByteArray &payload, bool &isMIDI, bool needRDM)
{
    uchar byte = 0;
    DMXInterface *interface = iface();

    // Skip bytes until we find the start of the next message
    if ((interface->readByte()) != ENTTEC_PRO_START_OF_MSG)
        return 0;

    // Check the message type
    byte = interface->readByte();
    if (byte == uchar(ENTTEC_PRO_MIDI_IN_MSG))
    {
        isMIDI = true;
    }
    else if (byte == uchar(ENTTEC_PRO_RDM_RECV_TIMEOUT) || byte == uchar(ENTTEC_PRO_RDM_RECV_TIMEOUT2))
    {
        qDebug() << "Got RDM timeout";
        // read end byte
        interface->readByte();
        return 0;
    }
    else if (byte != ENTTEC_PRO_RECV_DMX_PKT && byte != ENTTEC_PRO_READ_SERIAL)
    {
        qWarning() << Q_FUNC_INFO << "Got unrecognized label:" << (uchar) byte;
        return 0;
    }

    // Get payload length
    ushort dataLength = (ushort) interface->readByte() | ((ushort) interface->readByte() << 8);

    if (isMIDI == false)
    {
        // Check status bytes
        byte = interface->readByte();
        if (byte & char(0x01))
            qWarning() << Q_FUNC_INFO << "Widget receive queue overflowed";
        else if (byte & char(0x02))
            qWarning() << Q_FUNC_INFO << "Widget receive overrun occurred";

        if (needRDM == false)
        {
            // Check DMX startcode
            byte = interface->readByte();
            if (byte != char(0))
                qWarning() << Q_FUNC_INFO << "Non-standard DMX startcode received:" << (uchar) byte;
            dataLength -= 2;
        }
    }

    // Read the whole payload
    payload.clear();
    payload = interface->read(dataLength);

    // read end byte
    interface->readByte();

    //qDebug() << "Got payload:" << payload.toHex(',');

#ifdef DEBUG_RDM
    if (needRDM)
        qDebug() << "Got payload:" << payload.toHex(',');
#endif

    return dataLength;
}

/****************************************************************************
 * Name & Serial
 ****************************************************************************/

QString EnttecDMXUSBPro::uniqueName(ushort line, bool input) const
{
    QString devName;

    devName = realName();

    if (devName.isEmpty())
        devName = name();

    if (input)
    {
        if (m_inputLines[line].m_lineType == MIDI)
            return QString("%1 - %2 - (S/N: %3)").arg(devName, QObject::tr("MIDI Input"), m_proSerial);
        else
            return QString("%1 - %2 - (S/N: %3)").arg(devName, QObject::tr("DMX Input"), m_proSerial);
    }
    else
    {
        if (m_outputLines[line].m_lineType == MIDI)
            return QString("%1 - %2 - (S/N: %3)").arg(devName, QObject::tr("MIDI Output"), m_proSerial);
        else
            return QString("%1 - %2 %3 - (S/N: %4)").arg(devName, QObject::tr("DMX Output"), QString::number(line + 1), m_proSerial);
    }
}

bool EnttecDMXUSBPro::extractSerial()
{
    bool result = false;
    QByteArray request;
    request.append(ENTTEC_PRO_START_OF_MSG);
    request.append(ENTTEC_PRO_READ_SERIAL);
    request.append(ENTTEC_PRO_DMX_ZERO); // data length LSB
    request.append(ENTTEC_PRO_DMX_ZERO); // data length MSB
    request.append(ENTTEC_PRO_END_OF_MSG);

    iface()->open();
    iface()->clearRts();

    if (iface()->write(request) == true)
    {
        msleep(50);
        QByteArray reply;
        bool notUsed;
        int bytesRead = readData(reply, notUsed, false);

        if (bytesRead != 4)
        {
            qWarning() << Q_FUNC_INFO << name() << "gave malformed serial reply - length:" << bytesRead;
            return result;
        }

        /* Reply message is:
           { 0x7E 0x0A 0x04 0x00 0xNN, 0xNN, 0xNN, 0xNN 0xE7 }
           Where 0xNN represent widget's unique serial number in BCD */
        if (bytesRead == 4)
        {
            m_proSerial = m_proSerial.asprintf("%x%.2x%.2x%.2x", uchar(reply[3]),
                                                                 uchar(reply[2]),
                                                                 uchar(reply[1]),
                                                                 uchar(reply[0]));
            qDebug() << Q_FUNC_INFO << "Serial number OK: " << m_proSerial;
            result = true;
        }
        else
        {
            qWarning() << Q_FUNC_INFO << name() << "gave malformed serial reply:" << reply.toHex(',');
        }
    }
    else
    {
        qWarning() << Q_FUNC_INFO << name() << "will not accept serial request";
    }

    iface()->close();
    return result;
}

/************************************************************************
 * Output
 ************************************************************************/

bool EnttecDMXUSBPro::writeUniverse(quint32 universe, quint32 output, const QByteArray& data, bool dataChanged)
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
    {
        m_outputLines[devLine].m_universeData.append(data);
        m_outputLines[devLine].m_universeData.append(DMX_CHANNELS - data.size(), 0);
    }

    if (dataChanged)
        m_outputLines[devLine].m_universeData.replace(0, data.size(), data);

    return true;
}

/************************************************************************
 * Input/Output Thread
 ************************************************************************/

void EnttecDMXUSBPro::run()
{
    qDebug() << "ENTTEC PRO: INPUT/OUTPUT thread started";
    QElapsedTimer timer;

    /** Flag that indicates if the IO thread
     *  should read input data as well */
    bool readInput = false;

    m_isThreadRunning = true;

    while (m_isThreadRunning == true)
    {
        timer.restart();

        /* **************************************************************
         *                       CHECK PENDING ACTIONS
         * ************************************************************ */
        if (m_actionsQueue.length())
        {
            InterfaceAction cmd = m_actionsQueue.takeFirst();

            switch (cmd.action)
            {
                case OpenLine:
                {
                    quint32 line = cmd.param1.toUInt();
                    bool input = cmd.param2.toBool();

                    if (DMXUSBWidget::open(line, input) == false)
                    {
                        close(line, input);
                        m_isThreadRunning = false;
                        break;
                    }

                    if (iface()->clearRts() == false)
                    {
                        close(line, input);
                        m_isThreadRunning = false;
                        break;
                    }

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

                    if (input)
                        readInput = true;
                }
                break;
                case CloseLine:
                {
                    //quint32 line = cmd.param1.toUInt();
                    bool input = cmd.param2.toBool();

                    // disable input if no longer needed
                    if (input == true && openInputLines() == 1)
                        readInput = false;
                }
                break;
                case RDMCommand:
                break;
            }
        }

        // no open output lines: do nothing
        if (openOutputLines() == 0 && openInputLines() == 0)
            goto framesleep;

        /* **************************************************************
         *                  SEND DMX DATA TO OUTPUT PORTS
         * ************************************************************ */
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
                    char val = m_outputLines[i].m_universeData[j];

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
                        m_outputMutex.lock();
                        if (iface()->write(request) == false)
                        {
                            qWarning() << Q_FUNC_INFO << name() << "will not accept MIDI data";
                            m_outputMutex.unlock();
                            continue;
                        }
                        m_outputMutex.unlock();
                    }
                }
            }
            else
            {
                QByteArray request;
                request.append(ENTTEC_PRO_START_OF_MSG); // Start byte

                // Command - port selection
                if (m_dmxKingMode)
                    request.append(DMXKING_SEND_DMX_PORT1 + i);
                else
                    request.append(i == 0 ? ENTTEC_PRO_SEND_DMX_RQ : ENTTEC_PRO_SEND_DMX_RQ2);

                request.append((dataLen + 1) & 0xff); // Data length LSB
                request.append(((dataLen + 1) >> 8) & 0xff); // Data length MSB
                request.append(char(ENTTEC_PRO_DMX_ZERO)); // DMX start code (Which constitutes the + 1 below)
                request.append(m_outputLines[i].m_universeData);
                request.append(ENTTEC_PRO_END_OF_MSG); // Stop byte

                //qDebug() << "OUTPUT" << request.length() << "bytes on line" << i;
                //qDebug() << "DATA" << QString::number(request.at(5)) << QString::number(request.at(6)) << QString::number(request.at(7));

                /* Write "Output Only Send DMX Packet Request" message */
                m_outputMutex.lock();
                if (iface()->write(request) == false)
                {
                    qWarning() << Q_FUNC_INFO << name() << "will not accept DMX data";
                    m_outputMutex.unlock();
                    continue;
                }
                m_outputMutex.unlock();
            }
        }

        /* **************************************************************
         *                  READ INPUT DATA (DMX/MIDI/RDM)
         * ************************************************************ */
        if (readInput)
        {
            QByteArray payload;
            bool isMIDI = false;

            if (readData(payload, isMIDI, false))
            {
                // count the received MIDI packets.
                // When reaching 3 (cmd + data1 + data2) a complete MIDI packet is ready to be sent
                int midiCounter = 0;
                uchar midiCmd = 0;
                uchar midiData1 = 0;
                uchar midiData2 = 0;

                int devLine = isMIDI ? m_inputLines.count() - 1 : 0;
                int emitLine = m_inputBaseLine + devLine;

                for (int i = 0; i < payload.length(); i++)
                {
                    uchar byte = uchar(payload.at(i));

                    if (isMIDI == false)
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
                            if (MIDI_IS_CMD(byte))
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

void EnttecDMXUSBPro::stopThread()
{
    qDebug() << Q_FUNC_INFO;

    if (m_isThreadRunning == true)
    {
        m_isThreadRunning = false;
        wait();
    }
}

/********************************************************************
 * RDM
 ********************************************************************/

bool EnttecDMXUSBPro::supportRDM()
{
    return true;
}

bool EnttecDMXUSBPro::sendRDMCommand(quint32 universe, quint32 line, uchar command, QVariantList params)
{
    QByteArray ba;
    quint32 devLine = line - m_outputBaseLine;
    int i, len;
    bool ok;
    int discoveryFailureCount = 0;
    int discoveryNoReplyCount = 0;

    if (m_rdm == NULL)
        m_rdm = new RDMProtocol();

    QString sn = m_proSerial.isEmpty() ? serial() : m_proSerial;
    quint32 devID = sn.toUInt(&ok, 16);

    m_rdm->setEstaID(0x454E);
    m_rdm->setDeviceId(devLine == 1 ? devID + 1 : devID);

    m_rdm->packetizeCommand(command, params, true, ba);
    len = ba.length();

    ba.prepend(len >> 8);
    ba.prepend(len & 0xFF);

    if (command == DISCOVERY_COMMAND)
    {
        ba.prepend(devLine == 1 ? ENTTEC_PRO_RDM_DISCOVERY_REQ2 : ENTTEC_PRO_RDM_DISCOVERY_REQ);
    }
    else if (params.length() >= 2)
    {
        ba.prepend(devLine == 1 ? ENTTEC_PRO_RDM_SEND2 : ENTTEC_PRO_RDM_SEND);
    }
    ba.prepend(ENTTEC_PRO_START_OF_MSG);

    ba.append(ENTTEC_PRO_END_OF_MSG);
#ifdef DEBUG_RDM
    qDebug().nospace().noquote() << "[RDM] Sending RDM command 0x" << QString::number(command, 16) << " with params: " << params;
    qDebug() << "[RDM] Sending RDM command" << universe << line << ba.toHex(',');
#endif

    QMutexLocker locker(&m_outputMutex);
    if (iface()->write(ba) == false)
    {
        qWarning() << Q_FUNC_INFO << name() << "will not accept RDM data";
        return false;
    }

    for (i = 0; i < RDM_MAX_RETRY; i++)
    {
        QByteArray reply;
        bool isMIDI = false;
        int bytesRead = readData(reply, isMIDI, true);

        if (bytesRead)
        {
            QVariantMap values;
            bool result = false;

            if (command == DISCOVERY_COMMAND)
                result = m_rdm->parseDiscoveryReply(reply, values);
            else
                result = m_rdm->parsePacket(reply, values);

            if (result == true)
            {
                discoveryFailureCount = 0;
                discoveryNoReplyCount = 0;
                emit rdmValueChanged(universe, line, values);
                break;
            }
            else
            {
                discoveryFailureCount++;
            }
        }

        // no reply to discovery at all
        if (command == DISCOVERY_COMMAND && bytesRead == 0 && discoveryFailureCount == 0)
            discoveryNoReplyCount++;

        // nothing read. Sleep a bit and retry
        QThread::msleep(50);
        //qDebug() << "RETRY TO READ" << i;
    }

    if (discoveryFailureCount)
    {
        QVariantMap values;
        values.insert("DISCOVERY_ERRORS", discoveryFailureCount);
        emit rdmValueChanged(universe, line, values);
    }
    else if (discoveryNoReplyCount)
    {
        QVariantMap values;
        values.insert("DISCOVERY_NO_REPLY", 1);
        emit rdmValueChanged(universe, line, values);
    }

    if (command != DISCOVERY_COMMAND && i == RDM_MAX_RETRY)
        return false;

    return true;
}
