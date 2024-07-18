/*
  Q Light Controller Plus
  inputprofileeditor.cpp

  Copyright (C) Massimo Callegari

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

#include <QDebug>

#include "inputprofileeditor.h"
#include "qlcinputchannel.h"
#include "inputoutputmap.h"
#include "doc.h"

InputProfileEditor::InputProfileEditor(QLCInputProfile *profile, Doc *doc,
                                       QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_profile(profile)
    , m_modified(false)
    , m_detection(false)
    , m_editChannel(nullptr)
{
}

InputProfileEditor::~InputProfileEditor()
{
}

bool InputProfileEditor::modified() const
{
    return m_modified;
}

void InputProfileEditor::setModified(bool newModified)
{
    if (m_modified == newModified)
        return;
    m_modified = newModified;
    emit modifiedChanged();
}

QString InputProfileEditor::manufacturer() const
{
    return m_profile == nullptr ? "" : m_profile->manufacturer();
}

void InputProfileEditor::setManufacturer(const QString &newManufacturer)
{
    if (m_profile == nullptr || m_profile->manufacturer() == newManufacturer)
        return;

    m_profile->setManufacturer(newManufacturer);
    setModified();
    emit manufacturerChanged();
}

QString InputProfileEditor::model() const
{
    return m_profile == nullptr ? "" : m_profile->model();
}

void InputProfileEditor::setModel(const QString &newModel)
{
    if (m_profile == nullptr || m_profile->model() == newModel)
        return;

    m_profile->setModel(newModel);
    setModified();
    emit modelChanged();
}

QLCInputProfile::Type InputProfileEditor::type()
{
    return m_profile == nullptr ? QLCInputProfile::MIDI : m_profile->type();
}

void InputProfileEditor::setType(const QLCInputProfile::Type &newType)
{
    if (m_profile == nullptr || m_profile->type() == newType)
        return;

    m_profile->setType(QLCInputProfile::Type(newType));
    setModified();
    emit typeChanged();
}

bool InputProfileEditor::midiNoteOff()
{
    return m_profile == nullptr ? 0 : m_profile->midiSendNoteOff();
}

void InputProfileEditor::setMidiNoteOff(const bool &newNoteOff)
{
    if (m_profile == nullptr || m_profile->midiSendNoteOff() == newNoteOff)
        return;

    m_profile->setMidiSendNoteOff(newNoteOff);
    setModified();
    emit midiNoteOffChanged();
}

void InputProfileEditor::toggleDetection()
{

    if (m_detection == false)
    {
        /* Listen to input data */
        connect(m_doc->inputOutputMap(), &InputOutputMap::inputValueChanged,
                this, &InputProfileEditor::slotInputValueChanged);
    }
    else
    {
        disconnect(m_doc->inputOutputMap(), &InputOutputMap::inputValueChanged,
                   this, &InputProfileEditor::slotInputValueChanged);
    }
    m_detection = !m_detection;
}

QVariant InputProfileEditor::channels()
{
    if (m_profile == nullptr)
        return QVariant();

    QVariantList chList;

    QMapIterator <quint32, QLCInputChannel*> it(m_profile->channels());
    while (it.hasNext() == true)
    {
        it.next();
        QVariantMap chMap;
        QLCInputChannel *ich = it.value();
        chMap.insert("chNumber", it.key() + 1);
        chMap.insert("cRef", QVariant::fromValue(ich));
        chList.append(chMap);
    }

    return QVariant::fromValue(chList);
}

QVariantList InputProfileEditor::channelTypeModel()
{
    QVariantList types;

    for (QString &type : QLCInputChannel::types())
    {
        QVariantMap typeMap;
        QLCInputChannel::Type typeEnum = QLCInputChannel::stringToType(type);
        typeMap.insert("mLabel", type);
        typeMap.insert("mValue", typeEnum);
        typeMap.insert("mIcon", QLCInputChannel::iconResource(typeEnum, true));
        types.append(typeMap);
    }

    return types;
}

QLCInputChannel *InputProfileEditor::getEditChannel(int channelNumber)
{
    if (m_profile == nullptr)
        return nullptr;

    QLCInputChannel *ich = m_profile->channel(channelNumber);
    if (ich == nullptr)
        m_editChannel = new QLCInputChannel();
    else
        m_editChannel = ich->createCopy();

    return m_editChannel;
}

int InputProfileEditor::saveChannel(int originalChannelNumber, int channelNumber)
{
    if (m_profile == nullptr)
        return -3;

    if (originalChannelNumber >= 0 && originalChannelNumber != channelNumber)
    {
        QLCInputChannel *ich = m_profile->channel(originalChannelNumber);
        if (ich != nullptr)
            return -1;
    }

    if (m_editChannel->name().isEmpty())
        return -2;

    m_profile->removeChannel(originalChannelNumber);
    m_profile->insertChannel(channelNumber, m_editChannel);
    setModified();

    emit channelsChanged();

    return 0;
}

bool InputProfileEditor::removeChannel(int channelNumber)
{
    if (m_profile == nullptr)
        return false;

    if (m_profile->removeChannel(channelNumber))
    {
        emit channelsChanged();
        return true;
    }

    return false;
}

void InputProfileEditor::slotInputValueChanged(quint32 universe, quint32 channel, uchar value, const QString &key)
{
    Q_UNUSED(universe)

    //qDebug() << "Got input value" << universe << channel << value << key;
    QLCInputChannel *ich = m_profile->channel(channel);
    if (ich == nullptr)
    {
        ich = new QLCInputChannel();
        if (key.isEmpty())
            ich->setName(tr("Button %1").arg(channel + 1));
        else
            ich->setName(key);
        ich->setType(QLCInputChannel::Button);
        m_profile->insertChannel(channel, ich);
        m_channelsMap[channel].push_back(value);
        emit channelsChanged();
    }
    else
    {
        QVector<uchar> vect = m_channelsMap[channel];
        if (vect.length() < 3)
        {
            if (!vect.contains(value))
                m_channelsMap[channel].push_back(value);
        }
        else if (vect.length() == 3)
        {
            if (ich->type() == QLCInputChannel::Button)
            {
                ich->setType(QLCInputChannel::Slider);
                if (key.isEmpty())
                    ich->setName(tr("Slider %1").arg(channel + 1));
                else
                    ich->setName(key);
                emit channelsChanged();
            }
        }
    }
}

