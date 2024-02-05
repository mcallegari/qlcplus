/*
  Q Light Controller Plus
  vcclock.cpp

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QQmlEngine>
#include <QDateTime>
#include <QTimer>

#include "vcclock.h"
#include "doc.h"

#define KXMLQLCVCClockEnabled   QString("Enable")
#define KXMLQLCVCClockType      QString("Type")
#define KXMLQLCVCClockTime      QString("Time")

#define KXMLQLCVCClockHours     QString("Hours")     // LEGACY
#define KXMLQLCVCClockMinutes   QString("Minutes")   // LEGACY
#define KXMLQLCVCClockSeconds   QString("Seconds")   // LEGACY

#define KXMLQLCVCClockSchedule          QString("Schedule")
#define KXMLQLCVCClockScheduleFunc      QString("Function")
#define KXMLQLCVCClockScheduleStartTime QString("StartTime")
#define KXMLQLCVCClockScheduleStopTime  QString("StopTime")
#define KXMLQLCVCClockScheduleWeekFlags QString("WeekFlags")

#define KXMLQLCVCClockScheduleTime QString("Time")  // LEGACY

VCClock::VCClock(Doc *doc, QObject *parent)
    : VCWidget(doc, parent)
    , m_clocktype(Clock)
    , m_targetTime(0)
    , m_enableSchedule(false)
{
    setType(VCWidget::ClockWidget);

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimerTimeout()));
    m_timer->start(1000);
}

VCClock::~VCClock()
{
    delete m_timer;

    int schNum = m_scheduleList.count();
    for (int i = 0; i < schNum; i++)
    {
        VCClockSchedule *sch = m_scheduleList.takeLast();
        delete sch;
    }

    if (m_item)
        delete m_item;
}

QString VCClock::defaultCaption()
{
    return tr("Clock %1").arg(id() + 1);
}

void VCClock::setupLookAndFeel(qreal pixelDensity, int page)
{
    setPage(page);
    QFont wFont = font();
    wFont.setBold(true);
    wFont.setPointSize(pixelDensity * 5.0);
    setFont(wFont);
}

void VCClock::render(QQuickView *view, QQuickItem *parent)
{
    if (view == nullptr || parent == nullptr)
        return;

    QQmlComponent *component = new QQmlComponent(view->engine(), QUrl("qrc:/VCClockItem.qml"));

    if (component->isError())
    {
        qDebug() << component->errors();
        return;
    }

    m_item = qobject_cast<QQuickItem*>(component->create());

    m_item->setParentItem(parent);
    m_item->setProperty("clockObj", QVariant::fromValue(this));
}

QString VCClock::propertiesResource() const
{
    return QString("qrc:/VCClockProperties.qml");
}

VCWidget *VCClock::createCopy(VCWidget *parent)
{
    Q_ASSERT(parent != nullptr);

    VCClock *clock = new VCClock(m_doc, parent);
    if (clock->copyFrom(this) == false)
    {
        delete clock;
        clock = nullptr;
    }

    return clock;
}

bool VCClock::copyFrom(const VCWidget *widget)
{
    const VCClock *clock = qobject_cast<const VCClock*> (widget);
    if (clock == nullptr)
        return false;

    /* Clock type */
    setClockType(clock->clockType());
    setEnableSchedule(clock->enableSchedule());

    /* Copy schedules */
    for (VCClockSchedule *sch : clock->schedules())
    {
        VCClockSchedule *dst = new VCClockSchedule();
        dst->setFunctionID(sch->functionID());
        dst->setStartTime(sch->startTime());
        dst->setStopTime(sch->stopTime());
        dst->setWeekFlags(sch->weekFlags());
        addSchedule(dst);
    }

    /* Common stuff */
    return VCWidget::copyFrom(widget);
}

FunctionParent VCClock::functionParent() const
{
    return FunctionParent(FunctionParent::AutoVCWidget, id());
}

/*********************************************************************
 * Type
 *********************************************************************/

void VCClock::setClockType(VCClock::ClockType type)
{
    if (type == m_clocktype)
        return;

    m_clocktype = type;

    if (m_clocktype == Clock)
        m_timer->start(1000);
    else
        m_timer->stop();

    emit clockTypeChanged(type);
}

VCClock::ClockType VCClock::clockType() const
{
    return m_clocktype;
}

