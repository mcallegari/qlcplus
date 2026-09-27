/*
  Q Light Controller Plus
  aliasedit.cpp

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

#include "aliasedit.h"

#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "qlcchannel.h"
#include "listmodel.h"

AliasEdit::AliasEdit(QLCFixtureDef *fixtureDef, QLCChannel *channel,
                     QLCCapability *capability, QObject *parent)
    : QObject(parent)
    , m_fixtureDef(fixtureDef)
    , m_channel(channel)
    , m_capability(capability)
{
    m_aliasList = new ListModel(this);
    QStringList aliasRoles;
    aliasRoles << "mode" << "sourceChannel" << "targetChannel";
    m_aliasList->setRoleNames(aliasRoles);

    updateAliasList();
}

AliasEdit::~AliasEdit()
{
    delete m_aliasList;
}

QString AliasEdit::capabilityName() const
{
    if (m_channel == nullptr || m_capability == nullptr)
        return QString();

    return QString("%1 - %2 [%3-%4]").arg(m_channel->name(), m_capability->name())
                                     .arg(m_capability->min()).arg(m_capability->max());
}

QVariant AliasEdit::aliasList() const
{
    return QVariant::fromValue(m_aliasList);
}

QStringList AliasEdit::applicableModes() const
{
    QStringList list;

    if (m_fixtureDef == nullptr || m_channel == nullptr)
        return list;

    for (QLCFixtureMode *mode : m_fixtureDef->modes())
    {
        if (mode->channel(m_channel->name()) != nullptr)
            list << mode->name();
    }

    return list;
}

QStringList AliasEdit::allChannels() const
{
    QStringList list;

    if (m_fixtureDef == nullptr)
        return list;

    for (QLCChannel *channel : m_fixtureDef->channels())
        list << channel->name();

    return list;
}

QString AliasEdit::sourceChannelName() const
{
    return m_channel != nullptr ? m_channel->name() : QString();
}

void AliasEdit::addAlias(QString targetMode, QString targetChannel)
{
    if (m_capability == nullptr || m_channel == nullptr)
        return;

    if (targetMode.isEmpty() || targetChannel.isEmpty())
        return;

    AliasInfo alias;
    alias.targetMode = targetMode;
    alias.sourceChannel = m_channel->name();
    alias.targetChannel = targetChannel;

    // avoid duplicating an identical alias
    for (const AliasInfo &info : m_capability->aliasList())
    {
        if (info.targetMode == alias.targetMode &&
            info.sourceChannel == alias.sourceChannel &&
            info.targetChannel == alias.targetChannel)
            return;
    }

    m_capability->addAlias(alias);
    updateAliasList();
    emit aliasesModified();
}

void AliasEdit::setAliasTarget(int index, QString targetChannel)
{
    if (m_capability == nullptr)
        return;

    QList<AliasInfo> list = m_capability->aliasList();
    if (index < 0 || index >= list.count())
        return;

    if (list.at(index).targetChannel == targetChannel)
        return;

    AliasInfo info = list.at(index);
    info.targetChannel = targetChannel;
    list.replace(index, info);
    commitAliases(list);
}

void AliasEdit::setAliasMode(int index, QString targetMode)
{
    if (m_capability == nullptr)
        return;

    QList<AliasInfo> list = m_capability->aliasList();
    if (index < 0 || index >= list.count())
        return;

    if (list.at(index).targetMode == targetMode)
        return;

    AliasInfo info = list.at(index);
    info.targetMode = targetMode;
    list.replace(index, info);
    commitAliases(list);
}

void AliasEdit::removeAliasAtIndex(int index)
{
    if (m_capability == nullptr)
        return;

    QList<AliasInfo> list = m_capability->aliasList();
    if (index < 0 || index >= list.count())
        return;

    list.removeAt(index);
    commitAliases(list);
}

int AliasEdit::applyToAllModes(QString targetChannel)
{
    if (m_capability == nullptr || m_channel == nullptr)
        return 0;

    QString target = targetChannel;
    if (target.isEmpty())
    {
        QStringList channels = allChannels();
        if (channels.isEmpty())
            return 0;
        target = channels.first();
    }

    QList<AliasInfo> list = m_capability->aliasList();
    QStringList existing = modesWithAlias(list);
    int added = 0;

    for (const QString &mode : applicableModes())
    {
        if (existing.contains(mode))
            continue;

        AliasInfo alias;
        alias.targetMode = mode;
        alias.sourceChannel = m_channel->name();
        alias.targetChannel = target;
        list.append(alias);
        added++;
    }

    if (added > 0)
        commitAliases(list);

    return added;
}

QString AliasEdit::aliasWarning(int index) const
{
    if (m_capability == nullptr || m_fixtureDef == nullptr)
        return QString();

    QList<AliasInfo> list = m_capability->aliasList();
    if (index < 0 || index >= list.count())
        return QString();

    AliasInfo alias = list.at(index);

    if (alias.sourceChannel == alias.targetChannel)
        return tr("Source and target channels are the same");

    QLCFixtureMode *mode = m_fixtureDef->mode(alias.targetMode);
    if (mode == nullptr)
        return tr("Mode '%1' does not exist").arg(alias.targetMode);

    if (mode->channel(alias.sourceChannel) == nullptr)
        return tr("Channel '%1' is not present in mode '%2'")
                    .arg(alias.sourceChannel, alias.targetMode);

    if (m_fixtureDef->channel(alias.targetChannel) == nullptr)
        return tr("Target channel '%1' does not exist").arg(alias.targetChannel);

    // duplicate mode: another alias already targets the same mode
    for (int i = 0; i < list.count(); i++)
    {
        if (i != index && list.at(i).targetMode == alias.targetMode)
            return tr("Mode '%1' is aliased more than once").arg(alias.targetMode);
    }

    return QString();
}

void AliasEdit::updateAliasList()
{
    m_aliasList->clear();

    if (m_capability == nullptr)
    {
        emit aliasListChanged();
        return;
    }

    for (const AliasInfo &alias : m_capability->aliasList())
    {
        QVariantMap aliasMap;
        aliasMap.insert("mode", alias.targetMode);
        aliasMap.insert("sourceChannel", alias.sourceChannel);
        aliasMap.insert("targetChannel", alias.targetChannel);
        m_aliasList->addDataMap(aliasMap);
    }

    emit aliasListChanged();
}

void AliasEdit::commitAliases(const QList<AliasInfo> &list)
{
    if (m_capability == nullptr)
        return;

    m_capability->replaceAliases(list);
    updateAliasList();
    emit aliasesModified();
}

QStringList AliasEdit::modesWithAlias(const QList<AliasInfo> &list) const
{
    QStringList modes;
    for (const AliasInfo &info : list)
    {
        if (!modes.contains(info.targetMode))
            modes << info.targetMode;
    }
    return modes;
}
