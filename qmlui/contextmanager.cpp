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
#include <QtMath>

#include "contextmanager.h"
#include "monitorproperties.h"
#include "genericdmxsource.h"
#include "functionmanager.h"
#include "fixturemanager.h"
#include "qlcfixturemode.h"
#include "mainviewdmx.h"
#include "mainview2d.h"
#include "mainview3d.h"
#include "tardis.h"
#include "doc.h"

ContextManager::ContextManager(QQuickView *view, Doc *doc,
                               FixtureManager *fxMgr,
                               FunctionManager *funcMgr,
                               QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
    , m_fixtureManager(fxMgr)
    , m_functionManager(funcMgr)
    , m_positionPicking(false)
    , m_universeFilter(Universe::invalid())
    , m_editingEnabled(false)
{
    m_view->rootContext()->setContextProperty("contextManager", this);

    m_source = new GenericDMXSource(m_doc);
    m_source->setOutputEnabled(true);

    m_uniGridView = new PreviewContext(m_view, m_doc, "UNIGRID");
    m_uniGridView->setContextResource("qrc:/UniverseGridView.qml");
    m_uniGridView->setContextTitle(tr("Universe Grid View"));
    registerContext(m_uniGridView);

    m_2DView = new MainView2D(m_view, m_doc);
    registerContext(m_2DView);
    m_view->rootContext()->setContextProperty("View2D", m_2DView);

    m_3DView = new MainView3D(m_view, m_doc);
    registerContext(m_3DView);
    m_view->rootContext()->setContextProperty("View3D", m_3DView);

    m_DMXView = new MainViewDMX(m_view, m_doc);
    registerContext(m_DMXView);

    connect(m_fixtureManager, &FixtureManager::newFixtureCreated, this, &ContextManager::slotNewFixtureCreated);
    connect(m_fixtureManager, &FixtureManager::fixtureDeleted, this, &ContextManager::slotFixtureDeleted);
    connect(m_fixtureManager, &FixtureManager::channelValueChanged, this, &ContextManager::slotChannelValueChanged);
    connect(m_fixtureManager, SIGNAL(channelTypeValueChanged(int,quint8)),
            this, SLOT(slotChannelTypeValueChanged(int,quint8)));
    connect(m_fixtureManager, &FixtureManager::colorChanged, this, &ContextManager::slotColorChanged);
    connect(m_fixtureManager, &FixtureManager::positionTypeValueChanged, this, &ContextManager::slotPositionChanged);
    connect(m_fixtureManager, &FixtureManager::presetChanged, this, &ContextManager::slotPresetChanged);
    connect(m_doc->inputOutputMap(), &InputOutputMap::universesWritten, this, &ContextManager::slotUniversesWritten);
    connect(m_functionManager, &FunctionManager::isEditingChanged, this, &ContextManager::slotFunctionEditingChanged);
}

ContextManager::~ContextManager()
{
    for (PreviewContext *context : m_contextsMap.values())
    {
        if (context->detached())
            context->deleteLater();
    }

    m_view->rootContext()->setContextProperty("contextManager", NULL);
}

void ContextManager::registerContext(PreviewContext *context)
{
    if (context == NULL)
        return;

    m_contextsMap[context->name()] = context;
    connect(context, &PreviewContext::keyPressed, this, &ContextManager::handleKeyPress);
    connect(context, &PreviewContext::keyReleased, this, &ContextManager::handleKeyRelease);
}

void ContextManager::unregisterContext(QString name)
{
    if (m_contextsMap.contains(name) == false)
        return;

    PreviewContext *context = m_contextsMap.take(name);

    disconnect(context, SIGNAL(keyPressed(QKeyEvent*)),
               this, SLOT(handleKeyPress(QKeyEvent*)));
    disconnect(context, SIGNAL(keyReleased(QKeyEvent*)),
               this, SLOT(handleKeyRelease(QKeyEvent*)));
}

void ContextManager::enableContext(QString name, bool enable, QQuickItem *item)
{
    if (m_contextsMap.contains(name) == false)
        return;

    PreviewContext *context = m_contextsMap[name];

    if (enable == false && context->detached() == true)
        reattachContext(name);

    context->setContextItem(item);
    context->enableContext(enable);

    if (name == "DMX")
        m_DMXView->updateFixtureSelection(m_selectedFixtures);
    else if (name == "2D")
        m_2DView->updateFixtureSelection(m_selectedFixtures);
    else if (name == "3D")
        m_3DView->updateFixtureSelection(m_selectedFixtures);

    emit currentContextChanged();
}

void ContextManager::detachContext(QString name)
{
    qDebug() << "[ContextManager] detaching context:" << name;
    if (m_contextsMap.contains(name) == false)
        return;

    PreviewContext *context = m_contextsMap[name];
    context->setDetached(true);
}

void ContextManager::reattachContext(QString name)
{
    qDebug() << "[ContextManager] reattaching context:" << name;
    if (m_contextsMap.contains(name) == false)
        return;

    PreviewContext *context = m_contextsMap[name];
    context->setDetached(false);

    if (name == "DMX" || name == "2D" || name == "3D" || name == "UNIGRID")
    {
        QQuickItem *viewObj = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("fixturesAndFunctions"));
        if (viewObj == NULL)
            return;
        QMetaObject::invokeMethod(viewObj, "enableContext",
                Q_ARG(QVariant, name),
                Q_ARG(QVariant, false));
    }
    else if (name.startsWith("PAGE-"))
    {
        QQuickItem *viewObj = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("virtualConsole"));
        if (viewObj == NULL)
            return;
        QMetaObject::invokeMethod(viewObj, "enableContext",
                Q_ARG(QVariant, name),
                Q_ARG(QVariant, false));
    }
    else
    {
        QMetaObject::invokeMethod(m_view->rootObject(), "enableContext",
                Q_ARG(QVariant, name),
                Q_ARG(QVariant, false));
    }
}

