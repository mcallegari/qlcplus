/*
  Q Light Controller Plus
  configureartnet.h

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

#ifndef CONFIGUREARTNET_H
#define CONFIGUREARTNET_H

#include "ui_configureartnet.h"

class ArtNetPlugin;

class ConfigureArtNet : public QDialog, public Ui_ConfigureArtNet
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    ConfigureArtNet(ArtNetPlugin* plugin, QWidget* parent = 0);
    virtual ~ConfigureArtNet();

    /** @reimp */
    void accept();

public slots:
    int exec();

private:
    void fillNodesTree();

private:
    ArtNetPlugin* m_plugin;

};

#endif
