/*
  Q Light Controller
  monitorlayout.cpp

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

#include <QWidgetItem>
#include <QLayout>
#include <QDebug>

#include "monitorfixture.h"
#include "monitorlayout.h"

/****************************************************************************
 * MonitorLayoutItem
 ****************************************************************************/

MonitorLayoutItem::MonitorLayoutItem(MonitorFixture* mof) : QWidgetItem(mof)
{
}

MonitorLayoutItem::~MonitorLayoutItem()
{
}

bool MonitorLayoutItem::operator<(const MonitorLayoutItem& item)
{
    MonitorLayoutItem& ncitem = const_cast<MonitorLayoutItem&> (item);
    MonitorFixture* item_mof;
    MonitorFixture* mof;

    mof = qobject_cast<MonitorFixture*> (widget());
    Q_ASSERT(mof != NULL);

    item_mof = qobject_cast<MonitorFixture*> (ncitem.widget());
    Q_ASSERT(item_mof != NULL);

    if ((*mof) < (*item_mof))
        return true;
    else
        return false;
}

/****************************************************************************
 * Initialization
 ****************************************************************************/

MonitorLayout::MonitorLayout(QWidget *parent) : QLayout(parent)
{
}

MonitorLayout::~MonitorLayout()
{
    while (m_items.isEmpty() == false)
        delete m_items.takeFirst();
}

/****************************************************************************
 * Items
 ****************************************************************************/

void MonitorLayout::addItem(QLayoutItem* item)
{
    m_items.append(static_cast<MonitorLayoutItem*> (item));
    sort();
    update();
}

int MonitorLayout::count() const
{
    return m_items.size();
}

MonitorLayoutItem* MonitorLayout::itemAt(int index) const
{
    return m_items.value(index);
}

MonitorLayoutItem* MonitorLayout::takeAt(int index)
{
    if (index >= 0 && index < m_items.size())
        return m_items.takeAt(index);
    else
        return NULL;
}

static bool MonitorLayoutLessThan(MonitorLayoutItem* i1, MonitorLayoutItem* i2)
{
    if ((*i1) < (*i2))
        return true;
    else
        return false;
}

void MonitorLayout::sort()
{
    qSort(m_items.begin(), m_items.end(), MonitorLayoutLessThan);
}

/****************************************************************************
 * Size & Geometry
 ****************************************************************************/

Qt::Orientations MonitorLayout::expandingDirections() const
{
    return 0;
}

bool MonitorLayout::hasHeightForWidth() const
{
    return true;
}

int MonitorLayout::heightForWidth(int width) const
{
    int height = doLayout(QRect(0, 0, width, 0), true);
    return height;
}

void MonitorLayout::setGeometry(const QRect& rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

QSize MonitorLayout::sizeHint() const
{
    return minimumSize();
}

QSize MonitorLayout::minimumSize() const
{
    QSize size;
    QLayoutItem* item;

    foreach (item, m_items)
    size = size.expandedTo(item->minimumSize());

    size += QSize(2 * margin(), 2 * margin());

    return size;
}

int MonitorLayout::doLayout(const QRect& rect, bool testOnly) const
{
    int x = rect.x();
    int y = rect.y();
    int lineHeight = 0;
    QLayoutItem* item;

    foreach (item, m_items)
    {
        int nextX = x + item->sizeHint().width() + spacing();
        if (nextX - spacing() > rect.right() && lineHeight > 0)
        {
            x = rect.x();
            y = y + lineHeight + spacing();
            nextX = x + item->sizeHint().width() + spacing();
            lineHeight = 0;
        }

        if (testOnly == false)
        {
            item->setGeometry(QRect(QPoint(x, y),
                                    item->sizeHint()));
        }

        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }

    return y + lineHeight - rect.y();
}