void ContextManager::switchToContext(QString name)
{
    QString ctxName = name;
    QStringList qlc4names, qlc5names;
    qlc4names << "FixtureManager" << "FunctionManager" << "ShowManager" << "VirtualConsole" << "SimpleDesk" << "InputOutputManager";
    qlc5names << "FIXANDFUNC" << "FIXANDFUNC" << "SHOWMGR" << "VC" << "SDESK" << "IOMGR";

    int ctxIndex = qlc5names.indexOf(name);
    if (ctxIndex < 0)
    {
        ctxIndex = qlc4names.indexOf(name);
        ctxName = qlc5names.at(ctxIndex < 0 ? 0 : ctxIndex);
    }

    QMetaObject::invokeMethod(m_view->rootObject(), "switchToContext",
                              Q_ARG(QVariant, ctxName),
                              Q_ARG(QVariant, QString()));
}

QString ContextManager::currentContext() const
{
    if (m_view == NULL || m_view->rootObject() == NULL)
        return "";

    return m_view->rootObject()->property("currentContext").toString();
}

bool ContextManager::positionPicking() const
{
    return m_positionPicking;
}

void ContextManager::setPositionPicking(bool enable)
{
    if (enable == m_positionPicking)
        return;

    m_positionPicking = enable;

    emit positionPickingChanged();
}

