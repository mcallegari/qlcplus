/*
  Q Light Controller Plus
  rdmmanager.cpp

  Copyright (c) Massimo Callegari

  Licensed under the Apache License Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing software
  distributed under the License is distributed on an "AS IS" BASIS
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/


#include <QMessageBox>
#include <QDebug>

#include "rdmmanager.h"

// RDM view column numbers
#define KColumnRDMModel    0
#define KColumnRDMUniverse 1
#define KColumnRDMAddress  2
#define KColumnRDMChannels 3
#define KColumnRDMUID      4

RDMManager::RDMManager(QWidget *parent, QObject *service)
    : QWidget(parent)
    , m_service(qobject_cast<QLCRDMService*>(service))
{
    Q_ASSERT(m_service != NULL);

    setupUi(this);

    m_getInfoButton->setEnabled(false);
    m_readButton->setEnabled(false);

    connect(m_refreshButton, SIGNAL(clicked()), this, SLOT(slotRefresh()));
    connect(m_getInfoButton, SIGNAL(clicked()), this, SLOT(slotGetInfo()));
    connect(m_rdmTree, SIGNAL(itemSelectionChanged()), this, SLOT(slotSelectionChanged()));
    connect(m_readButton, SIGNAL(clicked()), this, SLOT(slotReadPID()));
    connect(m_writeButton, SIGNAL(clicked()), this, SLOT(slotWritePID()));

    // the service is a QObject implementing the QLCRDMService interface
    connect(service, SIGNAL(deviceFound(QLCRDMDevice)),
            this, SLOT(slotDeviceFound(QLCRDMDevice)));
    connect(service, SIGNAL(deviceInfoReady(QString)),
            this, SLOT(slotDeviceInfoReady(QString)));
    connect(service, SIGNAL(pidResultReady(QString)),
            this, SLOT(slotUpdatePidInfo(QString)));
    connect(service, SIGNAL(message(QString, QString)),
            this, SLOT(slotDisplayMessage(QString, QString)));
    connect(service, SIGNAL(busyChanged(bool)),
            this, SLOT(slotBusyChanged(bool)));
}

RDMManager::~RDMManager()
{
}

QString RDMManager::selectedUID() const
{
    QList<QTreeWidgetItem*> items = m_rdmTree->selectedItems();
    if (items.isEmpty())
        return QString();

    return items.first()->text(KColumnRDMUID);
}

void RDMManager::slotRefresh()
{
    // reset any previously collected information
    m_rdmTree->clear();

    m_devFoundLabel->setText("Discovering fixtures...");

    m_service->discoverAll();
}

void RDMManager::slotGetInfo()
{
    QString UID = selectedUID();
    if (UID.isEmpty())
        return;

    m_service->queryDevice(UID);
}

void RDMManager::slotReadPID()
{
    QString UID = selectedUID();
    if (UID.isEmpty())
        return;

    m_pidResult->clear();
    m_service->readPid(UID, m_pidEdit->text(), m_dataTypeCombo->currentIndex(), m_pidArgsEdit->text());
}

void RDMManager::slotWritePID()
{
    QString UID = selectedUID();
    if (UID.isEmpty())
        return;

    m_pidResult->clear();
    m_service->writePid(UID, m_pidEdit->text(), m_dataTypeCombo->currentIndex(), m_pidArgsEdit->text());
}

void RDMManager::slotSelectionChanged()
{
    int selectedCount = m_rdmTree->selectedItems().size();

    m_getInfoButton->setEnabled(selectedCount ? true : false);
    m_readButton->setEnabled(selectedCount ? true : false);
}

void RDMManager::slotUpdatePidInfo(QString info)
{
    m_pidResult->setText(info);
}

void RDMManager::slotDeviceInfoReady(QString html)
{
    emit fixtureInfoReady(html);
}

void RDMManager::slotDisplayMessage(QString title, QString text)
{
    QMessageBox::information(this, title, text);
}

void RDMManager::slotBusyChanged(bool busy)
{
    m_refreshButton->setEnabled(!busy);
}

void RDMManager::slotDeviceFound(QLCRDMDevice device)
{
    QTreeWidgetItem *item = NULL;

    for (int i = 0; i < m_rdmTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *tlItem = m_rdmTree->topLevelItem(i);
        if (tlItem->text(KColumnRDMUID) == device.uid)
        {
            item = tlItem;
            break;
        }
    }

    if (item == NULL)
    {
        item = new QTreeWidgetItem(m_rdmTree);
        item->setText(KColumnRDMUID, device.uid);
    }

    item->setText(KColumnRDMModel, QString ("%1 - %2").arg(device.manufacturer).arg(device.model));
    item->setText(KColumnRDMUniverse, QString::number(device.universe + 1));
    item->setText(KColumnRDMAddress,  QString::number(device.address));
    item->setText(KColumnRDMChannels, QString::number(device.channels));

    m_rdmTree->header()->resizeSections(QHeaderView::ResizeToContents);

    if (m_rdmTree->topLevelItemCount())
        m_devFoundLabel->setText(QString("Fixtures found: %1").arg(m_rdmTree->topLevelItemCount()));
    else
        m_devFoundLabel->setText("No fixtures found");
}
