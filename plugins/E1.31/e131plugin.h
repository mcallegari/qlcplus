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

typedef struct
{
    QNetworkInterface interface;
    QNetworkAddressEntry address;
    E131Controller* controller;
} E131IO;

#define E131_MULTICAST "multicast"
#define E131_MCASTIP "mcastIP"
#define E131_UCASTIP "ucastIP"
#define E131_UCASTPORT "ucastPort"
#define E131_UNIVERSE "universe"
#define E131_TRANSMITMODE "transmitMode"
#define E131_PRIORITY "priority"

class E131Plugin : public QLCIOPlugin
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
    virtual ~E131Plugin();

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
    QList<E131IO> getIOMapping();

private:
    /** Map of the E131 plugin Input/Output lines */
    QList<E131IO> m_IOmapping;
};

#endif
