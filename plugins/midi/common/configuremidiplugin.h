/*
  Q Light Controller
  configuremidiplugin.h

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

#ifndef CONFIGUREMIDIPLUGIN_H
#define CONFIGUREMIDIPLUGIN_H

#include <QDialog>

#include "ui_configuremidiplugin.h"
#include "mididevice.h"

class MidiPlugin;
class QWidget;

class ConfigureMidiPlugin : public QDialog, public Ui_ConfigureMidiPlugin
{
    Q_OBJECT

public:
    ConfigureMidiPlugin(MidiPlugin* plugin, QWidget* parent = 0);
    ~ConfigureMidiPlugin();

public slots:
    void slotRefresh();

private slots:
    void slotMidiChannelValueChanged(int index);
    void slotModeActivated(int index);
    void slotUpdateTree();

private:
    QWidget* createMidiChannelWidget(int select);
    QWidget* createModeWidget(MidiDevice::Mode mode);

private:
    MidiPlugin* m_plugin;
};

#endif
