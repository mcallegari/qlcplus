/*
  Q Light Controller Plus
  inputoutputmap.cpp

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

#if defined(WIN32) || defined(Q_OS_WIN)
#	include <Windows.h>
#else
#   include <unistd.h>
#endif

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QElapsedTimer>
#include <QSettings>
#include <QDebug>
#include <qmath.h>

#include "inputoutputmap.h"
#include "qlcinputchannel.h"
#include "qlcinputsource.h"
#include "qlcioplugin.h"
#include "outputpatch.h"
#include "inputpatch.h"
#include "qlcconfig.h"
#include "universe.h"
#include "qlcfile.h"
#include "doc.h"

InputOutputMap::InputOutputMap(Doc *doc, quint32 universes)
  : QObject(doc)
  , m_blackout(false)
  , m_universeChanged(false)
  , m_currentBPM(0)
  , m_beatTime(new QElapsedTimer())
{
    m_grandMaster = new GrandMaster(this);
    for (quint32 i = 0; i < universes; i++)
        addUniverse();

    connect(doc->ioPluginCache(), SIGNAL(pluginConfigurationChanged(QLCIOPlugin*)),
            this, SLOT(slotPluginConfigurationChanged(QLCIOPlugin*)));
    connect(doc->masterTimer(), SIGNAL(beat()), this, SLOT(slotMasterTimerBeat()));
}

InputOutputMap::~InputOutputMap()
{
    removeAllUniverses();
    delete m_grandMaster;
    delete m_beatTime;
}

Doc* InputOutputMap::doc() const
{
    return qobject_cast<Doc*> (parent());
}

/*****************************************************************************
 * Blackout
 *****************************************************************************/

bool InputOutputMap::toggleBlackout()
{
    if (m_blackout == true)
        setBlackout(false);
    else
        setBlackout(true);

    return m_blackout;
}

bool InputOutputMap::setBlackout(bool blackout)
{
    /* Don't do blackout twice */
    if (m_blackout == blackout)
        return false;

    m_blackout = blackout;

    // blackout is an atomic setting, so it's safe to do it
    // without mutex locking
    foreach (Universe *universe, m_universeArray)
    {
        for (int i = 0; i < universe->outputPatchesCount(); i++)
        {
            OutputPatch *op = universe->outputPatch(i);
            if (op != NULL)
                op->setBlackout(blackout);
        }

        const QByteArray postGM = universe->postGMValues()->mid(0, universe->usedChannels());
        universe->dumpOutput(postGM, true);
    }

    emit blackoutChanged(m_blackout);

    return true;
}

void InputOutputMap::requestBlackout(BlackoutRequest blackout)
{
    if (blackout != BlackoutRequestNone)
        setBlackout(blackout == BlackoutRequestOn ? true : false);
}

bool InputOutputMap::blackout() const
{
    return m_blackout;
}

/*****************************************************************************
 * Universes
 *****************************************************************************/

quint32 InputOutputMap::invalidUniverse()
{
    return UINT_MAX;
}

bool InputOutputMap::addUniverse(quint32 id)
{
    {
        QMutexLocker locker(&m_universeMutex);
        Universe *uni = NULL;

        if (id == InputOutputMap::invalidUniverse())
        {
            id = universesCount();
        }
        else if (id < universesCount())
        {
            qWarning() << Q_FUNC_INFO
                << "Universe" << id << "is already present in the list."
                << "The universe list may be unsorted.";
            return false;
        }
        else if (id > universesCount())
        {
            qDebug() << Q_FUNC_INFO
                << "Gap between universe" << (universesCount() - 1)
                << "and universe" << id << ", filling the gap...";
            while (id > universesCount())
            {
                uni = new Universe(universesCount(), m_grandMaster);
                connect(doc()->masterTimer(), SIGNAL(tickReady()), uni, SLOT(tick()), Qt::QueuedConnection);
                connect(uni, SIGNAL(universeWritten(quint32,QByteArray)), this, SIGNAL(universeWritten(quint32,QByteArray)));
                m_universeArray.append(uni);
            }
        }

        uni = new Universe(id, m_grandMaster);
        connect(doc()->masterTimer(), SIGNAL(tickReady()), uni, SLOT(tick()), Qt::QueuedConnection);
        connect(uni, SIGNAL(universeWritten(quint32,QByteArray)), this, SIGNAL(universeWritten(quint32,QByteArray)));
        m_universeArray.append(uni);
    }

    emit universeAdded(id);
    return true;
}

