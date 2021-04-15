/*
  Q Light Controller Plus
  channeledit.h

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

#ifndef CHANNELEDIT_H
#define CHANNELEDIT_H

#include <QQuickView>

class QLCChannel;

class ChannelEdit : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QLCChannel *channel READ channel)
    Q_PROPERTY(QVariantList channelPresetList READ channelPresetList CONSTANT)
    Q_PROPERTY(QVariantList channelTypeList READ channelTypeList CONSTANT)
    Q_PROPERTY(QVariantList capabilityPresetList READ capabilityPresetList CONSTANT)
    Q_PROPERTY(QVariantList capabilities READ capabilities NOTIFY capabilitiesChanged)

public:
    ChannelEdit(QLCChannel *channel, QObject *parent = nullptr);
    ~ChannelEdit();

    QLCChannel *channel();

    QVariantList channelPresetList() const;
    QVariantList channelTypeList() const;
    QVariantList capabilityPresetList() const;

    /** Get the list of capabilities for the channel being edited */
    QVariantList capabilities() const;

    /** Get the selected preset for a capability at the given index */
    Q_INVOKABLE int getCapabilityPresetAtIndex(int index);

    /** Get the type of preset for a capability at the given index */
    Q_INVOKABLE int getCapabilityPresetType(int index);

    /** Get the units of a preset for a capability at the given index */
    Q_INVOKABLE QString getCapabilityPresetUnits(int index);

    /** Get the value/resource of a preset for a capability at the given index */
    Q_INVOKABLE QVariant getCapabilityValueAt(int index, int vIndex);

signals:
    void channelChanged();
    void capabilitiesChanged();

private:
    /** Reference to the channel being edited */
    QLCChannel *m_channel;
};

#endif // CHANNELEDIT_H
