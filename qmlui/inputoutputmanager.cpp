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

InputOutputManager::InputOutputManager(QQuickView *view, Doc *doc, QObject *parent)
    : PreviewContext(view, doc, "IOMGR", parent)
    , m_selectedItem(NULL)
    , m_selectedUniverseIndex(-1)
    , m_beatType("INTERNAL")
{
    Q_ASSERT(m_doc != NULL);
    m_ioMap = m_doc->inputOutputMap();
    Q_ASSERT(m_ioMap != NULL);

    setContextResource("qrc:/InputOutputManager.qml");
    setContextTitle(tr("Input/Output Manager"));

    qmlRegisterType<Universe>("org.qlcplus.classes", 1, 0, "Universe");
    qmlRegisterType<InputPatch>("org.qlcplus.classes", 1, 0, "InputPatch");
    qmlRegisterType<OutputPatch>("org.qlcplus.classes", 1, 0, "OutputPatch");

    connect(m_doc, SIGNAL(loaded()), this, SLOT(slotDocLoaded()));
    connect(m_ioMap, SIGNAL(beat()), this, SIGNAL(beat()), Qt::QueuedConnection);
    connect(m_ioMap, SIGNAL(beatGeneratorTypeChanged()), this, SLOT(slotBeatTypeChanged()));
    connect(m_ioMap, SIGNAL(bpmNumberChanged(int)), this, SLOT(slotBpmNumberChanged(int)));

    m_bpmNumber = m_doc->masterTimer()->bpmNumber();
}

void InputOutputManager::slotDocLoaded()
{
    emit universesListModelChanged();
}

/*********************************************************************
 * Universes
 *********************************************************************/

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

/*********************************************************************
 * Audio IO
 *********************************************************************/

