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

    Q_PROPERTY(QVariantList channelPresetList READ channelPresetList CONSTANT)
    Q_PROPERTY(QVariantList channelTypeList READ channelTypeList CONSTANT)
    Q_PROPERTY(QVariantList capabilityPresetList READ capabilityPresetList CONSTANT)

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(int preset READ preset WRITE setPreset NOTIFY presetChanged)
    Q_PROPERTY(int group READ group WRITE setGroup NOTIFY groupChanged)

    Q_PROPERTY(int controlByte READ controlByte WRITE setControlByte NOTIFY controlByteChanged)
    Q_PROPERTY(int defaultValue READ defaultValue WRITE setDefaultValue NOTIFY defaultValueChanged)

    Q_PROPERTY(QVariantList capabilities READ capabilities NOTIFY capabilitiesChanged)

public:
    ChannelEdit(QLCChannel *channel, QObject *parent = nullptr);
    ~ChannelEdit();

    QVariantList channelPresetList() const;
    QVariantList channelTypeList() const;
    QVariantList capabilityPresetList() const;

    /** Get/Set the name of the channel being edited */
    QString name() const;
    void setName(QString name);

    /** Get/Set the preset of the channel being edited */
    int preset() const;
    void setPreset(int index);

    /** Get/Set the group of the channel being edited */
    int group() const;
    void setGroup(int group);

    /** Get/Set the byte role of the channel being edited */
    int controlByte() const;
    void setControlByte(int byte);

    /** Get/Set the default value of the channel being edited */
    int defaultValue() const;
    void setDefaultValue(int value);

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
    void nameChanged();
    void presetChanged();
    void groupChanged();
    void controlByteChanged();
    void defaultValueChanged();
    void capabilitiesChanged();

private:
    /** Reference to the channel being edited */
    QLCChannel *m_channel;
};

#endif // CHANNELEDIT_H
