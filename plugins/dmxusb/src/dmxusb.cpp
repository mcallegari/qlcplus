/*
  Q Light Controller
  dmxusb.cpp

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

#include "dmxusbconfig.h"
#include "dmxusbwidget.h"
#include "enttecdmxusbprorx.h"
#include "enttecdmxusbopen.h"
#include "dmxusb.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

DMXUSB::~DMXUSB()
{
    while (m_outputs.isEmpty() == false)
        delete m_outputs.takeFirst();

    while (m_inputs.isEmpty() == false)
        delete m_inputs.takeFirst();
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
    return QLCIOPlugin::Output | QLCIOPlugin::Input;
}

bool DMXUSB::rescanWidgets()
{
    while (m_outputs.isEmpty() == false)
        delete m_outputs.takeFirst();
    while (m_inputs.isEmpty() == false)
        delete m_inputs.takeFirst();

    QList <DMXUSBWidget*> widgets(QLCFTDI::widgets());

    foreach (DMXUSBWidget* widget, widgets)
    {
        if (widget->type() == DMXUSBWidget::ProRX)
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

QList <DMXUSBWidget*> DMXUSB::widgets() const
{
    QList <DMXUSBWidget*> widgets;
    widgets << m_outputs;
    widgets << m_inputs;
    return widgets;
}

/****************************************************************************
 * Outputs
 ****************************************************************************/

void DMXUSB::openOutput(quint32 output)
{
    if (output < quint32(m_outputs.size()))
        m_outputs.at(output)->open();
}

void DMXUSB::closeOutput(quint32 output)
{
    if (output < quint32(m_outputs.size()))
        m_outputs.at(output)->close();
}

QStringList DMXUSB::outputs()
{
    QStringList list;
    int i = 1;

    QListIterator <DMXUSBWidget*> it(m_outputs);
    while (it.hasNext() == true)
        list << QString("%1: %2").arg(i++).arg(it.next()->uniqueName());
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
                   "Enttec Open DMX USB, FTDI USB COM485 Plus1 ");
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
        str += QString("</P>");
        QString add = m_outputs[output]->additionalInfo();
        if (add.isEmpty() == false)
            str += add;
    }

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void DMXUSB::writeUniverse(quint32 output, const QByteArray& universe)
{
    if (output < quint32(m_outputs.size()))
        m_outputs.at(output)->writeUniverse(universe);
}

/****************************************************************************
 * Inputs
 ****************************************************************************/

void DMXUSB::openInput(quint32 input)
{
    if (input < quint32(m_inputs.size()))
        m_inputs.at(input)->open();
}

void DMXUSB::closeInput(quint32 input)
{
    if (input < quint32(m_inputs.size()))
        m_inputs.at(input)->close();
}

QStringList DMXUSB::inputs()
{
    QStringList list;
    int i = 1;

    QListIterator <DMXUSBWidget*> it(m_inputs);
    while (it.hasNext() == true)
        list << QString("%1: %2").arg(i++).arg(it.next()->uniqueName());
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
}

bool DMXUSB::canConfigure()
{
    return true;
}

/****************************************************************************
 * Plugin export
 ****************************************************************************/
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(dmxusb, DMXUSB)
#endif
