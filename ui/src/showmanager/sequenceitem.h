/*
  Q Light Controller Plus
  sequenceitem.h

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

#ifndef SEQUENCEITEM_H
#define SEQUENCEITEM_H

#include <QGraphicsItem>
#include <QObject>
#include <QAction>
#include <QFont>

#include "showitem.h"
#include "chaser.h"

/** @addtogroup ui_functions
 * @{
 */

/**
 *
 * Sequence Item. Clickable and draggable object identifying a chaser in sequence mode
 *
 */
class SequenceItem : public ShowItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    SequenceItem(Chaser *seq, ShowFunction *func);

    /** @reimp */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    /** @reimp */
    void setTimeScale(int val);

    /** @reimp */
    void setDuration(quint32 msec, bool stretch);

    /** @reimp */
    QString functionName();

    void setSelectedStep(int idx);

    /** Return a pointer to a Chaser Function associated to this item */
    Chaser *getChaser();

protected:
    /** @reimp */
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *);

protected slots:
    void slotSequenceChanged(quint32);

private:
    /** Calculate sequence width for paint() and boundingRect() */
    void calculateWidth();

private:
    /** Reference to the actual Chaser Function which holds the sequence steps */
    Chaser *m_chaser;

    /** index of the selected step for highlighting (-1 if none) */
    int m_selectedStep;
};

/** @} */

#endif
