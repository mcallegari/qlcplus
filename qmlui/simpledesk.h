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
#include "scenevalue.h"
#include "dmxsource.h"

#include <QMutex>

class FunctionManager;
class GenericFader;
class KeyPadParser;
class FadeChannel;
class ListModel;

class SimpleDesk : public PreviewContext, public DMXSource
{
    Q_OBJECT

    Q_PROPERTY(QVariant universesListModel READ universesListModel NOTIFY universesListModelChanged)
    Q_PROPERTY(QVariant channelList READ channelList NOTIFY channelListChanged)
    Q_PROPERTY(int dumpValuesCount READ dumpValuesCount NOTIFY dumpValuesCountChanged)
    Q_PROPERTY(quint32 dumpChannelMask READ dumpChannelMask NOTIFY dumpChannelMaskChanged)
    Q_PROPERTY(QVariantList fixtureList READ fixtureList NOTIFY fixtureListChanged)
    Q_PROPERTY(QStringList commandHistory READ commandHistory NOTIFY commandHistoryChanged)

public:
    SimpleDesk(QQuickView *view, Doc *doc,
               FunctionManager *funcMgr, QObject *parent = 0);
    ~SimpleDesk();

    QVariant universesListModel() const;

    /** @reimp */
    void setUniverseFilter(quint32 universeFilter);

    /** Return the actual list of channels for
     *  the currently selected universe */
    QVariant channelList() const;

    /** Return the list of fixtures for
     *  the currently selected universe */
    QVariantList fixtureList() const;

protected slots:
    void updateChannelList();

signals:
    void universesListModelChanged();
    void channelListChanged();
    void fixtureListChanged();

private:
    /** Reference to the Function Manager */
    FunctionManager *m_functionManager;

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
    Q_INVOKABLE void setValue(quint32 fixtureID, uint channel, uchar value);

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

signals:
    /** Informed the listeners that a channel value has changed.
     *  This is connected to ContextManager for Scene dump */
    void channelValueChanged(quint32 fixtureID, quint32 channelIndex, quint8 value);

private:
    /** A map of channel absolute addresses and their values.
      * Note that only channels overridden by Simple Desk are here */
    QHash <uint,uchar> m_values;

    /** Mutex to sync data access */
    mutable QMutex m_mutex;

    /** A list of commands to be executed on writeDMX.
     *  This is used to sync reset requests with mastertimer ticks */
    QList< QPair<int,quint32> > m_commandQueue;

    /*********************************************************************
     * DMX channels dump
     *********************************************************************/
public:
    /** Store a channel value for Scene dumping */
    Q_INVOKABLE void setDumpValue(quint32 fxID, quint32 channel, uchar value);

    /** Remove a channel from the Scene dumping list */
    Q_INVOKABLE void unsetDumpValue(quint32 fxID, quint32 channel);

    /** Return the number of DMX channels currently available for dumping */
    int dumpValuesCount() const;

    /** Return the current DMX dump channel type mask */
    int dumpChannelMask() const;

    Q_INVOKABLE void dumpDmxChannels(QString name, quint32 mask);

signals:
    void dumpValuesCountChanged();
    void dumpChannelMaskChanged();

private:
    /** List of the values available for dumping to a Scene */
    QList <SceneValue> m_dumpValues;

    /** Bitmask representing the available channel types for
     *  the DMX channels ready for dumping */
    quint32 m_dumpChannelMask;

    /************************************************************************
     * Keypad
     ************************************************************************/
public:
    Q_INVOKABLE bool sendKeypadCommand(QString command);

    /** Return a list of the last N commands
     *  entered on the keypad */
    QStringList commandHistory() const;

signals:
    void commandHistoryChanged();

private:
    KeyPadParser *m_keyPadParser;
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
