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

#include <QQmlContext>
#include <QSettings>
#include <QDebug>

#include "inputoutputmanager.h"
#include "inputprofileeditor.h"
#include "monitorproperties.h"
#include "audioplugincache.h"
#include "qlcioplugin.h"
#include "outputpatch.h"
#include "inputpatch.h"
#include "universe.h"
#include "qlcfile.h"
#include "tardis.h"
#include "doc.h"

InputOutputManager::InputOutputManager(QQuickView *view, Doc *doc, QObject *parent)
    : PreviewContext(view, doc, "IOMGR", parent)
    , m_selectedUniverseIndex(-1)
    , m_blackout(false)
    , m_profileEditor(nullptr)
    , m_editProfile(nullptr)
    , m_beatType("INTERNAL")
{
    Q_ASSERT(m_doc != nullptr);
    m_ioMap = m_doc->inputOutputMap();
    Q_ASSERT(m_ioMap != nullptr);

    setContextResource("qrc:/InputOutputManager.qml");
    setContextTitle(tr("Input/Output Manager"));

    view->rootContext()->setContextProperty("ioManager", this);
    qmlRegisterType<Universe>("org.qlcplus.classes", 1, 0, "Universe");
    qmlRegisterType<InputPatch>("org.qlcplus.classes", 1, 0, "InputPatch");
    qmlRegisterType<OutputPatch>("org.qlcplus.classes", 1, 0, "OutputPatch");
    qmlRegisterUncreatableType<QLCInputProfile>("org.qlcplus.classes", 1, 0, "QLCInputProfile", "Can't create a QLCInputProfile!");
    qmlRegisterUncreatableType<InputProfileEditor>("org.qlcplus.classes", 1, 0, "InputProfEditor", "Can't create a InputProfileEditor!");

    connect(m_doc, SIGNAL(loaded()), this, SLOT(slotDocLoaded()));
    connect(m_ioMap, SIGNAL(universeAdded(quint32)), this, SIGNAL(universesListModelChanged()));
    connect(m_ioMap, SIGNAL(universeAdded(quint32)), this, SIGNAL(universeNamesChanged()));
    connect(m_ioMap, SIGNAL(universeRemoved(quint32)), this, SIGNAL(universesListModelChanged()));
    connect(m_ioMap, SIGNAL(universeRemoved(quint32)), this, SIGNAL(universeNamesChanged()));
    connect(m_ioMap, SIGNAL(beat()), this, SIGNAL(beat()), Qt::QueuedConnection);
    connect(m_ioMap, SIGNAL(beatGeneratorTypeChanged()), this, SLOT(slotBeatTypeChanged()));
    connect(m_ioMap, SIGNAL(bpmNumberChanged(int)), this, SIGNAL(bpmNumberChanged(int)));
}

void InputOutputManager::slotDocLoaded()
{
    emit universesListModelChanged();
}

/*********************************************************************
 * Universes
 *********************************************************************/

QVariant InputOutputManager::universes()
{
    QVariantList universesList;

    for (Universe *uni : m_ioMap->universes())
    {
        QVariantMap uniMap;
        uniMap.insert("classRef", QVariant::fromValue(uni));
        universesList.append(uniMap);
    }

    return QVariant::fromValue(universesList);
}

QStringList InputOutputManager::universeNames() const
{
    return m_ioMap->universeNames();
}

QString InputOutputManager::universeName(quint32 universeId)
{
    if (universeId == Universe::invalid())
        return tr("All universes");
    else
    {
        Universe *uni = m_ioMap->universe(universeId);
        if (uni != nullptr)
            return uni->name();
    }

    return QString();
}

QVariant InputOutputManager::universesListModel() const
{
    QVariantList universesList;

    QVariantMap allMap;
    allMap.insert("mLabel", tr("All universes"));
    allMap.insert("mValue", (int)Universe::invalid());
    universesList.append(allMap);

    for (Universe *uni : m_ioMap->universes())
    {
        QVariantMap uniMap;
        uniMap.insert("mLabel", uni->name());
        uniMap.insert("mValue", uni->id());
        universesList.append(uniMap);
    }

    return QVariant::fromValue(universesList);
}

int InputOutputManager::selectedIndex() const
{
    return m_selectedUniverseIndex;
}

void InputOutputManager::setSelectedIndex(int index)
{
    if (index == m_selectedUniverseIndex)
        return;

    m_selectedUniverseIndex = index;

    emit selectedIndexChanged();
    emit inputCanConfigureChanged();
    emit outputCanConfigureChanged();
}

void InputOutputManager::addUniverse()
{
    m_ioMap->addUniverse();
    m_ioMap->startUniverses();

    quint32 uniID = m_ioMap->universes().last()->id();
    Tardis::instance()->enqueueAction(Tardis::IOAddUniverse, uniID, QVariant(),
                                      Tardis::instance()->actionToByteArray(Tardis::IOAddUniverse, uniID));

    emit universesChanged();
    emit universeNamesChanged();
}

