/*
  Q Light Controller
  fixtureconsole.h

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef FIXTURECONSOLE_H
#define FIXTURECONSOLE_H

#include <QWidget>
#include <QList>

#include "consolechannel.h"
#include "scene.h"

class QDomDocument;
class QDomElement;
class MasterTimer;
class QHBoxLayout;
class OutputMap;
class InputMap;
class Doc;

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

    FixtureConsole(QWidget* parent, Doc* doc, GroupType type = GroupNone);
    ~FixtureConsole();

private:
    Doc* m_doc;
    GroupType m_groupType;
    QHBoxLayout* m_layout;

    /*********************************************************************
     * Fixture
     *********************************************************************/
public:
    /** Set the fixture that this console is controlling */
    void setFixture(quint32 id);

    /** Get the fixture that this console is controlling */
    quint32 fixture() const;

protected:
    quint32 m_fixture;

    /*********************************************************************
     * Channels
     *********************************************************************/
public:
    /** Set channels' check state (UINT_MAX to set all) */
    void setChecked(bool state, quint32 channel = UINT_MAX);

    /** Enable/disable DMX output when sliders are dragged */
    void setOutputDMX(bool state);

    /** Set the value of one scene channel */
    void setSceneValue(const SceneValue& scv);

    /** Get all channel's values */
    QList <SceneValue> values() const;

    /** Set all channel's values */
    void setValues(const QList <SceneValue>& list);

    /** Set the value of one channel (doesn't enable it) */
    void setValue(quint32 ch, uchar value);

    /** Get the value of one channel (regardless of whether it's enabled) */
    uchar value(quint32 ch) const;

signals:
    /** Emitted when the value of a channel object changes (continuous) */
    void valueChanged(quint32 fxi, quint32 channel, uchar value);

    /** Emitted when the value of a channel object changes (single shot) */
    void valueSingleChange(quint32 fxi, quint32 channel, uchar value);

    /** Emitted when a channel's check state is changed */
    void checked(quint32 fxi, quint32 channel, bool state);

private:
    /** Get a console channel instance for the given relative channel */
    ConsoleChannel* channel(quint32 ch) const;

private:
    QList<ConsoleChannel*> m_channels;
};

#endif
