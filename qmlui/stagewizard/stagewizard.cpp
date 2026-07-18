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
#include "function.h"
#include "scene.h"
#include "qlcpalette.h"
#include "scenevalue.h"
#include "chaser.h"
#include "chaserstep.h"
#include "efx.h"
#include "collection.h"
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
#include "functionmanager.h"
#include "virtualconsole/virtualconsole.h"
#include "virtualconsole/vcpage.h"
#include "virtualconsole/vcframe.h"
#include "virtualconsole/vcsoloframe.h"
#include "virtualconsole/vcbutton.h"
#include "virtualconsole/vcslider.h"
#include "virtualconsole/vccuelist.h"
#include "virtualconsole/vcxypad.h"
#include "contextmanager.h"
#include "mainview3d.h"
#include "fixtureutils.h"
#include "inputoutputmap.h"
#include "outputpatch.h"
#include "inputpatch.h"
#include "qlcinputsource.h"
#include "qlcfixturedef.h"
#include "qlcfixturemode.h"

#include <QtMath>
#include <QSet>
#include <QDebug>

// This class's methods are split across several .cpp files for readability
// (all share stagewizard.h):
//   stagewizard.cpp            – state, navigation, models, generate() flow
//   stagewizard_placement.cpp  – 3D fixture placement
//   stagewizard_functions.cpp  – scene / palette / effect generation
//   stagewizard_vc.cpp         – Virtual Console layout + loopback page switching

// ── Construction ──────────────────────────────────────────────────────────────

