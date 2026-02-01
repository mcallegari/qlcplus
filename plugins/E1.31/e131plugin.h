/*
  Q Light Controller Plus
  e131plugin.h

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

#ifndef E131PLUGIN_H
#define E131PLUGIN_H

#include <QNetworkAddressEntry>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QString>
#include <QHash>
#include <QFile>

#include "qlcioplugin.h"
#include "e131controller.h"

typedef struct _eio
{
    QNetworkInterface iface;
    QNetworkAddressEntry address;
    E131Controller* controller;
} E131IO;

#define E131_MULTICAST "multicast"
#define E131_MCASTIP "mcastIP"
#define E131_MCASTFULLIP "mcastFullIP"
#define E131_UCASTIP "ucastIP"
#define E131_UCASTPORT "ucastPort"
#define E131_UNIVERSE "universe"
#define E131_TRANSMITMODE "transmitMode"
#define E131_PRIORITY "priority"

#define SETTINGS_IFACE_WAIT_TIME "E131Plugin/ifacewait"

class E131Plugin final : public QLCIOPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCIOPlugin)
    Q_PLUGIN_METADATA(IID QLCIOPlugin_iid)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** @reimp */
    virtual ~E131Plugin();

    /** @reimp */
    void init() override;

    /** @reimp */
    QString name() const override;

    /** @reimp */
    int capabilities() const override;

    /** @reimp */
    QString pluginInfo() const override;

private:
    bool requestLine(quint32 line);

    /*********************************************************************
     * Outputs
     *********************************************************************/
public:
    /** @reimp */
    bool openOutput(quint32 output, quint32 universe) override;

    /** @reimp */
    void closeOutput(quint32 output, quint32 universe) override;

    /** @reimp */
    QStringList outputs() override;

    /** @reimp */
    QString outputInfo(quint32 output) override;

    /** @reimp */
    void writeUniverse(quint32 universe, quint32 output, const QByteArray& data, bool dataChanged) override;

    /*************************************************************************
     * Inputs
     *************************************************************************/
public:
    /** @reimp */
    bool openInput(quint32 input, quint32 universe) override;

    /** @reimp */
    void closeInput(quint32 input, quint32 universe) override;

    /** @reimp */
    QStringList inputs() override;

    /** @reimp */
    QString inputInfo(quint32 input) override;

    /*********************************************************************
     * Configuration
     *********************************************************************/
public:
    /** @reimp */
    void configure() override;

    /** @reimp */
    bool canConfigure() const override;

    /** @reimp */
    void setParameter(quint32 universe, quint32 line, Capability type, QString name, QVariant value) override;

    /** Get a list of the available Input/Output lines */
    QList<E131IO> getIOMapping() const;

private:
    /** Map of the E131 plugin Input/Output lines */
    QList<E131IO> m_IOmapping;

    /** Time to wait (in seconds) for interfaces to be ready */
    int m_ifaceWaitTime;
};

#endif
