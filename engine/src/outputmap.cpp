/*
  Q Light Controller
  outputmap.cpp

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

#include <QCoreApplication>
#include <QPluginLoader>
#include <QByteArray>
#include <QSettings>
#include <QString>
#include <QDebug>
#include <QList>
#include <QtXml>
#include <QDir>

#include "hotplugmonitor.h"
#include "qlcioplugin.h"
#include "outputpatch.h"
#include "grandmaster.h"
#include "qlcconfig.h"
#include "outputmap.h"
#include "universe.h"
#include "qlci18n.h"
#include "qlcfile.h"
#include "doc.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

OutputMap::OutputMap(Doc* doc, quint32 universes)
    : QObject(doc)
    , m_universes(universes)
    , m_blackout(false)
    , m_universeChanged(false)
{
    m_grandMaster = new GrandMaster(this);
    for (quint32 i = 0; i < universes; i++)
        m_universeArray.append(new Universe(m_grandMaster, this));

    initPatch();

    connect(doc->ioPluginCache(), SIGNAL(pluginConfigurationChanged(QLCIOPlugin*)),
            this, SLOT(slotPluginConfigurationChanged(QLCIOPlugin*)));
}

OutputMap::~OutputMap()
{
    for (int i = 0; i < m_universeArray.size(); i++)
        m_universeArray.takeAt(i);

    delete m_grandMaster;

    for (quint32 i = 0; i < m_universes; i++)
    {
        delete m_patch[i];
        m_patch[i] = NULL;
    }
}

Doc* OutputMap::doc() const
{
    return qobject_cast<Doc*> (parent());
}

/*****************************************************************************
 * Blackout
 *****************************************************************************/

bool OutputMap::toggleBlackout()
{
    if (m_blackout == true)
        setBlackout(false);
    else
        setBlackout(true);

    return m_blackout;
}

void OutputMap::setBlackout(bool blackout)
{
    /* Don't do blackout twice */
    if (m_blackout == blackout)
        return;
    m_blackout = blackout;

    if (blackout == true)
    {
        QByteArray zeros(512, 0);
        for (quint32 i = 0; i < m_universes; i++)
            m_patch[i]->dump(zeros);
    }
    else
    {
        /* Force writing of values back to the plugins */
        m_universeChanged = true;
    }

    emit blackoutChanged(m_blackout);
}

bool OutputMap::blackout() const
{
    return m_blackout;
}

/*****************************************************************************
 * Universes
 *****************************************************************************/

bool OutputMap::addUniverse()
{
    m_universeMutex.lock();
    m_universeArray.append(new Universe(m_grandMaster));
    m_universeMutex.unlock();
    return true;
}

bool OutputMap::removeUniverse()
{
    m_universeMutex.lock();
    Universe *delUni = m_universeArray.takeLast();
    delete delUni;
    m_universeMutex.unlock();

    return true;
}

int OutputMap::universesCount()
{
    return m_universeArray.count();
}

QList<Universe*> OutputMap::claimUniverses()
{
    m_universeMutex.lock();
    return m_universeArray;
}

void OutputMap::releaseUniverses(bool changed)
{
    m_universeChanged = changed;
    m_universeMutex.unlock();
}

void OutputMap::dumpUniverses()
{
    m_universeMutex.lock();
    if (m_blackout == false)
    {
        int i = 0;
        foreach (Universe *universe, m_universeArray)
        {
            if (universe->hasChanged())
            {
                const QByteArray postGM = universe->postGMValues()->mid(0);
                m_patch[i++]->dump(postGM);

                m_universeMutex.unlock();
                emit universesWritten(postGM);
                m_universeMutex.lock();
            }
        }
    }
    m_universeMutex.unlock();
}

void OutputMap::resetUniverses()
{
    m_universeMutex.lock();
    for (int i = 0; i < m_universeArray.size(); i++)
        m_universeArray.at(i)->reset();
    m_universeMutex.unlock();

    /* Reset Grand Master parameters */
    setGrandMasterValue(255);
    setGrandMasterValueMode(GrandMaster::GMReduce);
    setGrandMasterChannelMode(GrandMaster::GMIntensity);
}

