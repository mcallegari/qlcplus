/*
  Q Light Controller
  dmxusbconfig.cpp

  Copyright (C) Heikki Junnila

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

#include <QPushButton>
#include <QHeaderView>
#include <QTreeWidget>
#include <QSettings>
#include <QComboBox>
#include <QLayout>
#include <QDebug>
#include <QTimer>

#include "dmxusbconfig.h"
#include "dmxusbwidget.h"
#include "dmxusb.h"

#define SETTINGS_GEOMETRY "dmxusbconfig/geometry"

#define COL_NAME   0
#define COL_SERIAL 1
#define COL_TYPE   2
#define PROP_SERIAL "serial"

DMXUSBConfig::DMXUSBConfig(DMXUSB* plugin, QWidget* parent)
    : QDialog(parent)
    , m_plugin(plugin)
    , m_tree(new QTreeWidget(this))
    , m_refreshButton(new QPushButton(tr("Refresh"), this))
    , m_closeButton(new QPushButton(tr("Close"), this))
    , m_ignoreItemChanged(false)
{
    Q_ASSERT(plugin != NULL);

    setWindowTitle(plugin->name());

    QStringList header;
    header << tr("Name") << tr("Serial") << QString("Mode");
    m_tree->setHeaderLabels(header);
    m_tree->setSelectionMode(QAbstractItemView::NoSelection);

    QVBoxLayout* vbox = new QVBoxLayout(this);
    vbox->addWidget(m_tree);

    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addWidget(m_refreshButton);
    hbox->addStretch();
    hbox->addWidget(m_closeButton);
    vbox->addLayout(hbox);

    connect(m_refreshButton, SIGNAL(clicked()), this, SLOT(slotRefresh()));
    connect(m_closeButton, SIGNAL(clicked()), this, SLOT(accept()));

    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        restoreGeometry(var.toByteArray());

    slotRefresh();
}

DMXUSBConfig::~DMXUSBConfig()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

void DMXUSBConfig::slotTypeComboActivated(int index)
{
    QComboBox* combo = qobject_cast<QComboBox*> (QObject::sender());
    Q_ASSERT(combo != NULL);

    QVariant var = combo->property(PROP_SERIAL);
    if (var.isValid() == true)
    {
        DMXUSBWidget::Type type = (DMXUSBWidget::Type)combo->itemData(index).toInt();
        QMap <QString,QVariant> typeMap(QLCFTDI::typeMap());
        typeMap[var.toString()] = type;
        QLCFTDI::storeTypeMap(typeMap);
    }

    QTimer::singleShot(0, this, SLOT(slotRefresh()));
}

void DMXUSBConfig::slotRefresh()
{
    m_plugin->rescanWidgets();

    m_ignoreItemChanged = true;

    m_tree->clear();
    QListIterator <DMXUSBWidget*> it(m_plugin->widgets());
    while (it.hasNext() == true)
    {
        DMXUSBWidget* widget = it.next();
        QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
        item->setText(COL_NAME, widget->uniqueName());
        item->setText(COL_SERIAL, widget->serial());
        m_tree->setItemWidget(item, COL_TYPE, createTypeCombo(widget));
    }

    m_tree->resizeColumnToContents(COL_NAME);
    m_tree->resizeColumnToContents(COL_SERIAL);

    m_ignoreItemChanged = false;
}

QComboBox* DMXUSBConfig::createTypeCombo(DMXUSBWidget *widget)
{
    Q_ASSERT(widget != NULL);
    QComboBox* combo = new QComboBox;
    combo->setProperty(PROP_SERIAL, widget->serial());
    combo->addItem(QString("Pro TX"), DMXUSBWidget::ProTX);
    combo->addItem(QString("Open TX"), DMXUSBWidget::OpenTX);
    combo->addItem(QString("Pro RX"), DMXUSBWidget::ProRX);
    combo->addItem(QString("Pro Mk2"), DMXUSBWidget::ProMk2);
    combo->addItem(QString("Ultra Pro Tx"), DMXUSBWidget::UltraProTx);
    combo->addItem(QString("DMX4ALL"), DMXUSBWidget::DMX4ALL);
    combo->addItem(QString("Vince TX"), DMXUSBWidget::VinceTX);
    int index = combo->findData(widget->type());
    combo->setCurrentIndex(index);

    connect(combo, SIGNAL(activated(int)), this, SLOT(slotTypeComboActivated(int)));

    return combo;
}
