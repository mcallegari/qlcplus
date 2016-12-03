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
    : PreviewContext(view, doc, "DMX", parent)
{
    setContextResource("qrc:/DMXView.qml");
    setContextTitle(tr("DMX View"));

    fixtureComponent = new QQmlComponent(m_view->engine(), QUrl("qrc:/FixtureDMXItem.qml"));
    if (fixtureComponent->isError())
        qDebug() << fixtureComponent->errors();
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

void MainViewDMX::setUniverseFilter(quint32 universeFilter)
{
    PreviewContext::setUniverseFilter(universeFilter);
    QMapIterator<quint32, QQuickItem*> it(m_itemsMap);
    while(it.hasNext())
    {
        it.next();
        quint32 fxID = it.key();
        QQuickItem *fxItem = it.value();
        Fixture *fixture = m_doc->fixture(fxID);
        if (fixture == NULL)
            continue;

        if (universeFilter == Universe::invalid() || fixture->universe() == universeFilter)
            fxItem->setProperty("visible", "true");
        else
            fxItem->setProperty("visible", "false");
    }
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

    newFixtureItem->setParentItem(contextItem());
    newFixtureItem->setProperty("fixtureObj", QVariant::fromValue(fixture));

    // and finally add the new item to the items map
    m_itemsMap[fxID] = newFixtureItem;

    updateFixture(fixture);
}

void MainViewDMX::updateFixture(Fixture *fixture)
{
    if (isEnabled() == false || fixture == NULL)
        return;

    if (m_itemsMap.contains(fixture->id()) == false)
        return;

    QByteArray fxValues = fixture->channelValues();
    QVariantList dmxValues;

    for (int i = 0; i < (int)fixture->channels(); i++)
        dmxValues.append(QString::number((uchar)fxValues.at(i)));

    QQuickItem *fxItem = m_itemsMap[fixture->id()];
    fxItem->setProperty("values", QVariant::fromValue(dmxValues));

}

void MainViewDMX::updateFixtureSelection(QList<quint32>fixtures)
{
    QMapIterator<quint32, QQuickItem*> it(m_itemsMap);
    while(it.hasNext())
    {
        it.next();
        quint32 fxID = it.key();
        QQuickItem *fxItem = it.value();
        if(fixtures.contains(fxID))
            fxItem->setProperty("isSelected", true);
        else
            fxItem->setProperty("isSelected", false);
    }
}

void MainViewDMX::updateFixtureSelection(quint32 fxID, bool enable)
{
    if (isEnabled() == false || m_itemsMap.contains(fxID) == false)
        return;

    QQuickItem *fxItem = m_itemsMap[fxID];
    fxItem->setProperty("isSelected", enable);
}

void MainViewDMX::slotRefreshView()
{
    if (isEnabled() == false)
        return;

    reset();

    foreach(Fixture *fixture, m_doc->fixtures())
        createFixtureItem(fixture->id());
}


