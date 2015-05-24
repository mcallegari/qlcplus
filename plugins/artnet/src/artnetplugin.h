/*
  Q Light Controller Plus
  artnetplugin.h

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

#ifndef ARTNETPLUGIN_H
#define ARTNETPLUGIN_H

#include <QNetworkAddressEntry>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QString>
#include <QHash>
#include <QFile>

#include "qlcioplugin.h"
#include "artnetcontroller.h"

typedef struct
{
    QString IPAddress;
    QString MACAddress;
    ArtNetController* controller;
} ArtNetIO;

class ArtNetPlugin : public QLCIOPlugin
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
    virtual ~ArtNetPlugin();

    /** @reimp */
    void init();

    /** @reimp */
    QString name();

    /** @reimp */
    int capabilities() const;

    /** @reimp */
    QString pluginInfo();

    /** @reimp */
    void setParameter(QString name, QVariant &value)
    { Q_UNUSED(name); Q_UNUSED(value); }

private:

    void saveSettings() const;
    void loadSettings();

    /*********************************************************************
     * Outputs
     *********************************************************************/
public:

    /** Specifies how the data is output */
    enum OutputMode
    {
        Full,       /**< Full 512 bytes are output; maximum compatibility, lowest efficiency */
        Patched,    /**< All patched channels are output; fair compatibility and efficiency; NOT IMPLEMENED!!! */
        Minimal     /**< Trailing zeroes are stripped; minimal compatibility, maximum efficiency */
    };

    static QString OutputModeToString(OutputMode mode);

    static OutputMode StringToOutputMode(QString mode);

    /** @reimp */
    bool openOutput(quint32 output);

    /** @reimp */
    void closeOutput(quint32 output);

    /** @reimp */
    QStringList outputs();

    /** @reimp */
    QString outputInfo(quint32 output);

    /** @reimp */
    void writeUniverse(quint32 universe, quint32 output, const QByteArray& data);

    OutputMode outputMode() const;
 
    void setOutputMode(OutputMode mode);

    /*************************************************************************
     * Inputs
     *************************************************************************/
public:
    /** @reimp */
    bool openInput(quint32 input, quint32 universe);

    /** @reimp */
    void closeInput(quint32 input);

    /** @reimp */
    QStringList inputs();

    /** @reimp */
    QString inputInfo(quint32 input);

    /** @reimp */
    void sendFeedBack(quint32 input, quint32 channel, uchar value, const QString& key)
        { Q_UNUSED(input); Q_UNUSED(channel); Q_UNUSED(value); Q_UNUSED(key); }

    /*********************************************************************
     * Configuration
     *********************************************************************/
public:
    /** @reimp */
    void configure();

    /** @reimp */
    bool canConfigure();

    QList<QNetworkAddressEntry> interfaces();

    /** Get a list of the available Input/Output lines */
    QList<ArtNetIO> getIOMapping();

private:
    /** List holding the detected system network interfaces */
    QList<QNetworkAddressEntry> m_netInterfaces;

    /** Map of the ArtNet plugin Input/Output lines */
    QList<ArtNetIO>m_IOmapping;

    OutputMode m_outputMode;
};

#endif
