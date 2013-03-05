/*
  Q Light Controller Plus
  clickandgowidget.h

  Copyright (c) Massimo Callegari

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef CLICKANDGOWIDGET_H
#define CLICKANDGOWIDGET_H

#include <QWidget>

class ClickAndGoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ClickAndGoWidget(QWidget *parent = 0);

    void setType(int type);
    int getType();

    QColor getColorAt(uchar pos);
    
protected:
    void setupGradient(QColor end);

protected:
    /** The Click And Go type. Essential for the whole widget behaviour */
    int m_type;

    /** Used to group all the primary colors */
    bool m_linearColor;

    /** Pre-rendered bitmap with the UI displayed by ClickAndGoAction */
    QImage m_image;

protected:
    QSize sizeHint() const;
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

signals:
    void levelChanged(uchar level);
    void colorChanged(uchar red, uchar green, uchar blue);
    
public slots:
    
};

#endif // CLICKANDGOWIDGET_H
