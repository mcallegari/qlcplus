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

#include <QDebug>

#include "qlcinputchannel.h"
#include "qlcioplugin.h"
#include "inputpatch.h"

#define GRACE_MS 1

/*****************************************************************************
 * Initialization
 *****************************************************************************/

InputPatch::InputPatch(QObject *parent)
    : QObject(parent)
    , m_universe(UINT_MAX)
    , m_plugin(NULL)
    , m_pluginLine(QLCIOPlugin::invalidLine())
    , m_profile(NULL)
    , m_nextPageCh(USHRT_MAX)
    , m_prevPageCh(USHRT_MAX)
    , m_pageSetCh(USHRT_MAX)
{

}

InputPatch::InputPatch(quint32 inputUniverse, QObject* parent)
    : QObject(parent)
    , m_universe(inputUniverse)
    , m_plugin(NULL)
    , m_pluginLine(QLCIOPlugin::invalidLine())
    , m_profile(NULL)
    , m_nextPageCh(USHRT_MAX)
    , m_prevPageCh(USHRT_MAX)
    , m_pageSetCh(USHRT_MAX)
{

}

InputPatch::~InputPatch()
{
    if (m_plugin != NULL)
        m_plugin->closeInput(m_pluginLine, m_universe);
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

bool InputPatch::set(QLCIOPlugin* plugin, quint32 input, QLCInputProfile* profile)
{
    bool result = false;

    qDebug() << "InputPatch::set - plugin:" << ((plugin == NULL)?"None":plugin->name())
             << ", line:" << input
             << ", profile:" << ((profile == NULL)?"None":profile->name());

    if (m_plugin != NULL && m_pluginLine != QLCIOPlugin::invalidLine())
    {
        disconnect(m_plugin, SIGNAL(valueChanged(quint32,quint32,quint32,uchar,QString)),
                   this, SLOT(slotValueChanged(quint32,quint32,quint32,uchar,QString)));
        m_plugin->closeInput(m_pluginLine, m_universe);
    }

    m_plugin = plugin;
    m_pluginLine = input;
    m_profile = profile;

    if (m_plugin != NULL)
    {
        emit pluginNameChanged();
        if (m_pluginLine != QLCIOPlugin::invalidLine())
            emit inputNameChanged();
        if (m_profile != NULL)
            emit profileNameChanged();
    }

    /* Open the assigned plugin input */
    if (m_plugin != NULL && m_pluginLine != QLCIOPlugin::invalidLine())
    {
        connect(m_plugin, SIGNAL(valueChanged(quint32,quint32,quint32,uchar,QString)),
                this, SLOT(slotValueChanged(quint32,quint32,quint32,uchar,QString)));
        result = m_plugin->openInput(m_pluginLine, m_universe);

        if (m_profile != NULL)
            setProfilePageControls();
    }
    return result;
}

bool InputPatch::set(QLCInputProfile *profile)
{
    if (m_plugin == NULL || m_pluginLine == QLCIOPlugin::invalidLine())
        return false;

    m_profile = profile;

    if (m_profile != NULL)
        setProfilePageControls();

    emit profileNameChanged();

    return true;
}

bool InputPatch::reconnect()
{
    if (m_plugin != NULL && m_pluginLine != QLCIOPlugin::invalidLine())
    {
        m_plugin->closeInput(m_pluginLine, m_universe);
#if defined(WIN32) || defined(Q_OS_WIN)
        Sleep(GRACE_MS);
#else
        usleep(GRACE_MS * 1000);
#endif
        bool ret = m_plugin->openInput(m_pluginLine, m_universe);
        if (ret == true)
        {
            foreach (QString par, m_parametersCache.keys())
            {
                qDebug() << "[InputPatch] restoring parameter:" << par << m_parametersCache[par];
                m_plugin->setParameter(m_universe, m_pluginLine, QLCIOPlugin::Input, par, m_parametersCache[par]);
            }
        }
        return ret;
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
    return m_pluginLine;
}

QString InputPatch::inputName() const
{
    if (m_plugin != NULL && m_pluginLine != QLCIOPlugin::invalidLine() &&
            m_pluginLine < quint32(m_plugin->inputs().count()))
        return m_plugin->inputs()[m_pluginLine];
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

void InputPatch::setPluginParameter(QString prop, QVariant value)
{
    qDebug() << "[InputPatch] caching parameter:" << prop << value;
    m_parametersCache[prop] = value;
    if (m_plugin != NULL)
        m_plugin->setParameter(m_universe, m_pluginLine, QLCIOPlugin::Input, prop, value);
}

QMap<QString, QVariant> InputPatch::getPluginParameters()
{
    if (m_plugin != NULL)
        return m_plugin->getParameters(m_universe, m_pluginLine, QLCIOPlugin::Input);

    return QMap<QString, QVariant>();
}

void InputPatch::slotValueChanged(quint32 universe, quint32 input, quint32 channel,
                                  uchar value, const QString& key)
{
    // In case we have several lines connected to the same plugin, emit only
    // such values that belong to this particular patch.
    if (input == m_pluginLine)
    {
        if (universe == UINT_MAX || universe == m_universe)
        {
            QMutexLocker inputBufferLocker(&m_inputBufferMutex);
            InputValue val(value, key);
            if (m_inputBuffer.contains(channel))
            {
                InputValue const& curVal = m_inputBuffer.value(channel);
                if (curVal.value != val.value)
                {
                    // Every ON/OFF changes must pass through
                    if (curVal.value == 0 || val.value == 0)
                    {
                        emit inputValueChanged(m_universe, channel, curVal.value, curVal.key);
                    }
                    m_inputBuffer.insert(channel, val);
                }
            }
            else
            {
                m_inputBuffer.insert(channel, val);
            }
        }
    }
}

void InputPatch::setProfilePageControls()
{
    if (m_profile != NULL)
    {
        if (m_plugin != NULL)
        {
            QMap<QString, QVariant> settings = m_profile->globalSettings();
            if (settings.isEmpty() == false)
            {
                QMapIterator <QString,QVariant> it(settings);
                while (it.hasNext() == true)
                {
                    it.next();
                    m_plugin->setParameter(m_universe, m_pluginLine, QLCIOPlugin::Input, it.key(), it.value());
                }
            }
        }
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

void InputPatch::flush(quint32 universe)
{
    if (universe == UINT_MAX || universe == m_universe)
    {
        QMutexLocker inputBufferLocker(&m_inputBufferMutex);
        for (QHash<quint32, InputValue>::const_iterator it = m_inputBuffer.begin(); it != m_inputBuffer.end(); ++it)
        {
            emit inputValueChanged(m_universe, it.key(), it.value().value, it.value().key);
        }
        m_inputBuffer.clear();
    }
}
