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

#include "qlcchannel.h"

class QLCCapability;

class ChannelEdit : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QLCChannel *channel READ channel)
    Q_PROPERTY(QVariantList channelPresetList READ channelPresetList CONSTANT)
    Q_PROPERTY(QVariantList channelTypeList READ channelTypeList CONSTANT)
    Q_PROPERTY(int group READ group WRITE setGroup NOTIFY groupChanged)
    Q_PROPERTY(QVariantList capabilityPresetList READ capabilityPresetList CONSTANT)
    Q_PROPERTY(QVariantList capabilities READ capabilities NOTIFY capabilitiesChanged)

public:
    ChannelEdit(QLCChannel *channel, QObject *parent = nullptr);
    ~ChannelEdit();

    QLCChannel *channel();

    QVariantList channelPresetList() const;
    QVariantList channelTypeList() const;
    QVariantList capabilityPresetList() const;

    /** Get/Set the channel's group */
    int group() const;
    void setGroup(int group);

    /** Get the list of capabilities for the channel being edited */
    QVariantList capabilities() const;

    /** Methods to add a new capability */
    Q_INVOKABLE QLCCapability *addNewCapability();
    Q_INVOKABLE QLCCapability *addCapability(int min, int max, QString name);

    /** Delete the capability at the given index */
    Q_INVOKABLE void removeCapabilityAtIndex(int index);

    /** Get/Set a preset for a capability at the given index */
    Q_INVOKABLE int getCapabilityPresetAtIndex(int index);
    Q_INVOKABLE void setCapabilityPresetAtIndex(int index, int preset);

    /** Get the type of preset for a capability at the given index */
    Q_INVOKABLE int getCapabilityPresetType(int index);

    /** Get the units of a preset for a capability at the given index */
    Q_INVOKABLE QString getCapabilityPresetUnits(int index);

    /** Get/Set the value/resource of a preset for a capability at the given index */
    Q_INVOKABLE QVariant getCapabilityValueAt(int index, int vIndex);
    Q_INVOKABLE void setCapabilityValueAt(int index, int vIndex, QVariant value);

    /** Perform a check on a recently modified capability for overlapping and integrity */
    Q_INVOKABLE void checkCapabilities();

private:
    void updateCapabilities();

protected slots:
    void setupPreset();

signals:
    void channelChanged();
    void groupChanged();
    void capabilitiesChanged();

private:
    /** Reference to the channel being edited */
    QLCChannel *m_channel;

    /** List of capabilities used in QML */
    QVariantList m_capabilities;
};

#endif // CHANNELEDIT_H
