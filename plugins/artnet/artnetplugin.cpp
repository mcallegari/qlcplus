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
                    m_IPAddressMap.append(outMapList.at(0));
                    m_IOPortMap.append(outMapList.at(1).toInt());
                    m_controllersList.append(NULL);
                }
            }
        }
    }
    else // default mapping: port 0 for each IP found
    {
        foreach (QNetworkAddressEntry entry, m_netInterfaces)
        {
            m_IPAddressMap.append(entry.ip().toString());
            m_IOPortMap.append(0);
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
    QStringList busyAddress;
    // first find out if there are Controllers used as input
    // and mark their IP address as busy
    for (int i = 0; i < m_IPAddressMap.length(); i++)
    {
        ArtNetController *contr = m_controllersList.at(i);
        if (contr != NULL && contr->getType() == ArtNetController::Input &&
            busyAddress.contains(m_IPAddressMap.at(i)) == false)
                busyAddress.append(m_IPAddressMap.at(i));
    }

    for (int j = 0; j < m_IPAddressMap.length(); j++)
    {
        // if the controller is not marked as busy then it's a valid line
        if (busyAddress.contains(m_IPAddressMap.at(j)) == false)
            list << QString(tr("%1: [%2] Universe: %3")).arg(j + 1).arg(m_IPAddressMap.at(j)).arg(m_IOPortMap.at(j) + 1);
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
    if (output >= (quint32)m_IPAddressMap.length())
        return;

    qDebug() << "Open output with address :" << m_IPAddressMap.at(output);

    // scan for an already open ArtNetController over the same network
    for (i = 0; i < m_controllersList.length(); i++)
    {
        if ((quint32)i != output && m_controllersList.at(i) != NULL)
        {
            ArtNetController *controller = m_controllersList.at(i);
            if (controller->getNetworkIP() == m_IPAddressMap.at(output))
            {
                m_controllersList.replace(output, controller);
                controller->addUniverse(output, m_IOPortMap.at(output));
                return;
            }
        }
    }

    // not found ? Create a new ArtNetController
    if (i == m_controllersList.length())
    {
        ArtNetController *controller = new ArtNetController(m_IPAddressMap.at(output),
                                                            m_netInterfaces, m_netMACAddresses,
                                                            ArtNetController::Output, this);
        controller->addUniverse(output, m_IOPortMap.at(output));
        m_controllersList.replace(output, controller);
    }
    //emit configurationChanged(); // why this doesn't work ??
}

void ArtNetPlugin::closeOutput(quint32 output)
{
    if (output >= (quint32)m_IPAddressMap.length())
        return;
    ArtNetController *controller = m_controllersList[output];
    if (controller != NULL)
    {
        // if a ArtNetController is managing more than one universe
        // then just remove an output interface
        if (controller->getUniversesNumber() > 1)
        {
            controller->removeUniverse(m_IOPortMap.at(output));
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
        controller->sendDmx(m_IOPortMap.at(output), universe);
}

/*************************************************************************
  * Inputs
  *************************************************************************/  
QStringList ArtNetPlugin::inputs()
{
    QStringList list;
    QStringList busyAddress;
    // first find out if there are Controllers used as output
    // and mark their IP address as busy
    for (int i = 0; i < m_IPAddressMap.length(); i++)
    {
        ArtNetController *contr = m_controllersList.at(i);
        if (contr != NULL && contr->getType() == ArtNetController::Output &&
            busyAddress.contains(m_IPAddressMap.at(i)) == false)
                busyAddress.append(m_IPAddressMap.at(i));
    }
    for (int i = 0; i < m_IPAddressMap.length(); i++)
    {
        // if the controller is not marked as busy then it's a valid line
        if (busyAddress.contains(m_IPAddressMap.at(i)) == false)
            list << QString(tr("%1: [%2] Universe: %3")).arg(i + 1).arg(m_IPAddressMap.at(i)).arg(m_IOPortMap.at(i) + 1);
    }
    return list;
}

void ArtNetPlugin::openInput(quint32 input)
{
    int i = 0;
    if (input >= (quint32)m_IPAddressMap.length())
        return;

    qDebug() << "Open input with address :" << m_IPAddressMap.at(input);

    // scan for an already open ArtNetController over the same network
    for (i = 0; i < m_controllersList.length(); i++)
    {
        if ((quint32)i != input && m_controllersList.at(i) != NULL)
        {
            ArtNetController *controller = m_controllersList.at(i);
            if (controller->getNetworkIP() == m_IPAddressMap.at(input))
            {
                m_controllersList.replace(input, controller);
                controller->addUniverse(input, m_IOPortMap.at(input));
                return;
            }
        }
    }

    // not found ? Create a new ArtNetController
    if (i == m_controllersList.length())
    {
        ArtNetController *controller = new ArtNetController(m_IPAddressMap.at(input),
                                                            m_netInterfaces, m_netMACAddresses,
                                                            ArtNetController::Input, this);
        controller->addUniverse(input, m_IOPortMap.at(input));
        connect(controller, SIGNAL(valueChanged(quint32,int,uchar)),
                this, SLOT(slotInputValueChanged(quint32,int,uchar)));
        m_controllersList.replace(input, controller);
    }
    //emit configurationChanged(); // why this doesn't work ??
}

void ArtNetPlugin::closeInput(quint32 input)
{
    if (input >= (quint32)m_IPAddressMap.length())
        return;
    ArtNetController *controller = m_controllersList[input];
    if (controller != NULL)
    {
        // if a ArtNetController is managing more than one universe
        // then just remove an output interface
        if (controller->getUniversesNumber() > 1)
        {
            controller->removeUniverse(m_IOPortMap.at(input));
            m_controllersList[input] = NULL;
        }
        else // otherwiase destroy it
        {
            delete m_controllersList[input];
            m_controllersList[input] = NULL;
        }
    }
}

QString ArtNetPlugin::inputInfo(quint32 input)
{
    Q_UNUSED(input);
    return QString();
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

QList<QString> ArtNetPlugin::mappedOutputs()
{
    return m_IPAddressMap;
}

QList<int> ArtNetPlugin::mappedPorts()
{
    return m_IOPortMap;
}

QList<ArtNetController*> ArtNetPlugin::mappedControllers()
{
    return m_controllersList;
}

void ArtNetPlugin::remapOutputs(QList<QString> IPs, QList<int> ports)
{
    if (IPs.length() > 0 && ports.length() > 0)
    {
        int oldIdx = 0;
        QList<ArtNetController*> newControllersList;
        for (int c = 0; c < IPs.length(); c++)
        {
            if (oldIdx < m_IPAddressMap.length() &&
                m_IPAddressMap.at(oldIdx) == IPs.at(c) &&
                m_IOPortMap.at(oldIdx) == ports.at(c))
            {
                newControllersList.append(m_controllersList.at(oldIdx));
                oldIdx++;
            }
            else
                newControllersList.append(NULL);
        }
        m_IPAddressMap.clear();
        m_IOPortMap.clear();
        m_controllersList.clear();
        m_IPAddressMap = IPs;
        m_IOPortMap = ports;
        m_controllersList = newControllersList;

        QSettings settings;
        // reset the previous state first
        settings.remove("ArtNetPlugin");
        QString countKey = QString("ArtNetPlugin/outputs");
        settings.setValue(countKey, QVariant(m_IPAddressMap.length()));

        for (int i = 0; i < m_IPAddressMap.length(); i++)
        {
            QString key = QString("ArtNetPlugin/Output%1").arg(i);
            QString value = m_IPAddressMap.at(i) + "#" + QString("%1").arg(m_IOPortMap.at(i));
            settings.setValue(key, QVariant(value));
        }

        emit configurationChanged();
    }
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(artnetplugin, ArtNetPlugin)