bool InputOutputMap::removeUniverse(int index)
{
    {
        QMutexLocker locker(&m_universeMutex);

        if (index < 0 || index >= m_universeArray.count())
            return false;

        if (index != (m_universeArray.size() - 1))
        {
            qWarning() << Q_FUNC_INFO << "Removing universe" << index
                << "would create a gap in the universe list, cancelling";
            return false;
        }

        delete  m_universeArray.takeAt(index);
    }

    emit universeRemoved(index);
    return true;
}

bool InputOutputMap::removeAllUniverses()
{
    QMutexLocker locker(&m_universeMutex);
    qDeleteAll(m_universeArray);
    m_universeArray.clear();
    return true;
}

void InputOutputMap::startUniverses()
{
    foreach (Universe *uni, m_universeArray)
        uni->start();
}

quint32 InputOutputMap::getUniverseID(int index)
{
    if (index >= 0 && index < m_universeArray.count())
        return index;

    return invalidUniverse();
}

QString InputOutputMap::getUniverseNameByIndex(int index)
{
    if (index >= 0 && index < m_universeArray.count())
        return m_universeArray.at(index)->name();

    return QString();
}

QString InputOutputMap::getUniverseNameByID(quint32 id)
{
    return getUniverseNameByIndex(id);
}

void InputOutputMap::setUniverseName(int index, QString name)
{
    if (index < 0 || index >= m_universeArray.count())
        return;
    m_universeArray.at(index)->setName(name);
}

void InputOutputMap::setUniversePassthrough(int index, bool enable)
{
    if (index < 0 || index >= m_universeArray.count())
        return;
    m_universeArray.at(index)->setPassthrough(enable);
}

bool InputOutputMap::getUniversePassthrough(int index)
{
    if (index < 0 || index >= m_universeArray.count())
        return false;
    return m_universeArray.at(index)->passthrough();
}

void InputOutputMap::setUniverseMonitor(int index, bool enable)
{
    if (index < 0 || index >= m_universeArray.count())
        return;
    m_universeArray.at(index)->setMonitor(enable);
}

bool InputOutputMap::getUniverseMonitor(int index)
{
    if (index < 0 || index >= m_universeArray.count())
        return false;
    return m_universeArray.at(index)->monitor();
}

bool InputOutputMap::isUniversePatched(int index)
{
    if (index < 0 || index >= m_universeArray.count())
        return false;

    return m_universeArray.at(index)->isPatched();
}

quint32 InputOutputMap::universesCount() const
{
    return (quint32)m_universeArray.count();
}

QList<Universe *> InputOutputMap::universes() const
{
    return m_universeArray;
}

Universe *InputOutputMap::universe(quint32 id)
{
    for (int i = 0; i < m_universeArray.size(); i++)
        if (m_universeArray.at(i)->id() == id)
            return m_universeArray.at(i);

    return NULL;
}

QList<Universe*> InputOutputMap::claimUniverses()
{
    m_universeMutex.lock();
    return m_universeArray;
}

void InputOutputMap::releaseUniverses(bool changed)
{
    m_universeChanged = changed;
    m_universeMutex.unlock();
}

void InputOutputMap::resetUniverses()
{
    {
        QMutexLocker locker(&m_universeMutex);
        for (int i = 0; i < m_universeArray.size(); i++)
            m_universeArray.at(i)->reset();
    }

    /* Reset Grand Master parameters */
    setGrandMasterValue(255);
    setGrandMasterValueMode(GrandMaster::Reduce);
    setGrandMasterChannelMode(GrandMaster::Intensity);
}

/*********************************************************************
 * Grand Master
 *********************************************************************/

void InputOutputMap::setGrandMasterChannelMode(GrandMaster::ChannelMode mode)
{
    Q_ASSERT(m_grandMaster != NULL);

    if (m_grandMaster->channelMode() != mode)
    {
        m_grandMaster->setChannelMode(mode);
        m_universeChanged = true;
    }
}

GrandMaster::ChannelMode InputOutputMap::grandMasterChannelMode()
{
    Q_ASSERT(m_grandMaster != NULL);

    return m_grandMaster->channelMode();
}

void InputOutputMap::setGrandMasterValueMode(GrandMaster::ValueMode mode)
{
    Q_ASSERT(m_grandMaster != NULL);

    if (m_grandMaster->valueMode() != mode)
    {
        m_grandMaster->setValueMode(mode);
        m_universeChanged = true;
    }

    emit grandMasterValueModeChanged(mode);
}

GrandMaster::ValueMode InputOutputMap::grandMasterValueMode()
{
    Q_ASSERT(m_grandMaster != NULL);

    return m_grandMaster->valueMode();
}

void InputOutputMap::setGrandMasterValue(uchar value)
{
    Q_ASSERT(m_grandMaster != NULL);

    if (m_grandMaster->value() != value)
    {
        m_grandMaster->setValue(value);
        m_universeChanged = true;
    }

    if (m_universeChanged == true)
        emit grandMasterValueChanged(value);
}

