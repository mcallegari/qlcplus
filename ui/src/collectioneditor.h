/*
  Q Light Controller
  collectioneditor.h

  Copyright (c) Heikki Junnila

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

#ifndef COLLECTIONEDITOR_H
#define COLLECTIONEDITOR_H

#include <QWidget>

#include "ui_collectioneditor.h"
#include "function.h"

class FunctionSelection;
class MasterTimer;
class Collection;
class Doc;

/** @addtogroup ui_functions
 * @{
 */

class CollectionEditor : public QWidget, public Ui_CollectionEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(CollectionEditor)

public:
    CollectionEditor(QWidget* parent, Collection* fc, Doc* doc);
    ~CollectionEditor();

private:
    Doc* m_doc;
    Collection* m_collection; // The Collection being edited

private slots:
    void slotNameEdited(const QString& text);
    void slotAdd();
    void slotRemove();
    void slotMoveUp();
    void slotMoveDown();
    void slotTestClicked();

private:
    FunctionParent functionParent() const;

private:
    void updateFunctionList();
};

/** @} */

#endif

