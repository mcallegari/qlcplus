/*
  Q Light Controller Plus
  oscplugin.h

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

#ifndef OSCPLUGIN_H
#define OSCPLUGIN_H

#include <QNetworkAddressEntry>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QString>
#include <QHash>
#include <QFile>

#include "qlcioplugin.h"
#include "osccontroller.h"

typedef struct _oio
{
    QString IPAddress;
    OSCController* controller;
} OSCIO;

#define OSC_INPUTPORT "inputPort"
#define OSC_FEEDBACKIP "feedbackIP"
#define OSC_FEEDBACKPORT "feedbackPort"
#define OSC_OUTPUTIP "outputIP"
#define OSC_OUTPUTPORT "outputPort"

#define SETTINGS_IFACE_WAIT_TIME "OSCPlugin/ifacewait"

class OSCPlugin : public QLCIOPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCIOPlugin)
    Q_PLUGIN_METADATA(IID QLCIOPlugin_iid)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** @reimp */
    virtual ~OSCPlugin() override;

    /** @reimp */
    void init() override;

    /** @reimp */
    QString name() override;

    /** @reimp */
    int capabilities() const override;

    /** @reimp */
    QString pluginInfo() override;

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

    /** @reimp */
    void sendFeedBack(quint32 universe, quint32 input, quint32 channel, uchar value, const QVariant &params) override;

    /*********************************************************************
     * Configuration
     *********************************************************************/
public:
    /** @reimp */
    void configure() override;

    /** @reimp */
    bool canConfigure() override;

    /** @reimp */
    void setParameter(quint32 universe, quint32 line, Capability type, QString name, QVariant value) override;

    /** Get a list of the available Input/Output lines */
    QList<OSCIO> getIOMapping();

private:
    /** Map of the OSC plugin Input/Output lines */
    QList<OSCIO>m_IOmapping;

    /** Time to wait (in seconds) for interfaces to be ready */
    int m_ifaceWaitTime;
};

#endif
