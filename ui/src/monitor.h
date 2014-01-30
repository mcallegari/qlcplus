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

class MonitorGraphicsView;
class MonitorFixture;
class MonitorLayout;
class QDomDocument;
class QDomElement;
class QScrollArea;
class QComboBox;
class QToolBar;
class QSpinBox;
class QAction;
class Fixture;
class Monitor;
class QTimer;
class Doc;

/** @addtogroup ui UI
 * @{
 */

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

    /** Initialize the monitor view */
    void initView();

    /** Initialize the monitor view in DMX mode */
    void initDMXView();

    /** Initialize the monitor view in 2D graphics mode */
    void initGraphicsView();

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
    enum DisplayMode { DMX, Graphics };
    enum ChannelStyle { DMXChannels, RelativeChannels };
    enum ValueStyle { DMXValues, PercentageValues };

    /** Get the display mode used to render the monitor */
    DisplayMode displayMode() const;

    /** Get the style used to draw DMX values in monitor fixtures */
    ValueStyle valueStyle() const;

    /** Get the style used to draw channel numbers in monitor fixtures */
    ChannelStyle channelStyle() const;

private:
    DisplayMode m_displayMode;
    ChannelStyle m_channelStyle;
    ValueStyle m_valueStyle;

    /*********************************************************************
     * Menu
     *********************************************************************/
protected:

    /** Create DMX view toolbar */
    void initDMXToolbar();

    void initGraphicsToolbar();

protected slots:
    /** Menu action slot for font selection */
    void slotChooseFont();

    /** Menu action slot for channel style selection */
    void slotChannelStyleTriggered();

    /** Menu action slot for value style selection */
    void slotValueStyleTriggered();

    /** Menu action slot to trigger display mode switch */
    void slotSwitchMode();

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

    /** Slot for getting the latest values from InputOutputMap */
    void slotUniversesWritten(int index, const QByteArray& ua);

signals:
    void channelStyleChanged(Monitor::ChannelStyle style);
    void valueStyleChanged(Monitor::ValueStyle style);

protected:
    QToolBar* m_toolBar;
    QScrollArea* m_scrollArea;
    QWidget* m_monitorWidget;
    MonitorLayout* m_monitorLayout;
    QList <MonitorFixture*> m_monitorFixtures;

    /********************************************************************
     * Graphics View
     ********************************************************************/

protected slots:
    /** Slot called when the grid width changes */
    void slotGridWidthChanged(int value);

    /** Slot called when the grid height changes */
    void slotGridHeightChanged(int value);

    /** Slot called when the unit metrics changes */
    void slotMetricsChanged(int index);

    /** Slot called when the user wants to add
     *  a fixture to the graphics view
     */
    void slotAddFixture();

protected:
    MonitorGraphicsView* m_graphicsView;
    QSpinBox* m_gridWSpin;
    QSpinBox *m_gridHSpin;
    QComboBox *m_unitsCombo;
};

/** @} */

#endif
