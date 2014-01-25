/*
  Q Light Controller Plus
  spiconfiguration.h

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

#ifndef SPICONFIGURATION_H
#define SPICONFIGURATION_H

#include "ui_spiconfiguration.h"

class SPIPlugin;

class SPIConfiguration : public QDialog, public Ui_SPIConfiguration
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    SPIConfiguration(SPIPlugin* plugin, QWidget* parent = 0);
    virtual ~SPIConfiguration();

    /** @reimp */
    void accept();

    quint32 frequency();

public slots:
    int exec();

private:
    SPIPlugin* m_plugin;

};

#endif
