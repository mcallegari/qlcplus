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
#include "mainview2d.h"
#include "qlccapability.h"
#include "qlcfixturemode.h"
#include "monitorproperties.h"

MainView2D::MainView2D(QQuickView *view, Doc *doc, QObject *parent)
    : PreviewContext(view, doc, "2D", parent)
{
    m_monProps = m_doc->monitorProperties();

    setGridSize(QSize(5 ,5));
    setGridUnits(1000);
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

void MainView2D::initialize2DProperties()
{
    setGridSize(m_monProps->gridSize());
    if (m_monProps->gridUnits() == MonitorProperties::Feet)
        setGridUnits(304.8);
    else
        setGridUnits(1000.0);

    m_contents2D = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("twoDContents"));

    if (m_contents2D == NULL)
    {
        qDebug() << "ERROR: got invalid contents2D" << m_contents2D;
        return;
    }

    m_xOffset = m_contents2D->property("x").toReal();
    m_yOffset = m_contents2D->property("y").toReal();
}

void MainView2D::createFixtureItem(quint32 fxID, qreal x, qreal y, bool mmCoords)
{
    if (isEnabled() == false)
        return;

    if (m_contents2D == NULL)
       initialize2DProperties();

    qDebug() << "[MainView2D] Creating fixture with ID" << fxID << "x:" << x << "y:" << y;

    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == NULL)
        return;

    QLCFixtureMode *fxMode = fixture->fixtureMode();
    QRectF fxRect;

    QQuickItem *newFixtureItem = qobject_cast<QQuickItem*>(fixtureComponent->create());

    newFixtureItem->setParentItem(m_contents2D);
    newFixtureItem->setProperty("fixtureID", fxID);

    if (m_monProps->hasFixturePosition(fxID) == false)
    {
        if (mmCoords == false)
        {
            if (x != 0 || y != 0)
            {
                m_xOffset = m_contents2D->property("x").toReal();
                m_yOffset = m_contents2D->property("y").toReal();
                x = ((x - m_xOffset) * m_gridUnits) / m_cellPixels;
                y = ((y - m_yOffset) * m_gridUnits) / m_cellPixels;
            }
        }
        fxRect.setX(x);
        fxRect.setY(y);
    }
    else
    {
        QVector3D fxOrig = m_monProps->fixturePosition(fxID);
        QVector3D fxRot = m_monProps->fixtureRotation(fxID);
        fxRect.setX(fxOrig.x());
        fxRect.setY(fxOrig.y());
        newFixtureItem->setProperty("rotation", fxRot.y());
    }

    if (fxMode != NULL)
    {
        QLCPhysical phy = fxMode->physical();

        if (phy.width() != 0)
            fxRect.setWidth(phy.width());
        else
            fxRect.setWidth(300);

        if (phy.height() != 0)
            fxRect.setHeight(phy.height());
        else
            fxRect.setHeight(300);

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
    else
    {
        // default to 300x300mm
        fxRect.setWidth(300);
        fxRect.setHeight(300);
    }

    newFixtureItem->setProperty("mmWidth", fxRect.width());
    newFixtureItem->setProperty("mmHeight", fxRect.height());

    if (m_monProps->hasFixturePosition(fxID) == false)
    {
        QPointF availablePos = m_doc->getAvailable2DPosition(fxRect);
        x = availablePos.x();
        y = availablePos.y();
        // add the new fixture to the Doc monitor properties
        m_monProps->setFixturePosition(fxID, QVector3D(x, y, 0));
    }
    else
    {
        QVector3D fxOrig = m_monProps->fixturePosition(fxID);
        x = fxOrig.x();
        y = fxOrig.y();
    }

    newFixtureItem->setProperty("mmXPos", x);
    newFixtureItem->setProperty("mmYPos", y);
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

    initialize2DProperties();

    if (m_contents2D == NULL)
        return;

    foreach(Fixture *fixture, m_doc->fixtures())
    {
        if (m_monProps->hasFixturePosition(fixture->id()))
        {
            QVector3D fxPos = m_monProps->fixturePosition(fixture->id());
            createFixtureItem(fixture->id(), fxPos.x(), fxPos.y());
        }
        else
            createFixtureItem(fixture->id(), 0, 0, false);
    }
}

