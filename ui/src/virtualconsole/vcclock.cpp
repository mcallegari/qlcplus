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

#include "vcclockproperties.h"
#include "vcclock.h"
#include "doc.h"

#define HYSTERESIS 3 // Hysteresis for pause/reset external input

#define KXMLQLCVCClockType      QString("Type")
#define KXMLQLCVCClockHours     QString("Hours")
#define KXMLQLCVCClockMinutes   QString("Minutes")
#define KXMLQLCVCClockSeconds   QString("Seconds")

#define KXMLQLCVCClockSchedule      QString("Schedule")
#define KXMLQLCVCClockScheduleFunc  QString("Function")
#define KXMLQLCVCClockScheduleTime  QString("Time")

#define KXMLQLCVCClockPlay  QString("PlayPause")
#define KXMLQLCVCClockReset QString("Reset")

const quint8 VCClock::playInputSourceId = 0;
const quint8 VCClock::resetInputSourceId = 1;

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
        m_scheduleIndex = -1;

        if (m_scheduleList.count() > 0)
        {
            QTime currTime = QDateTime::currentDateTime().time();

            // find the index of the next scheduled event to run
            for (int i = 0; i < m_scheduleList.count(); i++)
            {
                VCClockSchedule sch = m_scheduleList.at(i);
                if (sch.time().time() >= currTime)
                {
                    m_scheduleIndex = i;
                    qDebug() << "VC Clock set to play index:" << i;
                    break;
                }
            }
            // if no event is found after the current time, it means the next schedule 
            // will happen the day after so it's the first in the list
            if (m_scheduleIndex == -1)
                m_scheduleIndex = 0;
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
    updateFeedback();
    update();
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

void VCClock::addSchedule(VCClockSchedule schedule)
{
    qDebug() << Q_FUNC_INFO << "--- ID:" << schedule.function() << ", time:" << schedule.time().time().toString();
    if (schedule.function() != Function::invalidId())
        m_scheduleList.append(schedule);
    std::sort(m_scheduleList.begin(), m_scheduleList.end());
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

            emit timeChanged(m_currentTime);
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
    updateFeedback();
    update();
}

void VCClock::resetTimer()
{
    if (clockType() == Stopwatch)
        m_currentTime = 0;
    else if (clockType() == Countdown)
        m_currentTime = m_targetTime;

    emit timeChanged(m_currentTime);

    updateFeedback();
    update();
}

void VCClock::playPauseTimer()
{
    if (clockType() == Stopwatch || clockType() == Countdown)
        m_isPaused = !m_isPaused;

    updateFeedback();
    update();
}

/*****************************************************************************
 * Key Sequences
 *****************************************************************************/

void VCClock::setPlayKeySequence(const QKeySequence& keySequence)
{
    m_playKeySequence = QKeySequence(keySequence);
}

QKeySequence VCClock::playKeySequence() const
{
    return m_playKeySequence;
}

void VCClock::setResetKeySequence(const QKeySequence& keySequence)
{
    m_resetKeySequence = QKeySequence(keySequence);
}

QKeySequence VCClock::resetKeySequence() const
{
    return m_resetKeySequence;
}

void VCClock::slotKeyPressed(const QKeySequence& keySequence)
{
    if (acceptsInput() == false)
        return;

    if (m_playKeySequence == keySequence)
        playPauseTimer();
    else if (m_resetKeySequence == keySequence)
        resetTimer();
}

void VCClock::updateFeedback()
{
    if (clockType() == Stopwatch)
    {
        sendFeedback(!m_isPaused ? UCHAR_MAX : 0, playInputSourceId);
        sendFeedback(m_currentTime == 0 ? UCHAR_MAX : 0, resetInputSourceId);
    }
    else if (clockType() == Countdown)
    {
        sendFeedback(!m_isPaused ? UCHAR_MAX : 0, playInputSourceId);
        sendFeedback(m_currentTime == m_targetTime ? UCHAR_MAX : 0, resetInputSourceId);
    }
    else
    {
        sendFeedback(0, playInputSourceId);
        sendFeedback(0, resetInputSourceId);
    }
}

/*****************************************************************************
 * External Input
 *****************************************************************************/

void VCClock::slotInputValueChanged(quint32 universe, quint32 channel, uchar value)
{
    /* Don't let input data through in design mode or if disabled */
    if (acceptsInput() == false)
        return;

    quint32 pagedCh = (page() << 16) | channel;

    if (checkInputSource(universe, pagedCh, value, sender(), playInputSourceId))
    {
        // Use hysteresis for values, in case the timer is being controlled
        // by a slider. The value has to go to zero before the next non-zero
        // value is accepted as input. And the non-zero values have to visit
        // above $HYSTERESIS before a zero is accepted again.
        if (m_playLatestValue == 0 && value > 0)
        {
            playPauseTimer();
            m_playLatestValue = value;
        }
        else if (m_playLatestValue > HYSTERESIS && value == 0)
        {
            m_playLatestValue = 0;
        }

        if (value > HYSTERESIS)
            m_playLatestValue = value;
    }
    else if (checkInputSource(universe, pagedCh, value, sender(), resetInputSourceId))
    {
        // Use hysteresis for values, in case the timer is being controlled
        // by a slider. The value has to go to zero before the next non-zero
        // value is accepted as input. And the non-zero values have to visit
        // above $HYSTERESIS before a zero is accepted again.
        if (m_resetLatestValue == 0 && value > 0)
        {
            resetTimer();
            m_resetLatestValue = value;
        }
        else if (m_resetLatestValue > HYSTERESIS && value == 0)
        {
            m_resetLatestValue = 0;
        }

        if (value > HYSTERESIS)
            m_resetLatestValue = value;
    }
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

bool VCClock::copyFrom(const VCWidget* widget)
{
    const VCClock* clock = qobject_cast<const VCClock*> (widget);
    if (clock == NULL)
        return false;

    // TODO: copy schedules

    /* Clock type */
    setClockType(clock->clockType());

    /* Key sequence */
    setPlayKeySequence(clock->playKeySequence());
    setResetKeySequence(clock->resetKeySequence());

    /* Common stuff */
    return VCWidget::copyFrom(widget);
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
        else if (root.name() == KXMLQLCVCClockPlay)
        {
            QString str = loadXMLSources(root, playInputSourceId);
            if (str.isEmpty() == false)
                m_playKeySequence = stripKeySequence(QKeySequence(str));
        }else if (root.name() == KXMLQLCVCClockReset)
        {
            QString str = loadXMLSources(root, resetInputSourceId);
            if (str.isEmpty() == false)
                m_resetKeySequence = stripKeySequence(QKeySequence(str));
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

    foreach (VCClockSchedule sch, schedules())
        sch.saveXML(doc);

    if (type != Clock)
    {
        /* Play/Pause */
        doc->writeStartElement(KXMLQLCVCClockPlay);
        if (m_playKeySequence.toString().isEmpty() == false)
            doc->writeTextElement(KXMLQLCVCWidgetKey, m_playKeySequence.toString());
        saveXMLInput(doc, inputSource(playInputSourceId));
        doc->writeEndElement();

        /* Reset */
        doc->writeStartElement(KXMLQLCVCClockReset);
        if (m_resetKeySequence.toString().isEmpty() == false)
            doc->writeTextElement(KXMLQLCVCWidgetKey, m_resetKeySequence.toString());
        saveXMLInput(doc, inputSource(resetInputSourceId));
        doc->writeEndElement();
    }

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
        resetTimer();
    }
    else if (e->button() == Qt::LeftButton)
    {
        playPauseTimer();
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
