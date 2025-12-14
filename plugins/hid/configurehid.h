/*
  Q Light Controller
  configurehid.h

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

#ifndef CONFIGUREHID_H
#define CONFIGUREHID_H

#include "ui_configurehid.h"

class HIDDevice;
class HIDPlugin;

class ConfigureHID : public QDialog, public Ui_ConfigureHID
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    ConfigureHID(QWidget* parent, HIDPlugin* plugin);
    virtual ~ConfigureHID();

private:
    HIDPlugin* m_plugin;

    /*********************************************************************
     * Refresh
     *********************************************************************/
private slots:
    /** Invoke refresh for the interface list. */
    void slotRefreshClicked();

    /** Callback for HIDInput::deviceAdded() signals. */
    void slotDeviceAdded(HIDDevice* device);

    /** Callback for HIDInput::deviceRemoved() signals. */
    void slotDeviceRemoved(HIDDevice* device);

    /** Change the merger mode. */
    void slotMergerModeChanged(int state);

private:
    /** Refresh the interface list */
    void refreshList();

    /** Checkbox for merger mode (de-)activation. */
    QWidget* createMergerModeWidget(bool mergerModeEnabled);
};

#endif
