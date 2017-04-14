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

SequenceItem::SequenceItem(Chaser *seq, ShowFunction *func)
    : ShowItem(func)
    , m_chaser(seq)
    , m_selectedStep(-1)
{
    Q_ASSERT(seq != NULL);

    if (func->color().isValid())
        setColor(func->color());
    else
        setColor(ShowFunction::defaultColor(Function::ChaserType));

    if (func->duration() == 0)
        func->setDuration(seq->totalDuration());

    calculateWidth();

    connect(m_chaser, SIGNAL(changed(quint32)),
            this, SLOT(slotSequenceChanged(quint32)));
}

void SequenceItem::calculateWidth()
{
    int newWidth = 0;
    unsigned long seq_duration = m_chaser->totalDuration();

    if (seq_duration != 0)
        newWidth = ((50/(float)getTimeScale()) * (float)seq_duration) / 1000;

    if (newWidth < (50 / m_timeScale))
        newWidth = 50 / m_timeScale;
    setWidth(newWidth);
}

void SequenceItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    float xpos = 0;
    float timeScale = 50/(float)m_timeScale;
    int stepIdx = 0;

    ShowItem::paint(painter, option, widget);

    if (this->isSelected() == false)
        m_selectedStep = -1;

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

    ShowItem::postPaint(painter);
}

void SequenceItem::setTimeScale(int val)
{
    ShowItem::setTimeScale(val);
    calculateWidth();
}

void SequenceItem::setDuration(quint32 msec, bool stretch)
{
    Q_UNUSED(stretch)
    m_chaser->setTotalDuration(msec);
}

QString SequenceItem::functionName()
{
    if (m_chaser)
        return m_chaser->name();
    return QString();
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
    if (m_function)
        m_function->setDuration(m_chaser->totalDuration());
    updateTooltip();
}

void SequenceItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *)
{
    QMenu menu;
    QFont menuFont = qApp->font();
    menuFont.setPixelSize(14);
    menu.setFont(menuFont);

    foreach(QAction *action, getDefaultActions())
        menu.addAction(action);

    menu.exec(QCursor::pos());
}
