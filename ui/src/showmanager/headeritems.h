/*
  Q Light Controller Plus
  headeritems.h

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

#ifndef HEADERITEM_H
#define HEADERITEM_H

#include <QGraphicsItem>
#include <QObject>
#include <QAction>
#include <QFont>

#include "show.h"

#define HEADER_HEIGHT       35
#define HALF_SECOND_WIDTH   25

/** @addtogroup ui_functions
 * @{
 */

/**
 *
 * Show Manager Header class. Clickable time line header
 *
 */

class ShowHeaderItem :  public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    ShowHeaderItem(int width);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void setTimeScale(int val);
    int getTimeScale();

    void setTimeDivisionType(Show::TimeDivision type);
    Show::TimeDivision getTimeDivisionType();
    void setBPMValue(int value);

    int getHalfSecondWidth();
    float getTimeDivisionStep();

    void setWidth(int);
    void setHeight(int);

signals:
    void itemClicked(QGraphicsSceneMouseEvent *);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

private:
    /** Total width of the item */
    int m_width;
    /** Total height of the item */
    int m_height;
    /** Distance in pixels between the time division bars */
    float m_timeStep;
    /** Divisor of the time division hit bar (the highest bar) */
    char m_timeHit;
    /** Scale of the time division */
    int m_timeScale;
    /** When BPM mode is active, this holds the number of BPM to display */
    int m_BPMValue;
    /** The type of time division */
    Show::TimeDivision m_type;
};

/**
 *
 * Show Manager Cursor class. Cursor which marks the time position in a scene
 *
 */
class ShowCursorItem : public QGraphicsItem
{
public:
    ShowCursorItem(int h);

    void setHeight(int height);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void setTime(quint32 t);
    quint32 getTime();
private:
    int m_height;
    quint32 m_time;
};

/** @} */

#endif