uchar InputOutputMap::grandMasterValue()
{
    Q_ASSERT(m_grandMaster != NULL);

    return m_grandMaster->value();
}

/*********************************************************************
 * Patch
 *********************************************************************/

void InputOutputMap::flushInputs()
{
    QMutexLocker locker(&m_universeMutex);
    foreach (Universe *universe, m_universeArray)
        universe->flushInput();
}

bool InputOutputMap::setInputPatch(quint32 universe, const QString &pluginName,
                                   const QString &inputUID, quint32 input,
                                   const QString &profileName)
{
    /* Check that the universe that we're doing mapping for is valid */
    if (universe >= universesCount())
    {
        qWarning() << Q_FUNC_INFO << "Universe" << universe << "out of bounds.";
        return false;
    }

    QMutexLocker locker(&m_universeMutex);
    InputPatch *currInPatch = m_universeArray.at(universe)->inputPatch();
    QLCInputProfile *currProfile = NULL;
    if (currInPatch != NULL)
    {
        currProfile = currInPatch->profile();
        disconnect(currInPatch, SIGNAL(inputValueChanged(quint32,quint32,uchar,const QString&)),
                this, SIGNAL(inputValueChanged(quint32,quint32,uchar,const QString&)));
        if (currInPatch->plugin()->capabilities() & QLCIOPlugin::Beats)
        {
            disconnect(currInPatch, SIGNAL(inputValueChanged(quint32,quint32,uchar,const QString&)),
                       this, SLOT(slotPluginBeat(quint32,quint32,uchar,const QString&)));
        }
    }
    InputPatch *ip = NULL;
    QLCIOPlugin *plugin = doc()->ioPluginCache()->plugin(pluginName);

    if (!inputUID.isEmpty() && plugin != NULL)
    {
        QStringList inputs = plugin->inputs();
        int lIdx = inputs.indexOf(inputUID);
        if (lIdx != -1)
        {
            qDebug() << "[IOMAP] Found match on input by name on universe" << universe << "-" << input << "vs" << lIdx;
            input = lIdx;
        }
        else
        {
            qDebug() << "[IOMAP] !!No match found!! for input on universe" << universe << "-" << input << inputUID;
            qDebug() << plugin->inputs();
        }
    }

    if (m_universeArray.at(universe)->setInputPatch(
                plugin, input, profile(profileName)) == true)
    {
        ip = m_universeArray.at(universe)->inputPatch();
        if (ip != NULL)
        {
            connect(ip, SIGNAL(inputValueChanged(quint32,quint32,uchar,const QString&)),
                    this, SIGNAL(inputValueChanged(quint32,quint32,uchar,const QString&)));
            if (ip->plugin()->capabilities() & QLCIOPlugin::Beats)
            {
                connect(ip, SIGNAL(inputValueChanged(quint32,quint32,uchar,const QString&)),
                        this, SLOT(slotPluginBeat(quint32,quint32,uchar,const QString&)));
            }
        }
    }
    else
    {
        return false;
    }

    if (ip != NULL && currProfile != ip->profile())
        emit profileChanged(universe, ip->profileName());

    return true;
}

bool InputOutputMap::setInputProfile(quint32 universe, const QString &profileName)
{
    /* Check that the universe that we're doing mapping for is valid */
    if (universe >= universesCount())
    {
        qWarning() << Q_FUNC_INFO << "Universe" << universe << "out of bounds.";
        return false;
    }

    InputPatch *currInPatch = m_universeArray.at(universe)->inputPatch();
    if (currInPatch != NULL)
        currInPatch->set(profile(profileName));

    /* if no input patch is set, then setting a profile is useless,
       but there's no reason to cause an error here */
    return true;
}

bool InputOutputMap::setOutputPatch(quint32 universe, const QString &pluginName,
                                    const QString &outputUID, quint32 output,
                                    bool isFeedback, int index)
{
    /* Check that the universe that we're doing mapping for is valid */
    if (universe >= universesCount())
    {
        qWarning() << Q_FUNC_INFO << "Universe" << universe << "out of bounds.";
        return false;
    }

    QMutexLocker locker(&m_universeMutex);
    QLCIOPlugin *plugin = doc()->ioPluginCache()->plugin(pluginName);

    if (!outputUID.isEmpty() && plugin != NULL)
    {
        QStringList inputs = plugin->outputs();
        int lIdx = inputs.indexOf(outputUID);
        if (lIdx != -1)
        {
            qDebug() << "[IOMAP] Found match on output by name on universe" << universe << "-" << output << "vs" << lIdx;
            output = lIdx;
        }
        else
        {
            qDebug() << "[IOMAP] !!No match found!! for output on universe" << universe << "-" << output << outputUID;
            qDebug() << plugin->outputs();
        }
    }

    if (isFeedback == false)
        return m_universeArray.at(universe)->setOutputPatch(plugin, output, index);
    else
        return m_universeArray.at(universe)->setFeedbackPatch(plugin, output);

    return false;
}

