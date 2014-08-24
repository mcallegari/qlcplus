/*
  Q Light Controller
  outputpatch.cpp

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

#if defined(WIN32) || defined(Q_OS_WIN)
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#else
#   include <unistd.h>
#endif

#include <QObject>
#include <QtXml>

#include "qlcioplugin.h"
#include "outputpatch.h"

#define GRACE_MS 1

/*****************************************************************************
 * Initialization
 *****************************************************************************/

OutputPatch::OutputPatch(QObject* parent) : QObject(parent)
{
    Q_ASSERT(parent != NULL);

    m_plugin = NULL;
    m_output = QLCIOPlugin::invalidLine();
}

OutputPatch::~OutputPatch()
{
    if (m_plugin != NULL)
        m_plugin->closeOutput(m_output);
}

/****************************************************************************
 * Plugin & Output
 ****************************************************************************/

bool OutputPatch::set(QLCIOPlugin* plugin, quint32 output)
{
    if (m_plugin != NULL && m_output != QLCIOPlugin::invalidLine())
        m_plugin->closeOutput(m_output);

    m_plugin = plugin;
    m_output = output;

    if (m_plugin != NULL && m_output != QLCIOPlugin::invalidLine())
        return m_plugin->openOutput(m_output);
    return false;
}

bool OutputPatch::reconnect()
{
    if (m_plugin != NULL && m_output != QLCIOPlugin::invalidLine())
    {
        m_plugin->closeOutput(m_output);
#if defined(WIN32) || defined(Q_OS_WIN)
        Sleep(GRACE_MS);
#else
        usleep(GRACE_MS * 1000);
#endif
        return m_plugin->openOutput(m_output);
    }
    return false;
}

QString OutputPatch::pluginName() const
{
    if (m_plugin != NULL)
        return m_plugin->name();
    else
        return KOutputNone;
}

QLCIOPlugin* OutputPatch::plugin() const
{
    return m_plugin;
}

QString OutputPatch::outputName() const
{
    if (m_plugin != NULL && m_output != QLCIOPlugin::invalidLine() &&
        m_output < quint32(m_plugin->outputs().size()))
    {
        return m_plugin->outputs()[m_output];
    }
    else
    {
        return KOutputNone;
    }
}

quint32 OutputPatch::output() const
{
    if (m_plugin != NULL && m_output < quint32(m_plugin->outputs().size()))
        return m_output;
    else
        return QLCIOPlugin::invalidLine();
}

bool OutputPatch::isPatched() const
{
    return output() != QLCIOPlugin::invalidLine();
}

void OutputPatch::setPluginProperty(QString prop, QVariant value)
{
    if (m_plugin != NULL)
        m_plugin->setParameter(prop.toLatin1().data(), value);
}

/*****************************************************************************
 * Value dump
 *****************************************************************************/

void OutputPatch::dump(quint32 universe, const QByteArray& data)
{
    /* Don't do anything if there is no plugin and/or output line. */
    if (m_plugin != NULL && m_output != QLCIOPlugin::invalidLine())
        m_plugin->writeUniverse(universe, m_output, data);
}
