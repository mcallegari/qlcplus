/*
  Q Light Controller
  fixtureselection.h

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

#ifndef FIXTURESELECTION_H
#define FIXTURESELECTION_H

#include <QDialog>
#include <QList>

#include "ui_fixtureselection.h"

class QTreeWidgetItem;
class GroupHead;
class QWidget;
class Doc;

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

    /************************************************************************
     * Selected fixtures
     ************************************************************************/
public:
    /** List of selected fixtures (valid only in Fixture Selection Mode) */
    QList <quint32> selection() const;

    /** Get a list of selected fixture heads (valid only in Head Selection Mode) */
    QList <GroupHead> selectedHeads() const;

private:
    QList <quint32> m_selection;
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
     * Disabled fixtures
     ************************************************************************/
public:
    /** Disable (==prevent selection of) a list of fixtures */
    void setDisabledFixtures(const QList <quint32>& disabled);

    void setDisabledHeads(const QList <GroupHead>& disabled);

private:
    QList <quint32> m_disabledFixtures;
    QList <GroupHead> m_disabledHeads;

    /************************************************************************
     * Tree
     ************************************************************************/
private:
    /** Fill the tree */
    void fillTree();

private slots:
    /** Item double clicks */
    void slotItemDoubleClicked();

    /** Tree selection changes */
    void slotSelectionChanged();

    /** OK button click */
    void accept();
};

#endif