int InputOutputMap::outputPatchesCount(quint32 universe) const
{
    if (universe >= universesCount())
    {
        qWarning() << Q_FUNC_INFO << "Universe" << universe << "out of bounds.";
        return 0;
    }

    return m_universeArray.at(universe)->outputPatchesCount();
}

InputPatch *InputOutputMap::inputPatch(quint32 universe) const
{
    if (universe >= universesCount())
    {
        qWarning() << Q_FUNC_INFO << "Universe" << universe << "out of bounds.";
        return NULL;
    }
    return m_universeArray.at(universe)->inputPatch();
}

OutputPatch *InputOutputMap::outputPatch(quint32 universe, int index) const
{
    if (universe >= universesCount())
    {
        qWarning() << Q_FUNC_INFO << "Universe" << universe << "out of bounds.";
        return NULL;
    }
    return m_universeArray.at(universe)->outputPatch(index);
}

OutputPatch *InputOutputMap::feedbackPatch(quint32 universe) const
{
    if (universe >= universesCount())
    {
        qWarning() << Q_FUNC_INFO << "Universe" << universe << "out of bounds.";
        return NULL;
    }
    return m_universeArray.at(universe)->feedbackPatch();
}

QStringList InputOutputMap::universeNames() const
{
    QStringList list;
    for (quint32 i = 0; i < universesCount(); i++)
        list << m_universeArray.at(i)->name();

    return list;
}

quint32 InputOutputMap::inputMapping(const QString &pluginName, quint32 input) const
{
    for (quint32 uni = 0; uni < universesCount(); uni++)
    {
        const InputPatch* p = m_universeArray.at(uni)->inputPatch();
        if (p != NULL && p->pluginName() == pluginName && p->input() == input)
            return uni;
    }

    return QLCIOPlugin::invalidLine();
}

quint32 InputOutputMap::outputMapping(const QString &pluginName, quint32 output) const
{
    for (quint32 uni = 0; uni < universesCount(); uni++)
    {
        Universe *universe = m_universeArray.at(uni);
        for (int i = 0; i < universe->outputPatchesCount(); i++)
        {
            const OutputPatch* p = universe->outputPatch(i);
            if (p != NULL && p->pluginName() == pluginName && p->output() == output)
                return uni;
        }
    }

    return QLCIOPlugin::invalidLine();
}

/*****************************************************************************
 * Plugins
 *****************************************************************************/

QString InputOutputMap::pluginDescription(const QString &pluginName)
{
    QLCIOPlugin* plugin = NULL;

    if (pluginName.isEmpty() == false)
        plugin = doc()->ioPluginCache()->plugin(pluginName);

    if (plugin != NULL)
    {
        return plugin->pluginInfo();
    }
    else
        return "";
}

void InputOutputMap::removeDuplicates(QStringList &list)
{
    if (list.count() == 1)
        return;

    int c = 2;

    for (int i = 1; i < list.count(); i++)
    {
        for (int j = 0; j < i; j++)
        {
            if (list.at(i) == list.at(j))
            {
                list.replace(i, QString("%1 %2").arg(list.at(j)).arg(c));
                c++;
            }
        }
    }
}

QStringList InputOutputMap::inputPluginNames()
{
    QStringList list;
    QListIterator <QLCIOPlugin*> it(doc()->ioPluginCache()->plugins());
    while (it.hasNext() == true)
    {
        QLCIOPlugin* plg(it.next());
        if (plg->capabilities() & QLCIOPlugin::Input)
            list << plg->name();
    }
    return list;
}

QStringList InputOutputMap::outputPluginNames()
{
    QStringList list;
    QListIterator <QLCIOPlugin*> it(doc()->ioPluginCache()->plugins());
    while (it.hasNext() == true)
    {
        QLCIOPlugin* plg(it.next());
        if (plg->capabilities() & QLCIOPlugin::Output)
            list << plg->name();
    }
    return list;
}

QStringList InputOutputMap::pluginInputs(const QString& pluginName)
{
    QLCIOPlugin* ip = doc()->ioPluginCache()->plugin(pluginName);
    if (ip == NULL)
        return QStringList();
    else
    {
        QStringList iList = ip->inputs();
        removeDuplicates(iList);
        return iList;
    }
}

