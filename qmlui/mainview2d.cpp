/*
  Q Light Controller Plus
  mainview2d.cpp

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

#include "doc.h"
#include "tardis.h"
#include "mainview2d.h"
#include "fixtureutils.h"
#include "qlccapability.h"
#include "qlcfixturemode.h"
#include "monitorproperties.h"

MainView2D::MainView2D(QQuickView *view, Doc *doc, QObject *parent)
    : PreviewContext(view, doc, "2D", parent)
    , m_monProps(doc->monitorProperties())
{
    setGridSize(m_monProps->gridSize());
    setGridScale(1.0);
    setCellPixels(100);

    m_xOffset = 0;
    m_yOffset = 0;

    m_contents2D = NULL;

    setContextResource("qrc:/2DView.qml");
    setContextTitle(tr("2D View"));

    fixtureComponent = new QQmlComponent(m_view->engine(), QUrl("qrc:/Fixture2DItem.qml"));
    if (fixtureComponent->isError())
        qDebug() << fixtureComponent->errors();
}

MainView2D::~MainView2D()
{
    resetItems();
}

void MainView2D::enableContext(bool enable)
{
    PreviewContext::enableContext(enable);
    if (enable == true)
        slotRefreshView();
    else
        resetItems();
}

void MainView2D::setUniverseFilter(quint32 universeFilter)
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

void MainView2D::resetItems()
{
    QMapIterator<quint32, QQuickItem*> it(m_itemsMap);
    while(it.hasNext())
    {
        it.next();
        delete it.value();
    }
    m_itemsMap.clear();
}

bool MainView2D::initialize2DProperties()
{
    setGridSize(m_monProps->gridSize());

    m_contents2D = qobject_cast<QQuickItem*>(contextItem()->findChild<QObject *>("twoDContents"));

    if (m_contents2D == NULL)
    {
        qDebug() << "ERROR: got invalid contents2D" << m_contents2D;
        return false;
    }

    m_xOffset = m_contents2D->property("x").toReal();
    m_yOffset = m_contents2D->property("y").toReal();

    return true;
}


void MainView2D::createFixtureItem(quint32 fxID, QVector3D pos, bool mmCoords)
{
    if (isEnabled() == false)
        return;

    if (m_contents2D == NULL)
       initialize2DProperties();

    qDebug() << "[MainView2D] Creating fixture with ID" << fxID << "pos:" << pos;

    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == NULL)
        return;

    QLCFixtureMode *fxMode = fixture->fixtureMode();

    QQuickItem *newFixtureItem = qobject_cast<QQuickItem*>(fixtureComponent->create());

    newFixtureItem->setParentItem(m_contents2D);
    newFixtureItem->setProperty("fixtureID", fxID);

    if (fxMode != NULL)
    {
        QLCPhysical phy = fxMode->physical();

        //qDebug() << "Current mode fixture heads:" << fxMode->heads().count();
        newFixtureItem->setProperty("headsNumber", fxMode->heads().count());

        if (fixture->channelNumber(QLCChannel::Pan, QLCChannel::MSB) != QLCChannel::invalid())
        {
            int panDeg = phy.focusPanMax();
            if (panDeg == 0) panDeg = 360;
            newFixtureItem->setProperty("panMaxDegrees", panDeg);
        }
        if (fixture->channelNumber(QLCChannel::Tilt, QLCChannel::MSB) != QLCChannel::invalid())
        {
            int tiltDeg = phy.focusTiltMax();
            if (tiltDeg == 0) tiltDeg = 270;
            newFixtureItem->setProperty("tiltMaxDegrees", tiltDeg);
        }
    }

    QPointF itemPos;
    QSizeF size = FixtureUtils::item2DDimension(fxMode, m_monProps->pointOfView());

    if (mmCoords == false && (pos.x() != 0 || pos.y() != 0))
    {
        float gridUnits = m_monProps->gridUnits() == MonitorProperties::Meters ? 1000.0 : 304.8;
        m_xOffset = m_contents2D->property("x").toReal();
        m_yOffset = m_contents2D->property("y").toReal();
        itemPos.setX(((pos.x() - m_xOffset) * gridUnits) / m_cellPixels);
        itemPos.setY(((pos.y() - m_yOffset) * gridUnits) / m_cellPixels);
    }

    if (m_monProps->hasFixturePosition(fxID))
    {
        itemPos = FixtureUtils::item2DPosition(m_monProps, m_monProps->pointOfView(), pos);
        newFixtureItem->setProperty("rotation", FixtureUtils::item2DRotation(m_monProps->pointOfView(),
                                                                             m_monProps->fixtureRotation(fxID)));
    }
    else
    {
        itemPos = FixtureUtils::available2DPosition(m_doc, m_monProps->pointOfView(),
                                                       QRectF(itemPos.x(), itemPos.y(), size.width(), size.height()));
        // add the new fixture to the Doc monitor properties
        QVector3D newPos = FixtureUtils::item3DPosition(m_monProps, itemPos, 1000.0);
        m_monProps->setFixturePosition(fxID, newPos);
        Tardis::instance()->enqueueAction(FixtureSetPosition, fixture->id(), QVariant(QVector3D(0, 0, 0)), QVariant(newPos));
    }

    newFixtureItem->setProperty("mmXPos", itemPos.x());
    newFixtureItem->setProperty("mmYPos", itemPos.y());
    newFixtureItem->setProperty("mmWidth", size.width());
    newFixtureItem->setProperty("mmHeight", size.height());
    newFixtureItem->setProperty("fixtureName", fixture->name());

    // and finally add the new item to the items map
    m_itemsMap[fxID] = newFixtureItem;

    updateFixture(fixture);
}

QList<quint32> MainView2D::selectFixturesRect(QRectF rect)
{
    QList<quint32>fxList;
    QMapIterator<quint32, QQuickItem *> it(m_itemsMap);
    while (it.hasNext())
    {
        it.next();
        QQuickItem *fxItem = it.value();
        qreal itemXPos = fxItem->property("x").toReal();
        qreal itemYPos = fxItem->property("y").toReal();
        qreal itemWidth = fxItem->property("width").toReal();
        qreal itemHeight = fxItem->property("height").toReal();

        QRectF itemRect(itemXPos, itemYPos, itemWidth, itemHeight);

        qDebug() << "Rect:" << rect << "itemRect:" << itemRect;

        if (rect.contains(itemRect))
        {
            if (fxItem->property("isSelected").toBool() == false)
            {
                fxItem->setProperty("isSelected", true);
                fxList.append(it.key());
            }
        }
    }
    return fxList;
}

void MainView2D::slotRefreshView()
{
    if (isEnabled() == false)
        return;

    resetItems();

    if (initialize2DProperties() == false)
        return;

    if (m_monProps->pointOfView() == MonitorProperties::Undefined)
    {
        QMetaObject::invokeMethod(m_contents2D, "showPovPopup");
        return;
    }

    for (Fixture *fixture : m_doc->fixtures())
    {
        if (m_monProps->hasFixturePosition(fixture->id()))
            createFixtureItem(fixture->id(), m_monProps->fixturePosition(fixture->id()), true);
        else
            createFixtureItem(fixture->id(), QVector3D(0, 0, 0), false);
    }
}

void MainView2D::updateFixture(Fixture *fixture)
{
    if (m_enabled == false || fixture == NULL)
        return;

    if (m_itemsMap.contains(fixture->id()) == false)
        return;

    QQuickItem *fxItem = m_itemsMap[fixture->id()];
    bool setColor = false;
    bool setGobo = false;
    bool setPosition = false;
    int panDegrees = 0;
    int tiltDegrees = 0;

    for (int headIdx = 0; headIdx < fixture->heads(); headIdx++)
    {
        quint32 headDimmerIndex = fixture->channelNumber(QLCChannel::Intensity, QLCChannel::MSB, headIdx);
        //qDebug() << "Head" << headIdx << "dimmer channel:" << mdIndex;
        qreal intValue = 1.0;
        if (headDimmerIndex != QLCChannel::invalid())
            intValue = (qreal)fixture->channelValueAt(headDimmerIndex) / 255;

        QMetaObject::invokeMethod(fxItem, "setHeadIntensity",
                Q_ARG(QVariant, headIdx),
                Q_ARG(QVariant, intValue));

        QMetaObject::invokeMethod(fxItem, "setHeadRGBColor",
                                  Q_ARG(QVariant, headIdx),
                                  Q_ARG(QVariant, FixtureUtils::headColor(m_doc, fixture, headIdx)));
        setColor = true;
    } // for heads

    // now scan all the channels for "common" capabilities
    for (quint32 i = 0; i < fixture->channels(); i++)
    {
        const QLCChannel *ch = fixture->channel(i);
        if (ch == NULL)
            continue;
        uchar value = fixture->channelValueAt(i);

        switch (ch->group())
        {
            case QLCChannel::Pan:
            {
                if (ch->controlByte() == QLCChannel::MSB)
                    panDegrees += (value << 8);
                else
                    panDegrees += (value);
                setPosition = true;
            }
            break;
            case QLCChannel::Tilt:
            {
                if (ch->controlByte() == QLCChannel::MSB)
                    tiltDegrees += (value << 8);
                else
                    tiltDegrees += (value);
                setPosition = true;
            }
            break;
            case QLCChannel::Colour:
            {
                if (setColor && value == 0)
                    break;

                foreach(QLCCapability *cap, ch->capabilities())
                {
                    if (value >= cap->min() && value <= cap->max())
                    {
                        QColor wheelColor1 = cap->resourceColor1();
                        QColor wheelColor2 = cap->resourceColor2();

                        if (wheelColor1.isValid())
                        {
                            if (wheelColor2.isValid())
                            {
                                QMetaObject::invokeMethod(fxItem, "setWheelColor",
                                                          Q_ARG(QVariant, 0),
                                                          Q_ARG(QVariant, wheelColor1),
                                                          Q_ARG(QVariant, wheelColor2));
                            }
                            else
                            {
                                QMetaObject::invokeMethod(fxItem, "setHeadRGBColor",
                                                          Q_ARG(QVariant, 0),
                                                          Q_ARG(QVariant, wheelColor1));
                            }
                            setColor = true;
                        }
                        break;
                    }
                }
            }
            break;
            case QLCChannel::Gobo:
            {
                if (setGobo)
                    break;

                foreach(QLCCapability *cap, ch->capabilities())
                {
                    if (value >= cap->min() && value <= cap->max())
                    {
                        QString resName = cap->resourceName();

                        if(resName.isEmpty() == false && resName.contains("open.png") == false)
                        {
                            QMetaObject::invokeMethod(fxItem, "setGoboPicture",
                                    Q_ARG(QVariant, 0),
                                    Q_ARG(QVariant, resName));
                            // here we don't look for any other gobos, so if a
                            // fixture has more than one gobo wheel, the second
                            // one will be skipped if the first one has been set
                            // to a non-open gobo
                            setGobo = true;
                        }
                    }
                }
            }
            break;
            // ... more preset channels to be added here (Shutter ?)
            default:
            break;
        }
    }
    if (setPosition == true)
    {
        QMetaObject::invokeMethod(fxItem, "setPosition",
                Q_ARG(QVariant, panDegrees),
                Q_ARG(QVariant, tiltDegrees));
    }
}

void MainView2D::updateFixtureSelection(QList<quint32> fixtures)
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

void MainView2D::updateFixtureSelection(quint32 fxID, bool enable)
{
    if (isEnabled() == false || m_itemsMap.contains(fxID) == false)
        return;

    QQuickItem *fxItem = m_itemsMap[fxID];
    fxItem->setProperty("isSelected", enable);

    if (enable)
    {

        QQuickItem *dragArea = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("contentsDragArea"));
        if (dragArea)
        {
            qDebug() << "Reparenting fixture" << fxID << "to drag area";
            fxItem->setParentItem(dragArea);
        }
    }
    else
        fxItem->setParentItem(m_contents2D);
}

void MainView2D::updateFixtureRotation(quint32 fxID, QVector3D degrees)
{
    if (isEnabled() == false || m_itemsMap.contains(fxID) == false)
        return;

    QQuickItem *fxItem = m_itemsMap[fxID];
    fxItem->setProperty("rotation", degrees.y());
}

void MainView2D::updateFixturePosition(quint32 fxID, QVector3D pos)
{
    if (isEnabled() == false)
        return;

    if (m_itemsMap.contains(fxID) == false)
    {
        createFixtureItem(fxID, pos, false);
    }
    else
    {
        QQuickItem *fxItem = m_itemsMap[fxID];
        QPointF point = FixtureUtils::item2DPosition(m_monProps, m_monProps->pointOfView(), pos);
        fxItem->setProperty("mmXPos", point.x());
        fxItem->setProperty("mmYPos", point.y());
    }
}

void MainView2D::removeFixtureItem(quint32 fxID)
{
    if (isEnabled() == false || m_itemsMap.contains(fxID) == false)
        return;

    QQuickItem *fixtureItem = m_itemsMap.take(fxID);
    delete fixtureItem;
}

QSize MainView2D::gridSize() const
{
    return m_gridSize;
}

void MainView2D::setGridSize(QVector3D sz)
{
    switch(m_monProps->pointOfView())
    {
        case MonitorProperties::TopView:
            m_gridSize = QSize(sz.x(), sz.z());
        break;
        case MonitorProperties::LeftSideView:
        case MonitorProperties::RightSideView:
            m_gridSize = QSize(sz.z(), sz.y());
        break;
        //case MonitorProperties::Undefined:
        //case MonitorProperties::FrontView:
        default:
            m_gridSize = QSize(sz.x(), sz.y());
        break;
    }
    emit gridSizeChanged();
}

int MainView2D::gridUnits() const
{
    return m_monProps->gridUnits();
}

void MainView2D::setGridUnits(int units)
{
    if (units == m_monProps->gridUnits())
        return;

    m_monProps->setGridUnits(MonitorProperties::GridUnits(units));
    emit gridUnitsChanged();
}

qreal MainView2D::gridScale() const
{
    return m_gridScale;
}

void MainView2D::setGridScale(qreal gridScale)
{
    if (m_gridScale == gridScale)
        return;

    m_gridScale = gridScale;
    emit gridScaleChanged(gridScale);
}

qreal MainView2D::cellPixels() const
{
    return m_cellPixels;
}

void MainView2D::setCellPixels(qreal cellPixels)
{
    if (m_cellPixels == cellPixels)
        return;

    m_cellPixels = cellPixels;
    emit cellPixelsChanged(cellPixels);
}

int MainView2D::pointOfView() const
{
    return m_monProps->pointOfView();
}

void MainView2D::setPointOfView(int pointOfView)
{
    qDebug() << "Point of view:" << pointOfView;

    if (pointOfView == m_monProps->pointOfView())
        return;

    m_monProps->setPointOfView(MonitorProperties::PointOfView(pointOfView));
    emit pointOfViewChanged(pointOfView);

    setGridSize(m_monProps->gridSize());

    slotRefreshView();
    //for (Fixture *fixture : m_doc->fixtures())
    //    updateFixturePosition(fixture->id(), m_monProps->fixturePosition(fixture->id()));
}





