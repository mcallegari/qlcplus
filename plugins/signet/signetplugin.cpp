/*
  Q Light Controller Plus
  signetplugin.cpp

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

#include "signetplugin.h"

#include <QDebug>
#include <QRandomGenerator>
#include <QSettings>
#include <limits>
#include <utility>

#include "configuresignet.h"
#include "signetpacketizer.h"
#include "sig-net-crypto.hpp"

namespace
{
bool addressCompare(const SigNetIO& left, const SigNetIO& right)
{
    return left.address.ip().toString() < right.address.ip().toString();
}

QByteArray hexToBytes(const QString& hex)
{
    QByteArray clean = hex.trimmed().toLatin1();
    if (clean.size() % 2 != 0)
        return QByteArray();
    return QByteArray::fromHex(clean);
}
}

SigNetPlugin::~SigNetPlugin()
{
}

void SigNetPlugin::init()
{
    if (m_IOmapping.isEmpty())
    {
        foreach (const QNetworkInterface& iface, QNetworkInterface::allInterfaces())
        {
            foreach (const QNetworkAddressEntry& entry, iface.addressEntries())
            {
                const QHostAddress addr = entry.ip();
                if (addr.protocol() == QAbstractSocket::IPv6Protocol || addr == QHostAddress::LocalHostIPv6)
                    continue;

                SigNetIO io;
                io.iface = iface;
                io.address = entry;
                bool alreadyPresent = false;
                for (const SigNetIO& existing : std::as_const(m_IOmapping))
                {
                    if (existing.address == io.address)
                    {
                        alreadyPresent = true;
                        break;
                    }
                }
                if (!alreadyPresent)
                    m_IOmapping.append(io);
            }
        }

        std::sort(m_IOmapping.begin(), m_IOmapping.end(), addressCompare);
        loadSettings();
    }
}

void SigNetPlugin::loadSettings()
{
    QSettings settings;
    m_ifaceWaitTime = settings.value(SETTINGS_IFACE_WAIT_TIME, 0).toInt();
    m_scope = settings.value(SETTINGS_SCOPE, QStringLiteral("local")).toString().trimmed();
    if (m_scope.isEmpty())
        m_scope = QStringLiteral("local");

    m_k0Hex = settings.value(SETTINGS_K0).toString().trimmed().toLower();
    m_tuid = settings.value(SETTINGS_TUID).toString().trimmed().toUpper();
    if (m_tuid.isEmpty())
    {
        m_tuid = generateTuid();
        settings.setValue(SETTINGS_TUID, m_tuid);
    }

    QByteArray tuidBytes;
    if (SigNetPacketizer::parseTuid(m_tuid, tuidBytes))
        m_localTuid = tuidBytes;

    deriveKeys();

    m_sessionId = settings.value(SETTINGS_SESSION, 0).toUInt();
    m_messageId = 0;
    incrementSessionOnStartup();
}

void SigNetPlugin::deriveKeys()
{
    QMutexLocker locker(&m_securityMutex);

    m_k0.clear();
    m_senderKey.clear();
    m_citizenKey.clear();
    m_managerGlobalKey.clear();

    const QByteArray k0 = hexToBytes(m_k0Hex);
    if (k0.size() != 32)
        return;

    m_k0 = k0;
    m_senderKey.resize(32);
    m_citizenKey.resize(32);
    m_managerGlobalKey.resize(32);

    SigNet::Crypto::DeriveSenderKey(reinterpret_cast<const uint8_t*>(m_k0.constData()),
                                    reinterpret_cast<uint8_t*>(m_senderKey.data()));
    SigNet::Crypto::DeriveCitizenKey(reinterpret_cast<const uint8_t*>(m_k0.constData()),
                                     reinterpret_cast<uint8_t*>(m_citizenKey.data()));
    SigNet::Crypto::DeriveManagerGlobalKey(reinterpret_cast<const uint8_t*>(m_k0.constData()),
                                           reinterpret_cast<uint8_t*>(m_managerGlobalKey.data()));
}

void SigNetPlugin::incrementSessionOnStartup()
{
    if (!securityReady())
        return;

    if (m_sessionId == std::numeric_limits<quint32>::max())
        return;

    ++m_sessionId;
    QSettings().setValue(SETTINGS_SESSION, m_sessionId);
}

QString SigNetPlugin::name() const
{
    return QStringLiteral("SigNet");
}

int SigNetPlugin::capabilities() const
{
    return QLCIOPlugin::Output | QLCIOPlugin::Input | QLCIOPlugin::Infinite | QLCIOPlugin::RDM;
}

QString SigNetPlugin::pluginInfo() const
{
    QString str;
    str += QStringLiteral("<html><head><title>%1</title></head><body>").arg(name());
    str += QStringLiteral("<p><h3>%1</h3>").arg(name());
    str += tr("This plugin provides secure DMX and RDM transport over the Sig-Net protocol.");
    str += QStringLiteral("</p></body></html>");
    return str;
}

bool SigNetPlugin::requestLine(quint32 line)
{
    int retryCount = 0;
    while (line >= quint32(m_IOmapping.count()))
    {
        if (m_ifaceWaitTime)
        {
            Sleep(1000);
            init();
        }
        if (retryCount++ >= m_ifaceWaitTime)
            return false;
    }

    return true;
}

QStringList SigNetPlugin::outputs()
{
    init();
    QStringList list;
    for (const SigNetIO& io : std::as_const(m_IOmapping))
        list << io.address.ip().toString();
    return list;
}

QStringList SigNetPlugin::inputs()
{
    return outputs();
}

bool SigNetPlugin::openOutput(quint32 output, quint32 universe)
{
    if (!requestLine(output))
        return false;

    if (m_IOmapping[output].controller == nullptr)
    {
        auto* controller = new SigNetController(this, m_IOmapping.at(output).iface, m_IOmapping.at(output).address, output, this);
        connect(controller, &SigNetController::valueChanged, this,
            [this](quint32 universe, quint32 input, quint32 channel, uchar value)
            {
                emit valueChanged(universe, input, channel, value, QString());
            });
        connect(controller, &SigNetController::rdmValueChanged, this, &SigNetPlugin::rdmValueChanged);
        m_IOmapping[output].controller = controller;
    }

    m_IOmapping[output].controller->addUniverse(universe, SigNetController::Output);
    addToMap(universe, output, Output);
    return true;
}

void SigNetPlugin::closeOutput(quint32 output, quint32 universe)
{
    if (output >= quint32(m_IOmapping.count()))
        return;

    removeFromMap(output, universe, Output);
    SigNetController* controller = m_IOmapping.at(output).controller;
    if (controller)
    {
        controller->removeUniverse(universe, SigNetController::Output);
        if (controller->universesList().isEmpty())
        {
            delete controller;
            m_IOmapping[output].controller = nullptr;
        }
    }
}

bool SigNetPlugin::openInput(quint32 input, quint32 universe)
{
    if (!requestLine(input))
        return false;

    if (m_IOmapping[input].controller == nullptr)
    {
        auto* controller = new SigNetController(this, m_IOmapping.at(input).iface, m_IOmapping.at(input).address, input, this);
        connect(controller, &SigNetController::valueChanged, this,
            [this](quint32 universe, quint32 input, quint32 channel, uchar value)
            {
                emit valueChanged(universe, input, channel, value, QString());
            });
        connect(controller, &SigNetController::rdmValueChanged, this, &SigNetPlugin::rdmValueChanged);
        m_IOmapping[input].controller = controller;
    }

    m_IOmapping[input].controller->addUniverse(universe, SigNetController::Input);
    addToMap(universe, input, Input);
    return true;
}

void SigNetPlugin::closeInput(quint32 input, quint32 universe)
{
    if (input >= quint32(m_IOmapping.count()))
        return;

    removeFromMap(input, universe, Input);
    SigNetController* controller = m_IOmapping.at(input).controller;
    if (controller)
    {
        controller->removeUniverse(universe, SigNetController::Input);
        if (controller->universesList().isEmpty())
        {
            delete controller;
            m_IOmapping[input].controller = nullptr;
        }
    }
}

QString SigNetPlugin::outputInfo(quint32 output)
{
    if (output >= quint32(m_IOmapping.length()))
        return QString();

    QString str;
    str += QStringLiteral("<h3>%1 %2</h3><p>").arg(tr("Output"), outputs().value(output));
    SigNetController* controller = m_IOmapping.at(output).controller;
    if (!controller || controller->type() == SigNetController::Input)
    {
        str += tr("Status: Not open");
    }
    else
    {
        str += tr("Status: Open");
        str += QStringLiteral("<br><b>%1:</b> %2").arg(tr("Security ready"), securityReady() ? tr("Yes") : tr("No"));
        str += QStringLiteral("<br><b>%1:</b> %2").arg(tr("Scope"), m_scope);
        str += QStringLiteral("<br><b>%1:</b> %2").arg(tr("Discovered nodes"), controller->discoveredNodes().size());
        str += QStringLiteral("<br><b>%1:</b> %2").arg(tr("Packets sent"), controller->getPacketSentNumber());
    }
    str += QStringLiteral("</p>");
    return str;
}

QString SigNetPlugin::inputInfo(quint32 input)
{
    if (input >= quint32(m_IOmapping.length()))
        return QString();

    QString str;
    str += QStringLiteral("<h3>%1 %2</h3><p>").arg(tr("Input"), inputs().value(input));
    SigNetController* controller = m_IOmapping.at(input).controller;
    if (!controller || controller->type() == SigNetController::Output)
    {
        str += tr("Status: Not open");
    }
    else
    {
        str += tr("Status: Open");
        str += QStringLiteral("<br><b>%1:</b> %2").arg(tr("Packets received"), controller->getPacketReceivedNumber());
    }
    str += QStringLiteral("</p>");
    return str;
}

void SigNetPlugin::writeUniverse(quint32 universe, quint32 output, const QByteArray& data, bool dataChanged)
{
    if (output >= quint32(m_IOmapping.count()))
        return;

    SigNetController* controller = m_IOmapping.at(output).controller;
    if (controller)
        controller->sendDmx(universe, data, dataChanged);
}

void SigNetPlugin::configure()
{
    ConfigureSigNet conf(this);
    conf.exec();
}

bool SigNetPlugin::canConfigure() const
{
    return true;
}

void SigNetPlugin::setParameter(quint32 universe, quint32 line, Capability type, QString name, QVariant value)
{
    if (line >= quint32(m_IOmapping.count()))
        return;

    SigNetController* controller = m_IOmapping[line].controller;
    if (!controller)
        return;

    if (name == QLatin1String(SIGNET_UNIVERSE))
    {
        controller->setSigNetUniverse(universe, value.toUInt());
    }
    else if (name == QLatin1String(SIGNET_ENDPOINT))
    {
        controller->setSenderEndpoint(universe, value.toUInt());
    }
    else if (name == QLatin1String(SIGNET_RDM_TUID))
    {
        SigNetUniverseInfo* info = controller->getUniverseInfo(universe);
        if (info)
            controller->setRdmTarget(universe, value.toString(), info->rdmTargetEndpoint, info->rdmTargetAddress);
    }
    else if (name == QLatin1String(SIGNET_RDM_ENDPOINT))
    {
        SigNetUniverseInfo* info = controller->getUniverseInfo(universe);
        if (info)
            controller->setRdmTarget(universe, info->rdmTargetTuid, value.toUInt(), info->rdmTargetAddress);
    }
    else if (name == QLatin1String(SIGNET_RDM_ADDRESS))
    {
        SigNetUniverseInfo* info = controller->getUniverseInfo(universe);
        if (info)
            controller->setRdmTarget(universe, info->rdmTargetTuid, info->rdmTargetEndpoint, value.toString());
    }

    QLCIOPlugin::setParameter(universe, line, type, name, value);
}

bool SigNetPlugin::sendRDMCommand(quint32 universe, quint32 line, uchar command, QVariantList params)
{
    if (line >= quint32(m_IOmapping.count()))
        return false;

    SigNetController* controller = m_IOmapping.at(line).controller;
    if (!controller)
        return false;

    return controller->sendRDMCommand(universe, command, params);
}

QList<SigNetIO> SigNetPlugin::getIOMapping() const
{
    return m_IOmapping;
}

QHash<QString, SigNetNodeInfo> SigNetPlugin::discoveredNodes() const
{
    QHash<QString, SigNetNodeInfo> nodes;
    for (const SigNetIO& io : m_IOmapping)
    {
        if (!io.controller)
            continue;

        const auto controllerNodes = io.controller->discoveredNodes();
        for (auto it = controllerNodes.cbegin(); it != controllerNodes.cend(); ++it)
            nodes.insert(it.key(), it.value());
    }

    return nodes;
}

QString SigNetPlugin::scope() const
{
    return m_scope;
}

QString SigNetPlugin::k0Hex() const
{
    return m_k0Hex;
}

QString SigNetPlugin::tuidString() const
{
    return m_tuid;
}

QByteArray SigNetPlugin::localTuidBytes() const
{
    return m_localTuid;
}

QByteArray SigNetPlugin::senderKey() const
{
    QMutexLocker locker(&m_securityMutex);
    return m_senderKey;
}

QByteArray SigNetPlugin::citizenKey() const
{
    QMutexLocker locker(&m_securityMutex);
    return m_citizenKey;
}

QByteArray SigNetPlugin::managerGlobalKey() const
{
    QMutexLocker locker(&m_securityMutex);
    return m_managerGlobalKey;
}

QByteArray SigNetPlugin::managerLocalKey(const QString& targetTuid) const
{
    QMutexLocker locker(&m_securityMutex);
    if (m_k0.size() != 32)
        return QByteArray();

    QByteArray tuidBytes;
    if (!SigNetPacketizer::parseTuid(targetTuid, tuidBytes))
        return QByteArray();

    QByteArray key(32, 0);
    if (SigNet::Crypto::DeriveManagerLocalKey(reinterpret_cast<const uint8_t*>(m_k0.constData()),
                                              reinterpret_cast<const uint8_t*>(tuidBytes.constData()),
                                              reinterpret_cast<uint8_t*>(key.data())) != SigNet::SIGNET_SUCCESS)
    {
        return QByteArray();
    }

    return key;
}

bool SigNetPlugin::securityReady() const
{
    QMutexLocker locker(&m_securityMutex);
    return m_senderKey.size() == 32 && m_citizenKey.size() == 32 && m_managerGlobalKey.size() == 32;
}

quint32 SigNetPlugin::sessionId() const
{
    return m_sessionId;
}

quint32 SigNetPlugin::nextSequence(quint16 endpoint)
{
    if (!m_endpointSequences.contains(endpoint))
    {
        m_endpointSequences.insert(endpoint, 1);
        return 1;
    }

    quint32& sequence = m_endpointSequences[endpoint];
    if (sequence == std::numeric_limits<quint32>::max())
    {
        ++m_sessionId;
        QSettings().setValue(SETTINGS_SESSION, m_sessionId);
        m_endpointSequences.clear();
        m_endpointSequences.insert(endpoint, 1);
        return 1;
    }

    ++sequence;
    return sequence;
}

quint16 SigNetPlugin::nextMessageId()
{
    if (m_messageId == std::numeric_limits<quint16>::max())
        m_messageId = 1;
    else
        ++m_messageId;

    return m_messageId;
}

void SigNetPlugin::setGlobalSettings(const QString& scope, const QString& tuid, const QString& k0Hex)
{
    m_scope = scope.trimmed().isEmpty() ? QStringLiteral("local") : scope.trimmed();
    m_tuid = tuid.trimmed().toUpper();
    m_k0Hex = k0Hex.trimmed().toLower();

    QByteArray tuidBytes;
    if (SigNetPacketizer::parseTuid(m_tuid, tuidBytes))
        m_localTuid = tuidBytes;

    deriveKeys();

    QSettings settings;
    settings.setValue(SETTINGS_SCOPE, m_scope);
    settings.setValue(SETTINGS_TUID, m_tuid);
    settings.setValue(SETTINGS_K0, m_k0Hex);
}

QString SigNetPlugin::generateTuid() const
{
    QByteArray tuid(6, 0);
    tuid[0] = char(0x7F);
    tuid[1] = char(0xF8);
    const quint32 deviceId = 0x80000000u | (QRandomGenerator::global()->generate() & 0x7FFFFFFFu);
    tuid[2] = char((deviceId >> 24) & 0xFF);
    tuid[3] = char((deviceId >> 16) & 0xFF);
    tuid[4] = char((deviceId >> 8) & 0xFF);
    tuid[5] = char(deviceId & 0xFF);
    return SigNetPacketizer::tuidToString(tuid);
}

QString SigNetPlugin::generateK0() const
{
    QByteArray k0(32, 0);
    for (int i = 0; i < k0.size(); ++i)
        k0[i] = char(QRandomGenerator::global()->bounded(0, 256));
    return QString::fromLatin1(k0.toHex());
}
