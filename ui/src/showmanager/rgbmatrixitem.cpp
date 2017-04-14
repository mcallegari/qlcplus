/*
  Q Light Controller Plus
  rgbmatrixitem.cpp

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

#include <QApplication>
#include <QPainter>
#include <qmath.h>
#include <QDebug>
#include <QMenu>

#include "rgbmatrixitem.h"
#include "trackitem.h"
#include "headeritems.h"
#include "audiodecoder.h"

RGBMatrixItem::RGBMatrixItem(RGBMatrix *rgbm, ShowFunction *func)
    : ShowItem(func)
    , m_matrix(rgbm)
{
    Q_ASSERT(rgbm != NULL);

    if (func->color().isValid())
        setColor(func->color());
    else
        setColor(ShowFunction::defaultColor(Function::RGBMatrixType));

    calculateWidth();
    connect(m_matrix, SIGNAL(changed(quint32)), this, SLOT(slotRGBMatrixChanged(quint32)));
}

void RGBMatrixItem::calculateWidth()
{
    int newWidth = 0;
    qint64 matrix_duration = m_function->duration();

    if (matrix_duration != 0)
        newWidth = ((50/(float)getTimeScale()) * (float)matrix_duration) / 1000;
    else
        newWidth = 100;

    if (newWidth < (50 / m_timeScale))
        newWidth = 50 / m_timeScale;
    setWidth(newWidth);
}

void RGBMatrixItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    float xpos = 0;
    float timeScale = 50/(float)m_timeScale;
    quint32 matrixDuration = m_matrix->totalDuration();

    ShowItem::paint(painter, option, widget);

    if (matrixDuration)
    {
        int loopCount = qFloor(m_function->duration() / matrixDuration);
        for (int i = 0; i < loopCount; i++)
        {
            xpos += ((timeScale * (float)matrixDuration) / 1000);
            // draw loop vertical delimiter
            painter->setPen(QPen(Qt::white, 1));
            painter->drawLine(xpos, 1, xpos, TRACK_HEIGHT - 5);
        }
    }

    ShowItem::postPaint(painter);
}

void RGBMatrixItem::setTimeScale(int val)
{
    ShowItem::setTimeScale(val);
    calculateWidth();
}

void RGBMatrixItem::setDuration(quint32 msec, bool stretch)
{
    if (stretch == true)
        m_matrix->setTotalDuration(msec);
    else
    {
        if (m_function)
            m_function->setDuration(msec);
        prepareGeometryChange();
        calculateWidth();
        updateTooltip();
    }
}

QString RGBMatrixItem::functionName()
{
    if (m_matrix)
        return m_matrix->name();
    return QString();
}

RGBMatrix *RGBMatrixItem::getRGBMatrix()
{
    return m_matrix;
}

void RGBMatrixItem::slotRGBMatrixChanged(quint32)
{
    prepareGeometryChange();
    if (m_function)
        m_function->setDuration(m_matrix->totalDuration());
    calculateWidth();
    updateTooltip();
}

void RGBMatrixItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *)
{
    QMenu menu;
    QFont menuFont = qApp->font();
    menuFont.setPixelSize(14);
    menu.setFont(menuFont);

    foreach(QAction *action, getDefaultActions())
        menu.addAction(action);

    menu.exec(QCursor::pos());
}
