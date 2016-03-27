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
#include "monitorproperties.h"
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
    , m_universeFilter(Universe::invalid())
    , m_prevRotation(0)
    , m_editingEnabled(false)
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
    connect(m_fixtureManager, SIGNAL(positionTypeValueChanged(int,int)),
            this, SLOT(slotPositionChanged(int,int)));
    connect(m_fixtureManager, SIGNAL(presetChanged(const QLCChannel*,quint8)),
            this, SLOT(slotPresetChanged(const QLCChannel*,quint8)));
    connect(m_doc->inputOutputMap(), SIGNAL(universesWritten(int, const QByteArray&)),
            this, SLOT(slotUniversesWritten(int, const QByteArray&)));
    connect(m_functionManager, SIGNAL(functionEditingChanged(bool)),
            this, SLOT(slotFunctionEditingChanged(bool)));
}

void ContextManager::enableContext(QString context, bool enable)
{
    if (context == "DMX")
    {
        m_DMXView->enableContext(enable);
        m_DMXView->updateFixtureSelection(m_selectedFixtures);
    }
    else if (context == "2D")
    {
        m_2DView->enableContext(enable);
        m_2DView->updateFixtureSelection(m_selectedFixtures);
    }
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
                Q_ARG(QVariant, context),
                Q_ARG(QVariant, false));
    }
    else
    {

        QMetaObject::invokeMethod(m_view->rootObject(), "enableContext",
                Q_ARG(QVariant, context),
                Q_ARG(QVariant, false));
    }
}

void ContextManager::resetContexts()
{
    m_channelsMap.clear();
    resetValues();
    foreach(quint32 fxID, m_selectedFixtures)
        setFixtureSelection(fxID, false);

    m_selectedFixtures.clear();
    m_editingEnabled = false;
    m_DMXView->enableContext(m_DMXView->isEnabled());
    m_2DView->enableContext(m_2DView->isEnabled());
}

void ContextManager::resetValues()
{
    QMap<QPair<quint32, quint32>, uchar> dumpValues = m_functionManager->dumpValues();
    int dumpValuesCount = m_functionManager->dumpValuesCount();
    QMutableMapIterator <QPair<quint32,quint32>,uchar> it(dumpValues);
    while (it.hasNext() == true)
    {
        it.next();
        SceneValue sv;
        sv.fxi = it.key().first;
        sv.channel = it.key().second;
        m_source->set(sv.fxi, sv.channel, 0);
    }
    m_source->unsetAll();
    m_functionManager->resetDumpValues();
    checkDumpButton(dumpValuesCount);
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

    if (m_DMXView->isEnabled())
        m_DMXView->updateFixtureSelection(fxID, enable);
    if (m_2DView->isEnabled())
        m_2DView->updateFixtureSelection(fxID, enable);

    QMultiHash<int, SceneValue> channels = m_fixtureManager->setFixtureCapabilities(fxID, enable);
    if(channels.keys().isEmpty())
        return;

    qDebug() << "[ContextManager] found" << channels.keys().count() << "capabilities";

    QHashIterator<int, SceneValue>it(channels);
    while(it.hasNext())
    {
        it.next();
        quint32 chType = it.key();
        SceneValue sv = it.value();
        if (enable)
            m_channelsMap.insert(chType, sv);
        else
            m_channelsMap.remove(chType, sv);
    }
    emit selectedFixturesChanged();
}

void ContextManager::setRectangleSelection(qreal x, qreal y, qreal width, qreal height)
{
    QList<quint32> fxIDList;
    if (m_2DView->isEnabled())
        fxIDList = m_2DView->selectFixturesRect(QRectF(x, y, width, height));

    foreach(quint32 fxID, fxIDList)
        setFixtureSelection(fxID, true);
    emit selectedFixturesChanged();
}

bool ContextManager::hasSelectedFixtures()
{
    if (m_selectedFixtures.isEmpty())
        return false;
    return true;
}

void ContextManager::setFixturePosition(quint32 fxID, qreal x, qreal y)
{
    MonitorProperties *mProps = m_doc->monitorProperties();
    mProps->setFixturePosition(fxID, QPointF(x, y));
}

void ContextManager::dumpDmxChannels()
{
    m_functionManager->dumpOnNewScene(m_selectedFixtures);
}

void ContextManager::createFixtureGroup()
{
    if (m_selectedFixtures.isEmpty())
        return;

    m_fixtureManager->addFixturesToNewGroup(m_selectedFixtures);
}

void ContextManager::handleKeyPress(QKeyEvent *e)
{
    //qDebug() << "Key event received:" << e->text();

    if (e->modifiers() & Qt::ControlModifier)
    {
        switch(e->key())
        {
            case Qt::Key_A:
            {
                bool selectAll = true;
                if (m_selectedFixtures.count() == m_doc->fixtures().count())
                    selectAll = false;

                foreach(Fixture *fixture, m_doc->fixtures())
                {
                    if (fixture != NULL)
                        setFixtureSelection(fixture->id(), selectAll);
                }
            }
            break;
            case Qt::Key_R:
            {
                int valCount = m_functionManager->dumpValuesCount();
                resetValues();
                checkDumpButton(valCount);
            }
            break;
            default:
            break;
        }
    }
}

int ContextManager::fixturesRotation() const
{
    int commonRotation = -1;
    MonitorProperties *mProps = m_doc->monitorProperties();

    foreach(quint32 fxID, m_selectedFixtures)
    {
        if (mProps->hasFixturePosition(fxID) == false)
            continue;

        int rot = mProps->fixtureRotation(fxID);
        if (commonRotation == -1)
            commonRotation = rot;
        else
        {
            if (rot != commonRotation)
                return 0;
        }
    }

    if (commonRotation != -1)
        return commonRotation;

    return 0;
}

