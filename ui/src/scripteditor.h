/*
  Q Light Controller
  scripteditor.h

  Copyright (C) Heikki Junnila

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

#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QWidget>
#include "ui_scripteditor.h"

class QTextDocument;
class MasterTimer;
class OutputMap;
class InputMap;
class QAction;
class Script;
class QMenu;
class Doc;

/** @addtogroup ui_functions
 * @{
 */

class ScriptEditor : public QWidget, public Ui_ScriptEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(ScriptEditor)

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    ScriptEditor(QWidget* parent, Script* script, Doc* doc);
    ~ScriptEditor();

private:
    QTextDocument* m_document;
    Script* m_script;
    Doc* m_doc;
    QString m_lastUsedPath;

    /************************************************************************
     * Add menu
     ************************************************************************/
private:
    void initAddMenu();
    QString getFilePath();

private slots:
    void slotNameEdited(const QString& text);
    void slotContentsChanged();
    void slotFunctionStopped(quint32 id);

    void slotAddStartFunction();
    void slotAddStopFunction();
    void slotAddWait();
    void slotAddWaitKey();
    void slotAddSetHtp();
    void slotAddSetLtp();
    void slotAddSetFixture();
    void slotAddSystemCommand();
    void slotAddComment();
    void slotAddRandom();
    void slotAddFilePath();
    void slotCheckSyntax();

private:
    QAction* m_addStartFunctionAction;
    QAction* m_addStopFunctionAction;
    QAction* m_addWaitAction;
    QAction* m_addWaitKeyAction;
    QAction* m_addSetHtpAction;
    QAction* m_addSetLtpAction;
    QAction* m_addSetFixtureAction;
    QAction* m_addSystemCommandAction;
    QAction* m_addCommentAction;
    QAction* m_addRandomAction;
    QAction* m_addFilePathAction;
    QMenu* m_addMenu;

    /************************************************************************
     * Test execution
     ************************************************************************/
protected slots:
    void slotTestRun();
};

/** @} */

#endif