QString VCClock::typeToString(VCClock::ClockType type)
{
    if (type == Stopwatch)
        return "Stopwatch";
    else if (type == Countdown)
        return "Countdown";
    else
        return "Clock";
}

VCClock::ClockType VCClock::stringToType(QString str)
{
    if (str == "Stopwatch")
        return Stopwatch;
    else if (str == "Countdown")
        return Countdown;
    else
        return Clock;
}

/*********************************************************************
 * Time
 *********************************************************************/

int VCClock::currentTime() const
{
    QTime currTime = QDateTime::currentDateTime().time();
    int dayTimeSecs = (currTime.hour() * 60 * 60) + (currTime.minute() * 60) + currTime.second();
    return dayTimeSecs;
}

int VCClock::targetTime() const
{
    if (clockType() == Countdown)
        return m_targetTime;

    return 0;
}

void VCClock::setTargetTime(int ms)
{
    if (ms == m_targetTime)
        return;

    m_targetTime = ms;
    emit targetTimeChanged(ms);
}

void VCClock::slotTimerTimeout()
{
    QDateTime currDate = QDateTime::currentDateTime();
    QTime currTime = currDate.time();
    int currDay = 1 << (currDate.date().dayOfWeek() - 1);
    int dayTimeSecs = (currTime.hour() * 60 * 60) + (currTime.minute() * 60) + currTime.second();

    // if we're editing or the widget is disabled, just emit the current time
    if (isEditing() || enableSchedule() == false)
    {
        emit currentTimeChanged(dayTimeSecs);
        return;
    }

    for (VCClockSchedule *sch : m_scheduleList) // C++11
    {
        if (sch->m_cachedDuration == -1)
        {
            Function *f = m_doc->function(sch->functionID());
            if (f != nullptr)
                sch->m_cachedDuration = f->totalDuration() / 1000;
        }

        /**
         *  Now, there are a bunch of cases to be checked:
         *
         *  1- Function with no duration (e.g. Scene) and no stop time. This case runs indefinitely
         *  2- Function with duration and no stop time (must loop if repeat flag is high)
         *  3- Function with no duration and stop time
         *  4- Function with duration and stop time (must loop until stop time)
         *
         *  Each case must be checked against days of the week
         */

        if (dayTimeSecs >= sch->startTime())
        {
            // if there's a stop time and we past it, then skip
            if (sch->stopTime() > 0 && dayTimeSecs > sch->stopTime())
                continue;

            // check for existing Function
            Function *f = m_doc->function(sch->functionID());
            if (f == nullptr)
                continue;

            // case #3 and #4
            if (dayTimeSecs == sch->stopTime())
            {
                if (f->isRunning())
                    f->stop(functionParent());
                continue;
            }

            // case #2: check for 'one shot' Functions with duration
            if (sch->stopTime() == -1 && sch->m_cachedDuration > 0)
            {
                if (dayTimeSecs >= sch->startTime() + sch->m_cachedDuration &&
                    (sch->weekFlags() & 0x80) == 0)
                    continue;
            }

            // check for the day of the week, if selected
            if ((sch->weekFlags() & 0x7F) == 0 || sch->weekFlags() & currDay)
            {
                // check for repetition
                if (sch->m_canPlay)
                {
                    if (f->isRunning())
                        continue;

                    f->start(m_doc->masterTimer(), functionParent());
                    qDebug() << "VC Clock starting function:" << f->name();

                    // if repetition flag is down, then this Function
                    // must not be played anymore
                    if ((sch->weekFlags() & 0x80) == 0)
                        sch->m_canPlay = false;
                }
            }
        }
    }

    // at last, notify the UI that the time has changed
    emit currentTimeChanged(dayTimeSecs);
}

/*********************************************************************
 * Functions scheduling
 *********************************************************************/

bool VCClock::enableSchedule() const
{
    return m_enableSchedule;
}

void VCClock::setEnableSchedule(bool enableSchedule)
{
    if (m_enableSchedule == enableSchedule)
        return;

    /* When disabling, check for running functions and stop them */
    if (enableSchedule == false)
    {
        for (VCClockSchedule *sch : m_scheduleList) // C++11
        {
            Function *f = m_doc->function(sch->functionID());
            if (f != nullptr && f->isRunning())
                f->stop(functionParent());
            sch->m_canPlay = true;
        }
    }

    m_enableSchedule = enableSchedule;
    emit enableScheduleChanged(enableSchedule);
}

