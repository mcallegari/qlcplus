/*
  Q Light Controller Plus
  configuresignet.h

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

#ifndef CONFIGURESIGNET_H
#define CONFIGURESIGNET_H

#include "ui_configuresignet.h"

class SigNetPlugin;

class ConfigureSigNet final : public QDialog, public Ui_ConfigureSigNet
{
    Q_OBJECT

public:
    explicit ConfigureSigNet(SigNetPlugin* plugin, QWidget* parent = nullptr);
    ~ConfigureSigNet() override;

    void accept() override;

private slots:
    void slotGenerateTuid();
    void slotGenerateKey();

private:
    void fillMappingTree();
    void fillNodesTree();
    void showError(const QString& title, const QString& message);

private:
    SigNetPlugin* m_plugin;
};

#endif
