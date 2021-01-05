/*
  Q Light Controller Plus
  functionliveeditdialog.h

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

#ifndef FUNCTIONLIVEEDITDIALOG_H
#define FUNCTIONLIVEEDITDIALOG_H

#include <QScrollArea>
#include <QDialog>

class Doc;

/** @addtogroup ui_functions
 * @{
 */

class FunctionLiveEditDialog : public QDialog
{
    Q_OBJECT
public:
    FunctionLiveEditDialog(Doc *doc, quint32 fid, QWidget *parent = 0);

    ~FunctionLiveEditDialog();

private:
    Doc *m_doc;
    QWidget* m_editor;

protected:
    QScrollArea* m_scrollArea;

signals:

public slots:

};

/** @} */

#endif // FUNCTIONLIVEEDITDIALOG_H
