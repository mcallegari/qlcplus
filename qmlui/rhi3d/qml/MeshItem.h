/*
  Q Light Controller Plus
  MeshItem.h

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

#pragma once

#include <QtCore/QObject>
#include <QtGui/QVector3D>

class MeshItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(quint32 itemID READ itemID WRITE setItemID NOTIFY itemIDChanged)
    Q_PROPERTY(QVector3D position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(QVector3D rotationDegrees READ rotationDegrees WRITE setRotationDegrees NOTIFY rotationDegreesChanged)
    Q_PROPERTY(QVector3D scale READ scale WRITE setScale NOTIFY scaleChanged)
    Q_PROPERTY(bool isSelected READ isSelected WRITE setIsSelected NOTIFY isSelectedChanged)
    Q_PROPERTY(bool selectable READ selectable WRITE setSelectable NOTIFY selectableChanged)
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)

public:
    enum class MeshType
    {
        Model,
        StaticLight,
        MovingHead,
        Cube,
        Sphere,
        PixelBar,
        BeamBar,
        Video
    };
    Q_ENUM(MeshType)

    explicit MeshItem(QObject *parent = nullptr);

    virtual MeshType type() const = 0;

    quint32 itemID() const { return m_itemID; }
    void setItemID(quint32 itemID);

    QVector3D position() const { return m_position; }
    void setPosition(const QVector3D &position);

    QVector3D rotationDegrees() const { return m_rotationDegrees; }
    void setRotationDegrees(const QVector3D &rotation);

    QVector3D scale() const { return m_scale; }
    void setScale(const QVector3D &scale);

    bool isSelected() const { return m_isSelected; }
    void setIsSelected(bool selected);

    bool selectable() const { return m_selectable; }
    void setSelectable(bool selectable);

    bool visible() const { return m_visible; }
    void setVisible(bool visible);

Q_SIGNALS:
    void itemIDChanged();
    void positionChanged();
    void rotationDegreesChanged();
    void scaleChanged();
    void isSelectedChanged();
    void selectableChanged();
    void visibleChanged();

protected:
    void notifyParent();

    quint32 m_itemID = 0;
    QVector3D m_position = QVector3D(0.0f, 0.0f, 0.0f);
    QVector3D m_rotationDegrees = QVector3D(0.0f, 0.0f, 0.0f);
    QVector3D m_scale = QVector3D(1.0f, 1.0f, 1.0f);
    bool m_isSelected = false;
    bool m_selectable = true;
    bool m_visible = true;
};
