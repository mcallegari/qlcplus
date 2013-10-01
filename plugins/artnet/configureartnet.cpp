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
    QList<QNetworkAddressEntry> ifaces = m_plugin->interfaces();
    QList<ArtNetIO> IOmap = m_plugin->getIOMapping();

    foreach (QNetworkAddressEntry entry, ifaces)
    {
        QString ifaceStr = entry.ip().toString();
        for (int u = 0; u < UNIVERSES_PER_ADDRESS; u++)
        {
            QTreeWidgetItem* pitem = new QTreeWidgetItem(m_outputTree);
            pitem->setFlags(pitem->flags() | Qt::ItemIsUserCheckable);
            pitem->setCheckState(KOutputColumnNetwork, Qt::Unchecked);
            for (int idx = 0; idx < IOmap.length(); idx++)
            {
                if (IOmap.at(idx).IPAddress == ifaceStr && IOmap.at(idx).port == u)
                {
                    pitem->setCheckState(KOutputColumnNetwork, Qt::Checked);
                    break;
                }
            }
            pitem->setText(KOutputColumnNetwork, ifaceStr);
            pitem->setText(KOutputColumnUniverse, tr("Universe %1").arg(u));
            pitem->setData(KOutputColumnUniverse, Qt::UserRole, u);
        }
    }

    m_outputTree->resizeColumnToContents(KOutputColumnNetwork);
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

    m_nodesTree->resizeColumnToContents(KNodesColumnIP);
    m_nodesTree->resizeColumnToContents(KNodesColumnShortName);
    m_nodesTree->resizeColumnToContents(KNodesColumnLongName);
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

