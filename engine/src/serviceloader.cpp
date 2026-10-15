/*
  Q Light Controller Plus
  serviceloader.cpp

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

#include <QStandardPaths>
#include <QPluginLoader>
#include <QSettings>
#include <QDebug>

#include "serviceloader.h"
#include "qlcserviceplugin.h"
#include "qlcconfig.h"
#include "qlcfile.h"

/** Engine side implementation of the interface handed to the services */
class ServiceHost final : public QLCServiceHost
{
public:
    explicit ServiceHost(const ServiceLoader *loader)
        : m_loader(loader)
    {
    }

    int hostApiVersion() const override
    {
        return QLC_SERVICE_HOST_API_VERSION;
    }

    QString hostVersion() const override
    {
        return QString(APPVERSION);
    }

    QObject *service(const QString &id) override
    {
        return m_loader->service(id);
    }

    QString settingsValue(const QString &key) override
    {
        QSettings settings;
        return settings.value(QString("services/%1").arg(key)).toString();
    }

    QString userDataDir(const QString &serviceId) override
    {
        QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
        QString path = QString("services/%1").arg(serviceId);
        dir.mkpath(path);
        return dir.absoluteFilePath(path);
    }

    void log(int level, const QString &message) override
    {
        if (level <= 0)
            qDebug().noquote() << "[Service]" << message;
        else if (level == 1)
            qInfo().noquote() << "[Service]" << message;
        else
            qWarning().noquote() << "[Service]" << message;
    }

private:
    const ServiceLoader *m_loader;
};

ServiceLoader::ServiceLoader(QObject *parent)
    : QObject(parent)
    , m_host(new ServiceHost(this))
{
}

ServiceLoader::~ServiceLoader()
{
    while (m_entries.isEmpty() == false)
    {
        Entry entry = m_entries.takeLast();
        entry.plugin->shutdown();
        entry.loader->unload();
        delete entry.loader;
    }
    delete m_host;
}

void ServiceLoader::load(const QDir &dir)
{
    qDebug() << Q_FUNC_INFO << dir.path();

    /* Check that we can access the directory */
    if (dir.exists() == false || dir.isReadable() == false)
        return;

    /* Pass 1: instantiate the plugins and register their ids */
    QList<Entry> loaded;
    const QStringList files = dir.entryList();
    for (const QString &fileName : files)
    {
        QString path = dir.absoluteFilePath(fileName);
        QPluginLoader *loader = new QPluginLoader(path, this);
        QLCServicePlugin *ptr = qobject_cast<QLCServicePlugin*>(loader->instance());

        if (ptr == nullptr)
        {
            qWarning() << Q_FUNC_INFO << fileName << "doesn't contain a service plugin:"
                       << loader->errorString();
            loader->unload();
            delete loader;
            continue;
        }

        if (ptr->minHostApiVersion() > QLC_SERVICE_HOST_API_VERSION)
        {
            qWarning() << Q_FUNC_INFO << "Service" << ptr->serviceId() << "in" << fileName
                       << "requires host API" << ptr->minHostApiVersion()
                       << "but this is" << QLC_SERVICE_HOST_API_VERSION;
            loader->unload();
            delete loader;
            continue;
        }

        if (hasService(ptr->serviceId()))
        {
            qWarning() << Q_FUNC_INFO << "Discarded duplicate service"
                       << ptr->serviceId() << "in" << path;
            loader->unload();
            delete loader;
            continue;
        }

        Entry entry = { loader, ptr };
        m_entries.append(entry);
        loaded.append(entry);
    }

    /* Pass 2: initialize them. Now every service can see the others */
    for (const Entry &entry : loaded)
    {
        QString id = entry.plugin->serviceId();

        if (entry.plugin->init(m_host) == false)
        {
            qWarning() << Q_FUNC_INFO << "Service" << id << "refused to initialize";
            m_entries.removeOne(entry);
            entry.loader->unload();
            delete entry.loader;
            continue;
        }

        qInfo() << "Loaded service plugin" << entry.plugin->name()
                << entry.plugin->version() << "(" << id << ")";
        emit serviceAvailable(id);
    }
}

QList<QLCServicePlugin*> ServiceLoader::plugins() const
{
    QList<QLCServicePlugin*> list;
    for (const Entry &entry : m_entries)
        list.append(entry.plugin);
    return list;
}

bool ServiceLoader::hasService(const QString &id) const
{
    return service(id) != nullptr;
}

QObject *ServiceLoader::service(const QString &id) const
{
    for (const Entry &entry : m_entries)
    {
        if (entry.plugin->serviceId() == id)
            return entry.plugin->service();
    }
    return nullptr;
}

QDir ServiceLoader::systemServiceDirectory()
{
    return QLCFile::systemDirectory(QString("%1/services").arg(PLUGINDIR), KExtPlugin);
}
