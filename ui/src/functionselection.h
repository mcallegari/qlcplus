/*
  Q Light Controller
  functionselection.h

  Copyright (c) Heikki Junnila

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

#ifndef FUNCTIONSELECTION_H
#define FUNCTIONSELECTION_H

#include <QDialog>
#include <QList>

#include "ui_functionselection.h"
#include "function.h"

class QTreeWidgetItem;
class MasterTimer;
class OutputMap;
class InputMap;
class QToolBar;
class QAction;
class QWidget;
class Fixture;
class Doc;

#define SETTINGS_FILTER "functionselection/filter"

class FunctionSelection : public QDialog, public Ui_FunctionSelection
{
    Q_OBJECT
    Q_DISABLE_COPY(FunctionSelection)

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
    FunctionSelection(QWidget* parent, Doc* doc);
    ~FunctionSelection();

public slots:
    int exec();

private:
    Doc* m_doc;

    /*********************************************************************
     * Multi-selection
     *********************************************************************/
public:
    /**
     * Allow multiple selection.
     *
     * @param multi true for multiple selection, false for single selection
     */
    void setMultiSelection(bool multi = true);

private:
    bool m_multiSelection;

    /*********************************************************************
     * Type filter
     *********************************************************************/
public:
    /**
     * Set a filter to display only the given function types
     *
     * @param filter An OR'ed combination of Function::Type to show
     * @param constFilter true to disable user changes to filter settings
     */
    void setFilter(int types, bool constFilter = false);

private:
    int m_filter;
    bool m_constFilter;

    /*********************************************************************
     * Disabled functions
     *********************************************************************/
public:
    /** Disable the given list of function IDs in the tree */
    void setDisabledFunctions(const QList <quint32>& ids);

    /** Get a list of disabled functionIDs */
    QList <quint32> disabledFunctions() const;

protected:
    QList <quint32> m_disabledFunctions;

    /*********************************************************************
     * Selection
     *********************************************************************/
public:
    /** Get a list of selected function IDs */
    const QList <quint32> selection() const;

protected:
    /** The list of selected function IDs */
    QList <quint32> m_selection;

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

    void slotCollectionChecked(bool state);
    void slotEFXChecked(bool state);
    void slotChaserChecked(bool state);
    void slotSceneChecked(bool state);
    void slotScriptChecked(bool state);
    void slotRGBMatrixChecked(bool state);
    void slotShowChecked(bool state);
    void slotAudioChecked(bool state);
};

#endif