QVariantList VCClock::scheduleList()
{
    QVariantList list;
    for (VCClockSchedule *sch : m_scheduleList) // C++11
        list.append(QVariant::fromValue(sch));
    return list;
}

QList<VCClockSchedule *> VCClock::schedules() const
{
    return m_scheduleList;
}

void VCClock::addSchedule(VCClockSchedule *schedule)
{
    if (schedule->functionID() != Function::invalidId())
        m_scheduleList.append(schedule);
    std::sort(m_scheduleList.begin(), m_scheduleList.end());
    QQmlEngine::setObjectOwnership(schedule, QQmlEngine::CppOwnership);
    emit scheduleListChanged();
}

void VCClock::addSchedules(QVariantList idsList)
{
    for (QVariant vID : idsList) // C++11
    {
        quint32 funcID = vID.toUInt();
        if (m_doc->function(funcID) == nullptr)
            continue;

        VCClockSchedule *sch = new VCClockSchedule();
        QQmlEngine::setObjectOwnership(sch, QQmlEngine::CppOwnership);
        sch->setFunctionID(funcID);
        m_scheduleList.append(sch);
    }

    std::sort(m_scheduleList.begin(), m_scheduleList.end());
    emit scheduleListChanged();
}

void VCClock::removeSchedule(int index)
{
    if (index < 0 || index > m_scheduleList.count())
        return;

    VCClockSchedule *sch = m_scheduleList.takeAt(index);
    Function *f = m_doc->function(sch->functionID());
    if (f != nullptr && f->isRunning())
        f->stop(functionParent());
    delete sch;
    emit scheduleListChanged();
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

bool VCClock::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCClock)
    {
        qWarning() << Q_FUNC_INFO << "Clock node not found";
        return false;
    }

    QXmlStreamAttributes attrs = root.attributes();
    bool enableSchedule = true;

    if (attrs.hasAttribute(KXMLQLCVCClockEnabled))
    {
        QString en = attrs.value(KXMLQLCVCClockEnabled).toString();
        if (en == "False")
            enableSchedule = false;
    }
    setEnableSchedule(enableSchedule);

    if (attrs.hasAttribute(KXMLQLCVCClockType))
    {
        setClockType(stringToType(attrs.value(KXMLQLCVCClockType).toString()));
        if (clockType() == Countdown)
        {
            int msTime = 0;

            if (attrs.hasAttribute(KXMLQLCVCClockTime))
            {
                QTime tTime = QTime::fromString(attrs.value(KXMLQLCVCClockTime).toString(), "HH:mm:ss");
                msTime = tTime.msecsSinceStartOfDay() / 1000;
            }
            else // LEGACY
            {
                int h = 0, m = 0, s = 0;
                if (attrs.hasAttribute(KXMLQLCVCClockHours))
                    h = attrs.value(KXMLQLCVCClockHours).toString().toInt();
                if (attrs.hasAttribute(KXMLQLCVCClockMinutes))
                    m = attrs.value(KXMLQLCVCClockMinutes).toString().toInt();
                if (attrs.hasAttribute(KXMLQLCVCClockSeconds))
                    s = attrs.value(KXMLQLCVCClockSeconds).toString().toInt();

                msTime = (h * 60 * 60 * 1000) + (m * 60 * 1000) + (s * 1000);
            }
            setTargetTime(msTime);
        }
    }

    /* Widget commons */
    loadXMLCommon(root);

    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCWindowState)
        {
            bool visible = false;
            int x = 0, y = 0, w = 0, h = 0;
            loadXMLWindowState(root, &x, &y, &w, &h, &visible);
            setGeometry(QRect(x, y, w, h));
        }
        else if (root.name() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(root);
        }
        else if (root.name() == KXMLQLCVCClockSchedule)
        {
            VCClockSchedule *sch = new VCClockSchedule(m_doc);
            if (sch->loadXML(root) == true)
                addSchedule(sch);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown clock tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool VCClock::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != nullptr);

    /* VC Clock entry */
    doc->writeStartElement(KXMLQLCVCClock);

    doc->writeAttribute(KXMLQLCVCClockEnabled, enableSchedule() ? "True" : "False");

    /* Type */
    ClockType type = clockType();
    doc->writeAttribute(KXMLQLCVCClockType, typeToString(type));
    if (type == Countdown)
    {
        QTime tTime;
        int tt = targetTime();
        int hh = (tt / 3600);
        tt -= (hh * 3600);
        int mm = (tt / 60);
        tt -= (mm * 60);
        tTime.setHMS(hh, mm, tt);

        doc->writeAttribute(KXMLQLCVCClockTime, tTime.toString());
    }

    saveXMLCommon(doc);

    /* Window state */
    saveXMLWindowState(doc);

    /* Appearance */
    saveXMLAppearance(doc);

    for (VCClockSchedule *sch : m_scheduleList) // C++11
        sch->saveXML(doc);

    /* End the <Clock> tag */
    doc->writeEndElement();

    return true;
}

