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

    QVariant groupsTreeModel();
    QVariant functionsTreeModel();

    /** Get/Set a string to filter Group/Fixture/Channel names */
    QString fixtureSearchFilter() const;
    void setFixtureSearchFilter(QString searchFilter);

    /** Get/Set a string to filter Group/Fixture/Channel names */
    QString functionSearchFilter() const;
    void setFunctionSearchFilter(QString searchFilter);

private:
    /**
     * Load workspace contents from the given XML document.
     *
     * @param doc The XML document to load from.
     */
    bool loadXML(QXmlStreamReader &doc);

    /** Update a tree suitable to be displayed by the UI */
    void updateFunctionsTree();

    /** Method called recursively to check/uncheck tree sub-items */
    void setChildrenChecked(TreeModel *tree, bool checked);

    void getAvailableFixtureAddress(int channels, int &universe, int &address);

    /** Perform the actual import of Fixtures */
    void importFixtures();

    /** Perform the actual import of Functions */
    void importFunctions();

protected slots:
    void slotFixtureTreeDataChanged(TreeModelItem *item, int role, const QVariant &value);
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
    /** Reference to the QML view root */
    QQuickView *m_view;
    /** Reference to the project workspace */
    Doc *m_doc;
    /** Reference to the project where to import from */
    Doc *m_importDoc;

    /** Data model used by the QML UI to represent groups/fixtures/channels */
    TreeModel *m_fixtureTree;
    /** A string to filter the displayed fixture tree items */
    QString m_fixtureSearchFilter;
    /** The list of selected Fixture IDs */
    QList<quint32> m_fixtureIDList;
    /** A map of the Fixture IDs that need to be remapped */
    QMap<quint32, quint32> m_fixtureIDRemap;

    /** Data model used by the QML UI to represent groups/fixtures/channels */
    TreeModel *m_functionTree;
    /** A string to filter the displayed fixture tree items */
    QString m_functionSearchFilter;
    /** The list of selected Function IDs */
    QList<quint32> m_functionIDList;
    /** A map of the Function IDs that need to be remapped */
    QMap<quint32, quint32> m_functionIDRemap;
};
#endif /* IMPORTMANAGER_H */
