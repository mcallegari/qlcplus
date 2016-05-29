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

#include <QSettings>
#include <QDebug>

#include "inputoutputmanager.h"
#include "audiorenderer_qt.h"
#include "audiocapture_qt.h"
#include "audioplugincache.h"
#include "qlcioplugin.h"
#include "outputpatch.h"
#include "inputpatch.h"
#include "universe.h"
#include "doc.h"

InputOutputManager::InputOutputManager(Doc *doc, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
{
    Q_ASSERT(m_doc != NULL);
    m_ioMap = m_doc->inputOutputMap();
    m_selectedItem = NULL;

    qmlRegisterType<Universe>("com.qlcplus.classes", 1, 0, "Universe");
    qmlRegisterType<InputPatch>("com.qlcplus.classes", 1, 0, "InputPatch");
    qmlRegisterType<OutputPatch>("com.qlcplus.classes", 1, 0, "OutputPatch");

    connect(m_doc, SIGNAL(loaded()),
            this, SLOT(slotDocLoaded()));
}

QQmlListProperty<Universe> InputOutputManager::universes()
{
    m_selectedItem = NULL;
    m_universeList.clear();
    m_universeList = m_ioMap->universes();
    return QQmlListProperty<Universe>(this, m_universeList);
}

QStringList InputOutputManager::universeNames() const
{
    return m_ioMap->universeNames();
}

QVariant InputOutputManager::universesListModel() const
{
    QVariantList universesList;

    QVariantMap allMap;
    allMap.insert("mLabel", tr("All universes"));
    allMap.insert("mValue", (int)Universe::invalid());
    universesList.append(allMap);

    foreach(Universe *uni, m_ioMap->universes())
    {
        QVariantMap uniMap;
        uniMap.insert("mLabel", uni->name());
        uniMap.insert("mValue", uni->id());
        universesList.append(uniMap);
    }

    return QVariant::fromValue(universesList);
}

QVariant InputOutputManager::audioInputDevice()
{
    QSettings settings;
    QString devName = tr("Default device");
    QVariant var = settings.value(SETTINGS_AUDIO_INPUT_DEVICE);
    if (var.isValid() == true)
        devName = var.toString();

    if (var.isValid() == false || devName == tr("Default device"))
    {
        QVariantMap devMap;
        devMap.insert("name", tr("Default device"));
        devMap.insert("privateName", "__qlcplusdefault__");
        return QVariant::fromValue(devMap);
    }

    QList<AudioDeviceInfo> devList = m_doc->audioPluginCache()->audioDevicesList();
    foreach( AudioDeviceInfo info, devList)
    {
        if (info.capabilities & AUDIO_CAP_INPUT &&
            info.deviceName == devName)
        {
            QVariantMap devMap;
            devMap.insert("name", info.deviceName);
            devMap.insert("privateName", info.privateName);
            return QVariant::fromValue(devMap);
        }
    }

    return QVariant();
}

QVariant InputOutputManager::audioOutputDevice()
{
    QSettings settings;
    QString devName = tr("Default device");
    QVariant var = settings.value(SETTINGS_AUDIO_OUTPUT_DEVICE);
    if (var.isValid() == true)
        devName = var.toString();

    if (var.isValid() == false || devName == tr("Default device"))
    {
        QVariantMap devMap;
        devMap.insert("name", tr("Default device"));
        devMap.insert("privateName", "__qlcplusdefault__");
        return QVariant::fromValue(devMap);
    }

    QList<AudioDeviceInfo> devList = m_doc->audioPluginCache()->audioDevicesList();
    foreach( AudioDeviceInfo info, devList)
    {
        if (info.capabilities & AUDIO_CAP_OUTPUT &&
            info.deviceName == devName)
        {
            QVariantMap devMap;
            devMap.insert("name", info.deviceName);
            devMap.insert("privateName", info.privateName);
            return QVariant::fromValue(devMap);
        }
    }

    return QVariant();
}

QVariant InputOutputManager::audioInputSources()
{
    QVariantList inputSources;
    QList<AudioDeviceInfo> devList = m_doc->audioPluginCache()->audioDevicesList();

    QVariantMap defAudioMap;
    defAudioMap.insert("name", tr("Default device"));
    defAudioMap.insert("privateName", "__qlcplusdefault__");
    inputSources.append(defAudioMap);

    foreach( AudioDeviceInfo info, devList)
    {
        if (info.capabilities & AUDIO_CAP_INPUT)
        {
            QVariantMap devMap;
            devMap.insert("name", info.deviceName);
            devMap.insert("privateName", info.privateName);
            inputSources.append(devMap);
        }
    }

    return QVariant::fromValue(inputSources);
}

QVariant InputOutputManager::audioOutputSources()
{
    QVariantList outputSources;
    QList<AudioDeviceInfo> devList = m_doc->audioPluginCache()->audioDevicesList();

    QVariantMap defAudioMap;
    defAudioMap.insert("name", tr("Default device"));
    defAudioMap.insert("privateName", "__qlcplusdefault__");
    outputSources.append(defAudioMap);

    foreach( AudioDeviceInfo info, devList)
    {
        if (info.capabilities & AUDIO_CAP_OUTPUT)
        {
            QVariantMap devMap;
            devMap.insert("name", info.deviceName);
            devMap.insert("privateName", info.privateName);
            outputSources.append(devMap);
        }
    }

    return QVariant::fromValue(outputSources);
}

QVariant InputOutputManager::universeInputSources(int universe)
{
    QVariantList inputSources;
    QString currPlugin;
    int currLine = -1;
    InputPatch *ip = m_ioMap->inputPatch(universe);
    if (ip != NULL)
    {
        currPlugin = ip->pluginName();
        currLine = ip->input();
    }

    foreach(QString pluginName,  m_ioMap->inputPluginNames())
    {
        QLCIOPlugin *plugin = m_doc->ioPluginCache()->plugin(pluginName);
        int i = 0;
        foreach(QString pLine, m_ioMap->pluginInputs(pluginName))
        {
            if (pluginName == currPlugin && i == currLine)
            {
                i++;
                continue;
            }
            quint32 uni = m_ioMap->inputMapping(pluginName, i);
            if (uni == InputOutputMap::invalidUniverse() ||
               (uni == (quint32)universe || plugin->capabilities() & QLCIOPlugin::Infinite))
            {
                QVariantMap lineMap;
                lineMap.insert("universe", universe);
                lineMap.insert("name", pLine);
                lineMap.insert("line", i);
                lineMap.insert("plugin", pluginName);
                inputSources.append(lineMap);
            }
            i++;
        }
    }

    return QVariant::fromValue(inputSources);
}

QVariant InputOutputManager::universeOutputSources(int universe)
{
    QVariantList outputSources;
    QString currPlugin;
    int currLine = -1;
    OutputPatch *op = m_ioMap->outputPatch(universe);
    if (op != NULL)
    {
        currPlugin = op->pluginName();
        currLine = op->output();
    }

    foreach(QString pluginName,  m_ioMap->outputPluginNames())
    {
        QLCIOPlugin *plugin = m_doc->ioPluginCache()->plugin(pluginName);
        int i = 0;
        foreach(QString pLine, m_ioMap->pluginOutputs(pluginName))
        {
            if (pluginName == currPlugin && i == currLine)
            {
                i++;
                continue;
            }
            quint32 uni = m_ioMap->outputMapping(pluginName, i);
            if (uni == InputOutputMap::invalidUniverse() ||
               (uni == (quint32)universe || plugin->capabilities() & QLCIOPlugin::Infinite))
            {
                QVariantMap lineMap;
                lineMap.insert("universe", universe);
                lineMap.insert("name", pLine);
                lineMap.insert("line", i);
                lineMap.insert("plugin", pluginName);
                outputSources.append(lineMap);
            }
            i++;
        }
    }

    return QVariant::fromValue(outputSources);
}

QVariant InputOutputManager::universeInputProfiles(int universe)
{
    QVariantList profilesList;
    QString currentProfile = KInputNone;
    QStringList profileNames = m_doc->inputOutputMap()->profileNames();
    profileNames.sort();

    if (m_ioMap->inputPatch(universe) != NULL)
        currentProfile = m_ioMap->inputPatch(universe)->profileName();

    foreach(QString name, profileNames)
    {
        QLCInputProfile *ip = m_doc->inputOutputMap()->profile(name);
        if (ip != NULL)
        {
            QString type = ip->typeToString(ip->type());
            if (name != currentProfile)
            {
                QVariantMap profileMap;
                profileMap.insert("universe", universe);
                profileMap.insert("name", name);
                profileMap.insert("line", name);
                profileMap.insert("plugin", type);
                profilesList.append(profileMap);
            }
        }
    }

    return QVariant::fromValue(profilesList);
}

void InputOutputManager::addOutputPatch(int universe, QString plugin, QString line)
{
    m_doc->inputOutputMap()->setOutputPatch(universe, plugin, line.toUInt(), false);
}

void InputOutputManager::removeOutputPatch(int universe)
{
    m_doc->inputOutputMap()->setOutputPatch(universe, KOutputNone, QLCIOPlugin::invalidLine(), false);
}

void InputOutputManager::addInputPatch(int universe, QString plugin, QString line)
{
    m_doc->inputOutputMap()->setInputPatch(universe, plugin, line.toUInt());
}

void InputOutputManager::removeInputPatch(int universe)
{
    m_doc->inputOutputMap()->setInputPatch(universe, KInputNone, QLCIOPlugin::invalidLine());
}

void InputOutputManager::setInputProfile(int universe, QString profileName)
{
    m_doc->inputOutputMap()->setInputProfile(universe, profileName);
}

void InputOutputManager::setSelectedItem(QQuickItem *item, int index)
{
    if (m_selectedItem != NULL)
    {
        m_selectedItem->setProperty("isSelected", false);
        m_selectedItem->setProperty("z", 1);
    }

    m_selectedItem = item;
    m_selectedUniverseIndex = index;
    m_selectedItem->setProperty("z", 5);

    qDebug() << "[InputOutputManager] Selected universe:" << index;
}

void InputOutputManager::slotDocLoaded()
{
    emit universesListModelChanged();
}


