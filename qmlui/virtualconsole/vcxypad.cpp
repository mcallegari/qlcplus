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
#include <climits>

#include "vcxypad.h"
#include "vcxypadpreset.h"
#include "monitorproperties.h"
#include "fixturemanager.h"
#include "qlcfixturemode.h"
#include "fixturegroup.h"
#include "genericfader.h"
#include "qlcpalette.h"
#include "fadechannel.h"
#include "qlcchannel.h"
#include "listmodel.h"
#include "treemodel.h"
#include "qlcmacros.h"
#include "function.h"
#include "universe.h"
#include "tardis.h"
#include "scene.h"
#include "efx.h"
#include "doc.h"

/** ************** XML Tags and Attributes ************** */

#define KXMLQLCVCXYPadPan                   QStringLiteral("Pan")
#define KXMLQLCVCXYPadPanFine               QStringLiteral("PanFine")
#define KXMLQLCVCXYPadTilt                  QStringLiteral("Tilt")
#define KXMLQLCVCXYPadTiltFine              QStringLiteral("TiltFine")
#define KXMLQLCVCXYPadWidth                 QStringLiteral("Width")
#define KXMLQLCVCXYPadHeight                QStringLiteral("Height")
#define KXMLQLCVCXYPadPosition              QStringLiteral("Position")
#define KXMLQLCVCXYPadRangeWindow           QStringLiteral("Window")
#define KXMLQLCVCXYPadRangeHorizMin         QStringLiteral("hMin")
#define KXMLQLCVCXYPadRangeHorizMax         QStringLiteral("hMax")
#define KXMLQLCVCXYPadRangeVertMin          QStringLiteral("vMin")
#define KXMLQLCVCXYPadRangeVertMax          QStringLiteral("vMax")

#define KXMLQLCVCXYPadInvertedAppearance    QStringLiteral("InvertedAppearance")

#define KXMLQLCVCXYPadFloorControl          QStringLiteral("FloorControl")
#define KXMLQLCVCXYPadFloorPosition         QStringLiteral("FloorPosition")
#define KXMLQLCVCXYPadFloorPosX             QStringLiteral("X")
#define KXMLQLCVCXYPadFloorPosY             QStringLiteral("Y")
#define KXMLQLCVCXYPadFloorPosZ             QStringLiteral("Z")

#define KXMLQLCVCXYPadFixture               QStringLiteral("Fixture")
#define KXMLQLCVCXYPadFixtureID             QStringLiteral("ID")
#define KXMLQLCVCXYPadFixtureHead           QStringLiteral("Head")

#define KXMLQLCVCXYPadGroup                 QStringLiteral("Group")
#define KXMLQLCVCXYPadGroupID               QStringLiteral("ID")

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
#define INPUT_FLOOR_HEIGHT_ID   6
#define INPUT_PRESETS_BASE_ID   30

/** Floor control height fader boundaries, in metres */
static constexpr qreal kFloorHeightMax = 20.0;
static constexpr qreal kFloorHeightStep = 0.5;

VCXYPad::VCXYPad(Doc *doc, QObject *parent)
    : VCWidget(doc, parent)
    , m_invertedAppearance(false)
    , m_displayMode(Degrees)
    , m_currentPosition(QPointF(0, 0))
    , m_horizontalRange(QPointF(0.0, 255.0))
    , m_verticalRange(QPointF(0.0, 255.0))
    , m_positionChanged(false)
    , m_floorControl(false)
    , m_floorPosition(QVector3D(0, 0, 0))
    , m_fixtureTree(nullptr)
    , m_searchFilter(QString())
    , m_lastAssignedPresetId(15)
    , m_activePresetId(-1)
    , m_efxStartXOverrideId(Function::invalidAttributeId())
    , m_efxStartYOverrideId(Function::invalidAttributeId())
    , m_efxWidthOverrideId(Function::invalidAttributeId())
    , m_efxHeightOverrideId(Function::invalidAttributeId())
{
    setType(VCWidget::XYPadWidget);

    registerExternalControl(INPUT_PAN_ID, tr("Pan / Horizontal axis"), false);
    registerExternalControl(INPUT_PAN_FINE_ID, tr("Pan fine"), false);
    registerExternalControl(INPUT_TILT_ID, tr("Tilt / Vertical axis"), false);
    registerExternalControl(INPUT_TILT_FINE_ID, tr("Tilt fine"), false);
    registerExternalControl(INPUT_WIDTH_ID, tr("Width"), false);
    registerExternalControl(INPUT_HEIGHT_ID, tr("Height"), false);
    registerExternalControl(INPUT_FLOOR_HEIGHT_ID, tr("Floor target height"), false);

    // start with the floor target at the centre of the stage
    QVector3D envSize = floorSize();
    m_floorPosition = QVector3D(envSize.x() / 2.0f, 0.0f, envSize.z() / 2.0f);

    m_fixtureList = new ListModel(this);
    QStringList listRoles;
    listRoles << "name" << "fxID" << "head" << "groupID" << "isGroup"
              << "isSelected" << "xRange" << "yRange";
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

    detachEFX();
    clearPresets();

    if (m_item)
        delete m_item;
}

QString VCXYPad::defaultCaption() const
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
        delete component;
        return;
    }

    m_item = qobject_cast<QQuickItem*>(component->create());
    if (m_item == nullptr)
        qWarning() << Q_FUNC_INFO << "Unable to create XY pad component" << component->errors();
    delete component;
    if (m_item == nullptr)
        return;

    m_item->setParentItem(parent);
    m_item->setProperty("xyPadObj", QVariant::fromValue(this));
}

QString VCXYPad::propertiesResource() const
{
    return QString("qrc:/VCXYPadProperties.qml");
}

QString VCXYPad::presetsResource() const
{
    return QString("qrc:/VCXYPadPresets.qml");
}

bool VCXYPad::supportsPresets() const
{
    return true;
}

VCWidget *VCXYPad::createCopy(VCWidget *parent) const
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

void VCXYPad::remapChannels(const QMap<SceneValue, SceneValue> &remapMap)
{
    for (int i = 0; i < m_fixtures.count(); i++)
    {
        XYPadFixture &fix = m_fixtures[i];

        // group entries resolve their channels at write time, so there is
        // nothing cached here to remap
        if (fix.m_groupID != FixtureGroup::invalidId())
            continue;

        quint32 fxID = fix.m_head.fxi;

        SceneValue xKey(fxID, fix.m_xMSB);
        if (remapMap.contains(xKey))
            fix.m_xMSB = remapMap.value(xKey).channel;

        xKey.channel = fix.m_xLSB;
        if (remapMap.contains(xKey))
            fix.m_xLSB = remapMap.value(xKey).channel;

        SceneValue yKey(fxID, fix.m_yMSB);
        if (remapMap.contains(yKey))
            fix.m_yMSB = remapMap.value(yKey).channel;

        yKey.channel = fix.m_yLSB;
        if (remapMap.contains(yKey))
            fix.m_yLSB = remapMap.value(yKey).channel;
    }
}

bool VCXYPad::copyFrom(const VCWidget *widget)
{
    const VCXYPad *XYPad = qobject_cast<const VCXYPad*> (widget);
    if (XYPad == nullptr)
        return false;

    /* Copy and set properties */
    setInvertedAppearance(XYPad->invertedAppearance());
    setDisplayMode(XYPad->displayMode());
    setCurrentPosition(XYPad->currentPosition());
    setHorizontalRange(XYPad->horizontalRange());
    setVerticalRange(XYPad->verticalRange());
    setFloorControl(XYPad->floorControl());
    setFloorPosition(XYPad->floorPosition());

    /* Copy object lists */
    m_fixtures = XYPad->m_fixtures;
    updateFixtureList();

    clearPresets();
    for (VCXYPadPreset *preset : XYPad->presets())
        addPresetInternal(new VCXYPadPreset(*preset));
    m_lastAssignedPresetId = XYPad->m_lastAssignedPresetId;
    setActivePresetId(-1);
    emit presetsListChanged();

    /* Common stuff */
    return VCWidget::copyFrom(widget);
}

FunctionParent VCXYPad::functionParent() const
{
    return FunctionParent(FunctionParent::ManualVCWidget, id());
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

    // the Pan/Tilt range strings are formatted with the display mode units,
    // so the fixture list has to be rebuilt to show them. Do it before
    // notifying, so that listeners see the refreshed list.
    updateFixtureList();

    emit displayModeChanged();
}

VCXYPad::DisplayMode VCXYPad::displayMode() const
{
    return m_displayMode;
}

static constexpr qreal kPosMax = 255.0 + (255.0 / 256.0);

static inline qreal clampPos(qreal v)
{
    return CLAMP(v, 0.0, kPosMax);
}

// Range sliders are MSB-only in 0..255, expand to full 16-bit span.
static inline int clampMSB(qreal v)
{
    return CLAMP(int(qRound(v)), 0, 255);
};

static inline quint16 posToU16(qreal v)
{
    v = clampPos(v);
    const int msb = int(qFloor(v));
    int lsb = int(qRound((v - qFloor(v)) * 256.0));
    lsb = CLAMP(lsb, 0, 255);
    return quint16((msb << 8) | lsb);
}

static inline qreal u16ToPos(quint16 v)
{
    return qreal(v >> 8) + (qreal(v & 0xFF) / 256.0);
}

QPointF VCXYPad::currentPosition() const
{
    return m_currentPosition;
}

