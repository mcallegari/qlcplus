/*
  Q Light Controller Plus
  e131plugin.cpp

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

#include "e131plugin.h"
#include "configuree131.h"

#include <QSettings>
#include <QDebug>

E131Plugin::~E131Plugin()
{
}

void E131Plugin::init()
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
    QString key = QString("E131Plugin/outputs");
    QVariant outNum = settings.value(key);
    if (outNum.isValid() == true)
    {
        for (int o = 0; o < outNum.toInt(); o++)
        {
            QString outKey = QString("E131Plugin/Output%1").arg(o);
            QVariant value = settings.value(outKey);
            if (value.isValid() == true)
            {
                // values are stored as: IP#port
                QString outMapStr = value.toString();
                QStringList outMapList = outMapStr.split("#");
                if (outMapList.length() == 2)
                {
                    E131IO tmpIO;
                    tmpIO.IPAddress = outMapList.at(0);
                    tmpIO.port = outMapList.at(1).toInt();
                    tmpIO.controller = NULL;
                    tmpIO.type = E131Controller::Unknown;
                    m_IOmapping.append(tmpIO);
                }
            }
        }
    }
    else // default mapping: port 0 for each IP found
    {
        foreach (QNetworkAddressEntry entry, m_netInterfaces)
        {
            E131IO tmpIO;
            tmpIO.IPAddress = entry.ip().toString();
            tmpIO.port = 0;
            tmpIO.controller = NULL;
            tmpIO.type = E131Controller::Unknown;
            m_IOmapping.append(tmpIO);
        }
    }
}

QString E131Plugin::name()
{
    return QString("E1.31");
}

int E131Plugin::capabilities() const
{
    return QLCIOPlugin::Output | QLCIOPlugin::Input;
}

QString E131Plugin::pluginInfo()
{
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    str += QString("<P>");
    str += QString("<H3>%1</H3>").arg(name());
    str += tr("This plugin provides DMX output for devices supporting the E1.31 communication protocol.");
    str += QString("</P>");

    return str;
}

/*********************************************************************
 * Outputs
 *********************************************************************/
QStringList E131Plugin::outputs()
{
    QStringList list;
    int j = 0;
    foreach (E131IO line, m_IOmapping)
    {
        if(line.type != E131Controller::Input)
            list << QString(tr("%1: [%2] Universe: %3")).arg(j + 1).arg(line.IPAddress).arg(line.port);
        j++;
    }
    return list;
}

QString E131Plugin::outputInfo(quint32 output)
{
    if (output >= (quint32)m_IOmapping.length())
        return QString();

    QString str;

    str += QString("<H3>%1 %2</H3>").arg(tr("Output")).arg(outputs()[output]);
    str += QString("<P>");
    E131Controller *ctrl = m_IOmapping.at(output).controller;
    if (ctrl == NULL || ctrl->getType() == E131Controller::Input)
        str += tr("Status: Not open");
    else
    {
        str += tr("Status: Open");
        str += QString("<BR>");
        str += tr("Packets sent: ");
        str += QString("%1").arg(ctrl->getPacketSentNumber());
    }
    str += QString("</P>");
    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void E131Plugin::openOutput(quint32 output)
{
    int i = 0;
    if (output >= (quint32)m_IOmapping.length())
        return;

    qDebug() << "Open output with address :" << m_IOmapping.at(output).IPAddress;

    // scan for an already opened E131Controller over the same network
    for (i = 0; i < m_IOmapping.length(); i++)
    {
        if ((quint32)i != output && m_IOmapping.at(i).controller != NULL)
        {
            E131Controller *controller = m_IOmapping.at(i).controller;
            if (controller->getNetworkIP() == m_IOmapping.at(output).IPAddress)
            {
                m_IOmapping[output].controller = controller;
                controller->addUniverse(output, m_IOmapping.at(output).port);
                return;
            }
        }
    }

    // not found ? Create a new E131Controller
    if (i == m_IOmapping.length())
    {
        E131Controller *controller = new E131Controller(m_IOmapping.at(output).IPAddress,
                                                            m_netInterfaces, m_netMACAddresses,
                                                            E131Controller::Output, this);
        controller->addUniverse(output, m_IOmapping.at(output).port);
        m_IOmapping[output].controller = controller;
    }
}

void E131Plugin::closeOutput(quint32 output)
{
    if (output >= (quint32)m_IOmapping.length())
        return;
    E131Controller *controller = m_IOmapping.at(output).controller;
    if (controller != NULL)
    {
        // if a E131Controller is managing more than one universe
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
                    m_IOmapping[i].type = E131Controller::Unknown;
            }
            delete m_IOmapping[output].controller;
            m_IOmapping[output].controller = NULL;
        }
    }
}

void E131Plugin::writeUniverse(quint32 output, const QByteArray& universe)
{
    E131Controller *controller = m_IOmapping[output].controller;
    if (controller != NULL)
        controller->sendDmx(m_IOmapping.at(output).port, universe);
}

