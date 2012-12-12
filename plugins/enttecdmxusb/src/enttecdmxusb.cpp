/*
  Q Light Controller
  enttecdmx.cpp

  Copyright (C) Heikki Junnila

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,$
*/

#include <QStringList>
#include <QMessageBox>
#include <QDebug>

#include "enttecdmxusbconfig.h"
#include "enttecdmxusbwidget.h"
#include "enttecdmxusbprorx.h"
#include "enttecdmxusbopen.h"
#include "enttecdmxusb.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

EnttecDMXUSB::~EnttecDMXUSB()
{
    while (m_outputs.isEmpty() == false)
        delete m_outputs.takeFirst();

    while (m_inputs.isEmpty() == false)
        delete m_inputs.takeFirst();
}

void EnttecDMXUSB::init()
{
    /* Search for new widgets */
    rescanWidgets();
}

QString EnttecDMXUSB::name()
{
    return QString("Enttec DMX USB");
}

int EnttecDMXUSB::capabilities() const
{
    return QLCIOPlugin::Output | QLCIOPlugin::Input;
}

bool EnttecDMXUSB::rescanWidgets()
{
    while (m_outputs.isEmpty() == false)
        delete m_outputs.takeFirst();
    while (m_inputs.isEmpty() == false)
        delete m_inputs.takeFirst();

    QList <EnttecDMXUSBWidget*> widgets(QLCFTDI::widgets());

    foreach (EnttecDMXUSBWidget* widget, widgets)
    {
        if (widget->type() == EnttecDMXUSBWidget::ProRX)
        {
            m_inputs << widget;
            EnttecDMXUSBProRX* prorx = (EnttecDMXUSBProRX*) widget;
            connect(prorx, SIGNAL(valueChanged(quint32,quint32,uchar)),
                    this, SIGNAL(valueChanged(quint32,quint32,uchar)));
        }
        else
        {
            m_outputs << widget;
        }
    }

    emit configurationChanged();
    return true;
}

QList <EnttecDMXUSBWidget*> EnttecDMXUSB::widgets() const
{
    QList <EnttecDMXUSBWidget*> widgets;
    widgets << m_outputs;
    widgets << m_inputs;
    return widgets;
}

/****************************************************************************
 * Outputs
 ****************************************************************************/

void EnttecDMXUSB::openOutput(quint32 output)
{
    if (output < quint32(m_outputs.size()))
        m_outputs.at(output)->open();
}

void EnttecDMXUSB::closeOutput(quint32 output)
{
    if (output < quint32(m_outputs.size()))
        m_outputs.at(output)->close();
}

QStringList EnttecDMXUSB::outputs()
{
    QStringList list;
    int i = 1;

    QListIterator <EnttecDMXUSBWidget*> it(m_outputs);
    while (it.hasNext() == true)
        list << QString("%1: %2").arg(i++).arg(it.next()->uniqueName());
    return list;
}

QString EnttecDMXUSB::pluginInfo()
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
    str += QString(" DMXKing USB DMX512-A, Enttec DMX USB Pro, "
                   "Enttec Open DMX USB, FTDI USB COM485 Plus1 ");
    str += tr("and compatible devices.");
    str += QString("</P>");

    return str;
}

QString EnttecDMXUSB::outputInfo(quint32 output)
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
        str += QString("</P>");
        QString add = m_outputs[output]->additionalInfo();
        if (add.isEmpty() == false)
            str += add;
    }

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void EnttecDMXUSB::writeUniverse(quint32 output, const QByteArray& universe)
{
    if (output < quint32(m_outputs.size()))
        m_outputs.at(output)->writeUniverse(universe);
}

/****************************************************************************
 * Inputs
 ****************************************************************************/

void EnttecDMXUSB::openInput(quint32 input)
{
    if (input < quint32(m_inputs.size()))
        m_inputs.at(input)->open();
}

void EnttecDMXUSB::closeInput(quint32 input)
{
    if (input < quint32(m_inputs.size()))
        m_inputs.at(input)->close();
}

QStringList EnttecDMXUSB::inputs()
{
    QStringList list;
    int i = 1;

    QListIterator <EnttecDMXUSBWidget*> it(m_inputs);
    while (it.hasNext() == true)
        list << QString("%1: %2").arg(i++).arg(it.next()->uniqueName());
    return list;
}

QString EnttecDMXUSB::inputInfo(quint32 input)
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

void EnttecDMXUSB::configure()
{
    qDebug() << Q_FUNC_INFO;
    EnttecDMXUSBConfig config(this);
    config.exec();
    rescanWidgets();
}

bool EnttecDMXUSB::canConfigure()
{
    return true;
}

/****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(enttecdmxusb, EnttecDMXUSB)
