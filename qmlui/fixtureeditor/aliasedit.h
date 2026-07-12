/*
  Q Light Controller Plus
  aliasedit.h

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

#ifndef ALIASEDIT_H
#define ALIASEDIT_H

#include <QObject>
#include <QVariant>

#include "qlccapability.h"

class QLCChannel;
class QLCFixtureDef;
class ListModel;

/**
 * AliasEdit exposes the alias list of a single "alias" capability to QML.
 *
 * An alias capability describes one or more channel substitutions that take
 * effect while the owning channel's DMX value falls within the capability
 * range. Each substitution (AliasInfo) is scoped to a fixture mode and swaps
 * a source channel with a target channel.
 *
 * The editor works on the modes that actually contain the source channel and
 * always keeps the underlying QLCCapability alias list in sync, so callers
 * only ever manipulate one capability at a time (as opposed to the legacy Qt
 * editor, which juggled every alias capability of the whole definition through
 * a single combo box).
 */
class AliasEdit final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString capabilityName READ capabilityName CONSTANT)
    Q_PROPERTY(QVariant aliasList READ aliasList NOTIFY aliasListChanged)
    Q_PROPERTY(QStringList applicableModes READ applicableModes CONSTANT)
    Q_PROPERTY(QStringList allChannels READ allChannels CONSTANT)

public:
    AliasEdit(QLCFixtureDef *fixtureDef, QLCChannel *channel,
              QLCCapability *capability, QObject *parent = nullptr);
    ~AliasEdit();

    /** A human readable label for the capability being edited,
     *  e.g. "Control - Sound active [200-255]" */
    QString capabilityName() const;

    /** List model of the current alias rows, one entry per AliasInfo */
    QVariant aliasList() const;

    /** Names of the modes that contain the source channel, and are
     *  therefore eligible to receive an alias */
    QStringList applicableModes() const;

    /** Names of every channel in the definition, usable as a target */
    QStringList allChannels() const;

    /** The source channel used when adding aliases: it is always the channel
     *  owning the capability being edited */
    QString sourceChannelName() const;

    /** Add a single alias for the given mode/target pair. Does nothing if an
     *  identical alias already exists or the parameters are invalid */
    Q_INVOKABLE void addAlias(QString targetMode, QString targetChannel);

    /** Change the target channel of the alias at $index */
    Q_INVOKABLE void setAliasTarget(int index, QString targetChannel);

    /** Change the mode of the alias at $index */
    Q_INVOKABLE void setAliasMode(int index, QString targetMode);

    /** Remove the alias at $index */
    Q_INVOKABLE void removeAliasAtIndex(int index);

    /** Create one alias per applicable mode that does not have one yet,
     *  defaulting the target to $targetChannel (or the first available
     *  channel when empty). Returns the number of aliases added */
    Q_INVOKABLE int applyToAllModes(QString targetChannel);

    /** Return a validation warning string for the alias at $index, or an
     *  empty string when the alias is valid */
    Q_INVOKABLE QString aliasWarning(int index);

private:
    /** Rebuild the QML list model from the capability alias list */
    void updateAliasList();

    /** Push the current model back into the capability */
    void commitAliases(const QList<AliasInfo> &list);

    /** Return the modes (by name) that already have an alias in the list */
    QStringList modesWithAlias(const QList<AliasInfo> &list) const;

signals:
    void aliasListChanged();
    /** Emitted whenever the alias list is modified so the owning editor can
     *  flag the definition as modified */
    void aliasesModified();

private:
    /** The definition being edited (not owned) */
    QLCFixtureDef *m_fixtureDef;
    /** The channel owning the capability (source of the alias) */
    QLCChannel *m_channel;
    /** The alias capability being edited (not owned) */
    QLCCapability *m_capability;
    /** List model exposed to QML */
    ListModel *m_aliasList;
};

#endif // ALIASEDIT_H
