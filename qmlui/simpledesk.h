/*
  Q Light Controller Plus
  simpledesk.h

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

#ifndef SIMPLEDESK_H
#define SIMPLEDESK_H

#include "previewcontext.h"
#include "dmxsource.h"

#include <QMutex>

class GenericFader;
class FadeChannel;
class ListModel;

class SimpleDesk : public PreviewContext, public DMXSource
{
    Q_OBJECT

    Q_PROPERTY(QVariant universesListModel READ universesListModel CONSTANT)
    Q_PROPERTY(QVariant channelList READ channelList NOTIFY channelListChanged)
    Q_PROPERTY(QStringList commandHistory READ commandHistory NOTIFY commandHistoryChanged)

public:
    SimpleDesk(QQuickView *view, Doc *doc, QObject *parent = 0);
    ~SimpleDesk();

    QVariant universesListModel() const;

    /** @reimp */
    void setUniverseFilter(quint32 universeFilter);

    QVariant channelList() const;

protected slots:
    void updateChannelList();

signals:
    void channelListChanged();

private:
    /** QML ready model to hold channel values and changes */
    ListModel *m_channelList;

    /** Values array for comparison */
    QMap<quint32, QByteArray> m_prevUniverseValues;

    /************************************************************************
     * Universe Values
     ************************************************************************/
public:
    enum SimpleDeskCommand
    {
        ResetChannel,
        ResetUniverse
    };

    enum ChannelStatus { None = 0, Odd, Even, Override };
    Q_ENUM(ChannelStatus)

    /** Set the value of a single channel */
    Q_INVOKABLE void setValue(uint channel, uchar value);

    /** Get the value of a single channel */
    uchar value(uint channel) const;

    /** Check if Simple desk is currently controlling a channel */
    bool hasChannel(uint channel);

    /** Reset the values of the given universe to zero */
    Q_INVOKABLE void resetUniverse(int universe);

    /** Reset the value of the specified channel */
    Q_INVOKABLE void resetChannel(uint channel);

protected slots:
    /** Invoked by the QLC+ engine to inform the UI that the
     *  Universe at $idx has changed */
    void slotUniverseWritten(quint32 idx, const QByteArray& ua);

private:
    /** A map of channel absolute addresses and their values.
      * Note that only channels overridden by Simple Desk are here */
    QHash <uint,uchar> m_values;

    /** Mutex to sync data access */
    mutable QMutex m_mutex;

    /** A list of commands to be executed on writeDMX.
     *  This is used to sync reset requests with mastertimer ticks */
    QList< QPair<int,quint32> > m_commandQueue;

    /************************************************************************
     * Keypad
     ************************************************************************/
public:
    Q_INVOKABLE void sendKeypadCommand(QString command);

    /** Return a list of the last N commands
     *  entered on the keypad */
    QStringList commandHistory() const;

signals:
    void commandHistoryChanged();

private:
    QStringList m_keypadCommandHistory;

    /************************************************************************
     * DMXSource
     ************************************************************************/
public:
    /** @reimpl */
    void writeDMX(MasterTimer* timer, QList<Universe*> ua);

private:
    FadeChannel *getFader(QList<Universe *> universes, quint32 universeID,
                          quint32 fixtureID, quint32 channel);

private:
    /** Map used to lookup a GenericFader instance for a Universe ID */
    QMap<quint32, QSharedPointer<GenericFader> > m_fadersMap;
};

#endif // SIMPLEDESK_H
