/*
  Q Light Controller Plus
  channeledit.cpp

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

#include "qlcfixturedef.h"
#include "qlccapability.h"
#include "qlcchannel.h"

#include "channeledit.h"

ChannelEdit::ChannelEdit(QLCChannel *channel, QObject *parent)
    : QObject(parent)
    , m_channel(channel)
{

}

ChannelEdit::~ChannelEdit()
{

}

QLCChannel *ChannelEdit::channel()
{
    return m_channel;
}

QVariantList ChannelEdit::channelPresetList() const
{
    QVariantList list;

    QVariantMap custom;
    custom.insert("mIcon", "qrc:/edit.svg");
    custom.insert("mLabel", tr("Custom"));
    custom.insert("mValue", 0);
    list.append(custom);

    for (int i = QLCChannel::Custom + 1; i < QLCChannel::LastPreset; i++)
    {
        QLCChannel ch;
        ch.setPreset(QLCChannel::Preset(i));
        QVariantMap chMap;
        chMap.insert("mIcon", ch.getIconNameFromGroup(ch.group(), true));
        chMap.insert("mLabel", ch.name() + " (" + ch.presetToString(QLCChannel::Preset(i)) + ")");
        chMap.insert("mValue", i);
        list.append(chMap);
    }

    return list;
}

QVariantList ChannelEdit::capabilityPresetList() const
{
    QVariantList list;

    QVariantMap custom;
    //custom.insert("mIcon", "qrc:/edit.svg");
    custom.insert("mLabel", tr("Custom"));
    custom.insert("mValue", 0);
    list.append(custom);

    for (int i = QLCCapability::Custom + 1; i < QLCCapability::LastPreset; i++)
    {
        QLCCapability cap;
        QVariantMap capMap;
        cap.setPreset(QLCCapability::Preset(i));
        capMap.insert("mLabel", cap.presetToString(QLCCapability::Preset(i)));
        capMap.insert("mValue", i);
        list.append(capMap);
    }

    return list;
}

QVariantList ChannelEdit::channelTypeList() const
{
    QVariantList list;

    for (QString grp : QLCChannel::groupList())
    {
        QLCChannel ch;
        ch.setGroup(QLCChannel::stringToGroup(grp));

        QVariantMap chMap;
        chMap.insert("mIcon", ch.getIconNameFromGroup(ch.group(), true));
        chMap.insert("mLabel", ch.groupToString(ch.group()));
        chMap.insert("mValue", ch.group());
        list.append(chMap);

        if (ch.group() == QLCChannel::Intensity)
        {
            for (QString color : QLCChannel::colourList())
            {
                QLCChannel cc;
                cc.setGroup(QLCChannel::Intensity);
                cc.setColour(QLCChannel::stringToColour(color));

                QVariantMap chMap;
                chMap.insert("mIcon", cc.getIconNameFromGroup(ch.group(), true));
                chMap.insert("mLabel", cc.colourToString(cc.colour()));
                chMap.insert("mValue", cc.colour());
                list.append(chMap);
            }
        }
    }

    return list;
}

QVariantList ChannelEdit::capabilities() const
{
    QVariantList list;

    for (QLCCapability *cap : m_channel->capabilities())
    {
        QVariantMap capMap;
        capMap.insert("iMin", cap->min());
        capMap.insert("iMax", cap->max());
        capMap.insert("sDesc", cap->name());
        list.append(capMap);
    }

    return list;
}

int ChannelEdit::getCapabilityPresetAtIndex(int index)
{
    int i = 0;

    for (QLCCapability *cap : m_channel->capabilities())
    {
        if (i == index)
            return cap->preset();

        i++;
    }

    return 0;
}

int ChannelEdit::getCapabilityPresetType(int index)
{
    int i = 0;

    for (QLCCapability *cap : m_channel->capabilities())
    {
        if (i == index)
            return cap->presetType();

        i++;
    }

    return QLCCapability::None;
}

QString ChannelEdit::getCapabilityPresetUnits(int index)
{
    int i = 0;

    for (QLCCapability *cap : m_channel->capabilities())
    {
        if (i == index)
            return cap->presetUnits();

        i++;
    }

    return QString();
}

QVariant ChannelEdit::getCapabilityValueAt(int index, int vIndex)
{
    int i = 0;

    for (QLCCapability *cap : m_channel->capabilities())
    {
        if (i == index)
            return cap->resource(vIndex);

        i++;
    }

    return QVariant();
}

