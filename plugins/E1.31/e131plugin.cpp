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

#include <QSettings>
#include <QDebug>

#include "e131plugin.h"
#include "configuree131.h"

bool addressCompare(const E131IO &v1, const E131IO &v2)
{
    return v1.address.ip().toString() < v2.address.ip().toString();
}

E131Plugin::~E131Plugin()
{
}

void E131Plugin::init()
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
                E131IO tmpIO;
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

bool E131Plugin::requestLine(quint32 line)
{
    int retryCount = 0;

    while (line >= (quint32)m_IOmapping.length())
    {
        qDebug() << "[E1.31] cannot open line" << line << "(available:" << m_IOmapping.length() << ")";
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
QStringList E131Plugin::outputs()
{
    QStringList list;

    init();

    foreach (E131IO line, m_IOmapping)
        list << line.address.ip().toString();

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

bool E131Plugin::openOutput(quint32 output, quint32 universe)
{
    if (requestLine(output) == false)
        return false;

    qDebug() << "[E1.31] Open output with address :" << m_IOmapping.at(output).address.ip().toString();

    // if the controller doesn't exist, create it
    if (m_IOmapping[output].controller == NULL)
    {
        E131Controller *controller = new E131Controller(m_IOmapping.at(output).iface,
                                                        m_IOmapping.at(output).address,
                                                        output, this);
        connect(controller, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)),
                this, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)));
        m_IOmapping[output].controller = controller;
    }

    m_IOmapping[output].controller->addUniverse(universe, E131Controller::Output);
    addToMap(universe, output, Output);

    return true;
}

void E131Plugin::closeOutput(quint32 output, quint32 universe)
{
    if (output >= (quint32)m_IOmapping.length())
        return;

    removeFromMap(output, universe, Output);
    E131Controller *controller = m_IOmapping.at(output).controller;
    if (controller != NULL)
    {
        controller->removeUniverse(universe, E131Controller::Output);
        if (controller->universesList().count() == 0)
        {
            delete m_IOmapping[output].controller;
            m_IOmapping[output].controller = NULL;
        }
    }
}

void E131Plugin::writeUniverse(quint32 universe, quint32 output, const QByteArray &data, bool dataChanged)
{
    Q_UNUSED(dataChanged)

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

    init();

    foreach (E131IO line, m_IOmapping)
        list << line.address.ip().toString();

    return list;
}

bool E131Plugin::openInput(quint32 input, quint32 universe)
{
    if (requestLine(input) == false)
        return false;

    qDebug() << "[E1.31] Open input with address :" << m_IOmapping.at(input).address.ip().toString();

    // if the controller doesn't exist, create it
    if (m_IOmapping[input].controller == NULL)
    {
        E131Controller *controller = new E131Controller(m_IOmapping.at(input).iface,
                                                        m_IOmapping.at(input).address,
                                                        input, this);
        connect(controller, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)),
                this, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)));
        m_IOmapping[input].controller = controller;
    }

    m_IOmapping[input].controller->addUniverse(universe, E131Controller::Input);
    addToMap(universe, input, Input);

    return true;
}

void E131Plugin::closeInput(quint32 input, quint32 universe)
{
    if (input >= (quint32)m_IOmapping.length())
        return;

    removeFromMap(input, universe, Input);
    E131Controller *controller = m_IOmapping.at(input).controller;
    if (controller != NULL)
    {
        controller->removeUniverse(universe, E131Controller::Input);
        if (controller->universesList().count() == 0)
        {
            delete m_IOmapping[input].controller;
            m_IOmapping[input].controller = NULL;
        }
    }
}

QString E131Plugin::inputInfo(quint32 input)
{
    init();

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
    ConfigureE131 conf(this);
    conf.exec();
}

bool E131Plugin::canConfigure()
{
    return true;
}

void E131Plugin::setParameter(quint32 universe, quint32 line, Capability type,
                              QString name, QVariant value)
{
    if (line >= (quint32)m_IOmapping.length())
        return;

    E131Controller *controller = m_IOmapping.at(line).controller;
    if (controller == NULL)
        return;

    if (type == Input)
    {
        if (name == E131_MULTICAST)
            controller->setInputMulticast(universe, value.toInt());
        else if (name == E131_MCASTIP)
            controller->setInputMCastAddress(universe, value.toString(), true);
        else if (name == E131_MCASTFULLIP)
            controller->setInputMCastAddress(universe, value.toString(), false);
        else if (name == E131_UCASTPORT)
            controller->setInputUCastPort(universe, value.toUInt());
        else if (name == E131_UNIVERSE)
            controller->setInputUniverse(universe, value.toUInt());
        else
        {
            qWarning() << Q_FUNC_INFO << name << "is not a valid E1.31 input parameter";
            return;
        }
    }
    else // if (type == Output)
    {
        if (name == E131_MULTICAST)
            controller->setOutputMulticast(universe, value.toInt());
        else if (name == E131_MCASTIP)
            controller->setOutputMCastAddress(universe, value.toString(), true);
        else if (name == E131_MCASTFULLIP)
            controller->setOutputMCastAddress(universe, value.toString(), false);
        else if (name == E131_UCASTIP)
            controller->setOutputUCastAddress(universe, value.toString());
        else if (name == E131_UCASTPORT)
            controller->setOutputUCastPort(universe, value.toUInt());
        else if (name == E131_UNIVERSE)
            controller->setOutputUniverse(universe, value.toUInt());
        else if (name == E131_TRANSMITMODE)
            controller->setOutputTransmissionMode(universe, E131Controller::stringToTransmissionMode(value.toString()));
        else if (name == E131_PRIORITY)
            controller->setOutputPriority(universe, value.toUInt());
        else
            qWarning() << Q_FUNC_INFO << name << "is not a valid E1.31 output parameter";
    }

    QLCIOPlugin::setParameter(universe, line, type, name, value);
}

QList<E131IO> E131Plugin::getIOMapping()
{
    return m_IOmapping;
}
