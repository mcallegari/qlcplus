/*
  Q Light Controller Plus
  fixturegroupeditor.h

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

#ifndef FIXTUREGROUPEDITOR_H
#define FIXTUREGROUPEDITOR_H

#include <QQuickView>
#include <QObject>

class Doc;
class Fixture;
class FixtureGroup;

class FixtureGroupEditor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariant groupsListModel READ groupsListModel NOTIFY groupsListModelChanged)
    Q_PROPERTY(QString groupName READ groupName NOTIFY groupNameChanged)
    Q_PROPERTY(QSize groupSize READ groupSize WRITE setGroupSize NOTIFY groupSizeChanged)
    Q_PROPERTY(QVariantList groupMap READ groupMap NOTIFY groupMapChanged)

public:
    FixtureGroupEditor(QQuickView *view, Doc *doc, QObject *parent = 0);

    /** Returns the data model to display a list of FixtureGroups with icons */
    QVariant groupsListModel();

    /** Empty the Fixture Group currently being edited */
    Q_INVOKABLE void resetGroup();

public slots:
    /** Slot called whenever a new workspace has been loaded */
    void slotDocLoaded();

signals:
    /** Notify the listeners that the FixtureGroup list model has changed */
    void groupsListModelChanged();

private:
    /** Reference to the QML view root */
    QQuickView *m_view;
    /** Reference to the project workspace */
    Doc *m_doc;
    /** Reference to the Fixture Group currently being edited */
    FixtureGroup *m_editGroup;

    /*********************************************************************
     * Fixture Group Grid Editing
     *********************************************************************/
public:
    /** Set the reference of a FixtureGroup for editing */
    Q_INVOKABLE void setEditGroup(QVariant reference);

    /** Get the name of the Fixture Group currently being edited */
    QString groupName() const;

    /** Get/Set the size of the Fixture Group currently being edited */
    QSize groupSize() const;
    void setGroupSize(QSize size);

    /** Returns data for representation in a GridEditor QML component */
    QVariantList groupMap();

    /** Returns a list of indices with the selected heads */
    Q_INVOKABLE QVariantList groupSelection(int x, int y, int mouseMods);

    /** Returns a selection array from the provide $reference */
    Q_INVOKABLE QVariantList dragSelection(QVariant reference, int x, int y, int mouseMods);

    Q_INVOKABLE void addFixture(QVariant reference, int x, int y);

    /** Check if the current selection can be moved by $offset cells */
    Q_INVOKABLE bool checkSelection(int x, int y, int offset);

    /** Move the current selection by $offset cells */
    Q_INVOKABLE void moveSelection(int x, int y, int offset);

private:
    void updateGroupMap();

signals:
    void groupSizeChanged();
    void groupNameChanged();
    void groupMapChanged();

private:
    /** An array-like map of the current fixtures, filtered by m_universeFilter */
    QVariantList m_groupMap;
    /** An array of data representing the currently selected items on a Grid editor */
    QVariantList m_groupSelection;
};

#endif // FIXTUREGROUPEDITOR_H
