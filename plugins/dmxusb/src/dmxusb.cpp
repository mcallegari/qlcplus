/*
  Q Light Controller Plus
  dmxusb.cpp

  Copyright (C) Heikki Junnila
  Copyright (C) Massimo Callegari

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

#include <QStringList>
#include <QMessageBox>
#include <QDebug>

#include "dmxusbconfig.h"
#include "dmxusbwidget.h"
#include "enttecdmxusbpro.h"
#include "enttecdmxusbopen.h"
#include "euroliteusbdmxpro.h"
#include "dmxusb.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

DMXUSB::~DMXUSB()
{
    while (m_widgets.isEmpty() == false)
        delete m_widgets.takeFirst();
}

void DMXUSB::init()
{
    /* Search for new widgets */
    rescanWidgets();
}

QString DMXUSB::name()
{
    return QString("DMX USB");
}

int DMXUSB::capabilities() const
{
    return QLCIOPlugin::Output | QLCIOPlugin::Input | QLCIOPlugin::RDM;
}

bool DMXUSB::rescanWidgets()
{
    int linesCount = m_inputs.count() + m_outputs.count();
    m_inputs.clear();
    m_outputs.clear();

    while (m_widgets.isEmpty() == false)
        delete m_widgets.takeFirst();

    m_widgets = DMXUSBWidget::widgets();

    foreach (DMXUSBWidget* widget, m_widgets)
    {
        for (int o = 0; o < widget->outputsNumber(); o++)
            m_outputs.append(widget);

        for (int i = 0; i < widget->inputsNumber(); i++)
            m_inputs.append(widget);
    }

    if (m_inputs.count() + m_outputs.count() != linesCount)
        emit configurationChanged();

    return true;
}

QList <DMXUSBWidget*> DMXUSB::widgets() const
{
    return m_widgets;
}

/****************************************************************************
 * Outputs
 ****************************************************************************/

bool DMXUSB::openOutput(quint32 output, quint32 universe)
{
    if (output < quint32(m_outputs.size()))
    {
        DMXUSBWidget *widget = m_outputs.at(output);
        if (widget->supportRDM())
        {
            EnttecDMXUSBPro *pro = static_cast<EnttecDMXUSBPro*>(widget);
            connect(pro, SIGNAL(rdmValueChanged(quint32, quint32, QVariantMap)),
                    this , SIGNAL(rdmValueChanged(quint32, quint32, QVariantMap)));
        }
        addToMap(universe, output, Output);
        return m_outputs.at(output)->open(output, false);
    }
    return false;
}

void DMXUSB::closeOutput(quint32 output, quint32 universe)
{
    if (output < quint32(m_outputs.size()))
    {
        DMXUSBWidget *widget = m_outputs.at(output);
        if (widget->supportRDM())
        {
            EnttecDMXUSBPro *pro = static_cast<EnttecDMXUSBPro*>(widget);
            disconnect(pro, SIGNAL(rdmValueChanged(quint32, quint32, QVariantMap)),
                       this , SIGNAL(rdmValueChanged(quint32, quint32, QVariantMap)));
        }
        removeFromMap(output, universe, Output);
        m_outputs.at(output)->close(output, false);
    }
}

QStringList DMXUSB::outputs()
{
    QStringList list;

    for (int w = 0; w < m_outputs.count();)
    {
        DMXUSBWidget* widget = m_outputs.at(w);
        foreach (QString name, widget->outputNames())
            list << name;
        w += widget->outputsNumber();
    }
    return list;
}

QString DMXUSB::pluginInfo()
{
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    str += QString("<P>");
    str += QString("<H3>%1</H3>").arg(name());
    str += tr("This plugin provides DMX output support for");
    str += QString(" DMXKing ultraDMX range, Enttec DMX USB Pro, "
                   "Enttec Open DMX USB, FTDI USB COM485 Plus1, "
                   "Vince USB-DMX512 ");
    str += tr("and compatible devices.");
    str += QString("</P>");

    return str;
}

