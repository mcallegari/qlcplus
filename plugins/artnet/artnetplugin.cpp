/*
  Q Light Controller
  artnetplugin.cpp

  Copyright (c) Massimo Callegari

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
                    m_outputIPlist.append(outMapList.at(0));
                    m_outputPortList.append(outMapList.at(1).toInt());
                    m_controllersList.append(NULL);
                }
            }
        }
    }
    else // default mapping: port 0 for each IP found
    {
        foreach (QNetworkAddressEntry entry, m_netInterfaces)
        {
            m_outputIPlist.append(entry.ip().toString());
            m_outputPortList.append(0);
            m_controllersList.append(NULL);
        }
    }
}

QString ArtNetPlugin::name()
{
    return QString("ArtNet");
}

int ArtNetPlugin::capabilities() const
{
    return QLCIOPlugin::Output;
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
    for (int i = 0; i < m_outputIPlist.length(); i++)
    {
        list << QString(tr("%1: [%2] Output: %3")).arg(i + 1).arg(m_outputIPlist.at(i)).arg(m_outputPortList.at(i) + 1);
    }
    return list;
}

QString ArtNetPlugin::outputInfo(quint32 output)
{
    Q_UNUSED(output);
    return QString();
}

void ArtNetPlugin::openOutput(quint32 output)
{
    int i = 0;
    if (output >= (quint32)m_outputIPlist.length())
        return;

    qDebug() << "Open output with address :" << m_outputIPlist.at(output);

    // scan for an already open ArtNetController over the same network
    for (i = 0; i < m_controllersList.length(); i++)
    {
        if ((quint32)i != output && m_controllersList.at(i) != NULL)
        {
            ArtNetController *controller = m_controllersList.at(i);
            if (controller->getNetworkIP() == m_outputIPlist.at(output))
            {
                m_controllersList.replace(output, controller);
                controller->addUniverse(m_outputPortList.at(output));
                return;
            }
        }
    }

    // not found ? Create a new ArtNetController
    if (i == m_controllersList.length())
    {
        ArtNetController *controller = new ArtNetController(m_outputIPlist.at(output), m_outputPortList.at(output), m_netInterfaces, this);
        m_controllersList.replace(output, controller);
    }
}

void ArtNetPlugin::closeOutput(quint32 output)
{
    if (output >= (quint32)m_outputIPlist.length())
        return;
    ArtNetController *controller = m_controllersList[output];
    if (controller != NULL)
    {
        // if a ArtNetController is managing more than one universe
        // then just remove an output interface
        if (controller->getUniversesNumber() > 1)
        {
            controller->removeUniverse(m_outputPortList.at(output));
            m_controllersList[output] = NULL;
        }
        else // otherwiase destroy it
        {
            delete m_controllersList[output];
            m_controllersList[output] = NULL;
        }
    }
}

void ArtNetPlugin::writeUniverse(quint32 output, const QByteArray& universe)
{
    ArtNetController *controller = m_controllersList[output];
    if (controller != NULL)
        controller->sendDmx(m_outputPortList.at(output), universe);
}

/*************************************************************************
  * Inputs
  *************************************************************************/  
void ArtNetPlugin::openInput(quint32 input)
{
    Q_UNUSED(input);
}

void ArtNetPlugin::closeInput(quint32 input)
{
    Q_UNUSED(input);
}

QStringList ArtNetPlugin::inputs()
{
    return QStringList();
}

QString ArtNetPlugin::inputInfo(quint32 input)
{
    Q_UNUSED(input);
    return QString();
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

QList<QString> ArtNetPlugin::mappedOutputs()
{
    return m_outputIPlist;
}

QList<int> ArtNetPlugin::mappedPorts()
{
    return m_outputPortList;
}

QList<ArtNetController*> ArtNetPlugin::mappedControllers()
{
    return m_controllersList;
}

void ArtNetPlugin::remapOutputs(QList<QString> IPs, QList<int> ports)
{
    if (IPs.length() > 0 && ports.length() > 0)
    {
        m_outputIPlist.clear();
        m_outputPortList.clear();
        m_outputIPlist = IPs;
        m_outputPortList = ports;

        QSettings settings;
        QString countKey = QString("ArtNetPlugin/outputs");
        settings.setValue(countKey, QVariant(m_outputIPlist.length()));

        for (int i = 0; i < m_outputIPlist.length(); i++)
        {
            QString key = QString("ArtNetPlugin/Output%1").arg(i);
            QString value = m_outputIPlist.at(i) + "#" + QString("%1").arg(m_outputPortList.at(i));
            settings.setValue(key, QVariant(value));
        }

        emit configurationChanged();
    }
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(artnetplugin, ArtNetPlugin)
