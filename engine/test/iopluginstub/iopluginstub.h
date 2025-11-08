/*
  Q Light Controller
  iopluginstub.h

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

#ifndef IOPLUGINSTUB_H
#define IOPLUGINSTUB_H

#include <QStringList>
#include <QString>
#include <QList>

#include "qlcioplugin.h"

class IOPluginStub : public QLCIOPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCIOPlugin)
    Q_PLUGIN_METADATA(IID QLCIOPlugin_iid)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** @reimp */
    virtual ~IOPluginStub() override;

    /** @reimp */
    void init() override;

    /** @reimp */
    QString name() override;

    /** @reimp */
    int capabilities() const override;

    /** @reimp */
    QString pluginInfo() override;

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

public:
    /** List of outputs that have been opened */
    QList <quint32> m_openOutputs;

    /** Fake universe buffer */
    QByteArray m_universe;

    /*********************************************************************
     * Inputs
     *********************************************************************/
public:
    /** @reimp */
    bool openInput(quint32 input, quint32 universe) override;

    /** @reimp */
    void closeInput(quint32 input, quint32 universe) override;

    /** @reimp */
    QStringList inputs() override;

    /** @reimp */
    QString inputInfo(quint32 input) override;

    /** Tell the plugin to emit valueChanged signal */
    void emitValueChanged(quint32 universe, quint32 input, quint32 channel, uchar value)
    {
        emit valueChanged(universe, input, channel, value);
    }

public:
    /** List of inputs that have been opened */
    QList <quint32> m_openInputs;

    /*********************************************************************
     * Configuration
     *********************************************************************/
public:
    /** @reimp */
    void configure() override;

    /** @reimp */
    bool canConfigure() override;

public:
    int m_configureCalled;
    bool m_canConfigure;
};

#endif
