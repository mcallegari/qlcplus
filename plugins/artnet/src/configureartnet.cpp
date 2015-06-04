/*
  Q Light Controller Plus
  configureartnet.cpp

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

#include "configureartnet.h"
#include "artnetplugin.h"

#define KNodesColumnIP          0
#define KNodesColumnShortName   1
#define KNodesColumnLongName    2

#define KMapColumnInterface     0
#define KMapColumnUniverse      1
#define KMapColumnIPAddress     2
#define KMapColumnArtNetUni     3
#define KMapColumnTransmitMode  4

/*****************************************************************************
 * Initialization
 *****************************************************************************/

ConfigureArtNet::ConfigureArtNet(ArtNetPlugin* plugin, QWidget* parent)
        : QDialog(parent)
{
    Q_ASSERT(plugin != NULL);
    m_plugin = plugin;

    /* Setup UI controls */
    setupUi(this);

    fillNodesTree();
    fillMappingTree();
}


void ConfigureArtNet::fillNodesTree()
{
    ArtNetController *prevController = NULL;

    QList<ArtNetIO> IOmap = m_plugin->getIOMapping();

    for (int i = 0; i < IOmap.length(); i++)
    {
        ArtNetController *controller = IOmap.at(i).controller;

        if (controller != NULL && controller != prevController)
        {
            QTreeWidgetItem* pitem = new QTreeWidgetItem(m_nodesTree);
            pitem->setText(KNodesColumnIP, tr("%1 nodes").arg(controller->getNetworkIP()));
            QHash<QHostAddress, ArtNetNodeInfo> nodesList = controller->getNodesList();
            QHashIterator<QHostAddress, ArtNetNodeInfo> it(nodesList);
            while (it.hasNext())
            {
                it.next();
                QTreeWidgetItem* nitem = new QTreeWidgetItem(pitem);
                ArtNetNodeInfo nInfo = it.value();
                nitem->setText(KNodesColumnIP, it.key().toString());
                nitem->setText(KNodesColumnShortName, nInfo.shortName);
                nitem->setText(KNodesColumnLongName, nInfo.longName);
            }
            prevController = controller;
        }
    }

    m_nodesTree->resizeColumnToContents(KNodesColumnIP);
    m_nodesTree->resizeColumnToContents(KNodesColumnShortName);
    m_nodesTree->resizeColumnToContents(KNodesColumnLongName);
}

