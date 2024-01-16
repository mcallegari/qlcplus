/*
  Q Light Controller
  configureolaio.cpp

  Copyright (C) Simon Newton
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

#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QPushButton>
#include <QDialog>
#include <QString>
#include <QTimer>
#include <QSettings>

#include "configureolaio.h"
#include "olaio.h"

#define COL_NAME 0
#define COL_LINE 1

#define SETTINGS_GEOMETRY "configureolaio/geometry"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

ConfigureOlaIO::ConfigureOlaIO(OlaIO* plugin, QWidget* parent)
    : QDialog(parent)
    , m_plugin(plugin)
{
    Q_ASSERT(plugin != NULL);

    setupUi(this);
    populateOutputList();

    m_standaloneCheck->setChecked(m_plugin->isServerEmbedded());

    QSettings settings;
    QVariant geometrySettings = settings.value(SETTINGS_GEOMETRY);
    if (geometrySettings.isValid() == true)
        restoreGeometry(geometrySettings.toByteArray());
}

ConfigureOlaIO::~ConfigureOlaIO()
{
    m_plugin->setServerEmbedded(m_standaloneCheck->isChecked());

    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

void ConfigureOlaIO::populateOutputList()
{
    m_listView->clear();

    QList <uint> outputs(m_plugin->outputMapping());
    for (int i = 0; i != outputs.size(); ++i)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(m_listView);
        item->setText(COL_NAME, QString("OLA Output %1").arg(i + 1));
        item->setText(COL_LINE, QString("%1").arg(outputs[i]));
    }
}
