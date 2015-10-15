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

#include "artnetplugin.h"
#include "configureartnet.h"

#include <QSettings>
#include <QDebug>

ArtNetPlugin::~ArtNetPlugin()
{
}

void ArtNetPlugin::init()
{
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    if (m_netInterfaces.isEmpty())
        m_interfacesTotalCount = 0;

    if (interfaces.count() == m_interfacesTotalCount)
        return;

    m_interfacesTotalCount = interfaces.count();

    for(int i = 0; i < interfaces.count(); i++)
    {
        QNetworkInterface interface = interfaces.at(i);

        foreach (QNetworkAddressEntry entry, interface.addressEntries())
        {
            QHostAddress addr = entry.ip();
            if (addr.protocol() != QAbstractSocket::IPv6Protocol)
            {
                ArtNetIO tmpIO;
                tmpIO.IPAddress = entry.ip().toString();
                if (addr == QHostAddress::LocalHost)
                    tmpIO.MACAddress = "11:22:33:44:55:66";
                else
                    tmpIO.MACAddress = interface.hardwareAddress();
                tmpIO.controller = NULL;

                bool alreadyInList = false;
                for(int j = 0; j < m_IOmapping.count(); j++)
                {
                    if (m_IOmapping.at(j).MACAddress == tmpIO.MACAddress)
                    {
                        alreadyInList = true;
                        break;
                    }
                }
                if (alreadyInList == false)
                {
                    m_IOmapping.append(tmpIO);
                    m_netInterfaces.append(entry);
                }
            }
        }
    }
}

QString ArtNetPlugin::name()
{
    return QString("ArtNet");
}

int ArtNetPlugin::capabilities() const
{
    return QLCIOPlugin::Output | QLCIOPlugin::Input | QLCIOPlugin::Infinite;
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

/*********************************************************************
 * Outputs
 *********************************************************************/
QStringList ArtNetPlugin::outputs()
{
    QStringList list;
    int j = 0;

    init();

    foreach (ArtNetIO line, m_IOmapping)
    {
        list << QString("%1: %2").arg(j + 1).arg(line.IPAddress);
        j++;
    }
    return list;
}

QString ArtNetPlugin::outputInfo(quint32 output)
{
    init();

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
    init();

    if (output >= (quint32)m_IOmapping.length())
        return false;

    qDebug() << "[ArtNet] Open output on address :" << m_IOmapping.at(output).IPAddress;

    // if the controller doesn't exist, create it
    if (m_IOmapping[output].controller == NULL)
    {
        ArtNetController *controller = new ArtNetController(m_IOmapping.at(output).IPAddress,
                                                            m_netInterfaces.at(output),
                                                            m_IOmapping.at(output).MACAddress,
                                                            ArtNetController::Output, output, this);
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

void ArtNetPlugin::writeUniverse(quint32 universe, quint32 output, const QByteArray &data)
{
    if (output >= (quint32)m_IOmapping.count())
        return;

    ArtNetController *controller = m_IOmapping[output].controller;
    if (controller != NULL)
        controller->sendDmx(universe, data);
}

/*************************************************************************
  * Inputs
  *************************************************************************/  
QStringList ArtNetPlugin::inputs()
{
    QStringList list;
    int j = 0;

    init();

    foreach (ArtNetIO line, m_IOmapping)
    {
        list << QString("%1: %2").arg(j + 1).arg(line.IPAddress);
        j++;
    }
    return list;
}

bool ArtNetPlugin::openInput(quint32 input, quint32 universe)
{
    init();

    if (input >= (quint32)m_IOmapping.length())
        return false;

    qDebug() << "[ArtNet] Open input on address :" << m_IOmapping.at(input).IPAddress;

    // if the controller doesn't exist, create it
    if (m_IOmapping[input].controller == NULL)
    {
        ArtNetController *controller = new ArtNetController(m_IOmapping.at(input).IPAddress,
                                                            m_netInterfaces.at(input),
                                                            m_IOmapping.at(input).MACAddress,
                                                            ArtNetController::Input, input, this);
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
    init();

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
        str += tr("Status: Open");
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

    if (name == ARTNET_OUTPUTIP)
        controller->setOutputIPAddress(universe, value.toString());
    else if (name == ARTNET_OUTPUTUNI)
        controller->setOutputUniverse(universe, value.toUInt());
    else if (name == ARTNET_TRANSMITMODE)
        controller->setTransmissionMode(universe, ArtNetController::stringToTransmissionMode(value.toString()));

    QLCIOPlugin::setParameter(universe, line, type, name, value);
}

QList<QNetworkAddressEntry> ArtNetPlugin::interfaces()
{
    return m_netInterfaces;
}

QList<ArtNetIO> ArtNetPlugin::getIOMapping()
{
    return m_IOmapping;
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(artnetplugin, ArtNetPlugin)
#endif
