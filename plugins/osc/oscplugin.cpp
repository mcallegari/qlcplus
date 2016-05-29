/*
  Q Light Controller Plus
  oscplugin.cpp

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

#include "oscplugin.h"
#include "configureosc.h"

#include <QSettings>
#include <QDebug>

#define MAX_INIT_RETRY  10

OSCPlugin::~OSCPlugin()
{
}

void OSCPlugin::init()
{
    foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
    {
        foreach (QNetworkAddressEntry entry, interface.addressEntries())
        {
            QHostAddress addr = entry.ip();
            if (addr.protocol() != QAbstractSocket::IPv6Protocol)
            {
                OSCIO tmpIO;
                tmpIO.IPAddress = entry.ip().toString();
                tmpIO.controller = NULL;

                bool alreadyInList = false;
                for(int j = 0; j < m_IOmapping.count(); j++)
                {
                    if (m_IOmapping.at(j).IPAddress == tmpIO.IPAddress)
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
}

QString OSCPlugin::name()
{
    return QString("OSC");
}

int OSCPlugin::capabilities() const
{
    return QLCIOPlugin::Output | QLCIOPlugin::Input | QLCIOPlugin::Feedback | QLCIOPlugin::Infinite;
}

QString OSCPlugin::pluginInfo()
{
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    str += QString("<P>");
    str += QString("<H3>%1</H3>").arg(name());
    str += tr("This plugin provides input for devices supporting the OSC transmission protocol.");
    str += QString("</P>");

    return str;
}

bool OSCPlugin::requestLine(quint32 line, int retries)
{
    int retryCount = 0;

    while (line >= (quint32)m_IOmapping.length())
    {
        qDebug() << "[OSC] cannot open line" << line << "(available:" << m_IOmapping.length() << ")";
        Sleep(1000);
        init();
        if (retryCount++ == retries)
            return false;
    }

    return true;
}

/*********************************************************************
 * Outputs
 *********************************************************************/
QStringList OSCPlugin::outputs()
{
    QStringList list;
    int j = 0;

    init();

    foreach (OSCIO line, m_IOmapping)
    {
        list << QString("%1: %2").arg(j + 1).arg(line.IPAddress);
        j++;
    }
    return list;
}

