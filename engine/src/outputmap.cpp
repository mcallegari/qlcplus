/*
  Q Light Controller
  outputmap.cpp

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
#include "universearray.h"
#include "qlcioplugin.h"
#include "outputpatch.h"
#include "qlcconfig.h"
#include "outputmap.h"
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
    , m_universeArray(new UniverseArray(512 * universes))
    , m_universeChanged(false)
{
    initPatch();

    connect(doc->ioPluginCache(), SIGNAL(pluginConfigurationChanged(QLCIOPlugin*)),
            this, SLOT(slotPluginConfigurationChanged(QLCIOPlugin*)));
}

OutputMap::~OutputMap()
{
    delete m_universeArray;
    m_universeArray = NULL;

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
 * Values
 *****************************************************************************/

UniverseArray* OutputMap::claimUniverses()
{
    m_universeMutex.lock();
    return m_universeArray;
}

void OutputMap::releaseUniverses(bool changed)
{
    m_universeChanged = changed;
    m_universeMutex.unlock();
}

void OutputMap::setGrandMasterChannelMode(UniverseArray::GMChannelMode mode)
{
    bool changed = false;
    UniverseArray* ua = claimUniverses();
    if(ua->gMChannelMode() != mode)
    {
        ua->setGMChannelMode(mode);
        changed = true;
    }
    releaseUniverses(changed);
}

UniverseArray::GMChannelMode OutputMap::grandMasterChannelMode()
{
    UniverseArray* ua = claimUniverses();
    UniverseArray::GMChannelMode mode = ua->gMChannelMode();
    releaseUniverses(false);
    return mode;
}

void OutputMap::setGrandMasterValueMode(UniverseArray::GMValueMode mode)
{
    bool changed = false;
    UniverseArray* ua = claimUniverses();
    if(ua->gMValueMode() != mode)
    {
        ua->setGMValueMode(mode);
        changed = true;
    }
    releaseUniverses(changed);

    emit grandMasterValueModeChanged(mode);
}

UniverseArray::GMValueMode OutputMap::grandMasterValueMode()
{
    UniverseArray* ua = claimUniverses();
    UniverseArray::GMValueMode mode = ua->gMValueMode();
    releaseUniverses(false);
    return mode;
}

void OutputMap::setGrandMasterValue(uchar value)
{
    bool changed = false;

    UniverseArray* ua = claimUniverses();
    if (ua->gMValue() != value)
    {
        ua->setGMValue(value);
        changed = true;
    }
    releaseUniverses(changed);

    if (changed == true)
        emit grandMasterValueChanged(value);
}

uchar OutputMap::grandMasterValue()
{
    UniverseArray* ua = claimUniverses();
    uchar value = ua->gMValue();
    releaseUniverses(false);
    return value;
}

void OutputMap::dumpUniverses()
{
    QByteArray ba;

    m_universeMutex.lock();
    if (m_universeChanged == true && m_blackout == false)
    {
        const QByteArray* postGM = m_universeArray->postGMValues();
        for (quint32 i = 0; i < m_universes; i++)
            m_patch[i]->dump(postGM->mid(i * 512, 512));

        // Grab a copy of universe values to prevent timer thread blocking
        ba = *postGM;

        m_universeChanged = false;
    }
    m_universeMutex.unlock();

    // Emit new values
    if (ba.isEmpty() == false)
        emit universesWritten(ba);
}

void OutputMap::resetUniverses()
{
    claimUniverses();
    m_universeArray->reset();
    releaseUniverses();

    /* Reset Grand Master parameters */
    setGrandMasterValue(255);
    setGrandMasterValueMode(UniverseArray::GMReduce);
    setGrandMasterChannelMode(UniverseArray::GMIntensity);
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
