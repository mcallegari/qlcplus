/*
  Q Light Controller Plus
  remapwidget.cpp

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

#include <QTreeWidgetItem>
#include <QPainter>
#include <QDebug>

#include "fixtureremap.h"
#include "remapwidget.h"

RemapWidget::RemapWidget(QTreeWidget *src, QTreeWidget *target, QWidget *parent)
    : QWidget(parent)
    , m_sourceTree(src)
    , m_targetTree(target)
{
    setMaximumWidth(100);
}

RemapWidget::~RemapWidget()
{
}

void RemapWidget::setRemapList(QList<RemapInfo> list)
{
    m_list = list;
    update();
}

void RemapWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
}

void RemapWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)

    QPainter painter(this);
    painter.setBrush(QBrush(Qt::white));
    painter.drawRect(0, 0, width(), height());
    painter.setPen(QPen(Qt::black));

    int yOffset = m_sourceTree->header()->height() + 10;

    foreach(RemapInfo info, m_list)
    {
        QRect srcRect = m_sourceTree->visualItemRect(info.source);
        QRect tgtRect = m_targetTree->visualItemRect(info.target);

        if (srcRect.isEmpty())
            srcRect = m_sourceTree->visualItemRect(info.source->parent());

        if (tgtRect.isEmpty())
            tgtRect = m_targetTree->visualItemRect(info.target->parent());

        //qDebug() << "Source rect:" << srcRect << ", target rect:" << tgtRect;

        painter.drawLine(0, srcRect.y() + yOffset, 10, srcRect.y() + yOffset);
        painter.drawLine(10, srcRect.y() + yOffset, 90, tgtRect.y() + yOffset);
        painter.drawLine(90, tgtRect.y() + yOffset, 100, tgtRect.y() + yOffset);
    }
}
