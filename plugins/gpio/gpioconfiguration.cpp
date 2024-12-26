/*
  Q Light Controller Plus
  gpioconfiguration.cpp

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

#include <QComboBox>
#include <QSettings>

#include <gpiod.hpp>

#include "gpioconfiguration.h"
#include "gpioplugin.h"

#define KColumnGPIONumber           0
#define KColumnGPIOUsage            1
#define KColumnGPIOChannelNumber    2

#define SETTINGS_GEOMETRY "gpioconfiguration/geometry"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

GPIOConfiguration::GPIOConfiguration(GPIOPlugin* plugin, QWidget* parent)
    : QDialog(parent)
    , m_plugin(plugin)
{
    Q_ASSERT(plugin != NULL);

    /* Setup UI controls */
    setupUi(this);

    QSettings settings;
    QVariant geometrySettings = settings.value(SETTINGS_GEOMETRY);
    if (geometrySettings.isValid() == true)
        restoreGeometry(geometrySettings.toByteArray());

    for (auto& it: ::gpiod::make_chip_iter())
        m_chipCombo->addItem(QString::fromStdString(it.name()));
    m_chipCombo->setCurrentText(QString::fromStdString(m_plugin->chipName()));

    connect(m_chipCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotChipChanged(int)));

    fillTree();
}

GPIOConfiguration::~GPIOConfiguration()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

void GPIOConfiguration::fillTree()
{
    m_treeWidget->clear();

    foreach (GPIOLineInfo* gpio, m_plugin->gpioList())
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(m_treeWidget);
        item->setText(KColumnGPIONumber, gpio->m_name);
        item->setText(KColumnGPIOChannelNumber, QString::number(gpio->m_line + 1));

        QComboBox *combo = new QComboBox(this);
        combo->addItem(tr("Not used"), GPIOPlugin::NoDirection);
        combo->addItem(tr("Input"), GPIOPlugin::InputDirection);
        combo->addItem(tr("Output"), GPIOPlugin::OutputDirection);
        if (gpio->m_direction == GPIOPlugin::InputDirection)
            combo->setCurrentIndex(1);
        else if (gpio->m_direction == GPIOPlugin::OutputDirection)
            combo->setCurrentIndex(2);
        m_treeWidget->setItemWidget(item, KColumnGPIOUsage, combo);
    }

    m_treeWidget->header()->resizeSections(QHeaderView::ResizeToContents);
}

/*****************************************************************************
 * Dialog actions
 *****************************************************************************/

void GPIOConfiguration::accept()
{
    QList<GPIOLineInfo *> gpioList = m_plugin->gpioList();

    for (int i = 0; i < m_treeWidget->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = m_treeWidget->topLevelItem(i);

        QComboBox *combo = qobject_cast<QComboBox*>(m_treeWidget->itemWidget(item, KColumnGPIOUsage));
        if (combo != NULL)
        {
            GPIOPlugin::LineDirection usage = GPIOPlugin::LineDirection(combo->currentData().toInt());

            // not interested in not-changed directions
            if (usage == gpioList.at(i)->m_direction)
                continue;

            QString parName = QString("%1-%2").arg(GPIO_PARAM_USAGE).arg(i);

            if (usage == GPIOPlugin::InputDirection)
                m_plugin->setParameter(0, 0, QLCIOPlugin::Input, parName,
                                       m_plugin->lineDirectionToString(usage));
            else if (usage == GPIOPlugin::OutputDirection)
                m_plugin->setParameter(0, 0, QLCIOPlugin::Output, parName,
                                       m_plugin->lineDirectionToString(usage));
            else // GPIOPlugin::NoUsage
            {
                // we use the setParameter method here cause we need to perform
                // actual operations on the GPIO files
                // then setParameter will call unSetParameter
                if (gpioList.at(i)->m_direction == GPIOPlugin::InputDirection)
                    m_plugin->setParameter(0, 0, QLCIOPlugin::Input, parName,
                                           m_plugin->lineDirectionToString(usage));
                else
                    m_plugin->setParameter(0, 0, QLCIOPlugin::Output, parName,
                                           m_plugin->lineDirectionToString(usage));
            }
        }
    }

    QDialog::accept();
}

void GPIOConfiguration::slotChipChanged(int index)
{
    m_plugin->setChipName(m_chipCombo->itemText(index));

    fillTree();
}

int GPIOConfiguration::exec()
{
    return QDialog::exec();
}

