/*
  Q Light Controller Plus
  vcxypad.cpp

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QQmlEngine>

#include "vcxypad.h"
#include "qlcmacros.h"
#include "fadechannel.h"
#include "genericfader.h"
#include "fixturemanager.h"
#include "listmodel.h"
#include "treemodel.h"
#include "doc.h"

/** ************** XML Tags and Attributes ************** */

#define KXMLQLCVCXYPadPan           QStringLiteral("Pan")
#define KXMLQLCVCXYPadTilt          QStringLiteral("Tilt")
#define KXMLQLCVCXYPadWidth         QStringLiteral("Width")
#define KXMLQLCVCXYPadHeight        QStringLiteral("Height")
#define KXMLQLCVCXYPadPosition      QStringLiteral("Position")
#define KXMLQLCVCXYPadRangeWindow   QStringLiteral("Window")
#define KXMLQLCVCXYPadRangeHorizMin QStringLiteral("hMin")
#define KXMLQLCVCXYPadRangeHorizMax QStringLiteral("hMax")
#define KXMLQLCVCXYPadRangeVertMin  QStringLiteral("vMin")
#define KXMLQLCVCXYPadRangeVertMax  QStringLiteral("vMax")

#define KXMLQLCVCXYPadInvertedAppearance    QStringLiteral("InvertedAppearance")

#define KXMLQLCVCXYPadFixture               QStringLiteral("Fixture")
#define KXMLQLCVCXYPadFixtureID             QStringLiteral("ID")
#define KXMLQLCVCXYPadFixtureHead           QStringLiteral("Head")

#define KXMLQLCVCXYPadFixtureAxis           QStringLiteral("Axis")
#define KXMLQLCVCXYPadFixtureAxisID         QStringLiteral("ID")
#define KXMLQLCVCXYPadFixtureAxisX          QStringLiteral("X")
#define KXMLQLCVCXYPadFixtureAxisY          QStringLiteral("Y")
#define KXMLQLCVCXYPadFixtureAxisLowLimit   QStringLiteral("LowLimit")
#define KXMLQLCVCXYPadFixtureAxisHighLimit  QStringLiteral("HighLimit")
#define KXMLQLCVCXYPadFixtureAxisReverse    QStringLiteral("Reverse")

/** **************** External Control IDs ***************** */

#define INPUT_PAN_ID            0
#define INPUT_PAN_FINE_ID       1
#define INPUT_TILT_ID           2
#define INPUT_TILT_FINE_ID      3
#define INPUT_WIDTH_ID          4
#define INPUT_HEIGHT_ID         5

VCXYPad::VCXYPad(Doc *doc, QObject *parent)
    : VCWidget(doc, parent)
    , m_invertedAppearance(false)
    , m_displayMode(Degrees)
    , m_currentPosition(QPointF(0, 0))
    , m_horizontalRange(QPointF(0.0, 1.0))
    , m_verticalRange(QPointF(0.0, 1.0))
    , m_positionChanged(false)
{
    setType(VCWidget::XYPadWidget);

    registerExternalControl(INPUT_PAN_ID, tr("Pan / Horizontal axis"), false);
    registerExternalControl(INPUT_PAN_FINE_ID, tr("Pan fine"), false);
    registerExternalControl(INPUT_TILT_ID, tr("Tilt / Vertical axis"), false);
    registerExternalControl(INPUT_TILT_FINE_ID, tr("Tilt fine"), false);
    registerExternalControl(INPUT_WIDTH_ID, tr("Width"), false);
    registerExternalControl(INPUT_HEIGHT_ID, tr("Height"), false);

    m_fixtureList = new ListModel(this);
    QStringList listRoles;
    listRoles << "name" << "fxID" << "head" << "isSelected" << "xRange" << "yRange";
    m_fixtureList->setRoleNames(listRoles);

    m_doc->masterTimer()->registerDMXSource(this);
    connect(m_doc->inputOutputMap(), SIGNAL(universeWritten(quint32,QByteArray)),
            this, SLOT(slotUniverseWritten(quint32,QByteArray)));
}

VCXYPad::~VCXYPad()
{
    m_doc->masterTimer()->unregisterDMXSource(this);
    foreach (QSharedPointer<GenericFader> fader, m_fadersMap)
    {
        if (!fader.isNull())
            fader->requestDelete();
    }
    m_fadersMap.clear();

    if (m_item)
        delete m_item;
}

QString VCXYPad::defaultCaption()
{
    return tr("XY Pad %1").arg(id() + 1);
}

