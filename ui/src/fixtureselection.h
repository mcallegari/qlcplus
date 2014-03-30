/*
  Q Light Controller
  fixtureselection.h

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

#ifndef FIXTURESELECTION_H
#define FIXTURESELECTION_H

#include <QDialog>
#include <QList>

#include "ui_fixtureselection.h"

class FixtureTreeWidget;
class QTreeWidgetItem;
class GroupHead;
class QWidget;
class Doc;

/** @addtogroup ui UI
 * @{
 */

class FixtureSelection : public QDialog, public Ui_FixtureSelection
{
    Q_OBJECT
    Q_DISABLE_COPY(FixtureSelection)

public:
    FixtureSelection(QWidget* parent, Doc* doc);
    ~FixtureSelection();

public slots:
    /** @reimp */
    int exec();

private:
    Doc* m_doc;
    FixtureTreeWidget *m_tree;
    quint32 m_treeFlags;

    /************************************************************************
     * Selected fixtures
     ************************************************************************/
public:
    /** List of selected fixtures (valid only in Fixture Selection Mode) */
    QList <quint32> selection() const;

    /** Get a list of selected fixture heads (valid only in Head Selection Mode) */
    QList <GroupHead> selectedHeads() const;

private:
    QList <quint32> m_selectedFixtures;
    QList <GroupHead> m_selectedHeads;

    /************************************************************************
     * Multi-selection
     ************************************************************************/
public:
     /** Enable or disable multi-selection */
    void setMultiSelection(bool multi);

    /************************************************************************
     * Selection mode
     ************************************************************************/
public:
    enum SelectionMode { Fixtures, Heads };
    void setSelectionMode(SelectionMode mode);

private:
    SelectionMode m_selectionMode;

    /************************************************************************
     * Disabled items
     ************************************************************************/
public:
    /** Disable (==prevent selection of) a list of fixtures */
    void setDisabledFixtures(const QList <quint32>& disabled);

    /** Disable (==prevent selection of) a list of heads */
    void setDisabledHeads(const QList <GroupHead>& disabled);

    /************************************************************************
     * Tree
     ************************************************************************/

private slots:
    /** Item double clicks */
    void slotItemDoubleClicked();

    /** Tree selection changes */
    void slotSelectionChanged();

    /** OK button click */
    void accept();
};

/** @} */

#endif
