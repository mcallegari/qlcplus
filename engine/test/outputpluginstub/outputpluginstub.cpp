/*
  Q Light Controller
  outputplugin_stub.cpp

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
#include "outputpluginstub.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

OutputPluginStub::~OutputPluginStub()
{
}

void OutputPluginStub::init()
{
    m_configureCalled = 0;
    m_canConfigure = false;
    m_universe = QByteArray(int(4 * 512), char(0));
}

QString OutputPluginStub::name()
{
    return QString("Output Plugin Stub");
}

int OutputPluginStub::capabilities() const
{
    return QLCIOPlugin::Output | QLCIOPlugin::Input;
}

/*****************************************************************************
 * Outputs
 *****************************************************************************/

void OutputPluginStub::openOutput(quint32 output)
{
    if (m_openOutputs.contains(output) == false && output < 4)
        m_openOutputs.append(output);
}

void OutputPluginStub::closeOutput(quint32 output)
{
    m_openOutputs.removeAll(output);
}

QStringList OutputPluginStub::outputs()
{
    QStringList list;
    for (quint32 i = 0; i < 4; i++)
        list << QString("%1: Stub %1").arg(i + 1);
    return list;
}


QString OutputPluginStub::pluginInfo()
{
    return QString("This is a plugin stub for testing.");
}

QString OutputPluginStub::outputInfo(quint32 output)
{
    Q_UNUSED(output);
    return QString("This is a plugin stub for testing.");
}

void OutputPluginStub::writeUniverse(quint32 output, const QByteArray& universe)
{
    m_universe = m_universe.replace(output * 512, universe.size(), universe);
}

/*****************************************************************************
 * Inputs
 *****************************************************************************/

void OutputPluginStub::openInput(quint32 input)
{
    if (m_openInputs.contains(input) == false && input < 4)
        m_openInputs.append(input);
}

void OutputPluginStub::closeInput(quint32 input)
{
    m_openInputs.removeAll(input);
}

QStringList OutputPluginStub::inputs()
{
    QStringList list;
    for (quint32 i = 0; i < 4; i++)
        list << QString("%1: Stub %1").arg(i + 1);
    return list;
}

QString OutputPluginStub::inputInfo(quint32 input)
{
    Q_UNUSED(input);
    return QString("This is a plugin stub for testing.");
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void OutputPluginStub::configure()
{
    m_configureCalled++;
    emit configurationChanged();
}

bool OutputPluginStub::canConfigure()
{
    return m_canConfigure;
}

/*****************************************************************************
 * Plugin export
 *****************************************************************************/
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(outputpluginstub, OutputPluginStub)
#endif
