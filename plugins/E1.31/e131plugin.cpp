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
    foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
    {
        foreach (QNetworkAddressEntry entry, interface.addressEntries())
        {
            QHostAddress addr = entry.ip();
            if (addr.protocol() != QAbstractSocket::IPv6Protocol && addr != QHostAddress::LocalHost)
            {
                E131IO tmpIO;
                tmpIO.IPAddress = entry.ip().toString();
                tmpIO.MACAddress = interface.hardwareAddress();
                tmpIO.controller = NULL;
                m_IOmapping.append(tmpIO);

                m_netInterfaces.append(entry);
            }
        }
    }
}

QString E131Plugin::name()
{
    return QString("E1.31");
}

int E131Plugin::capabilities() const
{
    return QLCIOPlugin::Output | QLCIOPlugin::Input | QLCIOPlugin::Infinite;
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
        list << QString(tr("%1: %2")).arg(j + 1).arg(line.IPAddress);
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
    if (ctrl == NULL || ctrl->type() == E131Controller::Input)
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

bool E131Plugin::openOutput(quint32 output)
{
    if (m_IOmapping.count() == 0)
        init();

    if (output >= (quint32)m_IOmapping.length())
        return false;

    qDebug() << "Open output with address :" << m_IOmapping.at(output).IPAddress;

    // already open ? Just add the type flag
    if (m_IOmapping[output].controller != NULL)
    {
        m_IOmapping[output].controller->setType(
                    (E131Controller::Type)(m_IOmapping[output].controller->type() | E131Controller::Output));
        m_IOmapping[output].controller->changeReferenceCount(E131Controller::Output, +1);
        return true;
    }

    // not open ? Create a new E131Controller
    E131Controller *controller = new E131Controller(m_IOmapping.at(output).IPAddress,
                                                    m_IOmapping.at(output).MACAddress,
                                                    E131Controller::Output, output, this);
    m_IOmapping[output].controller = controller;

    return true;
}

void E131Plugin::closeOutput(quint32 output)
{
    if (output >= (quint32)m_IOmapping.length())
        return;
    E131Controller *controller = m_IOmapping.at(output).controller;
    if (controller != NULL)
    {
        controller->changeReferenceCount(E131Controller::Output, -1);
        // if a E131Controller is also open as input
        // then just remove the output capability
        if (controller->type() & E131Controller::Input)
        {
            controller->setType(E131Controller::Input);
        }
        if (controller->referenceCount(E131Controller::Input) == 0 &&
            controller->referenceCount(E131Controller::Output) == 0)
        {
            delete m_IOmapping[output].controller;
            m_IOmapping[output].controller = NULL;
        }
    }
}

void E131Plugin::writeUniverse(quint32 universe, quint32 output, const QByteArray &data)
{
    if (output >= (quint32)m_IOmapping.count())
        return;

    E131Controller *controller = m_IOmapping[output].controller;
    if (controller != NULL)
        controller->sendDmx(universe, data);
}

/*************************************************************************
  * Inputs
  *************************************************************************/  
QStringList E131Plugin::inputs()
{
    QStringList list;
    int j = 0;

    if (m_IOmapping.count() == 0)
        init();

    foreach (E131IO line, m_IOmapping)
    {
        list << QString(tr("%1: %2")).arg(j + 1).arg(line.IPAddress);
        j++;
    }
    return list;
}

bool E131Plugin::openInput(quint32 input)
{
    if (m_IOmapping.count() == 0)
        init();

    if (input >= (quint32)m_IOmapping.length())
        return false;

    qDebug() << "Open input with address :" << m_IOmapping.at(input).IPAddress;

    // already open ? Just add the type flag
    if (m_IOmapping[input].controller != NULL)
    {
        m_IOmapping[input].controller->setType(
                    (E131Controller::Type)(m_IOmapping[input].controller->type() | E131Controller::Input));
        m_IOmapping[input].controller->changeReferenceCount(E131Controller::Input, +1);
        return true;
    }

    // not open ? Create a new ArtNetController
    E131Controller *controller = new E131Controller(m_IOmapping.at(input).IPAddress,
                                                    m_IOmapping.at(input).MACAddress,
                                                    E131Controller::Input, input, this);
    connect(controller, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)),
            this, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)));
    m_IOmapping[input].controller = controller;

    return true;
}

void E131Plugin::closeInput(quint32 input)
{
    if (input >= (quint32)m_IOmapping.length())
        return;
    E131Controller *controller = m_IOmapping.at(input).controller;
    if (controller != NULL)
    {
        controller->changeReferenceCount(E131Controller::Input, -1);
        // if a E131Controller is also open as output
        // then just remove the input capability
        if (controller->type() & E131Controller::Output)
        {
            controller->setType(E131Controller::Output);
        }
        if (controller->referenceCount(E131Controller::Input) == 0 &&
            controller->referenceCount(E131Controller::Output) == 0)
        {
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
    if (ctrl == NULL || ctrl->type() == E131Controller::Output)
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
void E131Plugin::configure()
{
    //ConfigureE131 conf(this);
    //conf.exec();
}

bool E131Plugin::canConfigure()
{
    return false;
}

QList<QNetworkAddressEntry> E131Plugin::interfaces()
{
    return m_netInterfaces;
}

QList<E131IO> E131Plugin::getIOMapping()
{
    return m_IOmapping;
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(e131plugin, E131Plugin)
#endif