QString DMXUSB::outputInfo(quint32 output)
{
    QString str;

    if (output == QLCIOPlugin::invalidLine())
    {
        if (m_outputs.size() == 0)
        {
            str += QString("<BR><B>%1</B>").arg(tr("No output support available."));
            str += QString("<P>");
            str += tr("Make sure that you have your hardware firmly plugged in. "
                      "NOTE: FTDI VCP interface is not supported by this plugin.");
            str += QString("</P>");
        }
    }
    else if (output < quint32(m_outputs.size()))
    {
        str += QString("<H3>%1</H3>").arg(outputs()[output]);
        str += QString("<P>");
        str += tr("Device is operating correctly.");
        str += QString("<BR>");
        str += tr("Driver in use: %1").arg(m_outputs[output]->interfaceTypeString());
        str += QString("</P>");
        QString add = m_outputs[output]->additionalInfo();
        if (add.isEmpty() == false)
            str += add;
    }

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void DMXUSB::writeUniverse(quint32 universe, quint32 output, const QByteArray &data, bool dataChanged)
{
    if (output < quint32(m_outputs.size()))
        m_outputs.at(output)->writeUniverse(universe, output, data, dataChanged);
}

/****************************************************************************
 * Inputs
 ****************************************************************************/

bool DMXUSB::openInput(quint32 input, quint32 universe)
{
    if (input < quint32(m_inputs.size()))
    {
        DMXUSBWidget *widget = m_inputs.at(input);
        if (widget->type() == DMXUSBWidget::ProRXTX ||
            widget->type() == DMXUSBWidget::ProMk2 ||
            widget->type() == DMXUSBWidget::OpenRX ||
            widget->type() == DMXUSBWidget::UltraPro)
        {
            EnttecDMXUSBPro *pro = static_cast<EnttecDMXUSBPro*>(widget);
            connect(pro, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)),
                    this, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)));
        }
        addToMap(universe, input, Input);
        return widget->open(input, true);
    }
    return false;
}

void DMXUSB::closeInput(quint32 input, quint32 universe)
{
    if (input < quint32(m_inputs.size()))
    {
        DMXUSBWidget *widget = m_inputs.at(input);
        removeFromMap(input, universe, Input);
        widget->close(input, true);
        if (widget->type() == DMXUSBWidget::ProRXTX ||
            widget->type() == DMXUSBWidget::ProMk2 ||
            widget->type() == DMXUSBWidget::UltraPro)
        {
            EnttecDMXUSBPro* pro = (EnttecDMXUSBPro*) widget;
            disconnect(pro, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)),
                       this, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)));
        }
    }
}

QStringList DMXUSB::inputs()
{
    QStringList list;

    for (int w = 0; w < m_inputs.count();)
    {
        DMXUSBWidget* widget = m_inputs.at(w);
        foreach (QString name, widget->inputNames())
            list << name;
        w += widget->inputsNumber();
    }

    return list;
}

QString DMXUSB::inputInfo(quint32 input)
{
    QString str;

    if (input == QLCIOPlugin::invalidLine())
    {
        if (m_inputs.size() == 0)
        {
            str += QString("<BR><B>%1</B>").arg(tr("No input support available."));
            /*
            str += QString("<P>");
            str += tr("Make sure that you have your hardware firmly plugged in. "
                      "NOTE: FTDI VCP interface is not supported by this plugin.");
            str += QString("</P>");
            */
        }
    }
    else if (input < quint32(m_inputs.size()))
    {
        str += QString("<H3>%1</H3>").arg(inputs()[input]);
        str += QString("<P>");
        str += tr("Device is operating correctly.");
        str += QString("<BR>");
        str += tr("Driver in use: %1").arg(m_inputs[input]->interfaceTypeString());
        str += QString("</P>");
        QString add = m_inputs[input]->additionalInfo();
        if (add.isEmpty() == false)
            str += add;
    }

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

/****************************************************************************
 * Configuration
 ****************************************************************************/

void DMXUSB::configure()
{
    qDebug() << Q_FUNC_INFO;
    DMXUSBConfig config(this);
    config.exec();
    rescanWidgets();
    emit configurationChanged();
}

bool DMXUSB::canConfigure()
{
    return true;
}

bool DMXUSB::sendRDMCommand(quint32 universe, quint32 line, uchar command, QVariantList params)
{
    if (line < quint32(m_outputs.size()))
    {
        DMXUSBWidget *widget = m_outputs.at(line);
        if (widget->supportRDM())
            return widget->sendRDMCommand(universe, line, command, params);
    }

    return false;
}

/*****************************************************************************
 * Hotplug
 *****************************************************************************/

void DMXUSB::slotDeviceAdded(uint vid, uint pid)
{
    qDebug() << Q_FUNC_INFO << QString::number(vid, 16) << QString::number(pid, 16);
    if (!DMXInterface::validInterface(vid, pid))
    {
        qDebug() << Q_FUNC_INFO << "Invalid DMX USB device, nothing to do";
        return;
    }

    rescanWidgets();
}

void DMXUSB::slotDeviceRemoved(uint vid, uint pid)
{
    qDebug() << Q_FUNC_INFO << QString::number(vid, 16) << QString::number(pid, 16);
    if (!DMXInterface::validInterface(vid, pid))
    {
        qDebug() << Q_FUNC_INFO << "Invalid DMX USB device, nothing to do";
        return;
    }

    rescanWidgets();
}
