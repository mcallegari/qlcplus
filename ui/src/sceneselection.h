/*
  Q Light Controller
  sceneselection.h

  Copyright (c) Massimo Callegari

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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

#endif