StageWizard::StageWizard(Doc *doc,
                         FixtureManager *fixtureManager,
                         FunctionManager *functionManager,
                         VirtualConsole *virtualConsole,
                         ContextManager  *contextManager,
                         QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_fixtureManager(fixtureManager)
    , m_functionManager(functionManager)
    , m_virtualConsole(virtualConsole)
    , m_contextManager(contextManager)
    , m_currentStep(0)
    , m_showType(ClubNight)
    , m_stageType(0) // MonitorProperties::StageSimple
    , m_envSize(m_doc->monitorProperties()->gridSize())
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

    // Skip the Venue & Stage step (index 2) when the wizard won't place any
    // fixtures: nothing new to lay out, so stage/positions are left untouched.
    // Jump over it in whichever direction the user is heading.
    if (step == 2 && !hasNewGroups())
        step = (m_currentStep < 2) ? 3 : 1;

    // On entering step 2: load existing groups (only the first time)
    if (step == 1 && m_groups.isEmpty())
        loadExistingGroups();

    // On entering step 3: set stage default for show type and suggest a size
    if (step == 2)
    {
        if (m_showType == ClubNight)     setStageType(1); // StageBox
        else if (m_showType == Concert)  setStageType(2); // StageRock
        else if (m_showType == Theatrical) setStageType(3); // StageTheatre
        else                             setStageType(0); // StageSimple

        suggestEnvSize();
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
        e.roleUserSet = false;
        e.hasMovement = e.hasRGB = e.hasCMY = e.hasColorWheel = e.hasGobo = e.hasShutter = e.hasDimmer = false;

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
    e.roleUserSet = false;
    e.hasMovement = e.hasRGB = e.hasCMY = e.hasColorWheel = e.hasGobo = e.hasShutter = e.hasDimmer = false;
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
    // Only column 3 (and the box's own checkmark) depend on selection. Emit a
    // dedicated signal instead of groupsModelChanged() so column 2's ListView
    // isn't reassigned a fresh model (which would reset its scroll position).
    emit groupSelectionChanged();
    rebuildRoleModel();
}

bool StageWizard::isGroupSelected(int groupIndex) const
{
    if (groupIndex < 0 || groupIndex >= m_groups.count())
        return false;

    return m_groups.at(groupIndex).selected;
}

bool StageWizard::hasNewGroups() const
{
    for (const FixtureGroupEntry &grp : m_groups)
    {
        if (grp.selected && grp.groupId == FixtureGroup::invalidId())
            return true;
    }
    return false;
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
    e.hasMovement = e.hasRGB = e.hasCMY = e.hasColorWheel = false;
    e.hasGobo = e.hasShutter = e.hasDimmer = false;

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
                case QLCChannel::Colour:
                    if (channel->capabilities().size() > 1) e.hasColorWheel = true;
                    break;
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
        if (e.roleUserSet)                continue;  // keep the user's choice
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
    m_groups[groupIndex].roleUserSet = true;   // stop auto-detect overriding it
    emit fixtureRoleModelChanged();
}

// ── Step 3 ────────────────────────────────────────────────────────────────────

int StageWizard::stageType() const { return m_stageType; }

void StageWizard::setStageType(int type)
{
    if (m_stageType == type)
        return;
    m_stageType = type;

    // The stage type and fixture placement are applied only at generation time
    // (see generate()); the wizard does not modify the live 3D environment while
    // the user is still navigating the steps.
    emit stageTypeChanged(m_stageType);
}

void StageWizard::setEnvWidth(qreal w)
{
    // Whole metres only (trusses use 1 m / 2 m segments)
    float nw = float(qBound(2, int(qRound(w)), 100));
    if (qFuzzyCompare(m_envSize.x(), nw)) return;
    m_envSize.setX(nw);
    emit envSizeChanged();
}

void StageWizard::setEnvHeight(qreal h)
{
    float nh = float(qBound(2, int(qRound(h)), 30));
    if (qFuzzyCompare(m_envSize.y(), nh)) return;
    m_envSize.setY(nh);
    emit envSizeChanged();
}

void StageWizard::setEnvDepth(qreal d)
{
    float nd = float(qBound(2, int(qRound(d)), 100));
    if (qFuzzyCompare(m_envSize.z(), nd)) return;
    m_envSize.setZ(nd);
    emit envSizeChanged();
}

void StageWizard::suggestEnvSize()
{
    // Count fixtures across the selected groups (fall back to whole project).
    int count = 0;
    for (const FixtureGroupEntry &g : m_groups)
        if (g.selected)
            count += g.fixtureIDs.count();
    if (count == 0)
        count = m_doc->fixtures().count();

    // Width scales the most (trusses get longer), depth moderately, height least.
    // Sizes are whole metres: trusses are built from 1 m / 2 m segments and
    // cannot be scaled to fractional lengths.
    float w = float(qBound(6, int(qRound(4.0 + count * 0.5)),  40));
    float d = float(qBound(5, int(qRound(4.0 + count * 0.3)),  30));
    float h = float(qBound(4, int(qRound(4.0 + count * 0.05)), 12));

    m_envSize = QVector3D(w, h, d);
    emit envSizeChanged();
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

void StageWizard::setFamilyEffectsEnabled(const QString &family)
{
    // Toggle: if any available effect in the family is off, turn them all on;
    // otherwise turn them all off. Applied in a single pass so the model is
    // rebuilt once (doing this per-effect in QML rebuilt the model mid-loop and
    // corrupted the iteration — the "All / None" button appeared to do nothing).
    bool anyOff = false;
    for (const EffectEntry &e : m_effects)
        if (e.family == family && e.available && !e.enabled)
        {
            anyOff = true;
            break;
        }

    bool changed = false;
    for (EffectEntry &e : m_effects)
        if (e.family == family && e.available && e.enabled != anyOff)
        {
            e.enabled = anyOff;
            changed = true;
        }

    if (changed)
        emit effectsModelChanged();
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
        if (hasNewGroups())
            m["detail"] = tr("%1 fixture group(s) positioned on stage type %2")
                              .arg(m_groups.count()).arg(m_stageType);
        else
            m["detail"] = tr("Existing layout kept — fixture positions left untouched");
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
    m_envSize     = m_doc->monitorProperties()->gridSize();
    m_isGenerating = false;
    m_groups.clear();
    m_groupCounter = 0;
    m_pendingDropGroup = -1;
    m_droppedFixtureIDs.clear();
    m_generatedFunctionIDs.clear();
    m_generatedGroupIDs.clear();
    m_hasAllGroups = false;
    buildEffectsModel();

    emit currentStepChanged(m_currentStep);
    emit showTypeChanged(m_showType);
    emit stageTypeChanged(m_stageType);
    emit envSizeChanged();
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

        // Pre-existing group: leave its name, membership and grid untouched.
        // We only need its ID so functions/VC can target it.
        if (grp.groupId != FixtureGroup::invalidId() &&
            m_doc->fixtureGroup(grp.groupId) != nullptr)
        {
            m_generatedGroupIDs.append(grp.groupId);
            continue;
        }

        // New group created inside the wizard: build it from scratch.
        FixtureGroup *group = new FixtureGroup(m_doc);
        group->setName(grp.name);

        int cols = qMax(1, static_cast<int>(qCeil(qSqrt(grp.fixtureIDs.count()))));
        int rows = (grp.fixtureIDs.count() + cols - 1) / cols;
        group->setSize(QSize(cols, rows));

        for (int i = 0; i < grp.fixtureIDs.count(); ++i)
            group->assignFixture(grp.fixtureIDs[i], QLCPoint(i % cols, i / cols));

        m_doc->addFixtureGroup(group);
        grp.groupId = group->id();

        m_generatedGroupIDs.append(group->id());
    }

    // Build the synthetic "All Groups" aggregate: a real FixtureGroup holding
    // every selected fixture, plus the union of capability flags. It backs the
    // master frame's page 0 so the user can drive all groups at once.
    m_hasAllGroups = false;
    m_allGroups = FixtureGroupEntry();
    m_allGroups.name        = tr("All Groups");
    m_allGroups.groupId     = FixtureGroup::invalidId();
    m_allGroups.selected    = true;
    // Non-Key role so generateMusicianKeyScenes() (Key + movement only) skips the
    // aggregate — musician choreography stays per-group; the aggregate only
    // provides "control everything at once" colours/dimmers/positions/effects.
    m_allGroups.role        = RoleEffect;
    m_allGroups.roleUserSet = false;

    for (const FixtureGroupEntry &grp : m_groups)
    {
        if (!grp.selected || grp.fixtureIDs.isEmpty())
            continue;
        for (quint32 fxID : grp.fixtureIDs)
            if (!m_allGroups.fixtureIDs.contains(fxID))
                m_allGroups.fixtureIDs.append(fxID);
        m_allGroups.hasMovement   |= grp.hasMovement;
        m_allGroups.hasRGB        |= grp.hasRGB;
        m_allGroups.hasCMY        |= grp.hasCMY;
        m_allGroups.hasColorWheel |= grp.hasColorWheel;
        m_allGroups.hasGobo       |= grp.hasGobo;
        m_allGroups.hasShutter    |= grp.hasShutter;
        m_allGroups.hasDimmer     |= grp.hasDimmer;
    }

    if (!m_allGroups.fixtureIDs.isEmpty())
    {
        FixtureGroup *allGrp = new FixtureGroup(m_doc);
        allGrp->setName(m_allGroups.name);
        int cols = qMax(1, static_cast<int>(qCeil(qSqrt(m_allGroups.fixtureIDs.count()))));
        int rows = (m_allGroups.fixtureIDs.count() + cols - 1) / cols;
        allGrp->setSize(QSize(cols, rows));
        for (int i = 0; i < m_allGroups.fixtureIDs.count(); ++i)
            allGrp->assignFixture(m_allGroups.fixtureIDs[i], QLCPoint(i % cols, i / cols));
        m_doc->addFixtureGroup(allGrp);
        m_allGroups.groupId = allGrp->id();
        m_generatedGroupIDs.append(allGrp->id());
        m_hasAllGroups = true;
    }
}

void StageWizard::generate()
{
    if (m_isGenerating) return;

    m_isGenerating = true;
    emit isGeneratingChanged();

    m_generatedFunctionIDs.clear();
    m_generatedGroupIDs.clear();
    m_basePositionScenes.clear();

    // Apply the chosen stage type and place the fixtures FIRST, while a box's
    // groupId still tells us whether the group pre-existed (loaded from the
    // project) or is about to be created by the wizard. applyStageLayout() skips
    // pre-existing groups; running it before createFixtureGroups() ensures newly
    // built groups are not mistaken for pre-existing ones.
    //
    // When no NEW groups are selected the Venue step was skipped: the user is
    // only generating functions/VC for an existing rig, so leave the stage type,
    // environment size and all fixture positions exactly as they are.
    if (hasNewGroups())
    {
        m_doc->monitorProperties()->setStageType(
            static_cast<MonitorProperties::StageType>(m_stageType));
        // Route the grid size through ContextManager so the 2D/3D views update
        // too (writing MonitorProperties directly would leave the 2D grid stale).
        m_contextManager->setEnvironmentSize(m_envSize);
        applyStageLayout();
    }

    // Persist the user-defined boxes as real FixtureGroups
    createFixtureGroups();

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
        generateGroupFunctions(grp);
    }

    // "All Groups" aggregate: same function set spanning every selected fixture,
    // so page 0 of the master frame drives all groups at once.
    if (m_hasAllGroups)
        generateGroupFunctions(m_allGroups);

    // Show-level cues (run once, not per group)
    const QString cuesPath = wizardPath(tr("Show Cues"));
    for (const EffectEntry &eff : m_effects)
    {
        if (!eff.enabled) continue;
        Function *c = nullptr;
        switch (eff.flag)
        {
            case EffectShowOpen:   c = generateShowOpen();   break;
            case EffectShowClose:  c = generateShowClose();  break;
            case EffectBigMoment:  c = generateBigMoment();  break;
            case EffectAmbientLoop: c = generateAmbientLoop(); break;
            default: break;
        }
        if (c)
        {
            m_generatedFunctionIDs.append(c->id());
            c->setPath(cuesPath);
        }
    }

    // Blackout lives at the wizard root
    if (blackout)
        blackout->setPath(wizardPath());

    // Blinder flash scene (only if there are blinders) for a VC Flash button
    generateBlinderFlash();

    createVCLayout();

    // Rebuild the function tree so all generated functions (and their paths)
    // show up in the QML function list without needing a reload. Palettes
    // refresh via Doc::paletteAdded (see PaletteManager).
    if (m_functionManager)
        m_functionManager->updateFunctionsTree();

    m_isGenerating = false;
    emit isGeneratingChanged();
    emit summaryModelChanged();
    emit generationComplete();
}

