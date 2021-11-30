/*
  Q Light Controller Plus

  serial-osc-bridge-plugin.h

  Copyright (C) House Gordon Software Company LTD

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

#ifndef SERIAL_OSC_BRIDGE_PLUGIN_H
#define SERIAL_OSC_BRIDGE_PLUGIN_H

#include "qlcioplugin.h"

#include "serial-osc-bridge-settings.h"

class ReadThread;

class SerialOscBridgePlugin : public QLCIOPlugin
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
    SerialOscBridgePlugin();
    virtual ~SerialOscBridgePlugin();

    void init();
    QString name();
    int capabilities() const;
    QString pluginInfo();

    /*********************************************************************
     * Input
     *********************************************************************/
public:
    bool openInput(quint32 input, quint32 universe);
    void closeInput(quint32 input, quint32 universe);
    QStringList inputs();
    QString inputInfo(quint32 input);

    /*********************************************************************
     * Configuration
     *********************************************************************/
public:
    SerialOscBridgeSettings getSettings() const { return m_settings ; };
    void applySettings(const SerialOscBridgeSettings& new_settings);
    void configure();
    bool canConfigure();
    void setParameter(quint32 universe, quint32 line, Capability type, QString name, QVariant value);


protected:
    ReadThread *m_readerThread;
    quint32 m_inputUniverse;
    SerialOscBridgeSettings m_settings;

};

#endif
