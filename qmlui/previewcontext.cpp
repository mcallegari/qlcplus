/*
  Q Light Controller Plus
  previewcontext.cpp

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

#include "previewcontext.h"
#include "doc.h"

PreviewContext::PreviewContext(QQuickView *view, Doc *doc, QString name, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_mainView(view)
    , m_doc(doc)
    , m_name(name)
    , m_title(name)
    , m_page(0)
    , m_enabled(false)
    , m_detached(false)
{
    //connect(m_doc, &Doc::loaded, this, &PreviewContext::slotRefreshView);
}

QString PreviewContext::contextResource() const
{
    return m_resource;
}

void PreviewContext::setContextResource(QString res)
{
    m_resource = res;
}

void PreviewContext::enableContext(bool enable)
{
    m_enabled = enable;
}

bool PreviewContext::isEnabled()
{
    return m_enabled;
}

void PreviewContext::setUniverseFilter(quint32 universeFilter)
{
    m_universeFilter = universeFilter;
}

QQuickView *PreviewContext::view()
{
    return m_view;
}

QQuickItem *PreviewContext::contextItem()
{
    return m_contextItem;
}

void PreviewContext::setContextItem(QQuickItem *item)
{
    m_contextItem = item;
}

QString PreviewContext::name() const
{
    return m_name;
}

QString PreviewContext::contextTitle() const
{
    return m_title;
}

void PreviewContext::setContextTitle(QString title)
{
    m_title = title;
}

int PreviewContext::contextPage() const
{
    return m_page;
}

void PreviewContext::setContextPage(int page)
{
    m_page = page;
}

bool PreviewContext::detached() const
{
    return m_detached;
}

void PreviewContext::setDetached(bool detached)
{
    if (m_detached == detached)
        return;

    if (detached == true)
    {
        /** Create a new Quick View, as a true separate window */
        ContextQuickView *cqView = new ContextQuickView();
        m_view = cqView;
        connect(cqView, &ContextQuickView::keyPressed, this, &PreviewContext::keyPressed);
        connect(cqView, &ContextQuickView::keyReleased, this, &PreviewContext::keyReleased);

        /** Copy all the global properties of the main context into the detached one.
         *  This is a bit ugly, but I guess it is a downside of the QML programming */
        m_view->rootContext()->setContextProperty("qlcplus", m_mainView->rootContext()->contextProperty("qlcplus"));
        m_view->rootContext()->setContextProperty("screenPixelDensity", m_mainView->rootContext()->contextProperty("screenPixelDensity"));
        m_view->rootContext()->setContextProperty("ioManager", m_mainView->rootContext()->contextProperty("ioManager"));
        m_view->rootContext()->setContextProperty("fixtureBrowser", m_mainView->rootContext()->contextProperty("fixtureBrowser"));
        m_view->rootContext()->setContextProperty("fixtureManager", m_mainView->rootContext()->contextProperty("fixtureManager"));
        m_view->rootContext()->setContextProperty("fixtureGroupEditor", m_mainView->rootContext()->contextProperty("fixtureGroupEditor"));
        m_view->rootContext()->setContextProperty("functionManager", m_mainView->rootContext()->contextProperty("functionManager"));
        m_view->rootContext()->setContextProperty("contextManager", m_mainView->rootContext()->contextProperty("contextManager"));
        m_view->rootContext()->setContextProperty("virtualConsole", m_mainView->rootContext()->contextProperty("virtualConsole"));
        m_view->rootContext()->setContextProperty("showManager", m_mainView->rootContext()->contextProperty("showManager"));
        m_view->rootContext()->setContextProperty("actionManager", m_mainView->rootContext()->contextProperty("actionManager"));
        m_view->rootContext()->setContextProperty("View2D", m_mainView->rootContext()->contextProperty("View2D"));

        /** Set the fundamental properties to allow the detached context to properly load */
        m_view->rootContext()->setContextProperty("viewSource", contextResource());
        m_view->rootContext()->setContextProperty("contextName", name());
        m_view->rootContext()->setContextProperty("contextPage", contextPage());

        /** Finally, load the context wrapper and show it on the screen */
        m_view->setSource(QUrl("qrc:/WindowLoader.qml"));

        m_view->setTitle(contextTitle());
        m_view->setIcon(QIcon(":/qlcplus.svg"));

        m_view->setGeometry(0, 0, 800, 600);
        m_view->show();

        connect(m_view, SIGNAL(closing(QQuickCloseEvent*)), this, SLOT(slotWindowClosing()));
    }
    else
    {
        ContextQuickView *cqView = qobject_cast<ContextQuickView *>(m_view);
        disconnect(cqView, &ContextQuickView::keyPressed, this, &PreviewContext::keyPressed);
        disconnect(cqView, &ContextQuickView::keyReleased, this, &PreviewContext::keyReleased);
        m_view->deleteLater();
        m_view = m_mainView;
    }

    m_detached = detached;
}

void PreviewContext::handleKeyEvent(QKeyEvent *e, bool pressed)
{
    Q_UNUSED(e)
    Q_UNUSED(pressed)
}

void PreviewContext::slotWindowClosing()
{
    QMetaObject::invokeMethod(m_view->rootObject(), "closeWindow", Qt::AutoConnection);
}

void PreviewContext::slotRefreshView()
{

}


void ContextQuickView::keyPressEvent(QKeyEvent *e)
{
    emit keyPressed(e);
    QQuickView::keyPressEvent(e);
}

void ContextQuickView::keyReleaseEvent(QKeyEvent *e)
{
    emit keyReleased(e);
    QQuickView::keyReleaseEvent(e);
}
