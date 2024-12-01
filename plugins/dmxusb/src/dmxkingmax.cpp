/*
  Q Light Controller
  dmxkingmax.cpp

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

#include "dmxkingmax.h"
#include "artnetpacketizer.h"

#define ENTTEC_PRO_START_OF_MSG  char(0x7E)
#define ENTTEC_PRO_END_OF_MSG    char(0xE7)

#define DMXKING_UDP_SEND_LABEL      char(0x6F)
#define DMXKING_UDP_RECEIVE_LABEL   char(0x70)

DMXKingMAX::DMXKingMAX(DMXInterface *iface, quint32 outputLine, quint32 inputLine)
    : QThread(NULL)
    , DMXUSBWidget(iface, outputLine, DEFAULT_OUTPUT_FREQUENCY)
    , m_threadRunning(false)
    , m_rdm(NULL)
{
    m_inputBaseLine = inputLine;

    getDeviceInfo();
}

DMXKingMAX::~DMXKingMAX()
{
    qDebug() << Q_FUNC_INFO;
    close(m_inputBaseLine, true);
    close(m_outputBaseLine, false);

    if (m_rdm)
        free(m_rdm);
}

DMXUSBWidget::Type DMXKingMAX::type() const
{
    return DMXUSBWidget::DMXKingMax;
}

QString DMXKingMAX::additionalInfo() const
{
    QString info;

    info += QString("<P>");

    info += QString("<B>%1:</B> %2").arg(tr("Protocol")).arg("ultraDMX USB Pro");
    info += QString("<BR>");
    info += QString("<B>%1:</B> DMXKing").arg(tr("Manufacturer"));
    info += QString("<BR>");
    info += QString("<B>%1:</B> %2").arg(tr("Serial number")).arg(m_serialNumber);
    info += QString("</P>");

    return info;
}

void DMXKingMAX::getDeviceInfo()
{
    QByteArray pollPacket;
    ArtNetPacketizer packetizer;
    packetizer.setupArtNetPoll(pollPacket);

    if (iface()->open() == false)
        return;

    iface()->clearRts();

    if (sendUDPMessage(pollPacket) == false)
    {
        qDebug() << "Failed to send ArtPoll data";
        return;
    }

    usleep(100000);

    bool noReply = false;

    while (noReply == false)
    {
        QByteArray reply;
        ArtNetNodeInfo devNode;

        if (receiveUDPMessage(reply) == false)
        {
            qDebug() << "Failed to receive ArtPollReply data";
            break;
        }

        qDebug() << "Universe" << devNode.universe << "isInput" << devNode.isInput << "isOutput" << devNode.isOutput;

        if (!packetizer.fillArtPollReplyInfo(reply, devNode))
        {
            qWarning() << "[DMXKing] Bad ArtPollReply received";
            break;
        }
        msleep(1000);
    }

    iface()->close();
}

bool DMXKingMAX::sendUDPMessage(QByteArray &data)
{
    QByteArray packet;
    ushort udpDataLen = data.length() + 8;

    // Append packet header
    packet.append(ENTTEC_PRO_START_OF_MSG);
    packet.append(DMXKING_UDP_SEND_LABEL);
    packet.append(udpDataLen & 0xFF);        // UDP data length LSB
    packet.append((udpDataLen >> 8) & 0xFF); // UDP data length MSB

    // Append UDP Protocol Datagram info
    packet.append(char(0x19));  // source port MSB
    packet.append(char(0x79));  // source port LSB
    packet.append(char(0x19));  // destination port MSB - Art-Net
    packet.append(char(0x36));  // destination port LSB - Art-Net
    packet.append((udpDataLen >> 8) & 0xFF); // UDP data length MSB
    packet.append(udpDataLen & 0xFF);        // UDP data length LSB
    packet.append(char(0x00));  // checksum MSB - unused
    packet.append(char(0x00));  // checksum LSB - unused

    packet.append(data);
    packet.append(ENTTEC_PRO_END_OF_MSG);

    if (iface()->write(packet) == false)
    {
        qWarning() << Q_FUNC_INFO << name() << "cannot write UDP message";
        return false;
    }

    return true;
}

bool DMXKingMAX::receiveUDPMessage(QByteArray &data)
{
    QByteArray recvData = iface()->read(1024);

    if (recvData.isEmpty())
    {
        qDebug() << "No data received!";
        return false;
    }

    if (recvData.at(0) != ENTTEC_PRO_START_OF_MSG)
    {
        qDebug() << "Bad start code. Got" << recvData.at(0) << "instead of" << ENTTEC_PRO_START_OF_MSG;
        return false;
    }

    if (recvData.at(1) != DMXKING_UDP_RECEIVE_LABEL)
    {
        qDebug() << "Bad label code. Got" << recvData.at(1) << "instead of" << DMXKING_UDP_RECEIVE_LABEL;
        return false;
    }

    int udpLen = (uchar(recvData.at(3)) << 8) + uchar(recvData.at(2));

    data = recvData.mid(12, udpLen - 8);
    qDebug() << "Packet payload:" << data.toHex(',');

    return true;
}

void DMXKingMAX::stopThread()
{
    qDebug() << Q_FUNC_INFO;

    if (m_threadRunning == true)
    {
        m_threadRunning = false;
        wait();
    }
}

void DMXKingMAX::run()
{
    qDebug() << "OUTPUT thread started";
    QElapsedTimer timer;
    ArtNetPacketizer packetizer;

    m_threadRunning = true;

    while (m_threadRunning == true)
    {
        timer.restart();

        // no open output lines: do nothing
        if (openOutputLines() == 0)
            goto framesleep;

        // output data first
        for (int i = 0; i < m_outputLines.count(); i++)
        {

            int dataLen = m_outputLines[i].m_universeData.length();
            if (dataLen == 0)
                continue;

            QByteArray outPacket;
            packetizer.setupArtNetDmx(outPacket, i, m_outputLines[i].m_universeData);
            if (iface()->write(outPacket) == false)
            {
                qWarning() << Q_FUNC_INFO << name() << "will not accept DMX data";
                continue;
            }
        }

framesleep:
        int timetoSleep = m_frameTimeUs - (timer.nsecsElapsed() / 1000);
        if (timetoSleep < 0)
            qWarning() << "DMX output is running late !";
        else
            usleep(timetoSleep);
    }
}

/****************************************************************************
 * Open & Close
 ****************************************************************************/

