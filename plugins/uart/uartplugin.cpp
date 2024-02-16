/*
  Q Light Controller Plus
  uartplugin.cpp

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

#include <QtSerialPort/QSerialPortInfo>

#include "uartplugin.h"
#include "uartwidget.h"

/** Place here the global defines specific to this plugin */

/*****************************************************************************
 * Initialization
 *****************************************************************************/

UARTPlugin::~UARTPlugin()
{
    /** Clean up the plugin resources here.
     *  E.g. running threads, allocated memory, etc..
     */
}

void UARTPlugin::init()
{
    /** Initialize the plugin variables here */
    foreach (QSerialPortInfo info, QSerialPortInfo::availablePorts())
    {
        // ignore all the USB adapters that have a valid VID/PID
        if (info.hasVendorIdentifier() || info.hasProductIdentifier())
            continue;

        m_widgets << new UARTWidget(info);
    }
}

QString UARTPlugin::name()
{
    return QString("UART");
}

int UARTPlugin::capabilities() const
{
    return QLCIOPlugin::Output; // | QLCIOPlugin::Input;
}

/*****************************************************************************
 * Outputs
 *****************************************************************************/

bool UARTPlugin::openOutput(quint32 output, quint32 universe)
{
    if (output < quint32(m_widgets.count()))
    {
        addToMap(universe, output, Output);

        return m_widgets.at(output)->open(UARTWidget::Output);
    }
    return false;
}

void UARTPlugin::closeOutput(quint32 output, quint32 universe)
{
    if (output < quint32(m_widgets.count()))
    {
        removeFromMap(output, universe, Output);

        m_widgets.at(output)->close(UARTWidget::Output);
    }
}

QStringList UARTPlugin::outputs()
{
    QStringList list;
    for (int i = 0; i < m_widgets.count(); i++)
    {
        UARTWidget *widget = m_widgets.at(i);
        list << widget->name();
    }
    return list;
}

QString UARTPlugin::pluginInfo()
{
    /** Return a description of the purpose of this plugin
     *  in HTML format */
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    str += QString("<P>");
    str += QString("<H3>%1</H3>").arg(name());
    str += tr("This plugin provides output for UART devices.");
    str += QString("</P>");

    return str;
}

QString UARTPlugin::outputInfo(quint32 output)
{
    QString str;

    if (output != QLCIOPlugin::invalidLine())
        str += QString("<H3>%1</H3>").arg(outputs()[output]);

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void UARTPlugin::writeUniverse(quint32 universe, quint32 output, const QByteArray &data, bool dataChanged)
{
    Q_UNUSED(universe)
    Q_UNUSED(dataChanged)

    if (output < quint32(m_widgets.count()))
        m_widgets.at(output)->writeUniverse(data);
}

/*************************************************************************
 * Inputs - If the plugin doesn't provide input
 * just remove this whole block
 *************************************************************************/

bool UARTPlugin::openInput(quint32 input, quint32 universe)
{
    if (input < quint32(m_widgets.count()))
    {
        addToMap(universe, input, Input);

        return m_widgets.at(input)->open(UARTWidget::Input);
    }

    return false;
}

void UARTPlugin::closeInput(quint32 input, quint32 universe)
{
    if (input < quint32(m_widgets.count()))
    {
        removeFromMap(input, universe, Input);
        m_widgets.at(input)->close(UARTWidget::Input);
    }
}

QStringList UARTPlugin::inputs()
{
    QStringList list;
    for (int i = 0; i < m_widgets.count(); i++)
    {
        UARTWidget *widget = m_widgets.at(i);
        list << widget->name();
    }
    return list;
}

QString UARTPlugin::inputInfo(quint32 input)
{
    QString str;

    if (input != QLCIOPlugin::invalidLine())
        str += QString("<H3>%1</H3>").arg(inputs()[input]);

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}
