/*
  Q Light Controller Plus
  stagewizard.cpp

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

#include "stagewizard.h"

#include "doc.h"
#include "fixture.h"
#include "scene.h"
#include "scenevalue.h"
#include "chaser.h"
#include "chaserstep.h"
#include "efx.h"
#include "efxfixture.h"
#include "rgbmatrix.h"
#include "rgbalgorithm.h"
#include "fixturegroup.h"
#include "monitorproperties.h"
#include "qlccapability.h"
#include "qlcchannel.h"
#include "qlcfixturehead.h"
#include "qlcfixturemode.h"

#include "fixturemanager.h"
#include "virtualconsole/virtualconsole.h"
#include "virtualconsole/vcpage.h"
#include "virtualconsole/vcframe.h"
#include "virtualconsole/vcsoloframe.h"
#include "virtualconsole/vcbutton.h"
#include "virtualconsole/vcslider.h"
#include "virtualconsole/vccuelist.h"
#include "virtualconsole/vcxypad.h"
#include "contextmanager.h"

#include <QtMath>
#include <QDebug>

// ── Helpers ──────────────────────────────────────────────────────────────────

static const int kBtnW  = 17;
static const int kBtnH  = 17;
static const int kSlW   = 15;
static const int kSlH   = 40;
static const int kListW = 80;
static const int kListH = 50;
static const int kPadW  = 50;
static const int kPadH  = 50;
static const int kFrmPad = 4;

// ── Construction ──────────────────────────────────────────────────────────────

StageWizard::StageWizard(Doc *doc,
                         FixtureManager *fixtureManager,
                         VirtualConsole *virtualConsole,
                         ContextManager  *contextManager,
                         QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_fixtureManager(fixtureManager)
    , m_virtualConsole(virtualConsole)
    , m_contextManager(contextManager)
    , m_currentStep(0)
    , m_showType(ClubNight)
    , m_stageType(0) // MonitorProperties::StageSimple
    , m_isGenerating(false)
{
    buildEffectsModel();

    // Auto-assign newly patched fixtures to the armed drop-target box
    // Collect IDs of fixtures patched via the browser drag. Doc::fixtureAdded
    // fires *before* FixtureManager sets the fixture name, so we only record the
    // IDs here and assign them to the target box once patching is fully done
    // (fixturesCountChanged is emitted at the end of addFixture()).
    connect(m_doc, &Doc::fixtureAdded, this, &StageWizard::slotFixtureAdded);
    connect(m_fixtureManager, &FixtureManager::fixturesCountChanged, this, [this]()
    {
        if (m_droppedFixtureIDs.isEmpty())
            return;
        if (m_pendingDropGroup >= 0 && m_pendingDropGroup < m_groups.count())
        {
            for (quint32 id : m_droppedFixtureIDs)
                assignFixtureToGroup(m_pendingDropGroup, id);
        }
        m_droppedFixtureIDs.clear();
    });
}

StageWizard::~StageWizard()
{
}

// ── Navigation ────────────────────────────────────────────────────────────────

int StageWizard::currentStep() const { return m_currentStep; }

void StageWizard::setCurrentStep(int step)
{
    if (step == m_currentStep || step < 0 || step >= totalSteps())
        return;

    // On entering step 2: load existing groups (only the first time)
    if (step == 1 && m_groups.isEmpty())
        loadExistingGroups();

    // On entering step 3: set stage default for show type and auto-place
    if (step == 2)
    {
        if (m_showType == ClubNight)     setStageType(1); // StageBox
        else if (m_showType == Concert)  setStageType(2); // StageRock
        else if (m_showType == Theatrical) setStageType(3); // StageTheatre
        else                             setStageType(0); // StageSimple
    }

    // On entering step 4: rebuild effects model with current group capabilities
    if (step == 3)
    {
        buildEffectsModel();
        applyShowTypeDefaults();
    }

    m_currentStep = step;
    emit currentStepChanged(m_currentStep);
    emit canGoNextChanged();
}

// ── Step 1 ────────────────────────────────────────────────────────────────────

int StageWizard::showType() const { return m_showType; }

void StageWizard::setShowType(int type)
{
    if (m_showType == type)
        return;
    m_showType = type;
    emit showTypeChanged(m_showType);
    emit canGoNextChanged();
}

// ── Step 2 ────────────────────────────────────────────────────────────────────

QVariant StageWizard::groupsModel() const
{
    QVariantList list;
    for (int i = 0; i < m_groups.count(); i++)
    {
        const FixtureGroupEntry &e = m_groups.at(i);
        QVariantMap m;
        m["index"]        = i;
        m["name"]         = e.name;
        m["fixtureCount"] = e.fixtureIDs.count();
        m["selected"]     = e.selected;
        list.append(m);
    }
    return QVariant::fromValue(list);
}

void StageWizard::loadExistingGroups()
{
    m_groups.clear();
    m_groupCounter = 0;

    for (FixtureGroup *grp : m_doc->fixtureGroups())
    {
        if (grp == nullptr)
            continue;

        FixtureGroupEntry e;
        e.groupId  = grp->id();
        e.name     = grp->name();
        e.selected = false;
        e.role     = RoleKey;
        e.hasMovement = e.hasRGB = e.hasCMY = e.hasGobo = e.hasShutter = e.hasDimmer = false;

        for (quint32 fxID : grp->fixtureList())
        {
            if (m_doc->fixture(fxID) != nullptr)
                e.fixtureIDs.append(fxID);
        }

        detectGroupCapabilities(e);
        m_groups.append(e);
        m_groupCounter++;
    }

    emit groupsModelChanged();
    rebuildRoleModel();
}

int StageWizard::addGroup()
{
    FixtureGroupEntry e;
    e.groupId  = FixtureGroup::invalidId();
    e.name     = tr("Group %1").arg(++m_groupCounter);
    e.selected = false;
    e.role     = RoleKey;
    e.hasMovement = e.hasRGB = e.hasCMY = e.hasGobo = e.hasShutter = e.hasDimmer = false;
    m_groups.append(e);

    emit groupsModelChanged();
    emit canGoNextChanged();
    return m_groups.count() - 1;
}

void StageWizard::removeGroup(int groupIndex)
{
    if (groupIndex < 0 || groupIndex >= m_groups.count())
        return;

    m_groups.removeAt(groupIndex);
    emit groupsModelChanged();
    emit canGoNextChanged();
    rebuildRoleModel();
}

void StageWizard::renameGroup(int groupIndex, const QString &name)
{
    if (groupIndex < 0 || groupIndex >= m_groups.count() || name.isEmpty())
        return;

    m_groups[groupIndex].name = name;
    emit groupsModelChanged();
    rebuildRoleModel();
}

void StageWizard::setGroupSelected(int groupIndex, bool selected)
{
    if (groupIndex < 0 || groupIndex >= m_groups.count())
        return;

    m_groups[groupIndex].selected = selected;
    emit groupsModelChanged();
    rebuildRoleModel();
}

void StageWizard::assignFixtureToGroup(int groupIndex, quint32 fixtureID)
{
    if (groupIndex < 0 || groupIndex >= m_groups.count())
        return;
    if (m_doc->fixture(fixtureID) == nullptr)
        return;

    // A fixture belongs to a single box: remove it from any other box first
    for (FixtureGroupEntry &e : m_groups)
        e.fixtureIDs.removeAll(fixtureID);

    m_groups[groupIndex].fixtureIDs.append(fixtureID);
    detectGroupCapabilities(m_groups[groupIndex]);

    emit groupsModelChanged();
    emit canGoNextChanged();
    rebuildRoleModel();
}

void StageWizard::removeFixtureFromGroup(int groupIndex, quint32 fixtureID)
{
    if (groupIndex < 0 || groupIndex >= m_groups.count())
        return;

    m_groups[groupIndex].fixtureIDs.removeAll(fixtureID);
    detectGroupCapabilities(m_groups[groupIndex]);

    emit groupsModelChanged();
    emit canGoNextChanged();
    rebuildRoleModel();
}

QVariant StageWizard::groupFixtures(int groupIndex) const
{
    QVariantList list;
    if (groupIndex < 0 || groupIndex >= m_groups.count())
        return QVariant::fromValue(list);

    for (quint32 fxID : m_groups.at(groupIndex).fixtureIDs)
    {
        Fixture *fx = m_doc->fixture(fxID);
        if (fx == nullptr)
            continue;

        QVariantMap m;
        m["id"]       = fxID;
        m["name"]     = fx->name();
        m["address"]  = fx->address() + 1;   // present 1-based
        m["universe"] = fx->universe() + 1;   // present 1-based
        list.append(m);
    }
    return QVariant::fromValue(list);
}

void StageWizard::setPendingDropGroup(int groupIndex)
{
    m_pendingDropGroup = groupIndex;
}

void StageWizard::slotFixtureAdded(quint32 fixtureID)
{
    // Only record while a drop target is armed; the actual assignment happens
    // on fixturesCountChanged, once the fixture name has been set.
    if (m_pendingDropGroup < 0 || m_pendingDropGroup >= m_groups.count())
        return;

    m_droppedFixtureIDs.append(fixtureID);
}

void StageWizard::detectGroupCapabilities(FixtureGroupEntry &e) const
{
    e.hasMovement = e.hasRGB = e.hasCMY = e.hasGobo = e.hasShutter = e.hasDimmer = false;

    for (quint32 fxID : e.fixtureIDs)
    {
        Fixture *fx = m_doc->fixture(fxID);
        if (fx == nullptr)
            continue;

        bool hasPan = false, hasTilt = false;
        bool hasR = false, hasG = false, hasB = false;
        bool hasC = false, hasM = false, hasY = false;

        for (quint32 ch = 0; ch < fx->channels(); ++ch)
        {
            const QLCChannel *channel = fx->channel(ch);
            if (!channel) continue;
            switch (channel->group())
            {
                case QLCChannel::Pan:       hasPan  = true; break;
                case QLCChannel::Tilt:      hasTilt = true; break;
                case QLCChannel::Gobo:
                    if (channel->capabilities().size() > 1) e.hasGobo = true;
                    break;
                case QLCChannel::Shutter:
                    if (channel->capabilities().size() > 1) e.hasShutter = true;
                    break;
                case QLCChannel::Intensity:
                    switch (channel->colour())
                    {
                        case QLCChannel::Red:     hasR = true; break;
                        case QLCChannel::Green:   hasG = true; break;
                        case QLCChannel::Blue:    hasB = true; break;
                        case QLCChannel::Cyan:    hasC = true; break;
                        case QLCChannel::Magenta: hasM = true; break;
                        case QLCChannel::Yellow:  hasY = true; break;
                        default:
                            e.hasDimmer = true;
                            break;
                    }
                    break;
                default: break;
            }
        }
        if (hasPan && hasTilt)    e.hasMovement = true;
        if (hasR && hasG && hasB) e.hasRGB      = true;
        if (hasC && hasM && hasY) e.hasCMY      = true;
    }
}

void StageWizard::rebuildRoleModel()
{
    autoDetectRoles();
    emit fixtureRoleModelChanged();
    emit canGoNextChanged();
}

QString StageWizard::capabilityGroupName(const Fixture *fx) const
{
    if (!fx) return tr("Generic");

    bool hasPan = false, hasTilt = false;
    bool hasR = false, hasG = false, hasB = false;
    bool hasGobo = false, hasShutter = false, hasDimmer = false;

    for (quint32 ch = 0; ch < fx->channels(); ++ch)
    {
        const QLCChannel *c = fx->channel(ch);
        if (!c) continue;
        switch (c->group())
        {
            case QLCChannel::Pan:     hasPan  = true; break;
            case QLCChannel::Tilt:    hasTilt = true; break;
            case QLCChannel::Gobo:    if (c->capabilities().size() > 1) hasGobo = true; break;
            case QLCChannel::Shutter: if (c->capabilities().size() > 1) hasShutter = true; break;
            case QLCChannel::Intensity:
                switch (c->colour())
                {
                    case QLCChannel::Red:
                    case QLCChannel::Green:
                    case QLCChannel::Blue:   hasR = hasG = hasB = true; break;
                    default: hasDimmer = true; break;
                }
                break;
            default: break;
        }
    }

    if (hasPan && hasTilt && hasGobo && hasShutter)    return tr("Moving Head Spot");
    if (hasPan && hasTilt && hasR && hasG && hasB)     return tr("Moving Head Wash");
    if (hasPan && hasTilt)                             return tr("Moving Head");
    if (hasR && hasG && hasB && !hasPan)               return tr("LED PAR / Wash");
    if (hasGobo && !hasPan)                            return tr("Static Gobo");
    if (hasShutter && hasDimmer && !hasPan)            return tr("Strobe / Blinder");
    if (hasDimmer && !hasR && !hasPan)                 return tr("Dimmer");
    return tr("Generic");
}

void StageWizard::autoDetectRoles()
{
    for (FixtureGroupEntry &e : m_groups)
    {
        if (e.hasMovement && e.hasGobo)   { e.role = RoleEffect;  continue; }
        if (e.hasMovement && e.hasRGB)    { e.role = RoleKey;     continue; }
        if (e.hasMovement)                { e.role = RoleKey;     continue; }
        if (e.name.contains(tr("Strobe"), Qt::CaseInsensitive) ||
            e.name.contains(tr("Blinder"), Qt::CaseInsensitive))
                                          { e.role = RoleBlinder; continue; }
        if (e.name.contains(tr("Strip"), Qt::CaseInsensitive) ||
            e.name.contains(tr("Bar"), Qt::CaseInsensitive))
                                          { e.role = RoleStrip;   continue; }
        if (e.hasDimmer && !e.hasRGB)     { e.role = RoleFill;    continue; }
        e.role = RoleFill;
    }
}

QVariant StageWizard::fixtureRoleModel() const
{
    // Column 3 reflects only the selected boxes
    QVariantList list;
    for (int i = 0; i < m_groups.count(); i++)
    {
        const FixtureGroupEntry &e = m_groups.at(i);
        if (!e.selected)
            continue;

        QVariantMap m;
        m["index"]        = i;            // real index into m_groups
        m["name"]         = e.name;
        m["fixtureCount"] = e.fixtureIDs.count();
        m["role"]         = static_cast<int>(e.role);
        m["hasMovement"]  = e.hasMovement;
        m["hasRGB"]       = e.hasRGB || e.hasCMY;
        m["hasGobo"]      = e.hasGobo;
        m["hasShutter"]   = e.hasShutter;
        m["hasDimmer"]    = e.hasDimmer;
        list.append(m);
    }
    return QVariant::fromValue(list);
}

void StageWizard::setGroupRole(int groupIndex, int role)
{
    if (groupIndex < 0 || groupIndex >= m_groups.count())
        return;
    m_groups[groupIndex].role = static_cast<FixtureRole>(role);
    emit fixtureRoleModelChanged();
}

// ── Step 3 ────────────────────────────────────────────────────────────────────

int StageWizard::stageType() const { return m_stageType; }

void StageWizard::setStageType(int type)
{
    if (m_stageType == type)
        return;
    m_stageType = type;

    // Push to 3D view immediately so user sees change live
    if (m_contextManager)
    {
        MonitorProperties *props = m_doc->monitorProperties();
        props->setStageType(static_cast<MonitorProperties::StageType>(type));
    }

    applyStageLayout();
    emit stageTypeChanged(m_stageType);
}

/*
 * Coordinate system used by MonitorProperties::setFixturePosition():
 *   - units are MILLIMETRES
 *   - origin is a stage CORNER (not the centre). MainView3D converts with:
 *         worldX = pos.x/1000 - gridX/2 + meshExtents.x/2
 *         worldY = pos.y/1000          + meshExtents.y/2
 *         worldZ = pos.z/1000 - gridZ/2 + meshExtents.z/2
 *   - therefore in monitor space:
 *         x in [0, gridX*1000]   left .. right
 *         y in [0, gridY*1000]   floor .. top of trusses
 *         z in [0, gridZ*1000]   rear  .. front (audience side)
 *
 * The trusses (Box/Rock stages) sit at the top (y = gridY) along the stage
 * perimeter: front truss at z = gridZ, rear at z = 0, side trusses at x = 0/gridX.
 */
