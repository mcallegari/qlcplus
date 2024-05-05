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

#define UNIVERSE_SIZE 512
#define LOOPBACK_LINES 4

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Loopback::~Loopback()
{
    QMapIterator<quint32, quint32> i(m_inputMap);
    while (i.hasNext())
    {
        closeInput(i.key(), i.value());
    }
    m_inputMap.clear();
    QMapIterator<quint32, quint32> o(m_outputMap);
    while (o.hasNext())
    {
        closeOutput(o.key(), o.value());
    }
    m_outputMap.clear();
}

void Loopback::init()
{
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

bool Loopback::openOutput(quint32 output, quint32 universe)
{
    QByteArray & data = m_channelData[output];
    if (data.size() < UNIVERSE_SIZE)
        data.fill(0, UNIVERSE_SIZE - data.size());

    m_outputMap[output] = universe;
    addToMap(universe, output, Output);

    return true;
}

void Loopback::closeOutput(quint32 output, quint32 universe)
{
    m_outputMap.remove(output);
    m_channelData.remove(output);
    removeFromMap(output, universe, Output);
}

QStringList Loopback::outputs()
{
    QStringList list;
    for (int i = 0; i < LOOPBACK_LINES; i++)
        list << QString("Loopback %1").arg(i + 1);
    return list;
}

/*****************************************************************************
 * Inputs
 *****************************************************************************/

bool Loopback::openInput(quint32 input, quint32 universe)
{
    m_inputMap[input] = universe;
    addToMap(universe, input, Input);

    return true;
}

void Loopback::closeInput(quint32 input, quint32 universe)
{
    m_inputMap.remove(input);
    removeFromMap(input, universe, Input);
}

QStringList Loopback::inputs()
{
    QStringList list;
    for (int i = 0; i < LOOPBACK_LINES; i++)
        list << QString("Loopback %1").arg(i + 1);
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
    str += tr("This plugin provides DMX loopback. Data written to each output is forwarded to the respective input.");
    str += QString("</P>");

    return str;
}

QString Loopback::outputInfo(quint32 output)
{
    if (output >= LOOPBACK_LINES)
        return QString();

    QString str;

    str += QString("<H3>%1 %2</H3>").arg(tr("Output")).arg(outputs()[output]);
    str += QString("<P>");
    if (m_outputMap.contains(output))
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
    if (input >= LOOPBACK_LINES)
        return QString();

    QString str;

    str += QString("<H3>%1 %2</H3>").arg(tr("Input")).arg(inputs()[input]);
    str += QString("<P>");
    if (m_inputMap.contains(input))
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

void Loopback::writeUniverse(quint32 universe, quint32 output, const QByteArray &data, bool dataChanged)
{
    Q_UNUSED(universe)
    Q_UNUSED(dataChanged)

    if (!m_outputMap.contains(output))
        return;

    QByteArray &chData = m_channelData[output];

    if (m_inputMap.contains(output))
    {
        quint32 inputUniverse = m_inputMap[output];

        for (int i = 0; i < data.size(); i++)
        {
            if (chData[i] != data[i])
            {
                chData[i] = data[i];
                //qDebug() << "Data changed at" << i << QString::number(chData[i]) << QString::number(data[i]);
                emit valueChanged(inputUniverse, output, i, chData[i]);
            }
        }
    }
}

void Loopback::sendFeedBack(quint32 universe, quint32 input, quint32 channel, uchar value, const QVariant &)
{
    if (!m_inputMap.contains(input))
        return;

    emit valueChanged(universe, input, channel, value);
}
