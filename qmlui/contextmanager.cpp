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
#include "fixturemanager.h"
#include "mainviewdmx.h"
#include "mainview2d.h"

ContextManager::ContextManager(QQuickView *view, Doc *doc,
                               FixtureManager *fxMgr, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
    , m_fixtureManager(fxMgr)
{
    m_source = new GenericDMXSource(m_doc);
    m_source->setOutputEnabled(true);

    m_2DView = new MainView2D(m_view, m_doc);
    m_view->rootContext()->setContextProperty("View2D", m_2DView);

    m_DMXView = new MainViewDMX(m_view, m_doc);

    connect(m_fixtureManager, SIGNAL(newFixtureCreated(quint32,qreal,qreal)),
            this, SLOT(slotNewFixtureCreated(quint32,qreal,qreal)));
    connect(m_fixtureManager, SIGNAL(channelTypeValueChanged(int,quint8)),
            this, SLOT(slotChannelTypeValueChanged(int,quint8)));
    connect(m_fixtureManager, SIGNAL(colorChanged(QColor,QColor)),
            this, SLOT(slotColorChanged(QColor,QColor)));
}

void ContextManager::activateContext(QString context)
{
    if (context == "DMX")
    {
        m_DMXView->enableContext(true);
        m_2DView->enableContext(false);
    }
    else if (context == "2D")
    {
        m_DMXView->enableContext(false);
        m_2DView->enableContext(true);
    }
}

void ContextManager::setFixtureSelection(quint32 fxID, bool enable)
{
    QMultiHash<int, SceneValue> channels = m_fixtureManager->setFixtureCapabilities(fxID, enable);
    if(channels.keys().isEmpty())
        return;
    QHashIterator<int, SceneValue>it(channels);
    while(it.hasNext())
    {
        it.next();
        quint32 chType = it.key();
        SceneValue sv = it.value();
        if (enable)
        {
            m_channelsMap.insert(chType, sv);
        }
        else
        {
            m_channelsMap.remove(chType, sv);
        }
    }
}

void ContextManager::setRectangleSelection(qreal x, qreal y, qreal width, qreal height)
{
    QList<quint32> fxIDList;
    if (m_2DView->isEnabled())
        fxIDList = m_2DView->selectFixturesRect(QRectF(x, y, width, height));

    foreach(quint32 fxID, fxIDList)
        setFixtureSelection(fxID, true);
}

void ContextManager::slotNewFixtureCreated(quint32 fxID, qreal x, qreal y, qreal z)
{
    Q_UNUSED(z)

    QObject *viewObj = m_view->rootObject()->findChild<QObject *>("fixturesAndFunctions");
    if (viewObj == NULL)
        return;

    QString currentView = viewObj->property("currentView").toString();
    qDebug() << "[ContextManager] Current view:" << currentView;

    if (m_DMXView->isEnabled())
        m_DMXView->createFixtureItem(fxID);
    if (m_2DView->isEnabled())
        m_2DView->createFixtureItem(fxID, x, y, false);
}

void ContextManager::slotChannelTypeValueChanged(int type, quint8 value)
{
    //qDebug() << "type:" << type << "value:" << value;
    QList<SceneValue> svList = m_channelsMap.values(type);
    foreach(SceneValue sv, svList)
        m_source->set(sv.fxi, sv.channel, (uchar)value);
}

void ContextManager::slotColorChanged(QColor col, QColor wauv)
{
    slotChannelTypeValueChanged((int)QLCChannel::Red, (quint8)col.red());
    slotChannelTypeValueChanged((int)QLCChannel::Green, (quint8)col.green());
    slotChannelTypeValueChanged((int)QLCChannel::Blue, (quint8)col.blue());

    slotChannelTypeValueChanged((int)QLCChannel::White, (quint8)wauv.red());
    slotChannelTypeValueChanged((int)QLCChannel::Amber, (quint8)wauv.green());
    slotChannelTypeValueChanged((int)QLCChannel::UV, (quint8)wauv.blue());

    QColor cmykColor = col.toCmyk();
    slotChannelTypeValueChanged((int)QLCChannel::Cyan, (quint8)cmykColor.cyan());
    slotChannelTypeValueChanged((int)QLCChannel::Magenta, (quint8)cmykColor.magenta());
    slotChannelTypeValueChanged((int)QLCChannel::Yellow, (quint8)cmykColor.yellow());


}

