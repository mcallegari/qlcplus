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
#include "fixturemanager.h"
#include "qlcfixturemode.h"
#include "fixturegroup.h"
#include "genericfader.h"
#include "fadechannel.h"
#include "qlcchannel.h"
#include "listmodel.h"
#include "treemodel.h"
#include "qlcmacros.h"
#include "function.h"
#include "universe.h"
#include "scene.h"
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
#define INPUT_PRESETS_BASE_ID   30

VCXYPad::VCXYPad(Doc *doc, QObject *parent)
    : VCWidget(doc, parent)
    , m_invertedAppearance(false)
    , m_displayMode(Degrees)
    , m_currentPosition(QPointF(0, 0))
    , m_horizontalRange(QPointF(0.0, 255.0))
    , m_verticalRange(QPointF(0.0, 255.0))
    , m_positionChanged(false)
    , m_fixtureTree(nullptr)
    , m_searchFilter(QString())
    , m_lastAssignedPresetId(15)
    , m_activePresetId(-1)
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

    m_currentPosition = newCurrentPosition;

    m_x16 = posToU16(m_currentPosition.x());
    m_y16 = posToU16(m_currentPosition.y());

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
    if (reference.canConvert<Universe *>())
    {
        Universe *uni = reference.value<Universe *>();
        if (uni == nullptr)
            return;

        for (Fixture *fixture : m_doc->fixtures())
        {
            if (fixture->universe() != uni->id())
                continue;
            addFixture(QVariant::fromValue(fixture));
        }
    }
    else if (reference.canConvert<FixtureGroup *>())
    {
        FixtureGroup *group = reference.value<FixtureGroup *>();
        if (group == nullptr)
            return;

        for (const GroupHead &head : group->headList())
            addHead(head.fxi, head.head);
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

        qDebug() << "Removing fixture" << fixtureID << "head" << headIndex;

        int fIdx = 0;
        for (XYPadFixture &fixture : m_fixtures)
        {
            if (fixture.m_head.fxi == fixtureID && fixture.m_head.head == headIndex)
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
        entry.insert("headsCount", preset->m_fxGroup.count());
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

    if (reference.canConvert<Universe *>())
    {
        Universe *uni = reference.value<Universe *>();
        if (uni != nullptr)
        {
            for (const XYPadFixture &fixture : m_fixtures)
            {
                Fixture *fxi = m_doc->fixture(fixture.m_head.fxi);
                if (fxi != nullptr && fxi->universe() == uni->id())
                    heads.append(fixture.m_head);
            }
        }
    }
    else if (reference.canConvert<FixtureGroup *>())
    {
        FixtureGroup *group = reference.value<FixtureGroup *>();
        if (group != nullptr)
            heads = group->headList();
    }
    else if (reference.canConvert<Fixture *>())
    {
        Fixture *fixture = reference.value<Fixture *>();
        if (fixture != nullptr)
        {
            for (const XYPadFixture &fxItem : m_fixtures)
            {
                if (fxItem.m_head.fxi == fixture->id())
                    heads.append(fxItem.m_head);
            }
        }
    }

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
        return;
    }

    if (m_activePresetId == presetId)
    {
        deactivatePreset(preset);
        setActivePresetId(-1);
        return;
    }

    if (m_activePresetId >= 0)
        deactivatePreset(findPreset(m_activePresetId));

    if (activatePreset(preset))
        setActivePresetId(presetId);
    else
        setActivePresetId(-1);
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
        if (fixture.m_head == head)
            return true;
    }
    return false;
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
        function->start(m_doc->masterTimer(), functionParent());
        emit functionStarting(this, function->id(), intensity());
        return true;
    }

    if (preset->m_type == VCXYPadPreset::FixtureGroup)
    {
        for (XYPadFixture &fixture : m_fixtures)
            fixture.m_enabled = preset->m_fxGroup.contains(fixture.m_head);

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
    QVariantList positions;

    for (const XYPadFixture &fixture : m_fixtures)
    {
        if (fixture.m_enabled == false)
            continue;

        if (fixture.m_universe != idx)
            continue;

        if (fixture.m_xMSB == QLCChannel::invalid() || fixture.m_yMSB == QLCChannel::invalid())
            continue;

        Fixture *fxi = m_doc->fixture(fixture.m_head.fxi);
        if (fxi == nullptr)
            continue;

        quint32 fixtureAddress = fxi->address();
        if (fixtureAddress == QLCChannel::invalid())
            continue;

        int x = -1;
        int y = -1;

        if ((fixture.m_xMSB + fixtureAddress) < quint32(universeData.size()))
            x = int(uchar(universeData.at(fixture.m_xMSB + fixtureAddress))) * 256;
        if ((fixture.m_yMSB + fixtureAddress) < quint32(universeData.size()))
            y = int(uchar(universeData.at(fixture.m_yMSB + fixtureAddress))) * 256;

        if (x == -1 || y == -1)
            continue;

        if (fixture.m_xLSB != QLCChannel::invalid() &&
            (fixture.m_xLSB + fixtureAddress) < quint32(universeData.size()))
        {
            x += int(uchar(universeData.at(fixture.m_xLSB + fixtureAddress)));
        }
        if (fixture.m_yLSB != QLCChannel::invalid() &&
            (fixture.m_yLSB + fixtureAddress) < quint32(universeData.size()))
        {
            y += int(uchar(universeData.at(fixture.m_yLSB + fixtureAddress)));
        }

        qreal xNorm = qreal(x) / qreal(USHRT_MAX);
        qreal yNorm = qreal(y) / qreal(USHRT_MAX);

        if (invertedAppearance())
            yNorm = 1.0 - yNorm;

        QVariantMap posMap;
        posMap.insert("x", xNorm * 256.0);
        posMap.insert("y", yNorm * 256.0);
        positions.append(posMap);
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

        const quint32 universe = fixture.m_universe;
        if (universe == Universe::invalid())
            continue;

        if (universe >= quint32(universes.size()) || universes[universe] == nullptr)
            continue;

        // Skip fixtures that cannot be driven (do NOT abort the whole pad)
        if (fixture.m_xMSB == QLCChannel::invalid() || fixture.m_yMSB == QLCChannel::invalid())
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

        FadeChannel *fc = fader->getChannelFader(m_doc, pUniverse, fixture.m_head.fxi, fixture.m_xMSB);
        updateChannel(fc, uchar(xVal >> 8));

        fc = fader->getChannelFader(m_doc, pUniverse, fixture.m_head.fxi, fixture.m_yMSB);
        updateChannel(fc, uchar(yVal >> 8));

        if (fixture.m_xLSB != QLCChannel::invalid())
        {
            fc = fader->getChannelFader(m_doc, pUniverse, fixture.m_head.fxi, fixture.m_xLSB);
            updateChannel(fc, uchar(xVal & 0xFF));
        }

        if (fixture.m_yLSB != QLCChannel::invalid())
        {
            fc = fader->getChannelFader(m_doc, pUniverse, fixture.m_head.fxi, fixture.m_yLSB);
            updateChannel(fc, uchar(yVal & 0xFF));
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
    initXYFixtureItem(fxItem);

    /* Fixture ID */
    fxItem.m_head.fxi = root.attributes().value(KXMLQLCVCXYPadFixtureID).toUInt();
    fxItem.m_head.head = root.attributes().value(KXMLQLCVCXYPadFixtureHead).toInt();

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
    emit presetsListChanged();

    return true;
}

bool VCXYPad::saveXMLFixture(QXmlStreamWriter *doc, const XYPadFixture &fxItem) const
{
    Q_ASSERT(doc != NULL);

    /* VCXYPad Fixture */
    doc->writeStartElement(KXMLQLCVCXYPadFixture);
    doc->writeAttribute(KXMLQLCVCXYPadFixtureID, QString("%1").arg(fxItem.m_head.fxi));
    doc->writeAttribute(KXMLQLCVCXYPadFixtureHead, QString("%1").arg(fxItem.m_head.head));

    /* X-Axis */
    doc->writeStartElement(KXMLQLCVCXYPadFixtureAxis);
    doc->writeAttribute(KXMLQLCVCXYPadFixtureAxisID, KXMLQLCVCXYPadFixtureAxisX);
    doc->writeAttribute(KXMLQLCVCXYPadFixtureAxisLowLimit, QString("%1").arg(fxItem.m_xMin));
    doc->writeAttribute(KXMLQLCVCXYPadFixtureAxisHighLimit, QString("%1").arg(fxItem.m_xMax));
    if (fxItem.m_xReverse == true)
        doc->writeAttribute(KXMLQLCVCXYPadFixtureAxisReverse, KXMLQLCTrue);
    else
        doc->writeAttribute(KXMLQLCVCXYPadFixtureAxisReverse, KXMLQLCFalse);
    doc->writeEndElement();

    /* Y-Axis */
    doc->writeStartElement(KXMLQLCVCXYPadFixtureAxis);
    doc->writeAttribute(KXMLQLCVCXYPadFixtureAxisID, KXMLQLCVCXYPadFixtureAxisY);
    doc->writeAttribute(KXMLQLCVCXYPadFixtureAxisLowLimit, QString("%1").arg(fxItem.m_yMin));
    doc->writeAttribute(KXMLQLCVCXYPadFixtureAxisHighLimit, QString("%1").arg(fxItem.m_yMax));
    if (fxItem.m_yReverse == true)
        doc->writeAttribute(KXMLQLCVCXYPadFixtureAxisReverse, KXMLQLCTrue);
    else
        doc->writeAttribute(KXMLQLCVCXYPadFixtureAxisReverse, KXMLQLCFalse);
    doc->writeEndElement();

    /* End the <Fixture> tag */
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

    for (const VCXYPadPreset *preset : presets())
        preset->saveXML(doc);

    /* Write the <end> tag */
    doc->writeEndElement();

    return true;
}
