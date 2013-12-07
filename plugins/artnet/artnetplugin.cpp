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
    QSettings settings;

    foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
    {
        foreach (QNetworkAddressEntry entry, interface.addressEntries())
        {
            QHostAddress addr = entry.ip();
            if (addr.protocol() != QAbstractSocket::IPv6Protocol && addr != QHostAddress::LocalHost)
            {
                m_netInterfaces.append(entry);
                m_netMACAddresses.append(interface.hardwareAddress());
            }
        }
    }
    QString key = QString("ArtNetPlugin/outputs");
    QVariant outNum = settings.value(key);
    if (outNum.isValid() == true)
    {
        for (int o = 0; o < outNum.toInt(); o++)
        {
            QString outKey = QString("ArtNetPlugin/Output%1").arg(o);
            QVariant value = settings.value(outKey);
            if (value.isValid() == true)
            {
                // values are stored as: IP#port
                QString outMapStr = value.toString();
                QStringList outMapList = outMapStr.split("#");
                if (outMapList.length() == 2)
                {
                    ArtNetIO tmpIO;
                    tmpIO.IPAddress = outMapList.at(0);
                    tmpIO.port = outMapList.at(1).toInt();
                    tmpIO.controller = NULL;
                    tmpIO.type = ArtNetController::Unknown;
                    m_IOmapping.append(tmpIO);
                }
            }
        }
    }
    else // default mapping: port 0 for each IP found
    {
        foreach (QNetworkAddressEntry entry, m_netInterfaces)
        {
            ArtNetIO tmpIO;
            tmpIO.IPAddress = entry.ip().toString();
            tmpIO.port = 0;
            tmpIO.controller = NULL;
            tmpIO.type = ArtNetController::Unknown;
            m_IOmapping.append(tmpIO);
        }
    }
}

QString ArtNetPlugin::name()
{
    return QString("ArtNet");
}

