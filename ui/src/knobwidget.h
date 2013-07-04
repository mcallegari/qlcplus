/*
  Q Light Controller Plus
  knobwidget.h

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

#ifndef KNOBWIDGET_H
#define KNOBWIDGET_H

#include <QDial>

class KnobWidget : public QDial
{
    Q_OBJECT
public:
    /** Constructor */
    KnobWidget(QWidget *parent = 0);

    /** Destructor */
    ~KnobWidget();

    void setEnabled(bool);

protected:
    void prepareCursor();

    void resizeEvent(QResizeEvent *e);
    void paintEvent(QPaintEvent* e);

private:
    QPixmap rotatePix(QPixmap *p_pix, float p_deg);
private:
    QPixmap* m_background;
    QPixmap* m_cursor;
};

#endif
