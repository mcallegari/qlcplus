/*
  Q Light Controller
  configurehid.h

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef CONFIGUREHID_H
#define CONFIGUREHID_H

#include "ui_configurehid.h"

class HIDDevice;
class HID;

class ConfigureHID : public QDialog, public Ui_ConfigureHID
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    ConfigureHID(QWidget* parent, HID* plugin);
    virtual ~ConfigureHID();

private:
    HID* m_plugin;

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

private:
    /** Refresh the interface list */
    void refreshList();
};

#endif
