/*
  Q Light Controller Plus
  channelmodifiereditor.h

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

#ifndef CHANNELMODIFIEREDITOR_H
#define CHANNELMODIFIEREDITOR_H

#include <QGraphicsView>
#include <QDialog>

#include "ui_channelmodifiereditor.h"

class Doc;
class ChannelModifier;
class ChannelModifierGraphicsView;

class ChannelModifierEditor : public QDialog, public Ui_ChannelModifierEditor
{
    Q_OBJECT

public:
    explicit ChannelModifierEditor(Doc* doc, QString modifier, QWidget *parent = 0);
    ~ChannelModifierEditor();

    ChannelModifier *selectedModifier();

private:
    Doc* m_doc;
    ChannelModifierGraphicsView *m_view;

protected:
    /**
     * Update the modifier tree view and select
     * the given $modifier if present
     */
    void updateModifiersList(QString modifier);

protected slots:
    /** Slot called when the user clicks on the graphics view space */
    void slotViewClicked();

    /** Slot called when the user clicks on one handler of the modifier */
    void slotHandlerClicked(uchar pos, uchar value);

    /** Slot called while the user is dragging a handler around */
    void slotItemDMXChanged(uchar pos, uchar value);

    /** Slot called when a tree item is selected */
    void slotItemSelectionChanged();

    void slotOriginalDMXValueChanged(int value);
    void slotModifiedDMXValueChanged(int value);
    void slotAddHandlerClicked();
    void slotRemoveHandlerClicked();
    void slotSaveClicked();

    void slotUnsetClicked();

private:
    ChannelModifier *m_currentTemplate;
};

#endif // CHANNELMODIFIEREDITOR_H
