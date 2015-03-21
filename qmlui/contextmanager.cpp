/*
  Q Light Controller Plus
  contextmanager.cpp

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

#include <QQmlContext>
#include <QQuickItem>
#include <QDebug>

#include "contextmanager.h"
#include "genericdmxsource.h"
#include "mainview2d.h"

ContextManager::ContextManager(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
{
    m_source = new GenericDMXSource(m_doc);

    m_2DView = new MainView2D(m_view, m_doc);
    m_view->rootContext()->setContextProperty("View2D", m_2DView);
}

void ContextManager::activateContext(QString context)
{
    if (context == "2D")
        m_2DView->enableContext(true);
}

void ContextManager::slotNewFixtureCreated(quint32 fxID, qreal x, qreal y, qreal z)
{
    Q_UNUSED(z)

    QObject *viewObj = m_view->rootObject()->findChild<QObject *>("fixturesAndFunctions");
    if (viewObj == NULL)
        return;

    QString currentView = viewObj->property("currentView").toString();
    qDebug() << "[ContextManager] Current view:" << currentView;

    m_2DView->createFixtureItem(fxID, x, y, false);
}