void VCXYPad::setupLookAndFeel(qreal pixelDensity, int page)
{
    setPage(page);
    QFont wFont = font();
    wFont.setBold(true);
    wFont.setPointSize(pixelDensity * 5.0);
    setFont(wFont);
}

void VCXYPad::render(QQuickView *view, QQuickItem *parent)
{
    if (view == nullptr || parent == nullptr)
        return;

    QQmlComponent *component = new QQmlComponent(view->engine(), QUrl("qrc:/VCXYPadItem.qml"));

    if (component->isError())
    {
        qDebug() << component->errors();
        return;
    }

    m_item = qobject_cast<QQuickItem*>(component->create());

    m_item->setParentItem(parent);
    m_item->setProperty("xyPadObj", QVariant::fromValue(this));
}

QString VCXYPad::propertiesResource() const
{
    return QString("qrc:/VCXYPadProperties.qml");
}

VCWidget *VCXYPad::createCopy(VCWidget *parent)
{
    Q_ASSERT(parent != nullptr);

    VCXYPad *XYPad = new VCXYPad(m_doc, parent);
    if (XYPad->copyFrom(this) == false)
    {
        delete XYPad;
        XYPad = nullptr;
    }

    return XYPad;
}

bool VCXYPad::copyFrom(const VCWidget *widget)
{
    const VCXYPad *XYPad = qobject_cast<const VCXYPad*> (widget);
    if (XYPad == nullptr)
        return false;

    /* Copy and set properties */

    /* Copy object lists */

    /* Common stuff */
    return VCWidget::copyFrom(widget);
}

FunctionParent VCXYPad::functionParent() const
{
    return FunctionParent(FunctionParent::AutoVCWidget, id());
}

/*********************************************************************
 * Properties
 *********************************************************************/

bool VCXYPad::invertedAppearance() const
{
    return m_invertedAppearance;
}

void VCXYPad::setInvertedAppearance(bool newInvertedAppearance)
{
    if (m_invertedAppearance == newInvertedAppearance)
        return;

    m_invertedAppearance = newInvertedAppearance;
    emit invertedAppearanceChanged();
}

void VCXYPad::setDisplayMode(DisplayMode mode)
{
    if (mode == m_displayMode)
        return;

    m_displayMode = mode;
    emit displayModeChanged();
}

VCXYPad::DisplayMode VCXYPad::displayMode() const
{
    return m_displayMode;
}

QPointF VCXYPad::currentPosition() const
{
    return m_currentPosition;
}

void VCXYPad::setCurrentPosition(QPointF newCurrentPosition)
{
    if (m_currentPosition == newCurrentPosition)
        return;

    m_currentPosition = newCurrentPosition;
    m_positionChanged = true;
    emit currentPositionChanged();
}

QPointF VCXYPad::horizontalRange() const
{
    return m_horizontalRange;
}

void VCXYPad::setHorizontalRange(QPointF newHorizontalRange)
{
    if (m_horizontalRange == newHorizontalRange)
        return;

    m_horizontalRange = newHorizontalRange;
    emit horizontalRangeChanged();
}

QPointF VCXYPad::verticalRange() const
{
    return m_verticalRange;
}

void VCXYPad::setVerticalRange(QPointF newVerticalRange)
{
    if (m_verticalRange == newVerticalRange)
        return;

    m_verticalRange = newVerticalRange;
    emit verticalRangeChanged();
}

/*************************************************************************
 * Fixtures
 *************************************************************************/

void VCXYPad::addGroup(QVariant reference)
{
    Q_UNUSED(reference)
}

void VCXYPad::addFixture(QVariant reference)
{
    if (reference.canConvert<Fixture *>() == false)
        return;

    Fixture *fixture = reference.value<Fixture *>();

    Q_UNUSED(fixture)
}

void VCXYPad::removeFixture(QVariant reference)
{
    Q_UNUSED(reference)
}

QVariant VCXYPad::fixtureList() const
{
    return QVariant::fromValue(m_fixtureList);
}

QVariant VCXYPad::groupsTreeModel()
{
    if (m_fixtureTree == nullptr)
    {
        m_fixtureTree = new TreeModel(this);
        QQmlEngine::setObjectOwnership(m_fixtureTree, QQmlEngine::CppOwnership);
        QStringList treeColumns;
        treeColumns << "classRef" << "type" << "id" << "subid" << "chIdx" << "inGroup";
        m_fixtureTree->setColumnNames(treeColumns);
        m_fixtureTree->enableSorting(false);

        FixtureManager::updateGroupsTree(m_doc, m_fixtureTree, m_searchFilter,
                                         FixtureManager::ShowCheckBoxes | FixtureManager::ShowGroups | FixtureManager::ShowHeads);
    }

    return QVariant::fromValue(m_fixtureTree);
}

