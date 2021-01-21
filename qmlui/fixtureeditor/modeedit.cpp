/*
  Q Light Controller Plus
  modeedit.cpp

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

#include "qlcfixturemode.h"
#include "physicaledit.h"

#include "modeedit.h"

ModeEdit::ModeEdit(QLCFixtureMode *mode, QObject *parent)
    : QObject(parent)
    , m_mode(mode)
    , m_physical(nullptr)
{

}

ModeEdit::~ModeEdit()
{

}

QString ModeEdit::name() const
{
    return m_mode->name();
}

void ModeEdit::setName(QString name)
{
    if (name == m_mode->name())
        return;

    m_mode->setName(name);
    emit nameChanged();
}

bool ModeEdit::useGlobalPhysical()
{
    return m_mode->useGlobalPhysical();
}

PhysicalEdit *ModeEdit::physical()
{
    if (m_physical == nullptr)
        m_physical = new PhysicalEdit(m_mode->physical());
    return m_physical;
}

QVariantList ModeEdit::channels() const
{
    QVariantList list;

    for (QLCChannel *channel : m_mode->channels())
    {
        QVariantMap chMap;
        chMap.insert("mIcon", channel->getIconNameFromGroup(channel->group(), true));
        chMap.insert("mLabel", channel->name());
        chMap.insert("mGroup", channel->groupToString(channel->group()));
        list.append(chMap);
    }

    return list;
}


