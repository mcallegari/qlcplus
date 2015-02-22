/*
  Q Light Controller Plus
  inputoutputmanager.cpp

  Copyright (c) Massimo Callegari

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

#include <QDebug>

#include "inputoutputmanager.h"
#include "inputoutputobject.h"
#include "audiorenderer_qt.h"
#include "audiocapture_qt.h"
#include "outputpatch.h"
#include "inputpatch.h"
#include "doc.h"

InputOutputManager::InputOutputManager(Doc *doc, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
{
    Q_ASSERT(m_doc != NULL);
    m_ioMap = m_doc->inputOutputMap();
    m_selectedItem = NULL;
}

QStringList InputOutputManager::universes()
{
    return m_ioMap->universeNames();
}

void InputOutputManager::clearInputList()
{
    int count = m_inputSources.count();
    for (int i = 0; i < count; i++)
    {
        QObject *src = m_inputSources.takeLast();
        delete src;
    }
    m_inputSources.clear();
}

void InputOutputManager::clearOutputList()
{
    int count = m_outputSources.count();
    for (int i = 0; i < count; i++)
    {
        QObject *src = m_outputSources.takeLast();
        delete src;
    }
    m_outputSources.clear();
}

QVariant InputOutputManager::audioInputSources()
{
    QList<AudioDeviceInfo> devList = AudioRendererQt::getDevicesInfo();

    clearInputList();

    m_inputSources.append(new InputOutputObject(tr("Default device"), "__qlcplusdefault__", ""));

    foreach( AudioDeviceInfo info, devList)
    {
        if (info.capabilities & AUDIO_CAP_INPUT)
            m_inputSources.append(new InputOutputObject(info.deviceName, info.privateName, ""));
    }

    return QVariant::fromValue(m_inputSources);
}

QVariant InputOutputManager::audioOutputSources()
{
    QList<AudioDeviceInfo> devList = AudioRendererQt::getDevicesInfo();

    clearOutputList();

    m_outputSources.append(new InputOutputObject(tr("Default device"), "__qlcplusdefault__", ""));

    foreach( AudioDeviceInfo info, devList)
    {
        if (info.capabilities & AUDIO_CAP_OUTPUT)
            m_outputSources.append(new InputOutputObject(info.deviceName, info.privateName, ""));
    }

    return QVariant::fromValue(m_outputSources);
}

QVariant InputOutputManager::universeInputSources(int universe)
{
    clearInputList();
    QString currPlugin;
    int currLine;
    InputPatch *ip = m_ioMap->inputPatch(universe);
    if (ip != NULL)
    {
        currPlugin = ip->pluginName();
        currLine = ip->input();
    }

    foreach(QString pluginName,  m_ioMap->inputPluginNames())
    {
        //QLCIOPlugin *plugin = m_doc->ioPluginCache()->plugin(pluginName);
        int i = 0;
        foreach(QString pLine, m_ioMap->pluginInputs(pluginName))
        {
            if (pluginName != currPlugin || i != currLine)
                m_inputSources.append(new InputOutputObject(pLine, QString::number(i), pluginName));
            i++;
        }
    }

    return QVariant::fromValue(m_inputSources);
}

QVariant InputOutputManager::universeOutputSources(int universe)
{
    clearOutputList();
    QString currPlugin;
    int currLine;
    OutputPatch *op = m_ioMap->outputPatch(universe);
    if (op != NULL)
    {
        currPlugin = op->pluginName();
        currLine = op->output();
    }

    foreach(QString pluginName,  m_ioMap->outputPluginNames())
    {
        //QLCIOPlugin *plugin = m_doc->ioPluginCache()->plugin(pluginName);
        int i = 0;
        foreach(QString pLine, m_ioMap->pluginOutputs(pluginName))
        {
            if (pluginName != currPlugin || i != currLine)
                m_outputSources.append(new InputOutputObject(pLine, QString::number(i), pluginName));
            i++;
        }
    }

    return QVariant::fromValue(m_outputSources);
}

QVariant InputOutputManager::universeInputProfiles(int universe)
{
    QStringList profileNames = m_doc->inputOutputMap()->profileNames();
    QString currentProfile = KInputNone;
    if (m_ioMap->inputPatch(universe) != NULL)
        currentProfile = m_ioMap->inputPatch(universe)->profileName();

    foreach(QString name, profileNames)
    {
        if (name != currentProfile)
            m_inputProfiles.append(new InputOutputObject(name, name, ""));
    }

    return QVariant::fromValue(m_inputProfiles);
}

void InputOutputManager::setSelectedItem(QQuickItem *item, int index)
{
    if (m_selectedItem != NULL)
    {
        m_selectedItem->setProperty("isSelected", false);
    }
    m_selectedItem = item;
    m_selectedUniverseIndex = index;

    qDebug() << "Selected universe:" << index;
}


