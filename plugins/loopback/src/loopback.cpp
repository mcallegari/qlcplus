/*
  Q Light Controller Plus
  loopback.cpp

  Copyright (c) Jano Svitok

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

#include "qlcmacros.h"
#include "loopback.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Loopback::~Loopback()
{
    for (int i = 0; i < QLCIOPLUGINS_UNIVERSES; i++)
    {
        closeOutput(i);
        closeInput(i);
        delete [] m_values[i];
    }
}

void Loopback::init()
{
    for (int i = 0; i < QLCIOPLUGINS_UNIVERSES; i++)
    {
        m_values[i] = new qint32[512];
        m_outputCurrentlyOpen[i] = false;
        m_inputCurrentlyOpen[i] = false;
    }
}

QString Loopback::name()
{
    return QString("Loopback");
}

int Loopback::capabilities() const
{
    return QLCIOPlugin::Output | QLCIOPlugin::Input | QLCIOPlugin::Feedback;
}

/*****************************************************************************
 * Outputs
 *****************************************************************************/

bool Loopback::openOutput(quint32 output)
{
    if (output >= QLCIOPLUGINS_UNIVERSES)
        return false;

    if (m_outputCurrentlyOpen[output] == false)
    {
        m_outputCurrentlyOpen[output] = true;
    }
    return true;
}

void Loopback::closeOutput(quint32 output)
{
    if (output >= QLCIOPLUGINS_UNIVERSES)
        return;

    if (m_outputCurrentlyOpen[output] == true)
    {
        m_outputCurrentlyOpen[output] = false;
    }
}

QStringList Loopback::outputs()
{
    QStringList list;
    for (int i = 0; i < QLCIOPLUGINS_UNIVERSES; i++)
        list << QString("%1: %2 %1").arg(i + 1).arg(tr("Loopback"));
    return list;
}

/*****************************************************************************
 * Inputs
 *****************************************************************************/

bool Loopback::openInput(quint32 input)
{
    if (input >= QLCIOPLUGINS_UNIVERSES)
        return false;

    if (m_inputCurrentlyOpen[input] == false)
    {
        m_inputCurrentlyOpen[input] = true;
    }
    return true;
}

void Loopback::closeInput(quint32 input)
{
    if (input >= QLCIOPLUGINS_UNIVERSES)
        return;

    if (m_inputCurrentlyOpen[input] == true)
    {
        m_inputCurrentlyOpen[input] = false;
    }
}

QStringList Loopback::inputs()
{
    QStringList list;
    for (int i = 0; i < QLCIOPLUGINS_UNIVERSES; i++)
        list << QString("%1: %2 %1").arg(i + 1).arg(tr("Loopback"));
    return list;
}

QString Loopback::pluginInfo()
{
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    str += QString("<P>");
    str += QString("<H3>%1</H3>").arg(name());
    str += tr("This plugin provides DMX loopback. Data written to each output is forwarded to the respective input." );
    str += QString("</P>");

    return str;
}

QString Loopback::outputInfo(quint32 output)
{
    if (output >= QLCIOPLUGINS_UNIVERSES)
        return QString();

    QString str;

    str += QString("<H3>%1 %2</H3>").arg(tr("Output")).arg(outputs()[output]);
    str += QString("<P>");
    if (m_outputCurrentlyOpen[output] == true)
        str += tr("Status: Used");
    else
    {
        str += tr("Status: Not used");
    }
    str += QString("</P>");
    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

QString Loopback::inputInfo(quint32 input)
{
    if (input >= QLCIOPLUGINS_UNIVERSES)
        return QString();

    QString str;

    str += QString("<H3>%1 %2</H3>").arg(tr("Input")).arg(inputs()[input]);
    str += QString("<P>");
    if (m_inputCurrentlyOpen[input] == true)
        str += tr("Status: Used");
    else
    {
        str += tr("Status: Not used");
    }
    str += QString("</P>");
    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void Loopback::writeUniverse(quint32 universe, quint32 output, const QByteArray &data)
{
    Q_UNUSED(universe)

    if (output >= QLCIOPLUGINS_UNIVERSES || m_outputCurrentlyOpen[output] == false)
        return;

    for (int i = 0; i < data.size(); i++)
    {
        if (m_inputCurrentlyOpen[output] && m_values[output][i] != (qint32) data[i])
        {
            emit valueChanged(UINT_MAX, output, i, data[i]);
        }
        m_values[output][i] = (qint32) data[i];
    }
}

void Loopback::sendFeedBack(quint32 input, quint32 channel, uchar value, const QString &)
{
    if (input >= QLCIOPLUGINS_UNIVERSES || m_inputCurrentlyOpen[input] == false)
        return;

    emit valueChanged(UINT_MAX, input, channel, value);
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void Loopback::configure()
{
}

bool Loopback::canConfigure()
{
    return false;
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(loopback, Loopback)
#endif
