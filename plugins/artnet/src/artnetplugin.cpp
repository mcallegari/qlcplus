/*
  Q Light Controller Plus
  artnetplugin.cpp

  Copyright (c) Massimo Callegari

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

#include "artnetplugin.h"
#include "configureartnet.h"

bool addressCompare(const ArtNetIO &v1, const ArtNetIO &v2)
{
    return v1.address.ip().toString() < v2.address.ip().toString();
}

ArtNetPlugin::~ArtNetPlugin()
{
}

void ArtNetPlugin::init()
{
    QSettings settings;
    QVariant value = settings.value(SETTINGS_IFACE_WAIT_TIME);
    if (value.isValid() == true)
        m_ifaceWaitTime = value.toInt();
    else
        m_ifaceWaitTime = 0;

    foreach (QNetworkInterface iface, QNetworkInterface::allInterfaces())
    {
        foreach (QNetworkAddressEntry entry, iface.addressEntries())
        {
            QHostAddress addr = entry.ip();
            if (addr.protocol() != QAbstractSocket::IPv6Protocol)
            {
                ArtNetIO tmpIO;
                tmpIO.iface = iface;
                tmpIO.address = entry;
                tmpIO.controller = NULL;

                bool alreadyInList = false;
                for (int j = 0; j < m_IOmapping.count(); j++)
                {
                    if (m_IOmapping.at(j).address == tmpIO.address)
                    {
                        alreadyInList = true;
                        break;
                    }
                }
                if (alreadyInList == false)
                {
                    m_IOmapping.append(tmpIO);
                }
            }
        }
    }
    std::sort(m_IOmapping.begin(), m_IOmapping.end(), addressCompare);
}

QString ArtNetPlugin::name()
{
    return QString("ArtNet");
}

int ArtNetPlugin::capabilities() const
{
    return QLCIOPlugin::Output | QLCIOPlugin::Input | QLCIOPlugin::Infinite | QLCIOPlugin::RDM;
}

QString ArtNetPlugin::pluginInfo()
{
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    str += QString("<P>");
    str += QString("<H3>%1</H3>").arg(name());
    str += tr("This plugin provides DMX output for devices supporting the ArtNet communication protocol.");
    str += QString("</P>");

    return str;
}

bool ArtNetPlugin::requestLine(quint32 line)
{
    int retryCount = 0;

    while (line >= (quint32)m_IOmapping.length())
    {
        qDebug() << "[ArtNet] cannot open line" << line << "(available:" << m_IOmapping.length() << ")";
        if (m_ifaceWaitTime)
        {
            Sleep(1000);
            init();
        }
        if (retryCount++ >= m_ifaceWaitTime)
            return false;
    }

    return true;
}

/*********************************************************************
 * Outputs
 *********************************************************************/
QStringList ArtNetPlugin::outputs()
{
    QStringList list;

    init();

    foreach (ArtNetIO line, m_IOmapping)
        list << line.address.ip().toString();

    return list;
}

QString ArtNetPlugin::outputInfo(quint32 output)
{
    if (output >= (quint32)m_IOmapping.length())
        return QString();

    QString str;

    str += QString("<H3>%1 %2</H3>").arg(tr("Output")).arg(outputs()[output]);
    str += QString("<P>");
    ArtNetController *ctrl = m_IOmapping.at(output).controller;
    if (ctrl == NULL || ctrl->type() == ArtNetController::Input)
        str += tr("Status: Not open");
    else
    {
        str += tr("Status: Open");
        str += QString("<BR>");

        QString boundString;
        if (!ctrl->socketBound())
            boundString = QString("<FONT COLOR=\"#aa0000\">%1</FONT>").arg(tr("No"));
        else
           boundString = QString("<FONT COLOR=\"#00aa00\">%1</FONT>").arg(tr("Yes"));
        str += QString("<B>%1:</B> %2").arg(tr("Can receive nodes information")).arg(boundString);
        str += QString("<BR>");

        str += tr("Nodes discovered: ");
        str += QString("%1").arg(ctrl->getNodesList().size());
        str += QString("<BR>");
        str += tr("Packets sent: ");
        str += QString("%1").arg(ctrl->getPacketSentNumber());
    }
    str += QString("</P>");
    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

bool ArtNetPlugin::openOutput(quint32 output, quint32 universe)
{
    if (requestLine(output) == false)
        return false;

    qDebug() << "[ArtNet] Open output on address :" << m_IOmapping.at(output).address.ip().toString();

    // if the controller doesn't exist, create it
    if (m_IOmapping[output].controller == NULL)
    {
        ArtNetController *controller = new ArtNetController(m_IOmapping.at(output).iface,
                                                            m_IOmapping.at(output).address,
                                                            getUdpSocket(),
                                                            output, this);
        connect(controller, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)),
                this, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)));
        connect(controller, SIGNAL(rdmValueChanged(quint32, quint32, QVariantMap)),
                this , SIGNAL(rdmValueChanged(quint32, quint32, QVariantMap)));
        m_IOmapping[output].controller = controller;
    }

    m_IOmapping[output].controller->addUniverse(universe, ArtNetController::Output);
    addToMap(universe, output, Output);

    return true;
}