void StageWizard::applyStageLayout()
{
    MonitorProperties *props = m_doc->monitorProperties();

    for (const FixtureGroupEntry &grp : m_groups)
    {
        if (!grp.selected || grp.fixtureIDs.isEmpty())
            continue;

        // One placement slot per head so multi-head bars spread out too
        QList<QPair<quint32, int>> placements;
        for (quint32 fxID : grp.fixtureIDs)
        {
            Fixture *fx = m_doc->fixture(fxID);
            if (!fx) continue;
            int heads = qMax(1, fx->heads());
            for (int h = 0; h < heads; ++h)
                placements.append(qMakePair(fxID, h));
        }

        int total = placements.count();
        for (int i = 0; i < total; ++i)
        {
            quint32 fxID = placements[i].first;
            int     head = placements[i].second;
            QVector3D pos = computePosition(i, total, grp.role, props->gridSize());
            QVector3D rot = computeRotation(grp.role, i, total);
            props->setFixturePosition(fxID, head, 0, pos);
            props->setFixtureRotation(fxID, head, 0, rot);
        }
    }
}

QVector3D StageWizard::computePosition(int index, int total,
                                       FixtureRole role,
                                       const QVector3D &gridM) const
{
    // Grid size in millimetres (monitor units)
    const float gx = gridM.x() * 1000.0f;
    const float gy = gridM.y() * 1000.0f;
    const float gz = gridM.z() * 1000.0f;

    // Nominal fixture size, so fixtures hang just under the truss bar instead of
    // floating above it (MainView3D adds half the mesh extents back).
    const float fxDrop = 300.0f; // ~30 cm

    // Evenly spread along the stage width, leaving a margin at both ends.
    // For N fixtures: centres at (k + 0.5)/N of the span.
    auto spreadX = [&](float fromX, float toX) -> float
    {
        if (total <= 1)
            return (fromX + toX) * 0.5f;
        return fromX + (toX - fromX) * ((index + 0.5f) / float(total));
    };
    const float margin = gx * 0.08f;   // keep clear of the truss corners
    const float topY   = gy - fxDrop;  // hung under the top truss
    const float frontZ = gz;           // front truss / downstage edge
    const float rearZ  = 0.0f;         // rear truss / upstage edge

    switch (role)
    {
        case RoleKey:
            // Front truss, hung overhead, spread across the full width
            return QVector3D(spreadX(margin, gx - margin), topY, frontZ);

        case RoleFill:
            // Front truss too, but a touch upstage of the key light row
            return QVector3D(spreadX(margin, gx - margin), topY, gz * 0.75f);

        case RoleBack:
            // Rear truss, overhead
            return QVector3D(spreadX(margin, gx - margin), topY, rearZ);

        case RoleEffect:
            // Top mid truss, over the centre line
            return QVector3D(spreadX(margin, gx - margin), topY, gz * 0.5f);

        case RoleStrip:
            // Full-width batten on the front truss
            return QVector3D(spreadX(0.0f, gx), topY, frontZ);

        case RoleBlinder:
            // Front truss, audience-facing
            return QVector3D(spreadX(margin, gx - margin), topY, frontZ);

        case RoleSide:
        {
            // Side trusses / booms: alternate stage-left and stage-right,
            // spread along depth, at mid height
            bool left = (index % 2 == 0);
            float xSide = left ? margin : gx - margin;
            int   pairIndex = index / 2;
            int   pairCount = (total + 1) / 2;
            float t = (pairCount <= 1) ? 0.5f : (pairIndex + 0.5f) / float(pairCount);
            float z = rearZ + (frontZ - rearZ) * t;
            return QVector3D(xSide, gy * 0.55f, z);
        }

        case RoleHazer:
            // On the floor, upstage centre
            return QVector3D(spreadX(gx * 0.3f, gx * 0.7f), 0.0f, gz * 0.2f);

        case RoleFloor:
            // Floor level, front row, uplighting
            return QVector3D(spreadX(margin, gx - margin), 0.0f, gz * 0.6f);

        default:
            return QVector3D(spreadX(margin, gx - margin), topY, frontZ);
    }
}

