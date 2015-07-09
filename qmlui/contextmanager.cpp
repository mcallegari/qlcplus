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
#include "functionmanager.h"
#include "fixturemanager.h"
#include "qlcfixturemode.h"
#include "mainviewdmx.h"
#include "mainview2d.h"
#include "doc.h"

ContextManager::ContextManager(QQuickView *view, Doc *doc,
                               FixtureManager *fxMgr, FunctionManager *funcMgr,
                               QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
    , m_fixtureManager(fxMgr)
    , m_functionManager(funcMgr)
{
    m_source = new GenericDMXSource(m_doc);
    m_source->setOutputEnabled(true);

    m_2DView = new MainView2D(m_view, m_doc);
    m_view->rootContext()->setContextProperty("View2D", m_2DView);

    m_DMXView = new MainViewDMX(m_view, m_doc);

    connect(m_fixtureManager, SIGNAL(newFixtureCreated(quint32,qreal,qreal)),
            this, SLOT(slotNewFixtureCreated(quint32,qreal,qreal)));
    connect(m_fixtureManager, SIGNAL(channelValueChanged(quint32,quint32,quint8)),
            this, SLOT(slotChannelValueChanged(quint32,quint32,quint8)));
    connect(m_fixtureManager, SIGNAL(channelTypeValueChanged(int,quint8)),
            this, SLOT(slotChannelTypeValueChanged(int,quint8)));
    connect(m_fixtureManager, SIGNAL(colorChanged(QColor,QColor)),
            this, SLOT(slotColorChanged(QColor,QColor)));
    connect(m_fixtureManager, SIGNAL(presetChanged(const QLCChannel*,quint8)),
            this, SLOT(slotPresetChanged(const QLCChannel*,quint8)));
    connect(m_doc->inputOutputMap(), SIGNAL(universesWritten(int, const QByteArray&)),
            this, SLOT(slotUniversesWritten(int, const QByteArray&)));
}

void ContextManager::enableContext(QString context, bool enable)
{
    if (context == "DMX")
        m_DMXView->enableContext(enable);
    else if (context == "2D")
        m_2DView->enableContext(enable);
}

void ContextManager::detachContext(QString context)
{
    qDebug() << "[ContextManager] detaching context:" << context;
}

void ContextManager::reattachContext(QString context)
{
    qDebug() << "[ContextManager] reattaching context:" << context;
    if (context == "DMX" || context == "2D")
    {
        QQuickItem *viewObj = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("fixturesAndFunctions"));
        if (viewObj == NULL)
            return;
        QMetaObject::invokeMethod(viewObj, "enableContext",
                Q_ARG(QVariant, context));
    }
}

void ContextManager::updateContexts()
{
    m_channelsMap.clear();
    m_selectedFixtures.clear();
    m_DMXView->enableContext(m_DMXView->isEnabled());
    m_2DView->enableContext(m_2DView->isEnabled());
}

void ContextManager::setFixtureSelection(quint32 fxID, bool enable)
{
    if (m_selectedFixtures.contains(fxID))
    {
        if (enable == false)
            m_selectedFixtures.removeAll(fxID);
        else
            return;
    }
    else
    {
        if (enable)
            m_selectedFixtures.append(fxID);
        else
            return;
    }

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

void ContextManager::dumpDmxChannels()
{
    QList<SceneValue> chList = m_source->channels();
    QList<SceneValue> dumpList;

    /** Now create a list with only the channels of the
     *  currently selected fixtures */
    foreach(SceneValue sv, chList)
    {
        if (m_selectedFixtures.contains(sv.fxi))
            dumpList.append(sv);
    }
    m_functionManager->dumpOnNewScene(dumpList);
}

void ContextManager::createFixtureGroup()
{
    if (m_selectedFixtures.isEmpty())
        return;

    m_fixtureManager->addFixturesToNewGroup(m_selectedFixtures);
}

void ContextManager::slotNewFixtureCreated(quint32 fxID, qreal x, qreal y, qreal z)
{
    Q_UNUSED(z)
/*
    QObject *viewObj = m_view->rootObject()->findChild<QObject *>("fixturesAndFunctions");
    if (viewObj == NULL)
        return;

    QString currentView = viewObj->property("currentView").toString();
    qDebug() << "[ContextManager] Current view:" << currentView;
*/
    if (m_DMXView->isEnabled())
        m_DMXView->createFixtureItem(fxID);
    if (m_2DView->isEnabled())
        m_2DView->createFixtureItem(fxID, x, y, false);
}

void ContextManager::slotChannelValueChanged(quint32 fxID, quint32 channel, quint8 value)
{
    SceneValue sv(fxID, channel);
    m_source->set(sv.fxi, sv.channel, (uchar)value);
}

void ContextManager::slotChannelTypeValueChanged(int type, quint8 value, quint32 channel)
{
    quint32 valCount = m_source->channelsCount();
    //qDebug() << "type:" << type << "value:" << value;
    QList<SceneValue> svList = m_channelsMap.values(type);
    foreach(SceneValue sv, svList)
    {
        if (channel == UINT_MAX || (channel != UINT_MAX && channel == sv.channel))
            m_source->set(sv.fxi, sv.channel, (uchar)value);
    }

    /** Monitor the changes from/to 0 */
    if ((valCount == 0 && m_source->channelsCount() > 0) ||
        (valCount > 0 && m_source->channelsCount() == 0))
    {
        QQuickItem *dumpBtn = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("dumpButton"));
        if (dumpBtn != NULL)
        {
            if (valCount)
                dumpBtn->setProperty("visible", false);
            else
                dumpBtn->setProperty("visible", true);
        }
    }
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

void ContextManager::slotPresetChanged(const QLCChannel *channel, quint8 value)
{
    foreach(quint32 fxID, m_selectedFixtures)
    {
        Fixture *fixture = m_doc->fixture(fxID);
        if (fixture == NULL)
            continue;

        if (fixture->fixtureDef() != NULL && fixture->fixtureMode() != NULL)
        {
            quint32 chIdx = fixture->fixtureMode()->channelNumber((QLCChannel *)channel);
            if (chIdx != QLCChannel::invalid())
                slotChannelTypeValueChanged((int)channel->group(), value, chIdx);
        }
    }
}

void ContextManager::slotUniversesWritten(int idx, const QByteArray &ua)
{
    foreach(Fixture *fixture, m_doc->fixtures())
    {
        if (fixture->universe() != (quint32)idx)
            continue;

        int fxStartAddr = fixture->address();
        if (fxStartAddr >= ua.size())
            continue;

        if (fixture->setChannelValues(ua.mid(fxStartAddr, fixture->channels())) == true)
        {
            if (m_DMXView->isEnabled())
                m_DMXView->updateFixture(fixture);
            if (m_2DView->isEnabled())
                m_2DView->updateFixture(fixture);
        }
    }
}

