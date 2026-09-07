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

#include <algorithm>

#include <QHeaderView>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#include "signetplugin.h"
#include "sig-net-crypto.hpp"
#include "sig-net-constants.hpp"

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
    connect(m_deriveKeyButton, &QPushButton::clicked, this, &ConfigureSigNet::slotDeriveKeyFromPassphrase);

    auto* snowTab = new QWidget(m_tabWidget);
    auto* snowLayout = new QVBoxLayout(snowTab);
    auto* description = new QLabel(tr("SNOW discovers offboarded devices on the local scope and shows their advertised role capabilities. It stores manifest-authorized device identities."), snowTab);
    description->setWordWrap(true);
    snowLayout->addWidget(description);

    m_snowTree = new QTreeWidget(snowTab);
    m_snowTree->setAlternatingRowColors(true);
    m_snowTree->setRootIsDecorated(false);
    m_snowTree->setColumnCount(5);
    m_snowTree->setHeaderLabels(QStringList() << tr("TUID") << tr("Status") << tr("Source / OTW") << tr("Advertised roles") << tr("Trust"));
    snowLayout->addWidget(m_snowTree);

    auto* buttons = new QHBoxLayout();
    auto* refresh = new QPushButton(tr("Refresh"), snowTab);
    auto* importManifest = new QPushButton(tr("Import Manifest…"), snowTab);
    auto* fetchPublicKey = new QPushButton(tr("Request Public Key"), snowTab);
    auto* provision = new QPushButton(tr("Provision Selected"), snowTab);
    buttons->addWidget(refresh);
    buttons->addWidget(importManifest);
    buttons->addWidget(fetchPublicKey);
    buttons->addWidget(provision);
    buttons->addStretch();
    snowLayout->addLayout(buttons);
    m_tabWidget->addTab(snowTab, tr("SNOW Provisioning"));
    connect(refresh, &QPushButton::clicked, this, &ConfigureSigNet::fillSnowTree);
    connect(importManifest, &QPushButton::clicked, this, &ConfigureSigNet::slotImportSnowManifest);
    connect(fetchPublicKey, &QPushButton::clicked, this, &ConfigureSigNet::slotFetchSnowPublicKey);
    connect(provision, &QPushButton::clicked, this, &ConfigureSigNet::slotProvisionSnowDevice);

    fillMappingTree();
    fillNodesTree();
    fillSnowTree();
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
        if (node.roleCapability & SigNet::ROLE_CAP_NODE)
            roles << tr("Node");
        if (node.roleCapability & SigNet::ROLE_CAP_SENDER)
            roles << tr("Sender");
        if (node.roleCapability & SigNet::ROLE_CAP_MANAGER)
            roles << tr("Manager");
        if (node.roleCapability & SigNet::ROLE_CAP_VISUALISER)
            roles << tr("Visualiser");
        item->setText(NodeRoles, roles.isEmpty() ? tr("None advertised") : roles.join(", "));
        item->setText(NodeEndpoints, QString::number(node.endpointCount));
        item->setText(NodeStatus, node.offboarded ? tr("Offboarded") : tr("Onboarded"));

        QList<quint16> rdmEndpoints = node.rdmTodUids.keys();
        std::sort(rdmEndpoints.begin(), rdmEndpoints.end());
        for (quint16 endpoint : std::as_const(rdmEndpoints))
        {
            const QStringList uids = node.rdmTodUids.value(endpoint);
            for (const QString& uid : uids)
            {
                auto* rdmItem = new QTreeWidgetItem(m_nodesTree);
                rdmItem->setText(NodeTuid, uid);
                rdmItem->setText(NodeAddress, node.address.toString());
                rdmItem->setText(NodeLabel, tr("RDM responder (EP %1)").arg(endpoint));
                rdmItem->setText(NodeRoles, tr("Node"));
                rdmItem->setText(NodeEndpoints, QStringLiteral("1"));
                rdmItem->setText(NodeStatus, node.offboarded ? tr("Offboarded") : tr("Onboarded"));
            }
        }
    }

    m_nodesTree->header()->resizeSections(QHeaderView::ResizeToContents);
}