void ContextManager::setPositionPickPoint(QVector3D point)
{
    if (positionPicking() == false)
        return;

    point = QVector3D(point.x() + m_3DView->stageSize().x() / 2,
                      0.0,
                      point.z() + m_3DView->stageSize().z() / 2);

    for (quint32 fxID : m_selectedFixtures)
    {
        Fixture *fixture = m_doc->fixture(fxID);
        if (fixture == NULL)
            continue;

        quint32 panMSB = fixture->channelNumber(QLCChannel::Pan, QLCChannel::MSB);
        quint32 tiltMSB = fixture->channelNumber(QLCChannel::Tilt, QLCChannel::MSB);

        // don't even bother if the fixture doesn't have PAN/TILT channels
        if (panMSB == QLCChannel::invalid() && tiltMSB == QLCChannel::invalid())
            continue;

        QVector3D lightPos = m_3DView->lightPosition(fxID);
        lightPos = QVector3D(lightPos.x() + m_3DView->stageSize().x() / 2,
                             lightPos.y() + 1.0,
                             lightPos.z() + m_3DView->stageSize().z() / 2);

        qDebug() << "3D point picked:" << point << "light position:" << lightPos;

        if (panMSB != QLCChannel::invalid())
        {
            bool xLeft = point.x() < lightPos.x();
            bool zBack = point.z() < lightPos.z();
            qreal b = qAbs(lightPos.x() - point.x()); // Cathetus
            qreal c = qAbs(lightPos.z() - point.z()); // Cathetus
            qreal panDeg = qRadiansToDegrees(M_PI_2 - qAtan(c / b)); // PI/2 - angle

            if (xLeft && !zBack)
                panDeg = 90.0 + (90.0 - panDeg);
            else if(!xLeft && !zBack)
                panDeg = 180.0 + (90.0 - panDeg);
            else if(!xLeft && zBack)
                panDeg = 270.0 + (90.0 - panDeg);

            qDebug() << "Fixture" << fxID << "pan degrees:" << panDeg;

            QList<SceneValue> svList = m_fixtureManager->getFixturePosition(fxID, QLCChannel::Pan, panDeg);
            foreach(SceneValue posSv, svList)
            {
                if (m_editingEnabled == false)
                    m_functionManager->setDumpValue(posSv.fxi, posSv.channel, posSv.value, m_source);
                else
                    m_functionManager->setChannelValue(posSv.fxi, posSv.channel, posSv.value);
            }
        }

        if (tiltMSB != QLCChannel::invalid())
        {
            //bool zBack = point.z() < lightPos.z();
            qreal b1 = qAbs(lightPos.x() - point.x()); // Cathetus
            qreal b2 = qAbs(lightPos.z() - point.z()); // Cathetus
            qreal b = qSqrt(b1 * b1 + b2 * b2); // Hypotenuse
            qreal c = qAbs(lightPos.y() - point.y()); // Cathetus
            qreal tiltDeg = qRadiansToDegrees(M_PI_2 - qAtan(c / b)); // PI/2 - angle
            QLCPhysical phy = fixture->fixtureMode()->physical();

            tiltDeg = phy.focusTiltMax() / 2 - tiltDeg;

            qDebug() << "Fixture" << fxID << "tilt degrees:" << tiltDeg;

            QList<SceneValue> svList = m_fixtureManager->getFixturePosition(fxID, QLCChannel::Tilt, tiltDeg);
            foreach(SceneValue posSv, svList)
            {
                if (m_editingEnabled == false)
                    m_functionManager->setDumpValue(posSv.fxi, posSv.channel, posSv.value, m_source);
                else
                    m_functionManager->setChannelValue(posSv.fxi, posSv.channel, posSv.value);
            }
        }
    }

    setPositionPicking(false);
}

void ContextManager::resetContexts()
{
    m_channelsMap.clear();
    resetDumpValues();
    foreach(quint32 fxID, m_selectedFixtures)
        setFixtureSelection(fxID, false);

    m_selectedFixtures.clear();
    m_editingEnabled = false;
    m_DMXView->enableContext(m_DMXView->isEnabled());
    m_2DView->enableContext(m_2DView->isEnabled());
    m_3DView->enableContext(m_3DView->isEnabled());

    /** TODO: nothing to do on the other contexts ? */
}

void ContextManager::handleKeyPress(QKeyEvent *e)
{
    int key = e->key();

    /* Do not propagate single modifiers events */
    if (key == Qt::Key_Control || key == Qt::Key_Alt || key == Qt::Key_Shift || key == Qt::Key_Meta)
        return;

    qDebug() << "Key press event received:" << e->text();

    if (e->modifiers() & Qt::ControlModifier)
    {
        switch(e->key())
        {
            case Qt::Key_A:
                toggleFixturesSelection();
            break;
            case Qt::Key_R:
                resetDumpValues();
            break;
            case Qt::Key_P:
                setPositionPicking(true);
            break;
            case Qt::Key_Z:
                if (e->modifiers() & Qt::ShiftModifier)
                    Tardis::instance()->redoAction();
                else
                    Tardis::instance()->undoAction();
            break;
            default:
            break;
        }
    }


    for(PreviewContext *context : m_contextsMap.values()) // C++11
        context->handleKeyEvent(e, true);
}

void ContextManager::handleKeyRelease(QKeyEvent *e)
{
    int key = e->key();

    /* Do not propagate single modifiers events */
    if (key == Qt::Key_Control || key == Qt::Key_Alt || key == Qt::Key_Shift || key == Qt::Key_Meta)
        return;

    qDebug() << "Key release event received:" << e->text();

    for(PreviewContext *context : m_contextsMap.values()) // C++11
        context->handleKeyEvent(e, false);
}

/*********************************************************************
 * Universe filtering
 *********************************************************************/

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
    if (m_3DView->isEnabled())
        m_3DView->setUniverseFilter(m_universeFilter);

    emit universeFilterChanged(universeFilter);
}

/*********************************************************************
 * Common fixture helpers
 *********************************************************************/

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
    if (m_3DView->isEnabled())
        m_3DView->updateFixtureSelection(fxID, enable);

    QMultiHash<int, SceneValue> channels = m_fixtureManager->getFixtureCapabilities(fxID, enable);
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
    emit fixturesPositionChanged();
    emit fixturesRotationChanged();
}

