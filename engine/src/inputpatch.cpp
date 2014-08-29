/*
  Q Light Controller
  inputpatch.cpp

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
#   include <Windows.h>
#else
#   include <unistd.h>
#endif

#include <QObject>
#include <QDebug>
#include <QtXml>

#include "qlcinputchannel.h"
#include "qlcioplugin.h"
#include "inputpatch.h"

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
    , m_nextPageCh(USHRT_MAX)
    , m_prevPageCh(USHRT_MAX)
    , m_pageSetCh(USHRT_MAX)
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

bool InputPatch::set(QLCIOPlugin* plugin, quint32 input, QLCInputProfile* profile)
{
    bool result = false;

    if (m_plugin != NULL && m_input != QLCIOPlugin::invalidLine())
    {
        disconnect(m_plugin, SIGNAL(valueChanged(quint32,quint32,quint32,uchar,QString)),
                   this, SLOT(slotValueChanged(quint32,quint32,quint32,uchar,QString)));
        m_plugin->closeInput(m_input);
    }

    m_plugin = plugin;
    m_input = input;
    m_profile = profile;

    /* Open the assigned plugin input */
    if (m_plugin != NULL && m_input != QLCIOPlugin::invalidLine())
    {
        connect(m_plugin, SIGNAL(valueChanged(quint32,quint32,quint32,uchar,QString)),
                this, SLOT(slotValueChanged(quint32,quint32,quint32,uchar,QString)));
        result = m_plugin->openInput(m_input);

        if (m_profile != NULL)
        {
            QMapIterator <quint32,QLCInputChannel*> it(m_profile->channels());
            while (it.hasNext() == true)
            {
                it.next();
                QLCInputChannel *ch = it.value();
                if (ch != NULL)
                {
                    if (m_nextPageCh == USHRT_MAX && ch->type() == QLCInputChannel::NextPage)
                        m_nextPageCh = m_profile->channelNumber(ch);
                    else if (m_prevPageCh == USHRT_MAX && ch->type() == QLCInputChannel::PrevPage)
                        m_prevPageCh = m_profile->channelNumber(ch);
                    else if (m_pageSetCh == USHRT_MAX && ch->type() == QLCInputChannel::PageSet)
                        m_pageSetCh = m_profile->channelNumber(ch);
                }
            }
        }
    }
    return result;
}

bool InputPatch::reconnect()
{
    if (m_plugin != NULL && m_input != QLCIOPlugin::invalidLine())
    {
        m_plugin->closeInput(m_input);
#if defined(WIN32) || defined(Q_OS_WIN)
        Sleep(GRACE_MS);
#else
        usleep(GRACE_MS * 1000);
#endif
        return m_plugin->openInput(m_input);
    }
    return false;
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

bool InputPatch::isPatched() const
{
    return input() != QLCIOPlugin::invalidLine();
}
 
void InputPatch::slotValueChanged(quint32 universe, quint32 input, quint32 channel,
                                  uchar value, const QString& key)
{
    // In case we have several lines connected to the same plugin, emit only
    // such values that belong to this particular patch.
    if (input == m_input)
    {
        if (universe == UINT_MAX || (universe != UINT_MAX && universe == m_inputUniverse))
        emit inputValueChanged(m_inputUniverse, channel, value, key);
    }
}
