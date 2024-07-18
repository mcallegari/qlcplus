/*
  Q Light Controller Plus
  remapwidget.cpp

  Copyright (c) Massimo Callegari

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

    foreach (RemapInfo info, m_list)
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