QVector3D StageWizard::computeRotation(FixtureRole role,
                                       int index, int total) const
{
    Q_UNUSED(index)
    Q_UNUSED(total)

    // Rotation in degrees: X = tilt (about world X), Y = pan (about Y), Z = roll.
    // Overhead fixtures hang inverted: a 180° rotation about X flips the body so
    // the base points up and the head hangs below the truss. Beam aim for movers
    // is then handled by pan/tilt at runtime.
    switch (role)
    {
        case RoleKey:      return QVector3D(180.0f,   0.0f, 0.0f); // hung, facing audience
        case RoleFill:     return QVector3D(180.0f,   0.0f, 0.0f);
        case RoleBack:     return QVector3D(180.0f, 180.0f, 0.0f); // hung, facing upstage
        case RoleEffect:   return QVector3D(180.0f,   0.0f, 0.0f);
        case RoleStrip:    return QVector3D(180.0f,   0.0f, 0.0f);
        case RoleBlinder:  return QVector3D(180.0f,   0.0f, 0.0f);
        case RoleSide:     return QVector3D(180.0f,  90.0f, 0.0f); // hung, aimed inward
        case RoleHazer:    return QVector3D(0.0f,     0.0f, 0.0f); // upright on floor
        case RoleFloor:    return QVector3D(0.0f,     0.0f, 0.0f); // upright, uplight
        default:           return QVector3D(180.0f,   0.0f, 0.0f);
    }
}

// ── Step 4 ────────────────────────────────────────────────────────────────────

void StageWizard::buildEffectsModel()
{
    // Determine which capabilities exist across all groups
    bool anyMovement = false, anyRGB = false, anyGobo = false;
    bool anyShutter = false, anyDimmer = false;
    for (const FixtureGroupEntry &g : m_groups)
    {
        if (g.hasMovement) anyMovement = true;
        if (g.hasRGB || g.hasCMY) anyRGB = true;
        if (g.hasGobo)    anyGobo    = true;
        if (g.hasShutter) anyShutter = true;
        if (g.hasDimmer)  anyDimmer  = true;
    }

    m_effects.clear();

    auto addEffect = [&](int flag, const QString &name,
                         const QString &family, bool avail)
    {
        EffectEntry e;
        e.flag      = flag;
        e.name      = name;
        e.family    = family;
        e.enabled   = false; // set by applyShowTypeDefaults()
        e.available = avail;
        m_effects.append(e);
    };

    // Color family
    addEffect(EffectColorPalette,   tr("Color Palette"),    tr("Color"),     anyRGB || anyGobo);
    addEffect(EffectColorRainbow,   tr("Color Rainbow"),    tr("Color"),     anyRGB);
    addEffect(EffectSplitColor,     tr("Split Color"),      tr("Color"),     anyRGB);
    addEffect(EffectGoboPalette,    tr("Gobo Palette"),     tr("Color"),     anyGobo);

    // Intensity / Strobe family
    addEffect(EffectShutter,        tr("Shutter Effects"),  tr("Intensity"), anyShutter);
    addEffect(EffectBlinderHit,     tr("Blinder Hit"),      tr("Intensity"), anyDimmer || anyShutter);
    addEffect(EffectStrobeChase,    tr("Strobe Chase"),     tr("Intensity"), anyShutter);
    addEffect(EffectHeartbeat,      tr("Heartbeat"),        tr("Intensity"), anyDimmer || anyRGB);

    // Movement family
    addEffect(EffectPositionPreset, tr("Position Presets"), tr("Movement"),  anyMovement);
    addEffect(EffectFlyOut,         tr("Fly Out"),          tr("Movement"),  anyMovement);
    addEffect(EffectFlyIn,          tr("Fly In"),           tr("Movement"),  anyMovement);
    addEffect(EffectCircleChase,    tr("Circle Chase"),     tr("Movement"),  anyMovement);
    addEffect(EffectFigureEight,    tr("Figure Eight"),     tr("Movement"),  anyMovement);
    addEffect(EffectAudienceSweep,  tr("Audience Sweep"),   tr("Movement"),  anyMovement);

    // Matrix family
    addEffect(EffectPixelChase,     tr("Pixel Chase"),      tr("Matrix"),    anyRGB);
    addEffect(EffectWave,           tr("Wave"),             tr("Matrix"),    anyRGB);
    addEffect(EffectFireworks,      tr("Fireworks"),        tr("Matrix"),    anyRGB);
    addEffect(EffectPlasma,         tr("Plasma"),           tr("Matrix"),    anyRGB);
    addEffect(EffectMarquee,        tr("Marquee"),          tr("Matrix"),    anyRGB);

    // Show cues family
    addEffect(EffectShowOpen,       tr("Show Open"),        tr("Show Cues"), true);
    addEffect(EffectShowClose,      tr("Show Close"),       tr("Show Cues"), true);
    addEffect(EffectBigMoment,      tr("Big Moment"),       tr("Show Cues"), true);
    addEffect(EffectAmbientLoop,    tr("Ambient Loop"),     tr("Show Cues"), anyRGB || anyDimmer);
}

