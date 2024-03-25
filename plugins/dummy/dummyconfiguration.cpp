/*
  Q Light Controller Plus
  dummyconfiguration.cpp

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

#include <QSettings>

#include "dummyconfiguration.h"
#include "dummyplugin.h"

#define SETTINGS_GEOMETRY "dummyconfiguration/geometry"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

DummyConfiguration::DummyConfiguration(DummyPlugin* plugin, QWidget* parent)
        : QDialog(parent)
{
    Q_ASSERT(plugin != NULL);
    m_plugin = plugin;

    /* Setup UI controls */
    setupUi(this);

    QSettings settings;
    QVariant geometrySettings = settings.value(SETTINGS_GEOMETRY);
    if (geometrySettings.isValid() == true)
        restoreGeometry(geometrySettings.toByteArray());

    /**
     * Do the dialog initializations here.
     *  E.g.: fill combo boxes, set spin boxes values, fill lists, etc...
     */
}

DummyConfiguration::~DummyConfiguration()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

/*****************************************************************************
 * Dialog actions
 *****************************************************************************/

void DummyConfiguration::accept()
{
    /** Write code before QDialog::accept if you want to immediately
     *  apply the changes via m_plugin methods. */

    QDialog::accept();

    /** Otherwise just call QDialog::accept() and implement
     *  the configuration changes in DummyPlugin::configure()
     *  by calling DummyConfiguration methods (that you have to add)
     *  before destroying it
     */
}

int DummyConfiguration::exec()
{
    return QDialog::exec();
}

