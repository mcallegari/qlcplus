/*
  Q Light Controller
  showeditor.h

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

#ifndef SHOWEDITOR_H
#define SHOWEDITOR_H

#include "ui_showeditor.h"

class Show;
class Doc;

/** @addtogroup ui_shows
 * @{
 */

class ShowEditor : public QWidget, public Ui_ShowEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(ShowEditor)

public:
    ShowEditor(QWidget* parent, Show* fc, Doc* doc);
    ~ShowEditor();

private:
    Doc* m_doc;
    Show* m_show; // The Show being edited

private slots:
    void slotNameEdited(const QString& text);
    void slotAdd();
    void slotRemove();

private:
    void updateFunctionList();
};

/** @} */

#endif
