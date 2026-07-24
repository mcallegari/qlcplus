/*
  Q Light Controller Plus
  idn.h

  Copyright (c) Daniel Schröder

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

#ifndef IDN_H
#define IDN_H

#include <QString>
#include <QSettings>
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QHostAddress>
#include <QNetworkInterface>

#include "idncontroller.h"
#include "qlcioplugin.h"

#define IDN_MAX_CLIENTS 8

typedef struct
{
    QNetworkAddressEntry address;
    IdnController* controller;
}IdnOutput;

#define MAX_OUTPUT 8

class IdnPlugin : public QLCIOPlugin
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
    virtual ~IdnPlugin();

    /** @reimp */
    void init() override;

    /** @reimp */
    QString name() const override;

    /** @reimp */
    int capabilities() const override;

    /** @reimp */
    QString pluginInfo() const override;

    bool requestLine(quint32 line, int retries);

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

    /*********************************************************************
     * Settings
     *********************************************************************/
private:
      /** Map of the IDN clients discovered */
    QHash<IdnHostInfo, IdnSettings> m_manualClients;
    QHash<IdnHostInfo, IdnSettings> loadSettings();


     /*********************************************************************
     * Mapping
     *********************************************************************/
private:
    /** Map of the IDN plugin Output lines */
    QList<IdnOutput> m_Outputmapping;
    void reconfigureController();


    /**********************************************************************
     * Configuration
     **********************************************************************/
public:
    /** @reimp */
    void configure() override;

    /** @reimp */
    bool canConfigure() const override;

    QList<IdnOutput> getOutputmapping();

    QHash<IdnHostInfo, IdnSettings> getSetting();
    void setSetting(QHash<IdnHostInfo, IdnSettings> settings);
    void sendScanRequest(QUdpSocket *socket);
};

#endif
