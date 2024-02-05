/*
  Q Light Controller Plus
  importmanager.h

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

#ifndef IMPORTMANAGER_H
#define IMPORTMANAGER_H

#include <QQuickView>

#include "treemodel.h"

class QXmlStreamReader;
class Doc;

class ImportManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariant groupsTreeModel READ groupsTreeModel NOTIFY groupsTreeModelChanged)
    Q_PROPERTY(QVariant functionsTreeModel READ functionsTreeModel NOTIFY functionsTreeModelChanged)
    Q_PROPERTY(QString fixtureSearchFilter READ fixtureSearchFilter WRITE setFixtureSearchFilter NOTIFY fixtureSearchFilterChanged)
    Q_PROPERTY(QString functionSearchFilter READ functionSearchFilter WRITE setFunctionSearchFilter NOTIFY functionSearchFilterChanged)

public:
    ImportManager(QQuickView *view, Doc *doc, QObject *parent = 0);
    ~ImportManager();

    bool loadWorkspace(const QString &fileName);

    void apply();

private:
    /**
     * Load workspace contents from the given XML document.
     *
     * @param doc The XML document to load from.
     */
    bool loadXML(QXmlStreamReader &doc);

    /** Get the first available fixture address with a space of $channels positions.
     *  This sets $universe and $address accordingly */
    void getAvailableFixtureAddress(int channels, int &universe, int &address);

    /** Perform the actual import of Fixtures */
    void importFixtures();

    /** Perform the actual import of Palettes */
    void importPalettes();

    /** Recursive method that imports a Function ID
     *  satisfying the Function dependecies first */
    void importFunctionID(quint32 funcID);

    /** Method called recursively to check/uncheck all the sub-items of a tree */
    void setChildrenChecked(TreeModel *tree, bool checked);

private:
    /** Reference to the QML view root */
    QQuickView *m_view;
    /** Reference to the project workspace */
    Doc *m_doc;
    /** Reference to the project where to import from */
    Doc *m_importDoc;

    /** The list of selected Palette IDs */
    QList<quint32> m_paletteIDList;
    /** A map of the Palette IDs that need to be remapped */
    QMap<quint32, quint32> m_paletteIDRemap;

    /*********************************************************************
     * Fixture tree
     *********************************************************************/
public:
    QVariant groupsTreeModel();

    /** Get/Set a string to filter Group/Fixture/Channel names */
    QString fixtureSearchFilter() const;
    void setFixtureSearchFilter(QString searchFilter);

    /** Method called recursively to update Fixture items checked state */
    void checkFixtureTree(TreeModel *tree);

protected slots:
    void slotFixtureTreeDataChanged(TreeModelItem *item, int role, const QVariant &value);

private:
    /** Data model used by the QML UI to represent groups/fixtures/channels */
    TreeModel *m_fixtureTree;
    /** Flag to filter signals when updating checked states */
    bool m_fixtureTreeUpdating;
    /** A string to filter the displayed fixture tree items */
    QString m_fixtureSearchFilter;
    /** The list of selected Fixture IDs */
    QList<quint32> m_fixtureIDList;
    /** A list of item IDs holding basically linked fixtures */
    QList<quint32> m_itemIDList;
    /** A map of the Fixture IDs that need to be remapped */
    QMap<quint32, quint32> m_fixtureIDRemap;

    /** The list of selected Fixture group IDs */
    QList<quint32> m_fixtureGroupIDList;
    /** A map of the Fixture group IDs that need to be remapped */
    QMap<quint32, quint32> m_fixtureGroupIDRemap;

    /*********************************************************************
     * Function tree
     *********************************************************************/
public:
    QVariant functionsTreeModel();

    /** Get/Set a string to filter Group/Fixture/Channel names */
    QString functionSearchFilter() const;
    void setFunctionSearchFilter(QString searchFilter);

private:
    /** Update a tree suitable to be displayed by the UI */
    void updateFunctionsTree();

    /** Method called recursively to create a map of ID / TreeModelItems */
    void checkFunctionTree(TreeModel *tree);

    /** Method called recursively to check all the Functions needed
     *  by the Function with the provided $id */
    void checkFunctionDependency(quint32 fid);

protected slots:
    void slotFunctionTreeDataChanged(TreeModelItem *item, int role, const QVariant &value);

signals:
    /** Notify the listeners that the fixture tree model has changed */
    void groupsTreeModelChanged();
    /** Notify the listeners that the fixture search filter has changed */
    void fixtureSearchFilterChanged();

    /** Notify the listeners that the function tree model has changed */
    void functionsTreeModelChanged();
    /** Notify the listeners that the function search filter has changed */
    void functionSearchFilterChanged();

private:
    /** Data model used by the QML UI to represent groups/fixtures/channels */
    TreeModel *m_functionTree;
    /** Flag to filter signals when updating checked states */
    bool m_functionTreeUpdating;
    /** A string to filter the displayed fixture tree items */
    QString m_functionSearchFilter;
    /** The list of selected Function IDs */
    QList<quint32> m_functionIDList;
    /** A map of the Function IDs that need to be remapped */
    QMap<quint32, quint32> m_functionIDRemap;
};
#endif /* IMPORTMANAGER_H */
