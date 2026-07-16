/*
  Q Light Controller Plus
  configuresignet.cpp

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

#include "configuresignet.h"

#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QSettings>
#include <QSpinBox>
#include <QTreeWidgetItem>

#include "signetplugin.h"

namespace
{
enum MappingColumns
{
    ColumnInterface = 0,
    ColumnUniverse,
    ColumnSigNetUniverse,
    ColumnSenderEndpoint,
    ColumnRdmTuid,
    ColumnRdmEndpoint,
    ColumnNodeAddress
};

enum NodeColumns
{
    NodeTuid = 0,
    NodeAddress,
    NodeLabel,
    NodeRoles,
    NodeEndpoints,
    NodeStatus
};

constexpr int PropUniverse = Qt::UserRole + 0;
constexpr int PropLine = Qt::UserRole + 1;
constexpr int PropType = Qt::UserRole + 2;
constexpr const char* SettingsGeometry = "configuresignet/geometry";
}

ConfigureSigNet::ConfigureSigNet(SigNetPlugin* plugin, QWidget* parent)
    : QDialog(parent)
    , m_plugin(plugin)
{
    Q_ASSERT(plugin != nullptr);

    setupUi(this);

    m_scopeEdit->setText(m_plugin->scope());
    m_tuidEdit->setText(m_plugin->tuidString());
    m_keyEdit->setText(m_plugin->k0Hex());

    QSettings settings;
    m_waitReadySpin->setValue(settings.value(SETTINGS_IFACE_WAIT_TIME, 0).toInt());
    if (settings.contains(SettingsGeometry))
        restoreGeometry(settings.value(SettingsGeometry).toByteArray());

    connect(m_generateTuidButton, &QPushButton::clicked, this, &ConfigureSigNet::slotGenerateTuid);
    connect(m_generateKeyButton, &QPushButton::clicked, this, &ConfigureSigNet::slotGenerateKey);

    fillMappingTree();
    fillNodesTree();
}

ConfigureSigNet::~ConfigureSigNet()
{
    QSettings().setValue(SettingsGeometry, saveGeometry());
}

void ConfigureSigNet::fillMappingTree()
{
    QTreeWidgetItem* inputRoot = nullptr;
    QTreeWidgetItem* outputRoot = nullptr;

    const QList<SigNetIO> ioMap = m_plugin->getIOMapping();
    for (const SigNetIO& io : ioMap)
    {
        SigNetController* controller = io.controller;
        if (!controller)
            continue;

        if ((controller->type() & SigNetController::Input) && inputRoot == nullptr)
        {
            inputRoot = new QTreeWidgetItem(m_uniMapTree);
            inputRoot->setText(ColumnInterface, tr("Inputs"));
            inputRoot->setExpanded(true);
        }

        if ((controller->type() & SigNetController::Output) && outputRoot == nullptr)
        {
            outputRoot = new QTreeWidgetItem(m_uniMapTree);
            outputRoot->setText(ColumnInterface, tr("Outputs"));
            outputRoot->setExpanded(true);
        }

        for (quint32 universe : controller->universesList())
        {
            SigNetUniverseInfo* info = controller->getUniverseInfo(universe);
            if (!info)
                continue;

            auto createSpin = [this](int minimum, int maximum, int value) {
                auto* spin = new QSpinBox(this);
                spin->setRange(minimum, maximum);
                spin->setValue(value);
                return spin;
            };

            auto createEdit = [this](const QString& value) {
                auto* edit = new QLineEdit(this);
                edit->setText(value);
                return edit;
            };

            if (info->type & SigNetController::Input)
            {
                auto* item = new QTreeWidgetItem(inputRoot);
                item->setData(ColumnInterface, PropUniverse, universe);
                item->setData(ColumnInterface, PropLine, controller->line());
                item->setData(ColumnInterface, PropType, SigNetController::Input);
                item->setText(ColumnInterface, controller->getNetworkIP());
                item->setText(ColumnUniverse, QString::number(universe + 1));
                m_uniMapTree->setItemWidget(item, ColumnSigNetUniverse, createSpin(1, 63999, info->signetUniverse));
                item->setText(ColumnSenderEndpoint, QStringLiteral("-"));
                item->setText(ColumnRdmTuid, QStringLiteral("-"));
                item->setText(ColumnRdmEndpoint, QStringLiteral("-"));
                item->setText(ColumnNodeAddress, QStringLiteral("-"));
            }

            if (info->type & SigNetController::Output)
            {
                auto* item = new QTreeWidgetItem(outputRoot);
                item->setData(ColumnInterface, PropUniverse, universe);
                item->setData(ColumnInterface, PropLine, controller->line());
                item->setData(ColumnInterface, PropType, SigNetController::Output);
                item->setText(ColumnInterface, controller->getNetworkIP());
                item->setText(ColumnUniverse, QString::number(universe + 1));
                m_uniMapTree->setItemWidget(item, ColumnSigNetUniverse, createSpin(1, 63999, info->signetUniverse));
                m_uniMapTree->setItemWidget(item, ColumnSenderEndpoint, createSpin(1, 63999, info->senderEndpoint));
                m_uniMapTree->setItemWidget(item, ColumnRdmTuid, createEdit(info->rdmTargetTuid));
                m_uniMapTree->setItemWidget(item, ColumnRdmEndpoint, createSpin(1, 63999, info->rdmTargetEndpoint));
                m_uniMapTree->setItemWidget(item, ColumnNodeAddress, createEdit(info->rdmTargetAddress));
            }
        }
    }

    m_uniMapTree->header()->resizeSections(QHeaderView::ResizeToContents);
}

void ConfigureSigNet::fillNodesTree()
{
    const auto nodes = m_plugin->discoveredNodes();
    for (auto it = nodes.cbegin(); it != nodes.cend(); ++it)
    {
        const SigNetNodeInfo& node = it.value();
        auto* item = new QTreeWidgetItem(m_nodesTree);
        item->setText(NodeTuid, node.tuid);
        item->setText(NodeAddress, node.address.toString());
        item->setText(NodeLabel, node.label);

        QStringList roles;
        if (node.roleCapability & 0x01)
            roles << tr("Node");
        if (node.roleCapability & 0x02)
            roles << tr("Sender");
        if (node.roleCapability & 0x04)
            roles << tr("Manager");
        if (node.roleCapability & 0x08)
            roles << tr("Visualiser");
        item->setText(NodeRoles, roles.join(", "));
        item->setText(NodeEndpoints, QString::number(node.endpointCount));
        item->setText(NodeStatus, node.offboarded ? tr("Offboarded") : tr("Onboarded"));
    }

    m_nodesTree->header()->resizeSections(QHeaderView::ResizeToContents);
}

void ConfigureSigNet::showError(const QString& title, const QString& message)
{
    QMessageBox::critical(this, title, message);
}

void ConfigureSigNet::slotGenerateTuid()
{
    m_tuidEdit->setText(m_plugin->generateTuid());
}

void ConfigureSigNet::slotGenerateKey()
{
    m_keyEdit->setText(m_plugin->generateK0());
}

void ConfigureSigNet::accept()
{
    QByteArray tuidBytes;
    if (!SigNetPacketizer::parseTuid(m_tuidEdit->text().trimmed(), tuidBytes))
    {
        showError(tr("Invalid TUID"), tr("The local TUID must be a 12-character hexadecimal value."));
        return;
    }

    const QString k0Hex = m_keyEdit->text().trimmed().toLower();
    if (!k0Hex.isEmpty())
    {
        const QByteArray k0 = QByteArray::fromHex(k0Hex.toLatin1());
        if (k0Hex.size() != 64 || k0.size() != 32)
        {
            showError(tr("Invalid Root Key"), tr("K0 must be a 64-character hexadecimal value."));
            return;
        }
    }

    m_plugin->setGlobalSettings(m_scopeEdit->text(), m_tuidEdit->text(), k0Hex);

    for (int topIndex = 0; topIndex < m_uniMapTree->topLevelItemCount(); ++topIndex)
    {
        QTreeWidgetItem* rootItem = m_uniMapTree->topLevelItem(topIndex);
        for (int childIndex = 0; childIndex < rootItem->childCount(); ++childIndex)
        {
            QTreeWidgetItem* item = rootItem->child(childIndex);
            const quint32 universe = item->data(ColumnInterface, PropUniverse).toUInt();
            const quint32 line = item->data(ColumnInterface, PropLine).toUInt();
            const auto type = static_cast<SigNetController::Type>(item->data(ColumnInterface, PropType).toInt());
            const QLCIOPlugin::Capability capability = (type == SigNetController::Output) ? QLCIOPlugin::Output : QLCIOPlugin::Input;

            auto* signetUniverse = qobject_cast<QSpinBox*>(m_uniMapTree->itemWidget(item, ColumnSigNetUniverse));
            if (signetUniverse)
                m_plugin->setParameter(universe, line, capability, SIGNET_UNIVERSE, signetUniverse->value());

            if (type == SigNetController::Output)
            {
                auto* endpoint = qobject_cast<QSpinBox*>(m_uniMapTree->itemWidget(item, ColumnSenderEndpoint));
                auto* rdmTuid = qobject_cast<QLineEdit*>(m_uniMapTree->itemWidget(item, ColumnRdmTuid));
                auto* rdmEndpoint = qobject_cast<QSpinBox*>(m_uniMapTree->itemWidget(item, ColumnRdmEndpoint));
                auto* rdmAddress = qobject_cast<QLineEdit*>(m_uniMapTree->itemWidget(item, ColumnNodeAddress));

                if (endpoint)
                    m_plugin->setParameter(universe, line, capability, SIGNET_ENDPOINT, endpoint->value());
                if (rdmTuid)
                    m_plugin->setParameter(universe, line, capability, SIGNET_RDM_TUID, rdmTuid->text().trimmed().toUpper());
                if (rdmEndpoint)
                    m_plugin->setParameter(universe, line, capability, SIGNET_RDM_ENDPOINT, rdmEndpoint->value());
                if (rdmAddress)
                    m_plugin->setParameter(universe, line, capability, SIGNET_RDM_ADDRESS, rdmAddress->text().trimmed());
            }
        }
    }

    QSettings().setValue(SETTINGS_IFACE_WAIT_TIME, m_waitReadySpin->value());
    QDialog::accept();
}
