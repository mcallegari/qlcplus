/*
  Q Light Controller
  configurehid.cpp

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
#include "hid.h"

#define KColumnNumber  0
#define KColumnName    1

/*****************************************************************************
 * Initialization
 *****************************************************************************/

ConfigureHID::ConfigureHID(QWidget* parent, HID* plugin)
        : QDialog(parent)
{
    Q_ASSERT(plugin != NULL);
    m_plugin = plugin;

    /* Setup UI controls */
    setupUi(this);
    m_list->header()->setResizeMode(QHeaderView::ResizeToContents);

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
