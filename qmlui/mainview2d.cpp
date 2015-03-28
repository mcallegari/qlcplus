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

QPointF MainView2D::getAvailablePosition(QRectF& fxRect)
{
    qreal xPos = fxRect.x(), yPos = fxRect.y();
    qreal maxYOffset = 0;

    if (m_view2D == NULL || m_contents2D == NULL)
        initialize2DProperties();

    QRectF gridArea(0, 0, m_gridSize.width() * m_gridUnits, m_gridSize.height() * m_gridUnits);

    qreal origWidth = fxRect.width();
    qreal origHeight = fxRect.height();

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

        if (fxRect.intersects(itemRect) == true)
        {
            xPos = itemXPos + itemWidth + 50; //add an extra 50mm spacing
            if (xPos + fxRect.width() > gridArea.width())
            {
                xPos = 0;
                yPos = maxYOffset + 50;
                maxYOffset = 0;
            }
            fxRect.setX(xPos);
            fxRect.setY(yPos);
            // restore width and height as setX and setY mess them
            fxRect.setWidth(origWidth);
            fxRect.setHeight(origHeight);
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
    QRectF fxRect;

    QQuickItem *newFixtureItem = qobject_cast<QQuickItem*>(fixtureComponent->create());

    newFixtureItem->setParentItem(m_contents2D);
    newFixtureItem->setProperty("fixtureID", fxID);

    if (mmCoords == false)
    {
        x = ((x - m_xOffset) * m_gridUnits) / m_cellPixels;
        y = ((y - m_yOffset) * m_gridUnits) / m_cellPixels;
    }
    fxRect.setX(x);
    fxRect.setY(y);

    if (fxMode != NULL)
    {
        if (fxMode->physical().width() != 0)
        {
            newFixtureItem->setProperty("mmWidth", fxMode->physical().width());
            fxRect.setWidth(fxMode->physical().width());
        }
        if (fxMode->physical().height() != 0)
        {
            newFixtureItem->setProperty("mmHeight", fxMode->physical().height());
            fxRect.setHeight(fxMode->physical().height());
        }
        qDebug() << "Current mode fixture heads:" << fxMode->heads().count();
        newFixtureItem->setProperty("headsNumber", fxMode->heads().count());
    }
    else
    {
        // default to 300x300mm
        fxRect.setWidth(300);
        fxRect.setHeight(300);
    }

    QPointF availablePos = getAvailablePosition(fxRect);
    x = availablePos.x();
    y = availablePos.y();

    newFixtureItem->setProperty("mmXPos", x);
    newFixtureItem->setProperty("mmYPos", y);

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
            createFixtureItem(fixture->id(), 0, 0, false);
    }
}

void MainView2D::slotUniversesWritten(int idx, const QByteArray &ua)
{
    if (m_enabled == false)
        return;

    QMapIterator<quint32, QQuickItem*> it(m_itemsMap);
    while(it.hasNext())
    {
        it.next();
        Fixture *fixture = m_doc->fixture(it.key());
        if (fixture == NULL)
            continue;

        if (fixture->universe() != (quint32)idx)
            continue;

        int fxStartAddr = fixture->address();
        QQuickItem *fxItem = it.value();

        for (int headIdx = 0; headIdx < fixture->heads(); headIdx++)
        {
            QVector <quint32> rgbCh = fixture->rgbChannels(headIdx);
            if (rgbCh.size() > 0)
            {
                quint8 r = 0, g = 0, b = 0;
                if (fxStartAddr + rgbCh.at(0) < (quint32)ua.size())
                    r = ua.at(fxStartAddr + rgbCh.at(0));
                if (fxStartAddr + rgbCh.at(1) < (quint32)ua.size())
                    g = ua.at(fxStartAddr + rgbCh.at(1));
                if (fxStartAddr + rgbCh.at(2) < (quint32)ua.size())
                    b = ua.at(fxStartAddr + rgbCh.at(2));

                QMetaObject::invokeMethod(fxItem, "setHeadColor",
                        Q_ARG(QVariant, headIdx),
                        Q_ARG(QVariant, QColor(r, g, b)));
            }
        }
    }
}

