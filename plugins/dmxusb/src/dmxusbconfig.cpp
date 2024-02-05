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
#include <QSpinBox>
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
#define COL_FREQ   3
#define PROP_SERIAL "serial"
#define PROP_WIDGET "widget"

DMXUSBConfig::DMXUSBConfig(DMXUSB* plugin, QWidget* parent)
    : QDialog(parent)
    , m_plugin(plugin)
    , m_tree(new QTreeWidget(this))
    , m_refreshButton(new QPushButton(tr("Refresh"), this))
    , m_closeButton(new QPushButton(tr("Close"), this))
{
    Q_ASSERT(plugin != NULL);

    setWindowTitle(plugin->name());

    QStringList header;
    header << tr("Name") << tr("Serial") << tr("Mode") << tr("Output frequency");
    m_tree->setHeaderLabels(header);
    m_tree->setSelectionMode(QAbstractItemView::NoSelection);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->addWidget(m_tree);

    QHBoxLayout *hbox = new QHBoxLayout;
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
    else
        setGeometry(QRect(100, 100, 700, 350));

    slotRefresh();
}

DMXUSBConfig::~DMXUSBConfig()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

void DMXUSBConfig::slotTypeComboActivated(int index)
{
    QComboBox *combo = qobject_cast<QComboBox*> (QObject::sender());
    Q_ASSERT(combo != NULL);

    QVariant var = combo->property(PROP_SERIAL);
    if (var.isValid() == true)
    {
        DMXUSBWidget::Type type = (DMXUSBWidget::Type)combo->itemData(index).toInt();
        QMap <QString,QVariant> typeMap(DMXInterface::typeMap());
        typeMap[var.toString()] = type;
        DMXInterface::storeTypeMap(typeMap);
    }

    QTimer::singleShot(0, this, SLOT(slotRefresh()));
}

void DMXUSBConfig::slotFrequencyValueChanged(int value)
{
    QSpinBox *spin = qobject_cast<QSpinBox*> (QObject::sender());
    Q_ASSERT(spin != NULL);

    QVariant var = spin->property(PROP_SERIAL);
    if (var.isValid() == true)
    {
        QMap <QString,QVariant> frequencyMap(DMXInterface::frequencyMap());
        frequencyMap[var.toString()] = value;
        DMXInterface::storeFrequencyMap(frequencyMap);
    }

    var = spin->property(PROP_WIDGET);
    DMXUSBWidget *widget = (DMXUSBWidget *) var.value<void *>();
    widget->setOutputFrequency(value);
}

void DMXUSBConfig::slotRefresh()
{
    m_plugin->rescanWidgets();

    m_tree->clear();
    QListIterator <DMXUSBWidget*> it(m_plugin->widgets());
    while (it.hasNext() == true)
    {
        DMXUSBWidget *widget = it.next();
        QTreeWidgetItem *item = new QTreeWidgetItem(m_tree);
        item->setText(COL_NAME, widget->uniqueName());
        item->setText(COL_SERIAL, widget->serial());
        m_tree->setItemWidget(item, COL_TYPE, createTypeCombo(widget));
        m_tree->setItemWidget(item, COL_FREQ, createFrequencySpin(widget));
    }

    m_tree->header()->resizeSections(QHeaderView::ResizeToContents);
}

QComboBox *DMXUSBConfig::createTypeCombo(DMXUSBWidget *widget)
{
    Q_ASSERT(widget != NULL);
    QComboBox* combo = new QComboBox;
    combo->setProperty(PROP_SERIAL, widget->serial());
    combo->addItem(QString("Pro RX/TX"), DMXUSBWidget::ProRXTX);
    combo->addItem(QString("Open TX"), DMXUSBWidget::OpenTX);
    combo->addItem(QString("Open RX"), DMXUSBWidget::OpenRX);
    combo->addItem(QString("Pro Mk2"), DMXUSBWidget::ProMk2);
    combo->addItem(QString("Ultra Pro"), DMXUSBWidget::UltraPro);
    combo->addItem(QString("DMX4ALL"), DMXUSBWidget::DMX4ALL);
    combo->addItem(QString("Vince TX"), DMXUSBWidget::VinceTX);
    combo->addItem(QString("Eurolite"), DMXUSBWidget::Eurolite);
    int index = combo->findData(widget->type());
    combo->setCurrentIndex(index);

    connect(combo, SIGNAL(activated(int)), this, SLOT(slotTypeComboActivated(int)));

    return combo;
}

QSpinBox *DMXUSBConfig::createFrequencySpin(DMXUSBWidget *widget)
{
    Q_ASSERT(widget != NULL);
    QSpinBox *spin = new QSpinBox;
    spin->setProperty(PROP_SERIAL, widget->serial());
    spin->setProperty(PROP_WIDGET, QVariant::fromValue((void *)widget));
    spin->setRange(1, 60);
    spin->setValue(widget->outputFrequency());
    spin->setSuffix("Hz");

    connect(spin, SIGNAL(valueChanged(int)), this, SLOT(slotFrequencyValueChanged(int)));

    return spin;
}