/*********************************************************************
 * VCClockSchedule Class methods
 *********************************************************************/

bool VCClockSchedule::operator <(const VCClockSchedule &sch) const
{
    if (sch.startTime() < startTime())
        return false;
    return true;
}


VCClockSchedule::VCClockSchedule(QObject *parent)
    : QObject(parent)
    , m_canPlay(true)
    , m_cachedDuration(-1)
    , m_id(Function::invalidId())
    , m_startTime(0)
    , m_stopTime(-1)
    , m_weekFlags(0)
{
}

bool VCClockSchedule::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCClockSchedule)
    {
        qWarning() << Q_FUNC_INFO << "Clock Schedule node not found";
        return false;
    }

    QXmlStreamAttributes attrs = root.attributes();

    if (attrs.hasAttribute(KXMLQLCVCClockScheduleFunc))
    {
        setFunctionID(attrs.value(KXMLQLCVCClockScheduleFunc).toString().toUInt());

        if (attrs.hasAttribute(KXMLQLCVCClockScheduleStartTime))
        {
            QTime start = QTime::fromString(attrs.value(KXMLQLCVCClockScheduleStartTime).toString(), "HH:mm:ss");
            setStartTime(start.msecsSinceStartOfDay() / 1000);
        }
        if (attrs.hasAttribute(KXMLQLCVCClockScheduleStopTime))
        {
            QTime stop = QTime::fromString(attrs.value(KXMLQLCVCClockScheduleStopTime).toString(), "HH:mm:ss");
            setStopTime(stop.msecsSinceStartOfDay() / 1000);
        }
        if (attrs.hasAttribute(KXMLQLCVCClockScheduleTime)) // LEGACY
        {
            QTime start = QTime::fromString(attrs.value(KXMLQLCVCClockScheduleTime).toString(), "HH:mm:ss");
            setStartTime(start.msecsSinceStartOfDay() / 1000);
        }

        if (attrs.hasAttribute(KXMLQLCVCClockScheduleWeekFlags))
            setWeekFlags(uchar(attrs.value(KXMLQLCVCClockScheduleWeekFlags).toUInt()));
    }
    root.skipCurrentElement();

    return true;
}

bool VCClockSchedule::saveXML(QXmlStreamWriter *doc)
{
    /* Schedule tag */
    doc->writeStartElement(KXMLQLCVCClockSchedule);

    /* Schedule function */
    doc->writeAttribute(KXMLQLCVCClockScheduleFunc, QString::number(functionID()));

    qDebug() << "Start time:" << startTime() << ", stopTime:" << stopTime();

    /* Schedule start time */
    QTime start;
    int st = startTime();
    int hh = (st / 3600);
    st -= (hh * 3600);
    int mm = (st / 60);
    st -= (mm * 60);
    start.setHMS(hh, mm, st);
    doc->writeAttribute(KXMLQLCVCClockScheduleStartTime, start.toString());

    if (stopTime() != -1)
    {
        /* Schedule stop time */
        QTime stop;
        st = stopTime();
        hh = (st / 3600);
        st -= (hh * 3600);
        mm = (st / 60);
        st -= (mm * 60);
        stop.setHMS(hh, mm, st);
        doc->writeAttribute(KXMLQLCVCClockScheduleStopTime, stop.toString());
    }
    if (weekFlags() != 0)
        doc->writeAttribute(KXMLQLCVCClockScheduleWeekFlags, QString::number(weekFlags()));

    doc->writeEndElement();

    return true;
}
