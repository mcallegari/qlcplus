/*
  Q Light Controller
  ioplugincache.cpp

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
#include <QDebug>

#ifdef WIN32
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
    QDir dir;
#ifdef __APPLE__
    dir.setPath(QString("%1/../%2").arg(QCoreApplication::applicationDirPath())
                                   .arg(PLUGINDIR));
#else
    dir.setPath(PLUGINDIR);
#endif

    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("*%1").arg(KExtPlugin));

    return dir;
}

