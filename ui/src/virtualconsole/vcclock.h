/*
  Q Light Controller Plus
  vcclock.h

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

#ifndef VCCLOCK_H
#define VCCLOCK_H

#include <QDateTime>
#include <QKeySequence>

#include "vcwidget.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class QPaintEvent;
class InputMap;
class Doc;

/** @addtogroup ui_vc_widgets
 * @{
 */

#define KXMLQLCVCClock QString("Clock")

class VCClockSchedule
{
public:
    VCClockSchedule() { }

    void setFunction(quint32 id) { m_id = id; }
    quint32 function() const { return m_id; }
    void setTime(QDateTime time) { m_time = time; }
    QDateTime time() const { return m_time; }

    /** Sorting operator */
    bool operator<(const VCClockSchedule& sch) const;

    /** Load & Save */
    bool loadXML(QXmlStreamReader &root);
    bool saveXML(QXmlStreamWriter *doc);

private:
    quint32 m_id;
    QDateTime m_time;
};

class VCClock : public VCWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(VCClock)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCClock(QWidget* parent, Doc* doc);
    ~VCClock();

    /*********************************************************************
     * QLC+ Mode
     *********************************************************************/
public slots:
    void slotModeChanged(Doc::Mode mode);

    /*********************************************************************
     * Type
     *********************************************************************/
public:
    enum ClockType
    {
        Clock,
        Stopwatch,
        Countdown
    };

    void setClockType(ClockType type);
    ClockType clockType() const;

    QString typeToString(ClockType type);
    ClockType stringToType(QString str);


    /*********************************************************************
     * Functions scheduling
     *********************************************************************/
public:
    void addSchedule(VCClockSchedule schedule);
    void removeSchedule(int index);
    void removeAllSchedule();
    QList<VCClockSchedule> schedules();

private:
    ClockType m_clocktype;
    QList<VCClockSchedule>m_scheduleList;
    int m_scheduleIndex;

private:
    FunctionParent functionParent() const;

    /*********************************************************************
     * Time
     *********************************************************************/

public:
    void setCountdown(int h, int m, int s);
    void playPauseTimer();
    void resetTimer();
    long currentTime() { return m_currentTime; }
    int getHours() { return m_hh; }
    int getMinutes() { return m_mm; }
    int getSeconds() { return m_ss; }

signals:
    void timeChanged(quint32 time);

protected slots:
    void slotUpdateTime();

private:
    int m_hh, m_mm, m_ss;
    quint32 m_targetTime;
    quint32 m_currentTime;
    bool m_isPaused;

public:
    static const quint8 playInputSourceId;
    static const quint8 resetInputSourceId;

    /*************************************************************************
     * Key sequences
     *************************************************************************/
public:
    /** Set the keyboard key combination for playing/pausing the timer */
    void setPlayKeySequence(const QKeySequence& keySequence);

    /** Get the keyboard key combination for playing/pausing the timer */
    QKeySequence playKeySequence() const;

    /** Set the keyboard key combination for resetting the timer */
    void setResetKeySequence(const QKeySequence& keySequence);

    /** Get the keyboard key combination for resetting the timer */
    QKeySequence resetKeySequence() const;

private:
    QKeySequence m_playKeySequence;
    QKeySequence m_resetKeySequence;

protected slots:
    void slotKeyPressed(const QKeySequence& keySequence);

    /*************************************************************************
     * External Input
     *************************************************************************/
public:
    void updateFeedback();

protected slots:
    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value);

private:
    quint32 m_playLatestValue;
    quint32 m_resetLatestValue;

    /*********************************************************************
     * Clipboard
     *********************************************************************/
public:
    VCWidget* createCopy(VCWidget* parent);
    bool copyFrom(const VCWidget *widget);

    /*********************************************************************
     * Properties
     *********************************************************************/
public:
    void editProperties();

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    bool loadXML(QXmlStreamReader &root);
    bool saveXML(QXmlStreamWriter *doc);

    /*********************************************************************
     * Painting
     *********************************************************************/
protected:
    void paintEvent(QPaintEvent* e);
    void mousePressEvent(QMouseEvent* e);
};

/** @} */

#endif