void VCXYPad::setCurrentPosition(QPointF newCurrentPosition)
{
    if (m_currentPosition == newCurrentPosition)
        return;

    newCurrentPosition.setX(clampPos(newCurrentPosition.x()));
    newCurrentPosition.setY(clampPos(newCurrentPosition.y()));

    QPointF previousPosition = m_currentPosition;

    m_currentPosition = newCurrentPosition;

    m_x16 = posToU16(m_currentPosition.x());
    m_y16 = posToU16(m_currentPosition.y());

    m_positionChanged = true;
    emit currentPositionChanged();

    // If the position was changed by something other than external input
    // (UI drag, preset, undo, ...) send feedback so external controllers stay
    // in sync: absolute faders track the cursor, encoders keep acting relative
    // to the new position
    if (m_handlingExternalInput == false)
        updateFeedback();

    Tardis::instance()->enqueueAction(Tardis::VCXYPadSetPosition, id(),
                                      previousPosition, m_currentPosition);
}

QPointF VCXYPad::horizontalRange() const
{
    return m_horizontalRange;
}

void VCXYPad::setHorizontalRange(QPointF newHorizontalRange)
{
    if (m_horizontalRange == newHorizontalRange)
        return;

    // Geometry (rubber band) is carried as a QRectF: horizontal range on x/width,
    // vertical range on y/height
    QRectF oldGeometry(m_horizontalRange.x(), m_verticalRange.x(),
                       m_horizontalRange.y(), m_verticalRange.y());

    m_horizontalRange = newHorizontalRange;
    emit horizontalRangeChanged();
    emit floorRangeAreaChanged();

    // pull the floor target back inside the window if it just moved out
    if (m_floorControl)
        setFloorPosition(m_floorPosition);

    // squeeze a running EFX preset into the new window
    updateEFXGeometry();

    QRectF newGeometry(m_horizontalRange.x(), m_verticalRange.x(),
                       m_horizontalRange.y(), m_verticalRange.y());
    Tardis::instance()->enqueueAction(Tardis::VCXYPadSetGeometry, id(), oldGeometry, newGeometry);
}

QPointF VCXYPad::verticalRange() const
{
    return m_verticalRange;
}

void VCXYPad::setVerticalRange(QPointF newVerticalRange)
{
    if (m_verticalRange == newVerticalRange)
        return;

    // Geometry (rubber band) is carried as a QRectF: horizontal range on x/width,
    // vertical range on y/height
    QRectF oldGeometry(m_horizontalRange.x(), m_verticalRange.x(),
                       m_horizontalRange.y(), m_verticalRange.y());

    m_verticalRange = newVerticalRange;
    emit verticalRangeChanged();
    emit floorRangeAreaChanged();

    // pull the floor target back inside the window if it just moved out
    if (m_floorControl)
        setFloorPosition(m_floorPosition);

    // squeeze a running EFX preset into the new window
    updateEFXGeometry();

    QRectF newGeometry(m_horizontalRange.x(), m_verticalRange.x(),
                       m_horizontalRange.y(), m_verticalRange.y());
    Tardis::instance()->enqueueAction(Tardis::VCXYPadSetGeometry, id(), oldGeometry, newGeometry);
}

bool VCXYPad::floorControl() const
{
    return m_floorControl;
}

void VCXYPad::setFloorControl(bool enable)
{
    if (m_floorControl == enable)
        return;

    m_floorControl = enable;

    // forget where the heads were pointing: on re-entering floor mode the
    // Pan wrap is resolved from scratch
    m_lastFloorPan.clear();

    // the environment may have been resized since this pad was last used:
    // re-clamp the target so it always sits on the stage
    if (m_floorControl)
    {
        QVector3D envSize = floorSize();
        m_floorPosition = QVector3D(qBound(0.0f, m_floorPosition.x(), envSize.x()),
                                    m_floorPosition.y(),
                                    qBound(0.0f, m_floorPosition.z(), envSize.z()));
        emit floorSizeChanged();
        emit floorRangeAreaChanged();
        emit floorPositionChanged();
    }

    // force a DMX write so that the fixtures follow the mode change
    m_positionChanged = true;

    emit floorControlChanged();
    m_doc->setModified();
}

QVector3D VCXYPad::floorPosition() const
{
    return m_floorPosition;
}

void VCXYPad::setFloorPosition(QVector3D newFloorPosition)
{
    QRectF area = floorRangeArea();

    newFloorPosition = QVector3D(qBound(float(area.left()), newFloorPosition.x(), float(area.right())),
                                 qBound(0.0f, newFloorPosition.y(), float(kFloorHeightMax)),
                                 qBound(float(area.top()), newFloorPosition.z(), float(area.bottom())));

    if (m_floorPosition == newFloorPosition)
        return;

    QVector3D previousPosition = m_floorPosition;

    m_floorPosition = newFloorPosition;
    m_positionChanged = true;

    emit floorPositionChanged();

    if (m_handlingExternalInput == false)
        updateFeedback();

    Tardis::instance()->enqueueAction(Tardis::VCXYPadSetFloorPosition, id(),
                                      previousPosition, m_floorPosition);
}

QRectF VCXYPad::floorRangeArea() const
{
    QVector3D envSize = floorSize();

    // The range window is stored as a 0-255 window on each axis. In floor
    // mode it limits the reachable portion of the stage rather than the
    // Pan/Tilt travel, so it is mapped onto the environment size.
    qreal xMin = qMin(m_horizontalRange.x(), m_horizontalRange.y());
    qreal xMax = qMax(m_horizontalRange.x(), m_horizontalRange.y());
    qreal zMin = qMin(m_verticalRange.x(), m_verticalRange.y());
    qreal zMax = qMax(m_verticalRange.x(), m_verticalRange.y());

    QRectF area(SCALE(xMin, 0.0, 255.0, 0.0, qreal(envSize.x())),
                SCALE(zMin, 0.0, 255.0, 0.0, qreal(envSize.z())),
                0, 0);

    area.setRight(SCALE(xMax, 0.0, 255.0, 0.0, qreal(envSize.x())));
    area.setBottom(SCALE(zMax, 0.0, 255.0, 0.0, qreal(envSize.z())));

    return area;
}

QVector3D VCXYPad::floorSize() const
{
    MonitorProperties *mProps = m_doc->monitorProperties();
    if (mProps == nullptr)
        return QVector3D(10.0f, 10.0f, 10.0f);

    // the environment size is stored in the currently selected grid units:
    // convert it to metres, which is what Position3D targets expect
    float unitScale = mProps->gridUnits() == MonitorProperties::Meters ? 1.0f : 0.3048f;
    QVector3D size = mProps->gridSize() * unitScale;

    // never return a degenerate area, or the pad would be unusable
    if (size.x() <= 0.0f || size.z() <= 0.0f)
        return QVector3D(10.0f, 10.0f, 10.0f);

    return size;
}

qreal VCXYPad::floorHeightMax() const
{
    return kFloorHeightMax;
}

qreal VCXYPad::floorHeightStep() const
{
    return kFloorHeightStep;
}

/*************************************************************************
 * Fixtures
 *************************************************************************/

void VCXYPad::addGroup(QVariant reference)
{
    // resolve the concrete type of the dropped item
    QObject *object = reference.value<QObject *>();

    if (FixtureGroup *group = qobject_cast<FixtureGroup *>(object))
    {
        if (hasGroup(group->id()))
            return;

        // the group is kept as a single entry: it is resolved to its member
        // heads only when writing DMX
        XYPadFixture fxItem;
        initXYFixtureItem(fxItem);
        fxItem.m_groupID = group->id();

        computeRange(fxItem);
        m_fixtures.append(fxItem);
        m_doc->setModified();

        updateFixtureList();

        // a dropped group gets its own preset, so that it can be
        // recalled from the pad right away
        int presetId = addFixtureGroupPreset(reference);
        if (presetId >= 0)
            setPresetName(quint8(presetId), group->name());
    }
    else if (Universe *uni = qobject_cast<Universe *>(object))
    {
        // a Universe has no persistent identity to keep, so it is expanded
        // to the fixtures it patches
        for (Fixture *fixture : m_doc->fixtures())
        {
            if (fixture->universe() != uni->id())
                continue;
            addFixture(QVariant::fromValue(fixture));
        }
    }
}

void VCXYPad::addFixture(QVariant reference)
{
    if (reference.canConvert<Fixture *>() == false)
        return;

    Fixture *fixture = reference.value<Fixture *>();
    int hIdx = 0;

    for (QLCFixtureHead const &head : fixture->fixtureMode()->heads())
    {
        quint32 panCh = head.channelNumber(QLCChannel::Pan, QLCChannel::MSB);
        quint32 tiltCh = head.channelNumber(QLCChannel::Tilt, QLCChannel::MSB);

        if (panCh == QLCChannel::invalid() && tiltCh == QLCChannel::invalid())
            continue;

        XYPadFixture fxItem;
        initXYFixtureItem(fxItem);

        fxItem.m_head.fxi = fixture->id();
        fxItem.m_head.head = hIdx++;
        if (hasHead(fxItem.m_head))
            continue;
        fxItem.m_universe = fixture->universe();
        fxItem.m_xMSB = panCh;
        fxItem.m_xLSB = head.channelNumber(QLCChannel::Pan, QLCChannel::LSB);
        fxItem.m_yMSB = tiltCh;
        fxItem.m_yLSB = head.channelNumber(QLCChannel::Tilt, QLCChannel::LSB);

        computeRange(fxItem);
        m_fixtures.append(fxItem);
        m_doc->setModified();
    }
    updateFixtureList();
}

