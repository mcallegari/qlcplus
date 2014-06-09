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
    explicit ChannelModifierEditor(Doc* doc, QWidget *parent = 0);
    ~ChannelModifierEditor();

private:
    Doc* m_doc;
    ChannelModifierGraphicsView *m_view;

protected:
    void updateModifiersList();

protected slots:
    void slotViewClicked();
    void slotHandlerClicked(uchar pos, uchar value);
    void slotItemDMXChanged(uchar pos, uchar value);

    void slotItemSelectionChanged();

    void slotOriginalDMXValueChanged(int value);
    void slotModifiedDMXValueChanged(int value);
    void slotAddHandlerClicked();
    void slotRemoveHandlerClicked();
    void slotSaveClicked();

private:
    ChannelModifier *m_currentTemplate;
};

#endif // CHANNELMODIFIEREDITOR_H