QStringList InputOutputMap::pluginOutputs(const QString& pluginName)
{
    QLCIOPlugin* op = doc()->ioPluginCache()->plugin(pluginName);
    if (op == NULL)
        return QStringList();
    else
    {
        QStringList oList = op->outputs();
        removeDuplicates(oList);
        return oList;
    }
}

bool InputOutputMap::pluginSupportsFeedback(const QString& pluginName)
{
    QLCIOPlugin* outputPlugin = doc()->ioPluginCache()->plugin(pluginName);
    if (outputPlugin != NULL)
        return (outputPlugin->capabilities() & QLCIOPlugin::Feedback) > 0;
    else
        return false;
}

void InputOutputMap::configurePlugin(const QString& pluginName)
{
    QLCIOPlugin* outputPlugin = doc()->ioPluginCache()->plugin(pluginName);
    if (outputPlugin != NULL)
        outputPlugin->configure();
}

bool InputOutputMap::canConfigurePlugin(const QString& pluginName)
{
    QLCIOPlugin* outputPlugin = doc()->ioPluginCache()->plugin(pluginName);
    if (outputPlugin != NULL)
        return outputPlugin->canConfigure();
    else
        return false;
}

QString InputOutputMap::inputPluginStatus(const QString& pluginName, quint32 input)
{
    QLCIOPlugin* inputPlugin = NULL;
    QString info;

    if (pluginName.isEmpty() == false)
        inputPlugin = doc()->ioPluginCache()->plugin(pluginName);

    if (inputPlugin != NULL)
    {
        info = inputPlugin->inputInfo(input);
    }
    else
    {
        /* Nothing selected */
        info += QString("<HTML><HEAD></HEAD><BODY>");
        info += QString("<H3>%1</H3>").arg(tr("Nothing selected"));
        info += QString("</BODY></HTML>");
    }

    return info;
}

QString InputOutputMap::outputPluginStatus(const QString& pluginName, quint32 output)
{
    QLCIOPlugin* outputPlugin = doc()->ioPluginCache()->plugin(pluginName);
    if (outputPlugin != NULL)
    {
        return outputPlugin->outputInfo(output);
    }
    else
    {
        QString info;
        info += QString("<HTML><HEAD></HEAD><BODY>");
        info += QString("<H3>%1</H3>").arg(tr("Nothing selected"));
        info += QString("</BODY></HTML>");
        return info;
    }
}

bool InputOutputMap::sendFeedBack(quint32 universe, quint32 channel, uchar value, const QVariant &params)
{
    if (universe >= universesCount())
        return false;

    OutputPatch* patch = m_universeArray.at(universe)->feedbackPatch();

    if (patch != NULL && patch->isPatched())
    {
        patch->plugin()->sendFeedBack(universe, patch->output(), channel, value, params);
        return true;
    }
    else
    {
        return false;
    }
}

void InputOutputMap::slotPluginConfigurationChanged(QLCIOPlugin* plugin)
{
    QMutexLocker locker(&m_universeMutex);
    bool success = true;
    for (quint32 i = 0; i < universesCount(); i++)
    {
        Universe *universe = m_universeArray.at(i);
        for (int oi = 0; oi < universe->outputPatchesCount(); oi++)
        {
            OutputPatch* op = universe->outputPatch(oi);

            if (op != NULL && op->plugin() == plugin)
            {
                /*success = */ op->reconnect();
            }
        }

        InputPatch* ip = m_universeArray.at(i)->inputPatch();

        if (ip != NULL && ip->plugin() == plugin)
        {
            /*success = */ ip->reconnect();
        }

        OutputPatch* fp = m_universeArray.at(i)->feedbackPatch();
        if (fp != NULL && fp->plugin() == plugin)
        {
            /*success = */ fp->reconnect();
        }
    }
    locker.unlock();

    emit pluginConfigurationChanged(plugin->name(), success);
}

/*****************************************************************************
 * Profiles
 *****************************************************************************/

void InputOutputMap::loadProfiles(const QDir& dir)
{
    if (dir.exists() == false || dir.isReadable() == false)
        return;

    /* Go thru all found file entries and attempt to load an input
       profile from each of them. */
    QStringListIterator it(dir.entryList());
    while (it.hasNext() == true)
    {
        QLCInputProfile* prof;
        QString path;

        path = dir.absoluteFilePath(it.next());
        prof = QLCInputProfile::loader(path);
        if (prof != NULL)
        {
            /* Check for duplicates */
            if (profile(prof->name()) == NULL)
                addProfile(prof);
            else
                delete prof;
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unable to find an input profile from" << path;
        }
    }
}

QStringList InputOutputMap::profileNames()
{
    QStringList list;
    QListIterator <QLCInputProfile*> it(m_profiles);
    while (it.hasNext() == true)
        list << it.next()->name();
    return list;
}