void StageWizard::applyShowTypeDefaults()
{
    // Default all off, then turn on show-type presets
    for (EffectEntry &e : m_effects)
        e.enabled = false;

    auto enable = [&](int flag)
    {
        for (EffectEntry &e : m_effects)
            if (e.flag == flag && e.available)
                e.enabled = true;
    };

    switch (static_cast<ShowType>(m_showType))
    {
        case ClubNight:
            enable(EffectColorPalette);
            enable(EffectColorRainbow);
            enable(EffectSplitColor);
            enable(EffectBlinderHit);
            enable(EffectStrobeChase);
            enable(EffectHeartbeat);
            enable(EffectFlyOut);
            enable(EffectFlyIn);
            enable(EffectCircleChase);
            enable(EffectPixelChase);
            enable(EffectBigMoment);
            break;

        case Concert:
            enable(EffectColorPalette);
            enable(EffectColorRainbow);
            enable(EffectGoboPalette);
            enable(EffectBlinderHit);
            enable(EffectStrobeChase);
            enable(EffectPositionPreset);
            enable(EffectFlyOut);
            enable(EffectFlyIn);
            enable(EffectCircleChase);
            enable(EffectAudienceSweep);
            enable(EffectShowOpen);
            enable(EffectShowClose);
            enable(EffectBigMoment);
            break;

        case Theatrical:
            enable(EffectColorPalette);
            enable(EffectPositionPreset);
            enable(EffectGoboPalette);
            enable(EffectShowOpen);
            enable(EffectShowClose);
            enable(EffectAmbientLoop);
            break;

        case Architectural:
            enable(EffectColorPalette);
            enable(EffectPixelChase);
            enable(EffectWave);
            enable(EffectPlasma);
            enable(EffectAmbientLoop);
            break;

        case Custom:
            break;
    }

    emit effectsModelChanged();
}

QVariant StageWizard::effectsModel() const
{
    QVariantList list;
    for (const EffectEntry &e : m_effects)
    {
        QVariantMap m;
        m["flag"]      = e.flag;
        m["name"]      = e.name;
        m["family"]    = e.family;
        m["enabled"]   = e.enabled;
        m["available"] = e.available;
        m["preview"]   = effectPreview(e.flag);
        list.append(m);
    }
    return QVariant::fromValue(list);
}

void StageWizard::setEffectEnabled(int effectFlag, bool enabled)
{
    for (EffectEntry &e : m_effects)
    {
        if (e.flag == effectFlag)
        {
            e.enabled = enabled;
            emit effectsModelChanged();
            return;
        }
    }
}

QString StageWizard::effectPreview(int effectFlag) const
{
    // Rough preview counts – not computed yet, just indicative for the UI
    int rgbGroups = 0, movGroups = 0, allGroups = m_groups.count();
    for (const FixtureGroupEntry &g : m_groups)
    {
        if (g.hasRGB || g.hasCMY) rgbGroups++;
        if (g.hasMovement) movGroups++;
    }

    switch (effectFlag)
    {
        case EffectColorPalette:
            return tr("%1 scenes + %2 chasers").arg(7 * allGroups).arg(allGroups);
        case EffectGoboPalette:
            return tr("scenes + 1 chaser per group");
        case EffectShutter:
            return tr("shutter scenes + chaser per group");
        case EffectColorRainbow:
            return tr("%1 chaser(s)").arg(qMax(1, rgbGroups));
        case EffectSplitColor:
            return tr("%1 chaser(s)").arg(qMax(1, rgbGroups));
        case EffectPositionPreset:
            return tr("5 scenes × %1 group(s)").arg(movGroups);
        case EffectFlyOut:
        case EffectFlyIn:
        case EffectCircleChase:
        case EffectFigureEight:
        case EffectAudienceSweep:
            return tr("%1 EFX function(s)").arg(qMax(1, movGroups));
        case EffectBlinderHit:
            return tr("1 scene");
        case EffectStrobeChase:
            return tr("%1 chaser(s)").arg(allGroups);
        case EffectHeartbeat:
            return tr("%1 chaser(s)").arg(allGroups);
        case EffectPixelChase:
        case EffectWave:
        case EffectFireworks:
        case EffectPlasma:
        case EffectMarquee:
            return tr("%1 RGB matrix(es)").arg(qMax(1, rgbGroups));
        case EffectShowOpen:
        case EffectShowClose:
        case EffectBigMoment:
        case EffectAmbientLoop:
            return tr("1 chaser");
        default:
            return QString();
    }
}

// ── State ─────────────────────────────────────────────────────────────────────

bool StageWizard::canGoNext() const
{
    // Step 1 requires a show type selection (always true after default)
    // Step 2 requires at least one selected box that contains fixtures
    if (m_currentStep == 1)
    {
        for (const FixtureGroupEntry &e : m_groups)
            if (e.selected && !e.fixtureIDs.isEmpty())
                return true;
        return false;
    }
    return true;
}

bool StageWizard::isGenerating() const { return m_isGenerating; }

QVariant StageWizard::summaryModel() const
{
    QVariantList list;

    int enabledEffects = 0;
    for (const EffectEntry &e : m_effects)
        if (e.enabled) enabledEffects++;

    {
        QVariantMap m;
        m["section"] = tr("Stage");
        m["detail"]  = tr("%1 fixture group(s) positioned on stage type %2")
                           .arg(m_groups.count()).arg(m_stageType);
        list.append(m);
    }
    {
        QVariantMap m;
        m["section"] = tr("Functions");
        m["detail"]  = tr("%1 effect(s) selected").arg(enabledEffects);
        list.append(m);
    }
    {
        QVariantMap m;
        m["section"] = tr("Virtual Console");
        int vcGroups = 0;
        for (const FixtureGroupEntry &g : m_groups)
            if (!g.fixtureIDs.isEmpty()) vcGroups++;
        m["detail"]  = tr("1 main page + %1 group frame(s)").arg(vcGroups);
        list.append(m);
    }

    return QVariant::fromValue(list);
}

void StageWizard::reset()
{
    m_currentStep = 0;
    m_showType    = ClubNight;
    m_stageType   = 0;
    m_isGenerating = false;
    m_groups.clear();
    m_groupCounter = 0;
    m_pendingDropGroup = -1;
    m_droppedFixtureIDs.clear();
    m_generatedFunctionIDs.clear();
    m_generatedGroupIDs.clear();
    buildEffectsModel();

    emit currentStepChanged(m_currentStep);
    emit showTypeChanged(m_showType);
    emit stageTypeChanged(m_stageType);
    emit groupsModelChanged();
    emit fixtureRoleModelChanged();
    emit effectsModelChanged();
    emit canGoNextChanged();
    emit isGeneratingChanged();
}

// ── Generation pipeline ───────────────────────────────────────────────────────

void StageWizard::createFixtureGroups()
{
    for (FixtureGroupEntry &grp : m_groups)
    {
        if (!grp.selected || grp.fixtureIDs.isEmpty())
            continue;

        FixtureGroup *group = nullptr;
        bool isNew = false;

        if (grp.groupId != FixtureGroup::invalidId())
            group = m_doc->fixtureGroup(grp.groupId);

        if (group == nullptr)
        {
            group = new FixtureGroup(m_doc);
            isNew = true;
        }

        group->setName(grp.name);

        int cols = qMax(1, static_cast<int>(qCeil(qSqrt(grp.fixtureIDs.count()))));
        int rows = (grp.fixtureIDs.count() + cols - 1) / cols;
        group->setSize(QSize(cols, rows));

        // Re-assign all member fixtures into a clean grid
        group->reset();
        for (int i = 0; i < grp.fixtureIDs.count(); ++i)
            group->assignFixture(grp.fixtureIDs[i], QLCPoint(i % cols, i / cols));

        if (isNew)
        {
            m_doc->addFixtureGroup(group);
            grp.groupId = group->id();
        }

        m_generatedGroupIDs.append(group->id());
    }
}

