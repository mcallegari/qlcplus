/*
  Q Light Controller Plus
  modeedit.h

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

#ifndef MODEEDIT_H
#define MODEEDIT_H

#include <QQuickView>

#include "physicaledit.h"

class QLCFixtureMode;
class QLCChannel;
class ListModel;

class ModeEdit : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QVariant channels READ channels NOTIFY channelsChanged)
    Q_PROPERTY(QVariant heads READ heads NOTIFY headsChanged)
    Q_PROPERTY(bool useGlobalPhysical READ useGlobalPhysical CONSTANT)
    Q_PROPERTY(PhysicalEdit *physical READ physical CONSTANT)
    Q_PROPERTY(QStringList actsOnChannels READ actsOnChannels NOTIFY actsOnChannelsChanged)

public:
    ModeEdit(QLCFixtureMode *mode, QObject *parent = nullptr);
    ~ModeEdit();

    /** Get/Set the name of the mode being edited */
    QString name() const;
    void setName(QString name);

signals:
    void nameChanged();

private:
    /** Reference to the mode being edited */
    QLCFixtureMode *m_mode;

    /************************************************************************
     * Channels
     ************************************************************************/
public:
    /** Get a list of all the available channels in the definition */
    QVariant channels() const;

    /** Add a new channel to the mode being edited */
    Q_INVOKABLE void addChannel(QLCChannel *channel, int insertIndex = 0);

    /** Move $channel to a new position in the mode being edited */
    Q_INVOKABLE void moveChannel(QLCChannel *channel, int insertIndex = 0);

    /** Return the reference to a channel with the given $index
     *  in the mode being edited */
    Q_INVOKABLE QLCChannel *channelFromIndex(int index) const;

    /** Delete the given $channel from the mode being edited */
    Q_INVOKABLE bool deleteChannel(QLCChannel *channel);

    /** Return a simple list with the possible channels to act on */
    QStringList actsOnChannels();

    /** Return the channel where channel at $index acts on */
    Q_INVOKABLE int actsOnChannel(int index);

    Q_INVOKABLE void setActsOnChannel(int sourceIndex, int destIndex);

private:
    void updateChannelList();

signals:
    void channelsChanged();
    void actsOnChannelsChanged();

private:
    /** Reference to a channel list usable in QML */
    ListModel *m_channelList;

    /************************************************************************
     * Heads
     ************************************************************************/
public:
    /** Get a list of all the available heads in the definition */
    QVariant heads() const;

    /** Create a new head from the provided channel indices list */
    Q_INVOKABLE void addHead(QVariantList chIndexList);

    /** Delete the heads with the provided $headIndexList */
    Q_INVOKABLE void deleteHeads(QVariantList headIndexList);

private:
    void updateHeadsList();

signals:
    void headsChanged();

private:
    /** Reference to a head list usable in QML */
    ListModel *m_headList;

    /************************************************************************
     * Physical
     ************************************************************************/
public:
    /** Return if the selected mode is using global or overridden
     *  physical information */
    bool useGlobalPhysical();

    /** Get an editor reference for the
     *  override physical properties */
    PhysicalEdit *physical();

private:
    /** Reference to the override physical properties */
    PhysicalEdit *m_physical;
};

#endif /* MODEEDIT_H */