QLCInputProfile* InputOutputMap::profile(const QString& name)
{
    QListIterator <QLCInputProfile*> it(m_profiles);
    while (it.hasNext() == true)
    {
        QLCInputProfile* profile = it.next();
        if (profile->name() == name)
            return profile;
    }

    return NULL;
}

bool InputOutputMap::addProfile(QLCInputProfile* profile)
{
    Q_ASSERT(profile != NULL);

    /* Don't add the same profile twice */
    if (m_profiles.contains(profile) == false)
    {
        m_profiles.append(profile);
        return true;
    }
    else
    {
        return false;
    }
}

bool InputOutputMap::removeProfile(const QString& name)
{
    QMutableListIterator <QLCInputProfile*> it(m_profiles);
    while (it.hasNext() == true)
    {
        QLCInputProfile *profile = it.next();
        if (profile->name() == name)
        {
            it.remove();
            delete profile;
            return true;
        }
    }

    return false;
}

bool InputOutputMap::inputSourceNames(const QLCInputSource *src,
                                QString& uniName, QString& chName) const
{
    if (src == NULL || src->isValid() == false)
        return false;

    if (src->universe() >= universesCount())
        return false;

    InputPatch* pat = m_universeArray.at(src->universe())->inputPatch();
    if (pat == NULL)
    {
        /* There is no patch for the given universe */
        uniName = QString("%1 -UNPATCHED-").arg(src->universe() + 1);

        ushort page = src->page();
        ushort channel = (src->channel() & 0x0000FFFF) + 1;

        if (page != 0)
            chName = QString("%1: ? (Page %2)").arg(channel).arg(page + 1);
        else
            chName = QString("%1: ?").arg(channel);
        return true;
    }

    QLCInputProfile* profile = pat->profile();
    if (profile == NULL)
    {
        /* There is no profile. Display plugin name and channel number. */
        if (pat->plugin() != NULL)
            uniName = QString("%1: %2").arg(src->universe() + 1).arg(pat->plugin()->name());
        else
            uniName = QString("%1: ??").arg(src->universe() + 1);

        ushort page = src->page();
        ushort channel = (src->channel() & 0x0000FFFF) + 1;

        if (page != 0)
            chName = QString("%1: ? (Page %2)").arg(channel).arg(page + 1);
        else
            chName = QString("%1: ?").arg(channel);
    }
    else
    {
        QLCInputChannel* ich;
        QString name;

        /* Display profile name for universe */
        uniName = QString("%1: %2").arg(src->universe() + 1).arg(profile->name());

        /* User can input the channel number by hand, so put something
           rational to the channel name in those cases as well. */
        ushort page = src->page();
        ushort channel = (src->channel() & 0x0000FFFF);

        ich = profile->channel(channel);
        if (ich != NULL)
            name = ich->name();
        else
            name = QString("?");

        /* Display channel name */
        if (page != 0)
            chName = QString("%1: %2 (Page %3)").arg(channel + 1).arg(name).arg(page + 1);
        else
            chName = QString("%1: %2").arg(channel + 1).arg(name);
    }

    return true;
}

bool InputOutputMap::inputSourceNames(QSharedPointer<QLCInputSource> const& src,
                                QString& uniName, QString& chName) const
{
    return inputSourceNames(src.data(), uniName, chName);
}

QDir InputOutputMap::systemProfileDirectory()
{
    return QLCFile::systemDirectory(QString(INPUTPROFILEDIR), QString(KExtInputProfile));
}

QDir InputOutputMap::userProfileDirectory()
{
    return QLCFile::userDirectory(QString(USERINPUTPROFILEDIR), QString(INPUTPROFILEDIR),
                                  QStringList() << QString("*%1").arg(KExtInputProfile));
}

/*********************************************************************
 * Beats
 *********************************************************************/

void InputOutputMap::setBeatGeneratorType(InputOutputMap::BeatGeneratorType type)
{
    if (type == m_beatGeneratorType)
        return;

    m_beatGeneratorType = type;
    qDebug() << "[InputOutputMap] setting beat type:" << m_beatGeneratorType;

    switch (m_beatGeneratorType)
    {
        case Internal:
            doc()->masterTimer()->setBeatSourceType(MasterTimer::Internal);
            setBpmNumber(doc()->masterTimer()->bpmNumber());
        break;
        case Plugin:
            doc()->masterTimer()->setBeatSourceType(MasterTimer::External);
            // reset the current BPM number and detect it from the MIDI beats
            setBpmNumber(0);
            m_beatTime->restart();
        break;
        case Audio:
            doc()->masterTimer()->setBeatSourceType(MasterTimer::External);
            // reset the current BPM number and detect it from the audio input
            setBpmNumber(0);
            m_beatTime->restart();
        break;
        case Disabled:
        default:
            doc()->masterTimer()->setBeatSourceType(MasterTimer::None);
            setBpmNumber(0);
        break;
    }

    emit beatGeneratorTypeChanged();
}