QString OSCPlugin::outputInfo(quint32 output)
{
    if (output >= (quint32)m_IOmapping.length())
        return QString();

    QString str;

    str += QString("<H3>%1 %2</H3>").arg(tr("Output")).arg(outputs()[output]);
    str += QString("<P>");
    OSCController *ctrl = m_IOmapping.at(output).controller;
    if (ctrl == NULL || ctrl->type() == OSCController::Input)
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

bool OSCPlugin::openOutput(quint32 output, quint32 universe)
{
    if (requestLine(output, MAX_INIT_RETRY) == false)
        return false;

    qDebug() << "[OSC] Open output with address :" << m_IOmapping.at(output).IPAddress;

    // if the controller doesn't exist, create it
    if (m_IOmapping[output].controller == NULL)
    {
        OSCController *controller = new OSCController(m_IOmapping.at(output).IPAddress,
                                                        OSCController::Output, output, this);
        m_IOmapping[output].controller = controller;
    }

    m_IOmapping[output].controller->addUniverse(universe, OSCController::Output);
    addToMap(universe, output, Output);

    return true;
}

void OSCPlugin::closeOutput(quint32 output, quint32 universe)
{
    if (output >= (quint32)m_IOmapping.length())
        return;

    removeFromMap(output, universe, Output);
    OSCController *controller = m_IOmapping.at(output).controller;
    if (controller != NULL)
    {
        controller->removeUniverse(universe, OSCController::Output);
        if (controller->universesList().count() == 0)
        {
            delete m_IOmapping[output].controller;
            m_IOmapping[output].controller = NULL;
        }
    }
}

void OSCPlugin::writeUniverse(quint32 universe, quint32 output, const QByteArray &data)
{
    if (output >= (quint32)m_IOmapping.count())
        return;

    OSCController *controller = m_IOmapping[output].controller;
    if (controller != NULL)
        controller->sendDmx(universe, data);
}

/*************************************************************************
  * Inputs
  *************************************************************************/  
QStringList OSCPlugin::inputs()
{
    QStringList list;
    int j = 0;

    init();

    foreach (OSCIO line, m_IOmapping)
    {
        list << QString("%1: %2").arg(j + 1).arg(line.IPAddress);
        j++;
    }
    return list;
}

bool OSCPlugin::openInput(quint32 input, quint32 universe)
{
    if (requestLine(input, MAX_INIT_RETRY) == false)
        return false;

    qDebug() << "[OSC] Open input on address :" << m_IOmapping.at(input).IPAddress;

    // if the controller doesn't exist, create it
    if (m_IOmapping[input].controller == NULL)
    {
        OSCController *controller = new OSCController(m_IOmapping.at(input).IPAddress,
                                                        OSCController::Input, input, this);
        connect(controller, SIGNAL(valueChanged(quint32,quint32,quint32,uchar,QString)),
                this, SIGNAL(valueChanged(quint32,quint32,quint32,uchar,QString)));
        m_IOmapping[input].controller = controller;
    }

    m_IOmapping[input].controller->addUniverse(universe, OSCController::Input);
    addToMap(universe, input, Input);

    return true;
}

void OSCPlugin::closeInput(quint32 input, quint32 universe)
{
    if (input >= (quint32)m_IOmapping.length())
        return;

    removeFromMap(input, universe, Input);
    OSCController *controller = m_IOmapping.at(input).controller;
    if (controller != NULL)
    {
        controller->removeUniverse(universe, OSCController::Input);
        if (controller->universesList().count() == 0)
        {
            delete m_IOmapping[input].controller;
            m_IOmapping[input].controller = NULL;
        }
    }
}

QString OSCPlugin::inputInfo(quint32 input)
{
    if (input >= (quint32)m_IOmapping.length())
        return QString();

    QString str;

    str += QString("<H3>%1 %2</H3>").arg(tr("Input")).arg(inputs()[input]);
    str += QString("<P>");
    OSCController *ctrl = m_IOmapping.at(input).controller;
    if (ctrl == NULL || ctrl->type() == OSCController::Output)
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

void OSCPlugin::sendFeedBack(quint32 universe, quint32 input,
                             quint32 channel, uchar value, const QString &key)
{
    if (input >= (quint32)m_IOmapping.count())
        return;

    OSCController *controller = m_IOmapping[input].controller;
    if (controller != NULL)
        controller->sendFeedback(universe, channel, value, key);
}

/*********************************************************************
 * Configuration
 *********************************************************************/
void OSCPlugin::configure()
{
    ConfigureOSC conf(this);
    conf.exec();
}

bool OSCPlugin::canConfigure()
{
    return true;
}

void OSCPlugin::setParameter(quint32 universe, quint32 line, Capability type,
                              QString name, QVariant value)
{
    if (line >= (quint32)m_IOmapping.length())
        return;

    OSCController *controller = m_IOmapping.at(line).controller;
    if (controller == NULL)
        return;

    // If the Controller parameter is restored to its default value,
    // unset the corresponding plugin parameter
    bool unset;

    if (name == OSC_INPUTPORT)
        unset = controller->setInputPort(universe, value.toUInt());
    else if (name == OSC_FEEDBACKIP)
        unset = controller->setFeedbackIPAddress(universe, value.toString());
    else if (name == OSC_FEEDBACKPORT)
        unset = controller->setFeedbackPort(universe, value.toUInt());
    else if (name == OSC_OUTPUTIP)
        unset = controller->setOutputIPAddress(universe, value.toString());
    else if (name == OSC_OUTPUTPORT)
        unset = controller->setOutputPort(universe, value.toUInt());
    else
    {
        qWarning() << Q_FUNC_INFO << name << "is not a valid OSC parameter";
        return;
    }

    if (unset)
        QLCIOPlugin::unSetParameter(universe, line, type, name);
    else
        QLCIOPlugin::setParameter(universe, line, type, name, value);
}

QList<OSCIO> OSCPlugin::getIOMapping()
{
    return m_IOmapping;
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(OSCPlugin, OSCPlugin)
#endif