void InputOutputManager::removeLastUniverse()
{
    if (m_selectedUniverseIndex < 0)
        return;

    int index = m_selectedUniverseIndex;

    m_selectedUniverseIndex = -1;
    emit selectedIndexChanged();

    // Check if the universe is patched
    if (m_ioMap->isUniversePatched(index) == true)
    {
        // Show popup ?
    }

    // Check if there are fixtures using this universe
    quint32 uniID = m_ioMap->getUniverseID(index);
    if (uniID == m_ioMap->invalidUniverse())
        return;

    MonitorProperties *mProps = m_doc->monitorProperties();

    for (Fixture *fixture : m_doc->fixtures())
    {
        if (fixture->universe() != uniID)
            continue;

        for (quint32 subID : mProps->fixtureIDList(fixture->id()))
        {
            quint16 headIndex = mProps->fixtureHeadIndex(subID);
            quint16 linkedIndex = mProps->fixtureLinkedIndex(subID);

            // delete the fixture monitor properties
            Tardis::instance()->enqueueAction(Tardis::FixtureSetPosition, fixture->id(),
                                              QVariant(mProps->fixturePosition(fixture->id(), headIndex, linkedIndex)),
                                              QVariant());
        }
        // delete the fixture
        Tardis::instance()->enqueueAction(Tardis::FixtureDelete, fixture->id(),
                                          Tardis::instance()->actionToByteArray(Tardis::FixtureDelete, fixture->id()),
                                          QVariant());
        m_doc->deleteFixture(fixture->id());
        mProps->removeFixture(fixture->id());
    }

    Tardis::instance()->enqueueAction(Tardis::IORemoveUniverse, index,
                                      Tardis::instance()->actionToByteArray(Tardis::IORemoveUniverse, index),
                                      QVariant());

    m_ioMap->removeUniverse(index);

    emit universesChanged();
    emit universeNamesChanged();
}

int InputOutputManager::universesCount()
{
    return m_ioMap->universesCount();
}

bool InputOutputManager::blackout() const
{
    return m_blackout;
}