/*********************************************************************
 * Grand Master
 *********************************************************************/

void OutputMap::setGrandMasterChannelMode(GrandMaster::GMChannelMode mode)
{
    Q_ASSERT(m_grandMaster != NULL);

    if(m_grandMaster->gMChannelMode() != mode)
    {
        m_grandMaster->setGMChannelMode(mode);
        m_universeChanged = true;
    }
}

GrandMaster::GMChannelMode OutputMap::grandMasterChannelMode()
{
    Q_ASSERT(m_grandMaster != NULL);

    GrandMaster::GMChannelMode mode = m_grandMaster->gMChannelMode();
    return mode;
}

void OutputMap::setGrandMasterValueMode(GrandMaster::GMValueMode mode)
{
    Q_ASSERT(m_grandMaster != NULL);

    if(m_grandMaster->gMValueMode() != mode)
    {
        m_grandMaster->setGMValueMode(mode);
        m_universeChanged = true;
    }

    emit grandMasterValueModeChanged(mode);
}

GrandMaster::GMValueMode OutputMap::grandMasterValueMode()
{
    Q_ASSERT(m_grandMaster != NULL);

    return m_grandMaster->gMValueMode();
}

void OutputMap::setGrandMasterValue(uchar value)
{
    Q_ASSERT(m_grandMaster != NULL);

    if (m_grandMaster->gMValue() != value)
    {
        m_grandMaster->setGMValue(value);
        m_universeChanged = true;
    }

    if (m_universeChanged == true)
        emit grandMasterValueChanged(value);
}

uchar OutputMap::grandMasterValue()
{
    Q_ASSERT(m_grandMaster != NULL);

    return m_grandMaster->gMValue();
}

/*****************************************************************************
 * Patch
 *****************************************************************************/

void OutputMap::initPatch()
{
    for (quint32 i = 0; i < m_universes; i++)
        m_patch.insert(i, new OutputPatch(this));
    for (quint32 i = 0; i < m_universes; i++)
        m_fb_patch.insert(i, new OutputPatch(this));
}

quint32 OutputMap::invalidUniverse()
{
    return UINT_MAX;
}

quint32 OutputMap::universes() const
{
    return m_universes;
}

bool OutputMap::setPatch(quint32 universe, const QString& pluginName,
                         quint32 output, bool isFeedback)
{
    if (universe >= universes())
    {
        qWarning() << Q_FUNC_INFO << "Universe" << universe << "out of bounds.";
        return false;
    }

    m_universeMutex.lock();
    if (isFeedback == false)
    m_patch[universe]->set(doc()->ioPluginCache()->plugin(pluginName), output);
    else
        m_fb_patch[universe]->set(doc()->ioPluginCache()->plugin(pluginName), output);
    m_universeMutex.unlock();

    return true;
}

OutputPatch* OutputMap::patch(quint32 universe) const
{
    if (universe < universes())
        return m_patch[universe];
    else
        return NULL;
}

OutputPatch* OutputMap::feedbackPatch(quint32 universe) const
{
    if (universe < universes())
        return m_fb_patch[universe];
    else
        return NULL;
}


QStringList OutputMap::universeNames() const
{
    QStringList list;
    for (quint32 i = 0; i < universes(); i++)
    {
        OutputPatch* p(patch(i));
        Q_ASSERT(p != NULL);
        list << QString("%1: %2 (%3)").arg(i + 1)
                                      .arg(p->pluginName())
                                      .arg(p->outputName());
    }

    return list;
}

quint32 OutputMap::mapping(const QString& pluginName, quint32 output) const
{
    for (quint32 uni = 0; uni < universes(); uni++)
    {
        const OutputPatch* p = patch(uni);
        if (p->pluginName() == pluginName && p->output() == output)
            return uni;
    }

    return QLCIOPlugin::invalidLine();
}

