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
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
{
    m_gridSize = QSize(5, 5);
    m_gridScale = 1.0;
    m_cellPixels = 100;
    m_xOffset = 0;
    m_yOffset = 0;
    m_unitValue = 1000;
}

MainView2D::~MainView2D()
{

}

void MainView2D::createFixtureItem(quint32 fxID, qreal x, qreal y, bool mmCoords)
{
    QQuickItem *twoDView = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("twoDView"));
    if (twoDView == NULL)
        return;
    QQuickItem *twoDContents = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("twoDContents"));
    if (twoDContents == NULL)
        return;

    m_gridScale = twoDView->property("gridScale").toReal();
    m_cellPixels = twoDView->property("baseCellSize").toReal();

    m_xOffset = twoDContents->property("x").toReal();
    m_yOffset = twoDContents->property("y").toReal();

    Fixture *fixture = m_doc->fixture(fxID);
    QLCFixtureMode *fxMode = fixture->fixtureMode();

    QQmlComponent fixtureComponent(m_view->engine(), QUrl("qrc:/Fixture2DItem.qml"));
    if (fixtureComponent.isError())
        qDebug() << fixtureComponent.errors();
    QQuickItem *newFixtureItem = qobject_cast<QQuickItem*>(fixtureComponent.create());

    newFixtureItem->setParentItem(twoDContents);
    newFixtureItem->setProperty("fixtureID", fxID);

    if (x > m_xOffset)
    {
        if (mmCoords == false)
            x = ((x - m_xOffset) * m_unitValue) / m_cellPixels;
        newFixtureItem->setProperty("mmXPos", x);
    }
    if (y > m_yOffset)
    {
        if (mmCoords == false)
            y = ((y - m_yOffset) * m_unitValue) / m_cellPixels;
        newFixtureItem->setProperty("mmYPos", y);
    }
    if (fxMode != NULL)
    {
        if (fxMode->physical().width() != 0)
            newFixtureItem->setProperty("mmWidth", fxMode->physical().width());
        if (fxMode->physical().height() != 0)
            newFixtureItem->setProperty("mmHeight", fxMode->physical().height());
        qDebug() << "Current mode fixture heads:" << fxMode->heads().count();
        newFixtureItem->setProperty("headsNumber", fxMode->heads().count());
    }
}

