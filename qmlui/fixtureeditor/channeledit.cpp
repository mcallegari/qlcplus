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

#include <QQmlEngine>

#include "qlcfixturedef.h"
#include "qlccapability.h"

#include "channeledit.h"

ChannelEdit::ChannelEdit(QLCChannel *channel, QObject *parent)
    : QObject(parent)
    , m_channel(channel)
{
    if (m_channel->capabilities().count() == 0)
    {
        QLCCapability *cap = new QLCCapability(0, UCHAR_MAX);
        QQmlEngine::setObjectOwnership(cap, QQmlEngine::CppOwnership);
        cap->setWarning(QLCCapability::EmptyName);
        m_channel->addCapability(cap);
    }

    connect(m_channel, SIGNAL(presetChanged()), this, SLOT(setupPreset()));
    connect(m_channel, SIGNAL(nameChanged()), this, SIGNAL(channelChanged()));
    connect(m_channel, SIGNAL(defaultValueChanged()), this, SIGNAL(channelChanged()));
    connect(m_channel, SIGNAL(controlByteChanged()), this, SIGNAL(channelChanged()));

    updateCapabilities();
}

ChannelEdit::~ChannelEdit()
{
    disconnect(m_channel, SIGNAL(presetChanged()), this, SIGNAL(channelChanged()));
    disconnect(m_channel, SIGNAL(nameChanged()), this, SIGNAL(channelChanged()));
    disconnect(m_channel, SIGNAL(defaultValueChanged()), this, SIGNAL(channelChanged()));
    disconnect(m_channel, SIGNAL(controlByteChanged()), this, SIGNAL(channelChanged()));
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

int ChannelEdit::group() const
{
    QLCChannel::Group grp = m_channel->group();

    if (grp == QLCChannel::Intensity)
        return int(m_channel->colour());
    else
        return int(grp);
}

void ChannelEdit::setGroup(int group)
{
    if (group > QLCChannel::Nothing && group < QLCChannel::NoGroup)
    {
        m_channel->setColour(QLCChannel::PrimaryColour(group));
        m_channel->setGroup(QLCChannel::Intensity);
    }
    else
    {
        m_channel->setColour(QLCChannel::NoColour);
        m_channel->setGroup(QLCChannel::Group(group));
    }

    emit groupChanged();
    emit channelChanged();
}

QVariantList ChannelEdit::channelTypeList() const
{
    QVariantList list;

    for (QString &grp : QLCChannel::groupList())
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
            for (QString &color : QLCChannel::colourList())
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

void ChannelEdit::updateCapabilities()
{
    m_capabilities.clear();

    for (QLCCapability *cap : m_channel->capabilities())
    {
        QVariantMap capMap;
        capMap.insert("cRef", QVariant::fromValue(cap));
        m_capabilities.append(capMap);
    }

    emit capabilitiesChanged();
}

void ChannelEdit::setupPreset()
{
    if (m_channel->preset() == QLCChannel::Custom)
    {
        emit channelChanged();
        return;
    }

    for (QLCCapability *cap : m_channel->capabilities())
        m_channel->removeCapability(cap);

    m_channel->addPresetCapability();

    updateCapabilities();

    emit channelChanged();
}

QVariantList ChannelEdit::capabilities() const
{
    return m_capabilities;
}

QLCCapability *ChannelEdit::addNewCapability()
{
    int min = 0;
    if (m_channel->capabilities().count())
    {
        QLCCapability *last = m_channel->capabilities().last();
        min = last->max() + 1;
    }
    QLCCapability *cap = new QLCCapability(min, UCHAR_MAX);
    QQmlEngine::setObjectOwnership(cap, QQmlEngine::CppOwnership);
    cap->setWarning(QLCCapability::EmptyName);
    if (m_channel->addCapability(cap))
    {
        updateCapabilities();
    }
    else
    {
        delete cap;
        return nullptr;
    }

    return cap;
}

QLCCapability *ChannelEdit::addCapability(int min, int max, QString name)
{
    QLCCapability *cap = new QLCCapability(min, max);
    QQmlEngine::setObjectOwnership(cap, QQmlEngine::CppOwnership);
    cap->setName(name);
    if (m_channel->addCapability(cap))
    {
        updateCapabilities();
    }
    else
    {
        delete cap;
        return nullptr;
    }

    return cap;
}

void ChannelEdit::removeCapabilityAtIndex(int index)
{
    QList<QLCCapability *> caps = m_channel->capabilities();

    if (index < 0 || index >= caps.count())
        return;

    if (m_channel->removeCapability(caps[index]))
        updateCapabilities();
}

int ChannelEdit::getCapabilityPresetAtIndex(int index)
{
    QList<QLCCapability *> caps = m_channel->capabilities();

    if (index < 0 || index >= caps.count())
        return 0;

    return caps.at(index)->preset();
}

void ChannelEdit::setCapabilityPresetAtIndex(int index, int preset)
{
    QList<QLCCapability *> caps = m_channel->capabilities();

    if (index < 0 || index >= caps.count())
        return;

    QLCCapability *cap = caps.at(index);
    cap->setPreset(QLCCapability::Preset(preset));
}

int ChannelEdit::getCapabilityPresetType(int index)
{
    QList<QLCCapability *> caps = m_channel->capabilities();

    if (index < 0 || index >= caps.count())
        return QLCCapability::None;

    return caps.at(index)->presetType();
}

QString ChannelEdit::getCapabilityPresetUnits(int index)
{
    QList<QLCCapability *> caps = m_channel->capabilities();

    if (index < 0 || index >= caps.count())
        return QString();

    return caps.at(index)->presetUnits();
}

QVariant ChannelEdit::getCapabilityValueAt(int index, int vIndex)
{
    QList<QLCCapability *> caps = m_channel->capabilities();

    if (index < 0 || index >= caps.count())
        return QVariant();

    return caps.at(index)->resource(vIndex);
}

void ChannelEdit::setCapabilityValueAt(int index, int vIndex, QVariant value)
{
    QList<QLCCapability *> caps = m_channel->capabilities();

    if (index < 0 || index >= caps.count())
        return;

    caps.at(index)->setResource(vIndex, value);
}

void ChannelEdit::checkCapabilities()
{
    QVector<bool>allocation;
    allocation.fill(false, 256);

    QListIterator <QLCCapability*> it(m_channel->capabilities());
    while (it.hasNext() == true)
    {
        QLCCapability *cap = it.next();
        cap->setWarning(QLCCapability::NoWarning);
        if (cap->name().isEmpty())
            cap->setWarning(QLCCapability::EmptyName);

        for (int i = cap->min(); i <= cap->max(); i++)
        {
            if (allocation[i] == true)
                cap->setWarning(QLCCapability::Overlapping);
            else
                allocation[i] = true;
        }
    }
}
