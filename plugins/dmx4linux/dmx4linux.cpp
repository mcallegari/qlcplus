/*
  Q Light Controller
  dmx4linux.cpp

  Copyright (c) Heikki Junnila

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
#include <QString>
#include <QDebug>
#include <QFile>

#include "dmx4linux.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

DMX4Linux::~DMX4Linux()
{
    if (m_file.isOpen() == true)
        m_file.close();
}

void DMX4Linux::init()
{
    m_file.setFileName("/dev/dmx");
}

QString DMX4Linux::name()
{
    return QString("DMX4Linux");
}

int DMX4Linux::capabilities() const
{
    return QLCIOPlugin::Output;
}

/*****************************************************************************
 * Open/close
 *****************************************************************************/

bool DMX4Linux::openOutput(quint32 output, quint32 universe)
{
    Q_UNUSED(universe)
    if (output != 0)
        return false;

    m_file.unsetError();
    if (m_file.open(QIODevice::WriteOnly | QIODevice::Unbuffered) == false)
    {
        qWarning() << "DMX4Linux output is not available:"
                   << m_file.errorString();
        return false;
    }
    return true;
}

void DMX4Linux::closeOutput(quint32 output, quint32 universe)
{
    Q_UNUSED(universe)
    if (output != 0)
        return;

    m_file.close();
    m_file.unsetError();
}

QStringList DMX4Linux::outputs()
{
    QStringList list;
    if (m_file.exists() == true)
        list << QString("DMX4Linux");
    return list;
}

QString DMX4Linux::pluginInfo()
{
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    str += QString("<P>");
        str += QString("<H3>%1</H3>").arg(name());
        str += tr("This plugin provides DMX output for devices supported by "
                  "the DMX4Linux driver suite.");
        str += QString("</P>");

    return str;
    }

QString DMX4Linux::outputInfo(quint32 output)
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

void DMX4Linux::writeUniverse(quint32 universe, quint32 output, const QByteArray &data, bool dataChanged)
{
    Q_UNUSED(universe)
    Q_UNUSED(dataChanged)

    if (output != 0 || m_file.isOpen() == false)
        return;

    m_file.seek(0);
    if (m_file.write(data) == -1)
        qWarning() << "DMX4Linux: Unable to write:" << m_file.errorString();
}
