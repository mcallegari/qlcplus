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
#include <QDateTime>
#include <QStyle>
#include <QtGui>

#include "qlcfile.h"

#include "vcclockproperties.h"
#include "virtualconsole.h"
#include "vcclock.h"
#include "doc.h"

#define KXMLQLCVCClockType "Type"
#define KXMLQLCVCClockHours "Hours"
#define KXMLQLCVCClockMinutes "Minutes"
#define KXMLQLCVCClockSeconds "Seconds"

#define KXMLQLCVCClockSchedule "Schedule"
#define KXMLQLCVCClockScheduleFunc "Function"
#define KXMLQLCVCClockScheduleTime "Time"

VCClock::VCClock(QWidget* parent, Doc* doc)
    : VCWidget(parent, doc)
    , m_clocktype(Clock)
    , m_scheduleIndex(-1)
    , m_hh(0)
    , m_mm(0)
    , m_ss(0)
    , m_targetTime(0)
    , m_currentTime(0)
    , m_isPaused(true)
{
    /* Set the class name "VCClock" as the object name as well */
    setObjectName(VCClock::staticMetaObject.className());

    setType(VCWidget::ClockWidget);
    setCaption("");
    resize(QSize(150, 50));
    QFont font = qApp->font();
    font.setBold(true);
    font.setPixelSize(28);
    setFont(font);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(slotUpdateTime()));
    timer->start(1000);
}

VCClock::~VCClock()
{
}

void VCClock::slotModeChanged(Doc::Mode mode)
{
    qDebug() << Q_FUNC_INFO;

    if (mode == Doc::Operate)
    {
        if (m_scheduleList.count() > 0)
        {
            QTime currTime = QDateTime::currentDateTime().time();
            for(int i = 0; i < m_scheduleList.count(); i++)
            {
                VCClockSchedule sch = m_scheduleList.at(i);
                if (sch.time().time() >= currTime)
                {
                    m_scheduleIndex = i;
                    qDebug() << "VC Clock set to play index:" << i;
                    break;
                }
            }
        }
    }
    VCWidget::slotModeChanged(mode);
}

/*********************************************************************
 * Type
 *********************************************************************/

void VCClock::setClockType(VCClock::ClockType type)
{
    m_clocktype = type;
    update();
}

VCClock::ClockType VCClock::clockType()
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

void VCClock::addSchedule(VCClockSchedule schedule)
{
    qDebug() << Q_FUNC_INFO << "--- ID:" << schedule.function() << ", time:" << schedule.time().time().toString();
    if (schedule.function() != Function::invalidId())
        m_scheduleList.append(schedule);
    qSort(m_scheduleList);
}

void VCClock::removeSchedule(int index)
{
    if (index < 0 || index > m_scheduleList.count())
        return;

    m_scheduleList.removeAt(index);
}

void VCClock::removeAllSchedule()
{
    m_scheduleList.clear();
}

QList<VCClockSchedule> VCClock::schedules()
{
    return m_scheduleList;
}

FunctionParent VCClock::functionParent() const
{
    return FunctionParent(FunctionParent::AutoVCWidget, id());
}

void VCClock::setCountdown(int h, int m, int s)
{
    m_hh = h;
    m_mm = m;
    m_ss = s;
    m_targetTime = (m_hh * 3600) + (m_mm * 60) + m_ss;
    m_currentTime = m_targetTime;
}

void VCClock::resetTime()
{
    if (m_clocktype == Stopwatch)
        m_currentTime = 0;
    else if (m_clocktype == Countdown)
        m_currentTime = m_targetTime;

    update();
}

