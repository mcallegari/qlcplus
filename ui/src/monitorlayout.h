/*
  Q Light Controller
  monitorlayout.h

  Copyright (c) Nokia Corporation/QtSoftware
		Heikki Junnila

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

#ifndef MONITORLAYOUT_H
#define MONITORLAYOUT_H

#include <QWidgetItem>
#include <QLayout>
#include <QRect>

class MonitorFixture;

/****************************************************************************
 * MonitorLayoutItem
 ****************************************************************************/

class MonitorLayoutItem : public QWidgetItem
{
public:
    MonitorLayoutItem(MonitorFixture* mof);
    ~MonitorLayoutItem();

    bool operator<(const MonitorLayoutItem& item);
};

/****************************************************************************
 * MonitorLayout
 ****************************************************************************/

class MonitorLayout : public QLayout
{
    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    MonitorLayout(QWidget *parent);
    virtual ~MonitorLayout();

    /********************************************************************
     * Items
     ********************************************************************/
public:
    void addItem(QLayoutItem* item);
    int count() const;

    MonitorLayoutItem* itemAt(int index) const;
    MonitorLayoutItem* takeAt(int index);

    void sort();

protected:
    QList <MonitorLayoutItem*> m_items;

    /********************************************************************
     * Size & Geometry
     ********************************************************************/
public:
    Qt::Orientations expandingDirections() const;
    bool hasHeightForWidth() const;
    int heightForWidth(int) const;
    QSize minimumSize() const;
    void setGeometry(const QRect& rect);
    QSize sizeHint() const;

protected:
    int doLayout(const QRect &rect, bool testOnly) const;
};

#endif
