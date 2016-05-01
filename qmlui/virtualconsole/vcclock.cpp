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

#include "vcclock.h"
#include "doc.h"

#define KXMLQLCVCClockType "Type"
#define KXMLQLCVCClockTime "Time"

#define KXMLQLCVCClockHours "Hours"        // LEGACY
#define KXMLQLCVCClockMinutes "Minutes"    // LEGACY
#define KXMLQLCVCClockSeconds "Seconds"    // LEGACY

#define KXMLQLCVCClockSchedule "Schedule"
#define KXMLQLCVCClockScheduleFunc "Function"
#define KXMLQLCVCClockScheduleStartTime "StartTime"
#define KXMLQLCVCClockScheduleStopTime "StopTime"
#define KXMLQLCVCClockScheduleWeekFlags "WeekFlags"

#define KXMLQLCVCClockScheduleTime "Time"  // LEGACY

VCClock::VCClock(Doc *doc, QObject *parent)
    : VCWidget(doc, parent)
    , m_clocktype(Clock)
    , m_targetTime(0)
{
    setType(VCWidget::ClockWidget);
    setForegroundColor(Qt::white);
    QFont wFont = font();
    wFont.setBold(true);
    wFont.setPointSize(28);
    setFont(wFont);
}

VCClock::~VCClock()
{
    int schNum = m_scheduleList.count();
    for (int i = 0; i < schNum; i++)
    {
        VCClockSchedule *sch = m_scheduleList.takeLast();
        delete sch;
    }
}

void VCClock::setID(quint32 id)
{
    VCWidget::setID(id);

    if (caption().isEmpty())
        setCaption(tr("Clock %1").arg(id));
}

void VCClock::render(QQuickView *view, QQuickItem *parent)
{
    if (view == NULL || parent == NULL)
        return;

    QQmlComponent *component = new QQmlComponent(view->engine(), QUrl("qrc:/VCClockItem.qml"));

    if (component->isError())
    {
        qDebug() << component->errors();
        return;
    }

    QQuickItem *item = qobject_cast<QQuickItem*>(component->create());

    item->setParentItem(parent);
    item->setProperty("clockObj", QVariant::fromValue(this));
}

QString VCClock::propertiesResource() const
{
    return QString("qrc:/VCClockProperties.qml");
}

/*********************************************************************
 * Type
 *********************************************************************/

void VCClock::setClockType(VCClock::ClockType type)
{
    if (type == m_clocktype)
        return;

    m_clocktype = type;
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

/*********************************************************************
 * Functions scheduling
 *********************************************************************/

QVariantList VCClock::scheduleList()
{
    QVariantList list;
    foreach(VCClockSchedule *sch, m_scheduleList)
        list.append(QVariant::fromValue(sch));
    return list;
}

void VCClock::addSchedule(VCClockSchedule *schedule)
{
    if (schedule->functionID() != Function::invalidId())
        m_scheduleList.append(schedule);
    qSort(m_scheduleList);
}

void VCClock::removeSchedule(int index)
{
    if (index < 0 || index > m_scheduleList.count())
        return;

    VCClockSchedule *sch = m_scheduleList.takeAt(index);
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

    if (attrs.hasAttribute(KXMLQLCVCClockType))
    {
        setClockType(stringToType(attrs.value(KXMLQLCVCClockType).toString()));
        if (clockType() == Countdown)
        {
            int msTime = 0;

            if (attrs.hasAttribute(KXMLQLCVCClockTime))
            {
                QDateTime tTime;
                tTime.setTime(QTime::fromString(attrs.value(KXMLQLCVCClockTime).toString(), "HH:mm:ss"));
                msTime = (tTime.time().hour() * 60 * 60 * 1000) + (tTime.time().minute() * 60 * 1000) + (tTime.time().second() * 1000);
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
    Q_ASSERT(doc != NULL);

    /* VC Clock entry */
    doc->writeStartElement(KXMLQLCVCClock);

    /* Type */
    ClockType type = clockType();
    doc->writeAttribute(KXMLQLCVCClockType, typeToString(type));
    if (type == Countdown)
    {
        QDateTime tTime;
        tTime.addSecs(targetTime() / 1000);
        doc->writeAttribute(KXMLQLCVCClockTime, tTime.time().toString());
    }

#if 0 // TODO
    saveXMLCommon(doc);

    /* Window state */
    saveXMLWindowState(doc);

    /* Appearance */
    saveXMLAppearance(doc);
#endif

    foreach(VCClockSchedule *sch, m_scheduleList)
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

        if (attrs.hasAttribute(KXMLQLCVCClockScheduleTime))
        {
            QDateTime start;
            start.setTime(QTime::fromString(attrs.value(KXMLQLCVCClockScheduleTime).toString(), "HH:mm:ss"));
            setStartTime((start.time().hour() * 60 * 60) + (start.time().minute() * 60) + start.time().second());
        }
        if (attrs.hasAttribute(KXMLQLCVCClockScheduleStopTime))
        {
            QDateTime stop;
            stop.setTime(QTime::fromString(attrs.value(KXMLQLCVCClockScheduleStopTime).toString(), "HH:mm:ss"));
            setStopTime((stop.time().hour() * 60 * 60) + (stop.time().minute() * 60) + stop.time().second());
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

    /* Schedule start time */
    QDateTime start;
    start.addSecs(startTime());
    doc->writeAttribute(KXMLQLCVCClockScheduleStartTime, start.time().toString());

    if (stopTime() != -1)
    {
        /* Schedule stop time */
        QDateTime stop;
        stop.addSecs(startTime());
        doc->writeAttribute(KXMLQLCVCClockScheduleStopTime, stop.time().toString());
    }
    if (weekFlags() != 0)
        doc->writeAttribute(KXMLQLCVCClockScheduleWeekFlags, QString::number(weekFlags()));

    doc->writeEndElement();

    return true;
}