/*************************************************************************
  * Inputs
  *************************************************************************/  
QStringList E131Plugin::inputs()
{
    QStringList list;
    int j = 0;
    foreach (E131IO line, m_IOmapping)
    {
        if(line.type != E131Controller::Output)
            list << QString(tr("%1: [%2] Universe: %3")).arg(j + 1).arg(line.IPAddress).arg(line.port);
        j++;
    }
    return list;
}

void E131Plugin::openInput(quint32 input)
{
    int i = 0;
    if (input >= (quint32)m_IOmapping.length())
        return;

    qDebug() << "Open input with address :" << m_IOmapping.at(input).IPAddress;

    // scan for an already opened E131Controller over the same network
    for (i = 0; i < m_IOmapping.length(); i++)
    {
        if ((quint32)i != input && m_IOmapping.at(i).controller != NULL)
        {
            E131Controller *controller = m_IOmapping.at(i).controller;
            if (controller->getNetworkIP() == m_IOmapping.at(input).IPAddress)
            {
                m_IOmapping[input].controller = controller;
                controller->addUniverse(input, m_IOmapping.at(input).port);
                return;
            }
        }
    }

    // not found ? Create a new E131Controller
    if (i == m_IOmapping.length())
    {
        E131Controller *controller = new E131Controller(m_IOmapping.at(input).IPAddress,
                                                            m_netInterfaces, m_netMACAddresses,
                                                            E131Controller::Input, this);
        controller->addUniverse(input, m_IOmapping.at(input).port);
        connect(controller, SIGNAL(valueChanged(quint32,int,uchar)),
                this, SLOT(slotInputValueChanged(quint32,int,uchar)));
        m_IOmapping[input].controller = controller;
    }

}

void E131Plugin::closeInput(quint32 input)
{
    if (input >= (quint32)m_IOmapping.length())
        return;
    E131Controller *controller = m_IOmapping.at(input).controller;
    if (controller != NULL)
    {
        // if a E131Controller is managing more than one universe
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
                    m_IOmapping[i].type = E131Controller::Unknown;
            }
            delete m_IOmapping[input].controller;
            m_IOmapping[input].controller = NULL;
        }
    }
}

QString E131Plugin::inputInfo(quint32 input)
{
    if (input >= (quint32)m_IOmapping.length())
        return QString();

    QString str;

    str += QString("<H3>%1 %2</H3>").arg(tr("Input")).arg(inputs()[input]);
    str += QString("<P>");
    E131Controller *ctrl = m_IOmapping.at(input).controller;
    if (ctrl == NULL || ctrl->getType() == E131Controller::Output)
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

void E131Plugin::slotInputValueChanged(quint32 input, int channel, uchar value)
{
    qDebug() << "Sending input:" << input << ", channel:" << channel << ", value:" << value;
    emit valueChanged(input, (quint32)channel, value);
}

/*********************************************************************
 * Configuration
 *********************************************************************/
void E131Plugin::configure()
{
    ConfigureE131 conf(this);
    conf.exec();
}

bool E131Plugin::canConfigure()
{
    return true;
}

QList<QNetworkAddressEntry> E131Plugin::interfaces()
{
    return m_netInterfaces;
}

QList<E131IO> E131Plugin::getIOMapping()
{
    return m_IOmapping;
}

void E131Plugin::remapOutputs(QList<QString> IPs, QList<int> ports)
{
    if (IPs.length() > 0 && ports.length() > 0)
    {
        int oldIdx = 0;
        QList<E131IO> newIOMapping;
        for (int i = 0; i < IPs.length(); i++)
        {
            E131IO tmpIO;
            tmpIO.IPAddress = IPs.at(i);
            tmpIO.port = ports.at(i);
            if (oldIdx < m_IOmapping.length() &&
                m_IOmapping.at(oldIdx).IPAddress == IPs.at(i) &&
                m_IOmapping.at(oldIdx).port == ports.at(i))
            {
                tmpIO.controller = m_IOmapping.at(oldIdx).controller;
                if (tmpIO.controller != NULL)
                    tmpIO.type = (E131Controller::Type)tmpIO.controller->getType();
                else
                    tmpIO.type = E131Controller::Unknown;
                oldIdx++;
            }
            else
            {
                tmpIO.controller = NULL;
                tmpIO.type = E131Controller::Unknown;
            }
            newIOMapping.append(tmpIO);
        }
        m_IOmapping.clear();
        m_IOmapping = newIOMapping;

        QSettings settings;
        // reset the previous state first
        settings.remove("E131Plugin");
        QString countKey = QString("E131Plugin/outputs");
        settings.setValue(countKey, QVariant(m_IOmapping.length()));

        for (int i = 0; i < m_IOmapping.length(); i++)
        {
            QString key = QString("E131Plugin/Output%1").arg(i);
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
Q_EXPORT_PLUGIN2(e131plugin, E131Plugin)
#endif
