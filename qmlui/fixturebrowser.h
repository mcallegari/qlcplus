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
#include <QDebug>

class QLCFixtureMode;
class QLCFixtureDef;
class TreeModel;
class Doc;

class FixtureBrowser : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString searchString READ searchString WRITE setSearchString NOTIFY searchStringChanged)
    Q_PROPERTY(QVariant searchTreeModel READ searchTreeModel NOTIFY searchListChanged)
    Q_PROPERTY(QVariant modeChannelList READ modeChannelList NOTIFY modeChannelListChanged)

public:
    FixtureBrowser(QQuickView *view, Doc *doc, QObject *parent = 0);

    Q_INVOKABLE QStringList manufacturers();
    Q_INVOKABLE int genericIndex();
    Q_INVOKABLE QStringList models(QString manufacturer);
    Q_INVOKABLE QStringList modes(QString manufacturer, QString model);
    Q_INVOKABLE int modeChannels(QString modeName);

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
    QString searchString() const;
    void setSearchString(QString searchString);

    QVariant searchTreeModel() const;

    QVariant modeChannelList() const;

signals:
    void modeChanged();
    void modeChannelsChanged();
    void searchStringChanged(QString searchString);
    void searchListChanged();
    void modeChannelListChanged();

private:
    void updateSearchTree();

private:
    Doc *m_doc;
    QQuickView *m_view;
    /** Reference of the currently selected fixture definition */
    QLCFixtureDef *m_definition;
    /** Reference of the currently selected fixture mode */
    QLCFixtureMode *m_mode;
    /** Reference to the tree model used for searches */
    TreeModel *m_searchTree;
    QString m_searchString;
};

#endif // FIXTUREBROWSER_H
