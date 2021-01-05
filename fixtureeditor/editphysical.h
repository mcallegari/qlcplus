/*
  Q Light Controller Plus - Fixture Editor
  editphysical.h

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

#ifndef EDITPHYSICAL_H
#define EDITPHYSICAL_H

#include <QWidget>

#include "ui_editphysical.h"
#include "qlcphysical.h"

class EditPhysical : public QWidget, public Ui_EditPhysical
{
    Q_OBJECT

public:
    explicit EditPhysical(QLCPhysical physical, QWidget *parent = 0);
    ~EditPhysical();

    void pasteFromClipboard(QLCPhysical clipboard);

    QLCPhysical physical();

signals:
    void copyToClipboard(QLCPhysical physical);
    void requestPasteFromClipboard();

private slots:
    void slotCopyToClipboard();
    void slotPasteFromClipboard();

protected:
    QLCPhysical m_physical;
};

#endif // EDITPHYSICAL_H
