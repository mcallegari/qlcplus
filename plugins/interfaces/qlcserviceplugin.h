/*
  Q Light Controller Plus
  qlcserviceplugin.h

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

#ifndef QLCSERVICEPLUGIN_H
#define QLCSERVICEPLUGIN_H

#include <QtPlugin>
#include <QObject>
#include <QString>

/** @addtogroup engine Engine
 * @{
 */

/** Current version of the host <-> service plugin contract (QLCServiceHost).
 *  Incremented whenever the host gains functionality a service may need.
 *  Breaking changes are instead signalled by a new QLCServicePlugin_iid. */
#define QLC_SERVICE_HOST_API_VERSION 1

/**
 * Minimal, feature-agnostic object that the engine hands to every service
 * plugin in QLCServicePlugin::init(), so that a service never needs to link
 * the QLC+ engine library.
 *
 * It deliberately knows nothing about any specific feature (I/O, fixtures,
 * etc.). Anything feature specific is part of that feature's own interface
 * and is provided by the code that consumes the feature.
 */
class QLCServiceHost
{
public:
    virtual ~QLCServiceHost() { }

    /** The version of this host API (QLC_SERVICE_HOST_API_VERSION) */
    virtual int hostApiVersion() const = 0;

    /** The QLC+ application version string */
    virtual QString hostVersion() const = 0;

    /** Get another loaded service by its id, or nullptr if not available */
    virtual QObject *service(const QString &id) = 0;

    /** Get a value from the application settings, namespaced under
     *  "services/<key>" */
    virtual QString settingsValue(const QString &key) = 0;

    /** Get (and create if needed) a writable directory reserved to
     *  the service with the given id */
    virtual QString userDataDir(const QString &serviceId) = 0;

    /** Write a message to the application log.
     *  @param level 0 = debug, 1 = info, 2 = warning */
    virtual void log(int level, const QString &message) = 0;
};

/**
 * Interface that every service plugin must implement. A service plugin is
 * loaded by the engine (ServiceLoader) from <PLUGINDIR>/services and extends
 * QLC+ with one feature (e.g. RDM). One plugin provides exactly one service.
 *
 * This is NOT an I/O plugin (see QLCIOPlugin).
 *
 * The plugin class must inherit QObject and this interface, and declare:
 *   Q_OBJECT
 *   Q_PLUGIN_METADATA(IID QLCServicePlugin_iid)
 *   Q_INTERFACES(QLCServicePlugin)
 */
class QLCServicePlugin
{
public:
    virtual ~QLCServicePlugin() { }

    /** Unique id of the provided service, e.g. "rdm" */
    virtual QString serviceId() const = 0;

    /** Human readable name of the service */
    virtual QString name() const = 0;

    /** Version of the plugin (informational only) */
    virtual QString version() const = 0;

    /** Oldest QLCServiceHost::hostApiVersion() this plugin can run on */
    virtual int minHostApiVersion() const = 0;

    /** Called once after all the services have been loaded.
     *  Return false to refuse to be used. */
    virtual bool init(QLCServiceHost *host) = 0;

    /** The service object, to be cast by the consumer to the
     *  feature interface it expects (e.g. QLCRDMService) */
    virtual QObject *service() = 0;

    /** Called before the plugin gets unloaded */
    virtual void shutdown() = 0;
};

#define QLCServicePlugin_iid "org.qlcplus.QLCServicePlugin/1"

Q_DECLARE_INTERFACE(QLCServicePlugin, QLCServicePlugin_iid)

/** @} */

#endif
