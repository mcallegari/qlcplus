/*
  Q Light Controller
  functionselection.h

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

#ifndef FUNCTIONSELECTION_H
#define FUNCTIONSELECTION_H

#include <QDialog>
#include <QList>

#include "ui_functionselection.h"
#include "function.h"

class FunctionsTreeWidget;
class QTreeWidgetItem;
class MasterTimer;
class QToolBar;
class QAction;
class QWidget;
class Fixture;
class Doc;

/** @addtogroup ui UI
 * @{
 */

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
    FunctionsTreeWidget *m_funcTree;
    bool m_isInitializing;

    /*********************************************************************
     * None entry
     *********************************************************************/
public:
    void showNone(bool show = false);

private:
    bool m_none;
    QTreeWidgetItem *m_noneItem;

    /*********************************************************************
     * New track entry
     *********************************************************************/
public:
    void showNewTrack(bool show = false);

private:
    bool m_newTrack;
    QTreeWidgetItem *m_newTrackItem;

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
     * Functions filter
     *********************************************************************/

protected slots:
    void slotAllFunctionsChecked();
    void slotRunningFunctionsChecked();

private:
    bool m_runningOnlyFlag;

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

    /**
     * Set a list of filters to be disabled
     *
     * @param filter An OR'ed combination of Function::Type to show
     */
    void disableFilters(int types);

protected slots:
    void slotCollectionChecked(bool state);
    void slotEFXChecked(bool state);
    void slotChaserChecked(bool state);
    void slotSequenceChecked(bool state);
    void slotSceneChecked(bool state);
    void slotScriptChecked(bool state);
    void slotRGBMatrixChecked(bool state);
    void slotShowChecked(bool state);
    void slotAudioChecked(bool state);
    void slotVideoChecked(bool state);

private:
    int m_filter;
    int m_disableFilters;
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
    /** Set current selection of function IDs in the tree */
    void setSelection(QList<quint32> selection);

    /** Get a list of selected function IDs */
    const QList <quint32> selection() const;

protected:
    /** The list of selected function IDs */
    QList <quint32> m_selection;

    /*********************************************************************
     * Internal
     *********************************************************************/
protected:

    /** Clear & (re)fill the tree */
    void refillTree();

protected slots:
    void slotItemSelectionChanged();
    void slotItemDoubleClicked(QTreeWidgetItem* item);

};

/** @} */

#endif
