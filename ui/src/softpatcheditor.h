/*
  Q Light Controller Plus
  channelsselection.h

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

#ifndef SOFTPATCHEDITOR_H
#define SOFTPATCHEDITOR_H

#include <QDialog>

#include "ui_softpatcheditor.h"
#include "scenevalue.h"

class Doc;

class SoftpatchEditor: public QDialog, public Ui_SoftpatchEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(SoftpatchEditor)

public:
    SoftpatchEditor(Doc* doc, QWidget *parent = 0);
    ~SoftpatchEditor();

private:
    Doc* m_doc;

protected:
    void updateFixturesTree();

protected slots:
    /** Slot called when Test Button is pressed / released */
    void slotTestButtonPressed();

    /** Callback for OK button clicks */
    void accept();
};

#endif // SOFTPATCHEDITOR_H
