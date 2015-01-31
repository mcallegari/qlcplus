/*
  Q Light Controller
  functionmanager.h

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

#ifndef FUNCTIONMANAGER_H
#define FUNCTIONMANAGER_H

#include <QWidget>
#include <QList>

#include "function.h"
#include "doc.h"

class FunctionsTreeWidget;
class QTreeWidgetItem;
class QSplitter;
class QToolBar;
class QAction;
class Fixture;
class QMenu;
class Doc;

/** @addtogroup ui_functions
 * @{
 */

class FunctionManager : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(FunctionManager)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    FunctionManager(QWidget* parent, Doc* doc);
    ~FunctionManager();

    /** Get the singleton instance */
    static FunctionManager* instance();

signals:
    /** Emitted when the FunctionManager's tab is de/activated */
    void functionManagerActive(bool active);

protected:
    /** @reimp */
    void showEvent(QShowEvent* ev);

    /** @reimp */
    void hideEvent(QHideEvent* ev);

protected slots:
    void slotModeChanged();
    void slotDocClearing();
    void slotDocLoaded();
    void slotFunctionNameChanged(quint32 id);
    void slotFunctionAdded(quint32 id);

protected:
    static FunctionManager* s_instance;
    Doc* m_doc;

    /*********************************************************************
     * Function tree
     *********************************************************************/
public:

    /** Select the function with the given ID */
    void selectFunction(quint32 id);

private:
    /** Init the splitter view */
    void initSplitterView();

    /** Init function tree view */
    void initTree();

    /** Delete all currently selected functions */
    void deleteSelectedFunctions();

private slots:
    /** Function selection was changed */
    void slotTreeSelectionChanged();

    /** Right mouse button was clicked on function tree */
    void slotTreeContextMenuRequested();

private:
    QSplitter* m_hsplitter;
    QSplitter* m_vsplitter;
    FunctionsTreeWidget* m_tree;

    /*********************************************************************
     * Menus, toolbar & actions
     *********************************************************************/
protected:
    void initActions();
    void initToolbar();

protected slots:
    void slotAddScene();
    void slotAddChaser();
    void slotAddSequence();
    void slotAddCollection();
    void slotAddEFX();
    void slotAddRGBMatrix();
    void slotAddScript();
    void slotAddAudio();
    void slotAddVideo();
    void slotAddFolder();

    void slotSelectAutostartFunction();
    void slotWizard();

    void slotClone();
    void slotDelete();
    void slotSelectAll();

protected:
    void updateActionStatus();

protected:
    QToolBar* m_toolbar;

    QAction* m_addSceneAction;
    QAction* m_addChaserAction;
    QAction* m_addSequenceAction;
    QAction* m_addCollectionAction;
    QAction* m_addEFXAction;
    QAction* m_addRGBMatrixAction;
    QAction* m_addScriptAction;
    QAction* m_addAudioAction;
    QAction* m_addVideoAction;

    QAction* m_autostartAction;
    QAction* m_wizardAction;
    QAction* m_addFolderAction;
    QAction* m_cloneAction;
    QAction* m_deleteAction;
    QAction* m_selectAllAction;

    /*********************************************************************
     * Helpers
     *********************************************************************/
private:
    /** Create a copy of the given function */
    void copyFunction(quint32 fid);

    /** Open an editor for the given function */
    void editFunction(Function* function);

    /** Delete current editor. Can be synchronous. */
    void deleteCurrentEditor(bool async = true);

private:
    QWidget* m_editor;
    QWidget* m_scene_editor;
};

/** @} */

#endif