QString VCXYPad::searchFilter() const
{
    return m_searchFilter;
}

void VCXYPad::setSearchFilter(QString searchFilter)
{
    if (m_searchFilter == searchFilter)
        return;

    int currLen = m_searchFilter.length();

    m_searchFilter = searchFilter;

    if (searchFilter.length() >= SEARCH_MIN_CHARS ||
        (currLen >= SEARCH_MIN_CHARS && searchFilter.length() < SEARCH_MIN_CHARS))
    {
        FixtureManager::updateGroupsTree(m_doc, m_fixtureTree, m_searchFilter,
                                         FixtureManager::ShowCheckBoxes | FixtureManager::ShowGroups | FixtureManager::ShowHeads);
        emit groupsTreeModelChanged();
    }

    emit searchFilterChanged();
}

void VCXYPad::computeRange(XYPadFixture &fixture)
{
    if (fixture.m_xReverse)
    {
        fixture.m_xOffset = fixture.m_xMax * qreal(USHRT_MAX);
        fixture.m_xRange = (fixture.m_xMin - fixture.m_xMax) * qreal(USHRT_MAX);
    }
    else
    {
        fixture.m_xOffset = fixture.m_xMin * qreal(USHRT_MAX);
        fixture.m_xRange = (fixture.m_xMax - fixture.m_xMin) * qreal(USHRT_MAX);
    }

    if (fixture.m_yReverse)
    {
        fixture.m_yOffset = fixture.m_yMax * qreal(USHRT_MAX);
        fixture.m_yRange = (fixture.m_yMin - fixture.m_yMax) * qreal(USHRT_MAX);
    }
    else
    {
        fixture.m_yOffset = fixture.m_yMin * qreal(USHRT_MAX);
        fixture.m_yRange = (fixture.m_yMax - fixture.m_yMin) * qreal(USHRT_MAX);
    }
}

void VCXYPad::updateFixtureList()
{
    m_fixtureList->clear();

    for (XYPadFixture &fixture : m_fixtures)
    {
        Fixture *fxi = m_doc->fixture(fixture.m_head.fxi);
        if (fxi == NULL)
            continue;

        if (fixture.m_head.head >= fxi->heads())
            continue;

        // cache data just once
        if (fixture.m_universe == Universe::invalid())
            fixture.m_universe = fxi->universe();

        if (fixture.m_xMSB == QLCChannel::invalid())
        {
            fixture.m_xMSB = fxi->channelNumber(QLCChannel::Pan, QLCChannel::MSB, fixture.m_head.head);
            fixture.m_xLSB = fxi->channelNumber(QLCChannel::Pan, QLCChannel::LSB, fixture.m_head.head);
        }
        if (fixture.m_yMSB == QLCChannel::invalid())
        {
            fixture.m_yMSB = fxi->channelNumber(QLCChannel::Tilt, QLCChannel::MSB, fixture.m_head.head);
            fixture.m_yLSB = fxi->channelNumber(QLCChannel::Tilt, QLCChannel::LSB, fixture.m_head.head);
        }

        QString name = fxi->name();
        QRectF degrees = fxi->degreesRange(fixture.m_head.head);
        qreal xScale = 100.0, yScale = 100.0;
        QString units = "%";
        QString xRange, yRange;

        if (fxi->heads() > 1)
            name = QString("%1 [%2]").arg(fxi->name()).arg(fixture.m_head.head);

        if (m_displayMode == DMX)
        {
            xScale = 255.0;
            yScale = 255.0;
            units = "";
        }
        else if (m_displayMode == Degrees)
        {
            xScale = degrees.width();
            yScale = degrees.height();
            units = "°";
        }

        if (fixture.m_xReverse == false)
            xRange = QString("%1%3 - %2%3").arg(qRound(fixture.m_xMin * xScale)).arg(qRound(fixture.m_xMax * xScale)).arg(units);
        else
            xRange = QString("%1%3 - %2%3 (R)").arg(qRound(fixture.m_xMax * xScale)).arg(qRound(fixture.m_xMin * xScale)).arg(units);

        if (fixture.m_yReverse == false)
            yRange = QString("%1%3 - %2%3").arg(qRound(fixture.m_yMin * yScale)).arg(qRound(fixture.m_yMax * yScale)).arg(units);
        else
            yRange = QString("%1%3 - %2%3 (R)").arg(qRound(fixture.m_yMax * yScale)).arg(qRound(fixture.m_yMin * yScale)).arg(units);

        QVariantMap fxMap;
        fxMap.insert("name", name);
        fxMap.insert("fxID", fixture.m_head.fxi);
        fxMap.insert("head", fixture.m_head.head);
        fxMap.insert("isSelected", false);
        fxMap.insert("xRange", xRange);
        fxMap.insert("yRange", yRange);

        m_fixtureList->addDataMap(fxMap);
    }

    emit fixtureListChanged();
}

