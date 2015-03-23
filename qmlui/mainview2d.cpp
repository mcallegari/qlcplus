/*
  Q Light Controller Plus
  mainview2d.cpp

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

#include <QDebug>
#include <QQuickItem>
#include <QQmlContext>
#include <QQmlComponent>

#include "mainview2d.h"
#include "qlcfixturemode.h"
#include "doc.h"

MainView2D::MainView2D(QQuickView *view, Doc *doc, QObject *parent)
    : PreviewContext(view, doc, parent)
{
    m_gridSize = QSize(5, 5);
    m_gridScale = 1.0;
    m_cellPixels = 100;
    m_xOffset = 0;
    m_yOffset = 0;
    m_gridUnits = 1000;

    m_view2D = NULL;
    m_contents2D = NULL;

    fixtureComponent = new QQmlComponent(m_view->engine(), QUrl("qrc:/Fixture2DItem.qml"));
    if (fixtureComponent->isError())
        qDebug() << fixtureComponent->errors();

    connect(m_doc, SIGNAL(loaded()),
            this, SLOT(slotRefreshView()));

    connect(m_doc->inputOutputMap(), SIGNAL(universesWritten(int, const QByteArray&)),
            this, SLOT(slotUniversesWritten(int, const QByteArray&)));
}

MainView2D::~MainView2D()
{
    reset();
}

void MainView2D::enableContext(bool enable)
{
    PreviewContext::enableContext(enable);
    if (enable == true)
        slotRefreshView();
}

void MainView2D::reset()
{
    QMapIterator<quint32, QQuickItem*> it(m_itemsMap);
    while(it.hasNext())
    {
        it.next();
        delete it.value();
    }
    m_itemsMap.clear();
}

void MainView2D::initialize2DProperties()
{
    m_view2D = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("twoDView"));
    m_contents2D = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("twoDContents"));

    if (m_view2D == NULL || m_contents2D == NULL)
        return;

    m_gridScale = m_view2D->property("gridScale").toReal();
    m_cellPixels = m_view2D->property("baseCellSize").toReal();

    m_xOffset = m_contents2D->property("x").toReal();
    m_yOffset = m_contents2D->property("y").toReal();
}

QPointF MainView2D::getAvailablePosition(qreal width, qreal height)
{
    qreal xPos = 0, yPos = 0;
    qreal maxYOffset = 0;

    if (m_view2D == NULL || m_contents2D == NULL)
        initialize2DProperties();

    QRectF gridArea(0, 0, m_gridSize.width() * m_gridUnits, m_gridSize.height() * m_gridUnits);

    QMapIterator<quint32, QQuickItem *> it(m_itemsMap);
    while (it.hasNext())
    {
        it.next();
        QQuickItem *fxItem = it.value();
        qreal itemXPos = fxItem->property("mmXPos").toReal();
        qreal itemYPos = fxItem->property("mmYPos").toReal();
        qreal itemWidth = fxItem->property("mmWidth").toReal();
        qreal itemHeight = fxItem->property("mmHeight").toReal();

        // store the next Y row in case we need to lower down
        if (itemYPos + itemHeight > maxYOffset )
            maxYOffset = itemYPos + itemHeight;

        QRectF itemRect(itemXPos, itemYPos, itemWidth, itemHeight);
        QRectF newItemRect(xPos, yPos, width, height);

        if (itemRect.intersects(newItemRect) == true)
        {
            xPos = itemXPos + itemWidth + 50; //add an extra 50mm spacing
            if (xPos + width > gridArea.width())
            {
                xPos = 0;
                yPos = maxYOffset + 50;
                maxYOffset = 0;
            }
        }
        else
            break;
    }

    return QPointF(xPos, yPos);
}

void MainView2D::createFixtureItem(quint32 fxID, qreal x, qreal y, bool mmCoords)
{
    if (isEnabled() == false)
        return;

    //if (m_view2D == NULL || m_contents2D == NULL)
        initialize2DProperties();

    qDebug() << "[MainView2D] Creating fixture with ID" << fxID << "x:" << x << "y:" << y;

    Fixture *fixture = m_doc->fixture(fxID);
    MonitorProperties *mProps = m_doc->monitorProperties();
    QLCFixtureMode *fxMode = fixture->fixtureMode();

    QQuickItem *newFixtureItem = qobject_cast<QQuickItem*>(fixtureComponent->create());

    newFixtureItem->setParentItem(m_contents2D);
    newFixtureItem->setProperty("fixtureID", fxID);

    if (fxMode != NULL)
    {
        if (fxMode->physical().width() != 0)
            newFixtureItem->setProperty("mmWidth", fxMode->physical().width());
        if (fxMode->physical().height() != 0)
            newFixtureItem->setProperty("mmHeight", fxMode->physical().height());
        qDebug() << "Current mode fixture heads:" << fxMode->heads().count();
        newFixtureItem->setProperty("headsNumber", fxMode->heads().count());
    }

    if (x == -1 && y == -1)
    {
        QPointF availablePos = getAvailablePosition(newFixtureItem->property("mmWidth").toReal(),
                                                    newFixtureItem->property("mmHeight").toReal());
        x = availablePos.x();
        y = availablePos.y();
    }

    if (x > m_xOffset)
    {
        if (mmCoords == false)
            x = ((x - m_xOffset) * m_gridUnits) / m_cellPixels;
        newFixtureItem->setProperty("mmXPos", x);
    }
    if (y > m_yOffset)
    {
        if (mmCoords == false)
            y = ((y - m_yOffset) * m_gridUnits) / m_cellPixels;
        newFixtureItem->setProperty("mmYPos", y);
    }

    // add the new fixture to the Doc monitor properties
    mProps->setFixturePosition(fxID, QPointF(x, y));

    // and finally add the new item to the items map
    m_itemsMap[fxID] = newFixtureItem;
}

void MainView2D::slotRefreshView()
{
    initialize2DProperties();

    MonitorProperties *mProps = m_doc->monitorProperties();
    QList<quint32> mPropsIDs;
    if (mProps)
    {
        mPropsIDs = mProps->fixtureItemsID();
        m_view2D->setProperty("gridWidth", mProps->gridSize().width());
        m_view2D->setProperty("gridHeight", mProps->gridSize().height());
        if (mProps->gridUnits() == MonitorProperties::Meters)
            m_view2D->setProperty("gridUnits", 1000);
        else
            m_view2D->setProperty("gridUnits", 304.8);
    }

    reset();

    foreach(Fixture *fixture, m_doc->fixtures())
    {
        if (mPropsIDs.contains(fixture->id()))
        {
            QPointF fxPos = mProps->fixturePosition(fixture->id());
            createFixtureItem(fixture->id(), fxPos.x(), fxPos.y());
        }
        else
            createFixtureItem(fixture->id(), -1, -1, false);
    }
}

void MainView2D::slotUniversesWritten(int idx, const QByteArray &ua)
{
    Q_UNUSED(idx)
    Q_UNUSED(ua)
    if (m_enabled == false)
        return;

}

