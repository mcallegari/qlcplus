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

class ShowFunction;

/** @addtogroup ui_functions
 * @{
 */

/**
 * @brief The ShowItem class is the base class for the Show Manager
 * items rendered in the multitrack view.
 * It holds the common properties of an item and, where necessary,
 * it defines some pure virtual methods specific to a subclass.
 * For example the function name or the item start time are
 * Function-dependent so they have to be implemented in the subclass.
 *
 * The paint mechanism, unless completely overridden by the subclass,
 * should happen in 3 stages:
 * 1- ShowItem::paint: paints the basic item rectangle and set the item font
 * 2- Subclass::paint: paints the item's specific contents (e.g. preview, steps, etc..)
 * 3- ShowItem::postPaint: paints the "overlay" information such as the function name,
 *                         the lock icon and the dragging time
 */

class ShowItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    ShowItem(ShowFunction *function, QObject *parent = 0);

protected:
    /**
     * @brief updateTooltip update the item's tooltip with the latest
     * timing information
     */
    void updateTooltip();

    /**
     * @brief getDefaultActions get the list of the default actions
     *        that can be performed on an item
     * @return a QList of QAction pointers
     */

    QList<QAction *> getDefaultActions();

public:

    /**
     * @brief setTimeScale sets the item time scale as selected in the Show Manager
     *
     * @param val the timescale value (range from 1 to 10)
     */
    virtual void setTimeScale(int val);

    /**
     * @brief getTimeScale get the item time scale
     *
     * @return the item time scale value
     */
    virtual int getTimeScale();

    /**
     * @brief setStartTime virtual method to set the item start time.
     *        This method doesn't update the item graphic position, so
     *        Show Manager is in charge of doing that
     *
     * @param time the start time in milliseconds
     */
    virtual void setStartTime(quint32 time);

    /**
     * @brief getStartTime virtual method that returns the item start time
     *
     * @return the item start time in milliseconds
     */
    virtual quint32 getStartTime();

    /**
     * @brief setDuration virtual method to set the item's total duration
     *        A subclass should reimplement this and update the item's width
     *
     * @param msec the item duration in milliseconds
     */
    virtual void setDuration(quint32 msec, bool stretch);

    /**
     * @brief getDuration virtual method that returns the item total duration
     * @return the item duration in milliseconds
     */
    virtual quint32 getDuration();

    /**
     * @brief setWidth set the item width for rendering
     *
     * @param w the item width in pixels
     */
    virtual void setWidth(int w);

    /**
     * @brief getWidth get the current item's width
     *
     * @return the item's width in pixels
     */
    virtual int getWidth();

    /**
     * @brief getDraggingPos returns the item position during a dragging event
     *
     * @return the dragging position as float point
     */
    virtual QPointF getDraggingPos();

    /**
     * @brief setTrackIndex set the multitrack track index of the item
     *
     * @param idx the track index
     */
    virtual void setTrackIndex(int idx);

    /**
     * @brief getTrackIndex get the current index of the track this item belongs to
     *
     * @return the track index
     */
    virtual int getTrackIndex();

    /**
     * @brief setColor set the item background color
     *
     * @param col the item RGB color
     */
    virtual void setColor(QColor col);

    /**
     * @brief getColor get the item's background color
     *
     * @return the current background RGB color
     */
    virtual QColor getColor();

    /**
     * @brief setLocked set the item lock state. When locked the item
     * will be overlaid with a locker icon
     *
     * @param locked boolean lock state
     */
    virtual void setLocked(bool locked);

    /**
     * @brief isLocked return if the item is locked or not
     *
     * @return boolean lock state
     */
    virtual bool isLocked();

    /**
     * @brief setFunctionID set the QLC+ Function ID associated to this item
     *
     * @param id the Function unique ID
     */
    virtual void setFunctionID(quint32 id);

    /**
     * @brief getFunctionID return the item's associated Function ID
     *
     * @return the Function ID
     */
    virtual quint32 functionID();

    /**
     * @brief showFunction return the item's associate ShowFunction
     * @return the ShowFuntion pointer
     */
    ShowFunction *showFunction() const;

    /**
     * @brief functionName pure virtual method that returns the item's associated
     *        Function name
     *
     * @return the Function name as a string
     */
    virtual QString functionName();

    /**
     * @brief boundingRect overridden method that returns the item bounding rectangle.
     *        This is very important for the item's drag and drop and selection
     *
     * @return the item bounding rectangle
     */
    virtual QRectF boundingRect() const;

    /**
     * @brief paint overridden method to paint the item's basic elements such as the
     * background rectangle and selection state
     */
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    /**
     * @brief postPaint method to be called to paint the "overlay" elements after a subclass
     * has painted its specific item's contents
     */
    virtual void postPaint(QPainter *painter);

protected slots:
    /**
     * @brief slotAlignToCursorClicked slot called when the user requests to align the item
     * to the current poisition of the Show Manager header cursor
     */
    void slotAlignToCursorClicked();

    /**
     * @brief slotLockItemClicked slot called when the user requests to lock the item
     * to its current time position
     */
    void slotLockItemClicked();

signals:
    /**
     * @brief itemDropped signal emitted when the user drops and item after dragging it
     */
    void itemDropped(QGraphicsSceneMouseEvent *, ShowItem *);

    /**
     * @brief alignToCursor signal emitted to request the item alignment to the
     * Show Manager header cursor
     */
    void alignToCursor(ShowItem *);

protected:
    /**
     * @brief mousePressEvent overridden method to handle the mouse pressure over the item.
     * This method stores the starting position of a dragging event, to be used later
     * to restore the item position if the drag is not valid
     */
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

    /**
     * @brief mouseReleaseEvent overridden method to handle the mouse release event over an item.
     * This method emits the itemDropped signal to be handled by the above layers
     */
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    /**
     * @brief contextMenuEvent overridden method to handle the mouse right click over an item
     * and request the display of a contextual menu.
     */
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

    /** The ShowFunction associated to this item */
    ShowFunction *m_function;

    /** Contextual menu actions */
    QAction *m_alignToCursor;
    QAction *m_lockAction;
};

/** @} */

#endif
