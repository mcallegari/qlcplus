/*
  Q Light Controller
  configureolaio.h

  Copyright (c) Simon Newton
                Heikki Junnila

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

#ifndef CONFIGUREOLAIO_H
#define CONFIGUREOLAIO_H

#include "ui_configureolaio.h"

class OlaIO;

class ConfigureOlaIO : public QDialog, public Ui_ConfigureOlaIO
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    ConfigureOlaIO(OlaIO* plugin, QWidget* parent = 0);
    virtual ~ConfigureOlaIO();

private:
    void populateOutputList();

private:
    OlaIO* m_plugin;
};

#endif