void VCXYPad::addHead(int fixtureID, int headIndex)
{
    Fixture *fixture = m_doc->fixture(fixtureID);
    if (fixture == nullptr)
        return;

    XYPadFixture fxItem;
    initXYFixtureItem(fxItem);

    fxItem.m_head.fxi = fixture->id();
    fxItem.m_head.head = headIndex;
    if (hasHead(fxItem.m_head))
        return;
    fxItem.m_universe = fixture->universe();

    computeRange(fxItem);
    m_fixtures.append(fxItem);
    m_doc->setModified();

    updateFixtureList();
}

void VCXYPad::removeHeads(QVariantList heads)
{
    for (QVariant &vIdx : heads)
    {
        QModelIndex idx = m_fixtureList->index(vIdx.toInt(), 0, QModelIndex());
        QVariant fixtureID = m_fixtureList->data(idx, "fxID");
        QVariant headIndex = m_fixtureList->data(idx, "head");
        quint32 groupID = m_fixtureList->data(idx, "groupID").toUInt();

        qDebug() << "Removing fixture" << fixtureID << "head" << headIndex
                 << "group" << groupID;

        int fIdx = 0;
        for (XYPadFixture &fixture : m_fixtures)
        {
            if (groupID != FixtureGroup::invalidId())
            {
                if (fixture.m_groupID == groupID)
                {
                    m_fixtures.takeAt(fIdx);
                    m_doc->setModified();
                    break;
                }
            }
            else if (fixture.m_groupID == FixtureGroup::invalidId() &&
                     fixture.m_head.fxi == fixtureID && fixture.m_head.head == headIndex)
            {
                m_fixtures.takeAt(fIdx);
                m_doc->setModified();
                break;
            }
            fIdx++;
        }
    }
    updateFixtureList();
}

QVariantMap VCXYPad::headsRangeInfo(QVariantList heads)
{
    QVariantMap info;
    qreal xScale = 100.0, yScale = 100.0;
    QString units = "%";

    if (m_displayMode == DMX)
    {
        xScale = yScale = 255.0;
        units = "";
    }
    else if (m_displayMode == Degrees)
    {
        xScale = yScale = 0;
        units = QString::fromUtf8("°");
    }

    // collect the selected fixtures and, in Degrees mode, compute the
    // smallest Pan/Tilt range among the selection
    QList<XYPadFixture *> selected;

    for (QVariant &vIdx : heads)
    {
        QModelIndex idx = m_fixtureList->index(vIdx.toInt(), 0, QModelIndex());
        quint32 fixtureID = m_fixtureList->data(idx, "fxID").toUInt();
        int headIndex = m_fixtureList->data(idx, "head").toInt();
        quint32 groupID = m_fixtureList->data(idx, "groupID").toUInt();

        for (XYPadFixture &fixture : m_fixtures)
        {
            bool match = groupID != FixtureGroup::invalidId()
                         ? fixture.m_groupID == groupID
                         : (fixture.m_groupID == FixtureGroup::invalidId() &&
                            fixture.m_head.fxi == fixtureID && fixture.m_head.head == headIndex);

            if (match == false)
                continue;

            selected.append(&fixture);

            if (m_displayMode == Degrees)
            {
                // a group entry contributes the smallest range among its members
                for (const GroupHead &head : entryHeads(fixture))
                {
                    Fixture *fxi = m_doc->fixture(head.fxi);
                    if (fxi == nullptr)
                        continue;

                    QRectF degrees = fxi->degreesRange(head.head);
                    if (xScale == 0 || degrees.width() < xScale)
                        xScale = degrees.width();
                    if (yScale == 0 || degrees.height() < yScale)
                        yScale = degrees.height();
                }
            }
            break;
        }
    }

    if (selected.isEmpty())
        return info;

    // display the current range of the first selected fixture, scaled
    // with the smallest range so that values stay within the max bounds
    XYPadFixture *first = selected.first();

    info.insert("units", units);
    info.insert("xMaxValue", qRound(xScale));
    info.insert("yMaxValue", qRound(yScale));
    info.insert("xMin", qRound(first->m_xMin * xScale));
    info.insert("xMax", qRound(first->m_xMax * xScale));
    info.insert("xReverse", first->m_xReverse);
    info.insert("yMin", qRound(first->m_yMin * yScale));
    info.insert("yMax", qRound(first->m_yMax * yScale));
    info.insert("yReverse", first->m_yReverse);

    return info;
}

void VCXYPad::setHeadsRange(QVariantList heads, int xMin, int xMax, bool xReverse,
                            int yMin, int yMax, bool yReverse)
{
    for (QVariant &vIdx : heads)
    {
        QModelIndex idx = m_fixtureList->index(vIdx.toInt(), 0, QModelIndex());
        quint32 fixtureID = m_fixtureList->data(idx, "fxID").toUInt();
        int headIndex = m_fixtureList->data(idx, "head").toInt();
        quint32 groupID = m_fixtureList->data(idx, "groupID").toUInt();

        for (XYPadFixture &fixture : m_fixtures)
        {
            bool match = groupID != FixtureGroup::invalidId()
                         ? fixture.m_groupID == groupID
                         : (fixture.m_groupID == FixtureGroup::invalidId() &&
                            fixture.m_head.fxi == fixtureID && fixture.m_head.head == headIndex);

            if (match == false)
                continue;

            qreal xScale = 100.0, yScale = 100.0;

            if (m_displayMode == DMX)
            {
                xScale = yScale = 255.0;
            }
            else if (m_displayMode == Degrees)
            {
                // scale over the smallest range among the driven heads, so
                // that the entered degrees stay valid for every one of them
                xScale = yScale = 0;

                for (const GroupHead &head : entryHeads(fixture))
                {
                    Fixture *fxi = m_doc->fixture(head.fxi);
                    if (fxi == nullptr)
                        continue;

                    QRectF degrees = fxi->degreesRange(head.head);
                    if (xScale == 0 || degrees.width() < xScale)
                        xScale = degrees.width();
                    if (yScale == 0 || degrees.height() < yScale)
                        yScale = degrees.height();
                }
            }

            if (xScale == 0 || yScale == 0)
                break;

            fixture.m_xMin = qreal(xMin) / xScale;
            fixture.m_xMax = qreal(xMax) / xScale;
            fixture.m_xReverse = xReverse;
            fixture.m_yMin = qreal(yMin) / yScale;
            fixture.m_yMax = qreal(yMax) / yScale;
            fixture.m_yReverse = yReverse;
            computeRange(fixture);
            m_doc->setModified();
            break;
        }
    }

    updateFixtureList();
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
        treeColumns << "classRef" << "type" << "id" << "subid" << "head";
        m_fixtureTree->setColumnNames(treeColumns);
        m_fixtureTree->enableSorting(false);

        FixtureManager::updateGroupsTree(m_doc, m_fixtureTree, m_searchFilter,
                                         FixtureManager::ShowGroups | FixtureManager::ShowHeads);
    }

    return QVariant::fromValue(m_fixtureTree);
}

QVariantList VCXYPad::fixturePositions() const
{
    return m_fixturePositions;
}

QVariantList VCXYPad::presetsList() const
{
    QVariantList list;

    for (const VCXYPadPreset *preset : presets())
    {
        QVariantMap entry;
        entry.insert("id", preset->m_id);
        entry.insert("name", preset->m_name);
        entry.insert("type", int(preset->m_type));
        entry.insert("typeString", VCXYPadPreset::typeToString(preset->m_type));
        entry.insert("functionID", preset->m_funcID);
        entry.insert("headsCount", presetHeads(preset).count());
        entry.insert("color", preset->color());
        entry.insert("active", preset->m_id == m_activePresetId);
        list.append(entry);
    }

    return list;
}

int VCXYPad::activePresetId() const
{
    return m_activePresetId;
}

int VCXYPad::addPositionPreset()
{
    quint8 newId = ++m_lastAssignedPresetId;
    VCXYPadPreset *preset = new VCXYPadPreset(newId);
    preset->m_type = VCXYPadPreset::Position;
    preset->m_dmxPos = m_currentPosition;
    preset->m_name = QString("X:%1 - Y:%2")
                     .arg(static_cast<int>(preset->m_dmxPos.x()))
                     .arg(static_cast<int>(preset->m_dmxPos.y()));
    addPresetInternal(preset);

    emit presetsListChanged();
    m_doc->setModified();

    return newId;
}

int VCXYPad::addFunctionPreset(quint32 functionID)
{
    Function *function = m_doc->function(functionID);
    if (function == nullptr)
        return -1;

    VCXYPadPreset::PresetType type = VCXYPadPreset::Position;
    if (function->type() == Function::EFXType)
        type = VCXYPadPreset::EFX;
    else if (function->type() == Function::SceneType)
        type = VCXYPadPreset::Scene;
    else
        return -1;

    if (type == VCXYPadPreset::Scene && sceneHasPanTilt(functionID) == false)
        return -1;

    quint8 newId = ++m_lastAssignedPresetId;
    VCXYPadPreset *preset = new VCXYPadPreset(newId);
    preset->m_type = type;
    preset->m_funcID = functionID;
    preset->m_name = function->name();
    addPresetInternal(preset);

    emit presetsListChanged();
    m_doc->setModified();

    return newId;
}