/*********************************************************************
 * DMXSource
 *********************************************************************/

void VCXYPad::updateChannel(FadeChannel *fc, uchar value)
{
    fc->setStart(value);
    fc->setCurrent(value);
    fc->setTarget(value);
    fc->setElapsed(0);
    fc->setReady(false);
}

void VCXYPad::slotUniverseWritten(quint32 idx, const QByteArray &universeData)
{
    Q_UNUSED(idx)
    Q_UNUSED(universeData)
}

void VCXYPad::writeDMX(MasterTimer *timer, QList<Universe *> universes)
{
    Q_UNUSED(timer)

    if (m_positionChanged == false)
        return;

    QPointF pt = currentPosition();

    /* Scale XY coordinate values to 0.0 - 1.0 */
    qreal x = SCALE(pt.x(), qreal(0), qreal(256), qreal(0), qreal(1));
    qreal y = SCALE(pt.y(), qreal(0), qreal(256), qreal(0), qreal(1));

    if (invertedAppearance())
        y = qreal(1) - y;

    /* Write values outside of mutex lock to keep UI snappy */
    for (XYPadFixture &fixture : m_fixtures)
    {
        if (fixture.m_universe == Universe::invalid())
            continue;

        if (fixture.m_enabled)
        {
            quint32 universe = fixture.m_universe;
            if (universe == Universe::invalid())
                continue;

            qDebug() << "HERE -1-";

            QSharedPointer<GenericFader> fader = m_fadersMap.value(universe, QSharedPointer<GenericFader>());
            if (fader.isNull())
            {
                fader = universes[universe]->requestFader();
                fader->adjustIntensity(intensity());
                m_fadersMap[universe] = fader;
            }

            if (fixture.m_xMSB == QLCChannel::invalid() || fixture.m_yMSB == QLCChannel::invalid())
                return;

            qDebug() << "HERE -2-";

            ushort xVal = floor(fixture.m_xRange * x + fixture.m_xOffset + 0.5);
            ushort yVal = floor(fixture.m_yRange * y + fixture.m_yOffset + 0.5);

            Universe *pUniverse = universes[universe];
            FadeChannel *fc = fader->getChannelFader(m_doc, pUniverse, fixture.m_head.fxi, fixture.m_xMSB);
            updateChannel(fc, uchar(xVal >> 8));

            fc = fader->getChannelFader(m_doc, pUniverse, fixture.m_head.fxi, fixture.m_yMSB);
            updateChannel(fc, uchar(yVal >> 8));

            if (fixture.m_xLSB != QLCChannel::invalid() && fixture.m_yLSB != QLCChannel::invalid())
            {
                fc = fader->getChannelFader(m_doc, pUniverse, fixture.m_head.fxi, fixture.m_xLSB);
                updateChannel(fc, uchar(xVal & 0xFF));

                fc = fader->getChannelFader(m_doc, pUniverse, fixture.m_head.fxi, fixture.m_yLSB);
                updateChannel(fc, uchar(yVal & 0xFF));
            }
        }
    }

    m_positionChanged = false;
}

/*********************************************************************
 * External input
 *********************************************************************/

void VCXYPad::updateFeedback()
{

}