void StageWizard::generate()
{
    if (m_isGenerating) return;

    m_isGenerating = true;
    emit isGeneratingChanged();

    m_generatedFunctionIDs.clear();
    m_generatedGroupIDs.clear();

    // Persist the user-defined boxes as real FixtureGroups
    createFixtureGroups();

    // First make sure stage layout is applied (user may have skipped step 3)
    applyStageLayout();

    // Blackout scene – always created
    Scene *blackout = makeBlackoutScene();
    if (blackout)
    {
        m_doc->addFunction(blackout);
        m_generatedFunctionIDs.append(blackout->id());
    }

    // Per-group generation (selected boxes only)
    for (const FixtureGroupEntry &grp : m_groups)
    {
        if (!grp.selected || grp.fixtureIDs.isEmpty())
            continue;

        const QString prefix = grp.name;

        for (const EffectEntry &eff : m_effects)
        {
            if (!eff.enabled || !eff.available)
                continue;

            switch (eff.flag)
            {
                case EffectColorPalette:
                    generateColorPalette(grp, prefix);
                    break;
                case EffectGoboPalette:
                    if (grp.hasGobo)
                        generateGoboPalette(grp, prefix);
                    break;
                case EffectShutter:
                    if (grp.hasShutter)
                        generateShutterEffects(grp, prefix);
                    break;
                case EffectPositionPreset:
                    if (grp.hasMovement)
                        generatePositionPresets(grp, prefix);
                    break;
                case EffectFlyOut:
                    if (grp.hasMovement)
                        generateEFX(grp, prefix, EFX::Line2, tr("Fly Out"));
                    break;
                case EffectFlyIn:
                    if (grp.hasMovement)
                        generateEFX(grp, prefix, EFX::Line2, tr("Fly In"));
                    break;
                case EffectCircleChase:
                    if (grp.hasMovement)
                        generateEFX(grp, prefix, EFX::Circle, tr("Circle Chase"));
                    break;
                case EffectFigureEight:
                    if (grp.hasMovement)
                        generateEFX(grp, prefix, EFX::Eight, tr("Figure Eight"));
                    break;
                case EffectAudienceSweep:
                    if (grp.hasMovement)
                        generateEFX(grp, prefix, EFX::Line, tr("Audience Sweep"));
                    break;
                case EffectColorRainbow:
                    if (grp.hasRGB || grp.hasCMY)
                    {
                        Chaser *ch = generateColorRainbow(grp, prefix);
                        if (ch) m_generatedFunctionIDs.append(ch->id());
                    }
                    break;
                case EffectSplitColor:
                    if (grp.hasRGB || grp.hasCMY)
                    {
                        Chaser *ch = generateSplitColor(grp, prefix);
                        if (ch) m_generatedFunctionIDs.append(ch->id());
                    }
                    break;
                case EffectBlinderHit:
                {
                    Scene *s = generateBlinderHit(grp, prefix);
                    if (s) m_generatedFunctionIDs.append(s->id());
                    break;
                }
                case EffectStrobeChase:
                    if (grp.hasShutter)
                    {
                        Chaser *ch = generateStrobeChase(grp, prefix);
                        if (ch) m_generatedFunctionIDs.append(ch->id());
                    }
                    break;
                case EffectHeartbeat:
                {
                    Chaser *ch = generateHeartbeat(grp, prefix);
                    if (ch) m_generatedFunctionIDs.append(ch->id());
                    break;
                }
                case EffectPixelChase:
                    if (grp.hasRGB || grp.hasCMY)
                        generateRGBMatrix(grp, "One By One", tr("Pixel Chase"));
                    break;
                case EffectWave:
                    if (grp.hasRGB || grp.hasCMY)
                        generateRGBMatrix(grp, "Sine wave", tr("Wave"));
                    break;
                case EffectFireworks:
                    if (grp.hasRGB || grp.hasCMY)
                        generateRGBMatrix(grp, "Fireworks", tr("Fireworks"));
                    break;
                case EffectPlasma:
                    if (grp.hasRGB || grp.hasCMY)
                        generateRGBMatrix(grp, "Plasma", tr("Plasma"));
                    break;
                case EffectMarquee:
                    if (grp.hasRGB || grp.hasCMY)
                        generateRGBMatrix(grp, "Marquee", tr("Marquee"));
                    break;
                default:
                    break;
            }
        }
    }

    // Show-level cues (run once, not per group)
    for (const EffectEntry &eff : m_effects)
    {
        if (!eff.enabled) continue;
        switch (eff.flag)
        {
            case EffectShowOpen:  { auto *c = generateShowOpen();   if (c) m_generatedFunctionIDs.append(c->id()); break; }
            case EffectShowClose: { auto *c = generateShowClose();  if (c) m_generatedFunctionIDs.append(c->id()); break; }
            case EffectBigMoment: { auto *c = generateBigMoment();  if (c) m_generatedFunctionIDs.append(c->id()); break; }
            case EffectAmbientLoop:{ auto *c = generateAmbientLoop(); if (c) m_generatedFunctionIDs.append(c->id()); break; }
            default: break;
        }
    }

    createVCLayout();

    m_isGenerating = false;
    emit isGeneratingChanged();
    emit summaryModelChanged();
    emit generationComplete();
}

// ── Generation helpers ────────────────────────────────────────────────────────

Scene *StageWizard::makeBlackoutScene()
{
    Scene *s = new Scene(m_doc);
    s->setName(tr("Blackout"));
    for (const FixtureGroupEntry &grp : m_groups)
        for (quint32 fxID : grp.fixtureIDs)
        {
            Fixture *fx = m_doc->fixture(fxID);
            if (!fx) continue;
            for (quint32 ch = 0; ch < fx->channels(); ++ch)
            {
                const QLCChannel *c = fx->channel(ch);
                if (c && c->group() == QLCChannel::Intensity)
                    s->setValue(SceneValue(fxID, ch, 0));
            }
        }
    return s;
}

Scene *StageWizard::makeFullScene(const FixtureGroupEntry &grp, const QString &name)
{
    Scene *s = new Scene(m_doc);
    s->setName(name);
    for (quint32 fxID : grp.fixtureIDs)
    {
        Fixture *fx = m_doc->fixture(fxID);
        if (!fx) continue;
        for (quint32 ch = 0; ch < fx->channels(); ++ch)
        {
            const QLCChannel *c = fx->channel(ch);
            if (c && c->group() == QLCChannel::Intensity)
                s->setValue(SceneValue(fxID, ch, 255));
        }
    }
    return s;
}

Chaser *StageWizard::makeChaserFromScenes(const QList<Scene *> &scenes,
                                          const QString &name,
                                          uint fadeMs, uint holdMs)
{
    Chaser *ch = new Chaser(m_doc);
    ch->setName(name);
    ch->setFadeInMode(Chaser::Common);
    ch->setFadeOutMode(Chaser::Common);
    ch->setDurationMode(Chaser::Common);
    ch->setFadeInSpeed(fadeMs);
    ch->setFadeOutSpeed(fadeMs);
    ch->setDuration(holdMs);

    for (Scene *s : scenes)
    {
        m_doc->addFunction(s);
        m_generatedFunctionIDs.append(s->id());
        ChaserStep step(s->id());
        step.fadeIn  = fadeMs;
        step.hold    = holdMs;
        step.fadeOut = fadeMs;
        ch->addStep(step);
    }
    return ch;
}

void StageWizard::generateColorPalette(const FixtureGroupEntry &grp, const QString &prefix)
{
    if (!grp.hasRGB && !grp.hasCMY) return;

    struct ColorDef { QString name; quint8 r, g, b; };
    static const ColorDef colors[] = {
        { "Red",     255,   0,   0 },
        { "Green",     0, 255,   0 },
        { "Blue",      0,   0, 255 },
        { "Cyan",      0, 255, 255 },
        { "Magenta", 255,   0, 255 },
        { "Yellow",  255, 255,   0 },
        { "White",   255, 255, 255 },
    };

    QList<Scene *> scenes;
    for (const ColorDef &cd : colors)
    {
        Scene *s = new Scene(m_doc);
        s->setName(tr("%1 – %2").arg(prefix).arg(cd.name));
        for (quint32 fxID : grp.fixtureIDs)
        {
            Fixture *fx = m_doc->fixture(fxID);
            if (!fx) continue;
            for (quint32 ch = 0; ch < fx->channels(); ++ch)
            {
                const QLCChannel *c = fx->channel(ch);
                if (!c || c->group() != QLCChannel::Intensity) continue;
                quint8 val = 0;
                switch (c->colour())
                {
                    case QLCChannel::Red:     val = cd.r; break;
                    case QLCChannel::Green:   val = cd.g; break;
                    case QLCChannel::Blue:    val = cd.b; break;
                    case QLCChannel::Cyan:    val = 255 - cd.r; break;
                    case QLCChannel::Magenta: val = 255 - cd.g; break;
                    case QLCChannel::Yellow:  val = 255 - cd.b; break;
                    case QLCChannel::White:   val = (cd.r == 255 && cd.g == 255 && cd.b == 255) ? 255 : 0; break;
                    default: break;
                }
                s->setValue(SceneValue(fxID, ch, val));
            }
        }
        scenes.append(s);
    }

    Chaser *ch = makeChaserFromScenes(scenes,
                                      tr("%1 – Color Palette").arg(prefix),
                                      500, 2000);
    m_doc->addFunction(ch);
    m_generatedFunctionIDs.append(ch->id());
}

void StageWizard::generateGoboPalette(const FixtureGroupEntry &grp, const QString &prefix)
{
    // Collect gobo capabilities from the first fixture in the group
    Fixture *sample = nullptr;
    for (quint32 id : grp.fixtureIDs)
    {
        sample = m_doc->fixture(id);
        if (sample) break;
    }
    if (!sample) return;

    QList<Scene *> scenes;
    for (quint32 ch = 0; ch < sample->channels(); ++ch)
    {
        const QLCChannel *c = sample->channel(ch);
        if (!c || c->group() != QLCChannel::Gobo) continue;
        for (const QLCCapability *cap : c->capabilities())
        {
            Scene *s = new Scene(m_doc);
            s->setName(tr("%1 – Gobo %2").arg(prefix).arg(cap->name()));
            for (quint32 fxID : grp.fixtureIDs)
            {
                quint8 midVal = (cap->min() + cap->max()) / 2;
                s->setValue(SceneValue(fxID, ch, midVal));
            }
            scenes.append(s);
            if (scenes.count() >= 16) break; // cap at 16 gobos
        }
        break; // first gobo channel only
    }

    if (scenes.isEmpty()) return;

    Chaser *ch = makeChaserFromScenes(scenes,
                                      tr("%1 – Gobo Palette").arg(prefix),
                                      0, 1000);
    m_doc->addFunction(ch);
    m_generatedFunctionIDs.append(ch->id());
}

