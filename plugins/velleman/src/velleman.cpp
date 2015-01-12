/*
  Q Light Controller
  velleman.cpp

  Copyright (c) Matthew Jaggard

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
#include <stdint.h>
#include <QString>
#include <QDebug>
#include <QFile>

#if defined(WIN32) || defined(Q_OS_WIN)
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

bool Velleman::openOutput(quint32 output)
{
    if (output != 0)
        return false;

    if (m_currentlyOpen == false)
    {
        StartDevice();
        m_currentlyOpen = true;
    }
    return true;
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

QString Velleman::pluginInfo()
{
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    str += QString("<P>");
    str += QString("<H3>%1</H3>").arg(name());
    str += tr("This plugin provides DMX output support for the Velleman "
              "K8062D using the DLL supplied with the product.");
    str += QString("</P>");

    return str;
}

QString Velleman::outputInfo(quint32 output)
{
    QString str;

    if (output != QLCIOPlugin::invalidLine() && output == 0)
    {
        str += QString("<H3>%1</H3>").arg(outputs()[output]);
    }

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void Velleman::writeUniverse(quint32 universe, quint32 output, const QByteArray &data)
{
    Q_UNUSED(universe)

    if (output != 0 || m_currentlyOpen == false)
        return;

    SetChannelCount((int32_t) data.size());
    for (int i = 0; i < data.size(); i++)
        m_values[i] = (qint32) data[i];

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
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(velleman, Velleman)
#endif
