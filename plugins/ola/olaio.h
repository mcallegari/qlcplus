/*
  Q Light Controller
  olaio.h

  Copyright (c) Simon Newton
                Heikki Junnila

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

#ifndef OLAIO_H
#define OLAIO_H

#include <QObject>
#include <QDebug>
#include <QList>
#include <ola/Logging.h>

#include "qlcioplugin.h"
#include "olaoutthread.h"

class ConfigureOlaIO;

class OlaIO : public QLCIOPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCIOPlugin)
    Q_PLUGIN_METADATA(IID QLCIOPlugin_iid)

    friend class ConfigureOlaIO;

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    /** @reimp */
    ~OlaIO();

    /** @reimp */
    void init() override;

    /** @reimp */
    QString name() override;

    /** @reimp */
    int capabilities() const override;

    /** @reimp */
    QString pluginInfo() override;

private:
    /** Is the plugin currently running as a stand alone daemon. */
    bool isServerEmbedded() const;

    /** Set whether or not to run as a standalone daemon. */
    void setServerEmbedded(bool embedServer);

    /************************************************************************
     * Outputs
     ************************************************************************/
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

private:
    /** Return the output: universe mapping */
    QList <uint> outputMapping() const;

    /**
     * Set the OLA universe for an output
     * @param output the id of the output to change
     * @param universe the OLA universe id
     */
    void setOutputUniverse(quint32 output, unsigned int universe);

    /************************************************************************
     * Configuration
     ************************************************************************/
public:
    /** @reimp */
    void configure() override;

    /** @reimp */
    bool canConfigure() override;

    /** @reimp */
    void setParameter(quint32 universe, quint32 line, Capability type, QString name, QVariant value) override;

private:
    QString m_configDir;
    OlaOutThread *m_thread;
    QList <uint> m_outputs;
    bool m_embedServer;
};

#endif