void InputOutputManager::setBlackout(bool blackout)
{
    if (m_blackout == blackout)
        return;

    m_blackout = blackout;
    m_ioMap->setBlackout(blackout);

    emit blackoutChanged(m_blackout);
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
    foreach (AudioDeviceInfo info, devList)
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
    foreach (AudioDeviceInfo info, devList)
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

QVariant InputOutputManager::audioInputSources() const
{
    QSettings settings;
    QVariantList inputSources;
    QList<AudioDeviceInfo> devList = m_doc->audioPluginCache()->audioDevicesList();
    QString currDevice = settings.value(SETTINGS_AUDIO_INPUT_DEVICE).toString();

    QVariantMap defAudioMap;
    defAudioMap.insert("mLabel", tr("Default device"));
    defAudioMap.insert("mValue", -1);
    defAudioMap.insert("privateName", "__qlcplusdefault__");
    inputSources.append(defAudioMap);

    int i = 0;
    for (AudioDeviceInfo &info : devList)
    {
        if (info.capabilities & AUDIO_CAP_INPUT)
        {
            if (info.privateName == currDevice)
                continue;

            QVariantMap devMap;
            devMap.insert("mLabel", info.deviceName);
            devMap.insert("mValue", i);
            devMap.insert("privateName", info.privateName);
            inputSources.append(devMap);
        }
        i++;
    }

    return QVariant::fromValue(inputSources);
}

QVariant InputOutputManager::audioOutputSources() const
{
    QSettings settings;
    QVariantList outputSources;
    QList<AudioDeviceInfo> devList = m_doc->audioPluginCache()->audioDevicesList();
    QString currDevice = settings.value(SETTINGS_AUDIO_OUTPUT_DEVICE).toString();

    QVariantMap defAudioMap;
    defAudioMap.insert("mLabel", tr("Default device"));
    defAudioMap.insert("mValue", -1);
    defAudioMap.insert("privateName", "__qlcplusdefault__");
    outputSources.append(defAudioMap);

    int i = 0;
    for (AudioDeviceInfo &info : devList)
    {
        if (info.capabilities & AUDIO_CAP_OUTPUT)
        {
            if (info.privateName == currDevice)
                continue;

            QVariantMap devMap;
            devMap.insert("mLabel", info.deviceName);
            devMap.insert("mValue", i);
            devMap.insert("privateName", info.privateName);
            outputSources.append(devMap);
        }
        i++;
    }

    return QVariant::fromValue(outputSources);
}

void InputOutputManager::setAudioInput(QString privateName)
{
    QSettings settings;
    if (privateName == "__qlcplusdefault__")
        settings.remove(SETTINGS_AUDIO_INPUT_DEVICE);
    else
        settings.setValue(SETTINGS_AUDIO_INPUT_DEVICE, privateName);
    m_doc->destroyAudioCapture();
    emit audioInputSourcesChanged();
    emit audioInputDeviceChanged();
}

void InputOutputManager::setAudioOutput(QString privateName)
{
    QSettings settings;
    if (privateName == "__qlcplusdefault__")
        settings.remove(SETTINGS_AUDIO_OUTPUT_DEVICE);
    else
        settings.setValue(SETTINGS_AUDIO_OUTPUT_DEVICE, privateName);
    emit audioOutputSourcesChanged();
    emit audioOutputDeviceChanged();
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
    if (ip != nullptr)
    {
        currPlugin = ip->pluginName();
        currLine = ip->input();
    }

    foreach (QString pluginName,  m_ioMap->inputPluginNames())
    {
        QLCIOPlugin *plugin = m_doc->ioPluginCache()->plugin(pluginName);
        int i = 0;
        foreach (QString pLine, m_ioMap->pluginInputs(pluginName))
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
    if (op != nullptr)
    {
        currPlugin = op->pluginName();
        currLine = op->output();
    }

    foreach (QString pluginName,  m_ioMap->outputPluginNames())
    {
        QLCIOPlugin *plugin = m_doc->ioPluginCache()->plugin(pluginName);
        int i = 0;
        foreach (QString pLine, m_ioMap->pluginOutputs(pluginName))
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

void InputOutputManager::setOutputPatch(int universe, QString plugin, QString line, int index)
{
    m_ioMap->setOutputPatch(universe, plugin, "", line.toUInt(), false, index);
    emit outputCanConfigureChanged();
}

void InputOutputManager::removeOutputPatch(int universe, int index)
{
    m_ioMap->setOutputPatch(universe, KOutputNone, "", QLCIOPlugin::invalidLine(), false, index);
    emit outputCanConfigureChanged();
}

void InputOutputManager::addInputPatch(int universe, QString plugin, QString line)
{
    m_ioMap->setInputPatch(universe, plugin, "", line.toUInt());
    emit inputCanConfigureChanged();
}

void InputOutputManager::setFeedbackPatch(int universe, bool enable)
{
    InputPatch *patch = m_ioMap->inputPatch(universe);

    if (patch == nullptr)
        return;

    if (enable)
        m_ioMap->setOutputPatch(universe, patch->pluginName(), "", patch->input(), true);
    else
        m_ioMap->setOutputPatch(universe, KInputNone, "", QLCIOPlugin::invalidLine(), true);
}

void InputOutputManager::removeInputPatch(int universe)
{
    m_ioMap->setInputPatch(universe, KInputNone, "", QLCIOPlugin::invalidLine());
    emit inputCanConfigureChanged();
}

void InputOutputManager::setInputProfile(int universe, QString profileName)
{
    m_ioMap->setInputProfile(universe, profileName);
}

void InputOutputManager::configurePlugin(bool input)
{
    if (m_selectedUniverseIndex == -1)
        return;

    QLCIOPlugin *plugin = nullptr;

    if (input)
    {
        InputPatch *patch = m_ioMap->inputPatch(m_selectedUniverseIndex);

        if (patch == nullptr || patch->plugin() == nullptr)
            return;
        plugin = patch->plugin();
    }
    else
    {
        OutputPatch *patch = m_ioMap->outputPatch(m_selectedUniverseIndex);

        if (patch == nullptr || patch->plugin() == nullptr)
            return;
        plugin = patch->plugin();
    }

    if (plugin)
        m_ioMap->configurePlugin(plugin->name());
}

bool InputOutputManager::inputCanConfigure() const
{
    if (m_selectedUniverseIndex == -1)
        return false;

    InputPatch *patch = m_ioMap->inputPatch(m_selectedUniverseIndex);

    if (patch == nullptr || patch->plugin() == nullptr)
        return false;

    return patch->plugin()->canConfigure();
}

bool InputOutputManager::outputCanConfigure() const
{
    if (m_selectedUniverseIndex == -1)
        return false;

    OutputPatch *patch = m_ioMap->outputPatch(m_selectedUniverseIndex);

    if (patch == nullptr || patch->plugin() == nullptr)
        return false;

    return patch->plugin()->canConfigure();
}

int InputOutputManager::outputPatchesCount(int universe) const
{
    return m_ioMap->outputPatchesCount(universe);
}

/*********************************************************************
 * Input Profiles
 *********************************************************************/

QString InputOutputManager::profileUserFolder()
{
    return m_ioMap->userProfileDirectory().absolutePath();
}

void InputOutputManager::createInputProfile()
{
    if (m_editProfile != nullptr)
        delete m_editProfile;

    m_editProfile = new QLCInputProfile();

    if (m_profileEditor == nullptr)
    {
        m_profileEditor = new InputProfileEditor(m_editProfile, m_doc);
        view()->rootContext()->setContextProperty("profileEditor", m_profileEditor);
    }
}

bool InputOutputManager::editInputProfile(QString name)
{
    QLCInputProfile *ip = m_ioMap->profile(name);
    if (ip == nullptr)
        return false;

    // create a copy first
    if (m_editProfile != nullptr)
        delete m_editProfile;

    m_editProfile = ip->createCopy();

    qDebug() << "Profile TYPE:" << m_editProfile->type();

    if (m_profileEditor == nullptr)
    {
        m_profileEditor = new InputProfileEditor(m_editProfile, m_doc);
        view()->rootContext()->setContextProperty("profileEditor", m_profileEditor);
    }

    qDebug() << "Edit profile" << ip->path();

    return true;
}

bool InputOutputManager::saveInputProfile()
{
    if (m_editProfile == nullptr)
        return false;

    QDir dir(InputOutputMap::userProfileDirectory());
    QString absPath = QString("%1/%2-%3%4").arg(dir.absolutePath())
                       .arg(m_editProfile->manufacturer())
                       .arg(m_editProfile->model())
                       .arg(KExtInputProfile);

    bool profileExists = QFileInfo::exists(absPath);

    m_editProfile->saveXML(absPath);
    m_profileEditor->setModified(false);

    if (profileExists == false)
        m_doc->inputOutputMap()->addProfile(m_editProfile);

    return true;
}

void InputOutputManager::finishInputProfile()
{
    if (m_editProfile != nullptr)
    {
        delete m_editProfile;
        m_editProfile = nullptr;
    }

    if (m_profileEditor != nullptr)
    {
        view()->rootContext()->setContextProperty("profileEditor", nullptr);
        delete m_profileEditor;
        m_profileEditor = nullptr;
    }
}

bool InputOutputManager::removeInputProfile(QString name)
{
    QLCInputProfile *profile = m_ioMap->profile(name);
    if (profile == nullptr)
        return false;

    QFile file(profile->path());
    if (file.remove() == true)
    {
        m_ioMap->removeProfile(name);
        return true;
    }

    qDebug() << "Failed to remove input profile" << profile->path();

    return false;
}

QVariant InputOutputManager::universeInputProfiles(int universe)
{
    QVariantList profilesList;
    QStringList profileNames = m_ioMap->profileNames();
    profileNames.sort(Qt::CaseInsensitive);
    QDir pSysPath = m_ioMap->systemProfileDirectory();

    foreach (QString name, profileNames)
    {
        QLCInputProfile *ip = m_ioMap->profile(name);
        if (ip != nullptr)
        {
            QString type = ip->typeToString(ip->type());
            QVariantMap profileMap;
            profileMap.insert("universe", universe);
            profileMap.insert("name", name);
            profileMap.insert("line", name);
            profileMap.insert("plugin", type);
            if (ip->path().startsWith(pSysPath.absolutePath()))
                profileMap.insert("isUser", false);
            else
                profileMap.insert("isUser", true);
            profilesList.append(profileMap);
        }
    }

    return QVariant::fromValue(profilesList);
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

    // add the currently open input devices that support beats
    foreach (Universe *uni, m_ioMap->universes())
    {
        InputPatch *ip = uni->inputPatch();
        if (ip == nullptr || (ip->plugin()->capabilities() & QLCIOPlugin::Beats) == 0)
            continue;

        QVariantMap pluginMap;
        pluginMap.insert("type", "PLUGIN");
        pluginMap.insert("name", ip->inputName());
        pluginMap.insert("uni", uni->id());
        pluginMap.insert("line", ip->input());
        pluginMap.insert("privateName", ip->pluginName());
        genList.append(pluginMap);
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
        foreach (AudioDeviceInfo info, devList)
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
    else if (m_beatType == "PLUGIN")
        m_ioMap->setBeatGeneratorType(InputOutputMap::Plugin);
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
        case InputOutputMap::Plugin: m_beatType = "PLUGIN"; break;
        case InputOutputMap::Audio: m_beatType = "AUDIO"; break;
        case InputOutputMap::Disabled:
        default:
            m_beatType = "OFF";
        break;
    }
    emit beatTypeChanged(m_beatType);
    emit bpmNumberChanged(m_ioMap->bpmNumber());
}

int InputOutputManager::bpmNumber() const
{
    return m_ioMap->bpmNumber();
}

void InputOutputManager::setBpmNumber(int bpmNumber)
{
    if (m_ioMap->bpmNumber() == bpmNumber)
        return;

    m_ioMap->setBpmNumber(bpmNumber);
    emit bpmNumberChanged(bpmNumber);
}