void StageWizard::generateShutterEffects(const FixtureGroupEntry &grp, const QString &prefix)
{
    Fixture *sample = nullptr;
    for (quint32 id : grp.fixtureIDs)
    {
        sample = m_doc->fixture(id);
        if (sample) break;
    }
    if (!sample) return;

    QList<Scene *> scenes;
    for (quint32 ch = 0; ch < sample->channels(); ++ch)
    {
        const QLCChannel *c = sample->channel(ch);
        if (!c || c->group() != QLCChannel::Shutter) continue;
        for (const QLCCapability *cap : c->capabilities())
        {
            if (cap->name().contains("open", Qt::CaseInsensitive) ||
                cap->name().contains("closed", Qt::CaseInsensitive))
                continue;
            Scene *s = new Scene(m_doc);
            s->setName(tr("%1 – %2").arg(prefix).arg(cap->name()));
            for (quint32 fxID : grp.fixtureIDs)
            {
                quint8 midVal = (cap->min() + cap->max()) / 2;
                s->setValue(SceneValue(fxID, ch, midVal));
            }
            scenes.append(s);
            if (scenes.count() >= 8) break;
        }
        break;
    }

    if (scenes.isEmpty()) return;

    Chaser *ch = makeChaserFromScenes(scenes,
                                      tr("%1 – Shutter Effects").arg(prefix),
                                      0, 500);
    m_doc->addFunction(ch);
    m_generatedFunctionIDs.append(ch->id());
}

void StageWizard::generatePositionPresets(const FixtureGroupEntry &grp, const QString &prefix)
{
    if (!grp.hasMovement) return;

    // Standard theatrical position presets: Centre, Left, Right, Up, Down
    struct PosDef { QString name; quint8 panVal; quint8 tiltVal; };
    static const PosDef positions[] = {
        { "Centre",       128, 90  },
        { "Stage Left",   64,  90  },
        { "Stage Right",  192, 90  },
        { "Up",           128, 30  },
        { "Down / Floor", 128, 150 },
    };

    for (const PosDef &pd : positions)
    {
        Scene *s = new Scene(m_doc);
        s->setName(tr("%1 – %2").arg(prefix).arg(pd.name));
        for (quint32 fxID : grp.fixtureIDs)
        {
            Fixture *fx = m_doc->fixture(fxID);
            if (!fx) continue;
            for (quint32 ch = 0; ch < fx->channels(); ++ch)
            {
                const QLCChannel *c = fx->channel(ch);
                if (!c) continue;
                if (c->group() == QLCChannel::Pan)
                    s->setValue(SceneValue(fxID, ch, pd.panVal));
                else if (c->group() == QLCChannel::Tilt)
                    s->setValue(SceneValue(fxID, ch, pd.tiltVal));
            }
        }
        m_doc->addFunction(s);
        m_generatedFunctionIDs.append(s->id());
    }
}

void StageWizard::generateEFX(const FixtureGroupEntry &grp, const QString &prefix,
                               int algorithm, const QString &label)
{
    if (!grp.hasMovement) return;

    EFX *efx = new EFX(m_doc);
    efx->setName(tr("%1 – %2").arg(prefix).arg(label));
    efx->setAlgorithm(static_cast<EFX::Algorithm>(algorithm));
    efx->setWidth(127);
    efx->setHeight(127);
    efx->setXOffset(127);
    efx->setYOffset(127);

    if (label == tr("Fly Out") || label == tr("Fly In"))
    {
        efx->setPropagationMode(EFX::Serial);
        // Fly Out: fixtures fan outward → use Line2 with spread
        efx->setWidth(200);
        efx->setHeight(100);
    }
    else if (label == tr("Circle Chase"))
    {
        efx->setPropagationMode(EFX::Serial);
    }
    else if (label == tr("Audience Sweep"))
    {
        efx->setHeight(20);  // keep tilt fixed low
        efx->setPropagationMode(EFX::Parallel);
    }

    for (quint32 fxID : grp.fixtureIDs)
    {
        Fixture *fx = m_doc->fixture(fxID);
        if (!fx) continue;
        int heads = qMax(1, fx->heads());
        for (int h = 0; h < heads; ++h)
            efx->addFixture(fxID, h);
    }

    m_doc->addFunction(efx);
    m_generatedFunctionIDs.append(efx->id());
}

void StageWizard::generateRGBMatrix(const FixtureGroupEntry &grp,
                                    const QString &scriptName,
                                    const QString &label)
{
    if (grp.fixtureIDs.isEmpty()) return;

    // Create a FixtureGroup for the matrix
    FixtureGroup *group = new FixtureGroup(m_doc);
    group->setName(tr("%1 – %2 Grid").arg(grp.name).arg(label));
    int cols = qMax(1, static_cast<int>(qCeil(qSqrt(grp.fixtureIDs.count()))));
    int rows = (grp.fixtureIDs.count() + cols - 1) / cols;
    group->setSize(QSize(cols, rows));
    for (int i = 0; i < grp.fixtureIDs.count(); ++i)
        group->assignFixture(grp.fixtureIDs[i], QLCPoint(i % cols, i / cols));

    m_doc->addFixtureGroup(group);
    m_generatedGroupIDs.append(group->id());

    RGBMatrix *matrix = new RGBMatrix(m_doc);
    matrix->setName(tr("%1 – %2").arg(grp.name).arg(label));
    matrix->setFixtureGroup(group->id());

    RGBAlgorithm *algo = RGBAlgorithm::algorithm(m_doc, scriptName);
    if (!algo)
        algo = RGBAlgorithm::algorithm(m_doc, "Stripes"); // fallback
    matrix->setAlgorithm(algo);

    m_doc->addFunction(matrix);
    m_generatedFunctionIDs.append(matrix->id());
}

Chaser *StageWizard::generateStrobeChase(const FixtureGroupEntry &grp, const QString &prefix)
{
    // Two scenes: full open, full closed shutter – alternating fast
    Scene *open   = new Scene(m_doc);
    Scene *closed = new Scene(m_doc);
    open->setName(tr("%1 – Strobe Open").arg(prefix));
    closed->setName(tr("%1 – Strobe Closed").arg(prefix));

    for (quint32 fxID : grp.fixtureIDs)
    {
        Fixture *fx = m_doc->fixture(fxID);
        if (!fx) continue;
        for (quint32 ch = 0; ch < fx->channels(); ++ch)
        {
            const QLCChannel *c = fx->channel(ch);
            if (!c || c->group() != QLCChannel::Shutter) continue;
            // Find open/closed capability values
            quint8 openVal = 255, closedVal = 0;
            for (const QLCCapability *cap : c->capabilities())
            {
                if (cap->name().contains("open", Qt::CaseInsensitive))
                    openVal = (cap->min() + cap->max()) / 2;
                if (cap->name().contains("closed", Qt::CaseInsensitive))
                    closedVal = (cap->min() + cap->max()) / 2;
            }
            open->setValue(SceneValue(fxID, ch, openVal));
            closed->setValue(SceneValue(fxID, ch, closedVal));
        }
    }

    return makeChaserFromScenes({ open, closed },
                                tr("%1 – Strobe Chase").arg(prefix),
                                0, 80);
}

Chaser *StageWizard::generateHeartbeat(const FixtureGroupEntry &grp, const QString &prefix)
{
    Scene *full = makeFullScene(grp, tr("%1 – Full").arg(prefix));
    Scene *dim  = new Scene(m_doc);
    dim->setName(tr("%1 – Dim").arg(prefix));
    for (quint32 fxID : grp.fixtureIDs)
    {
        Fixture *fx = m_doc->fixture(fxID);
        if (!fx) continue;
        for (quint32 ch = 0; ch < fx->channels(); ++ch)
        {
            const QLCChannel *c = fx->channel(ch);
            if (c && c->group() == QLCChannel::Intensity)
                dim->setValue(SceneValue(fxID, ch, 30));
        }
    }

    m_doc->addFunction(full);
    m_doc->addFunction(dim);
    m_generatedFunctionIDs.append(full->id());
    m_generatedFunctionIDs.append(dim->id());

    Chaser *ch = new Chaser(m_doc);
    ch->setName(tr("%1 – Heartbeat").arg(prefix));
    ch->setFadeInMode(Chaser::PerStep);
    ch->setFadeOutMode(Chaser::PerStep);
    ch->setDurationMode(Chaser::PerStep);

    ChaserStep s1(full->id()); s1.fadeIn = 80;  s1.hold = 100; s1.fadeOut = 0;
    ChaserStep s2(dim->id());  s2.fadeIn = 0;   s2.hold = 400; s2.fadeOut = 80;
    ch->addStep(s1);
    ch->addStep(s2);

    m_doc->addFunction(ch);
    return ch;
}