int VCXYPad::addFixtureGroupPreset(QVariant reference)
{
    QList<GroupHead> heads;
    quint32 groupID = FixtureGroup::invalidId();

    // resolve the concrete type of the dropped item
    QObject *object = reference.value<QObject *>();

    if (FixtureGroup *group = qobject_cast<FixtureGroup *>(object))
    {
        // keep a reference to the group rather than a snapshot of its heads,
        // so the preset follows the group when its members change
        groupID = group->id();
        heads = group->headList();
    }
    else if (Universe *uni = qobject_cast<Universe *>(object))
    {
        for (const XYPadFixture &fixture : m_fixtures)
        {
            for (const GroupHead &head : entryHeads(fixture))
            {
                Fixture *fxi = m_doc->fixture(head.fxi);
                if (fxi != nullptr && fxi->universe() == uni->id())
                    heads.append(head);
            }
        }
    }
    else if (Fixture *fixture = qobject_cast<Fixture *>(object))
    {
        for (const XYPadFixture &fxItem : m_fixtures)
        {
            for (const GroupHead &head : entryHeads(fxItem))
            {
                if (head.fxi == fixture->id())
                    heads.append(head);
            }
        }
    }

    heads = uniqueHeadsInPad(heads);
    if (heads.isEmpty())
        return -1;

    // don't stack duplicates if the same group or selection gets dropped again
    for (VCXYPadPreset *preset : m_presets)
    {
        if (preset->m_type != VCXYPadPreset::FixtureGroup)
            continue;

        if (groupID != FixtureGroup::invalidId())
        {
            if (preset->m_fxGroupID == groupID)
                return preset->m_id;
        }
        else if (preset->m_fxGroupID == FixtureGroup::invalidId() &&
                 preset->m_fxGroup == heads)
        {
            return preset->m_id;
        }
    }

    quint8 newId = ++m_lastAssignedPresetId;
    VCXYPadPreset *preset = new VCXYPadPreset(newId);
    preset->m_type = VCXYPadPreset::FixtureGroup;
    preset->m_name = tr("Fixture Group");
    preset->m_fxGroupID = groupID;
    if (groupID == FixtureGroup::invalidId())
        preset->m_fxGroup = heads;
    addPresetInternal(preset);

    emit presetsListChanged();
    m_doc->setModified();

    return newId;
}

int VCXYPad::addFixtureGroupHeadPreset(int fixtureID, int headIndex)
{
    QList<GroupHead> heads;
    heads.append(GroupHead(fixtureID, headIndex));
    heads = uniqueHeadsInPad(heads);
    if (heads.isEmpty())
        return -1;

    quint8 newId = ++m_lastAssignedPresetId;
    VCXYPadPreset *preset = new VCXYPadPreset(newId);
    preset->m_type = VCXYPadPreset::FixtureGroup;
    preset->m_name = tr("Fixture Group");
    preset->m_fxGroup = heads;
    addPresetInternal(preset);

    emit presetsListChanged();
    m_doc->setModified();

    return newId;
}

void VCXYPad::removePreset(quint8 presetId)
{
    for (int i = 0; i < m_presets.count(); ++i)
    {
        if (m_presets.at(i)->m_id == presetId)
        {
            if (presetId <= UCHAR_MAX - INPUT_PRESETS_BASE_ID)
                unregisterExternalControl(INPUT_PRESETS_BASE_ID + presetId);

            if (m_presets.at(i)->m_id == m_activePresetId)
            {
                deactivatePreset(m_presets.at(i));
                setActivePresetId(-1);
            }
            delete m_presets.takeAt(i);
            emit presetsListChanged();
            m_doc->setModified();
            return;
        }
    }
}

int VCXYPad::movePresetUp(quint8 presetId)
{
    QList<VCXYPadPreset*> list = presets();
    int idx = -1;
    for (int i = 0; i < list.count(); ++i)
    {
        if (list.at(i)->m_id == presetId)
        {
            idx = i;
            break;
        }
    }

    if (idx <= 0)
        return presetId;

    quint8 prevId = list.at(idx - 1)->m_id;
    list.at(idx - 1)->m_id = list.at(idx)->m_id;
    list.at(idx)->m_id = prevId;
    refreshPresetExternalControls();
    emit presetsListChanged();
    m_doc->setModified();

    return prevId;
}

int VCXYPad::movePresetDown(quint8 presetId)
{
    QList<VCXYPadPreset*> list = presets();
    int idx = -1;
    for (int i = 0; i < list.count(); ++i)
    {
        if (list.at(i)->m_id == presetId)
        {
            idx = i;
            break;
        }
    }

    if (idx == -1 || idx >= list.count() - 1)
        return presetId;

    quint8 nextId = list.at(idx + 1)->m_id;
    list.at(idx + 1)->m_id = list.at(idx)->m_id;
    list.at(idx)->m_id = nextId;
    refreshPresetExternalControls();
    emit presetsListChanged();
    m_doc->setModified();

    return nextId;
}

void VCXYPad::setPresetName(quint8 presetId, QString name)
{
    VCXYPadPreset *preset = findPreset(presetId);
    if (preset == nullptr)
        return;

    if (preset->m_name == name)
        return;

    preset->m_name = name;
    if (presetId <= UCHAR_MAX - INPUT_PRESETS_BASE_ID)
    {
        unregisterExternalControl(INPUT_PRESETS_BASE_ID + presetId);
        registerExternalControl(INPUT_PRESETS_BASE_ID + presetId,
                                tr("Preset: %1").arg(preset->m_name), true);
    }
    emit presetsListChanged();
    m_doc->setModified();
}

void VCXYPad::applyPreset(quint8 presetId)
{
    VCXYPadPreset *preset = findPreset(presetId);
    if (preset == nullptr)
        return;

    if (preset->m_type == VCXYPadPreset::Position)
    {
        if (m_activePresetId >= 0 && m_activePresetId != presetId)
            deactivatePreset(findPreset(m_activePresetId));

        setCurrentPosition(preset->m_dmxPos);
        setActivePresetId(presetId);
    }
    else if (m_activePresetId == presetId)
    {
        deactivatePreset(preset);
        setActivePresetId(-1);
    }
    else
    {
        if (m_activePresetId >= 0)
            deactivatePreset(findPreset(m_activePresetId));

        if (activatePreset(preset))
            setActivePresetId(presetId);
        else
            setActivePresetId(-1);
    }

    Tardis::instance()->enqueueAction(Tardis::VCXYPadActivatePreset, id(), QVariant(), presetId);
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
                                         FixtureManager::ShowGroups | FixtureManager::ShowHeads);
        emit groupsTreeModelChanged();
    }

    emit searchFilterChanged();
}

QList<VCXYPadPreset*> VCXYPad::presets() const
{
    QList<VCXYPadPreset*> list = m_presets;
    std::sort(list.begin(), list.end(), VCXYPadPreset::compare);
    return list;
}

VCXYPadPreset *VCXYPad::findPreset(quint8 presetId) const
{
    for (VCXYPadPreset *preset : m_presets)
    {
        if (preset->m_id == presetId)
            return preset;
    }
    return nullptr;
}

void VCXYPad::refreshPresetExternalControls()
{
    for (const VCXYPadPreset *preset : m_presets)
    {
        if (preset->m_id > UCHAR_MAX - INPUT_PRESETS_BASE_ID)
            continue;

        unregisterExternalControl(INPUT_PRESETS_BASE_ID + preset->m_id);
        registerExternalControl(INPUT_PRESETS_BASE_ID + preset->m_id,
                                tr("Preset: %1").arg(preset->m_name), true);
    }
}

void VCXYPad::clearPresets()
{
    if (m_activePresetId >= 0)
        deactivatePreset(findPreset(m_activePresetId));
    setActivePresetId(-1);

    for (const VCXYPadPreset *preset : m_presets)
    {
        if (preset->m_id <= UCHAR_MAX - INPUT_PRESETS_BASE_ID)
            unregisterExternalControl(INPUT_PRESETS_BASE_ID + preset->m_id);
    }

    qDeleteAll(m_presets);
    m_presets.clear();
}

void VCXYPad::addPresetInternal(VCXYPadPreset *preset)
{
    if (preset == nullptr)
        return;

    m_presets.append(preset);
    if (preset->m_id > m_lastAssignedPresetId)
        m_lastAssignedPresetId = preset->m_id;

    if (preset->m_id <= UCHAR_MAX - INPUT_PRESETS_BASE_ID)
    {
        registerExternalControl(INPUT_PRESETS_BASE_ID + preset->m_id,
                                tr("Preset: %1").arg(preset->m_name), true);
    }
}

bool VCXYPad::hasHead(const GroupHead &head) const
{
    for (const XYPadFixture &fixture : m_fixtures)
    {
        // a head is in the pad either on its own or as a member of a
        // group entry
        if (entryHeads(fixture).contains(head))
            return true;
    }
    return false;
}

QList<GroupHead> VCXYPad::presetHeads(const VCXYPadPreset *preset) const
{
    if (preset == nullptr)
        return QList<GroupHead>();

    if (preset->m_fxGroupID == FixtureGroup::invalidId())
        return preset->m_fxGroup;

    FixtureGroup *group = m_doc->fixtureGroup(preset->m_fxGroupID);
    if (group == nullptr)
        return QList<GroupHead>();

    return group->headList();
}

QList<GroupHead> VCXYPad::uniqueHeadsInPad(const QList<GroupHead> &heads) const
{
    QList<GroupHead> list;

    for (const GroupHead &head : heads)
    {
        if (hasHead(head) && list.contains(head) == false)
            list.append(head);
    }

    return list;
}

bool VCXYPad::sceneHasPanTilt(quint32 functionID) const
{
    Function *function = m_doc->function(functionID);
    if (function == nullptr || function->type() != Function::SceneType)
        return false;

    Scene *scene = qobject_cast<Scene*>(function);
    if (scene == nullptr)
        return false;

    for (const SceneValue &scv : scene->values())
    {
        Fixture *fixture = m_doc->fixture(scv.fxi);
        if (fixture == nullptr)
            continue;

        const QLCChannel *channel = fixture->channel(scv.channel);
        if (channel == nullptr)
            continue;

        if (channel->group() == QLCChannel::Pan || channel->group() == QLCChannel::Tilt)
            return true;
    }

    return false;
}