void ArtNetPlugin::closeOutput(quint32 output, quint32 universe)
{
    if (output >= (quint32)m_IOmapping.length())
        return;

    removeFromMap(output, universe, Output);
    ArtNetController *controller = m_IOmapping.at(output).controller;
    if (controller != NULL)
    {
        controller->removeUniverse(universe, ArtNetController::Output);
        if (controller->universesList().count() == 0)
        {
            delete m_IOmapping[output].controller;
            m_IOmapping[output].controller = NULL;
        }
    }
}

void ArtNetPlugin::writeUniverse(quint32 universe, quint32 output, const QByteArray &data, bool dataChanged)
{
    if (output >= (quint32)m_IOmapping.count())
        return;

    ArtNetController *controller = m_IOmapping.at(output).controller;
    if (controller != NULL)
        controller->sendDmx(universe, data, dataChanged);
}

/*************************************************************************
  * Inputs
  *************************************************************************/
QStringList ArtNetPlugin::inputs()
{
    QStringList list;

    init();

    foreach (ArtNetIO line, m_IOmapping)
        list << line.address.ip().toString();

    return list;
}

bool ArtNetPlugin::openInput(quint32 input, quint32 universe)
{
    if (requestLine(input) == false)
        return false;

    // if the controller doesn't exist, create it.
    // We need to have only one input controller.
    if (m_IOmapping[input].controller == NULL)
    {
        ArtNetController *controller = new ArtNetController(m_IOmapping.at(input).iface,
                                                            m_IOmapping.at(input).address,
                                                            getUdpSocket(),
                                                            input, this);
        connect(controller, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)),
                this, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)));
        m_IOmapping[input].controller = controller;
    }

    m_IOmapping[input].controller->addUniverse(universe, ArtNetController::Input);
    addToMap(universe, input, Input);

    return true;
}

void ArtNetPlugin::closeInput(quint32 input, quint32 universe)
{
    if (input >= (quint32)m_IOmapping.length())
        return;

    removeFromMap(input, universe, Input);
    ArtNetController *controller = m_IOmapping.at(input).controller;
    if (controller != NULL)
    {
        controller->removeUniverse(universe, ArtNetController::Input);
        if (controller->universesList().count() == 0)
        {
            delete m_IOmapping[input].controller;
            m_IOmapping[input].controller = NULL;
        }
    }
}

