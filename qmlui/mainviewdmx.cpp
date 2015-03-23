/*
  Q Light Controller Plus
  mainviewdmx.cpp

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
#include <QByteArray>
#include <QQuickItem>
#include <QQmlContext>
#include <QQmlComponent>

#include "mainviewdmx.h"
#include "qlcfixturemode.h"
#include "doc.h"

MainViewDMX::MainViewDMX(QQuickView *view, Doc *doc, QObject *parent)
    : PreviewContext(view, doc, parent)
{

    m_viewDMX = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("DMXFlowView"));

    fixtureComponent = new QQmlComponent(m_view->engine(), QUrl("qrc:/FixtureDMXItem.qml"));
    if (fixtureComponent->isError())
        qDebug() << fixtureComponent->errors();

    connect(m_doc, SIGNAL(loaded()),
            this, SLOT(slotRefreshView()));

    connect(m_doc->inputOutputMap(), SIGNAL(universesWritten(int, const QByteArray&)),
            this, SLOT(slotUniversesWritten(int, const QByteArray&)));
}

MainViewDMX::~MainViewDMX()
{
    reset();
}

void MainViewDMX::enableContext(bool enable)
{
    PreviewContext::enableContext(enable);
    if (enable == true)
        slotRefreshView();
}

void MainViewDMX::reset()
{
    QMapIterator<quint32, QQuickItem*> it(m_itemsMap);
    while(it.hasNext())
    {
        it.next();
        delete it.value();
    }
    m_itemsMap.clear();
}

void MainViewDMX::createFixtureItem(quint32 fxID)
{
    if (isEnabled() == false)
        return;

    qDebug() << "[MainViewDMX] Creating fixture with ID" << fxID;

    Fixture *fixture = m_doc->fixture(fxID);

    QQuickItem *newFixtureItem = qobject_cast<QQuickItem*>(fixtureComponent->create());

    newFixtureItem->setParentItem(m_viewDMX);
    newFixtureItem->setProperty("fixtureObj", QVariant::fromValue(fixture));

    // and finally add the new item to the items map
    m_itemsMap[fxID] = newFixtureItem;
}

void MainViewDMX::slotRefreshView()
{
    m_viewDMX = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("DMXFlowView"));

    reset();

    foreach(Fixture *fixture, m_doc->fixtures())
        createFixtureItem(fixture->id());
}

void MainViewDMX::slotUniversesWritten(int idx, const QByteArray &ua)
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

        QVariantList dmxValues;
        int startAddr = fixture->address();
        for (int i = startAddr; i < startAddr + (int)fixture->channels(); i++)
        {
            if (i < ua.size())
                dmxValues.append(QString::number((uchar)ua.at(i)));
        }

        QQuickItem *fxItem = it.value();
        fxItem->setProperty("values", QVariant::fromValue(dmxValues));
    }
}

