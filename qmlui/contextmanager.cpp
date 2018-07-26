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
#include "fixtureutils.h"
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
    , m_monProps(doc->monitorProperties())
    , m_fixtureManager(fxMgr)
    , m_functionManager(funcMgr)
    , m_multipleSelection(false)
    , m_positionPicking(false)
    , m_universeFilter(Universe::invalid())
    , m_editingEnabled(false)
    , m_dumpChannelMask(0)
{
    m_view->rootContext()->setContextProperty("contextManager", this);

    /** Create and enable a DMX source used for dumping */
    m_source = new GenericDMXSource(m_doc);
    m_source->setOutputEnabled(true);

    /** Set the initial point of view for the 2D preview */
    m_doc->monitorProperties()->setPointOfView(MonitorProperties::TopView);

    /** Create and register the 4 contexts handled by this class */
    m_uniGridView = new PreviewContext(m_view, m_doc, "UNIGRID");
    m_uniGridView->setContextResource("qrc:/UniverseGridView.qml");
    m_uniGridView->setContextTitle(tr("Universe Grid View"));
    registerContext(m_uniGridView);

    m_DMXView = new MainViewDMX(m_view, m_doc);
    registerContext(m_DMXView);
    m_view->rootContext()->setContextProperty("ViewDMX", m_DMXView);

    m_2DView = new MainView2D(m_view, m_doc);
    registerContext(m_2DView);
    m_view->rootContext()->setContextProperty("View2D", m_2DView);

    m_3DView = new MainView3D(m_view, m_doc);
    registerContext(m_3DView);
    m_view->rootContext()->setContextProperty("View3D", m_3DView);

    qmlRegisterUncreatableType<MonitorProperties>("org.qlcplus.classes", 1, 0, "MonitorProperties", "Can't create MonitorProperties!");

    connect(m_fixtureManager, &FixtureManager::newFixtureCreated, this, &ContextManager::slotNewFixtureCreated);
    connect(m_fixtureManager, &FixtureManager::fixtureDeleted, this, &ContextManager::slotFixtureDeleted);
    connect(m_fixtureManager, &FixtureManager::fixtureFlagsChanged, this, &ContextManager::slotFixtureFlagsChanged);

    connect(m_fixtureManager, &FixtureManager::channelValueChanged, this, &ContextManager::slotChannelValueChanged);
    connect(m_fixtureManager, SIGNAL(channelTypeValueChanged(int, quint8)),
            this, SLOT(slotChannelTypeValueChanged(int, quint8)));
    connect(m_fixtureManager, &FixtureManager::colorChanged, this, &ContextManager::slotColorChanged);
    connect(m_fixtureManager, &FixtureManager::positionTypeValueChanged, this, &ContextManager::slotPositionChanged);
    connect(m_fixtureManager, &FixtureManager::presetChanged, this, &ContextManager::slotPresetChanged);
    //connect(m_doc->inputOutputMap(), &InputOutputMap::universesWritten, this, &ContextManager::slotUniversesWritten);
    connect(m_doc->inputOutputMap(), SIGNAL(universesWritten(int,QByteArray)), this, SLOT(slotUniversesWritten(int,QByteArray)));
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

QVector3D ContextManager::environmentSize() const
{
    return m_monProps->gridSize();
}

void ContextManager::setEnvironmentSize(QVector3D environmentSize)
{
    if (environmentSize == m_monProps->gridSize())
        return;

    Tardis::instance()->enqueueAction(Tardis::EnvironmentSetSize, 0, m_monProps->gridSize(), environmentSize);

    m_monProps->setGridSize(environmentSize);
    if (m_2DView->isEnabled())
        m_2DView->setGridSize(environmentSize);
    if (m_3DView->isEnabled())
    {
        for(Fixture *fixture : m_doc->fixtures()) // C++11
        {
            for (quint32 subID : m_monProps->fixtureIDList(fixture->id()))
            {
                quint16 headIndex = m_monProps->fixtureHeadIndex(subID);
                quint16 linkedIndex = m_monProps->fixtureLinkedIndex(subID);
                quint32 itemID = FixtureUtils::fixtureItemID(fixture->id(), headIndex, linkedIndex);
                m_3DView->updateFixturePosition(itemID, m_monProps->fixturePosition(fixture->id(), headIndex, linkedIndex));
            }
        }
    }
    emit environmentSizeChanged();
}

bool ContextManager::multipleSelection() const
{
    return m_multipleSelection;
}

void ContextManager::setMultipleSelection(bool multipleSelection)
{
    if (m_multipleSelection == multipleSelection)
        return;

    m_multipleSelection = multipleSelection;
    emit multipleSelectionChanged();
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

    point = QVector3D(point.x() + m_monProps->gridSize().x() / 2, point.y(), point.z() + m_monProps->gridSize().z() / 2);

    for (quint32 itemID : m_selectedFixtures)
    {
        quint32 fxID = FixtureUtils::itemFixtureID(itemID);
        quint16 headIndex = FixtureUtils::itemHeadIndex(itemID);
        quint16 linkedIndex = FixtureUtils::itemLinkedIndex(itemID);

        Fixture *fixture = m_doc->fixture(fxID);
        if (fixture == NULL)
            continue;

        quint32 panMSB = fixture->channelNumber(QLCChannel::Pan, QLCChannel::MSB);
        quint32 tiltMSB = fixture->channelNumber(QLCChannel::Tilt, QLCChannel::MSB);

        // don't even bother if the fixture doesn't have PAN/TILT channels
        if (panMSB == QLCChannel::invalid() && tiltMSB == QLCChannel::invalid())
            continue;

        QVector3D lightPos = m_3DView->lightPosition(itemID);
        lightPos = QVector3D(lightPos.x() + m_monProps->gridSize().x() / 2,
                             lightPos.y(),
                             lightPos.z() + m_monProps->gridSize().z() / 2);

        qDebug() << "3D point picked:" << point << "light position:" << lightPos;

        if (panMSB != QLCChannel::invalid())
        {
            bool xLeft = point.x() < lightPos.x();
            bool zBack = point.z() < lightPos.z();
            qreal b = qAbs(lightPos.x() - point.x()); // Cathetus
            qreal c = qAbs(lightPos.z() - point.z()); // Cathetus
            qreal panDeg = qRadiansToDegrees(M_PI_2 - qAtan(c / b)); // PI/2 - angle

            if (qAbs(m_monProps->fixtureRotation(fxID, headIndex, linkedIndex).x()) == 180)
            {
                if (xLeft && zBack)
                    panDeg = 90.0 + (90.0 - panDeg);
                else if(!xLeft && !zBack)
                    panDeg = 270.0 + (90.0 - panDeg);
                else if(!xLeft && zBack)
                    panDeg = 180.0 + panDeg;
            }
            else
            {
                if (xLeft && !zBack)
                    panDeg = 90.0 + (90.0 - panDeg);
                else if(!xLeft && !zBack)
                    panDeg = 180.0 + panDeg;
                else if(!xLeft && zBack)
                    panDeg = 270.0 + (90.0 - panDeg);
            }

            // subtract the current fixture Y rotation
            panDeg -= m_monProps->fixtureRotation(fxID, headIndex, linkedIndex).y();
            if (panDeg < 0)
                panDeg += 360;

            qDebug() << "Fixture" << fxID << "pan degrees:" << panDeg;

            QList<SceneValue> svList = m_fixtureManager->getFixturePosition(fxID, QLCChannel::Pan, panDeg);
            for (SceneValue posSv : svList)
            {
                if (m_editingEnabled == false)
                    setDumpValue(posSv.fxi, posSv.channel, posSv.value);
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

            if (qAbs(m_monProps->fixtureRotation(fxID, headIndex, linkedIndex).x()) == 180)
                tiltDeg = qMin((qreal)phy.focusTiltMax(), phy.focusTiltMax() / 2 + (180 - tiltDeg));
            else
                tiltDeg = phy.focusTiltMax() / 2 - tiltDeg;

            qDebug() << "Fixture" << fxID << "tilt degrees:" << tiltDeg;

            QList<SceneValue> svList = m_fixtureManager->getFixturePosition(fxID, QLCChannel::Tilt, tiltDeg);
            for (SceneValue posSv : svList)
            {
                if (m_editingEnabled == false)
                    setDumpValue(posSv.fxi, posSv.channel, posSv.value);
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
    for (quint32 itemID : m_selectedFixtures)
        setFixtureSelection(itemID, -1, false);

    m_selectedFixtures.clear();
    m_editingEnabled = false;

    emit environmentSizeChanged();

    if (m_DMXView->isEnabled())
        m_DMXView->slotRefreshView();
    if (m_2DView->isEnabled())
        m_2DView->slotRefreshView();
    if (m_3DView->isEnabled())
        m_3DView->slotRefreshView();

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

void ContextManager::setItemSelection(quint32 itemID, bool enable, int keyModifiers)
{
    qDebug() << "ItemID" << itemID << "enable" << enable << "keymods" << keyModifiers;
    if (enable && keyModifiers == 0 && m_multipleSelection == false)
    {
        resetFixtureSelection();
    }

    quint32 fxID = FixtureUtils::itemFixtureID(itemID);
    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == NULL)
        return;

    if (fixture->type() == QLCFixtureDef::Dimmer)
    {
        setFixtureSelection(itemID, FixtureUtils::itemHeadIndex(itemID), enable);
    }
    else
    {
        setFixtureSelection(itemID, -1, enable);
    }
}

void ContextManager::setFixtureSelection(quint32 itemID, int headIndex, bool enable)
{
    quint32 fixtureID = FixtureUtils::itemFixtureID(itemID);
    int linkedIndex = FixtureUtils::itemLinkedIndex(itemID);
    int headIdx = FixtureUtils::itemHeadIndex(itemID);

    if (m_selectedFixtures.contains(itemID))
    {
        if (enable == false)
            m_selectedFixtures.removeAll(itemID);
        else
            return;
    }
    else
    {
        if (enable)
        {
            quint32 flags = m_monProps->fixtureFlags(fixtureID, headIdx, linkedIndex);

            // do not even select a hidden item
            if (flags & MonitorProperties::HiddenFlag)
                return;

            m_selectedFixtures.append(itemID);
        }
        else
            return;
    }

    emit dumpValuesCountChanged();

    Fixture *fixture = m_doc->fixture(fixtureID);

    if (m_DMXView->isEnabled())
        m_DMXView->updateFixtureSelection(fixtureID, enable);

    if (headIndex == -1 && fixture->type() == QLCFixtureDef::Dimmer)
    {
        for (quint32 subID : m_monProps->fixtureIDList(fixtureID))
        {
            quint16 hIndex = m_monProps->fixtureHeadIndex(subID);
            quint16 lIndex = m_monProps->fixtureLinkedIndex(subID);

            if (lIndex != linkedIndex)
                continue;

            quint32 id = FixtureUtils::fixtureItemID(fixtureID, hIndex, linkedIndex);

            if (m_2DView->isEnabled())
                m_2DView->updateFixtureSelection(id, enable);
            if (m_3DView->isEnabled())
                m_3DView->updateFixtureSelection(id, enable);
        }
    }
    else
    {
        if (m_2DView->isEnabled())
            m_2DView->updateFixtureSelection(itemID, enable);
        if (m_3DView->isEnabled())
            m_3DView->updateFixtureSelection(itemID, enable);
    }

    QMultiHash<int, SceneValue> channels = m_fixtureManager->getFixtureCapabilities(itemID, headIndex, enable);
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

    // parachute if we get out of sync
    if (m_selectedFixtures.isEmpty())
        m_fixtureManager->resetCapabilities();
}

void ContextManager::setFixtureIDSelection(quint32 fixtureID, bool enable)
{
    for (quint32 subID : m_monProps->fixtureIDList(fixtureID))
    {
        quint16 headIndex = m_monProps->fixtureHeadIndex(subID);
        quint16 linkedIndex = m_monProps->fixtureLinkedIndex(subID);

        if (headIndex != 0)
            continue;

        quint32 itemID = FixtureUtils::fixtureItemID(fixtureID, headIndex, linkedIndex);
        setFixtureSelection(itemID, -1, enable);
    }
}

void ContextManager::resetFixtureSelection()
{
    for (Fixture *fixture : m_doc->fixtures()) // C++11
    {
        if (fixture == NULL)
            continue;

        for (quint32 subID : m_monProps->fixtureIDList(fixture->id()))
        {
            quint16 headIndex = m_monProps->fixtureHeadIndex(subID);
            quint16 linkedIndex = m_monProps->fixtureLinkedIndex(subID);
            quint32 itemID = FixtureUtils::fixtureItemID(fixture->id(), headIndex, linkedIndex);
            setFixtureSelection(itemID, -1, false);
        }
    }
}

void ContextManager::toggleFixturesSelection()
{
    bool selectAll = true;
    int visibleCount = 0;

    for (quint32 fixtureID : m_monProps->fixtureItemsID())
    {
        for (quint32 subID : m_monProps->fixtureIDList(fixtureID))
        {
            quint16 headIndex = m_monProps->fixtureHeadIndex(subID);
            quint16 linkedIndex = m_monProps->fixtureLinkedIndex(subID);
            int flags = m_monProps->fixtureFlags(fixtureID, headIndex, linkedIndex);
            if (!(flags & MonitorProperties::HiddenFlag))
                visibleCount++;
        }
    }

    if (m_selectedFixtures.count() == visibleCount)
        selectAll = false;

    for (Fixture *fixture : m_doc->fixtures()) // C++11
    {
        if (fixture == NULL)
            continue;

        for (quint32 subID : m_monProps->fixtureIDList(fixture->id()))
        {
            quint16 headIndex = m_monProps->fixtureHeadIndex(subID);
            quint16 linkedIndex = m_monProps->fixtureLinkedIndex(subID);
            quint32 itemID = FixtureUtils::fixtureItemID(fixture->id(), headIndex, linkedIndex);
            setFixtureSelection(itemID, -1, selectAll);
        }
    }
}

void ContextManager::setRectangleSelection(qreal x, qreal y, qreal width, qreal height)
{
    QList<quint32> fxIDList;

    if (m_2DView->isEnabled())
        fxIDList = m_2DView->selectFixturesRect(QRectF(x, y, width, height));

    for (quint32 itemID : fxIDList)
        setFixtureSelection(itemID, -1, true);

    emit selectedFixturesChanged();
}

int ContextManager::selectedFixturesCount()
{
    return m_selectedFixtures.count();
}

bool ContextManager::isFixtureSelected(quint32 itemID)
{
    return m_selectedFixtures.contains(itemID) ? true : false;
}

void ContextManager::setFixturePosition(quint32 itemID, qreal x, qreal y, qreal z)
{
    quint32 fxID = FixtureUtils::itemFixtureID(itemID);
    quint16 headIndex = FixtureUtils::itemHeadIndex(itemID);
    quint16 linkedIndex = FixtureUtils::itemLinkedIndex(itemID);
    QVector3D currPos = m_monProps->fixturePosition(fxID, headIndex, linkedIndex);
    QVector3D newPos(x, y, z);

    Tardis::instance()->enqueueAction(Tardis::FixtureSetPosition, itemID, QVariant(currPos), QVariant(newPos));
    m_monProps->setFixturePosition(fxID, headIndex, linkedIndex, newPos);

    if (m_2DView->isEnabled())
        m_2DView->updateFixturePosition(itemID, newPos);
    if (m_3DView->isEnabled())
        m_3DView->updateFixturePosition(itemID, newPos);
}

void ContextManager::setFixturesOffset(qreal x, qreal y)
{
    for (quint32 itemID : m_selectedFixtures)
    {
        quint32 fxID = FixtureUtils::itemFixtureID(itemID);
        quint16 headIndex = FixtureUtils::itemHeadIndex(itemID);
        quint16 linkedIndex = FixtureUtils::itemLinkedIndex(itemID);
        QVector3D currPos = m_monProps->fixturePosition(fxID, headIndex, linkedIndex);
        QVector3D newPos;

        switch (m_monProps->pointOfView())
        {
            case MonitorProperties::TopView:
                newPos = QVector3D(currPos.x() + x, currPos.y(), currPos.z() + y);
            break;
            case MonitorProperties::RightSideView:
                newPos = QVector3D(currPos.x(), currPos.y() + y, currPos.z() - x);
            break;
            case MonitorProperties::LeftSideView:
                newPos = QVector3D(currPos.x(), currPos.y() + y, currPos.z() + x);
            break;
            default:
                newPos = QVector3D(currPos.x() + x, currPos.y() - y, currPos.z());
            break;
        }

        Tardis::instance()->enqueueAction(Tardis::FixtureSetPosition, itemID, QVariant(currPos), QVariant(newPos));
        m_monProps->setFixturePosition(fxID, headIndex, linkedIndex, newPos);
        if (m_2DView->isEnabled())
            m_2DView->updateFixturePosition(itemID, newPos);
        if (m_3DView->isEnabled())
            m_3DView->updateFixturePosition(itemID, newPos);
    }
}

QVector3D ContextManager::fixturesPosition() const
{
    if (m_selectedFixtures.count() == 1)
    {
        quint32 fxID = FixtureUtils::itemFixtureID(m_selectedFixtures.first());
        quint16 headIndex = FixtureUtils::itemHeadIndex(m_selectedFixtures.first());
        quint16 linkedIndex = FixtureUtils::itemLinkedIndex(m_selectedFixtures.first());
        return m_monProps->fixturePosition(fxID, headIndex, linkedIndex);
    }

    return QVector3D(0, 0, 0);
}

void ContextManager::setFixturesPosition(QVector3D position)
{
    if (m_selectedFixtures.count() == 1)
    {
        quint32 fxID = FixtureUtils::itemFixtureID(m_selectedFixtures.first());
        quint16 headIndex = FixtureUtils::itemHeadIndex(m_selectedFixtures.first());
        quint16 linkedIndex = FixtureUtils::itemLinkedIndex(m_selectedFixtures.first());

        // absolute position change
        m_monProps->setFixturePosition(fxID, headIndex, linkedIndex, position);
        if (m_3DView->isEnabled())
            m_3DView->updateFixturePosition(m_selectedFixtures.first(), position);
    }
    else
    {
        // relative position change
        for (quint32 itemID : m_selectedFixtures)
        {
            quint32 fxID = FixtureUtils::itemFixtureID(itemID);
            quint16 headIndex = FixtureUtils::itemHeadIndex(itemID);
            quint16 linkedIndex = FixtureUtils::itemLinkedIndex(itemID);

            QVector3D newPos = m_monProps->fixturePosition(fxID, headIndex, linkedIndex) + position;
            m_monProps->setFixturePosition(fxID, headIndex, linkedIndex, newPos);
            if (m_3DView->isEnabled())
                m_3DView->updateFixturePosition(itemID, newPos);
        }
    }

    emit fixturesPositionChanged();
}

void ContextManager::setFixturesAlignment(int alignment)
{
    if (m_selectedFixtures.count() == 0)
        return;

    quint32 firstFxID = FixtureUtils::itemFixtureID(m_selectedFixtures.first());
    quint16 firstHeadIndex = FixtureUtils::itemHeadIndex(m_selectedFixtures.first());
    quint16 firstLinkedIndex = FixtureUtils::itemLinkedIndex(m_selectedFixtures.first());
    QVector3D firstPos = m_monProps->fixturePosition(firstFxID, firstHeadIndex, firstLinkedIndex);

    for (quint32 itemID : m_selectedFixtures)
    {
        quint32 fxID = FixtureUtils::itemFixtureID(itemID);
        quint16 headIndex = FixtureUtils::itemHeadIndex(itemID);
        quint16 linkedIndex = FixtureUtils::itemLinkedIndex(itemID);
        QVector3D fxPos = m_monProps->fixturePosition(fxID, headIndex, linkedIndex);

        FixtureUtils::alignItem(firstPos, fxPos, m_monProps->pointOfView(), alignment);
        m_monProps->setFixturePosition(fxID, headIndex, linkedIndex, fxPos);
        if (m_2DView->isEnabled())
            m_2DView->updateFixturePosition(itemID, fxPos);
        if (m_3DView->isEnabled())
            m_3DView->updateFixturePosition(itemID, fxPos);
    }
}

void ContextManager::setFixturesDistribution(int direction)
{
    if (m_selectedFixtures.count() < 3)
        return;

    qreal min = 1000000;
    qreal max = 0;
    qreal fixturesSize = 0;
    qreal gap = 0;
    QVector<quint32> sortedIDs;
    QVector<quint32> sortedPos;

    /* cycle through selected fixtures and do the following:
     * 1- calculate the total width/height
     * 2- sort the fixture IDs from the leftmost/topmost item
     * 3- detect the minimum and maximum items position
     */
    for (quint32 itemID : m_selectedFixtures)
    {
        quint32 fxID = FixtureUtils::itemFixtureID(itemID);
        quint16 headIndex = FixtureUtils::itemHeadIndex(itemID);
        quint16 linkedIndex = FixtureUtils::itemLinkedIndex(itemID);
        Fixture *fixture = m_doc->fixture(fxID);
        QPointF fxPos = FixtureUtils::item2DPosition(m_monProps, m_monProps->pointOfView(),
                                                     m_monProps->fixturePosition(fxID, headIndex, linkedIndex));
        QSizeF fxRect = FixtureUtils::item2DDimension(fixture->fixtureMode(), m_monProps->pointOfView());
        qreal pos = direction == Qt::Horizontal ? fxPos.x() : fxPos.y();
        qreal size = direction == Qt::Horizontal ? fxRect.width() : fxRect.height();
        int i = 0;

        // 1
        fixturesSize += size;

        // 2
        for (i = 0; i < sortedPos.count(); i++)
        {
            if (pos < sortedPos[i])
                break;
        }
        if (sortedPos.isEmpty() || i == sortedIDs.count())
        {
            sortedIDs.append(itemID);
            sortedPos.append(pos);
        }
        else
        {
            sortedIDs.insert(i, itemID);
            sortedPos.insert(i, pos);
        }

        // 3
        if (pos + size > max)
            max = pos + size;
        if (pos < min)
            min = pos;
    }

    gap = ((max - min) - fixturesSize) / (sortedIDs.count() - 1);

    qDebug() << "Sorted IDs:" << sortedIDs << "min/max:" << min << max;

    qreal newPos = min;

    for (int idx = 0; idx < sortedIDs.count(); idx++)
    {
        quint32 itemID = sortedIDs[idx];
        quint32 fxID = FixtureUtils::itemFixtureID(itemID);
        quint16 headIndex = FixtureUtils::itemHeadIndex(itemID);
        quint16 linkedIndex = FixtureUtils::itemLinkedIndex(itemID);
        Fixture *fixture = m_doc->fixture(fxID);
        QSizeF fxRect = FixtureUtils::item2DDimension(fixture->fixtureMode(), m_monProps->pointOfView());
        qreal size = direction == Qt::Horizontal ? fxRect.width() : fxRect.height();
        QVector3D fxPos = m_monProps->fixturePosition(fxID, headIndex, linkedIndex);

        // the first and last fixture don't need any adjustment
        if (idx > 0 && idx < sortedIDs.count() - 1)
        {
            switch(m_monProps->pointOfView())
            {
                case MonitorProperties::TopView:
                    if (direction == Qt::Horizontal)
                        fxPos.setX(newPos);
                    else
                        fxPos.setZ(newPos);
                break;
                case MonitorProperties::RightSideView:
                    if (direction == Qt::Horizontal)
                        fxPos.setZ(m_monProps->gridSize().z() - newPos);
                    else
                        fxPos.setY(newPos);
                break;
                case MonitorProperties::LeftSideView:
                    if (direction == Qt::Horizontal)
                        fxPos.setZ(newPos);
                    else
                        fxPos.setY(newPos);
                break;
                default:
                    if (direction == Qt::Horizontal)
                        fxPos.setX(newPos);
                    else
                        fxPos.setY(newPos);
                break;
            }

            m_monProps->setFixturePosition(fxID, headIndex, linkedIndex, fxPos);
            if (m_2DView->isEnabled())
                m_2DView->updateFixturePosition(itemID, fxPos);
            if (m_3DView->isEnabled())
                m_3DView->updateFixturePosition(itemID, fxPos);
        }

        newPos += size + gap;
    }
}

void ContextManager::setLinkedFixture(quint32 itemID)
{
    quint32 fixtureID = FixtureUtils::itemFixtureID(itemID);
    int headIndex = FixtureUtils::itemHeadIndex(itemID);
    int linkedIndex = FixtureUtils::itemLinkedIndex(itemID);

    if (linkedIndex)
    {
        // remove an existing linked fixture - itemID is the linked fixture

        // 1- remove the node from Fixture Manager
        m_fixtureManager->updateLinkedFixtureNode(itemID, false);

        // 2- remove the item from previews
        if (m_2DView->isEnabled())
            m_2DView->removeFixtureItem(itemID);
        if (m_3DView->isEnabled())
            m_3DView->removeFixtureItem(itemID);

        // 3- remove the item from Monitor properties
        m_monProps->removeFixture(fixtureID, headIndex, linkedIndex);
    }
    else
    {
        // create a new linked fixture - itemID is the base fixture
        int newIndex = 1;
        Fixture *fixture = m_doc->fixture(fixtureID);
        if (fixture == NULL)
            return;

        // 1- iterate through Fixture subitems to find a new linked index
        for (quint32 subID : m_monProps->fixtureIDList(fixtureID))
        {
            quint16 hIdx = m_monProps->fixtureHeadIndex(subID);
            quint16 lIdx = m_monProps->fixtureLinkedIndex(subID);
            if (hIdx != headIndex)
                continue;

            if (lIdx >= newIndex)
                newIndex = lIdx + 1;
        }
        // 2- find a position for the new item
        QVector3D pos = m_monProps->fixturePosition(fixtureID, headIndex, linkedIndex);
        QLCPhysical phy = fixture->fixtureMode()->physical();
        if (m_monProps->pointOfView() == MonitorProperties::TopView)
            pos.setZ(pos.z() + phy.depth() + 50);
        else
            pos.setY(pos.y() + phy.height() + 50);

        // 3- add the new item to monitor properties
        QString newName = QString("%1 (%2 %3)").arg(fixture->name()).arg(tr("linked")).arg(newIndex);
        m_monProps->setFixturePosition(fixtureID, headIndex, newIndex, pos);
        m_monProps->setFixtureResource(fixtureID, headIndex, newIndex, newName);

        // 4- add the new item to the Fixture Manager tree
        quint32 linkedItemID = FixtureUtils::fixtureItemID(fixtureID, headIndex, newIndex);
        m_fixtureManager->updateLinkedFixtureNode(linkedItemID, true);

        // 5- create the new item in the previews
        if (m_2DView->isEnabled())
            m_2DView->createFixtureItem(fixtureID, headIndex, newIndex, pos, false);
        if (m_3DView->isEnabled())
            m_3DView->createFixtureItem(fixtureID, headIndex, newIndex, pos, false);
    }
}

void ContextManager::updateFixturesCapabilities()
{
    for (quint32 itemID : m_selectedFixtures)
        m_fixtureManager->getFixtureCapabilities(itemID, -1, true);
}

void ContextManager::createFixtureGroup()
{
    if (m_selectedFixtures.isEmpty())
        return;

    m_fixtureManager->addFixturesToNewGroup(selectedFixtureIDList());
}

QVector3D ContextManager::fixturesRotation() const
{
    if (m_selectedFixtures.count() == 1)
    {
        quint32 fixtureID = FixtureUtils::itemFixtureID(m_selectedFixtures.first());
        if (m_monProps->containsFixture(fixtureID) == true)
        {
            quint16 headIndex = FixtureUtils::itemHeadIndex(m_selectedFixtures.first());
            quint16 linkedIndex = FixtureUtils::itemLinkedIndex(m_selectedFixtures.first());

            return m_monProps->fixtureRotation(fixtureID, headIndex, linkedIndex);
        }
    }

    return QVector3D(0, 0, 0);
}

void ContextManager::setFixturesRotation(QVector3D degrees)
{
    if (m_selectedFixtures.count() == 1)
    {
        quint32 fxID = FixtureUtils::itemFixtureID(m_selectedFixtures.first());
        quint16 headIndex = FixtureUtils::itemHeadIndex(m_selectedFixtures.first());
        quint16 linkedIndex = FixtureUtils::itemLinkedIndex(m_selectedFixtures.first());

        // absolute rotation change
        m_monProps->setFixtureRotation(fxID, headIndex, linkedIndex, degrees);
        if (m_2DView->isEnabled())
            m_2DView->updateFixtureRotation(m_selectedFixtures.first(), degrees);
        if (m_3DView->isEnabled())
            m_3DView->updateFixtureRotation(m_selectedFixtures.first(), degrees);
    }
    else
    {
        // relative rotation change
        for (quint32 itemID : m_selectedFixtures)
        {
            quint32 fxID = FixtureUtils::itemFixtureID(itemID);
            quint16 headIndex = FixtureUtils::itemHeadIndex(itemID);
            quint16 linkedIndex = FixtureUtils::itemLinkedIndex(itemID);
            QVector3D newRot = m_monProps->fixtureRotation(fxID, headIndex, linkedIndex) + degrees;

            // normalize back to a 0-359 range
            if (newRot.x() < 0) newRot.setX(newRot.x() + 360);
            else if (newRot.x() >= 360) newRot.setX(newRot.x() - 360);

            if (newRot.y() < 0) newRot.setY(newRot.y() + 360);
            else if (newRot.y() >= 360) newRot.setY(newRot.y() - 360);

            if (newRot.z() < 0) newRot.setZ(newRot.z() + 360);
            else if (newRot.z() >= 360) newRot.setZ(newRot.z() - 360);

            m_monProps->setFixtureRotation(fxID, headIndex, linkedIndex, newRot);
            if (m_2DView->isEnabled())
                m_2DView->updateFixtureRotation(itemID, newRot);
            if (m_3DView->isEnabled())
                m_3DView->updateFixtureRotation(itemID, newRot);
        }
    }

    emit fixturesRotationChanged();
}

void ContextManager::setFixtureGroupSelection(quint32 id, bool enable, bool isUniverse)
{
    if (isUniverse)
    {
        for (Fixture *fixture : m_doc->fixtures())
        {
            if (fixture->universe() == id)
            {
                for (quint32 subID : m_monProps->fixtureIDList(fixture->id()))
                {
                    quint16 headIndex = m_monProps->fixtureHeadIndex(subID);
                    quint16 linkedIndex = m_monProps->fixtureLinkedIndex(subID);
                    quint32 itemID = FixtureUtils::fixtureItemID(fixture->id(), headIndex, linkedIndex);
                    setFixtureSelection(itemID, -1, enable);
                }
            }
        }
    }
    else
    {
        FixtureGroup *group = m_doc->fixtureGroup(id);
        if (group == NULL)
            return;

        for (quint32 fxID : group->fixtureList())
        {
            Fixture *fixture = m_doc->fixture(fxID);
            if (fixture == NULL)
                continue;

            for (quint32 subID : m_monProps->fixtureIDList(fxID))
            {
                quint16 headIndex = m_monProps->fixtureHeadIndex(subID);
                quint16 linkedIndex = m_monProps->fixtureLinkedIndex(subID);
                quint32 itemID = FixtureUtils::fixtureItemID(fxID, headIndex, linkedIndex);
                setFixtureSelection(itemID, -1, enable);
            }
        }
    }
}

void ContextManager::slotNewFixtureCreated(quint32 fxID, qreal x, qreal y, qreal z)
{
    if (m_doc->loadStatus() == Doc::Loading)
        return;

    qDebug() << "[ContextManager] New fixture created" << fxID;

    if (m_DMXView->isEnabled())
        m_DMXView->createFixtureItem(fxID);
    if (m_2DView->isEnabled())
        m_2DView->createFixtureItems(fxID, QVector3D(x, y, z), false);
    if (m_3DView->isEnabled())
        m_3DView->createFixtureItems(fxID, QVector3D(x, y, z), false);
}

void ContextManager::slotFixtureDeleted(quint32 itemID)
{
    if (m_doc->loadStatus() == Doc::Loading)
        return;

    qDebug() << "[ContextManager] Removing item" << itemID;

    if (m_DMXView->isEnabled())
        m_DMXView->removeFixtureItem(FixtureUtils::itemFixtureID(itemID));
    if (m_2DView->isEnabled())
        m_2DView->removeFixtureItem(itemID);
    if (m_3DView->isEnabled())
        m_3DView->removeFixtureItem(itemID);
}

void ContextManager::slotFixtureFlagsChanged(quint32 itemID, quint32 flags)
{
    if (m_DMXView->isEnabled())
        m_DMXView->setFixtureFlags(itemID, flags);
    if (m_2DView->isEnabled())
        m_2DView->setFixtureFlags(itemID, flags);
    if (m_3DView->isEnabled())
        m_3DView->setFixtureFlags(itemID, flags);
}

void ContextManager::slotChannelValueChanged(quint32 fxID, quint32 channel, quint8 value)
{
    if (m_editingEnabled == false)
        setDumpValue(fxID, channel, (uchar)value);
    else
        m_functionManager->setChannelValue(fxID, channel, (uchar)value);
}

void ContextManager::slotChannelTypeValueChanged(int type, quint8 value, quint32 channel)
{
    //qDebug() << "type:" << type << "value:" << value << "channel:" << channel;
    QList<SceneValue> svList = m_channelsMap.values(type);
    for (SceneValue sv : svList)
    {
        if (channel == UINT_MAX || (channel != UINT_MAX && channel == sv.channel))
        {
            if (m_editingEnabled == false)
                setDumpValue(sv.fxi, sv.channel, (uchar)value);
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

    for (SceneValue sv : typeList)
    {
        if (fxIDs.contains(sv.fxi) == true)
            continue;

        fxIDs.append(sv.fxi);

        QList<SceneValue> svList = m_fixtureManager->getFixturePosition(sv.fxi, type, degrees);
        for (SceneValue posSv : svList)
        {
            if (m_editingEnabled == false)
                setDumpValue(posSv.fxi, posSv.channel, posSv.value);
            else
                m_functionManager->setChannelValue(posSv.fxi, posSv.channel, posSv.value);
        }
    }
}

void ContextManager::slotPresetChanged(const QLCChannel *channel, quint8 value)
{
    for (quint32 itemID : m_selectedFixtures)
    {
        quint32 fixtureID = FixtureUtils::itemFixtureID(itemID);
        Fixture *fixture = m_doc->fixture(fixtureID);
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

        QByteArray prevValues;
        prevValues.append(fixture->channelValues());

        if (fixture->setChannelValues(ua) == true)
        {
            if (m_DMXView->isEnabled())
                m_DMXView->updateFixture(fixture);
            if (m_2DView->isEnabled())
                m_2DView->updateFixture(fixture, prevValues);
            if (m_3DView->isEnabled())
                m_3DView->updateFixture(fixture, prevValues);
        }
    }
}

void ContextManager::slotFunctionEditingChanged(bool status)
{
    resetFixtureSelection();
    m_editingEnabled = status;
}

/*********************************************************************
 * DMX channels dump
 *********************************************************************/

void ContextManager::setDumpValue(quint32 fxID, quint32 channel, uchar value)
{
    QVariant currentVal, newVal;
    SceneValue sValue(fxID, channel, value);
    int valIndex = m_dumpValues.indexOf(sValue);
    uchar currDmxValue = valIndex >= 0 ? m_dumpValues.at(valIndex).value : 0;
    currentVal.setValue(SceneValue(fxID, channel, currDmxValue));
    newVal.setValue(sValue);

    if (currentVal != newVal || value != currDmxValue)
    {
        Tardis::instance()->enqueueAction(Tardis::FixtureSetDumpValue, 0, currentVal, newVal);
        if (m_source)
            m_source->set(fxID, channel, value);

        if (valIndex >= 0)
        {
            m_dumpValues.replace(valIndex, sValue);
        }
        else
        {
            m_dumpValues.append(sValue);
            emit dumpValuesCountChanged();

            const QLCChannel *ch = m_doc->fixture(fxID)->channel(channel);
            if (ch != NULL)
            {
                if (ch->group() == QLCChannel::Intensity)
                {
                    if (ch->colour() == QLCChannel::NoColour)
                        m_dumpChannelMask |= DimmerType;
                    else
                        m_dumpChannelMask |= ColorType;
                }
                else
                {
                    m_dumpChannelMask |= (1 << ch->group());
                }
                emit dumpChannelMaskChanged();
            }
        }

        Fixture *fixture = m_doc->fixture(fxID);
        if (fixture)
            fixture->checkAlias(channel, value);
    }
}

QList<quint32> ContextManager::selectedFixtureIDList() const
{
    QList<quint32> fxIDList;

    for (quint32 itemID : m_selectedFixtures)
    {
        quint32 fixtureID = FixtureUtils::itemFixtureID(itemID);
        if (fxIDList.contains(fixtureID) == false)
            fxIDList.append(fixtureID);
    }

    return fxIDList;
}

int ContextManager::dumpValuesCount() const
{
    int i = 0;
    QList<quint32> fxIDList = selectedFixtureIDList();

    for (SceneValue sv : m_dumpValues)
        if (fxIDList.contains(sv.fxi))
            i++;

    return i;
}

int ContextManager::dumpChannelMask() const
{
    return m_dumpChannelMask;
}

void ContextManager::dumpDmxChannels(QString name, quint32 mask)
{
    m_functionManager->dumpOnNewScene(m_dumpValues, selectedFixtureIDList(), mask, name);
}

void ContextManager::dumpDmxChannels(quint32 sceneID, quint32 mask)
{
    m_functionManager->dumpOnScene(m_dumpValues, selectedFixtureIDList(), mask, sceneID);
}

void ContextManager::resetDumpValues()
{
    for (SceneValue sv : m_dumpValues)
        m_source->unset(sv.fxi, sv.channel);

    m_source->unsetAll();

    m_dumpValues.clear();
    emit dumpValuesCountChanged();

    m_dumpChannelMask = 0;
    emit dumpChannelMaskChanged();
}

GenericDMXSource *ContextManager::dmxSource() const
{
    return m_source;
}