InputOutputMap::BeatGeneratorType InputOutputMap::beatGeneratorType() const
{
    return m_beatGeneratorType;
}

QString InputOutputMap::beatTypeToString(BeatGeneratorType type) const
{
    switch (type)
    {
        case Internal:  return "Internal";
        case Plugin:    return "Plugin";
        case Audio:     return "Audio";
        default:        return "Disabled";
    }
}

InputOutputMap::BeatGeneratorType InputOutputMap::stringToBeatType(QString str)
{
    if (str == "Internal")
        return Internal;
    else if (str == "Plugin")
        return Plugin;
    else if (str == "Audio")
        return Audio;

    return Disabled;
}

void InputOutputMap::setBpmNumber(int bpm)
{
    if (m_beatGeneratorType == Disabled || bpm == m_currentBPM)
        return;

    //qDebug() << "[InputOutputMap] set BPM to" << bpm;
    m_currentBPM = bpm;

    if (bpm != 0)
        doc()->masterTimer()->requestBpmNumber(bpm);

    emit bpmNumberChanged(m_currentBPM);
}

int InputOutputMap::bpmNumber() const
{
    if (m_beatGeneratorType == Disabled)
        return 0;

    return m_currentBPM;
}

void InputOutputMap::slotMasterTimerBeat()
{
    if (m_beatGeneratorType != Internal)
        return;

    emit beat();
}

void InputOutputMap::slotPluginBeat(quint32 universe, quint32 channel, uchar value, const QString &key)
{
    Q_UNUSED(universe)

    // not interested in synthetic release or non-beat event
    if (m_beatGeneratorType != Plugin || value == 0 || key != "beat")
        return;

    qDebug() << "Plugin beat:" << channel << m_beatTime->elapsed();

    // process the timer as first thing, to avoid wasting time
    // with the operations below
    int elapsed = m_beatTime->elapsed();
    m_beatTime->restart();

    int bpm = qRound(60000.0 / (float)elapsed);
    float currBpmTime = 60000.0 / (float)m_currentBPM;
    // here we check if the difference between the current BPM duration
    // and the current time elapsed is within a range of +/-1ms.
    // If it isn't, then the BPM number has really changed, otherwise
    // it's just a tiny time drift
    if (qAbs((float)elapsed - currBpmTime) > 1)
        setBpmNumber(bpm);

    doc()->masterTimer()->requestBeat();
    emit beat();
}

void InputOutputMap::slotAudioSpectrum(double *spectrumBands, int size, double maxMagnitude, quint32 power)
{
    Q_UNUSED(spectrumBands)
    Q_UNUSED(size)
    Q_UNUSED(maxMagnitude)
    Q_UNUSED(power)
}

/*********************************************************************
 * Defaults - !! FALLBACK !!
 *********************************************************************/

void InputOutputMap::loadDefaults()
{
    /* ************************ INPUT *********************************** */
    QSettings settings;
    QString plugin;
    QString input;
    QString key;

    for (quint32 i = 0; i < universesCount(); i++)
    {
        QString profileName;
        bool passthrough;

        /* Plugin name */
        key = QString("/inputmap/universe%1/plugin/").arg(i);
        plugin = settings.value(key).toString();

        /* Plugin input */
        key = QString("/inputmap/universe%1/input/").arg(i);
        input = settings.value(key).toString();

        /* Input profile */
        key = QString("/inputmap/universe%1/profile/").arg(i);
        profileName = settings.value(key).toString();

        key = QString("/inputmap/universe%1/passthrough/").arg(i);
        passthrough = settings.value(key).toBool();
        if (passthrough == true)
            m_universeArray.at(i)->setPassthrough(passthrough);

        /* Do the mapping */
        if (plugin != KInputNone && input != KInputNone)
            setInputPatch(i, plugin, "", input.toUInt(), profileName);
    }

    /* ************************ OUTPUT *********************************** */
    QString output;
    QString fb_plugin;
    QString feedback;

    for (quint32 i = 0; i < universesCount(); i++)
    {
        /* Plugin name */
        key = QString("/outputmap/universe%1/plugin/").arg(i);
        plugin = settings.value(key).toString();

        /* Plugin output */
        key = QString("/outputmap/universe%1/output/").arg(i);
        output = settings.value(key).toString();

        /* Feedback plugin name */
        key = QString("/outputmap/universe%1/feedbackplugin/").arg(i);
        fb_plugin = settings.value(key).toString();

        /* Feedback line */
        key = QString("/outputmap/universe%1/feedback/").arg(i);
        feedback = settings.value(key).toString();

        if (plugin != KOutputNone && output != KOutputNone)
            setOutputPatch(i, plugin, "", output.toUInt());

        if (fb_plugin != KOutputNone && feedback != KOutputNone)
            setOutputPatch(i, fb_plugin, "", feedback.toUInt(), true);
    }
}