bool VCXYPad::activatePreset(VCXYPadPreset *preset)
{
    if (preset == nullptr)
        return false;

    if (preset->m_type == VCXYPadPreset::EFX || preset->m_type == VCXYPadPreset::Scene)
    {
        Function *function = m_doc->function(preset->m_funcID);
        if (function == nullptr)
            return false;

        if (preset->m_type == VCXYPadPreset::EFX && function->type() != Function::EFXType)
            return false;
        if (preset->m_type == VCXYPadPreset::Scene && function->type() != Function::SceneType)
            return false;

        adjustFunctionIntensity(function, intensity());

        // an EFX gets its geometry bound to the range window, so that the
        // pattern can be resized/moved while it is running
        if (preset->m_type == VCXYPadPreset::EFX)
            attachEFX(function);

        function->start(m_doc->masterTimer(), functionParent());
        emit functionStarting(this, function->id(), intensity());
        return true;
    }

    if (preset->m_type == VCXYPadPreset::FixtureGroup)
    {
        // An entry stays enabled when it drives at least one of the preset's
        // heads. Group entries have no m_head of their own, so they have to
        // be matched through the heads they resolve to.
        QList<GroupHead> selected = presetHeads(preset);

        for (XYPadFixture &fixture : m_fixtures)
        {
            fixture.m_enabled = false;

            for (const GroupHead &head : entryHeads(fixture))
            {
                if (selected.contains(head))
                {
                    fixture.m_enabled = true;
                    break;
                }
            }
        }

        m_fixturePositions.clear();
        emit fixturePositionsChanged();
        m_positionChanged = true;
        return true;
    }

    return false;
}

void VCXYPad::deactivatePreset(VCXYPadPreset *preset)
{
    if (preset == nullptr)
        return;

    if (preset->m_type == VCXYPadPreset::EFX || preset->m_type == VCXYPadPreset::Scene)
    {
        Function *function = m_doc->function(preset->m_funcID);
        if (function != nullptr && function->isRunning())
            function->stop(functionParent());

        if (preset->m_type == VCXYPadPreset::EFX)
            detachEFX();
        return;
    }

    if (preset->m_type == VCXYPadPreset::FixtureGroup)
    {
        for (XYPadFixture &fixture : m_fixtures)
            fixture.m_enabled = true;

        m_fixturePositions.clear();
        emit fixturePositionsChanged();
        m_positionChanged = true;
    }
}

void VCXYPad::attachEFX(Function *function)
{
    detachEFX();

    if (function == nullptr || function->type() != Function::EFXType)
        return;

    m_efx = qobject_cast<EFX*>(function);
    if (m_efx.isNull())
        return;

    QRectF rect = efxGeometry();
    if (rect.isValid() == false)
        return;

    m_efxStartXOverrideId = m_efx->requestAttributeOverride(EFX::XOffset, rect.center().x());
    m_efxStartYOverrideId = m_efx->requestAttributeOverride(EFX::YOffset, rect.center().y());
    m_efxWidthOverrideId = m_efx->requestAttributeOverride(EFX::Width, rect.width() / 2);
    m_efxHeightOverrideId = m_efx->requestAttributeOverride(EFX::Height, rect.height() / 2);

    // the EFX can also be stopped from outside this pad: drop the
    // references as soon as that happens
    connect(m_efx, SIGNAL(stopped(quint32)), this, SLOT(slotEFXStopped(quint32)));
}

void VCXYPad::slotEFXStopped(quint32 fid)
{
    if (m_efx.isNull() == false && m_efx->id() == fid)
        detachEFX();
}

void VCXYPad::detachEFX()
{
    if (m_efx.isNull() == false)
    {
        disconnect(m_efx, SIGNAL(stopped(quint32)), this, SLOT(slotEFXStopped(quint32)));

        if (m_efxStartXOverrideId != Function::invalidAttributeId())
            m_efx->releaseAttributeOverride(m_efxStartXOverrideId);
        if (m_efxStartYOverrideId != Function::invalidAttributeId())
            m_efx->releaseAttributeOverride(m_efxStartYOverrideId);
        if (m_efxWidthOverrideId != Function::invalidAttributeId())
            m_efx->releaseAttributeOverride(m_efxWidthOverrideId);
        if (m_efxHeightOverrideId != Function::invalidAttributeId())
            m_efx->releaseAttributeOverride(m_efxHeightOverrideId);
    }

    m_efx = nullptr;
    m_efxStartXOverrideId = Function::invalidAttributeId();
    m_efxStartYOverrideId = Function::invalidAttributeId();
    m_efxWidthOverrideId = Function::invalidAttributeId();
    m_efxHeightOverrideId = Function::invalidAttributeId();
}

QRectF VCXYPad::efxGeometry() const
{
    qreal xMin = qMin(m_horizontalRange.x(), m_horizontalRange.y());
    qreal xMax = qMax(m_horizontalRange.x(), m_horizontalRange.y());
    qreal yMin = qMin(m_verticalRange.x(), m_verticalRange.y());
    qreal yMax = qMax(m_verticalRange.x(), m_verticalRange.y());

    return QRectF(QPointF(xMin, yMin), QPointF(xMax, yMax));
}

void VCXYPad::updateEFXGeometry()
{
    if (m_efx.isNull() || m_efx->isRunning() == false)
        return;

    QRectF rect = efxGeometry();
    if (rect.isValid() == false)
        return;

    m_efx->adjustAttribute(rect.center().x(), m_efxStartXOverrideId);
    m_efx->adjustAttribute(rect.center().y(), m_efxStartYOverrideId);
    m_efx->adjustAttribute(rect.width() / 2, m_efxWidthOverrideId);
    m_efx->adjustAttribute(rect.height() / 2, m_efxHeightOverrideId);
}

void VCXYPad::setActivePresetId(int presetId)
{
    if (m_activePresetId == presetId)
        return;

    m_activePresetId = presetId;
    emit activePresetIdChanged();
    emit presetsListChanged();
}

void VCXYPad::initXYFixtureItem(XYPadFixture &fixture)
{
    fixture.m_head.fxi = Fixture::invalidId();
    fixture.m_head.head = 0;
    fixture.m_universe = Universe::invalid();
    fixture.m_fixtureAddress = QLCChannel::invalid();
    fixture.m_xMSB = QLCChannel::invalid();
    fixture.m_xLSB = QLCChannel::invalid();
    fixture.m_yMSB = QLCChannel::invalid();
    fixture.m_yLSB = QLCChannel::invalid();
    fixture.m_xReverse = false;
    fixture.m_yReverse = false;
    fixture.m_xMin = 0;
    fixture.m_xMax = 1.0;
    fixture.m_yMin = 0;
    fixture.m_yMax = 1.0;
    fixture.m_enabled = true;
    fixture.m_groupID = FixtureGroup::invalidId();
}

bool VCXYPad::hasGroup(quint32 groupID) const
{
    for (const XYPadFixture &fixture : m_fixtures)
    {
        if (fixture.m_groupID == groupID)
            return true;
    }
    return false;
}

