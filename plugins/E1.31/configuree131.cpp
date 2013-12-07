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
#include <QString>
#include <QDebug>

#include "configuree131.h"
#include "e131plugin.h"

#define UNIVERSES_PER_ADDRESS   4

#define KOutputColumnNetwork   0
#define KOutputColumnUniverse  1

#define KNodesColumnIP          0
#define KNodesColumnShortName   1
#define KNodesColumnLongName    2

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

    this->resize(400, 300);

    fillOutputTree();
}

void ConfigureE131::fillOutputTree()
{
    QList<QNetworkAddressEntry> ifaces = m_plugin->interfaces();
    QList<E131IO> IOmap = m_plugin->getIOMapping();

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

ConfigureE131::~ConfigureE131()
{
}

/*****************************************************************************
 * Dialog actions
 *****************************************************************************/

void ConfigureE131::accept()
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

int ConfigureE131::exec()
{
    return QDialog::exec();
}

