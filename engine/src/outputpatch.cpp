/*
  Q Light Controller
  outputpatch.cpp

  Copyright (c) Heikki Junnila

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

#ifdef WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#else
#   include <unistd.h>
#endif

#include <QObject>
#include <QtXml>

#include "qlcioplugin.h"
#include "outputpatch.h"
#include "outputmap.h"

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

void OutputPatch::set(QLCIOPlugin* plugin, quint32 output)
{
    if (m_plugin != NULL && m_output != QLCIOPlugin::invalidLine())
        m_plugin->closeOutput(m_output);

    m_plugin = plugin;
    m_output = output;

    if (m_plugin != NULL && m_output != QLCIOPlugin::invalidLine())
        m_plugin->openOutput(m_output);
}

void OutputPatch::reconnect()
{
    if (m_plugin != NULL && m_output != QLCIOPlugin::invalidLine())
    {
        m_plugin->closeOutput(m_output);
#ifdef WIN32
        Sleep(GRACE_MS);
#else
        usleep(GRACE_MS * 1000);
#endif
        m_plugin->openOutput(m_output);
    }
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

/*****************************************************************************
 * Value dump
 *****************************************************************************/

void OutputPatch::dump(const QByteArray& universe)
{
    /* Don't do anything if there is no plugin and/or output line. */
    if (m_plugin != NULL && m_output != QLCIOPlugin::invalidLine())
        m_plugin->writeUniverse(m_output, universe);
}