Chaser *StageWizard::generateColorRainbow(const FixtureGroupEntry &grp, const QString &prefix)
{
    struct ColorDef { QString name; quint8 r, g, b; };
    static const ColorDef colors[] = {
        { "Red",     255,   0,   0 },
        { "Orange",  255, 128,   0 },
        { "Yellow",  255, 255,   0 },
        { "Green",     0, 255,   0 },
        { "Cyan",      0, 255, 255 },
        { "Blue",      0,   0, 255 },
        { "Magenta", 255,   0, 255 },
    };

    QList<Scene *> scenes;
    for (const ColorDef &cd : colors)
    {
        Scene *s = new Scene(m_doc);
        s->setName(tr("%1 – Rainbow %2").arg(prefix).arg(cd.name));
        for (quint32 fxID : grp.fixtureIDs)
        {
            Fixture *fx = m_doc->fixture(fxID);
            if (!fx) continue;
            for (quint32 ch = 0; ch < fx->channels(); ++ch)
            {
                const QLCChannel *c = fx->channel(ch);
                if (!c || c->group() != QLCChannel::Intensity) continue;
                quint8 val = 0;
                switch (c->colour())
                {
                    case QLCChannel::Red:     val = cd.r; break;
                    case QLCChannel::Green:   val = cd.g; break;
                    case QLCChannel::Blue:    val = cd.b; break;
                    case QLCChannel::Cyan:    val = 255 - cd.r; break;
                    case QLCChannel::Magenta: val = 255 - cd.g; break;
                    case QLCChannel::Yellow:  val = 255 - cd.b; break;
                    default: break;
                }
                s->setValue(SceneValue(fxID, ch, val));
            }
        }
        scenes.append(s);
    }

    return makeChaserFromScenes(scenes,
                                tr("%1 – Color Rainbow").arg(prefix),
                                500, 1000);
}

Chaser *StageWizard::generateSplitColor(const FixtureGroupEntry &grp, const QString &prefix)
{
    // Even fixtures: red, Odd fixtures: blue – then swap
    auto makeScene = [&](const QString &name, bool swap) -> Scene *
    {
        Scene *s = new Scene(m_doc);
        s->setName(name);
        for (int i = 0; i < grp.fixtureIDs.count(); ++i)
        {
            quint32 fxID = grp.fixtureIDs[i];
            Fixture *fx  = m_doc->fixture(fxID);
            if (!fx) continue;
            bool isOdd = (i % 2 != 0) ^ swap;
            for (quint32 ch = 0; ch < fx->channels(); ++ch)
            {
                const QLCChannel *c = fx->channel(ch);
                if (!c || c->group() != QLCChannel::Intensity) continue;
                quint8 val = 0;
                if (!isOdd)
                {
                    if (c->colour() == QLCChannel::Red)   val = 255;
                    if (c->colour() == QLCChannel::Cyan)  val = 0;
                }
                else
                {
                    if (c->colour() == QLCChannel::Blue)    val = 255;
                    if (c->colour() == QLCChannel::Yellow)  val = 0;
                }
                s->setValue(SceneValue(fxID, ch, val));
            }
        }
        return s;
    };

    Scene *s1 = makeScene(tr("%1 – Split A").arg(prefix), false);
    Scene *s2 = makeScene(tr("%1 – Split B").arg(prefix), true);

    return makeChaserFromScenes({ s1, s2 },
                                tr("%1 – Split Color").arg(prefix),
                                200, 500);
}

Scene *StageWizard::generateBlinderHit(const FixtureGroupEntry &grp, const QString &prefix)
{
    Scene *s = new Scene(m_doc);
    s->setName(tr("%1 – Blinder Hit").arg(prefix));
    for (quint32 fxID : grp.fixtureIDs)
    {
        Fixture *fx = m_doc->fixture(fxID);
        if (!fx) continue;
        for (quint32 ch = 0; ch < fx->channels(); ++ch)
        {
            const QLCChannel *c = fx->channel(ch);
            if (c && c->group() == QLCChannel::Intensity)
                s->setValue(SceneValue(fxID, ch, 255));
        }
    }
    m_doc->addFunction(s);
    return s;
}

Chaser *StageWizard::generateShowOpen()
{
    Scene *blackout = makeBlackoutScene();
    blackout->setName(tr("Show Open – Blackout"));
    Scene *fadeUp   = nullptr;

    // Fade up key lights
    for (const FixtureGroupEntry &grp : m_groups)
    {
        if (grp.role == RoleKey)
        {
            fadeUp = makeFullScene(grp, tr("Show Open – Key Up"));
            break;
        }
    }

    QList<Scene *> steps;
    if (blackout) steps.append(blackout);
    if (fadeUp)   steps.append(fadeUp);

    if (steps.isEmpty()) return nullptr;

    Chaser *ch = makeChaserFromScenes(steps, tr("Show Open"), 3000, 5000);
    ch->setRunOrder(Chaser::SingleShot);
    m_doc->addFunction(ch);
    return ch;
}

Chaser *StageWizard::generateShowClose()
{
    QList<Scene *> steps;
    for (const FixtureGroupEntry &grp : m_groups)
    {
        if (grp.role == RoleKey || grp.role == RoleFill)
        {
            Scene *s = makeFullScene(grp, tr("Show Close – %1 Full").arg(grp.name));
            steps.append(s);
        }
    }
    Scene *blackout = makeBlackoutScene();
    blackout->setName(tr("Show Close – Blackout"));
    steps.append(blackout);

    if (steps.size() <= 1) return nullptr;

    Chaser *ch = makeChaserFromScenes(steps, tr("Show Close"), 5000, 3000);
    ch->setRunOrder(Chaser::SingleShot);
    m_doc->addFunction(ch);
    return ch;
}

Chaser *StageWizard::generateBigMoment()
{
    QList<Scene *> steps;

    // Step 1: everyone full
    for (const FixtureGroupEntry &grp : m_groups)
    {
        Scene *s = makeFullScene(grp, tr("Big Moment – %1 Full").arg(grp.name));
        steps.append(s);
    }

    // Step 2: blackout
    Scene *bo = makeBlackoutScene();
    bo->setName(tr("Big Moment – Drop"));
    steps.append(bo);

    if (steps.isEmpty()) return nullptr;

    Chaser *ch = makeChaserFromScenes(steps, tr("Big Moment"), 0, 80);
    ch->setRunOrder(Chaser::SingleShot);
    m_doc->addFunction(ch);
    return ch;
}

Chaser *StageWizard::generateAmbientLoop()
{
    QList<Scene *> scenes;
    // Warm white → cool blue → warm white cycle
    struct ColorDef { QString name; quint8 r, g, b; };
    static const ColorDef ambient[] = {
        { "Warm White", 255, 200, 100 },
        { "Cool Blue",   80, 120, 255 },
        { "Soft White", 220, 220, 255 },
    };
    for (const FixtureGroupEntry &grp : m_groups)
    {
        if (!grp.hasRGB && !grp.hasCMY && !grp.hasDimmer) continue;
        for (const ColorDef &cd : ambient)
        {
            Scene *s = new Scene(m_doc);
            s->setName(tr("Ambient – %1 %2").arg(grp.name).arg(cd.name));
            for (quint32 fxID : grp.fixtureIDs)
            {
                Fixture *fx = m_doc->fixture(fxID);
                if (!fx) continue;
                for (quint32 ch = 0; ch < fx->channels(); ++ch)
                {
                    const QLCChannel *c = fx->channel(ch);
                    if (!c || c->group() != QLCChannel::Intensity) continue;
                    quint8 val = 0;
                    switch (c->colour())
                    {
                        case QLCChannel::Red:     val = cd.r; break;
                        case QLCChannel::Green:   val = cd.g; break;
                        case QLCChannel::Blue:    val = cd.b; break;
                        case QLCChannel::White:   val = (cd.r > 200 && cd.g > 200) ? 200 : 0; break;
                        default: val = (cd.r + cd.g + cd.b) / 3; break;
                    }
                    s->setValue(SceneValue(fxID, ch, val));
                }
            }
            scenes.append(s);
        }
        break; // one ambient loop across the first suitable group
    }

    if (scenes.isEmpty()) return nullptr;

    Chaser *ch = makeChaserFromScenes(scenes, tr("Ambient Loop"), 4000, 8000);
    ch->setRunOrder(Chaser::Loop);
    m_doc->addFunction(ch);
    return ch;
}