int ArtNetPlugin::capabilities() const
{
    return QLCIOPlugin::Output | QLCIOPlugin::Input;
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
    foreach (ArtNetIO line, m_IOmapping)
    {
        if(line.type != ArtNetController::Input)
            list << QString(tr("%1: [%2] Universe: %3")).arg(j + 1).arg(line.IPAddress).arg(line.port);
        j++;
    }
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
    if (ctrl == NULL || ctrl->getType() == ArtNetController::Input)
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

void ArtNetPlugin::openOutput(quint32 output)
{
    int i = 0;
    if (output >= (quint32)m_IOmapping.length())
        return;

    qDebug() << "Open output with address :" << m_IOmapping.at(output).IPAddress;

    // scan for an already opened ArtNetController over the same network
    for (i = 0; i < m_IOmapping.length(); i++)
    {
        if ((quint32)i != output && m_IOmapping.at(i).controller != NULL)
        {
            ArtNetController *controller = m_IOmapping.at(i).controller;
            if (controller->getNetworkIP() == m_IOmapping.at(output).IPAddress)
            {
                m_IOmapping[output].controller = controller;
                controller->addUniverse(output, m_IOmapping.at(output).port);
                return;
            }
        }
    }

    // not found ? Create a new ArtNetController
    if (i == m_IOmapping.length())
    {
        ArtNetController *controller = new ArtNetController(m_IOmapping.at(output).IPAddress,
                                                            m_netInterfaces, m_netMACAddresses,
                                                            ArtNetController::Output, this);
        controller->addUniverse(output, m_IOmapping.at(output).port);
        m_IOmapping[output].controller = controller;
        // mark all the outputs with this IP address as output lines
        /*
        for (i = 0; i < m_IOmapping.length(); i++)
        {
            if (m_IOmapping.at(i).IPAddress == m_IOmapping.at(output).IPAddress)
                m_IOmapping[i].type = ArtNetController::Output;
        }
        */
    }
}

void ArtNetPlugin::closeOutput(quint32 output)
{
    if (output >= (quint32)m_IOmapping.length())
        return;
    ArtNetController *controller = m_IOmapping.at(output).controller;
    if (controller != NULL)
    {
        // if a ArtNetController is managing more than one universe
        // then just remove an output interface
        if (controller->getUniversesNumber() > 1)
        {
            controller->removeUniverse(m_IOmapping.at(output).port);
            m_IOmapping[output].controller = NULL;
        }
        else // otherwise destroy it
        {
            // reset all the outputs with this IP address as Unknown (Input/Output allowed)
            for (int i = 0; i < m_IOmapping.length(); i++)
            {
                if (m_IOmapping.at(i).IPAddress == m_IOmapping.at(output).IPAddress)
                    m_IOmapping[i].type = ArtNetController::Unknown;
            }
            delete m_IOmapping[output].controller;
            m_IOmapping[output].controller = NULL;
        }
    }
}

void ArtNetPlugin::writeUniverse(quint32 output, const QByteArray& universe)
{
    ArtNetController *controller = m_IOmapping[output].controller;
    if (controller != NULL)
        controller->sendDmx(m_IOmapping.at(output).port, universe);
}

/*************************************************************************
  * Inputs
  *************************************************************************/  
QStringList ArtNetPlugin::inputs()
{
    QStringList list;
    int j = 0;
    foreach (ArtNetIO line, m_IOmapping)
    {
        if(line.type != ArtNetController::Output)
            list << QString(tr("%1: [%2] Universe: %3")).arg(j + 1).arg(line.IPAddress).arg(line.port);
        j++;
    }
    return list;
}

void ArtNetPlugin::openInput(quint32 input)
{
    int i = 0;
    if (input >= (quint32)m_IOmapping.length())
        return;

    qDebug() << "Open input with address :" << m_IOmapping.at(input).IPAddress;

    // scan for an already opened ArtNetController over the same network
    for (i = 0; i < m_IOmapping.length(); i++)
    {
        if ((quint32)i != input && m_IOmapping.at(i).controller != NULL)
        {
            ArtNetController *controller = m_IOmapping.at(i).controller;
            if (controller->getNetworkIP() == m_IOmapping.at(input).IPAddress)
            {
                m_IOmapping[input].controller = controller;
                controller->addUniverse(input, m_IOmapping.at(input).port);
                return;
            }
        }
    }

    // not found ? Create a new ArtNetController
    if (i == m_IOmapping.length())
    {
        ArtNetController *controller = new ArtNetController(m_IOmapping.at(input).IPAddress,
                                                            m_netInterfaces, m_netMACAddresses,
                                                            ArtNetController::Input, this);
        controller->addUniverse(input, m_IOmapping.at(input).port);
        connect(controller, SIGNAL(valueChanged(quint32,int,uchar)),
                this, SLOT(slotInputValueChanged(quint32,int,uchar)));
        m_IOmapping[input].controller = controller;
        // mark all the inputs with this IP address as input lines
        /*
        for (i = 0; i < m_IOmapping.length(); i++)
        {
            if (m_IOmapping.at(i).IPAddress == m_IOmapping.at(input).IPAddress)
                m_IOmapping[i].type = ArtNetController::Input;
        }
        */
    }

}

void ArtNetPlugin::closeInput(quint32 input)
{
    if (input >= (quint32)m_IOmapping.length())
        return;
    ArtNetController *controller = m_IOmapping.at(input).controller;
    if (controller != NULL)
    {
        // if a ArtNetController is managing more than one universe
        // then just remove an output interface
        if (controller->getUniversesNumber() > 1)
        {
            controller->removeUniverse(m_IOmapping.at(input).port);
            m_IOmapping[input].controller = NULL;
        }
        else // otherwise destroy it
        {
            // reset all the outputs with this IP address as Unknown (Input/Output allowed)
            for (int i = 0; i < m_IOmapping.length(); i++)
            {
                if (m_IOmapping.at(i).IPAddress == m_IOmapping.at(input).IPAddress)
                    m_IOmapping[i].type = ArtNetController::Unknown;
            }
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
    if (ctrl == NULL || ctrl->getType() == ArtNetController::Output)
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

void ArtNetPlugin::slotInputValueChanged(quint32 input, int channel, uchar value)
{
    qDebug() << "Sending input:" << input << ", channel:" << channel << ", value:" << value;
    emit valueChanged(input, (quint32)channel, value);
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

void ArtNetPlugin::remapOutputs(QList<QString> IPs, QList<int> ports)
{
    if (IPs.length() > 0 && ports.length() > 0)
    {
        int oldIdx = 0;
        QList<ArtNetIO> newIOMapping;
        for (int i = 0; i < IPs.length(); i++)
        {
            ArtNetIO tmpIO;
            tmpIO.IPAddress = IPs.at(i);
            tmpIO.port = ports.at(i);
            if (oldIdx < m_IOmapping.length() &&
                m_IOmapping.at(oldIdx).IPAddress == IPs.at(i) &&
                m_IOmapping.at(oldIdx).port == ports.at(i))
            {
                tmpIO.controller = m_IOmapping.at(oldIdx).controller;
                if (tmpIO.controller != NULL)
                    tmpIO.type = (ArtNetController::Type)tmpIO.controller->getType();
                else
                    tmpIO.type = ArtNetController::Unknown;
                oldIdx++;
            }
            else
            {
                tmpIO.controller = NULL;
                tmpIO.type = ArtNetController::Unknown;
            }
            newIOMapping.append(tmpIO);
        }
        m_IOmapping.clear();
        m_IOmapping = newIOMapping;

        QSettings settings;
        // reset the previous state first
        settings.remove("ArtNetPlugin");
        QString countKey = QString("ArtNetPlugin/outputs");
        settings.setValue(countKey, QVariant(m_IOmapping.length()));

        for (int i = 0; i < m_IOmapping.length(); i++)
        {
            QString key = QString("ArtNetPlugin/Output%1").arg(i);
            QString value = m_IOmapping.at(i).IPAddress + "#" + QString("%1").arg(m_IOmapping.at(i).port);
            settings.setValue(key, QVariant(value));
        }

        emit configurationChanged();
    }
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(artnetplugin, ArtNetPlugin)
#endif
