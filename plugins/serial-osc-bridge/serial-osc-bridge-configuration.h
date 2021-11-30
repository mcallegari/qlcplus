/*
  Q Light Controller Plus

  serial-osc-bridge-configuration.h

  Copyright (c) House Gordon Software Company LTD

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

#ifndef SERIAL_OSC_BRIDGE_CONFIGURATION_H
#define SERIAL_OSC_BRIDGE_CONFIGURATION_H

#include "serial-osc-bridge-settings.h"
#include "ui_serial-osc-bridge-configuration.h"

class SerialOscBridgePlugin;

class SerialOscBridgeConfiguration : public QDialog, public Ui_SerialOscBridgeConfiguration
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    SerialOscBridgeConfiguration(SerialOscBridgePlugin* plugin, QWidget* parent = 0);
    virtual ~SerialOscBridgeConfiguration();

    /** @reimp */
    void accept();

  SerialOscBridgeSettings getNewSettings() const { return ui_settings ; };

public slots:
    int exec();

private:
  void showValueAlert(QString control_name, QString control_value_name, QString value);
  SerialOscBridgePlugin* m_plugin;
  SerialOscBridgeSettings ui_settings;

};

#endif
