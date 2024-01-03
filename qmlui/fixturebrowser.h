/*
  Q Light Controller Plus
  fixturebrowser.h

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

#ifndef FIXTUREBROWSER_H
#define FIXTUREBROWSER_H

#include <QQuickView>

class QLCFixtureMode;
class QLCFixtureDef;
class TreeModel;
class Doc;

class FixtureBrowser : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QStringList manufacturers READ manufacturers CONSTANT)
    Q_PROPERTY(int manufacturerIndex READ manufacturerIndex WRITE setManufacturerIndex NOTIFY manufacturerIndexChanged)
    Q_PROPERTY(QString selectedManufacturer READ selectedManufacturer WRITE setSelectedManufacturer NOTIFY selectedManufacturerChanged)

    Q_PROPERTY(QStringList modelsList READ modelsList NOTIFY modelsListChanged)
    Q_PROPERTY(QString selectedModel READ selectedModel WRITE setSelectedModel NOTIFY selectedModelChanged)

    Q_PROPERTY(QStringList modesList READ modesList NOTIFY modesListChanged)
    Q_PROPERTY(QString selectedMode READ selectedMode WRITE setSelectedMode NOTIFY selectedModeChanged)
    Q_PROPERTY(int modeChannelsCount READ modeChannelsCount WRITE setModeChannelsCount NOTIFY modeChannelsCountChanged)

    Q_PROPERTY(QString searchFilter READ searchFilter WRITE setSearchFilter NOTIFY searchFilterChanged)
    Q_PROPERTY(QVariant searchTreeModel READ searchTreeModel NOTIFY searchListChanged)
    Q_PROPERTY(QVariant modeChannelList READ modeChannelList NOTIFY modeChannelListChanged)

    Q_PROPERTY(QString fixtureName READ fixtureName WRITE setFixtureName NOTIFY fixtureNameChanged)

public:
    FixtureBrowser(QQuickView *view, Doc *doc, QObject *parent = 0);
    ~FixtureBrowser();

    QStringList manufacturers();

    int manufacturerIndex() const;
    void setManufacturerIndex(int index);

    QString selectedManufacturer() const;
    void setSelectedManufacturer(QString selectedManufacturer);

    QStringList modelsList();

    Q_INVOKABLE bool isUserDefinition(QString manufacturer, QString model);

    QString selectedModel() const;
    void setSelectedModel(QString selectedModel);

    /** Get/Set the name that will be used upon fixtures creation */
    QString fixtureName() const;
    void setFixtureName(QString fixtureName);

    QStringList modesList();

    QString selectedMode() const;
    void setSelectedMode(QString selectedMode);

    int modeChannelsCount();
    void setModeChannelsCount(int modeChannelsCount);
    QVariant modeChannelList() const;

    /** Check if the group of fixtures with the specified $uniIdx, $channels, $quantity and $gap
     *  can be created in the $requested DMX address.
     *  Returns:
     *  > $requested if the $requested address is available
     *  > the first available address if $requested is not available
     *  > -1 in case all the checks have failed
     */
    Q_INVOKABLE int availableChannel(quint32 uniIdx, int channels, int quantity, int gap, int requested);

    /** Check if a Fixture with $fixtureID can be moved to the $requested DMX address */
    Q_INVOKABLE int availableChannel(quint32 fixtureID, int requested);

    /** Get/Set the fixture search filter */
    QString searchFilter() const;
    void setSearchFilter(QString searchFilter);

    QVariant searchTreeModel() const;

signals:
    void manufacturerIndexChanged(int manufacturerIndex);
    void selectedManufacturerChanged(QString selectedManufacturer);

    void modelsListChanged();
    void selectedModelChanged(QString selectedModel);

    void modesListChanged();
    void selectedModeChanged(QString selectedMode);

    void modeChannelsCountChanged();
    void modeChannelListChanged();

    void searchFilterChanged(QString searchFilter);
    void searchListChanged();

    void fixtureNameChanged(QString fixtureName);

private:
    void updateSearchTree();
    QLCFixtureDef *fixtureDefinition();

private:
    Doc *m_doc;
    QQuickView *m_view;

    /** Cache of the organized definitions for browsing */
    QMap<QString, QMap<QString, bool>> m_defCache;
    /** The index of the currently selected manufacturer */
    int m_manufacturerIndex;
    /** The currently selected manufacturer as string */
    QString m_selectedManufacturer;
    /** The currently selected fixture model as string */
    QString m_selectedModel;
    /** The name used for fixtures creation */
    QString m_fixtureName;
    /** The currently selected fixture mode as string */
    QString m_selectedMode;
    /** The currently selected mode channels number.
     *  If no mode is available this can be defined by the user */
    int m_modeChannelsCount;
    /** Reference of the currently selected fixture mode */
    QLCFixtureMode *m_mode;
    /** Reference to the tree model used for searches */
    TreeModel *m_searchTree;
    /** A string holding the search keyword */
    QString m_searchFilter;
};

#endif // FIXTUREBROWSER_H
