/*
  Q Light Controller Plus
  configuree131.cpp

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

#include <QTreeWidgetItem>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QString>
#include <QDebug>

#include "configuree131.h"
#include "e131plugin.h"

#define KMapColumnInterface     0
#define KMapColumnUniverse      1
#define KMapColumnIPAddress     2
#define KMapColumnE131Uni       3
#define KMapColumnTransmitMode  4
#define KMapColumnPriority      5

/*****************************************************************************
 * Initialization
 *****************************************************************************/

ConfigureE131::ConfigureE131(E131Plugin* plugin, QWidget* parent)
        : QDialog(parent)
{
    Q_ASSERT(plugin != NULL);
    m_plugin = plugin;

    /* Setup UI controls */
    setupUi(this);

    fillMappingTree();
}

ConfigureE131::~ConfigureE131()
{
}

void ConfigureE131::fillMappingTree()
{
    QTreeWidgetItem* inputItem = NULL;
    QTreeWidgetItem* outputItem = NULL;

    QList<E131IO> IOmap = m_plugin->getIOMapping();
    foreach(E131IO io, IOmap)
    {
        if (io.controller == NULL)
            continue;

        E131Controller *controller = io.controller;
        if (controller == NULL)
            continue;

        qDebug() << "[ArtNet] controller IP" << controller->getNetworkIP() << "type:" << controller->type();
        if ((controller->type() & E131Controller::Input) && inputItem == NULL)
        {
            inputItem = new QTreeWidgetItem(m_uniMapTree);
            inputItem->setText(KMapColumnInterface, tr("Inputs"));
            inputItem->setExpanded(true);
        }
        if ((controller->type() & E131Controller::Output) && outputItem == NULL)
        {
            outputItem = new QTreeWidgetItem(m_uniMapTree);
            outputItem->setText(KMapColumnInterface, tr("Outputs"));
            outputItem->setExpanded(true);
        }
        foreach(quint32 universe, controller->universesList())
        {
            UniverseInfo *info = controller->getUniverseInfo(universe);

            if (info->type & E131Controller::Input)
            {
                QTreeWidgetItem *item = new QTreeWidgetItem(inputItem);
                item->setText(KMapColumnInterface, controller->getNetworkIP());
                item->setText(KMapColumnUniverse, QString::number(universe + 1));
            }
            if (info->type & E131Controller::Output)
            {
                QTreeWidgetItem *item = new QTreeWidgetItem(outputItem);
                item->setData(KMapColumnInterface, Qt::UserRole, universe);
                item->setData(KMapColumnInterface, Qt::UserRole + 1, controller->line());
                item->setData(KMapColumnInterface, Qt::UserRole + 2, E131Controller::Output);

                item->setText(KMapColumnInterface, controller->getNetworkIP());
                item->setText(KMapColumnUniverse, QString::number(universe + 1));

                if (info->mcastAddress == QHostAddress::LocalHost)
                {
                    // localhost (127.0.0.1) do not need broadcast or anything else
                    item->setText(KMapColumnIPAddress, info->mcastAddress.toString());
                }
                else
                {
                    QWidget *IPwidget = createIPWidget(info->mcastAddress.toString());
                    m_uniMapTree->setItemWidget(item, KMapColumnIPAddress, IPwidget);
                }

                QSpinBox *spin = new QSpinBox(this);
                spin->setRange(0, 65535);
                spin->setValue(info->outputUniverse);
                m_uniMapTree->setItemWidget(item, KMapColumnE131Uni, spin);

                QComboBox *combo = new QComboBox(this);
                combo->addItem(tr("Full"));
                combo->addItem(tr("Partial"));
                if (info->trasmissionMode == E131Controller::Partial)
                    combo->setCurrentIndex(1);
                m_uniMapTree->setItemWidget(item, KMapColumnTransmitMode, combo);

                QSpinBox *spin2 = new QSpinBox(this);
                spin2->setRange(0, 200);
                spin2->setValue(info->outputPriority);
                m_uniMapTree->setItemWidget(item, KMapColumnPriority, spin2);
            }
        }
    }

    m_uniMapTree->resizeColumnToContents(KMapColumnInterface);
    m_uniMapTree->resizeColumnToContents(KMapColumnUniverse);
    m_uniMapTree->resizeColumnToContents(KMapColumnIPAddress);
    m_uniMapTree->resizeColumnToContents(KMapColumnE131Uni);
    m_uniMapTree->resizeColumnToContents(KMapColumnTransmitMode);
    m_uniMapTree->resizeColumnToContents(KMapColumnPriority);
}

