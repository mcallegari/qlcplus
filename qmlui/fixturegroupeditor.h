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

#include <qlcpoint.h>

class Doc;
class Fixture;
class FixtureGroup;
class FixtureManager;

class FixtureGroupEditor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariant groupsListModel READ groupsListModel NOTIFY groupsListModelChanged)
    Q_PROPERTY(quint32 groupID READ groupID CONSTANT)
    Q_PROPERTY(QString groupName READ groupName WRITE setGroupName NOTIFY groupNameChanged)
    Q_PROPERTY(QSize groupSize READ groupSize WRITE setGroupSize NOTIFY groupSizeChanged)
    Q_PROPERTY(QVariantList groupMap READ groupMap NOTIFY groupMapChanged)
    Q_PROPERTY(QVariantList groupLabels READ groupLabels NOTIFY groupLabelsChanged)
    Q_PROPERTY(QVariantList selectionData READ selectionData NOTIFY selectionDataChanged)

public:
    FixtureGroupEditor(QQuickView *view, Doc *doc, FixtureManager *fxMgr, QObject *parent = 0);
    ~FixtureGroupEditor();

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
    /** Reference to the Fixture Manager */
    FixtureManager *m_fixtureManager;
    /** Reference to the Fixture Group currently being edited */
    FixtureGroup *m_editGroup;

    /*********************************************************************
     * Fixture Group Grid Editing
     *********************************************************************/
public:
    enum TransformType
    {
        Rotate90,
        Rotate180,
        Rotate270,
        HorizontalFlip,
        VerticalFlip
    };
    Q_ENUM(TransformType)

    /** Set the reference of a FixtureGroup for editing */
    Q_INVOKABLE void setEditGroup(QVariant reference);

    quint32 groupID() const;

    /** Get/Set the name of the Fixture Group currently being edited */
    QString groupName() const;
    void setGroupName(QString name);

    /** Get/Set the size of the Fixture Group currently being edited */
    QSize groupSize() const;
    void setGroupSize(QSize size);

    /** Returns the heads data for representation in a QML GridEditor */
    QVariantList groupMap();

    /** Returns the head labels data for representation in a QML GridEditor */
    QVariantList groupLabels();

    /** Returns a list of indices with the selected heads */
    QVariantList selectionData();

    /** Resets the currently selected items */
    Q_INVOKABLE void resetSelection();

    /** Check the head at the provided $x,$y position and
     *  returns a list of indices with the selected heads */
    Q_INVOKABLE QVariantList groupSelection(int x, int y, int mouseMods);

    /** Returns a selection array from the provided $reference */
    Q_INVOKABLE QVariantList fixtureSelection(QVariant reference, int x, int y, int mouseMods);

    /** Returns a selection array from the provided $itemID and $headIndex */
    Q_INVOKABLE QVariantList headSelection(int x, int y, int mouseMods);

    /** Add a Fixture with the provided $reference to x,y position */
    Q_INVOKABLE bool addFixture(QVariant reference, int x, int y);

    /** Add a Fixture head of the provided $itemID and $headIndex to x,y position */
    Q_INVOKABLE bool addHead(quint32 itemID, int headIndex, int x, int y);

    /** Check if the current selection can be moved by $offset cells */
    Q_INVOKABLE bool checkSelection(int x, int y, int offset);

    /** Move the current selection by $offset cells */
    Q_INVOKABLE void moveSelection(int x, int y, int offset);

    /** Delete the currently selected items */
    Q_INVOKABLE void deleteSelection();

    /** Rotate the current selection by $degrees */
    Q_INVOKABLE void transformSelection(int transformation);

    /** Get a string to be displayed as tooltip for a head at position x,y */
    Q_INVOKABLE QString getTooltip(int x, int y);

private:
    void updateGroupMap();
    QLCPoint pointFromAbsolute(int absoluteIndex);

signals:
    void groupSizeChanged();
    void groupNameChanged();
    void groupMapChanged();
    void groupLabelsChanged();
    void selectionDataChanged();

private:
    /** An array-like map of the heads data in  group */
    QVariantList m_groupMap;
    /** An array-like map of the heads labels in  group */
    QVariantList m_groupLabels;
    /** An array of data representing the currently selected items on a Grid editor */
    QVariantList m_groupSelection;
};

#endif // FIXTUREGROUPEDITOR_H
