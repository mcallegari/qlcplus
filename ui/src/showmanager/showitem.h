/*
  Q Light Controller Plus
  showitem.h

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


#ifndef SHOWITEM_H
#define SHOWITEM_H

#include <QGraphicsItem>
#include <QObject>
#include <QAction>
#include <QFont>

/** @addtogroup ui_functions
 * @{
 */

class ShowItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    ShowItem(QObject *parent = 0);

    virtual void setTimeScale(int val);
    virtual int getTimeScale();

    virtual void setStartTime(quint32 time) = 0;
    virtual quint32 getStartTime() = 0;

    virtual void setWidth(int w);
    virtual int getWidth();

    virtual QPointF getDraggingPos();

    virtual void setTrackIndex(int idx);
    virtual int getTrackIndex();

    virtual void setColor(QColor col);
    virtual QColor getColor();

    virtual void setLocked(bool locked);
    virtual bool isLocked();

    virtual void setFunctionID(quint32 id);
    virtual quint32 getFunctionID();

    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected slots:
    void slotAlignToCursorClicked();
    void slotLockItemClicked();

signals:
    void itemDropped(QGraphicsSceneMouseEvent *, ShowItem *);
    void alignToCursor(ShowItem *);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

protected:
    /** Font used for the item's labels */
    QFont m_font;

    /** The item background color */
    QColor m_color;

    /** Locked state of the item */
    bool m_locked;

    /** State flag that keeps if the item is pressed by mouse */
    bool m_pressed;

    /** Width of the item in pixels */
    int m_width;

    /** Position of the item top-left corner. This is used to handle unwanted dragging */
    QPointF m_pos;

    /** Horizontal scale to adapt width to the current time line */
    int m_timeScale;

    /** Track index this item belongs to */
    int m_trackIdx;

    /** The Function ID associated to this item */
    quint32 m_functionID;

    /** Contextual menu actions */
    QAction *m_alignToCursor;
    QAction *m_lockAction;
};

/** @} */

#endif
