/*
  Q Light Controller
  ioplugincache.cpp

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
#include <QDebug>

#if defined(WIN32) || defined(Q_OS_WIN)
#   include <windows.h>
#endif

#include "hotplugmonitor.h"
#include "ioplugincache.h"
#include "qlcioplugin.h"
#include "qlcconfig.h"
#include "qlcfile.h"

IOPluginCache::IOPluginCache(QObject* parent)
    : QObject(parent)
{
    qDebug() << Q_FUNC_INFO;
}

IOPluginCache::~IOPluginCache()
{
    qDebug() << Q_FUNC_INFO;
    while (m_plugins.isEmpty() == false)
        delete m_plugins.takeFirst();
}

void IOPluginCache::load(const QDir& dir)
{
    qDebug() << Q_FUNC_INFO << dir.path();

    /* Check that we can access the directory */
    if (dir.exists() == false || dir.isReadable() == false)
        return;

    /* Loop thru all files in the directory */
    QStringListIterator it(dir.entryList());
    while (it.hasNext() == true)
    {
        /* Attempt to load a plugin from the path */
        QString fileName(it.next());
        QString path = dir.absoluteFilePath(fileName);
        QPluginLoader loader(path, this);
        QLCIOPlugin* ptr = qobject_cast<QLCIOPlugin*> (loader.instance());
        if (ptr != NULL)
        {
            /* Check for duplicates */
            if (plugin(ptr->name()) == NULL)
            {
                /* New plugin. Append and init. */
                qDebug() << "Loaded I/O plugin" << ptr->name() << "from" << fileName;
                emit pluginLoaded(ptr->name());
                ptr->init();
                m_plugins << ptr;
                connect(ptr, SIGNAL(configurationChanged()),
                        this, SLOT(slotConfigurationChanged()));
                HotPlugMonitor::connectListener(ptr);
                // QLCi18n::loadTranslation(p->name().replace(" ", "_"));
            }
            else
            {
                /* Duplicate plugin. Unload it. */
                qWarning() << Q_FUNC_INFO << "Discarded duplicate I/O plugin"
                           << ptr->name() << "in" << path;
                loader.unload();
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << fileName << "doesn't contain an I/O plugin:"
                       << loader.errorString();
            loader.unload();
        }
    }
}

QList <QLCIOPlugin*> IOPluginCache::plugins() const
{
    return m_plugins;
}

QLCIOPlugin* IOPluginCache::plugin(const QString& name) const
{
    QListIterator <QLCIOPlugin*> it(m_plugins);
    while (it.hasNext() == true)
    {
        QLCIOPlugin* ptr(it.next());
        if (ptr->name() == name)
            return ptr;
    }

    return NULL;
}

void IOPluginCache::slotConfigurationChanged()
{
    qDebug() << Q_FUNC_INFO;

    QLCIOPlugin* plugin = static_cast<QLCIOPlugin*> (QObject::sender());
    if (plugin != NULL) // 3rd party plugins might not behave correctly
        emit pluginConfigurationChanged(plugin);
}

QDir IOPluginCache::systemPluginDirectory()
{
    return QLCFile::systemDirectory(PLUGINDIR, KExtPlugin);
}

