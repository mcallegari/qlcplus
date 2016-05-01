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

#include "vcwidget.h"

#define KXMLQLCVCClock "Clock"

class VCClockSchedule : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(VCClockSchedule)

    Q_PROPERTY(int functionID READ functionID WRITE setFunctionID NOTIFY functionIDChanged)
    Q_PROPERTY(int startTime READ startTime WRITE setStartTime NOTIFY startTimeChanged)
    Q_PROPERTY(int stopTime READ stopTime WRITE setStopTime NOTIFY stopTimeChanged)
    Q_PROPERTY(uchar weekFlags READ weekFlags WRITE setWeekFlags NOTIFY weekFlagsChanged)

public:
    VCClockSchedule(QObject* parent = 0);

    virtual ~VCClockSchedule() { }

    void setFunctionID(quint32 id) { m_id = id; }
    quint32 functionID() const { return m_id; }

    void setStartTime(int time) { m_startTime = time; }
    int startTime() const { return m_startTime; }

    void setStopTime(int time) { m_stopTime = time; }
    int stopTime() const { return m_stopTime; }

    void setWeekFlags(uchar flags) { m_weekFlags = flags; }
    uchar weekFlags() const { return m_weekFlags; }

    /** Sorting operator */
    bool operator<(const VCClockSchedule& sch) const;

    /** Load & Save */
    bool loadXML(QXmlStreamReader &root);
    bool saveXML(QXmlStreamWriter *doc);

signals:
    void functionIDChanged();
    void startTimeChanged();
    void stopTimeChanged();
    void weekFlagsChanged();

protected:
    /** Get the parent Doc object */
    Doc* doc() const { return qobject_cast<Doc*>(parent()); }

private:
    quint32 m_id;
    int m_startTime;
    int m_stopTime;
    uchar m_weekFlags;
};

class VCClock : public VCWidget
{
    Q_OBJECT

    Q_PROPERTY(ClockType clockType READ clockType WRITE setClockType NOTIFY clockTypeChanged)
    Q_PROPERTY(int  targetTime READ targetTime WRITE setTargetTime NOTIFY targetTimeChanged)
    Q_PROPERTY(QVariantList scheduleList READ scheduleList NOTIFY scheduleListChanged)
    
    /*********************************************************************
     * Initialization
     *********************************************************************/

public:
    VCClock(Doc* doc = NULL, QObject *parent = 0);
    virtual ~VCClock();

    /** @reimp */
    void setID(quint32 id);

    /** @reimp */
    void render(QQuickView *view, QQuickItem *parent);

    /** @reimp */
    QString propertiesResource() const;

    /*********************************************************************
     * Type
     *********************************************************************/
public:
    enum ClockType { Clock, Stopwatch, Countdown };
    Q_ENUMS(ClockType)

    void setClockType(ClockType type);
    ClockType clockType() const;

    QString typeToString(ClockType type);
    ClockType stringToType(QString str);

signals:
    void clockTypeChanged(ClockType type);

private:
    ClockType m_clocktype;
    
    /*********************************************************************
     * Time
     *********************************************************************/
public:
    int targetTime() const;
    void setTargetTime(int ms);

signals:
    void targetTimeChanged(int ms);

private:
    /** the target time in ms of the clock. Used by Countdown and Stopwatch */
    int m_targetTime;

    /*********************************************************************
     * Functions scheduling
     *********************************************************************/
public:
    QVariantList scheduleList();
    void addSchedule(VCClockSchedule *schedule);
    Q_INVOKABLE void removeSchedule(int index);

signals:
    void scheduleListChanged();

private:
    QList<VCClockSchedule*>m_scheduleList;
    /*********************************************************************
     * Load & Save
     *********************************************************************/

public:
    bool loadXML(QXmlStreamReader &root);
    bool saveXML(QXmlStreamWriter *doc);
};

#endif
