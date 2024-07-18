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
    , m_gridItem(nullptr)
    , m_monProps(doc->monitorProperties())
{
    setGridSize(m_monProps->gridSize());
    setGridScale(1.0);
    setCellPixels(100);

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
    if (enable == isEnabled())
        return;

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
    while (it.hasNext())
    {
        it.next();
        quint32 itemID = it.key();
        QQuickItem *fxItem = it.value();
        quint32 fxID = FixtureUtils::itemFixtureID(itemID);

        Fixture *fixture = m_doc->fixture(fxID);
        if (fixture == nullptr)
            return;

        if (universeFilter == Universe::invalid() || fixture->universe() == universeFilter)
            fxItem->setProperty("visible", "true");
        else
            fxItem->setProperty("visible", "false");
    }
}

void MainView2D::resetItems()
{
    QMapIterator<quint32, QQuickItem*> it(m_itemsMap);
    while (it.hasNext())
    {
        it.next();
        delete it.value();
    }
    m_itemsMap.clear();
}

bool MainView2D::initialize2DProperties()
{
    emit pointOfViewChanged(m_monProps->pointOfView());
    setGridSize(m_monProps->gridSize());

    m_gridItem = qobject_cast<QQuickItem*>(contextItem()->findChild<QObject *>("twoDContents"));

    if (m_gridItem == nullptr)
    {
        qDebug() << "ERROR: got invalid twoDContents" << m_gridItem;
        return false;
    }

    return true;
}

void MainView2D::createFixtureItems(quint32 fxID, QVector3D pos, bool mmCoords)
{
    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == nullptr)
        return;

    if (fixture->type() == QLCFixtureDef::Dimmer)
    {
        for (quint32 i = 0; i < fixture->channels(); i++)
            createFixtureItem(fixture->id(), i, 0, pos, mmCoords);
    }
    else
    {
        createFixtureItem(fixture->id(), 0, 0, pos, mmCoords);
    }
}

void MainView2D::createFixtureItem(quint32 fxID, quint16 headIndex, quint16 linkedIndex,
                                   QVector3D pos, bool mmCoords)
{
    if (isEnabled() == false)
        return;

    if (m_gridItem == nullptr)
       initialize2DProperties();

    qDebug() << "[MainView2D] Creating fixture with ID" << fxID << headIndex << linkedIndex << "pos:" << pos;

    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == nullptr)
        return;

    quint32 itemID = FixtureUtils::fixtureItemID(fxID, headIndex, linkedIndex);
    QLCFixtureMode *fxMode = fixture->fixtureMode();
    QQuickItem *newFixtureItem = qobject_cast<QQuickItem*>(fixtureComponent->create());
    quint32 itemFlags = m_monProps->fixtureFlags(fxID, headIndex, linkedIndex);

    newFixtureItem->setParentItem(m_gridItem);
    newFixtureItem->setProperty("itemID", itemID);
    newFixtureItem->setProperty("fixtureName", fixture->name());

    if (itemFlags & MonitorProperties::HiddenFlag)
        newFixtureItem->setProperty("visible", false);

    if (fxMode != nullptr && fixture->type() != QLCFixtureDef::Dimmer)
    {
        QLCPhysical phy = fxMode->physical();

        if (phy.layoutSize() != QSize(1, 1))
            newFixtureItem->setProperty("headsLayout", phy.layoutSize());

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
    QSizeF size = FixtureUtils::item2DDimension(fixture->type() == QLCFixtureDef::Dimmer ? nullptr : fxMode,
                                                m_monProps->pointOfView());

    if (mmCoords == false && (pos.x() != 0 || pos.y() != 0))
    {
        float gridUnits = m_monProps->gridUnits() == MonitorProperties::Meters ? 1000.0 : 304.8;
        itemPos.setX((pos.x() * gridUnits) / m_cellPixels);
        itemPos.setY((pos.y() * gridUnits) / m_cellPixels);
    }

    if (m_monProps->containsItem(fxID, headIndex, linkedIndex))
    {
        itemPos = FixtureUtils::item2DPosition(m_monProps, m_monProps->pointOfView(), pos);
        newFixtureItem->setProperty("rotation",
                                    FixtureUtils::item2DRotation(m_monProps->pointOfView(),
                                                                 m_monProps->fixtureRotation(fxID, headIndex, linkedIndex)));
    }
    else
    {
        itemPos = FixtureUtils::available2DPosition(m_doc, m_monProps->pointOfView(),
                                                    QRectF(itemPos.x(), itemPos.y(), size.width(), size.height()));
        // add the new fixture to the Doc monitor properties
        QVector3D newPos = FixtureUtils::item3DPosition(m_monProps, itemPos, 1000.0);
        m_monProps->setFixturePosition(fxID, headIndex, linkedIndex, newPos);
        m_monProps->setFixtureFlags(fxID, headIndex, linkedIndex, 0);
        if (fixture->type() == QLCFixtureDef::LEDBarPixels)
        {
            switch (m_monProps->pointOfView())
            {
                case MonitorProperties::FrontView:
                    m_monProps->setFixtureRotation(fxID, headIndex, linkedIndex, QVector3D(90, 0, 0));
                break;
                case MonitorProperties::LeftSideView:
                    m_monProps->setFixtureRotation(fxID, headIndex, linkedIndex, QVector3D(0, -90, 0));
                break;
                case MonitorProperties::RightSideView:
                    m_monProps->setFixtureRotation(fxID, headIndex, linkedIndex, QVector3D(0, 90, 0));
                break;
                default:
                break;
            }
        }
        Tardis::instance()->enqueueAction(Tardis::FixtureSetPosition, itemID, QVariant(QVector3D(0, 0, 0)), QVariant(newPos));
    }

    newFixtureItem->setProperty("mmXPos", itemPos.x());
    newFixtureItem->setProperty("mmYPos", itemPos.y());
    newFixtureItem->setProperty("mmWidth", size.width());
    newFixtureItem->setProperty("mmHeight", size.height());

    // and finally add the new item to the items map
    m_itemsMap[itemID] = newFixtureItem;

    QByteArray values;
    updateFixture(fixture, values);
}

