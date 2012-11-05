/*
  Q Light Controller
  velleman.cpp

  Copyright (c) Matthew Jaggard

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

#include <QStringList>
#include <stdint.h>
#include <QString>
#include <QDebug>
#include <QFile>

#ifdef WIN32
#   include <windows.h>
#endif

#include "qlcmacros.h"
#include "velleman.h"

/*****************************************************************************
 * The Velleman interface for k8062d.dll
 *****************************************************************************/

extern "C"
{
    void StartDevice();
    void StopDevice();

    void SetChannelCount(int32_t Count);

    void SetData(int32_t Channel, int32_t Data);
    void SetAllData(int32_t Data[]);
}

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Velleman::~Velleman()
{
    closeOutput(0);
    delete [] m_values;
}

void Velleman::init()
{
    m_values = new qint32[512];
    m_currentlyOpen = false;
}

QString Velleman::name()
{
    return QString("Velleman");
}

int Velleman::capabilities() const
{
    return QLCIOPlugin::Output;
}

/*****************************************************************************
 * Outputs
 *****************************************************************************/

void Velleman::openOutput(quint32 output)
{
    if (output != 0)
        return;

    if (m_currentlyOpen == false)
    {
        StartDevice();
        m_currentlyOpen = true;
    }
}

void Velleman::closeOutput(quint32 output)
{
    if (output != 0)
        return;

    if (m_currentlyOpen == true)
    {
        m_currentlyOpen = false;
        StopDevice();
    }
}

QStringList Velleman::outputs()
{
    QStringList list;
    list << QString("1: Velleman Device");
    return list;
}

QString Velleman::outputInfo(quint32 output)
{
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    if (output == QLCIOPlugin::invalidLine())
    {
        str += QString("<H3>%1</H3>").arg(name());
        str += QString("<P>");
        str += tr("This plugin provides DMX output support for the Velleman "
                  "K8062D using the DLL supplied with the product.");
        str += QString("</P>");
    }
    else if (output == 0)
    {
        str += QString("<H3>%1</H3>").arg(outputs()[output]);
    }

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void Velleman::writeUniverse(quint32 output, const QByteArray& universe)
{
    if (output != 0 || m_currentlyOpen == false)
        return;

    SetChannelCount((int32_t) universe.size());
    for (int i = 0; i < universe.size(); i++)
        m_values[i] = (qint32) universe[i];

    SetAllData((int32_t*) m_values);
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void Velleman::configure()
{
}

bool Velleman::canConfigure()
{
    return false;
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(velleman, Velleman)