void VCXYPad::slotInputValueChanged(quint8 id, uchar value)
{
    Q_UNUSED(value)

    switch (id)
    {
        case INPUT_PAN_ID:
        break;
        case INPUT_PAN_FINE_ID:
        break;
        case INPUT_TILT_ID:
        break;
        case INPUT_TILT_FINE_ID:
        break;
        case INPUT_WIDTH_ID:
        break;
        case INPUT_HEIGHT_ID:
        break;
    }
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

bool VCXYPad::loadXMLFixture(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCXYPadFixture)
    {
        qWarning() << Q_FUNC_INFO << "XYPad Fixture node not found";
        return false;
    }

    XYPadFixture fxItem;

    /* Fixture ID */
    fxItem.m_head.fxi = root.attributes().value(KXMLQLCVCXYPadFixtureID).toUInt();
    fxItem.m_head.head = root.attributes().value(KXMLQLCVCXYPadFixtureHead).toInt();
    fxItem.m_universe = Universe::invalid();
    fxItem.m_xMSB = QLCChannel::invalid();
    fxItem.m_xLSB = QLCChannel::invalid();
    fxItem.m_yMSB = QLCChannel::invalid();
    fxItem.m_yLSB = QLCChannel::invalid();
    fxItem.m_enabled = true;

    /* Children */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCVCXYPadFixtureAxis)
        {
            QXmlStreamAttributes attrs = root.attributes();
            QString axis = attrs.value(KXMLQLCVCXYPadFixtureAxisID).toString();
            double min = attrs.value(KXMLQLCVCXYPadFixtureAxisLowLimit).toDouble();
            double max = attrs.value(KXMLQLCVCXYPadFixtureAxisHighLimit).toDouble();
            QString rev = attrs.value(KXMLQLCVCXYPadFixtureAxisReverse).toString();

            if (axis == KXMLQLCVCXYPadFixtureAxisX)
            {
                fxItem.m_xMin = CLAMP(min, 0.0, 1.0);
                fxItem.m_xMax = CLAMP(max, 0.0, 1.0);
                fxItem.m_xReverse = rev == KXMLQLCTrue ? true : false;
            }
            else if (axis == KXMLQLCVCXYPadFixtureAxisY)
            {
                fxItem.m_yMin = CLAMP(min, 0.0, 1.0);
                fxItem.m_yMax = CLAMP(max, 0.0, 1.0);
                fxItem.m_yReverse = rev == KXMLQLCTrue ? true : false;
            }
            else
            {
                qWarning() << Q_FUNC_INFO << "Unknown XYPad axis" << axis;
            }
            root.skipCurrentElement();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown XY Pad tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    computeRange(fxItem);
    m_fixtures.append(fxItem);

    return true;
}

bool VCXYPad::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCXYPad)
    {
        qWarning() << Q_FUNC_INFO << "XY Pad node not found";
        return false;
    }

    QPointF currPos(0, 0);

    QXmlStreamAttributes attrs = root.attributes();

    /* Widget commons */
    loadXMLCommon(root);

    if (attrs.hasAttribute(KXMLQLCVCXYPadInvertedAppearance))
    {
        if (attrs.value(KXMLQLCVCXYPadInvertedAppearance).toString() == "0")
            setInvertedAppearance(false);
        else
            setInvertedAppearance(true);
    }

    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCWindowState)
        {
            bool visible = false;
            int x = 0, y = 0, w = 0, h = 0;
            loadXMLWindowState(root, &x, &y, &w, &h, &visible);
            setGeometry(QRect(x, y, w, h));
        }
        else if (root.name() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(root);
        }
        else if (root.name() == KXMLQLCVCXYPadFixture)
        {
            loadXMLFixture(root);
        }
        else if (root.name() == KXMLQLCVCXYPadPan)
        {
            currPos.setX(root.attributes().value(KXMLQLCVCXYPadPosition).toDouble());
            loadXMLSources(root, INPUT_PAN_ID);
        }
        else if (root.name() == KXMLQLCVCXYPadTilt)
        {
            currPos.setY(root.attributes().value(KXMLQLCVCXYPadPosition).toDouble());
            loadXMLSources(root, INPUT_TILT_ID);
        }
        else if (root.name() == KXMLQLCVCXYPadRangeWindow)
        {
            QXmlStreamAttributes wAttrs = root.attributes();
            float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
            if (wAttrs.hasAttribute(KXMLQLCVCXYPadRangeHorizMin))
                x1 = wAttrs.value(KXMLQLCVCXYPadRangeHorizMin).toFloat();
            if (wAttrs.hasAttribute(KXMLQLCVCXYPadRangeHorizMax))
                x2 = wAttrs.value(KXMLQLCVCXYPadRangeHorizMax).toFloat();
            if (wAttrs.hasAttribute(KXMLQLCVCXYPadRangeVertMin))
                y1 = wAttrs.value(KXMLQLCVCXYPadRangeVertMin).toFloat();
            if (wAttrs.hasAttribute(KXMLQLCVCXYPadRangeVertMax))
                y2 = wAttrs.value(KXMLQLCVCXYPadRangeVertMax).toFloat();

            setHorizontalRange(QPointF(x1, x2));
            setVerticalRange(QPointF(y1, y2));
            root.skipCurrentElement();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown XY pad tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    updateFixtureList();
    setCurrentPosition(currPos);

    return true;
}

bool VCXYPad::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != nullptr);

    /* VC object entry */
    doc->writeStartElement(KXMLQLCVCXYPad);

    saveXMLCommon(doc);

    /* Window state */
    saveXMLWindowState(doc);

    /* Appearance */
    saveXMLAppearance(doc);

    /* Write the <end> tag */
    doc->writeEndElement();

    return true;
}
