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

#include "gpioconfiguration.h"
#include "gpioplugin.h"

#define KColumnGPIONumber       0
#define KColumnGPIOUsage        1

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

    fillTree();
}

GPIOConfiguration::~GPIOConfiguration()
{
    /** Cleanup the allocated resources, if any */
}

void GPIOConfiguration::fillTree()
{
    foreach(GPIOPinInfo* gpio, m_plugin->gpioList())
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(m_treeWidget);
        item->setText(KColumnGPIONumber, QString::number(gpio->m_number));

        QComboBox *combo = new QComboBox(this);
        combo->addItem(tr("Not used"), GPIOPlugin::NoUsage);
        combo->addItem(tr("Input"), GPIOPlugin::InputUsage);
        combo->addItem(tr("Output"), GPIOPlugin::OutputUsage);
        if (gpio->m_usage == GPIOPlugin::InputUsage)
            combo->setCurrentIndex(1);
        else if (gpio->m_usage == GPIOPlugin::OutputUsage)
            combo->setCurrentIndex(2);
        m_treeWidget->setItemWidget(item, KColumnGPIOUsage, combo);
    }
}

/*****************************************************************************
 * Dialog actions
 *****************************************************************************/

void GPIOConfiguration::accept()
{
    QList<GPIOPinInfo *> gpioList = m_plugin->gpioList();

    for(int i = 0; i < m_treeWidget->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = m_treeWidget->topLevelItem(i);

        QString parName = QString("%1-%2").arg(GPIO_PARAM_USAGE).arg(i);

        QComboBox *combo = qobject_cast<QComboBox*>(m_treeWidget->itemWidget(item, KColumnGPIOUsage));
        if (combo != NULL)
        {
            GPIOPlugin::PinUsage usage = GPIOPlugin::PinUsage(combo->currentData().toInt());
            if (usage == GPIOPlugin::InputUsage)
                m_plugin->setParameter(0, 0, QLCIOPlugin::Input, parName,
                                       m_plugin->pinUsageToString(usage));
            else if (usage == GPIOPlugin::OutputUsage)
                m_plugin->setParameter(0, 0, QLCIOPlugin::Output, parName,
                                       m_plugin->pinUsageToString(usage));
            else // GPIOPlugin::NoUsage
            {
                // we use the setParameter method here cause we need to perform
                // actual operations on the GPIO files
                // then setParameter will call unSetParameter
                if (gpioList.at(i)->m_usage == GPIOPlugin::InputUsage)
                    m_plugin->setParameter(0, 0, QLCIOPlugin::Input, parName,
                                           m_plugin->pinUsageToString(usage));
                else
                    m_plugin->setParameter(0, 0, QLCIOPlugin::Output, parName,
                                           m_plugin->pinUsageToString(usage));
            }
        }
    }

    QDialog::accept();
}

int GPIOConfiguration::exec()
{
    return QDialog::exec();
}