bool DMXKingMAX::open(quint32 line, bool input)
{
    if (DMXUSBWidget::open(line, input) == false)
        return close(line, input);

    if (iface()->clearRts() == false)
        return close(line, input);

    // start the input/output thread
    if (m_threadRunning == false)
        start();

    return true;
}

bool DMXKingMAX::close(quint32 line, bool input)
{
    stopThread();

    return DMXUSBWidget::close(line, input);
}

/************************************************************************
 * Name & Serial
 ************************************************************************/

QString DMXKingMAX::uniqueName(ushort line, bool input) const
{
    QString devName;

    if (realName().isEmpty() == false)
        devName = realName();
    else
        devName = name();

    if (input)
    {
        return QString("%1 - %2 %3 - (S/N: %4)").arg(devName, QObject::tr("DMX Input"), QString::number(line + 1), m_serialNumber);
    }
    else
    {
        return QString("%1 - %2 %3 - (S/N: %4)").arg(devName, QObject::tr("DMX Output"), QString::number(line + 1), m_serialNumber);
    }
}

/************************************************************************
 * Output
 ************************************************************************/

bool DMXKingMAX::writeUniverse(quint32 universe, quint32 output, const QByteArray &data, bool dataChanged)
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

/********************************************************************
 * RDM
 ********************************************************************/

bool DMXKingMAX::supportRDM()
{
    return true;
}

bool DMXKingMAX::sendRDMCommand(quint32 universe, quint32 line, uchar command, QVariantList params)
{
    // TODO
    Q_UNUSED(universe)
    Q_UNUSED(line)
    Q_UNUSED(command)
    Q_UNUSED(params)

    return false;
}

