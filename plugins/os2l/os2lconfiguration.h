/*
  Q Light Controller Plus
  os2lconfiguration.h

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

#ifndef OS2LCONFIGURATION_H
#define OS2LCONFIGURATION_H

#include "ui_os2lconfiguration.h"

class OS2LPlugin;

class OS2LConfiguration : public QDialog, public Ui_OS2LConfiguration
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    OS2LConfiguration(OS2LPlugin* plugin, QWidget* parent = 0);
    virtual ~OS2LConfiguration();

    /** @reimp */
    void accept();

public slots:
    int exec();

private:
    OS2LPlugin* m_plugin;

};

#endif
