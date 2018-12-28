/*
  Q Light Controller Plus
  editorview.cpp

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
#include "qlcchannel.h"

#include "physicaledit.h"
#include "editorview.h"

EditorView::EditorView(QQuickView *view, QLCFixtureDef *fixtureDef, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_fixtureDef(fixtureDef)
{
    m_globalPhy = new PhysicalEdit(m_fixtureDef->physical(), this);
}

EditorView::~EditorView()
{

}

QString EditorView::manufacturer() const
{
    return m_fixtureDef->manufacturer();
}

void EditorView::setManufacturer(QString manufacturer)
{
    if (m_fixtureDef->manufacturer() == manufacturer)
        return;

    m_fixtureDef->setManufacturer(manufacturer);
    emit manufacturerChanged(manufacturer);
}

QString EditorView::model() const
{
    return m_fixtureDef->model();
}

void EditorView::setModel(QString model)
{
    if (m_fixtureDef->model() == model)
        return;

    m_fixtureDef->setModel(model);
    emit modelChanged(model);
}

QString EditorView::author() const
{
    return m_fixtureDef->author();
}

void EditorView::setAuthor(QString author)
{
    if (m_fixtureDef->author() == author)
        return;

    m_fixtureDef->setAuthor(author);
    emit authorChanged(author);
}

PhysicalEdit *EditorView::globalPhysical()
{
    return m_globalPhy;
}

/************************************************************************
 * Channels
 ************************************************************************/

QVariantList EditorView::channels() const
{
    QVariantList list;

    for (QLCChannel *channel : m_fixtureDef->channels())
    {
        QVariantMap chMap;
        chMap.insert("mIcon", channel->getIconNameFromGroup(channel->group(), true));
        chMap.insert("mLabel", channel->name());
        chMap.insert("mGroup", channel->groupToString(channel->group()));
        list.append(chMap);
    }

    return list;
}
