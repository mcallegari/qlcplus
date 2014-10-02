/*
  Q Light Controller
  monitorlayout.h

  Copyright (c) Nokia Corporation/QtSoftware
		Heikki Junnila

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

#ifndef MONITORLAYOUT_H
#define MONITORLAYOUT_H

#include <QWidgetItem>
#include <QLayout>
#include <QRect>

class MonitorFixture;

/** \addtogroup ui_mon DMX Monitor
 * @{
 */

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

/** @} */

#endif
