/*
  Q Light Controller Plus
  chasereditor.h

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

#ifndef CHASEREDITOR_H
#define CHASEREDITOR_H

#include "functioneditor.h"

class Chaser;

class ChaserEditor : public FunctionEditor
{
    Q_OBJECT

    Q_PROPERTY(QString chaserName READ chaserName WRITE setChaserName NOTIFY chaserNameChanged)

    Q_PROPERTY(QVariant stepsList READ stepsList NOTIFY stepsListChanged)

public:
    ChaserEditor(QQuickView *view, Doc *doc, QObject *parent = 0);

    /** Set the ID of the Function being edit */
    void setFunctionID(quint32 ID);

    QVariant stepsList() const;

    /** Return the name of the currently edited Chaser */
    QString chaserName() const;

    /** Set the name of the currently edited Chaser */
    void setChaserName(QString chaserName);

signals:
    void chaserNameChanged(QString chaserName);
    void stepsListChanged();

private:
    /** Reference of the Chaser currently being edited */
    Chaser *m_chaser;
};

#endif // CHASEREDITOR_H