QList<GroupHead> VCXYPad::entryHeads(const XYPadFixture &fixture) const
{
    if (fixture.m_groupID == FixtureGroup::invalidId())
        return QList<GroupHead>() << fixture.m_head;

    // a group is resolved every time, so that fixtures added to or removed
    // from it after the drop are honoured
    FixtureGroup *group = m_doc->fixtureGroup(fixture.m_groupID);
    if (group == nullptr)
        return QList<GroupHead>();

    return group->headList();
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
    m_fixturePositions.clear();
    emit fixturePositionsChanged();

    for (XYPadFixture &fixture : m_fixtures)
    {
        QString name;
        qreal xScale = 100.0, yScale = 100.0;
        QString units = "%";
        QString xRange, yRange;
        bool isGroup = fixture.m_groupID != FixtureGroup::invalidId();

        if (isGroup)
        {
            FixtureGroup *group = m_doc->fixtureGroup(fixture.m_groupID);
            if (group == nullptr)
                continue;

            name = group->name();

            if (m_displayMode == DMX)
            {
                xScale = yScale = 255.0;
                units = "";
            }
            else if (m_displayMode == Degrees)
            {
                // a group can mix fixture types: show the smallest Pan/Tilt
                // range among its members, so the values stay within bounds
                xScale = yScale = 0;
                units = "°";

                for (const GroupHead &head : group->headList())
                {
                    Fixture *fxi = m_doc->fixture(head.fxi);
                    if (fxi == nullptr)
                        continue;

                    QRectF degrees = fxi->degreesRange(head.head);
                    if (xScale == 0 || degrees.width() < xScale)
                        xScale = degrees.width();
                    if (yScale == 0 || degrees.height() < yScale)
                        yScale = degrees.height();
                }
            }
        }
        else
        {
            Fixture *fxi = m_doc->fixture(fixture.m_head.fxi);
            if (fxi == NULL)
                continue;

            if (fixture.m_head.head >= fxi->heads())
                continue;

            // cache data just once
            if (fixture.m_universe == Universe::invalid())
                fixture.m_universe = fxi->universe();
            fixture.m_fixtureAddress = fxi->address();

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

            name = fxi->name();
            QRectF degrees = fxi->degreesRange(fixture.m_head.head);

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
        fxMap.insert("groupID", fixture.m_groupID);
        fxMap.insert("isGroup", isGroup);
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
    // in floor mode the pad area is the stage floor, so Pan/Tilt feedback
    // dots would be meaningless there
    if (m_floorControl)
    {
        if (m_fixturePositions.isEmpty() == false)
        {
            m_fixturePositions.clear();
            emit fixturePositionsChanged();
        }
        return;
    }

    QVariantList positions;

    for (const XYPadFixture &fixture : m_fixtures)
    {
        if (fixture.m_enabled == false)
            continue;

        const bool isGroup = fixture.m_groupID != FixtureGroup::invalidId();

        for (const GroupHead &head : entryHeads(fixture))
        {
            Fixture *fxi = m_doc->fixture(head.fxi);
            if (fxi == nullptr)
                continue;

            quint32 universe = isGroup ? fxi->universe() : fixture.m_universe;
            if (universe != idx)
                continue;

            quint32 xMSB = fixture.m_xMSB, xLSB = fixture.m_xLSB;
            quint32 yMSB = fixture.m_yMSB, yLSB = fixture.m_yLSB;

            if (isGroup)
            {
                if (head.head >= fxi->heads())
                    continue;

                xMSB = fxi->channelNumber(QLCChannel::Pan, QLCChannel::MSB, head.head);
                xLSB = fxi->channelNumber(QLCChannel::Pan, QLCChannel::LSB, head.head);
                yMSB = fxi->channelNumber(QLCChannel::Tilt, QLCChannel::MSB, head.head);
                yLSB = fxi->channelNumber(QLCChannel::Tilt, QLCChannel::LSB, head.head);
            }

            if (xMSB == QLCChannel::invalid() || yMSB == QLCChannel::invalid())
                continue;

            quint32 fixtureAddress = fxi->address();
            if (fixtureAddress == QLCChannel::invalid())
                continue;

            int x = -1;
            int y = -1;

            if ((xMSB + fixtureAddress) < quint32(universeData.size()))
                x = int(uchar(universeData.at(xMSB + fixtureAddress))) * 256;
            if ((yMSB + fixtureAddress) < quint32(universeData.size()))
                y = int(uchar(universeData.at(yMSB + fixtureAddress))) * 256;

            if (x == -1 || y == -1)
                continue;

            if (xLSB != QLCChannel::invalid() &&
                (xLSB + fixtureAddress) < quint32(universeData.size()))
            {
                x += int(uchar(universeData.at(xLSB + fixtureAddress)));
            }
            if (yLSB != QLCChannel::invalid() &&
                (yLSB + fixtureAddress) < quint32(universeData.size()))
            {
                y += int(uchar(universeData.at(yLSB + fixtureAddress)));
            }

            // Map the raw DMX value back to the cursor position within the
            // configured Pan/Tilt range, so the dot tracks the cursor even when
            // the range is reduced. m_xRange/m_xOffset already account for reverse.
            // (writeDMX: dmx = m_xRange * cursor + m_xOffset)
            qreal xNorm = fixture.m_xRange != 0.0 ? (qreal(x) - fixture.m_xOffset) / fixture.m_xRange
                                                  : qreal(x) / qreal(USHRT_MAX);
            qreal yNorm = fixture.m_yRange != 0.0 ? (qreal(y) - fixture.m_yOffset) / fixture.m_yRange
                                                  : qreal(y) / qreal(USHRT_MAX);

            xNorm = qBound(qreal(0.0), xNorm, qreal(1.0));
            yNorm = qBound(qreal(0.0), yNorm, qreal(1.0));

            if (invertedAppearance())
                yNorm = 1.0 - yNorm;

            QVariantMap posMap;
            posMap.insert("x", xNorm * 256.0);
            posMap.insert("y", yNorm * 256.0);
            positions.append(posMap);
        }
    }

    if (positions == m_fixturePositions)
        return;

    m_fixturePositions = positions;
    emit fixturePositionsChanged();
}

void VCXYPad::writeDMX(MasterTimer *timer, QList<Universe *> universes)
{
    Q_UNUSED(timer)

    if (m_positionChanged == false)
        return;

    if (m_floorControl)
    {
        writeDMXFloor(universes);
        m_positionChanged = false;
        return;
    }

    // Read current position
    QPointF pt = currentPosition();
    quint16 x16 = posToU16(pt.x());
    quint16 y16 = posToU16(pt.y());

    int hMinMSB = clampMSB(m_horizontalRange.x());
    int hMaxMSB = clampMSB(m_horizontalRange.y());
    int vMinMSB = clampMSB(m_verticalRange.x());
    int vMaxMSB = clampMSB(m_verticalRange.y());

    if (hMaxMSB < hMinMSB)
        qSwap(hMinMSB, hMaxMSB);
    if (vMaxMSB < vMinMSB)
        qSwap(vMinMSB, vMaxMSB);

    const quint16 hMin16 = quint16((hMinMSB << 8) | 0x00);
    const quint16 hMax16 = quint16((hMaxMSB << 8) | 0xFF);
    const quint16 vMin16 = quint16((vMinMSB << 8) | 0x00);
    const quint16 vMax16 = quint16((vMaxMSB << 8) | 0xFF);

    // Clamp to window (do NOT renormalize: UI already clamps, and we want output limited)
    x16 = CLAMP(x16, hMin16, hMax16);
    y16 = CLAMP(y16, vMin16, vMax16);

    // Convert to absolute multipliers 0..1 in 16-bit space
    qreal x = qreal(x16) / qreal(USHRT_MAX);
    qreal y = qreal(y16) / qreal(USHRT_MAX);

    if (invertedAppearance())
        y = 1.0 - y;

    // Write DMX values
    for (XYPadFixture &fixture : m_fixtures)
    {
        if (fixture.m_enabled == false)
            continue;

        const bool isGroup = fixture.m_groupID != FixtureGroup::invalidId();

        // A fixture entry drives its own cached channels, while a group entry
        // is resolved to its member heads on the fly
        for (const GroupHead &head : entryHeads(fixture))
        {
            quint32 universe = fixture.m_universe;
            quint32 xMSB = fixture.m_xMSB, xLSB = fixture.m_xLSB;
            quint32 yMSB = fixture.m_yMSB, yLSB = fixture.m_yLSB;

            if (isGroup)
            {
                Fixture *fxi = m_doc->fixture(head.fxi);
                if (fxi == nullptr || head.head >= fxi->heads())
                    continue;

                universe = fxi->universe();
                xMSB = fxi->channelNumber(QLCChannel::Pan, QLCChannel::MSB, head.head);
                xLSB = fxi->channelNumber(QLCChannel::Pan, QLCChannel::LSB, head.head);
                yMSB = fxi->channelNumber(QLCChannel::Tilt, QLCChannel::MSB, head.head);
                yLSB = fxi->channelNumber(QLCChannel::Tilt, QLCChannel::LSB, head.head);
            }

            if (universe == Universe::invalid())
                continue;

            if (universe >= quint32(universes.size()) || universes[universe] == nullptr)
                continue;

            // Skip fixtures that cannot be driven (do NOT abort the whole pad)
            if (xMSB == QLCChannel::invalid() || yMSB == QLCChannel::invalid())
                continue;

            QSharedPointer<GenericFader> fader = m_fadersMap.value(universe, QSharedPointer<GenericFader>());
            if (fader.isNull())
            {
                fader = universes[universe]->requestFader();
                m_fadersMap[universe] = fader;
            }

            // Keep intensity coherent
            fader->adjustIntensity(intensity());

            const ushort xVal = ushort(floor(fixture.m_xRange * x + fixture.m_xOffset + 0.5));
            const ushort yVal = ushort(floor(fixture.m_yRange * y + fixture.m_yOffset + 0.5));

            Universe *pUniverse = universes[universe];

            FadeChannel *fc = fader->getChannelFader(m_doc, pUniverse, head.fxi, xMSB);
            updateChannel(fc, uchar(xVal >> 8));

            fc = fader->getChannelFader(m_doc, pUniverse, head.fxi, yMSB);
            updateChannel(fc, uchar(yVal >> 8));

            if (xLSB != QLCChannel::invalid())
            {
                fc = fader->getChannelFader(m_doc, pUniverse, head.fxi, xLSB);
                updateChannel(fc, uchar(xVal & 0xFF));
            }

            if (yLSB != QLCChannel::invalid())
            {
                fc = fader->getChannelFader(m_doc, pUniverse, head.fxi, yLSB);
                updateChannel(fc, uchar(yVal & 0xFF));
            }
        }
    }

    m_positionChanged = false;
}

qreal VCXYPad::resolvePanDegrees(const Fixture *fixture, qreal panDeg)
{
    if (fixture == nullptr || fixture->fixtureMode() == nullptr)
        return panDeg;

    QLCPhysical phy = fixture->fixtureMode()->physical();
    qreal maxPan = phy.focusPanMax() ? phy.focusPanMax() : 360.0;

    // Nothing to disambiguate on a head that cannot do a full turn
    if (maxPan <= 360.0)
        return panDeg;

    // Reference angle: where this fixture was last pointed. On the first
    // move, aim from the middle of the travel, which is the position that
    // leaves the most room to swing either way.
    qreal reference = m_lastFloorPan.value(fixture->id(), maxPan / 2.0);

    // Candidates are the same direction plus whole turns, e.g. 30, 390 on a
    // 540 degrees head. Keep the one that is reachable and needs the
    // smallest movement from the current position.
    qreal best = panDeg;
    qreal bestDistance = -1.0;

    for (qreal candidate = fmod(panDeg, 360.0); candidate <= maxPan; candidate += 360.0)
    {
        if (candidate < 0.0)
            continue;

        qreal distance = qAbs(candidate - reference);
        if (bestDistance < 0.0 || distance < bestDistance)
        {
            bestDistance = distance;
            best = candidate;
        }
    }

    m_lastFloorPan.insert(fixture->id(), best);

    return best;
}

void VCXYPad::writeDMXFloor(QList<Universe *> universes)
{
    // Collect the fixtures currently driven by this pad. The Position3D
    // aiming math works per fixture, so heads of the same fixture collapse
    // into a single entry.
    QList<quint32> fixtureIDs;

    // While a Fixture Group preset is active, only the heads it selects are
    // tracked, even when they come from a wider group entry
    VCXYPadPreset *activePreset = m_activePresetId >= 0 ? findPreset(m_activePresetId) : nullptr;
    bool restrictToPreset = activePreset != nullptr &&
                            activePreset->m_type == VCXYPadPreset::FixtureGroup;
    QList<GroupHead> presetSelection = restrictToPreset ? presetHeads(activePreset)
                                                        : QList<GroupHead>();

    for (const XYPadFixture &fixture : m_fixtures)
    {
        if (fixture.m_enabled == false)
            continue;

        for (const GroupHead &head : entryHeads(fixture))
        {
            if (restrictToPreset && presetSelection.contains(head) == false)
                continue;

            if (fixtureIDs.contains(head.fxi) == false)
                fixtureIDs.append(head.fxi);
        }
    }

    if (fixtureIDs.isEmpty())
        return;

    // This runs only on an actual position change, not on every tick, so
    // recomputing the aiming values here is affordable.

    // Reuse the Position3D palette to turn the floor target into Pan/Tilt
    // values: it already knows about fixture position, rotation, inverted
    // Pan/Tilt flags and physical ranges.
    QLCPalette palette(QLCPalette::Position3D);
    palette.setValue(m_floorPosition.x(), m_floorPosition.y(), m_floorPosition.z());

    QList<SceneValue> values = palette.valuesFromFixtures(m_doc, fixtureIDs);

    // The palette always resolves Pan within the first turn (0-360). On heads
    // with a wider range that is only one of the possible answers, so the Pan
    // values are recomputed here to pick the closest reachable one.
    // Tilt values are passed through untouched.
    for (quint32 fixtureID : fixtureIDs)
    {
        Fixture *fxi = m_doc->fixture(fixtureID);
        if (fxi == nullptr || fxi->fixtureMode() == nullptr)
            continue;

        QLCPhysical phy = fxi->fixtureMode()->physical();
        qreal maxPan = phy.focusPanMax() ? phy.focusPanMax() : 360.0;
        if (maxPan <= 360.0)
            continue;

        quint32 panMSB = fxi->channelNumber(QLCChannel::Pan, QLCChannel::MSB);
        quint32 panLSB = fxi->channelNumber(QLCChannel::Pan, QLCChannel::LSB);
        if (panMSB == QLCChannel::invalid())
            continue;

        // rebuild the 16-bit Pan value the palette produced
        int msbIdx = -1, lsbIdx = -1;
        quint16 pan16 = 0;

        for (int i = 0; i < values.count(); i++)
        {
            if (values.at(i).fxi != fixtureID)
                continue;

            if (values.at(i).channel == panMSB)
            {
                msbIdx = i;
                pan16 |= quint16(values.at(i).value) << 8;
            }
            else if (panLSB != QLCChannel::invalid() && values.at(i).channel == panLSB)
            {
                lsbIdx = i;
                pan16 |= quint16(values.at(i).value);
            }
        }

        if (msbIdx == -1)
            continue;

        qreal panDeg = (qreal(pan16) * maxPan) / 65535.0;
        qreal resolved = resolvePanDegrees(fxi, panDeg);

        quint16 newPan16 = quint16((resolved * 65535.0) / maxPan);
        values[msbIdx].value = uchar(newPan16 >> 8);
        if (lsbIdx != -1)
            values[lsbIdx].value = uchar(newPan16 & 0xFF);
    }

    for (const SceneValue &scv : values)
    {
        Fixture *fxi = m_doc->fixture(scv.fxi);
        if (fxi == nullptr)
            continue;

        quint32 universe = fxi->universe();
        if (universe == Universe::invalid() ||
            universe >= quint32(universes.size()) || universes[universe] == nullptr)
            continue;

        QSharedPointer<GenericFader> fader = m_fadersMap.value(universe, QSharedPointer<GenericFader>());
        if (fader.isNull())
        {
            fader = universes[universe]->requestFader();
            m_fadersMap[universe] = fader;
        }

        fader->adjustIntensity(intensity());

        FadeChannel *fc = fader->getChannelFader(m_doc, universes[universe], scv.fxi, scv.channel);
        updateChannel(fc, scv.value);
    }
}

/*********************************************************************
 * External input
 *********************************************************************/

void VCXYPad::updateFeedback()
{
    if (m_floorControl)
    {
        // In floor mode the axes carry stage coordinates rather than
        // Pan/Tilt, so feedback is scaled over the environment size
        QVector3D envSize = floorSize();

        sendFeedback(CLAMP(int(SCALE(qreal(m_floorPosition.x()), 0.0, qreal(envSize.x()), 0.0, 255.0)), 0, 255),
                     INPUT_PAN_ID);
        sendFeedback(CLAMP(int(SCALE(qreal(m_floorPosition.z()), 0.0, qreal(envSize.z()), 0.0, 255.0)), 0, 255),
                     INPUT_TILT_ID);
        sendFeedback(CLAMP(int(SCALE(qreal(m_floorPosition.y()), 0.0, kFloorHeightMax, 0.0, 255.0)), 0, 255),
                     INPUT_FLOOR_HEIGHT_ID);
        return;
    }

    // Send back the current position on each axis. This keeps external
    // controllers in sync with the pad: motorized/absolute faders track the
    // cursor, while relative controllers (encoders) get their internal value
    // seeded so they act relative to the actual position rather than from zero.
    int panFeedback = SCALE(qreal(m_x16 >> 8), m_horizontalRange.x(), m_horizontalRange.y(), 0, 255);
    int tiltFeedback = SCALE(qreal(m_y16 >> 8), m_verticalRange.x(), m_verticalRange.y(), 0, 255);

    sendFeedback(CLAMP(panFeedback, 0, 255), INPUT_PAN_ID);
    sendFeedback(m_x16 & 0xFF, INPUT_PAN_FINE_ID);
    sendFeedback(CLAMP(tiltFeedback, 0, 255), INPUT_TILT_ID);
    sendFeedback(m_y16 & 0xFF, INPUT_TILT_FINE_ID);
}

void VCXYPad::slotInputValueChanged(quint8 id, uchar value)
{
    if (id >= INPUT_PRESETS_BASE_ID)
    {
        if (value == UCHAR_MAX)
        {
            quint8 presetId = id - INPUT_PRESETS_BASE_ID;
            if (findPreset(presetId) != nullptr)
                applyPreset(presetId);
        }
        return;
    }

    // Mark that the following position change originates from external input,
    // so setCurrentPosition doesn't echo a feedback back to the controller
    m_handlingExternalInput = true;

    if (m_floorControl)
    {
        // In floor mode the Pan/Tilt controls move the target over the stage
        // floor (X/Z), and a dedicated control raises it on the Y axis
        QVector3D envSize = floorSize();

        switch (id)
        {
            case INPUT_PAN_ID:
                setFloorPosition(QVector3D(SCALE(qreal(value), 0.0, 255.0, 0.0, qreal(envSize.x())),
                                           m_floorPosition.y(), m_floorPosition.z()));
            break;
            case INPUT_TILT_ID:
                setFloorPosition(QVector3D(m_floorPosition.x(), m_floorPosition.y(),
                                           SCALE(qreal(value), 0.0, 255.0, 0.0, qreal(envSize.z()))));
            break;
            case INPUT_FLOOR_HEIGHT_ID:
            {
                // snap the height to the fader step
                qreal height = SCALE(qreal(value), 0.0, 255.0, 0.0, kFloorHeightMax);
                height = qRound(height / kFloorHeightStep) * kFloorHeightStep;
                setFloorPosition(QVector3D(m_floorPosition.x(), height, m_floorPosition.z()));
            }
            break;
        }

        m_handlingExternalInput = false;
        return;
    }

    switch (id)
    {
        case INPUT_PAN_ID:
            value = SCALE(value, 0, 255, m_horizontalRange.x(), m_horizontalRange.y());
            m_x16 = quint16((quint16(value) << 8) | (m_x16 & 0x00FF));
            setCurrentPosition(QPointF(u16ToPos(m_x16), m_currentPosition.y()));
        break;
        case INPUT_PAN_FINE_ID:
            m_x16 = quint16((m_x16 & 0xFF00) | quint16(value));
            setCurrentPosition(QPointF(u16ToPos(m_x16), m_currentPosition.y()));
        break;
        case INPUT_TILT_ID:
            value = SCALE(value, 0, 255, m_verticalRange.x(), m_verticalRange.y());
            m_y16 = quint16((quint16(value) << 8) | (m_y16 & 0x00FF));
            setCurrentPosition(QPointF(m_currentPosition.x(), u16ToPos(m_y16)));
        break;
        case INPUT_TILT_FINE_ID:
            m_y16 = quint16((m_y16 & 0xFF00) | quint16(value));
            setCurrentPosition(QPointF(m_currentPosition.x(), u16ToPos(m_y16)));
        break;
        case INPUT_WIDTH_ID:
            // resize the range window (and with it a running EFX preset)
            // by moving the horizontal upper limit
            setHorizontalRange(QPointF(m_horizontalRange.x(), value));
        break;
        case INPUT_HEIGHT_ID:
            setVerticalRange(QPointF(m_verticalRange.x(), value));
        break;
    }

    m_handlingExternalInput = false;
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

void VCXYPad::loadXMLAxes(QXmlStreamReader &root, XYPadFixture &fxItem)
{
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
}

bool VCXYPad::loadXMLFixture(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCXYPadFixture)
    {
        qWarning() << Q_FUNC_INFO << "XYPad Fixture node not found";
        return false;
    }

    XYPadFixture fxItem;
    initXYFixtureItem(fxItem);

    /* Fixture ID */
    fxItem.m_head.fxi = root.attributes().value(KXMLQLCVCXYPadFixtureID).toUInt();
    fxItem.m_head.head = root.attributes().value(KXMLQLCVCXYPadFixtureHead).toInt();

    /* Children */
    loadXMLAxes(root, fxItem);

    computeRange(fxItem);
    m_fixtures.append(fxItem);

    return true;
}

bool VCXYPad::loadXMLGroup(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCXYPadGroup)
    {
        qWarning() << Q_FUNC_INFO << "XYPad Group node not found";
        return false;
    }

    XYPadFixture fxItem;
    initXYFixtureItem(fxItem);

    /* Fixture Group ID */
    fxItem.m_groupID = root.attributes().value(KXMLQLCVCXYPadGroupID).toUInt();

    /* Children */
    loadXMLAxes(root, fxItem);

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
    QVector3D floorPos;
    bool hasFloorPos = false;

    m_fixtures.clear();
    clearPresets();
    m_lastAssignedPresetId = 15;

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

    if (attrs.hasAttribute(KXMLQLCVCXYPadFloorControl))
        setFloorControl(attrs.value(KXMLQLCVCXYPadFloorControl).toString() != "0");

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
        else if (root.name() == KXMLQLCVCXYPadGroup)
        {
            loadXMLGroup(root);
        }
        else if (root.name() == KXMLQLCVCXYPadPan)
        {
            currPos.setX(root.attributes().value(KXMLQLCVCXYPadPosition).toFloat());
            loadXMLSources(root, INPUT_PAN_ID);
        }
        else if (root.name() == KXMLQLCVCXYPadTilt)
        {
            currPos.setY(root.attributes().value(KXMLQLCVCXYPadPosition).toFloat());
            loadXMLSources(root, INPUT_TILT_ID);
        }
        else if (root.name() == KXMLQLCVCXYPadPanFine)
        {
            loadXMLSources(root, INPUT_PAN_FINE_ID);
        }
        else if (root.name() == KXMLQLCVCXYPadTiltFine)
        {
            loadXMLSources(root, INPUT_TILT_FINE_ID);
        }
        else if (root.name() == KXMLQLCVCXYPadWidth)
        {
            loadXMLSources(root, INPUT_WIDTH_ID);
        }
        else if (root.name() == KXMLQLCVCXYPadHeight)
        {
            loadXMLSources(root, INPUT_HEIGHT_ID);
        }
        else if (root.name() == KXMLQLCVCXYPadFloorPosition)
        {
            QXmlStreamAttributes fAttrs = root.attributes();
            floorPos = QVector3D(fAttrs.value(KXMLQLCVCXYPadFloorPosX).toFloat(),
                                 fAttrs.value(KXMLQLCVCXYPadFloorPosY).toFloat(),
                                 fAttrs.value(KXMLQLCVCXYPadFloorPosZ).toFloat());
            hasFloorPos = true;
            loadXMLSources(root, INPUT_FLOOR_HEIGHT_ID);
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
        else if (root.name() == KXMLQLCVCXYPadPreset)
        {
            VCXYPadPreset *preset = new VCXYPadPreset(0xff);
            if (preset->loadXML(root))
                addPresetInternal(preset);
            else
                delete preset;
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown XY pad tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    updateFixtureList();
    setCurrentPosition(currPos);
    if (hasFloorPos)
        setFloorPosition(floorPos);
    emit presetsListChanged();

    return true;
}

void VCXYPad::saveXMLAxis(QXmlStreamWriter *doc, const QString &axisID,
                          qreal min, qreal max, bool reverse) const
{
    // the default is the full range, not reversed: skip the element entirely
    // rather than bloating the project file with redundant data
    if (min == 0.0 && max == 1.0 && reverse == false)
        return;

    doc->writeStartElement(KXMLQLCVCXYPadFixtureAxis);
    doc->writeAttribute(KXMLQLCVCXYPadFixtureAxisID, axisID);
    doc->writeAttribute(KXMLQLCVCXYPadFixtureAxisLowLimit, QString("%1").arg(min));
    doc->writeAttribute(KXMLQLCVCXYPadFixtureAxisHighLimit, QString("%1").arg(max));
    doc->writeAttribute(KXMLQLCVCXYPadFixtureAxisReverse,
                        reverse ? KXMLQLCTrue : KXMLQLCFalse);
    doc->writeEndElement();
}

bool VCXYPad::saveXMLFixture(QXmlStreamWriter *doc, const XYPadFixture &fxItem) const
{
    Q_ASSERT(doc != NULL);

    if (fxItem.m_groupID != FixtureGroup::invalidId())
    {
        /* VCXYPad Fixture Group */
        doc->writeStartElement(KXMLQLCVCXYPadGroup);
        doc->writeAttribute(KXMLQLCVCXYPadGroupID, QString("%1").arg(fxItem.m_groupID));
    }
    else
    {
        /* VCXYPad Fixture */
        doc->writeStartElement(KXMLQLCVCXYPadFixture);
        doc->writeAttribute(KXMLQLCVCXYPadFixtureID, QString("%1").arg(fxItem.m_head.fxi));
        doc->writeAttribute(KXMLQLCVCXYPadFixtureHead, QString("%1").arg(fxItem.m_head.head));
    }

    saveXMLAxis(doc, KXMLQLCVCXYPadFixtureAxisX, fxItem.m_xMin, fxItem.m_xMax, fxItem.m_xReverse);
    saveXMLAxis(doc, KXMLQLCVCXYPadFixtureAxisY, fxItem.m_yMin, fxItem.m_yMax, fxItem.m_yReverse);

    /* End the <Fixture>/<Group> tag */
    doc->writeEndElement();

    return true;
}

bool VCXYPad::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != nullptr);

    /* VC object entry */
    doc->writeStartElement(KXMLQLCVCXYPad);

    saveXMLCommon(doc);

    doc->writeAttribute(KXMLQLCVCXYPadInvertedAppearance, QString::number(invertedAppearance()));

    if (m_floorControl)
        doc->writeAttribute(KXMLQLCVCXYPadFloorControl, QString::number(1));

    /* Window state */
    saveXMLWindowState(doc);

    /* Appearance */
    saveXMLAppearance(doc);

    /* Fixtures */
    for (const XYPadFixture &fixture : m_fixtures)
        saveXMLFixture(doc, fixture);

    /* Custom range window */
    if (m_horizontalRange.x() != 0 ||
        m_horizontalRange.x() != 255 ||
        m_verticalRange.x() != 0 ||
        m_verticalRange.y() != 256)
    {
        doc->writeStartElement(KXMLQLCVCXYPadRangeWindow);
        doc->writeAttribute(KXMLQLCVCXYPadRangeHorizMin, QString::number(m_horizontalRange.x()));
        doc->writeAttribute(KXMLQLCVCXYPadRangeHorizMax, QString::number(m_horizontalRange.y()));
        doc->writeAttribute(KXMLQLCVCXYPadRangeVertMin, QString::number(m_verticalRange.x()));
        doc->writeAttribute(KXMLQLCVCXYPadRangeVertMax, QString::number(m_verticalRange.y()));
        doc->writeEndElement();
    }

    /* Pan */
    doc->writeStartElement(KXMLQLCVCXYPadPan);
    doc->writeAttribute(KXMLQLCVCXYPadPosition, QString::number(m_currentPosition.x()));
    saveXMLInputControl(doc, INPUT_PAN_ID, false);
    doc->writeEndElement();

    /* Tilt */
    doc->writeStartElement(KXMLQLCVCXYPadTilt);
    doc->writeAttribute(KXMLQLCVCXYPadPosition, QString::number(m_currentPosition.y()));
    saveXMLInputControl(doc, INPUT_TILT_ID, false);
    doc->writeEndElement();

    saveXMLInputControl(doc, INPUT_PAN_FINE_ID, false, KXMLQLCVCXYPadPanFine);
    saveXMLInputControl(doc, INPUT_TILT_FINE_ID, false, KXMLQLCVCXYPadTiltFine);
    saveXMLInputControl(doc, INPUT_WIDTH_ID, false, KXMLQLCVCXYPadWidth);
    saveXMLInputControl(doc, INPUT_HEIGHT_ID, false, KXMLQLCVCXYPadHeight);

    /* Floor control target position */
    doc->writeStartElement(KXMLQLCVCXYPadFloorPosition);
    doc->writeAttribute(KXMLQLCVCXYPadFloorPosX, QString::number(m_floorPosition.x()));
    doc->writeAttribute(KXMLQLCVCXYPadFloorPosY, QString::number(m_floorPosition.y()));
    doc->writeAttribute(KXMLQLCVCXYPadFloorPosZ, QString::number(m_floorPosition.z()));
    saveXMLInputControl(doc, INPUT_FLOOR_HEIGHT_ID, false);
    doc->writeEndElement();

    for (const VCXYPadPreset *preset : presets())
        preset->saveXML(doc);

    /* Write the <end> tag */
    doc->writeEndElement();

    return true;
}
