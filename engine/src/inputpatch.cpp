/*
  Q Light Controller
  inputpatch.cpp

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
#   include <Windows.h>
#else
#   include <unistd.h>
#endif

#include <QObject>
#include <QDebug>
#include <QtXml>

#include "qlcioplugin.h"
#include "inputpatch.h"
#include "inputmap.h"

#define GRACE_MS 1

/*****************************************************************************
 * Initialization
 *****************************************************************************/

InputPatch::InputPatch(quint32 inputUniverse, QObject* parent)
    : QObject(parent)
    , m_inputUniverse(inputUniverse)
    , m_plugin(NULL)
    , m_input(QLCIOPlugin::invalidLine())
    , m_profile(NULL)
{
    Q_ASSERT(parent != NULL);
}

InputPatch::~InputPatch()
{
    if (m_plugin != NULL)
        m_plugin->closeInput(m_input);
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void InputPatch::set(QLCIOPlugin* plugin, quint32 input, QLCInputProfile* profile)
{
    if (m_plugin != NULL && m_input != QLCIOPlugin::invalidLine())
    {
        disconnect(m_plugin, SIGNAL(valueChanged(quint32,quint32,uchar,QString)),
                   this, SLOT(slotValueChanged(quint32,quint32,uchar,QString)));
        m_plugin->closeInput(m_input);
    }

    m_plugin = plugin;
    m_input = input;
    m_profile = profile;

    /* Open the assigned plugin input */
    if (m_plugin != NULL && m_input != QLCIOPlugin::invalidLine())
    {
        connect(m_plugin, SIGNAL(valueChanged(quint32,quint32,uchar,QString)),
                this, SLOT(slotValueChanged(quint32,quint32,uchar,QString)));
        connect(m_plugin, SIGNAL(pageChanged(quint32,quint32,quint32)),
                this, SLOT(slotPageChanged(quint32,quint32,quint32)));
        m_plugin->openInput(m_input);
    }
}

void InputPatch::reconnect()
{
    if (m_plugin != NULL && m_input != QLCIOPlugin::invalidLine())
    {
        m_plugin->closeInput(m_input);
#ifdef WIN32
        Sleep(GRACE_MS);
#else
        usleep(GRACE_MS * 1000);
#endif
        m_plugin->openInput(m_input);
    }
}

QLCIOPlugin* InputPatch::plugin() const
{
    return m_plugin;
}

QString InputPatch::pluginName() const
{
    if (m_plugin != NULL)
        return m_plugin->name();
    else
        return KInputNone;
}

quint32 InputPatch::input() const
{
    if (m_plugin != NULL && m_input < quint32(m_plugin->inputs().count()))
        return m_input;
    else
        return QLCIOPlugin::invalidLine();
}

QString InputPatch::inputName() const
{
    if (m_plugin != NULL && m_input != QLCIOPlugin::invalidLine() &&
            m_input < quint32(m_plugin->inputs().count()))
        return m_plugin->inputs()[m_input];
    else
        return KInputNone;
}

QLCInputProfile* InputPatch::profile() const
{
    return m_profile;
}

QString InputPatch::profileName() const
{
    if (m_profile != NULL)
        return m_profile->name();
    else
        return KInputNone;
}

void InputPatch::slotValueChanged(quint32 input, quint32 channel, uchar value, const QString& key)
{
    // In case we have several lines connected from the same plugin, emit only
    // such values that belong to this particular patch.
    if (input == m_input)
        emit inputValueChanged(m_inputUniverse, channel, value, key);
}

void InputPatch::slotPageChanged(quint32 input, quint32 pagesize, quint32 page)
{
    if (input == m_input)
        emit inputPageChanged(m_inputUniverse, pagesize, page);
}