void MainView2D::setFixtureFlags(quint32 itemID, quint32 flags)
{
    QQuickItem *fxItem = m_itemsMap.value(itemID, nullptr);

    if (fxItem == nullptr)
        return;

    fxItem->setProperty("visible", (flags & MonitorProperties::HiddenFlag) ? false : true);
}

QList<quint32> MainView2D::selectFixturesRect(QRectF rect)
{
    QList<quint32>fxList;

    if (rect.width() == 0 || rect.height() == 0)
        return fxList;

    QMap<quint32, QQuickItem *>::const_iterator i = m_itemsMap.constBegin();
    while (i != m_itemsMap.constEnd())
    {
        QQuickItem *fxItem = i.value();
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
                fxList.append(i.key());
            }
        }
        ++i;
    }
    return fxList;
}

int MainView2D::itemIDAtPos(QPointF pos)
{
    QMap<quint32, QQuickItem *>::const_iterator i = m_itemsMap.constBegin();
    while (i != m_itemsMap.constEnd())
    {
        QQuickItem *fxItem = i.value();
        qreal itemXPos = fxItem->property("x").toReal();
        qreal itemYPos = fxItem->property("y").toReal();
        qreal itemWidth = fxItem->property("width").toReal();
        qreal itemHeight = fxItem->property("height").toReal();
        QRectF itemRect(itemXPos, itemYPos, itemWidth, itemHeight);

        qDebug() << "Point:" << pos << "itemRect:" << itemRect;

        if (itemRect.contains(pos))
            return i.key();

        ++i;
    }

    return -1;
}

void MainView2D::slotRefreshView()
{
    if (isEnabled() == false)
        return;

    resetItems();

    if (initialize2DProperties() == false)
        return;

    qDebug() << "Refresh 2D view...";

    if (m_monProps->pointOfView() == MonitorProperties::Undefined)
    {
        QMetaObject::invokeMethod(m_gridItem, "showPovPopup");
        return;
    }

    for (Fixture *fixture : m_doc->fixtures())
    {
        if (m_monProps->containsFixture(fixture->id()))
        {
            for (quint32 subID : m_monProps->fixtureIDList(fixture->id()))
            {
                quint16 headIndex = m_monProps->fixtureHeadIndex(subID);
                quint16 linkedIndex = m_monProps->fixtureLinkedIndex(subID);
                createFixtureItem(fixture->id(), headIndex, linkedIndex,
                                  m_monProps->fixturePosition(fixture->id(), headIndex, linkedIndex), true);
            }
        }
        else
        {
            createFixtureItems(fixture->id(), QVector3D(0, 0, 0), false);
        }
    }
}

void MainView2D::updateFixture(Fixture *fixture, QByteArray &previous)
{
    if (m_enabled == false || fixture == nullptr)
        return;

    for (quint32 subID : m_monProps->fixtureIDList(fixture->id()))
    {
        quint16 headIndex = m_monProps->fixtureHeadIndex(subID);
        quint16 linkedIndex = m_monProps->fixtureLinkedIndex(subID);
        updateFixtureItem(fixture, headIndex, linkedIndex, previous);
    }
}