void InputOutputMap::saveDefaults()
{
    /* ************************ INPUT *********************************** */
    QSettings settings;
    QString key;

    for (quint32 i = 0; i < universesCount(); i++)
    {
        InputPatch* inPatch = inputPatch(i);

        /* Plugin name */
        key = QString("/inputmap/universe%1/plugin/").arg(i);
        if (inPatch != NULL)
            settings.setValue(key, inPatch->pluginName());
        else
            settings.setValue(key, KInputNone);

        /* Plugin input */
        key = QString("/inputmap/universe%1/input/").arg(i);
        if (inPatch != NULL)
            settings.setValue(key, QString::number(inPatch->input()));
        else
            settings.setValue(key, KInputNone);

        /* Input profile */
        key = QString("/inputmap/universe%1/profile/").arg(i);
        if (inPatch != NULL)
            settings.setValue(key, inPatch->profileName());
        else
            settings.setValue(key, KInputNone);

        /* Passthrough */
        key = QString("/inputmap/universe%1/passthrough/").arg(i);
        bool passthrough = m_universeArray.at(i)->passthrough();
        if (passthrough == true)
            settings.setValue(key, passthrough);
        else
            settings.remove(key);
    }

    /* ************************ OUTPUT *********************************** */

    for (quint32 i = 0; i < universesCount(); i++)
    {
        OutputPatch* outPatch = outputPatch(i);
        OutputPatch* fbPatch = feedbackPatch(i);

        key = QString("/outputmap/universe%1/plugin/").arg(i);

        /* Plugin name */
        if (outPatch != NULL)
            settings.setValue(key, outPatch->pluginName());
        else
            settings.setValue(key, KOutputNone);

        /* Plugin output */
        key = QString("/outputmap/universe%1/output/").arg(i);
        if (outPatch != NULL)
            settings.setValue(key, outPatch->output());
        else
            settings.setValue(key, KOutputNone);

        key = QString("/outputmap/universe%1/feedbackplugin/").arg(i);

        /* Feedback plugin name */
        if (fbPatch != NULL)
            settings.setValue(key, fbPatch->pluginName());
        else
            settings.setValue(key, KOutputNone);

        /* Feedback plugin output */
        key = QString("/outputmap/universe%1/feedback/").arg(i);
        if (fbPatch != NULL)
            settings.setValue(key, QString::number(fbPatch->output()));
        else
            settings.setValue(key, KOutputNone);
    }
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

bool InputOutputMap::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLIOMap)
    {
        qWarning() << Q_FUNC_INFO << "InputOutputMap node not found";
        return false;
    }

    /** Reset the current universe list and read the new one */
    removeAllUniverses();

    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCUniverse)
        {
            quint32 id = InputOutputMap::invalidUniverse();
            if (root.attributes().hasAttribute(KXMLQLCUniverseID))
                id = root.attributes().value(KXMLQLCUniverseID).toString().toUInt();
            if (addUniverse(id))
            {
                Universe *uni = m_universeArray.last();
                uni->loadXML(root, m_universeArray.count() - 1, this);
            }
        }
        else if (root.name() == KXMLIOBeatGenerator)
        {
            QXmlStreamAttributes attrs = root.attributes();

            if (attrs.hasAttribute(KXMLIOBeatType))
                setBeatGeneratorType(stringToBeatType(attrs.value(KXMLIOBeatType).toString()));

            if (attrs.hasAttribute(KXMLIOBeatsPerMinute))
                setBpmNumber(attrs.value(KXMLIOBeatsPerMinute).toInt());

            root.skipCurrentElement();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown IO Map tag:" << root.name();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool InputOutputMap::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    /* IO Map Instance entry */
    doc->writeStartElement(KXMLIOMap);

    doc->writeStartElement(KXMLIOBeatGenerator);
    doc->writeAttribute(KXMLIOBeatType, beatTypeToString(m_beatGeneratorType));
    doc->writeAttribute(KXMLIOBeatsPerMinute, QString::number(m_currentBPM));
    doc->writeEndElement();

    foreach (Universe *uni, m_universeArray)
        uni->saveXML(doc);

    doc->writeEndElement();

    return true;
}