void MainView2D::updateFixture(Fixture *fixture)
{
    if (m_enabled == false || fixture == NULL)
        return;

    if (m_itemsMap.contains(fixture->id()) == false)
        return;

    QQuickItem *fxItem = m_itemsMap[fixture->id()];
    bool colorSet = false;
    bool goboSet = false;

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

        QVector <quint32> rgbCh = fixture->rgbChannels(headIdx);
        if (rgbCh.size() == 3)
        {
            quint8 r = 0, g = 0, b = 0;
            r = fixture->channelValueAt(rgbCh.at(0));
            g = fixture->channelValueAt(rgbCh.at(1));
            b = fixture->channelValueAt(rgbCh.at(2));

            QMetaObject::invokeMethod(fxItem, "setHeadRGBColor",
                    Q_ARG(QVariant, headIdx),
                    Q_ARG(QVariant, QColor(r, g, b)));
            colorSet = true;
        }

        QVector <quint32> cmyCh = fixture->cmyChannels(headIdx);
        if (cmyCh.size() == 3)
        {
            quint8 c = 0, m = 0, y = 0;
            c = fixture->channelValueAt(cmyCh.at(0));
            m = fixture->channelValueAt(cmyCh.at(1));
            y = fixture->channelValueAt(cmyCh.at(2));
            QColor col;
            col.setCmyk(c, m, y, 0);
            QMetaObject::invokeMethod(fxItem, "setHeadRGBColor",
                    Q_ARG(QVariant, headIdx),
                    Q_ARG(QVariant, QColor(col.red(), col.green(), col.blue())));
            colorSet = true;
        }

        quint32 white = fixture->channelNumber(QLCChannel::White, QLCChannel::MSB, headIdx);
        if (white != QLCChannel::invalid())
            QMetaObject::invokeMethod(fxItem, "setHeadWhite", Q_ARG(QVariant, headIdx), Q_ARG(QVariant, fixture->channelValueAt(white)));

        quint32 amber = fixture->channelNumber(QLCChannel::Amber, QLCChannel::MSB, headIdx);
        if (amber != QLCChannel::invalid())
            QMetaObject::invokeMethod(fxItem, "setHeadAmber", Q_ARG(QVariant, headIdx), Q_ARG(QVariant, fixture->channelValueAt(amber)));

        quint32 UV = fixture->channelNumber(QLCChannel::UV, QLCChannel::MSB, headIdx);
        if (UV != QLCChannel::invalid())
            QMetaObject::invokeMethod(fxItem, "setHeadUV", Q_ARG(QVariant, headIdx), Q_ARG(QVariant, fixture->channelValueAt(UV)));

        if (colorSet == false && headDimmerIndex != QLCChannel::invalid())
        {
            QMetaObject::invokeMethod(fxItem, "setHeadRGBColor",
                    Q_ARG(QVariant, headIdx),
                    Q_ARG(QVariant, QColor(Qt::white)));
        }
    }

    bool setPosition = false;
    int panDegrees = 0;
    int tiltDegrees = 0;

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
                if(colorSet)
                    break;
                foreach(QLCCapability *cap, ch->capabilities())
                {
                    if (value >= cap->min() && value <= cap->max())
                    {
                        QColor wheelColor = cap->resourceColor1();
                        if (wheelColor.isValid())
                        {
                            QMetaObject::invokeMethod(fxItem, "setHeadRGBColor",
                                    Q_ARG(QVariant, 0),
                                    Q_ARG(QVariant, wheelColor));
                            colorSet = true;
                        }
                    }
                }
            }
            break;
            case QLCChannel::Gobo:
            {
                if (goboSet)
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
                            goboSet = true;
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
    if (isEnabled() == false || m_itemsMap.contains(fxID) == false)
        return;

    QQuickItem *fxItem = m_itemsMap[fxID];
    fxItem->setProperty("mmXPos", pos.x());
    fxItem->setProperty("mmYPos", pos.y());
}

QSize MainView2D::gridSize() const
{
    return m_gridSize;
}

void MainView2D::setGridSize(QSize sz)
{
    if (sz != m_gridSize)
    {
        m_gridSize = sz;
        m_monProps->setGridSize(m_gridSize);
        emit gridSizeChanged();
    }
}

float MainView2D::gridUnits() const
{
    return m_gridUnits;
}

void MainView2D::setGridUnits(float units)
{
    if (units != m_gridUnits)
    {
        m_gridUnits = units;
        if (units == 304.8)
            m_monProps->setGridUnits(MonitorProperties::Feet);
        else
            m_monProps->setGridUnits(MonitorProperties::Meters);
        emit gridUnitsChanged();
    }
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





