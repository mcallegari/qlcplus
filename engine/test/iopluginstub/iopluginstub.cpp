/*
  Q Light Controller
  ioplugin_stub.cpp

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

#include <QtPlugin>
#include "iopluginstub.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

IOPluginStub::~IOPluginStub()
{
}

void IOPluginStub::init()
{
    m_configureCalled = 0;
    m_canConfigure = false;
    m_universe = QByteArray(int(4 * 512), char(0));
}

QString IOPluginStub::name()
{
    return QString("I/O Plugin Stub");
}

int IOPluginStub::capabilities() const
{
    return QLCIOPlugin::Output | QLCIOPlugin::Input;
}

/*****************************************************************************
 * Outputs
 *****************************************************************************/

bool IOPluginStub::openOutput(quint32 output, quint32 universe)
{
    Q_UNUSED(universe)
    if (m_openOutputs.contains(output) == false && output < 4)
    {
        m_openOutputs.append(output);
        addToMap(universe, output, Output);
    }
    return true;
}

void IOPluginStub::closeOutput(quint32 output, quint32 universe)
{
    Q_UNUSED(universe)
    m_openOutputs.removeAll(output);
}

QStringList IOPluginStub::outputs()
{
    QStringList list;
    for (quint32 i = 0; i < 4; i++)
        list << QString("%1: Stub %1").arg(i + 1);
    return list;
}


QString IOPluginStub::pluginInfo()
{
    return QString("This is a plugin stub for testing.");
}

QString IOPluginStub::outputInfo(quint32 output)
{
    Q_UNUSED(output);
    return QString("This is a plugin stub for testing.");
}

void IOPluginStub::writeUniverse(quint32 universe, quint32 output, const QByteArray &data, bool dataChanged)
{
    Q_UNUSED(universe)
    Q_UNUSED(dataChanged)

    m_universe = m_universe.replace(output * 512, data.size(), data);
}

/*****************************************************************************
 * Inputs
 *****************************************************************************/

bool IOPluginStub::openInput(quint32 input, quint32 universe)
{
    Q_UNUSED(universe)

    if (m_openInputs.contains(input) == false && input < 4)
    {
        m_openInputs.append(input);
        addToMap(universe, input, Input);
    }
    return true;
}

void IOPluginStub::closeInput(quint32 input, quint32 universe)
{
    Q_UNUSED(universe)
    m_openInputs.removeAll(input);
}

QStringList IOPluginStub::inputs()
{
    QStringList list;
    for (quint32 i = 0; i < 4; i++)
        list << QString("%1: Stub %1").arg(i + 1);
    return list;
}

QString IOPluginStub::inputInfo(quint32 input)
{
    Q_UNUSED(input);
    return QString("This is a plugin stub for testing.");
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void IOPluginStub::configure()
{
    m_configureCalled++;
    emit configurationChanged();
}

bool IOPluginStub::canConfigure()
{
    return m_canConfigure;
}
