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

#include <QDomElement>

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
  , m_latestUniverseId(InputOutputMap::invalidUniverse())
  , m_universeChanged(false)
{
    m_grandMaster = new GrandMaster(this);
    for (quint32 i = 0; i < universes; i++)
        m_universeArray.append(new Universe(i, m_grandMaster, this));

    connect(doc->ioPluginCache(), SIGNAL(pluginConfigurationChanged(QLCIOPlugin*)),
            this, SLOT(slotPluginConfigurationChanged(QLCIOPlugin*)));
}

InputOutputMap::~InputOutputMap()
{
    removeAllUniverses();
    delete m_grandMaster;
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

void InputOutputMap::setBlackout(bool blackout)
{
    /* Don't do blackout twice */
    if (m_blackout == blackout)
        return;
    m_blackout = blackout;

    if (blackout == true)
    {
        QByteArray zeros(512, 0);
        for (quint32 i = 0; i < universes(); i++)
        {
            if (m_universeArray.at(i)->outputPatch() != NULL)
                m_universeArray.at(i)->outputPatch()->dump(zeros);
        }
    }
    else
    {
        /* Force writing of values back to the plugins */
        m_universeChanged = true;
    }

    emit blackoutChanged(m_blackout);
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
    m_universeMutex.lock();
    if (id == InputOutputMap::invalidUniverse())
        id = ++m_latestUniverseId;

    m_universeArray.append(new Universe(id, m_grandMaster));
    m_universeMutex.unlock();
    return true;
}

bool InputOutputMap::removeUniverse()
{
    m_universeMutex.lock();
    Universe *delUni = m_universeArray.takeLast();
    delete delUni;
    m_universeMutex.unlock();

    return true;
}

bool InputOutputMap::removeAllUniverses()
{
    quint32 uniCount = universes();
    for (quint32 i = 0; i < uniCount; i++)
    {
        Universe *uni = m_universeArray.takeLast();
        delete uni;
    }
    m_latestUniverseId = invalidUniverse();
    return true;
}

QString InputOutputMap::getUniverseName(int index)
{
    if (index >= 0 && index < m_universeArray.count())
        return m_universeArray.at(index)->name();

    return QString();
}

void InputOutputMap::setUniverseName(int index, QString name)
{
    if (index < 0 || index >= m_universeArray.count())
        return;
    m_universeArray.at(index)->setName(name);
}

quint32 InputOutputMap::universes() const
{
    return (quint32)m_universeArray.count();
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

void InputOutputMap::dumpUniverses()
{
    m_universeMutex.lock();
    if (m_blackout == false)
    {
        for (int i = 0; i < m_universeArray.count(); i++)
        {
            Universe *universe = m_universeArray.at(i);
            if (universe->hasChanged() && universe->outputPatch() != NULL)
            {
                const QByteArray postGM = universe->postGMValues()->mid(0);
                /*
                fprintf(stderr, "---- ");
                for (int d = 0; d < universe->usedChannels(); d++)
                    fprintf(stderr, "%d ", (unsigned char)postGM.at(d));
                fprintf(stderr, " ----\n");
                */
                universe->outputPatch()->dump(postGM);

                m_universeMutex.unlock();
                emit universesWritten(i, postGM);
                m_universeMutex.lock();
            }
        }
    }
    m_universeMutex.unlock();
}

void InputOutputMap::resetUniverses()
{
    m_universeMutex.lock();
    for (int i = 0; i < m_universeArray.size(); i++)
        m_universeArray.at(i)->reset();
    m_universeMutex.unlock();

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

    if(m_grandMaster->channelMode() != mode)
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

    if(m_grandMaster->valueMode() != mode)
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

bool InputOutputMap::setInputPatch(quint32 universe, const QString &pluginName,
                                   quint32 input, const QString &profileName)
{
    /* Check that the universe that we're doing mapping for is valid */
    if (universe >= universes())
    {
        qWarning() << Q_FUNC_INFO << "Universe" << universe << "out of bounds.";
        return false;
    }
    m_universeMutex.lock();
    if (m_universeArray.at(universe)->setInputPatch(
                doc()->ioPluginCache()->plugin(pluginName), input,
                profile(profileName)) == true)
    {
        InputPatch *ip = m_universeArray.at(universe)->inputPatch();
        if (ip != NULL)
            connect(ip, SIGNAL(inputValueChanged(quint32,quint32,uchar,const QString&)),
                    this, SIGNAL(inputValueChanged(quint32,quint32,uchar,const QString&)));
    }
    m_universeMutex.unlock();
    return true;
}

bool InputOutputMap::setOutputPatch(quint32 universe, const QString &pluginName,
                                    quint32 output, bool isFeedback)
{
    /* Check that the universe that we're doing mapping for is valid */
    if (universe >= universes())
    {
        qWarning() << Q_FUNC_INFO << "Universe" << universe << "out of bounds.";
        return false;
    }
    m_universeMutex.lock();
    if (isFeedback == false)
        m_universeArray.at(universe)->setOutputPatch(
                    doc()->ioPluginCache()->plugin(pluginName), output);
    else
        m_universeArray.at(universe)->setFeedbackPatch(
                    doc()->ioPluginCache()->plugin(pluginName), output);
    m_universeMutex.unlock();
    return true;
}

InputPatch *InputOutputMap::inputPatch(quint32 universe) const
{
    if (universe >= universes())
    {
        qWarning() << Q_FUNC_INFO << "Universe" << universe << "out of bounds.";
        return NULL;
    }
    return m_universeArray.at(universe)->inputPatch();
}

OutputPatch *InputOutputMap::outputPatch(quint32 universe) const
{
    if (universe >= universes())
    {
        qWarning() << Q_FUNC_INFO << "Universe" << universe << "out of bounds.";
        return NULL;
    }
    return m_universeArray.at(universe)->outputPatch();
}

OutputPatch *InputOutputMap::feedbackPatch(quint32 universe) const
{
    if (universe >= universes())
    {
        qWarning() << Q_FUNC_INFO << "Universe" << universe << "out of bounds.";
        return NULL;
    }
    return m_universeArray.at(universe)->feedbackPatch();
}

QStringList InputOutputMap::universeNames() const
{
    QStringList list;
    for (quint32 i = 0; i < universes(); i++)
    {
        list << m_universeArray.at(i)->name();
        /*
        OutputPatch* p(patch(i));
        Q_ASSERT(p != NULL);
        list << QString("%1: %2 (%3)").arg(i + 1).arg(p->pluginName()).arg(p->outputName());
        */
    }

    return list;
}

quint32 InputOutputMap::inputMapping(const QString &pluginName, quint32 input) const
{
    for (quint32 uni = 0; uni < universes(); uni++)
    {
        const InputPatch* p = m_universeArray.at(uni)->inputPatch();
        if (p != NULL && p->pluginName() == pluginName && p->input() == input)
            return uni;
    }

    return QLCIOPlugin::invalidLine();
}

quint32 InputOutputMap::outputMapping(const QString &pluginName, quint32 output) const
{
    for (quint32 uni = 0; uni < universes(); uni++)
    {
        const OutputPatch* p = m_universeArray.at(uni)->outputPatch();
        if (p != NULL && p->pluginName() == pluginName && p->output() == output)
            return uni;
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
        return ip->inputs();
}

QStringList InputOutputMap::pluginOutputs(const QString& pluginName)
{
    QLCIOPlugin* op = doc()->ioPluginCache()->plugin(pluginName);
    if (op == NULL)
        return QStringList();
    else
        return op->outputs();
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

bool InputOutputMap::sendFeedBack(quint32 universe, quint32 channel, uchar value, const QString& key)
{
    if (universe >= universes())
        return false;

    OutputPatch* patch = m_universeArray.at(universe)->feedbackPatch();

    if (patch != NULL && patch->isPatched())
    {
        patch->plugin()->sendFeedBack(patch->output(), channel, value, key);
        return true;
    }
    else
    {
        return false;
    }
}

void InputOutputMap::slotPluginConfigurationChanged(QLCIOPlugin* plugin)
{
    for (quint32 i = 0; i < universes(); i++)
    {
        OutputPatch* op = m_universeArray.at(i)->outputPatch();

        if (op != NULL && op->plugin() == plugin)
        {
            m_universeMutex.lock();
            op->reconnect();
            m_universeMutex.unlock();
        }
    }

    emit pluginConfigurationChanged(plugin->name());
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
    QLCInputProfile* profile;
    QMutableListIterator <QLCInputProfile*> it(m_profiles);
    while (it.hasNext() == true)
    {
        profile = it.next();
        if (profile->name() == name)
        {
            it.remove();
            delete profile;
            return true;
        }
    }

    return false;
}

bool InputOutputMap::inputSourceNames(const QLCInputSource& src,
                                QString& uniName, QString& chName) const
{
    if (src.isValid() == false)
        return false;

    if (src.universe() >= universes())
        return false;

    InputPatch* pat = m_universeArray.at(src.universe())->inputPatch();
    if (pat == NULL)
    {
        /* There is no patch for the given universe */
        return false;
    }

    QLCInputProfile* profile = pat->profile();
    if (profile == NULL)
    {
        /* There is no profile. Display plugin name and channel number. */
        if (pat->plugin() != NULL)
            uniName = QString("%1: %2").arg(src.universe() + 1).arg(pat->plugin()->name());
        else
            uniName = QString("%1: ??").arg(src.universe() + 1);

        ushort page = src.page();
        ushort channel = (src.channel() & 0x0000FFFF) + 1;

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
        uniName = QString("%1: %2").arg(src.universe() + 1).arg(profile->name());

        /* User can input the channel number by hand, so put something
           rational to the channel name in those cases as well. */
        ushort page = src.page();
        ushort channel = (src.channel() & 0x0000FFFF);

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

QDir InputOutputMap::systemProfileDirectory()
{
    QDir dir;

#if defined(__APPLE__) || defined(Q_OS_MAC)
    dir.setPath(QString("%1/../%2").arg(QCoreApplication::applicationDirPath())
                              .arg(INPUTPROFILEDIR));
#else
    dir.setPath(INPUTPROFILEDIR);
#endif

    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtInputProfile));
    return dir;
}

QDir InputOutputMap::userProfileDirectory()
{
    QDir dir;

#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
    // If the current user is root, return the system profile dir.
    // Otherwise return the user's home dir.
    if (geteuid() == 0)
        dir = QDir(INPUTPROFILEDIR);
    else
        dir.setPath(QString("%1/%2").arg(getenv("HOME")).arg(USERINPUTPROFILEDIR));
#elif defined(__APPLE__) || defined(Q_OS_MAC)
    /* User's input profile directory on OSX */
    dir.setPath(QString("%1/%2").arg(getenv("HOME")).arg(USERINPUTPROFILEDIR));
#else
    /* User's input profile directory on Windows */
    LPTSTR home = (LPTSTR) malloc(256 * sizeof(TCHAR));
    GetEnvironmentVariable(TEXT("UserProfile"), home, 256);
    dir.setPath(QString("%1/%2")
                    .arg(QString::fromUtf16(reinterpret_cast<ushort*> (home)))
                    .arg(USERINPUTPROFILEDIR));
    free(home);
#endif

    /* Ensure that the selected profile directory exists */
    if (dir.exists() == false)
        dir.mkpath(".");

    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtInputProfile));
    return dir;
}

/*********************************************************************
 * Defaults - !! FALLBACK !!
 *********************************************************************/

void InputOutputMap::loadDefaults()
{
    /* ************************ INPUT *********************************** */
    QString profileName;
    QSettings settings;
    QString plugin;
    QString input;
    QString key;

    for (quint32 i = 0; i < universes(); i++)
    {
        /* Plugin name */
        key = QString("/inputmap/universe%2/plugin/").arg(i);
        plugin = settings.value(key).toString();

        /* Plugin input */
        key = QString("/inputmap/universe%2/input/").arg(i);
        input = settings.value(key).toString();

        /* Input profile */
        key = QString("/inputmap/universe%2/profile/").arg(i);
        profileName = settings.value(key).toString();

        /* Do the mapping */
        if (plugin.length() > 0 && input.length() > 0)
        {
            /* Check that the same plugin & input are not mapped
               to more than one universe at a time. */
            quint32 m = inputMapping(plugin, input.toInt());
            if (m == InputOutputMap::invalidUniverse() || m == i)
            {
                setInputPatch(i, plugin, input.toInt(), profileName);
            }
        }
    }

    /* ************************ OUTPUT *********************************** */
    QString output;
    QString fb_plugin;
    QString feedback;

    for (quint32 i = 0; i < universes(); i++)
    {
        /* Plugin name */
        key = QString("/outputmap/universe%2/plugin/").arg(i);
        plugin = settings.value(key).toString();

        /* Plugin output */
        key = QString("/outputmap/universe%2/output/").arg(i);
        output = settings.value(key).toString();

        /* Feedback plugin name */
        key = QString("/outputmap/universe%2/feedbackplugin/").arg(i);
        fb_plugin = settings.value(key).toString();

        /* Feedback line */
        key = QString("/outputmap/universe%2/feedback/").arg(i);
        feedback = settings.value(key).toString();

        if (plugin.length() > 0 && output.length() > 0)
        {
            /* Check that the same plugin & output are not mapped
               to more than one universe at a time. */
            quint32 m = outputMapping(plugin, output.toInt());
            if (m == InputOutputMap::invalidUniverse() || m == i)
                setOutputPatch(i, plugin, output.toInt());
        }
        if (fb_plugin.length() > 0 && feedback.length() > 0)
        {
            quint32 m = outputMapping(feedback, fb_plugin.toInt());
            if (m == InputOutputMap::invalidUniverse() || m == i)
                setOutputPatch(i, fb_plugin, feedback.toInt(), true);
        }
    }
}

void InputOutputMap::saveDefaults()
{
    /* ************************ INPUT *********************************** */
    QSettings settings;
    QString key;
    QString str;

    for (quint32 i = 0; i < universes(); i++)
    {
        InputPatch* pat = inputPatch(i);
        Q_ASSERT(pat != NULL);

        /* Plugin name */
        key = QString("/inputmap/universe%2/plugin/").arg(i);
        if (pat->plugin() != NULL)
            settings.setValue(key, pat->plugin()->name());
        else
            settings.setValue(key, "None");

        /* Plugin input */
        key = QString("/inputmap/universe%2/input/").arg(i);
        settings.setValue(key, str.setNum(pat->input()));

        /* Input profile */
        key = QString("/inputmap/universe%2/profile/").arg(i);
        settings.setValue(key, pat->profileName());
    }

    /* ************************ OUTPUT *********************************** */

    for (quint32 i = 0; i < universes(); i++)
    {
        OutputPatch* outPatch = outputPatch(i);
        OutputPatch* fbPatch = feedbackPatch(i);
        Q_ASSERT(outPatch != NULL);
        Q_ASSERT(fbPatch != NULL);

        /* Plugin name */
        key = QString("/outputmap/universe%2/plugin/").arg(i);
        settings.setValue(key, outPatch->pluginName());

        /* Plugin output */
        key = QString("/outputmap/universe%2/output/").arg(i);
        settings.setValue(key, str.setNum(outPatch->output()));

        /* Plugin name */
        key = QString("/outputmap/universe%2/feedbackplugin/").arg(i);
        settings.setValue(key, fbPatch->pluginName());

        /* Plugin output */
        key = QString("/outputmap/universe%2/feedback/").arg(i);
        settings.setValue(key, str.setNum(fbPatch->output()));
    }
}

bool InputOutputMap::loadXML(const QDomElement &root)
{
    if (root.tagName() != KXMLIOMap)
    {
        qWarning() << Q_FUNC_INFO << "InputOutputMap node not found";
        return false;
    }

    /** Reset the current universe list and read the new one */
    removeAllUniverses();

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();

        if (tag.tagName() == KXMLQLCUniverse)
        {
            quint32 id = InputOutputMap::invalidUniverse();
            QString name = "";
            if (tag.hasAttribute(KXMLQLCUniverseID))
                id = tag.attribute(KXMLQLCUniverseID).toUInt();
            addUniverse(id);
            Universe *uni = m_universeArray.last();
            uni->loadXML(tag, m_universeArray.count() - 1, this);
        }

        node = node.nextSibling();
    }

    return true;
}

bool InputOutputMap::saveXML(QDomDocument *doc, QDomElement *wksp_root) const
{
    QDomElement root;
    QDomElement tag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);

    /* IO Map Instance entry */
    root = doc->createElement(KXMLIOMap);
    wksp_root->appendChild(root);

    foreach(Universe *uni, m_universeArray)
    {
        uni->saveXML(doc, &root);
    }

    return true;
}