QString ArtNetPlugin::inputInfo(quint32 input)
{
    if (input >= (quint32)m_IOmapping.length())
        return QString();

    QString str;

    str += QString("<H3>%1 %2</H3>").arg(tr("Input")).arg(inputs()[input]);
    str += QString("<P>");
    ArtNetController *ctrl = m_IOmapping.at(input).controller;
    if (ctrl == NULL || ctrl->type() == ArtNetController::Output)
        str += tr("Status: Not open");
    else
    {
        QString boundString;
        if (!ctrl->socketBound())
            boundString = QString("<FONT COLOR=\"#aa0000\">%1</FONT>").arg(tr("Bind failed"));
        else
           boundString = QString("<FONT COLOR=\"#00aa00\">%1</FONT>").arg(tr("Open"));
        str += QString("<B>%1:</B> %2").arg(tr("Status")).arg(boundString);
        str += QString("<BR>");

        str += tr("Packets received: ");
        str += QString("%1").arg(ctrl->getPacketReceivedNumber());
    }
    str += QString("</P>");
    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

/*********************************************************************
 * Configuration
 *********************************************************************/
void ArtNetPlugin::configure()
{
    ConfigureArtNet conf(this);
    conf.exec();
}

bool ArtNetPlugin::canConfigure()
{
    return true;
}

void ArtNetPlugin::setParameter(quint32 universe, quint32 line, Capability type,
                                QString name, QVariant value)
{
    if (line >= (quint32)m_IOmapping.length())
        return;

    ArtNetController *controller = m_IOmapping.at(line).controller;
    if (controller == NULL)
        return;

    // If the Controller parameter is restored to its default value,
    // unset the corresponding plugin parameter
    bool unset;

    if (type == Input)
    {
        if (name == ARTNET_INPUTUNI)
            unset = controller->setInputUniverse(universe, value.toUInt());
        else
        {
            qWarning() << Q_FUNC_INFO << name << "is not a valid ArtNet input parameter";
            return;
        }
    }
    else // if (type == Output)
    {
        if (name == ARTNET_OUTPUTIP)
            unset = controller->setOutputIPAddress(universe, value.toString());
        else if (name == ARTNET_OUTPUTUNI)
            unset = controller->setOutputUniverse(universe, value.toUInt());
        else if (name == ARTNET_TRANSMITMODE)
            unset = controller->setTransmissionMode(universe, ArtNetController::stringToTransmissionMode(value.toString()));
        else
        {
            qWarning() << Q_FUNC_INFO << name << "is not a valid ArtNet output parameter";
            return;
        }
    }

    if (unset)
        QLCIOPlugin::unSetParameter(universe, line, type, name);
    else
        QLCIOPlugin::setParameter(universe, line, type, name, value);
}

QList<ArtNetIO> ArtNetPlugin::getIOMapping()
{
    return m_IOmapping;
}

/********************************************************************
 * RDM
 ********************************************************************/

bool ArtNetPlugin::sendRDMCommand(quint32 universe, quint32 line, uchar command, QVariantList params)
{
    qDebug() << "Sending RDM command on universe" << universe << "and line" << line;
    if (line >= (quint32)m_IOmapping.count())
        return false;

    ArtNetController *controller = m_IOmapping.at(line).controller;
    if (controller != NULL)
        return controller->sendRDMCommand(universe, command, params);

    return false;
}

/*********************************************************************
 * ArtNet socket
 *********************************************************************/

QSharedPointer<QUdpSocket> ArtNetPlugin::getUdpSocket()
{
    // Is the socket already present ?
    QSharedPointer<QUdpSocket> udpSocket(m_udpSocket);
    if (udpSocket)
        return udpSocket;

    // Create a new socket
    udpSocket = QSharedPointer<QUdpSocket>(new QUdpSocket());
    m_udpSocket = udpSocket.toWeakRef();

    if (udpSocket->bind(ARTNET_PORT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))
    {
        connect(udpSocket.data(), SIGNAL(readyRead()),
                this, SLOT(slotReadyRead()));
    }
    else
    {
        qWarning() << "ArtNet: could not bind socket to address" << QString("0:%2").arg(ARTNET_PORT);
    }
    return udpSocket;
}

void ArtNetPlugin::slotReadyRead()
{
    QUdpSocket* udpSocket = qobject_cast<QUdpSocket*>(sender());
    Q_ASSERT(udpSocket != NULL);

    QByteArray datagram;
    QHostAddress senderAddress;
    while (udpSocket->hasPendingDatagrams())
    {
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size(), &senderAddress);
        handlePacket(datagram, senderAddress);
    }
}

void ArtNetPlugin::handlePacket(QByteArray const& datagram, QHostAddress const& senderAddress)
{
    // A first filter: look for a controller on the same subnet as the sender.
    // This allows having the same ArtNet Universe on 2 different network interfaces.
    foreach (ArtNetIO io, m_IOmapping)
    {
        if (senderAddress.isInSubnet(io.address.ip(), io.address.prefixLength()))
        {
            if (io.controller != NULL)
                io.controller->handlePacket(datagram, senderAddress);
            return;
        }
    }
    // Packet comming from another subnet. This is an unusual case.
    // We stop at the first controller that handles this packet.
    foreach (ArtNetIO io, m_IOmapping)
    {
        if (io.controller != NULL)
        {
            if (io.controller->handlePacket(datagram, senderAddress))
                break;
        }
    }
}
