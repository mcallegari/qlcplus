/*
  Q Light Controller Plus
  sequenceitem.cpp

  Copyright (C) Massimo Callegari

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

#include <QApplication>
#include <QPainter>
#include <QMenu>

#include "sequenceitem.h"
#include "headeritems.h"
#include "chaserstep.h"
#include "trackitem.h"

SequenceItem::SequenceItem(Chaser *seq)
    : ShowItem()
    , m_chaser(seq)
    , m_selectedStep(-1)
{
    Q_ASSERT(seq != NULL);

    setStartTime(m_chaser->getStartTime());
    setColor(m_chaser->getColor());
    setLocked(m_chaser->isLocked());
    setFunctionID(m_chaser->id());

    calculateWidth();

    connect(m_chaser, SIGNAL(changed(quint32)),
            this, SLOT(slotSequenceChanged(quint32)));
}

void SequenceItem::calculateWidth()
{
    int newWidth = 0;
    unsigned long seq_duration = 0;

    foreach (ChaserStep step, m_chaser->steps())
    {
        if (m_chaser->durationMode() == Chaser::Common)
            seq_duration += m_chaser->duration();
        else
            seq_duration += step.duration;
    }

    if (seq_duration != 0)
        newWidth = ((50/(float)getTimeScale()) * (float)seq_duration) / 1000;

    if (newWidth < (50 / m_timeScale))
        newWidth = 50 / m_timeScale;
    setWidth(newWidth);
}

void SequenceItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    float xpos = 0;
    float timeScale = 50/(float)m_timeScale;
    int stepIdx = 0;

    if (this->isSelected() == true)
    {
        painter->setPen(QPen(Qt::white, 3));
    }
    else
    {
        painter->setPen(QPen(Qt::white, 1));
        m_selectedStep = -1;
    }

    // draw chaser background
    painter->setBrush(QBrush(m_color));
    painter->drawRect(0, 0, m_width, 77);

    foreach (ChaserStep step, m_chaser->steps())
    {
        uint stepFadeIn = step.fadeIn;
        uint stepFadeOut = step.fadeOut;
        uint stepDuration = step.duration;
        if (m_chaser->fadeInMode() == Chaser::Common)
            stepFadeIn = m_chaser->fadeInSpeed();
        if (m_chaser->fadeOutMode() == Chaser::Common)
            stepFadeOut = m_chaser->fadeOutSpeed();
        if (m_chaser->durationMode() == Chaser::Common)
            stepDuration = m_chaser->duration();

        // draw fade in line
        if (stepFadeIn > 0)
        {
            int fadeXpos = xpos + ((timeScale * (float)stepFadeIn) / 1000);
            // doesn't even draw it if too small
            if (fadeXpos - xpos > 5)
            {
                painter->setPen(QPen(Qt::gray, 1));
                painter->drawLine(xpos, TRACK_HEIGHT - 4, fadeXpos, 1);
            }
        }
        float stepWidth = ((timeScale * (float)stepDuration) / 1000);
        // draw selected step
        if (stepIdx == m_selectedStep)
        {
            painter->setPen(QPen(Qt::yellow, 2));
            painter->setBrush(QBrush(Qt::NoBrush));
            painter->drawRect(xpos, 0, stepWidth, TRACK_HEIGHT - 3);
        }
        xpos += stepWidth;

        // draw step vertical delimiter
        painter->setPen(QPen(Qt::white, 1));
        painter->drawLine(xpos, 1, xpos, TRACK_HEIGHT - 5);

        // draw fade out line
        if (stepFadeOut > 0)
        {
            int fadeXpos = xpos + ((timeScale * (float)stepFadeOut) / 1000);
            // doesn't even draw it if too small
            if (fadeXpos - xpos > 5)
            {
                painter->setPen(QPen(Qt::gray, 1));
                painter->drawLine(xpos, 1, fadeXpos, TRACK_HEIGHT - 4);
            }
        }
        stepIdx++;
    }

    painter->setFont(m_font);
    // draw shadow
    painter->setPen(QPen(QColor(10, 10, 10, 150), 2));
    painter->drawText(QRect(4, 6, m_width - 6, 71), Qt::AlignLeft | Qt::TextWordWrap, m_chaser->name());

    // draw sequence name
    painter->setPen(QPen(QColor(220, 220, 220, 255), 2));
    painter->drawText(QRect(3, 5, m_width - 5, 72), Qt::AlignLeft | Qt::TextWordWrap, m_chaser->name());

    if (m_pressed)
    {
        quint32 s_time = 0;
        if (x() > TRACK_WIDTH)
            s_time = (double)(x() - TRACK_WIDTH) * (m_timeScale * 500) /
                     (double)(HALF_SECOND_WIDTH);
        painter->setFont(m_font);
        painter->drawText(3, TRACK_HEIGHT - 10, Function::speedToString(s_time));
    }

    if (m_locked)
        painter->drawPixmap(3, TRACK_HEIGHT >> 1, 24, 24, QIcon(":/lock.png").pixmap(24, 24));
}

void SequenceItem::setTimeScale(int val)
{
    ShowItem::setTimeScale(val);
    calculateWidth();
}

void SequenceItem::setStartTime(quint32 time)
{
    if (m_chaser == NULL)
        return;

    m_chaser->setStartTime(time);
    setToolTip(QString(tr("Name: %1\nStart time: %2\nDuration: %3\n%4"))
              .arg(m_chaser->name())
              .arg(Function::speedToString(m_chaser->getStartTime()))
              .arg(Function::speedToString(m_chaser->getDuration()))
              .arg(tr("Click to move this sequence across the timeline")));
}

quint32 SequenceItem::getStartTime()
{
    if (m_chaser)
        return m_chaser->getStartTime();
    return 0;
}

void SequenceItem::setLocked(bool locked)
{
    ShowItem::setLocked(locked);
    m_chaser->setLocked(locked);
}

void SequenceItem::setSelectedStep(int idx)
{
    m_selectedStep = idx;
    update();
}

Chaser *SequenceItem::getChaser()
{
    return m_chaser;
}

void SequenceItem::slotSequenceChanged(quint32)
{
    prepareGeometryChange();
    calculateWidth();
}

void SequenceItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *)
{
    QMenu menu;
    QFont menuFont = qApp->font();
    menuFont.setPixelSize(14);
    menu.setFont(menuFont);

    menu.addAction(m_alignToCursor);
    if (isLocked())
    {
        m_lockAction->setText(tr("Unlock item"));
        m_lockAction->setIcon(QIcon(":/unlock.png"));
    }
    else
    {
        m_lockAction->setText(tr("Lock item"));
        m_lockAction->setIcon(QIcon(":/lock.png"));
    }
    menu.addAction(m_lockAction);
    menu.exec(QCursor::pos());
}
