/*
  Q Light Controller
  configurehid.cpp

  Copyright (c) Heikki Junnila

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
#include <QHeaderView>
#include <QPushButton>
#include <QString>
#include <QTimer>
#include <QDebug>

#include "configurehid.h"
#include "hiddevice.h"
#include "hidplugin.h"

#define KColumnNumber  0
#define KColumnName    1

/*****************************************************************************
 * Initialization
 *****************************************************************************/

ConfigureHID::ConfigureHID(QWidget* parent, HIDPlugin* plugin)
        : QDialog(parent)
{
    Q_ASSERT(plugin != NULL);
    m_plugin = plugin;

    /* Setup UI controls */
    setupUi(this);

    connect(m_refreshButton, SIGNAL(clicked()),
            this, SLOT(slotRefreshClicked()));

    /* Listen to device additions/removals */
    connect(plugin, SIGNAL(deviceRemoved(HIDDevice*)),
            this, SLOT(slotDeviceRemoved(HIDDevice*)));
    connect(plugin, SIGNAL(deviceAdded(HIDDevice*)),
            this, SLOT(slotDeviceAdded(HIDDevice*)));

    refreshList();
}

ConfigureHID::~ConfigureHID()
{
}

/*****************************************************************************
 * Interface refresh
 *****************************************************************************/

void ConfigureHID::slotRefreshClicked()
{
    Q_ASSERT(m_plugin != NULL);
    m_plugin->rescanDevices();
    refreshList();
}

void ConfigureHID::refreshList()
{
    QString s;

    m_list->clear();

    for (int i = 0; i < m_plugin->m_devices.count(); i++)
    {
        HIDDevice* dev;
        QTreeWidgetItem* item;

        dev = m_plugin->device(i);
        Q_ASSERT(dev != NULL);

        item = new QTreeWidgetItem(m_list);
        item->setText(KColumnNumber, s.setNum(i + 1));
        item->setText(KColumnName, dev->name());
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    }
    m_list->header()->resizeSections(QHeaderView::ResizeToContents);
}

void ConfigureHID::slotDeviceAdded(HIDDevice*)
{
    refreshList();
}

void ConfigureHID::slotDeviceRemoved(HIDDevice* device)
{
    Q_ASSERT(device != NULL);

    for (int i = 0; i < m_list->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_list->topLevelItem(i);
        Q_ASSERT(item != NULL);
        if (item->text(KColumnName) == device->name())
        {
            delete item;
            break;
        }
    }
}