/*****************************************************************************
 * Plugins
 *****************************************************************************/

QStringList OutputMap::pluginNames()
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

QStringList OutputMap::pluginOutputs(const QString& pluginName)
{
    QLCIOPlugin* op = doc()->ioPluginCache()->plugin(pluginName);
    if (op == NULL)
        return QStringList();
    else
        return op->outputs();
}

bool OutputMap::pluginSupportsFeedback(const QString& pluginName)
{
    QLCIOPlugin* outputPlugin = doc()->ioPluginCache()->plugin(pluginName);
    if (outputPlugin != NULL)
        return (outputPlugin->capabilities() & QLCIOPlugin::Feedback) > 0;
    else
        return false;
}

void OutputMap::configurePlugin(const QString& pluginName)
{
    QLCIOPlugin* outputPlugin = doc()->ioPluginCache()->plugin(pluginName);
    if (outputPlugin != NULL)
        outputPlugin->configure();
}

bool OutputMap::canConfigurePlugin(const QString& pluginName)
{
    QLCIOPlugin* outputPlugin = doc()->ioPluginCache()->plugin(pluginName);
    if (outputPlugin != NULL)
        return outputPlugin->canConfigure();
    else
        return false;
}

QString OutputMap::pluginStatus(const QString& pluginName, quint32 output)
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

bool OutputMap::feedBack(quint32 universe, quint32 channel, uchar value, const QString& key)
{
    if (universe >= quint32(m_fb_patch.size()))
        return false;

    OutputPatch* patch = m_fb_patch[universe];
    Q_ASSERT(patch != NULL);

    if (patch->plugin() != NULL && patch->output() != QLCIOPlugin::invalidLine())
    {
        patch->plugin()->sendFeedBack(patch->output(), channel, value, key);
        return true;
    }
    else
    {
        return false;
    }
}

void OutputMap::slotPluginConfigurationChanged(QLCIOPlugin* plugin)
{
    for (quint32 i = 0; i < universes(); i++)
    {
        OutputPatch* op = patch(i);
        Q_ASSERT(op != NULL);
        if (op->plugin() == plugin)
        {
            m_universeMutex.lock();
            op->reconnect();
            m_universeMutex.unlock();
        }
    }

    emit pluginConfigurationChanged(plugin->name());
}

/*****************************************************************************
 * Defaults
 *****************************************************************************/

void OutputMap::loadDefaults()
{
    QSettings settings;
    QString plugin;
    QString output;
    QString fb_plugin;
    QString feedback;
    QString key;

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
            quint32 m = mapping(plugin, output.toInt());
            if (m == QLCChannel::invalid() || m == i)
                setPatch(i, plugin, output.toInt());
        }
        if (fb_plugin.length() > 0 && feedback.length() > 0)
        {
            quint32 m = mapping(feedback, fb_plugin.toInt());
            if (m == QLCChannel::invalid() || m == i)
                setPatch(i, fb_plugin, feedback.toInt(), true);
        }
    }
}

void OutputMap::saveDefaults()
{
    QSettings settings;
    QString key;
    QString str;

    for (quint32 i = 0; i < universes(); i++)
    {
        OutputPatch* outputPatch = patch(i);
        OutputPatch* fbPatch = feedbackPatch(i);
        Q_ASSERT(outputPatch != NULL);
        Q_ASSERT(fbPatch != NULL);

        /* Plugin name */
        key = QString("/outputmap/universe%2/plugin/").arg(i);
        settings.setValue(key, outputPatch->pluginName());

        /* Plugin output */
        key = QString("/outputmap/universe%2/output/").arg(i);
        settings.setValue(key, str.setNum(outputPatch->output()));

        /* Plugin name */
        key = QString("/outputmap/universe%2/feedbackplugin/").arg(i);
        settings.setValue(key, fbPatch->pluginName());

        /* Plugin output */
        key = QString("/outputmap/universe%2/feedback/").arg(i);
        settings.setValue(key, str.setNum(fbPatch->output()));
    }
}