QVariant InputOutputManager::audioInputDevice()
{
    QSettings settings;
    QString devName;
    QVariant var = settings.value(SETTINGS_AUDIO_INPUT_DEVICE);
    if (var.isValid() == true)
        devName = var.toString();

    if (var.isValid() == false || devName.isEmpty())
    {
        QVariantMap devMap;
        devMap.insert("name", tr("Default device"));
        devMap.insert("privateName", "__qlcplusdefault__");
        return QVariant::fromValue(devMap);
    }

    QList<AudioDeviceInfo> devList = m_doc->audioPluginCache()->audioDevicesList();
    foreach(AudioDeviceInfo info, devList)
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
    QString devName;
    QVariant var = settings.value(SETTINGS_AUDIO_OUTPUT_DEVICE);
    if (var.isValid() == true)
        devName = var.toString();

    if (var.isValid() == false || devName.isEmpty())
    {
        QVariantMap devMap;
        devMap.insert("name", tr("Default device"));
        devMap.insert("privateName", "__qlcplusdefault__");
        return QVariant::fromValue(devMap);
    }

    QList<AudioDeviceInfo> devList = m_doc->audioPluginCache()->audioDevicesList();
    foreach(AudioDeviceInfo info, devList)
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

    foreach(AudioDeviceInfo info, devList)
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

    foreach(AudioDeviceInfo info, devList)
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

/*********************************************************************
 * IO Patches
 *********************************************************************/

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

void InputOutputManager::setOutputPatch(int universe, QString plugin, QString line, int index)
{
    m_doc->inputOutputMap()->setOutputPatch(universe, plugin, line.toUInt(), false, index);
}

void InputOutputManager::removeOutputPatch(int universe, int index)
{
    m_doc->inputOutputMap()->setOutputPatch(universe, KOutputNone, QLCIOPlugin::invalidLine(), false, index);
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

int InputOutputManager::outputPatchesCount(int universe) const
{
    return m_doc->inputOutputMap()->outputPatchesCount(universe);
}

/*********************************************************************
 * Beats
 *********************************************************************/

QVariant InputOutputManager::beatGeneratorsList()
{
    QVariantList genList;

    // add a default entry to disable the beat system
    QVariantMap disableMap;
    disableMap.insert("type", "OFF");
    disableMap.insert("name", tr("Disabled"));
    disableMap.insert("uni", 0);
    disableMap.insert("line", 0);
    disableMap.insert("privateName", "");
    genList.append(disableMap);

    // add a default entry to enable MasterTimer beats generation
    QVariantMap internalMap;
    internalMap.insert("type", "INTERNAL");
    internalMap.insert("name", tr("Internal generator"));
    internalMap.insert("uni", 0);
    internalMap.insert("line", 0);
    internalMap.insert("privateName", "");
    genList.append(internalMap);

    // add the currently open MIDI input devices
    foreach(Universe *uni, m_ioMap->universes())
    {
        InputPatch *ip = uni->inputPatch();
        if (ip == NULL || ip->pluginName() != "MIDI")
            continue;

        QVariantMap midiInMap;
        midiInMap.insert("type", "MIDI");
        midiInMap.insert("name", ip->inputName());
        midiInMap.insert("uni", uni->id());
        midiInMap.insert("line", ip->input());
        midiInMap.insert("privateName", "");
        genList.append(midiInMap);
    }

    // add the currently selected audio input device
    QSettings settings;
    QString devName;
    QVariant var = settings.value(SETTINGS_AUDIO_INPUT_DEVICE);
    if (var.isValid() == true)
        devName = var.toString();

    if (var.isValid() == false || devName.isEmpty())
    {
        QVariantMap audioInMap;
        audioInMap.insert("type", "AUDIO");
        audioInMap.insert("name", tr("Default device"));
        audioInMap.insert("uni", 0);
        audioInMap.insert("line", 0);
        audioInMap.insert("privateName", "__qlcplusdefault__");
        genList.append(audioInMap);
    }
    else
    {
        QList<AudioDeviceInfo> devList = m_doc->audioPluginCache()->audioDevicesList();
        foreach(AudioDeviceInfo info, devList)
        {
            if (info.capabilities & AUDIO_CAP_INPUT &&
                info.deviceName == devName)
            {
                QVariantMap audioInMap;
                audioInMap.insert("type", "AUDIO");
                audioInMap.insert("name", info.deviceName);
                audioInMap.insert("uni", 0);
                audioInMap.insert("line", 0);
                audioInMap.insert("privateName", info.privateName);
                genList.append(audioInMap);
            }
        }
    }

    return QVariant::fromValue(genList);
}

QString InputOutputManager::beatType() const
{
    return m_beatType;
}

void InputOutputManager::setBeatType(QString beatType)
{
    if (m_beatType == beatType)
        return;

    m_beatType = beatType;

    qDebug() << "[InputOutputManager] Setting beat type:" << m_beatType;

    if (m_beatType == "INTERNAL")
        m_ioMap->setBeatGeneratorType(InputOutputMap::Internal);
    else if (m_beatType == "MIDI")
        m_ioMap->setBeatGeneratorType(InputOutputMap::MIDI);
    else if (m_beatType == "AUDIO")
        m_ioMap->setBeatGeneratorType(InputOutputMap::Audio);
    else
        m_ioMap->setBeatGeneratorType(InputOutputMap::Disabled);

    setBpmNumber(m_ioMap->bpmNumber());

    emit beatTypeChanged(beatType);
}

void InputOutputManager::slotBeatTypeChanged()
{
    switch(m_ioMap->beatGeneratorType())
    {
        case InputOutputMap::Internal: m_beatType = "INTERNAL"; break;
        case InputOutputMap::MIDI: m_beatType = "MIDI"; break;
        case InputOutputMap::Audio: m_beatType = "AUDIO"; break;
        case InputOutputMap::Disabled:
        default:
            m_beatType = "OFF";
        break;
    }
    emit beatTypeChanged(m_beatType);
    emit bpmNumberChanged(m_ioMap->bpmNumber());
}

void InputOutputManager::slotBpmNumberChanged(int bpmNumber)
{
    qDebug() << "[InputOutputManager] BPM changed to:" << bpmNumber;
    if (m_bpmNumber == bpmNumber)
        return;

    m_bpmNumber = bpmNumber;
    emit bpmNumberChanged(bpmNumber);
}

int InputOutputManager::bpmNumber() const
{
    return m_bpmNumber;
}

void InputOutputManager::setBpmNumber(int bpmNumber)
{
    if (m_bpmNumber == bpmNumber)
        return;

    m_bpmNumber = bpmNumber;
    m_ioMap->setBpmNumber(m_bpmNumber);

    emit bpmNumberChanged(bpmNumber);
}


