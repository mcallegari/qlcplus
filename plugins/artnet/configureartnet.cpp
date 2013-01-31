/*
  Q Light Controller
  configureartnet.cpp

  Copyright (c) Massimo Callegari

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
#include <QString>
#include <QDebug>

#include "configureartnet.h"
#include "artnetplugin.h"

#define UNIVERSES_PER_ADDRESS   4

#define KOutputColumnNetwork   0
#define KOutputColumnUniverse  1

#define KNodesColumnIP          0
#define KNodesColumnShortName   1
#define KNodesColumnLongName    2

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

    this->resize(400, 300);

    fillOutputTree();
    fillNodesTree();
}

void ConfigureArtNet::fillOutputTree()
{
    m_outputTree->header()->setResizeMode(QHeaderView::ResizeToContents);

    QList<QNetworkAddressEntry> ifaces = m_plugin->interfaces();
    QList<QString> outputMap = m_plugin->mappedOutputs();
    QList<int> outputPorts = m_plugin->mappedPorts();

    int idx = 0;

    foreach (QNetworkAddressEntry entry, ifaces)
    {
        QString ifaceStr = entry.ip().toString();
        for (int u = 0; u < UNIVERSES_PER_ADDRESS; u++)
        {
            QTreeWidgetItem* pitem = new QTreeWidgetItem(m_outputTree);
            pitem->setFlags(pitem->flags() | Qt::ItemIsUserCheckable);
            if (idx < outputMap.length() && outputMap.at(idx) == ifaceStr && outputPorts.at(idx) == u)
            {
                pitem->setCheckState(KOutputColumnNetwork, Qt::Checked);
                idx++;
            }
            else
                pitem->setCheckState(KOutputColumnNetwork, Qt::Unchecked);
            pitem->setText(KOutputColumnNetwork, ifaceStr);
            pitem->setText(KOutputColumnUniverse, tr("Universe %1").arg(u + 1));
            pitem->setData(KOutputColumnUniverse, Qt::UserRole, u);
        }
    }
}

void ConfigureArtNet::fillNodesTree()
{
    m_nodesTree->header()->setResizeMode(QHeaderView::ResizeToContents);

    ArtNetController *prevController = NULL;

    QList<ArtNetController*> nList = m_plugin->mappedControllers();

    for (int i = 0; i < nList.length(); i++)
    {
        ArtNetController *controller = nList.at(i);
        if (controller != NULL && controller != prevController)
        {
            QTreeWidgetItem* pitem = new QTreeWidgetItem(m_nodesTree);
            pitem->setText(KNodesColumnIP, controller->getNetworkIP() + " nodes");
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
}

ConfigureArtNet::~ConfigureArtNet()
{
}

/*****************************************************************************
 * Dialog actions
 *****************************************************************************/

void ConfigureArtNet::accept()
{
    qDebug() << Q_FUNC_INFO;

    QList<QString> newMappedIPs;
    QList<int> newMappedPorts;

    for (int i = 0; i < m_outputTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_outputTree->topLevelItem(i);
        if (item->checkState(KOutputColumnNetwork) == Qt::Checked)
        {
            newMappedIPs.append(item->text(KOutputColumnNetwork));
            newMappedPorts.append(item->data(KOutputColumnUniverse, Qt::UserRole).toInt());
        }
    }
    m_plugin->remapOutputs(newMappedIPs, newMappedPorts);
    QDialog::accept();
}

int ConfigureArtNet::exec()
{
    return QDialog::exec();
}