void ConfigureSigNet::fillSnowTree()
{
    if (!m_snowTree)
        return;

    m_snowTree->clear();
    const auto nodes = m_plugin->discoveredNodes();
    const auto trusted = m_plugin->snowTrustedDevices();
    QSet<QString> tuids;
    for (auto it = nodes.cbegin(); it != nodes.cend(); ++it)
    {
        if (it.value().offboarded || it.value().otwPort != 0)
            tuids.insert(it.key());
    }
    for (auto it = trusted.cbegin(); it != trusted.cend(); ++it)
        tuids.insert(it.key());

    for (const QString& tuid : tuids)
    {
        const SigNetNodeInfo node = nodes.value(tuid);
        auto* item = new QTreeWidgetItem(m_snowTree);
        item->setText(0, tuid);
        if (node.tuid.isEmpty())
            item->setText(1, tr("Not discovered"));
        else if (node.beaconCollision)
            item->setText(1, tr("Potential beacon spoofing"));
        else
            item->setText(1, node.offboarded ? tr("Offboarded") : tr("Onboarded"));

        QStringList transports;
        if (node.otwCapabilities & SigNet::OTW_CAP_TLS_1_3)
            transports << QStringLiteral("TLS 1.3");
        if (node.otwCapabilities & SigNet::OTW_CAP_TLS_1_2)
            transports << QStringLiteral("TLS 1.2");
        if (node.otwCapabilities & SigNet::OTW_CAP_DTLS_1_3)
            transports << QStringLiteral("DTLS 1.3");
        if (node.otwCapabilities & SigNet::OTW_CAP_DTLS_1_2)
            transports << QStringLiteral("DTLS 1.2");
        if (node.otwCapabilities & SigNet::OTW_CAP_PIN)
            transports << tr("PIN");
        QString source = node.address.isNull() ? QStringLiteral("-") : node.address.toString();
        if (node.otwPort != 0)
            source += QStringLiteral(":%1").arg(node.otwPort);
        if (!transports.isEmpty())
            source += QStringLiteral(" (%1)").arg(transports.join(QStringLiteral(", ")));
        item->setText(2, source);

        QStringList roles;
        if (node.roleCapability & SigNet::ROLE_CAP_NODE)
            roles << tr("Node");
        if (node.roleCapability & SigNet::ROLE_CAP_SENDER)
            roles << tr("Sender");
        if (node.roleCapability & SigNet::ROLE_CAP_MANAGER)
            roles << tr("Manager");
        item->setText(3, roles.isEmpty() ? tr("None advertised") : roles.join(QStringLiteral(", ")));
        item->setText(4, trusted.contains(tuid) ? tr("Trusted") : tr("Untrusted"));
    }

    m_snowTree->header()->resizeSections(QHeaderView::ResizeToContents);
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

void ConfigureSigNet::slotDeriveKeyFromPassphrase()
{
    QByteArray passphrase = m_passphraseEdit->text().toUtf8();
    const int32_t validation = SigNet::Crypto::ValidatePassphrase(passphrase.constData(), passphrase.size());
    if (validation != SigNet::SIGNET_PASSPHRASE_VALID)
    {
        QString message;
        switch (validation)
        {
            case SigNet::SIGNET_PASSPHRASE_TOO_SHORT:
                message = tr("The passphrase must contain at least 10 characters.");
                break;
            case SigNet::SIGNET_PASSPHRASE_TOO_LONG:
                message = tr("The passphrase may contain at most 64 characters.");
                break;
            case SigNet::SIGNET_PASSPHRASE_INSUFFICIENT_CLASSES:
                message = tr("The passphrase must use at least three of: uppercase letters, lowercase letters, digits, and symbols.");
                break;
            case SigNet::SIGNET_PASSPHRASE_CONSECUTIVE_IDENTICAL:
                message = tr("The passphrase may not contain three identical consecutive characters.");
                break;
            case SigNet::SIGNET_PASSPHRASE_CONSECUTIVE_SEQUENTIAL:
                message = tr("The passphrase may not contain four consecutive sequential characters.");
                break;
            default:
                message = tr("The passphrase could not be validated.");
                break;
        }
        showError(tr("Invalid Passphrase"), message);
        return;
    }

    QByteArray k0(SigNet::K0_KEY_LENGTH, 0);
    if (SigNet::Crypto::DeriveK0FromPassphrase(passphrase.constData(), passphrase.size(),
                                                reinterpret_cast<uint8_t*>(k0.data())) != SigNet::SIGNET_SUCCESS)
    {
        showError(tr("Root Key Generation Failed"), tr("Unable to derive K0 from the passphrase."));
        return;
    }

    m_keyEdit->setText(QString::fromLatin1(k0.toHex()));
    m_passphraseEdit->clear();
    passphrase.fill('\0');
}

void ConfigureSigNet::slotImportSnowManifest()
{
    const QString fileName = QFileDialog::getOpenFileName(this,
                                                          tr("Import SNOW Manifest"),
                                                          QString(),
                                                          tr("JSON files (*.json);;All files (*)"));
    if (fileName.isEmpty())
        return;

    QString error;
    if (!m_plugin->importSnowManifest(fileName, &error))
    {
        showError(tr("SNOW Manifest Import Failed"), error);
        return;
    }

    fillSnowTree();
}

void ConfigureSigNet::slotFetchSnowPublicKey()
{
    if (!m_snowTree || !m_snowTree->currentItem())
    {
        showError(tr("SNOW Public Key"), tr("Select a discovered SNOW device first."));
        return;
    }

    const QString tuid = m_snowTree->currentItem()->text(0);
    QByteArray publicKey;
    QString error;
    if (!m_plugin->fetchSnowDevicePublicKey(tuid, publicKey, &error))
    {
        showError(tr("SNOW Public Key Request Failed"), error);
        return;
    }

    QMessageBox::information(this, tr("SNOW Device Public Key"),
                             tr("%1 returned the following untrusted public key. Verify it with a manifest, visual confirmation, or a PIN before trusting it.\n\nBase64 DER SubjectPublicKeyInfo:\n%2")
                                 .arg(tuid, QString::fromLatin1(publicKey.toBase64())));
}

void ConfigureSigNet::slotProvisionSnowDevice()
{
    if (!m_snowTree || !m_snowTree->currentItem())
    {
        showError(tr("SNOW Provisioning"), tr("Select a discovered, trusted SNOW device first."));
        return;
    }
    const QString tuid = m_snowTree->currentItem()->text(0);
    if (QMessageBox::question(this, tr("Provision SNOW Device"),
                              tr("Provision %1 with the current Sig-Net keys?").arg(tuid)) != QMessageBox::Yes)
        return;
    QString error;
    if (!m_plugin->provisionSnowDevice(tuid, &error))
    {
        showError(tr("SNOW Provisioning Failed"), error);
        return;
    }
    QMessageBox::information(this, tr("SNOW Provisioning"), tr("%1 acknowledged the key commit.").arg(tuid));
    fillSnowTree();
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
