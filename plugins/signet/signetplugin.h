/*
  Q Light Controller Plus
  signetplugin.h

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

#ifndef SIGNETPLUGIN_H
#define SIGNETPLUGIN_H

#include <QByteArray>
#include <QDateTime>
#include <QHash>
#include <QMutex>
#include <QNetworkAddressEntry>
#include <QNetworkInterface>

#include "qlcioplugin.h"
#include "signetcontroller.h"

class QSettings;

#define SIGNET_UNIVERSE "signetUniverse"
#define SIGNET_ENDPOINT "signetEndpoint"
#define SIGNET_RDM_TUID "rdmTuid"
#define SIGNET_RDM_ENDPOINT "rdmEndpoint"
#define SIGNET_RDM_ADDRESS "rdmAddress"

#define SETTINGS_SCOPE "SigNetPlugin/scope"
#define SETTINGS_K0 "SigNetPlugin/k0"
#define SETTINGS_TUID "SigNetPlugin/tuid"
#define SETTINGS_SESSION "SigNetPlugin/session"
#define SETTINGS_IFACE_WAIT_TIME "SigNetPlugin/ifacewait"
#define SETTINGS_SNOW_TRUSTED "SigNetPlugin/snow/trusted"
#define SETTINGS_SNOW_MANIFEST_POM "SigNetPlugin/snow/manifestPomPublicKey"
#define SETTINGS_SNOW_POM_PRIVATE "SigNetPlugin/snow/pomPrivateKey"
#define SETTINGS_SNOW_POM_PUBLIC "SigNetPlugin/snow/pomPublicKey"

struct SigNetIO
{
    QNetworkInterface iface;
    QNetworkAddressEntry address;
    SigNetController* controller = nullptr;
};

struct SigNetSnowTrustedDevice
{
    QString tuid;
    QByteArray publicKey;
    QDateTime trustedAt;
};

class SigNetPlugin final : public QLCIOPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCIOPlugin)
    Q_PLUGIN_METADATA(IID QLCIOPlugin_iid)

public:
    ~SigNetPlugin() override;

    void init() override;
    QString name() const override;
    int capabilities() const override;
    QString pluginInfo() const override;

    bool openOutput(quint32 output, quint32 universe) override;
    void closeOutput(quint32 output, quint32 universe) override;
    QStringList outputs() override;
    QString outputInfo(quint32 output) override;
    void writeUniverse(quint32 universe, quint32 output, const QByteArray& data, bool dataChanged) override;

    bool openInput(quint32 input, quint32 universe) override;
    void closeInput(quint32 input, quint32 universe) override;
    QStringList inputs() override;
    QString inputInfo(quint32 input) override;

    void configure() override;
    bool canConfigure() const override;
    void setParameter(quint32 universe, quint32 line, Capability type, QString name, QVariant value) override;
    bool sendRDMCommand(quint32 universe, quint32 line, uchar command, QVariantList params) override;

    QList<SigNetIO> getIOMapping() const;
    QHash<QString, SigNetNodeInfo> discoveredNodes() const;
    QHash<QString, SigNetSnowTrustedDevice> snowTrustedDevices() const;
    QByteArray snowManifestPomPublicKey() const;
    bool importSnowManifest(const QString& fileName, QString* error = nullptr);
    bool importSnowManifestData(const QByteArray& manifest, QString* error = nullptr);
    void revokeSnowDevice(const QString& tuid);
    QByteArray snowPomPublicKey(QString* error = nullptr);
    bool fetchSnowDevicePublicKey(const QString& tuid, QByteArray& publicKey, QString* error = nullptr);
    bool provisionSnowDevice(const QString& tuid, QString* error = nullptr);

    QString scope() const;
    QString k0Hex() const;
    QString tuidString() const;
    QByteArray localTuidBytes() const;
    QByteArray senderKey() const;
    QByteArray citizenKey() const;
    QByteArray managerGlobalKey() const;
    QByteArray managerLocalKey(const QString& targetTuid) const;
    bool securityReady() const;

    quint32 sessionId() const;
    quint32 nextSequence(quint16 endpoint);
    quint16 nextMessageId();

    void setGlobalSettings(const QString& scope, const QString& tuid, const QString& k0Hex);
    QString generateTuid() const;
    QString generateK0() const;

signals:
    void rdmValueChanged(quint32 universe, quint32 line, QVariantMap data);

private:
    bool requestLine(quint32 line);
    void loadSettings();
    void loadSnowTrustDirectory(QSettings& settings);
    bool ensureSnowPomKey(QString* error);
    void deriveKeys();
    void incrementSessionOnStartup();

private:
    QList<SigNetIO> m_IOmapping;
    int m_ifaceWaitTime;
    QString m_scope;
    QString m_k0Hex;
    QString m_tuid;
    QByteArray m_localTuid;
    QByteArray m_k0;
    QByteArray m_senderKey;
    QByteArray m_citizenKey;
    QByteArray m_managerGlobalKey;
    QHash<QString, SigNetSnowTrustedDevice> m_snowTrustedDevices;
    QByteArray m_snowManifestPomPublicKey;
    QByteArray m_snowPomPrivateKey;
    QByteArray m_snowPomPublicKey;
    mutable QMutex m_securityMutex;
    quint32 m_sessionId;
    quint16 m_messageId;
    QHash<quint16, quint32> m_endpointSequences;
};

#endif
