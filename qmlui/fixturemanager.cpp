/*
  Q Light Controller Plus
  fixturemanager.cpp

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

#include "fixturemanager.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "fixture.h"
#include "doc.h"

FixtureManager::FixtureManager(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
{
    Q_ASSERT(m_doc != NULL);

    connect(m_doc, SIGNAL(loaded()),
            this, SIGNAL(docLoaded()));
}

quint32 FixtureManager::invalidFixture()
{
    return Fixture::invalidId();
}

quint32 FixtureManager::fixtureForAddress(quint32 index)
{
    return m_doc->fixtureForAddress(index);
}

bool FixtureManager::addFixture(QString manuf, QString model, QString mode, QString name,
                                int uniIdx, int address, int channels, int quantity, quint32 gap,
                                qreal xPos, qreal yPos)
{
    qDebug() << Q_FUNC_INFO << manuf << model << quantity;

    QLCFixtureDef *fxiDef = m_doc->fixtureDefCache()->fixtureDef(manuf, model);
    Q_ASSERT(fxiDef != NULL);

    QLCFixtureMode *fxiMode = fxiDef->mode(mode);
    Q_ASSERT(fxiMode != NULL);

    for (int i = 0; i < quantity; i++)
    {
        Fixture *fxi = new Fixture(m_doc);

        /* If we're adding more than one fixture,
           append a number to the end of the name */
        if (quantity > 1)
            fxi->setName(QString("%1 #%2").arg(name).arg(i + 1));
        else
            fxi->setName(name);
        fxi->setAddress(address + (i * channels) + (i * gap));
        fxi->setUniverse(uniIdx);
        fxi->setFixtureDefinition(fxiDef, fxiMode);

        m_doc->addFixture(fxi);
        createQMLFixture(fxi->id(), xPos, yPos);
    }
    m_fixtureList.clear();
    m_fixtureList = m_doc->fixtures();
    emit fixturesCountChanged();

    return true;
}

int FixtureManager::fixturesCount()
{
    return m_doc->fixtures().count();
}

QQmlListProperty<Fixture> FixtureManager::fixtures()
{
    m_fixtureList.clear();
    m_fixtureList = m_doc->fixtures();
    return QQmlListProperty<Fixture>(this, m_fixtureList);
}

void FixtureManager::createQMLFixture(quint32 fxID, qreal x, qreal y)
{
    QObject *viewObj = m_view->rootObject()->findChild<QObject *>("fixturesAndFunctions");
    if (viewObj == NULL)
        return;

    QString currentView = viewObj->property("currentView").toString();
    qDebug() << "Current view:" << currentView;

    if (currentView == "2D")
    {
        QQuickItem *twoDView = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("twoDView"));
        QQmlComponent fixtureComponent(m_view->engine(),
                        QUrl("qrc:/FixtureDMXItem.qml"));
        QQuickItem *newView = qobject_cast<QQuickItem*>(fixtureComponent.create());
        newView->setParentItem(twoDView);
        newView->setProperty("fixtureID", fxID);
        newView->setProperty("xPos", x);
        newView->setProperty("yPos", y);
    }
}