void MainView2D::updateFixtureItem(Fixture *fixture, quint16 headIndex, quint16 linkedIndex, QByteArray &previous)
{
    quint32 itemID = FixtureUtils::fixtureItemID(fixture->id(), headIndex, linkedIndex);
    QQuickItem *fxItem = m_itemsMap.value(itemID, nullptr);
    QColor color;
    bool colorSet = false;
    bool goboSet = false;
    bool setPosition = false;
    int panDegrees = 0;
    int tiltDegrees = 0;

    if (fxItem == nullptr)
        return;

    // in case of a dimmer pack, headIndex is actually the fixture channel
    // so treat this as a special case and go straight to the point
    if (fixture->type() == QLCFixtureDef::Dimmer)
    {
        qreal value = (qreal)fixture->channelValueAt(headIndex) / 255.0;
        QMetaObject::invokeMethod(fxItem, "setHeadIntensity",
                Q_ARG(QVariant, 0),
                Q_ARG(QVariant, value));

        QColor gelColor = m_monProps->fixtureGelColor(fixture->id(), headIndex, linkedIndex);
        if (gelColor.isValid() == false)
            gelColor = Qt::white;

        QMetaObject::invokeMethod(fxItem, "setHeadRGBColor",
                Q_ARG(QVariant, 0),
                Q_ARG(QVariant, gelColor));

        return;
    }

    quint32 masterDimmerChannel = fixture->masterIntensityChannel();
    qreal masterDimmerValue = masterDimmerChannel != QLCChannel::invalid() ?
                              qreal(fixture->channelValueAt(int(masterDimmerChannel))) / 255.0 : 1.0;

    for (int headIdx = 0; headIdx < fixture->heads(); headIdx++)
    {
        quint32 headDimmerChannel = fixture->channelNumber(QLCChannel::Intensity, QLCChannel::MSB, headIdx);
        if (headDimmerChannel == QLCChannel::invalid())
            headDimmerChannel = masterDimmerChannel;

        //qDebug() << "Head" << headIdx << "dimmer channel:" << headDimmerChannel;
        qreal intensityValue = 1.0;
        bool hasDimmer = false;

        if (headDimmerChannel != QLCChannel::invalid())
        {
            intensityValue = (qreal)fixture->channelValueAt(headDimmerChannel) / 255;
            hasDimmer = true;
        }

        if (headDimmerChannel != masterDimmerChannel)
            intensityValue *= masterDimmerValue;

        QMetaObject::invokeMethod(fxItem, "setHeadIntensity",
                Q_ARG(QVariant, headIdx),
                Q_ARG(QVariant, intensityValue));

        color = FixtureUtils::headColor(fixture, hasDimmer, headIdx);

        QMetaObject::invokeMethod(fxItem, "setHeadRGBColor",
                                  Q_ARG(QVariant, headIdx),
                                  Q_ARG(QVariant, color));
        colorSet = true;
    } // for heads

    // now scan all the channels for "common" capabilities
    for (quint32 i = 0; i < fixture->channels(); i++)
    {
        const QLCChannel *ch = fixture->channel(i);
        if (ch == nullptr)
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
                if (colorSet && value == 0)
                    break;

                QLCCapability *cap = ch->searchCapability(value);

                if (cap == nullptr ||
                   (cap->presetType() != QLCCapability::SingleColor &&
                    cap->presetType() != QLCCapability::DoubleColor))
                    break;

                QColor wheelColor1 = cap->resource(0).value<QColor>();
                QColor wheelColor2 = cap->resource(1).value<QColor>();

                if (wheelColor1.isValid())
                {
                    if (wheelColor2.isValid())
                    {
                        wheelColor1 = FixtureUtils::applyColorFilter(color, wheelColor1);
                        wheelColor2 = FixtureUtils::applyColorFilter(color, wheelColor2);
                        QMetaObject::invokeMethod(fxItem, "setWheelColor",
                                                  Q_ARG(QVariant, 0),
                                                  Q_ARG(QVariant, wheelColor1),
                                                  Q_ARG(QVariant, wheelColor2));
                    }
                    else
                    {
                        wheelColor1 = FixtureUtils::applyColorFilter(color, wheelColor1);
                        QMetaObject::invokeMethod(fxItem, "setHeadRGBColor",
                                                  Q_ARG(QVariant, 0),
                                                  Q_ARG(QVariant, wheelColor1));
                    }
                    colorSet = true;
                }
            }
            break;
            case QLCChannel::Gobo:
            {
                if (goboSet || (previous.length() && value == uchar(previous.at(i))))
                    break;

                QLCCapability *cap = ch->searchCapability(value);

                if (cap == nullptr)
                    break;

                switch (cap->preset())
                {
                    case QLCCapability::GoboMacro:
                    case QLCCapability::GoboShakeMacro:
                    {
                        QString resName = cap->resource(0).toString();

                        if (resName.isEmpty() == false && resName.endsWith("open.svg") == false)
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
                    break;
                    default:
                    break;
                }
            }
            break;
            case QLCChannel::Shutter:
            {
                if (previous.length() && value == uchar(previous.at(i)))
                    break;

                int high = 200, low = 800;
                int capPreset = FixtureUtils::shutterTimings(ch, value, high, low);

                if (capPreset != QLCCapability::Custom)
                {
                    QMetaObject::invokeMethod(fxItem, "setShutter",
                            Q_ARG(QVariant, capPreset),
                            Q_ARG(QVariant, low), Q_ARG(QVariant, high));
                }
            }
            break;
            default:
            break;
        }
    }

    if (setPosition)
    {
        QMetaObject::invokeMethod(fxItem, "setPosition",
                Q_ARG(QVariant, panDegrees),
                Q_ARG(QVariant, tiltDegrees));
    }
}

