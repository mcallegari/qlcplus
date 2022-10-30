/*
  Q Light Controller Plus
  configuree131.h

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

#ifndef CONFIGUREE131_H
#define CONFIGUREE131_H

#include "ui_configuree131.h"

class E131Plugin;

class ConfigureE131 : public QDialog, public Ui_ConfigureE131
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    ConfigureE131(E131Plugin* plugin, QWidget* parent = 0);
    virtual ~ConfigureE131();

    /** @reimp */
    void accept();

public slots:
    int exec();

private:
    void fillMappingTree();
    QWidget *createMcastIPWidget(QString ip);
    QString getIPAddress(QWidget *ipw);
    void showIPAlert(QString ip);

private slots:
    void slotMulticastCheckboxClicked();

private:
    E131Plugin* m_plugin;
};

#endif
