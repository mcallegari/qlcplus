/*
  Q Light Controller
  enttecdmxusbconfig.h

  Copyright (C) Heikki Junnila

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

#ifndef ENTTECDMXUSBCONFIG_H
#define ENTTECDMXUSBCONFIG_H

#include <QDialog>

class DMXUSBWidget;
class QTreeWidgetItem;
class DMXUSB;
class QPushButton;
class QTreeWidget;
class QComboBox;
class QSpinBox;

class DMXUSBConfig : public QDialog
{
    Q_OBJECT

public:
    DMXUSBConfig(DMXUSB *plugin, QWidget *parent = 0);
    ~DMXUSBConfig();

private slots:
    void slotTypeComboActivated(int index);
    void slotFrequencyValueChanged(int value);
    void slotRefresh();

private:
    QComboBox *createTypeCombo(DMXUSBWidget *widget);
    QSpinBox *createFrequencySpin(DMXUSBWidget *widget);

private:
    DMXUSB* m_plugin;

    QTreeWidget* m_tree;
    QPushButton* m_refreshButton;
    QPushButton* m_closeButton;
};

#endif