// ── Virtual Console layout ────────────────────────────────────────────────────

void StageWizard::createVCLayout()
{
    VCPage *page = m_virtualConsole->page(0);
    if (!page) return;

    int pd  = static_cast<int>(m_virtualConsole->pixelDensity());
    if (pd <= 0) pd = 1;

    // ── Row 1: Show-level cue buttons ───────────────────────────────────────
    int x = kFrmPad * pd, y = kFrmPad * pd;
    int btnW = kBtnW * pd, btnH = kBtnH * pd;

    // Blackout button
    auto addButton = [&](const QString &caption, quint32 funcID, int &xpos, int ypos) -> VCButton *
    {
        VCButton *btn = qobject_cast<VCButton *>(
            page->addWidget(nullptr, "Button", QPoint(xpos, ypos)));
        if (btn)
        {
            btn->setCaption(caption);
            if (funcID != Function::invalidId())
                btn->setFunctionID(funcID);
            xpos += btnW + kFrmPad * pd;
        }
        return btn;
    };

    // Find show cue functions by name suffix
    auto findFunc = [&](const QString &suffix) -> quint32
    {
        for (quint32 id : m_generatedFunctionIDs)
        {
            Function *f = m_doc->function(id);
            if (f && f->name().endsWith(suffix))
                return id;
        }
        return Function::invalidId();
    };

    addButton(tr("Blackout"),    findFunc("Blackout"),    x, y);
    addButton(tr("Show Open"),   findFunc("Show Open"),   x, y);
    addButton(tr("Big Moment"),  findFunc("Big Moment"),  x, y);
    addButton(tr("Show Close"),  findFunc("Show Close"),  x, y);
    addButton(tr("Ambient"),     findFunc("Ambient Loop"),x, y);

    y += btnH + kFrmPad * 2 * pd;

    // ── Per-group frames ─────────────────────────────────────────────────────
    int grpX = kFrmPad * pd;
    for (const FixtureGroupEntry &grp : m_groups)
    {
        createGroupFrame(page, grp, grpX, y);
        // Each frame is kListW + kSlW + kPadW wide (approx), advance X
        grpX += (kListW + kSlW + kPadW + kFrmPad * 4) * pd + kFrmPad * pd;
    }
}

void StageWizard::createGroupFrame(void *vcPagePtr,
                                   const FixtureGroupEntry &grp,
                                   int xPos, int yPos)
{
    VCPage *page = static_cast<VCPage *>(vcPagePtr);
    if (!page) return;

    int pd   = static_cast<int>(m_virtualConsole->pixelDensity());
    if (pd <= 0) pd = 1;

    int frmW = (kListW + kSlW + kPadW + kFrmPad * 4) * pd;
    int frmH = (kListH + kBtnH * 4 + kFrmPad * 6) * pd;

    // Outer group frame
    VCFrame *frame = qobject_cast<VCFrame *>(
        page->addWidget(nullptr, "Frame", QPoint(xPos, yPos)));
    if (!frame) return;
    frame->setCaption(grp.name);
    frame->setGeometry(QRectF(xPos, yPos, frmW, frmH));

    int innerX = kFrmPad * pd;
    int innerY = kFrmPad * 2 * pd; // leave room for caption

    // ── Color/Gobo CueList (if applicable) ──────────────────────────────────
    if (grp.hasRGB || grp.hasCMY || grp.hasGobo)
    {
        // Find the color palette chaser for this group
        quint32 colorChaserID = Function::invalidId();
        for (quint32 id : m_generatedFunctionIDs)
        {
            Function *f = m_doc->function(id);
            if (f && f->type() == Function::ChaserType &&
                f->name().contains(grp.name) &&
                (f->name().contains("Color Palette") || f->name().contains("Gobo Palette")))
            {
                colorChaserID = id;
                break;
            }
        }

        VCCueList *list = qobject_cast<VCCueList *>(
            frame->addWidget(nullptr, "CueList", QPoint(innerX, innerY)));
        if (list)
        {
            list->setCaption(tr("Colors"));
            list->setGeometry(QRectF(innerX, innerY, kListW * pd, kListH * pd));
            if (colorChaserID != Function::invalidId())
                list->setChaserID(colorChaserID);
        }
        innerX += (kListW + kFrmPad) * pd;
    }

    // ── Intensity Slider ──────────────────────────────────────────────────────
    if (grp.hasDimmer || grp.hasRGB || grp.hasCMY)
    {
        VCSlider *slider = qobject_cast<VCSlider *>(
            frame->addWidget(nullptr, "Slider", QPoint(innerX, innerY)));
        if (slider)
        {
            slider->setCaption(tr("Intensity"));
            slider->setGeometry(QRectF(innerX, innerY, kSlW * pd, kListH * pd));
            slider->setSliderMode(VCSlider::Level);
            for (quint32 fxID : grp.fixtureIDs)
            {
                Fixture *fx = m_doc->fixture(fxID);
                if (!fx) continue;
                for (quint32 ch = 0; ch < fx->channels(); ++ch)
                {
                    const QLCChannel *c = fx->channel(ch);
                    if (c && c->group() == QLCChannel::Intensity &&
                        c->colour() == QLCChannel::NoColour)
                        slider->addLevelChannel(fxID, ch);
                }
            }
            innerX += (kSlW + kFrmPad) * pd;
        }
    }

    // ── XY Pad ───────────────────────────────────────────────────────────────
    if (grp.hasMovement)
    {
        VCXYPad *pad = qobject_cast<VCXYPad *>(
            frame->addWidget(nullptr, "XYPad", QPoint(innerX, innerY)));
        if (pad)
        {
            pad->setCaption(tr("Position"));
            pad->setGeometry(QRectF(innerX, innerY, kPadW * pd, kPadH * pd));
            for (quint32 fxID : grp.fixtureIDs)
            {
                Fixture *fx = m_doc->fixture(fxID);
                if (!fx) continue;
                // addFixtureGroupHeadPreset expects (fixtureID, headIndex)
                for (int h = 0; h < qMax(1, fx->heads()); ++h)
                    pad->addFixtureGroupHeadPreset(static_cast<int>(fxID), h);
            }
        }
        innerX += (kPadW + kFrmPad) * pd;
    }

    int btnY = innerY + kListH * pd + kFrmPad * pd;
    int btnX = kFrmPad * pd;
    int btnW = kBtnW * pd;
    int btnH = kBtnH * pd;

    // ── EFX / Movement quick buttons ─────────────────────────────────────────
    if (grp.hasMovement)
    {
        QStringList movementSuffixes = {
            tr("Fly Out"), tr("Fly In"),
            tr("Circle Chase"), tr("Figure Eight"), tr("Audience Sweep")
        };
        for (const QString &suffix : movementSuffixes)
        {
            for (quint32 id : m_generatedFunctionIDs)
            {
                Function *f = m_doc->function(id);
                if (f && f->name().contains(grp.name) && f->name().endsWith(suffix))
                {
                    VCButton *btn = qobject_cast<VCButton *>(
                        frame->addWidget(nullptr, "Button", QPoint(btnX, btnY)));
                    if (btn)
                    {
                        btn->setCaption(suffix);
                        btn->setGeometry(QRectF(btnX, btnY, btnW, btnH));
                        btn->setFunctionID(id);
                        btnX += btnW + kFrmPad * pd;
                    }
                    break;
                }
            }
        }
        btnY += btnH + kFrmPad * pd;
        btnX  = kFrmPad * pd;
    }

    // ── Strobe / Intensity effect buttons ────────────────────────────────────
    QStringList intensitySuffixes = {
        tr("Strobe Chase"), tr("Heartbeat"), tr("Blinder Hit")
    };
    for (const QString &suffix : intensitySuffixes)
    {
        for (quint32 id : m_generatedFunctionIDs)
        {
            Function *f = m_doc->function(id);
            if (f && f->name().contains(grp.name) && f->name().endsWith(suffix))
            {
                VCButton *btn = qobject_cast<VCButton *>(
                    frame->addWidget(nullptr, "Button", QPoint(btnX, btnY)));
                if (btn)
                {
                    btn->setCaption(suffix);
                    btn->setGeometry(QRectF(btnX, btnY, btnW, btnH));
                    btn->setFunctionID(id);
                    btnX += btnW + kFrmPad * pd;
                }
                break;
            }
        }
    }
}