void ConfigureArtNet::fillMappingTree()
{
    QTreeWidgetItem* inputItem = NULL;
    QTreeWidgetItem* outputItem = NULL;

    QList<ArtNetIO> IOmap = m_plugin->getIOMapping();
    foreach(ArtNetIO io, IOmap)
    {
        if (io.controller == NULL)
            continue;

        ArtNetController *controller = io.controller;
        if (controller == NULL)
            continue;

        qDebug() << "[ArtNet] controller IP" << controller->getNetworkIP() << "type:" << controller->type();
        if ((controller->type() & ArtNetController::Input) && inputItem == NULL)
        {
            inputItem = new QTreeWidgetItem(m_uniMapTree);
            inputItem->setText(KMapColumnInterface, tr("Inputs"));
            inputItem->setExpanded(true);
        }
        if ((controller->type() & ArtNetController::Output) && outputItem == NULL)
        {
            outputItem = new QTreeWidgetItem(m_uniMapTree);
            outputItem->setText(KMapColumnInterface, tr("Outputs"));
            outputItem->setExpanded(true);
        }
        foreach(quint32 universe, controller->universesList())
        {
            UniverseInfo *info = controller->getUniverseInfo(universe);

            if (info->type & ArtNetController::Input)
            {
                QTreeWidgetItem *item = new QTreeWidgetItem(inputItem);
                item->setText(KMapColumnInterface, controller->getNetworkIP());
                item->setText(KMapColumnUniverse, QString::number(universe + 1));
            }
            if (info->type & ArtNetController::Output)
            {
                QTreeWidgetItem *item = new QTreeWidgetItem(outputItem);
                item->setData(KMapColumnInterface, Qt::UserRole, universe);
                item->setData(KMapColumnInterface, Qt::UserRole + 1, controller->line());
                item->setData(KMapColumnInterface, Qt::UserRole + 2, ArtNetController::Output);

                item->setText(KMapColumnInterface, controller->getNetworkIP());
                item->setText(KMapColumnUniverse, QString::number(universe + 1));

                if (info->outputAddress == QHostAddress::LocalHost)
                {
                    // localhost (127.0.0.1) do not need broadcast or anything else
                    item->setText(KMapColumnIPAddress, info->outputAddress.toString());
                }
                else
                {
                    QWidget *IPwidget = createIPWidget(info->outputAddress.toString());
                    m_uniMapTree->setItemWidget(item, KMapColumnIPAddress, IPwidget);
                }

                QSpinBox *spin = new QSpinBox(this);
                spin->setRange(0, 65535);
                spin->setValue(info->outputUniverse);
                m_uniMapTree->setItemWidget(item, KMapColumnArtNetUni, spin);

                QComboBox *combo = new QComboBox(this);
                combo->addItem(tr("Full"));
                combo->addItem(tr("Partial"));
                if (info->trasmissionMode == ArtNetController::Partial)
                    combo->setCurrentIndex(1);
                m_uniMapTree->setItemWidget(item, KMapColumnTransmitMode, combo);
            }
        }
    }

    m_uniMapTree->resizeColumnToContents(KMapColumnInterface);
    m_uniMapTree->resizeColumnToContents(KMapColumnUniverse);
    m_uniMapTree->resizeColumnToContents(KMapColumnIPAddress);
    m_uniMapTree->resizeColumnToContents(KMapColumnArtNetUni);
    m_uniMapTree->resizeColumnToContents(KMapColumnTransmitMode);
}

QWidget *ConfigureArtNet::createIPWidget(QString ip)
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

ConfigureArtNet::~ConfigureArtNet()
{
}

/*****************************************************************************
 * Dialog actions
 *****************************************************************************/

void ConfigureArtNet::accept()
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
            ArtNetController::Type type = ArtNetController::Type(item->data(KMapColumnInterface, Qt::UserRole + 2).toInt());
            QLCIOPlugin::Capability cap = QLCIOPlugin::Input;
            if (type == ArtNetController::Output)
                cap = QLCIOPlugin::Output;

            QWidget *ipWidget = m_uniMapTree->itemWidget(item, KMapColumnIPAddress);
            if (ipWidget != NULL)
            {
                QSpinBox *spin = qobject_cast<QSpinBox*>(ipWidget->layout()->itemAt(1)->widget());
                if (spin != NULL)
                {
                    if (spin->value() != 255)
                        m_plugin->setParameter(universe, line, cap, "outputIP", spin->value());
                    else
                        m_plugin->unSetParameter(universe, line, cap, "outputIP");
                }
            }

            QSpinBox *spin = qobject_cast<QSpinBox*>(m_uniMapTree->itemWidget(item, KMapColumnArtNetUni));
            if (spin != NULL)
            {
                if ((quint32)spin->value() != universe)
                    m_plugin->setParameter(universe, line, cap, "outputUni", spin->value());
                else
                    m_plugin->unSetParameter(universe, line, cap, "outputUni");
            }

            QComboBox *combo = qobject_cast<QComboBox*>(m_uniMapTree->itemWidget(item, KMapColumnTransmitMode));
            if (combo != NULL)
            {
                if(combo->currentIndex() == 1)
                    m_plugin->setParameter(universe, line, cap, "transmitMode",
                                           ArtNetController::transmissionModeToString(ArtNetController::Partial));
                else
                    m_plugin->unSetParameter(universe, line, cap, "transmitMode");
            }
        }
    }

    QDialog::accept();
}

int ConfigureArtNet::exec()
{
    return QDialog::exec();
}