void VCClock::slotUpdateTime()
{
    if (mode() == Doc::Operate)
    {
        if (m_isPaused == false)
        {
            if (m_clocktype == Stopwatch)
                m_currentTime++;
            else if (m_clocktype == Countdown && m_currentTime > 0)
                m_currentTime--;
        }
        else
        {
            if (m_clocktype == Clock)
            {
                if (m_scheduleIndex != -1 && m_scheduleIndex < m_scheduleList.count())
                {
                    QTime currTime = QDateTime::currentDateTime().time();
                    VCClockSchedule sch = m_scheduleList.at(m_scheduleIndex);
                    //qDebug() << "--- > currTime:" << currTime.toString() << ", schTime:" << sch.time().time().toString();
                    if (sch.time().time().toString() == currTime.toString())
                    {
                        quint32 fid = sch.function();
                        Function *func = m_doc->function(fid);
                        if (func != NULL)
                        {
                            func->start(m_doc->masterTimer(), functionParent());
                            qDebug() << "VC Clock starting function:" << func->name();
                        }
                        m_scheduleIndex++;
                        if (m_scheduleIndex == m_scheduleList.count())
                            m_scheduleIndex = 0;
                    }
                }
            }
        }
    }
    update();
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

VCWidget* VCClock::createCopy(VCWidget* parent)
{
    Q_ASSERT(parent != NULL);

    VCClock* clock = new VCClock(parent, m_doc);
    if (clock->copyFrom(this) == false)
    {
        delete clock;
        clock = NULL;
    }

    return clock;
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCClock::editProperties()
{
    VCClockProperties vccp(this, m_doc);
    if (vccp.exec() == QDialog::Rejected)
        return;

    m_doc->setModified();
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

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
            int h = 0, m = 0, s = 0;
            if (attrs.hasAttribute(KXMLQLCVCClockHours))
                h = attrs.value(KXMLQLCVCClockHours).toString().toInt();
            if (attrs.hasAttribute(KXMLQLCVCClockMinutes))
                m = attrs.value(KXMLQLCVCClockMinutes).toString().toInt();
            if (attrs.hasAttribute(KXMLQLCVCClockSeconds))
                s = attrs.value(KXMLQLCVCClockSeconds).toString().toInt();
            setCountdown(h, m ,s);
        }
    }

    /* Widget commons */
    loadXMLCommon(root);

    /* Children */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCWindowState)
        {
            int x = 0, y = 0, w = 0, h = 0;
            bool visible = false;
            loadXMLWindowState(root, &x, &y, &w, &h, &visible);
            setGeometry(x, y, w, h);
        }
        else if (root.name() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(root);
        }
        else if (root.name() == KXMLQLCVCClockSchedule)
        {
            VCClockSchedule sch;
            if (sch.loadXML(root) == true)
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
        doc->writeAttribute(KXMLQLCVCClockHours, QString::number(getHours()));
        doc->writeAttribute(KXMLQLCVCClockMinutes, QString::number(getMinutes()));
        doc->writeAttribute(KXMLQLCVCClockSeconds, QString::number(getSeconds()));
    }

    saveXMLCommon(doc);

    /* Window state */
    saveXMLWindowState(doc);

    /* Appearance */
    saveXMLAppearance(doc);

    foreach(VCClockSchedule sch, schedules())
        sch.saveXML(doc);

    /* End the <Clock> tag */
    doc->writeEndElement();

    return true;
}

/****************************************************************************
 * Drawing
 ****************************************************************************/

void VCClock::paintEvent(QPaintEvent* e)
{
    QPainter painter(this);

    if (clockType() == Clock)
    {
        QDateTime currTime = QDateTime::currentDateTime();
        style()->drawItemText(&painter, rect(), Qt::AlignCenter | Qt::TextWordWrap, palette(),
                              true, currTime.time().toString(), foregroundRole());
    }
    else
    {
        quint32 secTime = m_currentTime;
        uint h, m;

        h = secTime / 3600;
        secTime -= (h * 3600);

        m = secTime / 60;
        secTime -= (m * 60);
        style()->drawItemText(&painter, rect(), Qt::AlignCenter | Qt::TextWordWrap, palette(),
                              true, QString("%1:%2:%3").arg(h, 2, 10, QChar('0'))
                              .arg(m, 2, 10, QChar('0')).arg(secTime, 2, 10, QChar('0')), foregroundRole());
    }
    painter.end();

    VCWidget::paintEvent(e);
}

void VCClock::mousePressEvent(QMouseEvent *e)
{
    if (mode() == Doc::Design)
    {
        VCWidget::mousePressEvent(e);
        return;
    }

    if (e->button() == Qt::RightButton)
    {
        if (clockType() == Stopwatch)
            m_currentTime = 0;
        else if (clockType() == Countdown)
            m_currentTime = m_targetTime;
        update();
    }
    else if (e->button() == Qt::LeftButton)
    {
        if (clockType() == Stopwatch || clockType() == Countdown)
            m_isPaused = !m_isPaused;

        update();
    }
    VCWidget::mousePressEvent(e);
}

/*********************************************************************
 * VCClockSchedule Class methods
 *********************************************************************/

bool VCClockSchedule::operator <(const VCClockSchedule &sch) const
{
    if (sch.time() < time())
        return false;
    return true;
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
        setFunction(attrs.value(KXMLQLCVCClockScheduleFunc).toString().toUInt());
        if (attrs.hasAttribute(KXMLQLCVCClockScheduleTime))
        {
            QDateTime dt;
            dt.setTime(QTime::fromString(attrs.value(KXMLQLCVCClockScheduleTime).toString(), "HH:mm:ss"));
            setTime(dt);
        }
    }
    root.skipCurrentElement();

    return true;
}

bool VCClockSchedule::saveXML(QXmlStreamWriter *doc)
{
    /* Schedule tag */
    doc->writeStartElement(KXMLQLCVCClockSchedule);

    /* Schedule function */
    doc->writeAttribute(KXMLQLCVCClockScheduleFunc, QString::number(function()));
    /* Schedule time */
    doc->writeAttribute(KXMLQLCVCClockScheduleTime, time().time().toString());

    doc->writeEndElement();

    return true;
}