QWidget *ConfigureE131::createIPWidget(QString ip)
{
    QWidget* widget = new QWidget(this);
    widget->setLayout(new QHBoxLayout);
    widget->layout()->setContentsMargins(0, 0, 0, 0);

    QString baseIP = ip.mid(0, ip.lastIndexOf(".") + 1);
    QString finalIP = ip.mid(ip.lastIndexOf(".") + 1);

    QLabel *label = new QLabel(baseIP, this);
    QSpinBox *spin = new QSpinBox(this);
    spin->setRange(1, 255);
    spin->setValue(finalIP.toInt());

    widget->layout()->addWidget(label);
    widget->layout()->addWidget(spin);

    return widget;
}

/*****************************************************************************
 * Dialog actions
 *****************************************************************************/

void ConfigureE131::accept()
{
    for(int i = 0; i < m_uniMapTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *topItem = m_uniMapTree->topLevelItem(i);
        for(int c = 0; c < topItem->childCount(); c++)
        {
            QTreeWidgetItem *item = topItem->child(c);
            if (item->data(KMapColumnInterface, Qt::UserRole).isValid() == false)
                continue;

            quint32 universe = item->data(KMapColumnInterface, Qt::UserRole).toUInt();
            quint32 line = item->data(KMapColumnInterface, Qt::UserRole + 1).toUInt();
            E131Controller::Type type = E131Controller::Type(item->data(KMapColumnInterface, Qt::UserRole + 2).toInt());
            QLCIOPlugin::Capability cap = QLCIOPlugin::Input;
            if (type == E131Controller::Output)
                cap = QLCIOPlugin::Output;

            QWidget *ipWidget = m_uniMapTree->itemWidget(item, KMapColumnIPAddress);
            if (ipWidget != NULL)
            {
                QSpinBox *spin = qobject_cast<QSpinBox*>(ipWidget->layout()->itemAt(1)->widget());
                if (spin != NULL)
                {
                    if (spin->value() != 255)
                        m_plugin->setParameter(universe, line, cap, E131_MCASTIP, spin->value());
                    else
                        m_plugin->unSetParameter(universe, line, cap, E131_MCASTIP);
                }
            }

            QSpinBox *spin = qobject_cast<QSpinBox*>(m_uniMapTree->itemWidget(item, KMapColumnE131Uni));
            if (spin != NULL)
            {
                if ((quint32)spin->value() != universe)
                    m_plugin->setParameter(universe, line, cap, E131_OUTPUTUNI, spin->value());
                else
                    m_plugin->unSetParameter(universe, line, cap, E131_OUTPUTUNI);
            }

            QComboBox *combo = qobject_cast<QComboBox*>(m_uniMapTree->itemWidget(item, KMapColumnTransmitMode));
            if (combo != NULL)
            {
                if(combo->currentIndex() == 1)
                    m_plugin->setParameter(universe, line, cap, E131_TRANSMITMODE,
                                           E131Controller::transmissionModeToString(E131Controller::Partial));
                else
                    m_plugin->unSetParameter(universe, line, cap, E131_TRANSMITMODE);
            }

            QSpinBox *spin2 = qobject_cast<QSpinBox*>(m_uniMapTree->itemWidget(item, KMapColumnPriority));
            if (spin2 != NULL)
            {
                if ((quint32)spin2->value() != universe)
                    m_plugin->setParameter(universe, line, cap, E131_OUTPUTPRIORITY, spin2->value());
                else
                    m_plugin->unSetParameter(universe, line, cap, E131_OUTPUTPRIORITY);
            }
        }
    }

    QDialog::accept();
}

int ConfigureE131::exec()
{
    return QDialog::exec();
}

