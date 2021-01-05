/*
  Q Light Controller
  ioplugincache.h

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

#ifndef IOPLUGINCACHE_H
#define IOPLUGINCACHE_H

#include <QObject>
#include <QDir>

class QLCIOPlugin;

#define SETTINGS_HOTPLUG "inputmanager/hotplug"

/** @addtogroup engine Engine
 * @{
 */

class IOPluginCache : public QObject
{
    Q_OBJECT

public:
    IOPluginCache(QObject* parent);
    ~IOPluginCache();

    /** Load plugins from the given directory. */
    void load(const QDir& dir);

    /** Get a list of available I/O plugins. */
    QList <QLCIOPlugin*> plugins() const;

    /** Get an I/O plugin by its name. */
    QLCIOPlugin* plugin(const QString& name) const;

    /** Get the system plugin directory. */
    static QDir systemPluginDirectory();

signals:
    void pluginConfigurationChanged(QLCIOPlugin* plugin);
    void pluginLoaded(const QString& name);

private slots:
    void slotConfigurationChanged();

private:
    QList <QLCIOPlugin*> m_plugins;
};

/** @} */

#endif
