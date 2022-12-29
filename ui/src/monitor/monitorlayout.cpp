/*
  Q Light Controller
  monitorlayout.cpp

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
    std::sort(m_items.begin(), m_items.end(), MonitorLayoutLessThan);
}

/****************************************************************************
 * Size & Geometry
 ****************************************************************************/

Qt::Orientations MonitorLayout::expandingDirections() const
{
    return Qt::Vertical;
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

    size += QSize(2 * contentsMargins().left(), 2 * contentsMargins().top());

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