void MainView2D::selectFixture(QQuickItem *fxItem, bool enable)
{
    if (fxItem == nullptr)
        return;

    fxItem->setProperty("isSelected", enable);

    if (enable)
    {
        QQuickItem *dragArea = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("contentsDragArea"));
        if (dragArea)
            fxItem->setParentItem(dragArea);
    }
    else
    {
        fxItem->setParentItem(m_gridItem);
    }
}

void MainView2D::updateFixtureSelection(QList<quint32> fixtures)
{
    QMapIterator<quint32, QQuickItem*> it(m_itemsMap);
    while (it.hasNext())
    {
        it.next();
        quint32 fxID = it.key();
        bool enable = false;

        if (fixtures.contains(fxID))
            enable = true;

        selectFixture(it.value(), enable);
    }
}

void MainView2D::updateFixtureSelection(quint32 itemID, bool enable)
{
    if (isEnabled() == false || m_itemsMap.contains(itemID) == false)
        return;

    selectFixture(m_itemsMap[itemID], enable);
}

void MainView2D::updateFixtureRotation(quint32 itemID, QVector3D degrees)
{
    if (isEnabled() == false || m_itemsMap.contains(itemID) == false)
        return;

    QQuickItem *fxItem = m_itemsMap[itemID];

    switch(m_monProps->pointOfView())
    {
        case MonitorProperties::FrontView:
            fxItem->setProperty("rotation", degrees.z());
        break;
        case MonitorProperties::LeftSideView:
        case MonitorProperties::RightSideView:
            fxItem->setProperty("rotation", degrees.x());
        break;
        default:
            fxItem->setProperty("rotation", degrees.y());
        break;
    }
}

void MainView2D::updateFixturePosition(quint32 itemID, QVector3D pos)
{
    if (isEnabled() == false)
        return;

    if (m_itemsMap.contains(itemID) == false)
    {
        quint32 fxID = FixtureUtils::itemFixtureID(itemID);
        quint16 headIndex = FixtureUtils::itemHeadIndex(itemID);
        quint16 linkedIndex = FixtureUtils::itemLinkedIndex(itemID);
        createFixtureItem(fxID, headIndex, linkedIndex, pos, false);
    }
    else
    {
        QQuickItem *fxItem = m_itemsMap[itemID];
        QPointF point = FixtureUtils::item2DPosition(m_monProps, m_monProps->pointOfView(), pos);
        fxItem->setProperty("mmXPos", point.x());
        fxItem->setProperty("mmYPos", point.y());
    }
}

void MainView2D::updateFixtureSize(quint32 itemID, Fixture *fixture)
{
    if (isEnabled() == false)
        return;

    if (m_itemsMap.contains(itemID))
    {
        QQuickItem *fxItem = m_itemsMap[itemID];
        QLCFixtureMode *fxMode = fixture->fixtureMode();
        QSizeF size = FixtureUtils::item2DDimension(fixture->type() == QLCFixtureDef::Dimmer ? nullptr : fxMode,
                                                    m_monProps->pointOfView());
        fxItem->setProperty("mmWidth", size.width());
        fxItem->setProperty("mmHeight", size.height());
    }
}

void MainView2D::removeFixtureItem(quint32 itemID)
{
    if (isEnabled() == false || m_itemsMap.contains(itemID) == false)
        return;

    QQuickItem *fixtureItem = m_itemsMap.take(itemID);
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

QPoint MainView2D::gridPosition() const
{
    return m_gridPosition;
}

void MainView2D::setGridPosition(QPoint pos)
{
    if (pos == m_gridPosition)
        return;

    m_gridPosition = pos;

    emit gridPositionChanged();
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
}

QString MainView2D::backgroundImage()
{
    return m_monProps->commonBackgroundImage();
}

void MainView2D::setBackgroundImage(QString image)
{
    QString strippedPath = image.replace("file://", "");
    QString currentImage = m_monProps->commonBackgroundImage();

    if (strippedPath == currentImage)
        return;

    Tardis::instance()->enqueueAction(Tardis::EnvironmentBackgroundImage, 0, QVariant(currentImage), QVariant(strippedPath));
    m_monProps->setCommonBackgroundImage(strippedPath);
    emit backgroundImageChanged();
}





