/*
  Q Light Controller Plus
  rgbmatrixitem.h

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

#ifndef RGBMATRIXITEM_H
#define RGBMATRIXITEM_H

#include <QGraphicsItem>
#include <QObject>
#include <QAction>
#include <QFont>

#include "showitem.h"
#include "rgbmatrix.h"

/** @addtogroup ui_functions
 * @{
 */

/**
 *
 * RGBMatrix Item. Clickable and draggable object identifying a RGBMatrix object
 *
 */
class RGBMatrixItem : public ShowItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    RGBMatrixItem(RGBMatrix *rgbm, ShowFunction *func);

    /** @reimp */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    /** @reimp */
    void setTimeScale(int val);

    /** @reimp */
    void setDuration(quint32 msec, bool stretch);

    /** @reimp */
    quint32 getDuration();

    /** @reimp */
    QString functionName();

    /** Return a pointer to a RGBMatrix Function associated to this item */
    RGBMatrix *getRGBMatrix();

protected:
    /** @reimp */
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

protected slots:
    void slotRGBMatrixChanged(quint32);

private:
    /** Calculate sequence width for paint() and boundingRect() */
    void calculateWidth();

private:
    /** Reference to the actual RGBMatrix Function */
    RGBMatrix *m_matrix;
};

/** @} */

#endif
