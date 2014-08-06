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
#include <QComboBox>
#include <QSettings>
#include <QSpinBox>
#include <QString>
#include <QTimer>
#include <QDebug>

#include "configurehid.h"
#include "hidjsdevice.h"
#include "hiddevice.h"
#include "hidplugin.h"

#define KColumnNumber       0
#define KColumnName         1
#define KColumnAxesMovement 2
#define KColumnSensitivity  3

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

void ConfigureHID::accept()
{
    for (int i = 0; i < m_jslist->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = m_jslist->topLevelItem(i);
        QComboBox *combo = (QComboBox *)m_jslist->itemWidget(item, KColumnAxesMovement);
        QSpinBox *spin = (QSpinBox *)m_jslist->itemWidget(item, KColumnSensitivity);
        HIDDevice *device = m_plugin->device(i);
        if (combo == NULL || spin == NULL || device == NULL)
            continue;

        HIDJsDevice *joystick = qobject_cast<HIDJsDevice *>(device);
        if (joystick->axesBehaviour() != combo->currentIndex() ||
            joystick->axesSensitivity() != spin->value())
        {
            QString devName = item->text(KColumnName);
            QSettings settings;
            QString axesKey = QString("hidplugin/axes/%1").arg(devName);
            QString axesValue = QString("%1,%2")
                    .arg(combo->currentIndex()?"Absolute":"Relative")
                    .arg(spin->value());
            settings.setValue(axesKey, QVariant(axesValue));

            joystick->setAxesBehaviour(HIDJsDevice::HIDJSAxesMovement(combo->currentIndex()));
            joystick->setAxesSensitivity(spin->value());
        }
    }
    QDialog::accept();
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

    m_jslist->clear();
    m_otherlist->clear();

    for (int i = 0; i < m_plugin->m_devices.count(); i++)
    {
        HIDDevice* dev;
        QTreeWidgetItem* item;

        dev = m_plugin->device(i);
        Q_ASSERT(dev != NULL);

        if (dev->type() == HIDDevice::Joystick)
        {
            HIDJsDevice *joystick = qobject_cast<HIDJsDevice *>(dev);

            item = new QTreeWidgetItem(m_jslist);

            QComboBox *combo = new QComboBox();
            combo->addItem(tr("Relative"), false);
            combo->addItem(tr("Absolute"), false);
            m_jslist->setItemWidget(item, KColumnAxesMovement, combo);
            if (joystick->axesBehaviour() == HIDJsDevice::Absolute)
                combo->setCurrentIndex(1);

            QSpinBox *spin = new QSpinBox();
            spin->setRange(10, 100);
            spin->setValue(joystick->axesSensitivity());
            m_jslist->setItemWidget(item, KColumnSensitivity, spin);
        }
        else
            item = new QTreeWidgetItem(m_otherlist);

        item->setText(KColumnNumber, s.setNum(dev->line() + 1));
        item->setText(KColumnName, dev->name());
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    }
    m_jslist->resizeColumnToContents(KColumnNumber);
    m_jslist->resizeColumnToContents(KColumnName);
    m_otherlist->resizeColumnToContents(KColumnNumber);
    m_otherlist->resizeColumnToContents(KColumnName);
}

void ConfigureHID::slotDeviceAdded(HIDDevice*)
{
    refreshList();
}

void ConfigureHID::slotDeviceRemoved(HIDDevice*)
{
    refreshList();
}
