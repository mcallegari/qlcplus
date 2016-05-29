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

typedef struct
{
    QString IPAddress;
    OSCController* controller;
} OSCIO;

#define OSC_INPUTPORT "inputPort"
#define OSC_FEEDBACKIP "feedbackIP"
#define OSC_FEEDBACKPORT "feedbackPort"
#define OSC_OUTPUTIP "outputIP"
#define OSC_OUTPUTPORT "outputPort"


class OSCPlugin : public QLCIOPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCIOPlugin)
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID QLCIOPlugin_iid)
#endif

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** @reimp */
    virtual ~OSCPlugin();

    /** @reimp */
    void init();

    /** @reimp */
    QString name();

    /** @reimp */
    int capabilities() const;

    /** @reimp */
    QString pluginInfo();

private:
    bool requestLine(quint32 line, int retries);

    /*********************************************************************
     * Outputs
     *********************************************************************/
public:
    /** @reimp */
    bool openOutput(quint32 output, quint32 universe);

    /** @reimp */
    void closeOutput(quint32 output, quint32 universe);

    /** @reimp */
    QStringList outputs();

    /** @reimp */
    QString outputInfo(quint32 output);

    /** @reimp */
    void writeUniverse(quint32 universe, quint32 output, const QByteArray& data);

    /*************************************************************************
     * Inputs
     *************************************************************************/
public:
    /** @reimp */
    bool openInput(quint32 input, quint32 universe);

    /** @reimp */
    void closeInput(quint32 input, quint32 universe);

    /** @reimp */
    QStringList inputs();

    /** @reimp */
    QString inputInfo(quint32 input);

    /** @reimp */
    void sendFeedBack(quint32 universe, quint32 input, quint32 channel, uchar value, const QString& key);

    /*********************************************************************
     * Configuration
     *********************************************************************/
public:
    /** @reimp */
    void configure();

    /** @reimp */
    bool canConfigure();

    /** @reimp */
    void setParameter(quint32 universe, quint32 line, Capability type, QString name, QVariant value);

    /** Get a list of the available Input/Output lines */
    QList<OSCIO> getIOMapping();

private:
    /** Map of the OSC plugin Input/Output lines */
    QList<OSCIO>m_IOmapping;
};

#endif
