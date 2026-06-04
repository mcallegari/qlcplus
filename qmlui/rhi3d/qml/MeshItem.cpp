/*
  Q Light Controller Plus
  MeshItem.cpp

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

#include "qml/MeshItem.h"

#include <QtQuick/QQuickItem>

MeshItem::MeshItem(QObject *parent)
    : QObject(parent)
{
}

void MeshItem::setItemID(quint32 itemID)
{
    if (m_itemID == itemID)
        return;
    m_itemID = itemID;
    emit itemIDChanged();
    notifyParent();
}

void MeshItem::setPosition(const QVector3D &position)
{
    if (m_position == position)
        return;
    m_position = position;
    emit positionChanged();
    notifyParent();
}

void MeshItem::setRotationDegrees(const QVector3D &rotation)
{
    if (m_rotationDegrees == rotation)
        return;
    m_rotationDegrees = rotation;
    emit rotationDegreesChanged();
    notifyParent();
}

void MeshItem::setScale(const QVector3D &scale)
{
    if (m_scale == scale)
        return;
    m_scale = scale;
    emit scaleChanged();
    notifyParent();
}

void MeshItem::setIsSelected(bool selected)
{
    if (m_isSelected == selected)
        return;
    m_isSelected = selected;
    emit isSelectedChanged();
    notifyParent();
}

void MeshItem::setSelectable(bool selectable)
{
    if (m_selectable == selectable)
        return;
    m_selectable = selectable;
    emit selectableChanged();
    if (!m_selectable && m_isSelected)
    {
        m_isSelected = false;
        emit isSelectedChanged();
    }
    notifyParent();
}

void MeshItem::setVisible(bool visible)
{
    if (m_visible == visible)
        return;
    m_visible = visible;
    emit visibleChanged();
    notifyParent();
}

void MeshItem::notifyParent()
{
    auto *item = qobject_cast<QQuickItem *>(parent());
    if (item)
        item->update();
}
