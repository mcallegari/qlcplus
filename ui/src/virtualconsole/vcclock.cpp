/*
  Q Light Controller
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

#include <QtXml>
#include <QtGui>
#include <QStyle>
#include <QDateTime>

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

    setType(VCWidget::LabelWidget);
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
                            func->start(m_doc->masterTimer());
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

bool VCClock::loadXML(const QDomElement* root)
{
    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCVCClock)
    {
        qWarning() << Q_FUNC_INFO << "Clock node not found";
        return false;
    }

    if (root->hasAttribute(KXMLQLCVCClockType))
    {
        setClockType(stringToType(root->attribute(KXMLQLCVCClockType)));
        if (clockType() == Countdown)
        {
            int h = 0, m = 0, s = 0;
            if (root->hasAttribute(KXMLQLCVCClockHours))
                h = root->attribute(KXMLQLCVCClockHours).toInt();
            if (root->hasAttribute(KXMLQLCVCClockMinutes))
                m = root->attribute(KXMLQLCVCClockMinutes).toInt();
            if (root->hasAttribute(KXMLQLCVCClockSeconds))
                s = root->attribute(KXMLQLCVCClockSeconds).toInt();
            setCountdown(h, m ,s);
        }
    }

    /* Widget commons */
    loadXMLCommon(root);

    /* Children */
    QDomNode node = root->firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCWindowState)
        {
            int x = 0, y = 0, w = 0, h = 0;
            bool visible = false;
            loadXMLWindowState(&tag, &x, &y, &w, &h, &visible);
            setGeometry(x, y, w, h);
        }
        else if (tag.tagName() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(&tag);
        }
        else if (tag.tagName() == KXMLQLCVCClockSchedule)
        {
            VCClockSchedule sch;
            if (sch.loadXML(&tag) == true)
                addSchedule(sch);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown clock tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool VCClock::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
    QDomElement root;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(vc_root != NULL);

    /* VC Clock entry */
    root = doc->createElement(KXMLQLCVCClock);
    vc_root->appendChild(root);

    /* Type */
    ClockType type = clockType();
    root.setAttribute(KXMLQLCVCClockType, typeToString(type));
    if (type == Countdown)
    {
        root.setAttribute(KXMLQLCVCClockHours, getHours());
        root.setAttribute(KXMLQLCVCClockMinutes, getMinutes());
        root.setAttribute(KXMLQLCVCClockSeconds, getSeconds());
    }

    saveXMLCommon(doc, &root);

    /* Window state */
    saveXMLWindowState(doc, &root);

    /* Appearance */
    saveXMLAppearance(doc, &root);

    foreach(VCClockSchedule sch, schedules())
        sch.saveXML(doc, &root);

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

bool VCClockSchedule::loadXML(const QDomElement *root)
{
    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCVCClockSchedule)
    {
        qWarning() << Q_FUNC_INFO << "Clock Schedule node not found";
        return false;
    }

    if (root->hasAttribute(KXMLQLCVCClockScheduleFunc))
    {
        setFunction(root->attribute(KXMLQLCVCClockScheduleFunc).toUInt());
        if (root->hasAttribute(KXMLQLCVCClockScheduleTime))
        {
            QDateTime dt;
            dt.setTime(QTime::fromString(root->attribute(KXMLQLCVCClockScheduleTime), "HH:mm:ss"));
            setTime(dt);
        }
    }

    return true;
}

bool VCClockSchedule::saveXML(QDomDocument *doc, QDomElement *root)
{
    QDomElement tag;

    /* Schedule tag */
    tag = doc->createElement(KXMLQLCVCClockSchedule);
    root->appendChild(tag);

    /* Schedule function */
    tag.setAttribute(KXMLQLCVCClockScheduleFunc, function());
    /* Schedule time */
    tag.setAttribute(KXMLQLCVCClockScheduleTime, time().time().toString());

    return true;
}
