/*
  Q Light Controller
  fixtureconsole.h

  Copyright (c) Heikki Junnila

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

#ifndef FIXTURECONSOLE_H
#define FIXTURECONSOLE_H

#include <QWidget>
#include <QList>

#include "consolechannel.h"
#include "scene.h"

class MasterTimer;
class QHBoxLayout;
class OutputMap;
class InputMap;
class Doc;

/** @addtogroup ui_fixtures
 * @{
 */

#define KXMLQLCFixtureConsole "Console"

class FixtureConsole : public QGroupBox
{
    Q_OBJECT
    Q_DISABLE_COPY(FixtureConsole)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    enum GroupType
    {
        GroupNone,
        GroupEven,
        GroupOdd
    };

    FixtureConsole(QWidget* parent, Doc* doc, GroupType type = GroupNone, bool showCheck = true);
    ~FixtureConsole();

    void enableResetButton(bool enable);

signals:
    void resetRequest(quint32 fxID, quint32 channel);

protected:
    void showEvent(QShowEvent* ev);

private:
    Doc* m_doc;
    GroupType m_groupType;
    QHBoxLayout* m_layout;
    bool m_showCheckBoxes;

    /*********************************************************************
     * Fixture
     *********************************************************************/
public:
    /** Set the fixture that this console is controlling */
    void setFixture(quint32 id);

    /** Get the fixture that this console is controlling */
    quint32 fixture() const;

protected slots:
    void slotAliasChanged();

protected:
    quint32 m_fixture;

    /*********************************************************************
     * Channels
     *********************************************************************/
public:
    /** Set channels' check state (UINT_MAX to set all) */
    void setChecked(bool state, quint32 channel = UINT_MAX);

    /** Set the value of one scene channel */
    void setSceneValue(const SceneValue& scv);

    /** Get all channel's values */
    QList <SceneValue> values() const;

    /** Return true if at least one channel is checked and selected */
    bool hasSelections();

    /** Set all channel's values */
    void setValues(const QList <SceneValue>& list, bool fromSelection);

    /** Set the value of one channel (doesn't enable it) */
    void setValue(quint32 ch, uchar value, bool apply = true);

    /** Get the value of one channel (regardless of whether it's enabled) */
    uchar value(quint32 ch) const;

    /** Set the stylesheet of the ConsoleChannel at the given index */
    void setChannelStylesheet(quint32 ch, QString ss);

    /** Reset all the channels stylesheet to the original value */
    void resetChannelsStylesheet();

signals:
    /** Emitted when the value of a channel object changes */
    void valueChanged(quint32 fxi, quint32 channel, uchar value);

    /** Emitted when a channel's check state is changed */
    void checked(quint32 fxi, quint32 channel, bool state);

private:
    /** Get a console channel instance for the given relative channel */
    ConsoleChannel *channel(quint32 ch) const;

private:
    QList<ConsoleChannel*> m_channels;
    QString m_styleSheet;
};

/** @} */

#endif