void StageWizard::generateGroupFunctions(const FixtureGroupEntry &grp)
{
    const QString prefix = grp.name;

    // Group + palette based building blocks (colour/dimmer/shutter scenes)
    generateGroupPalettes(grp);

    // Front key lights: 6 musician position scenes aimed at the floor
    generateMusicianKeyScenes(grp);

    // Everything the effect generators add below goes under the group's
    // "Effects" folder. Snapshot the id list, then path the new entries.
    int effectsStart = m_generatedFunctionIDs.count();

    for (const EffectEntry &eff : m_effects)
    {
        if (!eff.enabled || !eff.available)
            continue;

        switch (eff.flag)
        {
            // Color / Shutter / Dimmer / Position are now generated as
            // group+palette scenes in generateGroupPalettes() and
            // generateMusicianKeyScenes(), so they are not handled here.
            case EffectGoboPalette:
                if (grp.hasGobo)
                    generateGoboPalette(grp, prefix);
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
                    generateMovementEffect(grp, prefix, EFX::Circle, tr("Circle Chase"), AnchorCentre);
                break;
            case EffectFigureEight:
                if (grp.hasMovement)
                    generateMovementEffect(grp, prefix, EFX::Eight, tr("Figure Eight"), AnchorCentre);
                break;
            case EffectAudienceSweep:
                if (grp.hasMovement)
                    generateMovementEffect(grp, prefix, EFX::Line, tr("Audience Sweep"), AnchorAudience);
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

    // Path all functions the effect generators just created for this group
    const QString effectsPath = wizardPath(prefix + "/" + tr("Effects"));
    for (int i = effectsStart; i < m_generatedFunctionIDs.count(); i++)
    {
        Function *f = m_doc->function(m_generatedFunctionIDs.at(i));
        if (f) f->setPath(effectsPath);
    }
}
