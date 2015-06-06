/*
  Q Light Controller Plus
  configureosc.cpp

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

#include "configureosc.h"
#include "oscplugin.h"

#define KMapColumnInterface     0
#define KMapColumnUniverse      1
#define KMapColumnInputPort     2
#define KMapColumnOutputAddress 3
#define KMapColumnOutputPort    4

/*****************************************************************************
 * Initialization
 *****************************************************************************/

ConfigureOSC::ConfigureOSC(OSCPlugin* plugin, QWidget* parent)
        : QDialog(parent)
{
    Q_ASSERT(plugin != NULL);
    m_plugin = plugin;

    /* Setup UI controls */
    setupUi(this);

    fillMappingTree();
}

ConfigureOSC::~ConfigureOSC()
{
}

void ConfigureOSC::fillMappingTree()
{
    QTreeWidgetItem* inputItem = NULL;
    QTreeWidgetItem* outputItem = NULL;

    QList<OSCIO> IOmap = m_plugin->getIOMapping();
    foreach(OSCIO io, IOmap)
    {
        if (io.controller == NULL)
            continue;

        OSCController *controller = io.controller;
        if (controller == NULL)
            continue;

        qDebug() << "[ArtNet] controller IP" << controller->getNetworkIP() << "type:" << controller->type();
        if ((controller->type() & OSCController::Input) && inputItem == NULL)
        {
            inputItem = new QTreeWidgetItem(m_uniMapTree);
            inputItem->setText(KMapColumnInterface, tr("Inputs"));
            inputItem->setExpanded(true);
        }
        if ((controller->type() & OSCController::Output) && outputItem == NULL)
        {
            outputItem = new QTreeWidgetItem(m_uniMapTree);
            outputItem->setText(KMapColumnInterface, tr("Outputs"));
            outputItem->setExpanded(true);
        }
        foreach(quint32 universe, controller->universesList())
        {
            UniverseInfo *info = controller->getUniverseInfo(universe);
            QString networkIP = controller->getNetworkIP();
            QString baseIP = networkIP.mid(0, networkIP.lastIndexOf(".") + 1);
            baseIP.append("1");

            if (info->type & OSCController::Input)
            {
                QTreeWidgetItem *item = new QTreeWidgetItem(inputItem);
                item->setData(KMapColumnInterface, Qt::UserRole, universe);
                item->setData(KMapColumnInterface, Qt::UserRole + 1, controller->line());
                item->setData(KMapColumnInterface, Qt::UserRole + 2, OSCController::Input);
                item->setText(KMapColumnInterface, networkIP);
                item->setText(KMapColumnUniverse, QString::number(universe + 1));

                QSpinBox *inSpin = new QSpinBox(this);
                inSpin->setRange(1, 65535);
                inSpin->setValue(info->inputPort);
                m_uniMapTree->setItemWidget(item, KMapColumnInputPort, inSpin);

                if (info->feedbackAddress == QHostAddress::LocalHost)
                {
                    // localhost (127.0.0.1) does not need configuration
                    item->setText(KMapColumnOutputAddress, info->feedbackAddress.toString());
                }
                else
                {
                    QWidget *IPwidget;
                    if (info->feedbackAddress == QHostAddress::Null)
                        IPwidget = createIPWidget(baseIP);
                    else
                        IPwidget = createIPWidget(info->feedbackAddress.toString());
                    m_uniMapTree->setItemWidget(item, KMapColumnOutputAddress, IPwidget);
                }

                QSpinBox *outSpin = new QSpinBox(this);
                outSpin->setRange(1, 65535);
                outSpin->setValue(info->feedbackPort);
                m_uniMapTree->setItemWidget(item, KMapColumnOutputPort, outSpin);
            }
            if (info->type & OSCController::Output)
            {
                QTreeWidgetItem *item = new QTreeWidgetItem(outputItem);
                item->setData(KMapColumnInterface, Qt::UserRole, universe);
                item->setData(KMapColumnInterface, Qt::UserRole + 1, controller->line());
                item->setData(KMapColumnInterface, Qt::UserRole + 2, OSCController::Output);

                item->setText(KMapColumnInterface, networkIP);
                item->setText(KMapColumnUniverse, QString::number(universe + 1));

                if (info->outputAddress == QHostAddress::LocalHost)
                {
                    // localhost (127.0.0.1) does not need configuration
                    item->setText(KMapColumnOutputAddress, info->outputAddress.toString());
                }
                else
                {
                    QWidget *IPwidget;
                    if (info->outputAddress == QHostAddress::Null)
                        IPwidget = createIPWidget(baseIP);
                    else
                        IPwidget = createIPWidget(info->outputAddress.toString());
                    m_uniMapTree->setItemWidget(item, KMapColumnOutputAddress, IPwidget);
                }

                QSpinBox *spin = new QSpinBox(this);
                spin->setRange(1, 65535);
                spin->setValue(info->outputPort);
                m_uniMapTree->setItemWidget(item, KMapColumnOutputPort, spin);
            }
        }
    }

    m_uniMapTree->resizeColumnToContents(KMapColumnInterface);
    m_uniMapTree->resizeColumnToContents(KMapColumnUniverse);
    m_uniMapTree->resizeColumnToContents(KMapColumnInputPort);
    m_uniMapTree->resizeColumnToContents(KMapColumnOutputAddress);
    m_uniMapTree->resizeColumnToContents(KMapColumnOutputPort);
}

