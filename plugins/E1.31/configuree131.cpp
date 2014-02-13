/*
  Q Light Controller Plus
  configuree131.cpp

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

#include <QTreeWidgetItem>
#include <QString>
#include <QDebug>

#include "configuree131.h"
#include "e131plugin.h"

#define KNodesColumnIP          0
#define KNodesColumnShortName   1
#define KNodesColumnLongName    2

/*****************************************************************************
 * Initialization
 *****************************************************************************/

ConfigureE131::ConfigureE131(E131Plugin* plugin, QWidget* parent)
        : QDialog(parent)
{
    Q_ASSERT(plugin != NULL);
    m_plugin = plugin;

    /* Setup UI controls */
    setupUi(this);

    this->resize(400, 300);
}

ConfigureE131::~ConfigureE131()
{
}

/*****************************************************************************
 * Dialog actions
 *****************************************************************************/

void ConfigureE131::accept()
{
    QDialog::accept();
}

int ConfigureE131::exec()
{
    return QDialog::exec();
}