void ContextManager::resetFixtureSelection()
{
    for(Fixture *fixture : m_doc->fixtures()) // C++11
    {
        if (fixture != NULL)
            setFixtureSelection(fixture->id(), false);
    }
}

void ContextManager::toggleFixturesSelection()
{
    bool selectAll = true;
    if (m_selectedFixtures.count() == m_doc->fixtures().count())
        selectAll = false;

    for(Fixture *fixture : m_doc->fixtures()) // C++11
    {
        if (fixture != NULL)
            setFixtureSelection(fixture->id(), selectAll);
    }
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

int ContextManager::selectedFixturesCount()
{
    return m_selectedFixtures.count();
}

bool ContextManager::isFixtureSelected(quint32 fxID)
{
    return m_selectedFixtures.contains(fxID) ? true : false;
}

void ContextManager::setFixturePosition(quint32 fxID, qreal x, qreal y, qreal z)
{
    MonitorProperties *mProps = m_doc->monitorProperties();
    QVector3D position(x, y, z);
    Tardis::instance()->enqueueAction(FixtureSetPosition, fxID,
                                      QVariant(mProps->fixturePosition(fxID)),
                                      QVariant(position));

    mProps->setFixturePosition(fxID, position);

    if (m_2DView->isEnabled())
        m_2DView->updateFixturePosition(fxID, position);
    if (m_3DView->isEnabled())
        m_3DView->updateFixturePosition(fxID, position);
}

QVector3D ContextManager::fixturesPosition() const
{
    if (m_selectedFixtures.count() == 1)
    {
        MonitorProperties *mProps = m_doc->monitorProperties();
        return mProps->fixturePosition(m_selectedFixtures.first());
    }

    return QVector3D(0, 0, 0);
}

void ContextManager::setFixturesPosition(QVector3D position)
{
    MonitorProperties *mProps = m_doc->monitorProperties();

    if (m_selectedFixtures.count() == 1)
    {
        // absolute position change
        mProps->setFixturePosition(m_selectedFixtures.first(), position);
        if (m_3DView->isEnabled())
            m_3DView->updateFixturePosition(m_selectedFixtures.first(), position);
    }
    else
    {
        // relative position change
        for (quint32 fxID : m_selectedFixtures)
        {
            QVector3D newPos = mProps->fixturePosition(fxID) + position;
            mProps->setFixturePosition(fxID, newPos);
            if (m_3DView->isEnabled())
                m_3DView->updateFixturePosition(fxID, newPos);
        }
    }

    emit fixturesPositionChanged();
}

void ContextManager::setFixturesAlignment(int alignment)
{
    if (m_selectedFixtures.count() == 0)
        return;

    MonitorProperties *mProps = m_doc->monitorProperties();

    QVector3D firstPos = mProps->fixturePosition(m_selectedFixtures.first());

    foreach(quint32 fxID, m_selectedFixtures)
    {
        QVector3D fxPos = mProps->fixturePosition(fxID);

        switch(alignment)
        {
            case Qt::AlignTop: fxPos.setY(firstPos.y()); break;
            case Qt::AlignLeft: fxPos.setX(firstPos.x()); break;
        }
        mProps->setFixturePosition(fxID, fxPos);
        if (m_2DView->isEnabled())
            m_2DView->updateFixturePosition(fxID, fxPos);
    }
}

void ContextManager::updateFixturesCapabilities()
{
    for (quint32 id : m_selectedFixtures)
        m_fixtureManager->getFixtureCapabilities(id, true);
}

void ContextManager::createFixtureGroup()
{
    if (m_selectedFixtures.isEmpty())
        return;

    m_fixtureManager->addFixturesToNewGroup(m_selectedFixtures);
}

QVector3D ContextManager::fixturesRotation() const
{
    if (m_selectedFixtures.count() == 1)
    {
        MonitorProperties *mProps = m_doc->monitorProperties();
        if (mProps->hasFixturePosition(m_selectedFixtures.first()) == true)
            return mProps->fixtureRotation(m_selectedFixtures.first());
    }

    return QVector3D(0, 0, 0);
}

void ContextManager::setFixturesRotation(QVector3D degrees)
{
    MonitorProperties *mProps = m_doc->monitorProperties();

    if (m_selectedFixtures.count() == 1)
    {
        // absolute rotation change
        mProps->setFixtureRotation(m_selectedFixtures.first(), degrees);
        if (m_2DView->isEnabled())
            m_2DView->updateFixtureRotation(m_selectedFixtures.first(), degrees);
        if (m_3DView->isEnabled())
            m_3DView->updateFixtureRotation(m_selectedFixtures.first(), degrees);
    }
    else
    {
        // relative rotation change
        for (quint32 fxID : m_selectedFixtures)
        {
            QVector3D newRot = mProps->fixtureRotation(fxID) + degrees;

            // normalize back to a 0-359 range
            if (newRot.x() < 0) newRot.setX(newRot.x() + 360);
            else if (newRot.x() >= 360) newRot.setX(newRot.x() - 360);

            if (newRot.y() < 0) newRot.setY(newRot.y() + 360);
            else if (newRot.y() >= 360) newRot.setY(newRot.y() - 360);

            if (newRot.z() < 0) newRot.setZ(newRot.z() + 360);
            else if (newRot.z() >= 360) newRot.setZ(newRot.z() - 360);

            mProps->setFixtureRotation(fxID, newRot);
            if (m_2DView->isEnabled())
                m_2DView->updateFixtureRotation(fxID, newRot);
            if (m_3DView->isEnabled())
                m_3DView->updateFixtureRotation(fxID, newRot);
        }
    }

    emit fixturesRotationChanged();
}

void ContextManager::slotNewFixtureCreated(quint32 fxID, qreal x, qreal y, qreal z)
{
    if (m_doc->loadStatus() == Doc::Loading)
        return;

    qDebug() << "[ContextManager] New fixture created" << fxID;

    if (m_DMXView->isEnabled())
        m_DMXView->createFixtureItem(fxID);
    if (m_2DView->isEnabled())
        m_2DView->createFixtureItem(fxID, x, y, false);
    if (m_3DView->isEnabled())
        m_3DView->createFixtureItem(fxID, x, y, z, false);
}

void ContextManager::slotFixtureDeleted(quint32 fxID)
{
    if (m_doc->loadStatus() == Doc::Loading)
        return;

    qDebug() << "[ContextManager] Removing fixture" << fxID;

    if (m_DMXView->isEnabled())
        m_DMXView->removeFixtureItem(fxID);
    if (m_2DView->isEnabled())
        m_2DView->removeFixtureItem(fxID);
    if (m_3DView->isEnabled())
        m_3DView->removeFixtureItem(fxID);
}

void ContextManager::slotChannelValueChanged(quint32 fxID, quint32 channel, quint8 value)
{
    if (m_editingEnabled == false)
        m_functionManager->setDumpValue(fxID, channel, (uchar)value, m_source);
    else
        m_functionManager->setChannelValue(fxID, channel, (uchar)value);
}

void ContextManager::slotChannelTypeValueChanged(int type, quint8 value, quint32 channel)
{
    //qDebug() << "type:" << type << "value:" << value << "channel:" << channel;
    QList<SceneValue> svList = m_channelsMap.values(type);
    foreach(SceneValue sv, svList)
    {
        if (channel == UINT_MAX || (channel != UINT_MAX && channel == sv.channel))
        {
            if (m_editingEnabled == false)
                m_functionManager->setDumpValue(sv.fxi, sv.channel, (uchar)value, m_source);
            else
                m_functionManager->setChannelValue(sv.fxi, sv.channel, (uchar)value);
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

void ContextManager::slotPositionChanged(int type, int degrees)
{
    // list to keep track of the already processed Fixture IDs
    QList<quint32>fxIDs;
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
                m_functionManager->setDumpValue(posSv.fxi, posSv.channel, posSv.value, m_source);
            else
                m_functionManager->setChannelValue(posSv.fxi, posSv.channel, posSv.value);
        }
    }
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
    for (Fixture *fixture : m_doc->fixtures())
    {
        if (fixture->universe() != (quint32)idx)
            continue;

        if (fixture->setChannelValues(ua) == true)
        {
            if (m_DMXView->isEnabled())
                m_DMXView->updateFixture(fixture);
            if (m_2DView->isEnabled())
                m_2DView->updateFixture(fixture);
            if (m_3DView->isEnabled())
                m_3DView->updateFixture(fixture);
        }
    }
}

void ContextManager::slotFunctionEditingChanged(bool status)
{
    resetContexts();
    m_editingEnabled = status;
}

/*********************************************************************
 * DMX channels dump
 *********************************************************************/

void ContextManager::dumpDmxChannels(QString name)
{
    m_functionManager->dumpOnNewScene(m_selectedFixtures, name);
}

void ContextManager::resetDumpValues()
{
    QMap<QPair<quint32, quint32>, uchar> dumpValues = m_functionManager->dumpValues();
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
}

GenericDMXSource *ContextManager::dmxSource() const
{
    return m_source;
}




