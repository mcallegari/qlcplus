/*
  Q Light Controller Plus
  configureosc.h

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

#ifndef CONFIGUREOSC_H
#define CONFIGUREOSC_H

#include "ui_configureosc.h"

class OSCPlugin;

class ConfigureOSC : public QDialog, public Ui_ConfigureOSC
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    ConfigureOSC(OSCPlugin* plugin, QWidget* parent = 0);
    virtual ~ConfigureOSC();

    /** @reimp */
    void accept();

public slots:
    void slotOSCPathChanged(QString path);
    int exec();

private:
    void fillMappingTree();
    void showIPAlert(QString ip);

private:
    OSCPlugin* m_plugin;

};

#endif
