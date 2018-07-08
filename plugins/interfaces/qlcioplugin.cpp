/*
  Q Light Controller Plus
  qlcioplugin.cpp

  Copyright (c) Heikki Junnila
                Massimo Callegari

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

#include "qlcioplugin.h"
#include <QDebug>

/*************************************************************************
 * Outputs
 *************************************************************************/

bool QLCIOPlugin::openOutput(quint32 output, quint32 universe)
{
    Q_UNUSED(universe)
    Q_UNUSED(output)
    return false;
}

void QLCIOPlugin::closeOutput(quint32 output, quint32 universe)
{
    Q_UNUSED(output)
    Q_UNUSED(universe)
}

QStringList QLCIOPlugin::outputs()
{
    return QStringList();
}

QString QLCIOPlugin::outputInfo(quint32 output)
{
    Q_UNUSED(output)
    return QString();
}

void QLCIOPlugin::writeUniverse(quint32 universe, quint32 output, const QByteArray &data)
{
    Q_UNUSED(universe)
    Q_UNUSED(output)
    Q_UNUSED(data)
}

/*************************************************************************
 * Inputs
 *************************************************************************/

bool QLCIOPlugin::openInput(quint32 input, quint32 universe)
{
    Q_UNUSED(input)
    Q_UNUSED(universe)
    return false;
}

void QLCIOPlugin::closeInput(quint32 input, quint32 universe)
{
    Q_UNUSED(input)
    Q_UNUSED(universe)
}

QStringList QLCIOPlugin::inputs()
{
    return QStringList();
}

QString QLCIOPlugin::inputInfo(quint32 input)
{
    Q_UNUSED(input)
    return QString();
}

void QLCIOPlugin::sendFeedBack(quint32 universe, quint32 inputLine,
                               quint32 channel, uchar value, const QString &key)
{
    Q_UNUSED(universe)
    Q_UNUSED(inputLine)
    Q_UNUSED(channel)
    Q_UNUSED(value)
    Q_UNUSED(key)
}

/*************************************************************************
 * Configure
 *************************************************************************/

void QLCIOPlugin::configure()
{
}

bool QLCIOPlugin::canConfigure()
{
    return false;
}

void QLCIOPlugin::setParameter(quint32 universe, quint32 line,
                               Capability type, QString name, QVariant value)
{
    if (m_universesMap.contains(universe) == false)
        return;

    qDebug() << "[QLCIOPlugin] set parameter:" << universe << line << name << value;

    if (type == Input && m_universesMap[universe].inputLine == line)
        m_universesMap[universe].inputParameters[name] = value;
    else if (type == Output && m_universesMap[universe].outputLine == line)
        m_universesMap[universe].outputParameters[name] = value;
}

void QLCIOPlugin::unSetParameter(quint32 universe, quint32 line, QLCIOPlugin::Capability type, QString name)
{
    if (m_universesMap.contains(universe) == false)
        return;

    qDebug() << "[QLCIOPlugin] unset parameter:" << universe << line << name;

    if (type == Input && m_universesMap[universe].inputLine == line)
    {
        if (m_universesMap[universe].inputParameters.contains(name))
            m_universesMap[universe].inputParameters.take(name);
    }
    else if (type == Output && m_universesMap[universe].outputLine == line)
    {
        if (m_universesMap[universe].outputParameters.contains(name))
            m_universesMap[universe].outputParameters.take(name);
    }
}

QMap<QString, QVariant> QLCIOPlugin::getParameters(quint32 universe, quint32 line, QLCIOPlugin::Capability type)
{
    if (m_universesMap.contains(universe) == false)
        return QMap<QString, QVariant>();

    if (type == Input && m_universesMap[universe].inputLine == line)
        return m_universesMap[universe].inputParameters;
    else if (type == Output && m_universesMap[universe].outputLine == line)
        return m_universesMap[universe].outputParameters;

    return QMap<QString, QVariant>();
}

void QLCIOPlugin::addToMap(quint32 universe, quint32 line,
                           QLCIOPlugin::Capability type)
{
    PluginUniverseDescriptor desc;

    if (m_universesMap.contains(universe))
        desc = m_universesMap[universe];
    else
    {
        // initialize a new descriptor
        desc.inputLine = UINT_MAX;
        desc.outputLine = UINT_MAX;
    }

    if (type == Input)
    {
        desc.inputLine = line;
    }
    else if(type == Output)
    {
        desc.outputLine = line;
    }
    qDebug() << "[QLCIOPlugin] setting lines:" << universe << desc.inputLine << desc.outputLine;
    m_universesMap[universe] = desc;
}

void QLCIOPlugin::removeFromMap(quint32 universe, quint32 line, QLCIOPlugin::Capability type)
{
    if (m_universesMap.contains(universe) == false)
        return;

    if (type == Input && m_universesMap[universe].inputLine == line)
    {
        m_universesMap[universe].inputLine = UINT_MAX;
        m_universesMap[universe].inputParameters.clear();
        return;
    }
    else if(type == Output && m_universesMap[universe].outputLine == line)
    {
        m_universesMap[universe].outputLine = UINT_MAX;
        m_universesMap[universe].outputParameters.clear();
        return;
    }

    // check if we can completely remove this entry
    if (m_universesMap[universe].inputLine == UINT_MAX &&
        m_universesMap[universe].outputLine == UINT_MAX)
    {
        m_universesMap.take(universe);
    }
}