void ContextManager::setFixturesRotation(int degrees)
{
    bool mixed = false;
    int commonRotation = -1;
    MonitorProperties *mProps = m_doc->monitorProperties();

    // first, detect if we're setting the rotation
    // in a mixed context
    foreach(quint32 fxID, m_selectedFixtures)
    {
        if (mProps->hasFixturePosition(fxID) == false)
            continue;

        int rot = mProps->fixtureRotation(fxID);
        if (commonRotation == -1)
            commonRotation = rot;
        else
        {
            if (rot != commonRotation)
            {
                mixed = true;
                break;
            }
        }
    }

    // if mixed, treat degrees as relative
    // otherwise set it directly
    int delta = degrees - m_prevRotation;
    foreach(quint32 fxID, m_selectedFixtures)
    {
        int rot = 0;
        if (mProps->hasFixturePosition(fxID))
            rot = mProps->fixtureRotation(fxID);

        if (mixed == false)
            rot = degrees;
        else
            rot += delta;

        // normalize back to a 0-359 range
        if (rot < 0) rot += 360;
        else if (rot >= 360) rot -= 360;
        mProps->setFixtureRotation(fxID, rot);
        if (m_2DView->isEnabled())
            m_2DView->updateFixtureRotation(fxID, rot);
    }
    m_prevRotation = degrees;
}

quint32 ContextManager::universeFilter() const
{
    return m_universeFilter;
}

void ContextManager::setUniverseFilter(quint32 universeFilter)
{
    if (m_universeFilter == universeFilter)
        return;

    m_universeFilter = universeFilter;

    if (m_DMXView->isEnabled())
        m_DMXView->setUniverseFilter(m_universeFilter);
    if (m_2DView->isEnabled())
        m_2DView->setUniverseFilter(m_universeFilter);

    emit universeFilterChanged(universeFilter);
}

void ContextManager::checkDumpButton(quint32 valCount)
{
    int dumpValuesCount = m_functionManager->dumpValuesCount();
    /** Monitor the changes from/to 0 */
    if ((valCount == 0 && dumpValuesCount > 0) ||
            (valCount > 0 && dumpValuesCount == 0))
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

void ContextManager::slotNewFixtureCreated(quint32 fxID, qreal x, qreal y, qreal z)
{
    Q_UNUSED(z)

    if (m_DMXView->isEnabled())
        m_DMXView->createFixtureItem(fxID);
    if (m_2DView->isEnabled())
        m_2DView->createFixtureItem(fxID, x, y, false);
}

void ContextManager::slotChannelValueChanged(quint32 fxID, quint32 channel, quint8 value)
{
    if (m_editingEnabled == false)
    {
        quint32 valCount = m_functionManager->dumpValuesCount();
        m_source->set(fxID, channel, (uchar)value);
        m_functionManager->setDumpValue(fxID, channel, (uchar)value);
        checkDumpButton(valCount);
    }
    else
        m_functionManager->setChannelValue(fxID, channel, (uchar)value);
}

void ContextManager::slotChannelTypeValueChanged(int type, quint8 value, quint32 channel)
{
    quint32 valCount = m_functionManager->dumpValuesCount();
    //qDebug() << "type:" << type << "value:" << value << "channel:" << channel;
    QList<SceneValue> svList = m_channelsMap.values(type);
    foreach(SceneValue sv, svList)
    {
        if (channel == UINT_MAX || (channel != UINT_MAX && channel == sv.channel))
        {
            if (m_editingEnabled == false)
            {
                m_source->set(sv.fxi, sv.channel, (uchar)value);
                m_functionManager->setDumpValue(sv.fxi, sv.channel, (uchar)value);
            }
            else
                m_functionManager->setChannelValue(sv.fxi, sv.channel, (uchar)value);
        }
    }

    if (m_editingEnabled == false)
        checkDumpButton(valCount);
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

void ContextManager::slotPositionChanged(int type, int degrees)
{
    // list to keep track of the already processed Fixture IDs
    QList<quint32>fxIDs;
    quint32 valCount = m_functionManager->dumpValuesCount();
    QList<SceneValue> typeList = m_channelsMap.values(type);

    foreach(SceneValue sv, typeList)
    {
        if (fxIDs.contains(sv.fxi) == true)
            continue;

        fxIDs.append(sv.fxi);

        QList<SceneValue> svList = m_fixtureManager->getFixturePosition(sv.fxi, type, degrees);
        foreach(SceneValue posSv, svList)
        {
            if (m_editingEnabled == false)
            {
                m_source->set(posSv.fxi, posSv.channel, posSv.value);
                m_functionManager->setDumpValue(posSv.fxi, posSv.channel, posSv.value);
            }
            else
                m_functionManager->setChannelValue(posSv.fxi, posSv.channel, posSv.value);
        }
    }

    if (m_editingEnabled == false)
        checkDumpButton(valCount);
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

        if (fixture->setChannelValues(ua) == true)
        {
            if (m_DMXView->isEnabled())
                m_DMXView->updateFixture(fixture);
            if (m_2DView->isEnabled())
                m_2DView->updateFixture(fixture);
        }
    }
}

void ContextManager::slotFunctionEditingChanged(bool status)
{
    resetContexts();
    m_editingEnabled = status;
}



