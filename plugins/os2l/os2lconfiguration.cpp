/*
  Q Light Controller Plus
  os2lconfiguration.cpp

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

#include "os2lconfiguration.h"
#include "os2lplugin.h"

#define SETTINGS_GEOMETRY "os2lconfiguration/geometry"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

OS2LConfiguration::OS2LConfiguration(OS2LPlugin* plugin, QWidget* parent)
        : QDialog(parent)
{
    Q_ASSERT(plugin != NULL);
    m_plugin = plugin;

    /* Setup UI controls */
    setupUi(this);

    if (m_plugin->universe() == UINT_MAX)
        m_hostGroup->hide();
    else
        m_activateLabel->hide();

    QSettings settings;
    QVariant geometrySettings = settings.value(SETTINGS_GEOMETRY);
    if (geometrySettings.isValid() == true)
        restoreGeometry(geometrySettings.toByteArray());
}

OS2LConfiguration::~OS2LConfiguration()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

/*****************************************************************************
 * Dialog actions
 *****************************************************************************/

void OS2LConfiguration::accept()
{
    m_plugin->setParameter(m_plugin->universe(), 0, QLCIOPlugin::Input, OS2L_HOST_ADDRESS, m_ipAddrEdit->text());
    m_plugin->setParameter(m_plugin->universe(), 0, QLCIOPlugin::Input, OS2L_HOST_PORT, m_portSpin->value());

    QDialog::accept();
}

int OS2LConfiguration::exec()
{
    return QDialog::exec();
}

