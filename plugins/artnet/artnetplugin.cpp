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
    m_IOmapping.clear();

    foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
    {
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
                m_IOmapping.append(tmpIO);

                m_netInterfaces.append(entry);
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
    if (m_IOmapping.count() < 2)
        init();
    foreach (ArtNetIO line, m_IOmapping)
    {
        list << QString(tr("%1: %2")).arg(j + 1).arg(line.IPAddress);
        j++;
    }
    return list;
}

QString ArtNetPlugin::outputInfo(quint32 output)
{
    if (m_IOmapping.count() < 2)
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

bool ArtNetPlugin::openOutput(quint32 output)
{
    if (m_IOmapping.count() < 2)
        init();

    if (output >= (quint32)m_IOmapping.length())
        return false;

    qDebug() << "Open output with address :" << m_IOmapping.at(output).IPAddress;

    // already open ? Just add the type flag
    if (m_IOmapping[output].controller != NULL)
    {
        m_IOmapping[output].controller->setType(
                    (ArtNetController::Type)(m_IOmapping[output].controller->type() | ArtNetController::Output));
        m_IOmapping[output].controller->changeReferenceCount(ArtNetController::Output, +1);
        return true;
    }

    // not open ? Create a new ArtNetController
    ArtNetController *controller = new ArtNetController(m_IOmapping.at(output).IPAddress,
                                                        m_netInterfaces, m_IOmapping.at(output).MACAddress,
                                                        ArtNetController::Output, output, this);
    m_IOmapping[output].controller = controller;
    return true;
}

void ArtNetPlugin::closeOutput(quint32 output)
{
    if (output >= (quint32)m_IOmapping.length())
        return;
    ArtNetController *controller = m_IOmapping.at(output).controller;
    if (controller != NULL)
    {
        controller->changeReferenceCount(ArtNetController::Output, -1);
        // if a ArtNetController is also open as input
        // then just remove the output capability
        if (controller->type() & ArtNetController::Input)
        {
            controller->setType(ArtNetController::Input);
        }

        if (controller->referenceCount(ArtNetController::Input) == 0 &&
            controller->referenceCount(ArtNetController::Output) == 0)
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
    if (m_IOmapping.count() < 2)
        init();
    foreach (ArtNetIO line, m_IOmapping)
    {
        list << QString(tr("%1: %2")).arg(j + 1).arg(line.IPAddress);
        j++;
    }
    return list;
}

bool ArtNetPlugin::openInput(quint32 input)
{
    if (m_IOmapping.count() < 2)
        init();

    if (input >= (quint32)m_IOmapping.length())
        return false;

    qDebug() << "Open input with address :" << m_IOmapping.at(input).IPAddress;

    // already open ? Just add the type flag
    if (m_IOmapping[input].controller != NULL)
    {
        m_IOmapping[input].controller->setType(
                    (ArtNetController::Type)(m_IOmapping[input].controller->type() | ArtNetController::Input));
        m_IOmapping[input].controller->changeReferenceCount(ArtNetController::Input, +1);
        return true;
    }

    // not open ? Create a new ArtNetController
    ArtNetController *controller = new ArtNetController(m_IOmapping.at(input).IPAddress,
                                                        m_netInterfaces, m_IOmapping.at(input).MACAddress,
                                                        ArtNetController::Input, input, this);
    connect(controller, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)),
            this, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)));
    m_IOmapping[input].controller = controller;

    return true;
}

void ArtNetPlugin::closeInput(quint32 input)
{
    if (input >= (quint32)m_IOmapping.length())
        return;
    ArtNetController *controller = m_IOmapping.at(input).controller;
    if (controller != NULL)
    {
        controller->changeReferenceCount(ArtNetController::Input, -1);
        // if a ArtNetController is also open as output
        // then just remove the input capability
        if (controller->type() & ArtNetController::Output)
        {
            controller->setType(ArtNetController::Output);
        }

        if (controller->referenceCount(ArtNetController::Input) == 0 &&
            controller->referenceCount(ArtNetController::Output) == 0)
        {
            delete m_IOmapping[input].controller;
            m_IOmapping[input].controller = NULL;
        }
    }
}

QString ArtNetPlugin::inputInfo(quint32 input)
{
    if (m_IOmapping.count() < 2)
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
