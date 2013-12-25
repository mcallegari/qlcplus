/*
  Q Light Controller
  monitor.h

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

#ifndef MONITOR_H
#define MONITOR_H

#include <QWidget>
#include <QHash>
#include <QList>

class MonitorFixture;
class MonitorLayout;
class QDomDocument;
class QDomElement;
class QScrollArea;
class OutputMap;
class QAction;
class Fixture;
class Monitor;
class QTimer;
class Doc;

class Monitor : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(Monitor)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** Get the monitor singleton instance. Can be NULL. */
    static Monitor* instance();

    /** Create or show Monitor */
    static void createAndShow(QWidget* parent, Doc* doc);

    /** Normal public destructor */
    ~Monitor();

protected:
    void loadSettings();
    void saveSettings();

    /** Protected constructor to prevent multiple instances. */
    Monitor(QWidget* parent, Doc* doc, Qt::WindowFlags f = 0);

protected:
    /** The singleton Monitor instance */
    static Monitor* s_instance;
    Doc* m_doc;

    /*********************************************************************
     * Channel & Value styles
     *********************************************************************/
public:
    enum ChannelStyle { DMXChannels, RelativeChannels };
    enum ValueStyle { DMXValues, PercentageValues };

    /** Get the style used to draw DMX values in monitor fixtures */
    ValueStyle valueStyle() const;

    /** Get the style used to draw channel numbers in monitor fixtures */
    ChannelStyle channelStyle() const;

private:
    ValueStyle m_valueStyle;
    ChannelStyle m_channelStyle;

    /*********************************************************************
     * Menu
     *********************************************************************/
protected:
    /** Create tool bar */
    void initToolBar();

protected slots:
    /** Menu action slot for font selection */
    void slotChooseFont();

    /** Menu action slot for channel style selection */
    void slotChannelStyleTriggered();

    /** Menu action slot for value style selection */
    void slotValueStyleTriggered();

    /********************************************************************
     * Monitor Fixtures
     ********************************************************************/
public:
    /** Update monitor fixture labels */
    void updateFixtureLabelStyles();

protected:
    /** Create a new MonitorFixture* and append it to the layout */
    void createMonitorFixture(Fixture* fxi);

protected slots:
    /** Slot for fixture additions (to append the new fixture to layout) */
    void slotFixtureAdded(quint32 fxi_id);

    /** Slot for fixture contents & layout changes */
    void slotFixtureChanged(quint32 fxi_id);

    /** Slot for fixture removals (to remove the fixture from layout) */
    void slotFixtureRemoved(quint32 fxi_id);

    /** Slot for getting the latest values from OutputMap */
    void slotUniversesWritten(int index, const QByteArray& ua);

signals:
    void channelStyleChanged(Monitor::ChannelStyle style);
    void valueStyleChanged(Monitor::ValueStyle style);

protected:
    QScrollArea* m_scrollArea;
    QWidget* m_monitorWidget;
    MonitorLayout* m_monitorLayout;
    QList <MonitorFixture*> m_monitorFixtures;
};

#endif
