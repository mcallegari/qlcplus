/*
  Q Light Controller Plus
  serviceloader.h

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

#ifndef SERVICELOADER_H
#define SERVICELOADER_H

#include <QObject>
#include <QDir>
#include <QList>

class QPluginLoader;
class QLCServicePlugin;
class QLCServiceHost;

/** @addtogroup engine Engine
 * @{
 */

/**
 * Loads the service plugins (see QLCServicePlugin) from <PLUGINDIR>/services
 * and gives access to the services they provide by id. The loader doesn't
 * know anything about what a service does: consumers cast the object
 * returned by service() to the feature interface they expect.
 *
 * Service plugins are optional: when none is found, service() returns nullptr
 * and consumers must cope with that.
 */
class ServiceLoader final : public QObject
{
    Q_OBJECT

public:
    ServiceLoader(QObject *parent);
    ~ServiceLoader();

    /** Load the service plugins found in the given directory.
     *  All the plugins are instantiated first, then initialized,
     *  so that a service can look up other services in init(). */
    void load(const QDir &dir);

    /** Get the list of loaded service plugins */
    QList<QLCServicePlugin*> plugins() const;

    /** Check if a service with the given id has been loaded */
    bool hasService(const QString &id) const;

    /** Get the service object with the given id, or nullptr if not loaded */
    QObject *service(const QString &id) const;

    /** Typed convenience wrapper around service() */
    template<class T>
    T *service(const QString &id) const
    {
        return qobject_cast<T*>(service(id));
    }

    /** Get the directory where service plugins are looked up:
     *  PLUGINDIR + "/services" */
    static QDir systemServiceDirectory();

signals:
    /** Emitted when a service has been loaded and initialized */
    void serviceAvailable(const QString &id);

private:
    struct Entry
    {
        QPluginLoader *loader;
        QLCServicePlugin *plugin;

        bool operator==(const Entry &other) const { return plugin == other.plugin; }
    };

    QList<Entry> m_entries;
    QLCServiceHost *m_host;
};

/** @} */

#endif
