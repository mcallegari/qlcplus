/*
  Q Light Controller Plus
  editorview.h

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

#ifndef EDITORVIEW_H
#define EDITORVIEW_H

#include <QQuickView>

class QLCFixtureDef;
class PhysicalEdit;
class ChannelEdit;

class EditorView : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString manufacturer READ manufacturer WRITE setManufacturer NOTIFY manufacturerChanged)
    Q_PROPERTY(QString model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(QString author READ author WRITE setAuthor NOTIFY authorChanged)

    Q_PROPERTY(PhysicalEdit *globalPhysical READ globalPhysical CONSTANT)
    Q_PROPERTY(QVariantList channels READ channels NOTIFY channelsChanged)

public:
    EditorView(QQuickView *view, QLCFixtureDef *fixtureDef, QObject *parent = nullptr);
    ~EditorView();

    /** Get/Set the fixture manufacturer */
    QString manufacturer() const;
    void setManufacturer(QString manufacturer);

    /** Get/Set the fixture model */
    QString model() const;
    void setModel(QString model);

    /** Get/Set the definition author */
    QString author() const;
    void setAuthor(QString author);

    PhysicalEdit *globalPhysical();

signals:
    void manufacturerChanged(QString manufacturer);
    void modelChanged(QString model);
    void authorChanged(QString author);

private:
    /** Reference to the QML view root */
    QQuickView *m_view;
    /** Reference to the definition being edited */
    QLCFixtureDef *m_fixtureDef;
    /** Reference to the global physical properties */
    PhysicalEdit *m_globalPhy;

    /************************************************************************
     * Channels
     ************************************************************************/
public:
    /** Get a list of all the available channels in the definition */
    QVariantList channels() const;

    /** Request a channel editor. If chName is empty,
     *  a new channel is added */
    Q_INVOKABLE ChannelEdit *requestChannelEditor(QString chName);

private:
    ChannelEdit *m_channelEdit;

signals:
    void channelsChanged();
};

#endif // EDITORVIEW_H