QWidget *ConfigureOSC::createIPWidget(QString ip)
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

void ConfigureOSC::accept()
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
            OSCController::Type type = OSCController::Type(item->data(KMapColumnInterface, Qt::UserRole + 2).toInt());
            QLCIOPlugin::Capability cap = QLCIOPlugin::Input;
            if (type == OSCController::Output)
                cap = QLCIOPlugin::Output;

            QSpinBox *inSpin = qobject_cast<QSpinBox*>(m_uniMapTree->itemWidget(item, KMapColumnInputPort));
            if (inSpin != NULL)
            {
                if ((quint32)inSpin->value() != 7700 + universe)
                    m_plugin->setParameter(universe, line, cap, OSC_INPUTPORT, inSpin->value());
                else
                    m_plugin->unSetParameter(universe, line, cap, OSC_INPUTPORT);
            }

            QWidget *ipWidget = m_uniMapTree->itemWidget(item, KMapColumnOutputAddress);
            if (ipWidget != NULL)
            {
                QSpinBox *spin = qobject_cast<QSpinBox*>(ipWidget->layout()->itemAt(1)->widget());
                if (spin != NULL)
                {
                    if (type == OSCController::Input)
                    {
                        if (spin->value() != 1)
                            m_plugin->setParameter(universe, line, QLCIOPlugin::Output, OSC_FEEDBACKIP, spin->value());
                        else
                            m_plugin->unSetParameter(universe, line, QLCIOPlugin::Output, OSC_FEEDBACKIP);
                    }
                    else
                    {
                        if (spin->value() != 1)
                            m_plugin->setParameter(universe, line, cap, OSC_OUTPUTIP, spin->value());
                        else
                            m_plugin->unSetParameter(universe, line, cap, OSC_OUTPUTIP);
                    }
                }
            }

            QSpinBox *outSpin = qobject_cast<QSpinBox*>(m_uniMapTree->itemWidget(item, KMapColumnOutputPort));
            if (outSpin != NULL)
            {
                if (type == OSCController::Input)
                {
                    if ((quint32)outSpin->value() != 9000 + universe)
                        m_plugin->setParameter(universe, line, QLCIOPlugin::Output, OSC_FEEDBACKPORT, outSpin->value());
                    else
                        m_plugin->unSetParameter(universe, line, QLCIOPlugin::Output, OSC_FEEDBACKPORT);
                }
                else
                {
                    if ((quint32)outSpin->value() != 9000 + universe)
                        m_plugin->setParameter(universe, line, cap, OSC_OUTPUTPORT, outSpin->value());
                    else
                        m_plugin->unSetParameter(universe, line, cap, OSC_OUTPUTPORT);
                }
            }
        }
    }

    QDialog::accept();
}

int ConfigureOSC::exec()
{
    return QDialog::exec();
}

