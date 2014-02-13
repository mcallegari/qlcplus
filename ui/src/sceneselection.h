/*
  Q Light Controller
  sceneselection.h

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

#ifndef SCENESELECTION_H
#define SCENESELECTION_H

#include <QDialog>
#include <QList>

#include "ui_sceneselection.h"
#include "scene.h"

class QTreeWidgetItem;
class QToolBar;
class QAction;
class QWidget;
class Doc;

/** @addtogroup ui UI
 * @{
 */

class SceneSelection : public QDialog, public Ui_SceneSelection
{
    Q_OBJECT
    Q_DISABLE_COPY(SceneSelection)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /**
     * Constructor
     *
     * @param parent The parent widget for the dialog
     * @param doc The QLC engine instance
     */
    SceneSelection(QWidget* parent, Doc* doc);
    ~SceneSelection();

    /** @reimp */
    void accept();

    quint32 getSelectedID();

public slots:
    int exec();

private:
    Doc* m_doc;
    quint32 m_selectedID;


    /*********************************************************************
     * Disabled Scene IDs
     *********************************************************************/
public:
    /** Disable the given list of scene IDs in the tree */
    void setDisabledScenes(const QList <quint32>& ids);

    /** Get a list of disabled scene IDs */
    QList <quint32> disabledScenes() const;

protected:
    QList <quint32> m_disabledScenes;

    /*********************************************************************
     * Internal
     *********************************************************************/
protected:
    /** Update the contents of the given function to the tree item */
    void updateFunctionItem(QTreeWidgetItem* item, Function* function);

    /** Clear & (re)fill the tree */
    void refillTree();

protected slots:
    void slotItemSelectionChanged();
    void slotItemDoubleClicked(QTreeWidgetItem* item);

};

/** @} */

#endif
