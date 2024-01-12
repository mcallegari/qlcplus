/*
  Q Light Controller Plus
  efxitem.cpp

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
#include <qmath.h>

#include "efxitem.h"
#include "trackitem.h"

EFXItem::EFXItem(EFX *efx, ShowFunction *func)
    : ShowItem(func)
    , m_efx(efx)
{
    Q_ASSERT(efx != NULL);

    if (func->color().isValid())
        setColor(func->color());
    else
        setColor(ShowFunction::defaultColor(Function::EFXType));

    calculateWidth();
    connect(m_efx, SIGNAL(changed(quint32)), this, SLOT(slotEFXChanged(quint32)));
}

void EFXItem::calculateWidth()
{
    int newWidth = 0;
    qint64 efxDuration = getDuration();

    if (efxDuration != 0)
        newWidth = ((50.0 / float(getTimeScale())) * float(efxDuration)) / 1000.0;
    else
        newWidth = 100;

    if (newWidth < (50 / m_timeScale))
        newWidth = 50 / m_timeScale;
    setWidth(newWidth);
}

void EFXItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    float xpos = 0;
    float timeScale = 50 / float(m_timeScale);

    ShowItem::paint(painter, option, widget);

    int loopCount = m_function->duration() ? qFloor(m_function->duration() / m_efx->duration()) : 0;
    for (int i = 0; i < loopCount; i++)
    {
        xpos += ((timeScale * float(m_efx->duration())) / 1000);
        // draw loop vertical delimiter
        painter->setPen(QPen(Qt::white, 1));
        painter->drawLine(int(xpos), 1, int(xpos), TRACK_HEIGHT - 5);
    }

    ShowItem::postPaint(painter);
}

void EFXItem::setTimeScale(int val)
{
    ShowItem::setTimeScale(val);
    calculateWidth();
}

void EFXItem::setDuration(quint32 msec, bool stretch)
{
    if (stretch == true)
    {
        m_efx->setDuration(msec);
    }
    else
    {
        if (m_function)
            m_function->setDuration(msec);
        prepareGeometryChange();
        calculateWidth();
        updateTooltip();
    }
}

quint32 EFXItem::getDuration()
{
    return m_function->duration() ? m_function->duration() : m_efx->duration();
}

QString EFXItem::functionName()
{
    if (m_efx)
        return m_efx->name();
    return QString();
}

EFX *EFXItem::getEFX()
{
    return m_efx;
}

void EFXItem::slotEFXChanged(quint32)
{
    prepareGeometryChange();
    calculateWidth();
    updateTooltip();
}

void EFXItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *)
{
    QMenu menu;
    QFont menuFont = qApp->font();
    menuFont.setPixelSize(14);
    menu.setFont(menuFont);

    foreach (QAction *action, getDefaultActions())
        menu.addAction(action);

    menu.exec(QCursor::pos());
}
