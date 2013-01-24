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

#define KColumnNetwork   0
#define KColumnUniverse  1

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

    m_outputTree->header()->setResizeMode(QHeaderView::ResizeToContents);

    QList<QHostAddress> ifaces = m_plugin->interfaces();
    QList<QString> outputMap = m_plugin->mappedOutputs();
    QList<int> outputPorts = m_plugin->mappedPorts();

    int idx = 0;

    for (int i = 0; i < ifaces.length(); i++)
    {
        QString ifaceStr = ifaces.at(i).toString();
        for (int u = 0; u < UNIVERSES_PER_ADDRESS; u++)
        {
            QTreeWidgetItem* pitem = new QTreeWidgetItem(m_outputTree);
            pitem->setFlags(pitem->flags() | Qt::ItemIsUserCheckable);
            if (idx < outputMap.length() && outputMap.at(idx) == ifaceStr && outputPorts.at(idx) == u)
            {
                pitem->setCheckState(KColumnNetwork, Qt::Checked);
                idx++;
            }
            else
                pitem->setCheckState(KColumnNetwork, Qt::Unchecked);
            pitem->setText(KColumnNetwork, ifaceStr);
            pitem->setText(KColumnUniverse, tr("Universe %1").arg(u + 1));
            pitem->setData(KColumnUniverse, Qt::UserRole, u);
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
        if (item->checkState(KColumnNetwork) == Qt::Checked)
        {
            newMappedIPs.append(item->text(KColumnNetwork));
            newMappedPorts.append(item->data(KColumnUniverse, Qt::UserRole).toInt());
        }
    }
    m_plugin->remapOutputs(newMappedIPs, newMappedPorts);
    QDialog::accept();
}

int ConfigureArtNet::exec()
{
    return QDialog::exec();
}

