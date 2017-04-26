/*
  Q Light Controller
  configuremidiplugin.h

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
    void slotInitMessageActivated(int index);
    void slotInitMessageChanged(QString midiTemplateName);
    void slotUpdateTree();

private:
    QWidget* createMidiChannelWidget(int select);
    QWidget* createModeWidget(MidiDevice::Mode mode);
    QWidget* createInitMessageWidget(QString midiTemplateName);

private:
    MidiPlugin* m_plugin;
};

#endif
