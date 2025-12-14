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

#define MAX_READ_ATTEMPTS   5

//#define DEBUG_RDM

/****************************************************************************
 * Initialization
 ****************************************************************************/

EnttecDMXUSBPro::EnttecDMXUSBPro(DMXInterface *iface, quint32 outputLine, quint32 inputLine)
    : QThread(NULL)
    , DMXUSBWidget(iface, outputLine, DEFAULT_OUTPUT_FREQUENCY)
    , m_dmxKingMode(false)
    , m_isThreadRunning(false)
    , m_rdm(NULL)
    , m_universe(UINT_MAX)
{
    m_inputBaseLine = inputLine;

    // configure one input/output port
    QList<int> ports;
    ports << (DMXUSBWidget::DMX | DMXUSBWidget::Output | DMXUSBWidget::Input);
    setPortsMapping(ports);

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
    if (openPortsCount() == 1)
        stopThread();

    return DMXUSBWidget::close(line, input);
}

/************************************************************************
 * Input
 ************************************************************************/

int EnttecDMXUSBPro::readData(QByteArray &payload, bool &isMIDI, int RDM)
{
    static QByteArray buffer;
    int attempt = 0;

    isMIDI = false;
    payload.clear();

    while (attempt < MAX_READ_ATTEMPTS)
    {
        QByteArray chunk = iface()->read(READ_CHUNK_SIZE);

        if (chunk.isEmpty())
        {
            // among DMX data, the RDM request might be in a queue
            // so give some time to the adapter to output the request
            if (RDM == GetSetCommand)
            {
                msleep(100);
                attempt++;
                continue;
            }

            if (attempt == 0)
                return 0; // No data at all
        }

        buffer.append(chunk);

        //qDebug() << "Data in buffer:" << buffer.toHex(',');

        int startIdx = buffer.indexOf(ENTTEC_PRO_START_OF_MSG);

        while (startIdx != -1 && buffer.size() >= startIdx + 5)
        {
            char label = buffer[startIdx + 1];
            int lenLSB = static_cast<uchar>(buffer[startIdx + 2]);
            int lenMSB = static_cast<uchar>(buffer[startIdx + 3]);
            int dataLen = (lenMSB << 8) | lenLSB;
            int packetLen = 4 + dataLen + 1;

            if (buffer.size() < startIdx + packetLen)
                break; // Incomplete packet

            if (buffer[startIdx + packetLen - 1] != ENTTEC_PRO_END_OF_MSG)
            {
                qWarning() << Q_FUNC_INFO << "Malformed packet (missing END byte)";
                buffer.remove(0, startIdx + 1);
                startIdx = buffer.indexOf(ENTTEC_PRO_START_OF_MSG);
                continue;
            }

            QByteArray packet = buffer.mid(startIdx, packetLen);
            buffer.remove(0, startIdx + packetLen); // Consume the packet

            // Skip RDM timeout packets
            if (label == ENTTEC_PRO_RDM_RECV_TIMEOUT || label == ENTTEC_PRO_RDM_RECV_TIMEOUT2)
            {
                //qDebug() << "RDM timeout received, skipping packet";
                startIdx = buffer.indexOf(ENTTEC_PRO_START_OF_MSG);
                continue;
            }

            // MIDI packet
            if (label == ENTTEC_PRO_MIDI_IN_MSG)
            {
                isMIDI = true;
                payload.append(packet.mid(4, dataLen));

                // Keep scanning next packets while they are MIDI
                while (true)
                {
                    int nextStart = buffer.indexOf(ENTTEC_PRO_START_OF_MSG);
                    if (nextStart == -1 || buffer.size() < nextStart + 5)
                        break;

                    char nextLabel = buffer[nextStart + 1];
                    int lenLSB = static_cast<uchar>(buffer[nextStart + 2]);
                    int lenMSB = static_cast<uchar>(buffer[nextStart + 3]);
                    int nextLen = (lenMSB << 8) | lenLSB;
                    int nextPacketLen = 4 + nextLen + 1;

                    if (buffer.size() < nextStart + nextPacketLen)
                        break; // Wait for more data

                    if (buffer[nextStart + nextPacketLen - 1] != ENTTEC_PRO_END_OF_MSG)
                    {
                        qWarning() << Q_FUNC_INFO << "Malformed MIDI packet (missing END)";
                        buffer.remove(0, nextStart + 1);
                        continue;
                    }

                    if (nextLabel != ENTTEC_PRO_MIDI_IN_MSG)
                        break; // Stop if label changes

                    QByteArray midiPkt = buffer.mid(nextStart, nextPacketLen);
                    payload.append(midiPkt.mid(4, nextLen));
                    buffer.remove(0, nextStart + nextPacketLen);
                }

                return payload.size();
            }

            // Serial number packet — return raw payload as-is
            if (label == ENTTEC_PRO_READ_SERIAL)
            {
                payload = packet.mid(4, dataLen);
                return payload.size();
            }

            // DMX data packet
            if (label == ENTTEC_PRO_RECV_DMX_PKT)
            {
                isMIDI = false;
                int offset = 4;

                uchar status = static_cast<uchar>(packet[offset++]);
                if (status & 0x01)
                    qWarning() << Q_FUNC_INFO << "Widget receive queue overflowed";
                if (status & 0x02)
                    qWarning() << Q_FUNC_INFO << "Widget receive overrun occurred";

                if (RDM == None)
                {
                    uchar startCode = static_cast<uchar>(packet[offset++]);
                    if (startCode != 0)
                        qWarning() << Q_FUNC_INFO << "Non-standard DMX startcode received:" << startCode;
                    dataLen -= 2;
                }
                else
                {
                    dataLen -= 1;
                }

                payload = packet.mid(packet.size() - 1 - dataLen, dataLen);
                return payload.size();
            }

            // Unknown label, skip
            qWarning() << Q_FUNC_INFO << "Unknown label:" << static_cast<int>(label);
            startIdx = buffer.indexOf(ENTTEC_PRO_START_OF_MSG);
        }

        attempt++;
    }

    // No valid packet found after retries
    return 0;
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
        if (m_portsInfo[line].m_portFlags & DMXUSBWidget::MIDI)
            return QString("%1 - %2 - (S/N: %3)").arg(devName, QObject::tr("MIDI Input"), m_proSerial);
        else
            return QString("%1 - %2 - (S/N: %3)").arg(devName, QObject::tr("DMX Input"), m_proSerial);
    }
    else
    {
        if (m_portsInfo[line].m_portFlags & DMXUSBWidget::MIDI)
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
        int bytesRead = readData(reply, notUsed);

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

    quint32 portIndex = lineToPortIndex(output, DMXUSBWidget::Output);
    if (portIndex >= quint32(m_portsInfo.count()))
        return false;

    if (m_portsInfo[portIndex].m_universeData.size() == 0)
    {
        m_portsInfo[portIndex].m_universeData.append(data);
        m_portsInfo[portIndex].m_universeData.append(DMX_CHANNELS - data.size(), 0);
    }

    if (dataChanged)
        m_portsInfo[portIndex].m_universeData.replace(0, data.size(), data);

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
                        quint32 portIndex = lineToPortIndex(line, input ? DMXUSBWidget::Input : DMXUSBWidget::Output);
                        if (input == false)
                        {
                            if (m_portsInfo[portIndex].m_portFlags & DMXUSBWidget::MIDI)
                                configureLine(portIndex, true);
                            else
                                configureLine(portIndex, false);
                        }
                        else
                        {
                            if (m_portsInfo[portIndex].m_portFlags & DMXUSBWidget::MIDI)
                                configureLine(portIndex, true);
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
                    int count = 0;
                    for (int i = 0; i < m_portsInfo.count(); i++)
                    {
                        if (m_portsInfo.at(i).m_openDirection == DMXUSBWidget::Input)
                            count++;
                    }
                    if (input == true && count == 0)
                        readInput = false;
                }
                break;
                case RDMCommand:
                {
                    quint32 line = cmd.param1.toUInt();
                    uchar command = cmd.param2.toUInt();
                    quint32 portIndex = lineToPortIndex(line, DMXUSBWidget::Output);

                    if (m_rdm == NULL)
                        m_rdm = new RDMProtocol();

                    QByteArray ba;
                    int len;
                    bool ok;

                    QString sn = m_proSerial.isEmpty() ? serial() : m_proSerial;
                    quint32 devID = sn.toUInt(&ok, 16);

                    m_rdm->setEstaID(0x454E);
                    m_rdm->setDeviceId(portIndex == 1 ? devID + 1 : devID);

                    m_rdm->packetizeCommand(command, cmd.param3, true, ba);
                    len = ba.length();

                    ba.prepend(len >> 8);
                    ba.prepend(len & 0xFF);

                    if (command == DISCOVERY_COMMAND)
                    {
                        ba.prepend(portIndex == 1 ? ENTTEC_PRO_RDM_DISCOVERY_REQ2 : ENTTEC_PRO_RDM_DISCOVERY_REQ);
                    }
                    else if (cmd.param3.length() >= 2)
                    {
                        ba.prepend(portIndex == 1 ? ENTTEC_PRO_RDM_SEND2 : ENTTEC_PRO_RDM_SEND);
                    }
                    ba.prepend(ENTTEC_PRO_START_OF_MSG);

                    ba.append(ENTTEC_PRO_END_OF_MSG);
#ifdef DEBUG_RDM
                    qDebug().nospace().noquote() << "[RDM] Sending RDM command 0x" << QString::number(command, 16) << " with params: " << cmd.param3;
                    qDebug() << "[RDM] Sending RDM command" << m_universe << portIndex << ba.toHex(',');
#endif
                    int bytesRead = 0;
                    QByteArray reply;

                    for (int r = 0; r < MAX_READ_ATTEMPTS; r++)
                    {
                        if (iface()->write(ba) == false)
                        {
                            qWarning() << Q_FUNC_INFO << name() << "will not accept RDM data";
                            break;
                        }

                        // give some time to reply
                        msleep(10);
                        bool isMIDI = false;
                        bytesRead = readData(reply, isMIDI, command == DISCOVERY_COMMAND ? Discovery : GetSetCommand);
#ifdef DEBUG_RDM
                        qDebug() << "[RDM] Data received" << bytesRead << reply.toHex(',');
#endif
                        if (bytesRead || command == DISCOVERY_COMMAND)
                            break;
                        else
                            msleep(100);
                    }
                    if (bytesRead)
                    {
                        QVariantMap values;
                        bool result = false;

                        if (command == DISCOVERY_COMMAND)
                            result = m_rdm->parseDiscoveryReply(reply, values);
                        else
                            result = m_rdm->parsePacket(reply, values);

                        if (result == true)
                            emit rdmValueChanged(m_universe, line, values);
                        else
                        {
                            values.insert("DISCOVERY_ERRORS", 1);
                            emit rdmValueChanged(m_universe, line, values);
                        }
                    }
                    else
                    {
                        if (command == DISCOVERY_COMMAND)
                        {
                            QVariantMap values;
                            values.insert("DISCOVERY_NO_REPLY", 1);
                            emit rdmValueChanged(m_universe, line, values);
                        }
                        else
                            qDebug() << "No RDM reply received";
                    }
                }
                break;
            }
        }

        // no open output lines: do nothing
        if (openPortsCount() == 0)
            goto framesleep;

        /* **************************************************************
         *               SEND DMX OR MIDI DATA TO OUTPUT PORTS
         * ************************************************************ */
        for (int i = 0; i < m_portsInfo.count(); i++)
        {
            // consider only open output ports
            if (m_portsInfo[i].m_openDirection != DMXUSBWidget::Output)
                continue;

            int dataLen = m_portsInfo[i].m_universeData.length();
            if (dataLen == 0)
                continue;

            if (m_portsInfo[i].m_portFlags & DMXUSBWidget::MIDI)
            {
                QByteArray request;

                if (m_portsInfo[i].m_compareData.size() == 0)
                    m_portsInfo[i].m_compareData.fill(0, 512);

                // send only values that changed
                for (int j = 0; j < m_portsInfo[i].m_universeData.length(); j++)
                {
                    char val = m_portsInfo[i].m_universeData[j];

                    if (val == m_portsInfo[i].m_compareData[j])
                        continue;

                    m_portsInfo[i].m_compareData[j] = val;

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

                        if (iface()->write(request) == false)
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

                // Command - port selection
                if (m_dmxKingMode)
                    request.append(DMXKING_SEND_DMX_PORT1 + i);
                else
                    request.append(i == 0 ? ENTTEC_PRO_SEND_DMX_RQ : ENTTEC_PRO_SEND_DMX_RQ2);

                request.append((dataLen + 1) & 0xff); // Data length LSB
                request.append(((dataLen + 1) >> 8) & 0xff); // Data length MSB
                request.append(char(ENTTEC_PRO_DMX_ZERO)); // DMX start code (Which constitutes the + 1 below)
                request.append(m_portsInfo[i].m_universeData);
                request.append(ENTTEC_PRO_END_OF_MSG); // Stop byte

                //qDebug() << "OUTPUT" << request.length() << "bytes on line" << i;
                //qDebug() << "DATA" << QString::number(request.at(5)) << QString::number(request.at(6)) << QString::number(request.at(7));

                /* Write "Output Only Send DMX Packet Request" message */
                if (iface()->write(request) == false)
                {
                    qWarning() << Q_FUNC_INFO << name() << "will not accept DMX data";
                    continue;
                }
            }
        }

        /* **************************************************************
         *                  READ DMX OR MIDI INPUT DATA
         * ************************************************************ */
        if (readInput)
        {
            QByteArray payload;
            bool isMIDI = false;

            if (readData(payload, isMIDI))
            {
                int devLine = isMIDI ? inputsNumber() - 1 : 0;
                int emitLine = m_inputBaseLine + devLine;

                if (!isMIDI)
                {
                    // DMX input optimization
                    if (payload.size() > 512)
                        payload.truncate(512);

                    if (m_portsInfo[devLine].m_universeData.size() == 0)
                        m_portsInfo[devLine].m_universeData.fill(0, 512);

                    // Skip entire processing if data is the same
                    if (::memcmp(payload.constData(),
                                 m_portsInfo[devLine].m_universeData.constData(),
                                 payload.size()) != 0)
                    {
                        for (int i = 0; i < payload.size(); ++i)
                        {
                            uchar byte = uchar(payload[i]);
                            if (byte != uchar(m_portsInfo[devLine].m_universeData[i]))
                            {
                                m_portsInfo[devLine].m_universeData[i] = byte;
                                emit valueChanged(UINT_MAX, emitLine, i, byte);
                            }
                        }
                    }
                }
                else
                {
                    // MIDI input optimization — process in chunks of 3
                    const uchar *data = reinterpret_cast<const uchar *>(payload.constData());
                    int length = payload.length();

                    for (int i = 0; i < length;)
                    {
                        if (!MIDI_IS_CMD(data[i]))
                        {
                            i++;
                            continue;
                        }

                        uchar midiCmd = data[i];
                        uchar midiData1 = data[i + 1];
                        uchar midiData2 = data[i + 2];

                        //qDebug() << "CMD" << midiCmd << "DATA1" << midiData1 << "DATA2" << midiData2;

                        uint channel = 0;
                        uchar value = 0;
                        if (QLCMIDIProtocol::midiToInput(midiCmd, midiData1, midiData2,
                                                         MAX_MIDI_CHANNELS, &channel, &value))
                        {
                            emit valueChanged(UINT_MAX, emitLine, channel, value);

                            if (midiCmd >= MIDI_BEAT_CLOCK && midiCmd <= MIDI_BEAT_STOP)
                                emit valueChanged(UINT_MAX, emitLine, channel, 0);
                        }

                        i += 3;
                    }
                }
            }
        }

framesleep:
        int timetoSleep = m_frameTimeUs - (timer.nsecsElapsed() / 1000);
        if (timetoSleep < 0)
            qWarning() << "DMX output is running late!";
        else
            usleep(timetoSleep);
    }

    qDebug() << "INPUT/OUTPUT thread terminated";
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
    m_universe = universe;

    InterfaceAction cmd;
    cmd.action = RDMCommand;
    cmd.param1 = line;
    cmd.param2 = command;
    cmd.param3 = params;
    m_actionsQueue.append(cmd);

    return true;
}
