/*
  Q Light Controller Plus
  stagewizard_functions.cpp

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

#include <QtMath>
#include <QSet>
#include <QDebug>

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

// ── Palette-based generation ────────────────────────────────────────────────

quint32 StageWizard::findPalette(int type, const QVariantList &values) const
{
    for (QLCPalette *p : m_doc->palettes())
    {
        if (p == nullptr || int(p->type()) != type)
            continue;
        if (p->values() == values)
            return p->id();
    }
    return QLCPalette::invalidId();
}

quint32 StageWizard::makeColorPalette(const QString &name, const QColor &color)
{
    QLCPalette *p = new QLCPalette(QLCPalette::Color);
    p->setValue(QLCPalette::colorToString(color, QColor()));

    // Colour palettes are value-based (group independent): reuse if one exists.
    quint32 existing = findPalette(QLCPalette::Color, p->values());
    if (existing != QLCPalette::invalidId()) { delete p; return existing; }

    p->setName(name);
    if (!m_doc->addPalette(p)) { delete p; return QLCPalette::invalidId(); }
    return p->id();
}

quint32 StageWizard::makeDimmerPalette(const QString &name, int value)
{
    QLCPalette *p = new QLCPalette(QLCPalette::Dimmer);
    p->setValue(value);

    quint32 existing = findPalette(QLCPalette::Dimmer, p->values());
    if (existing != QLCPalette::invalidId()) { delete p; return existing; }

    p->setName(name);
    if (!m_doc->addPalette(p)) { delete p; return QLCPalette::invalidId(); }
    return p->id();
}

quint32 StageWizard::makeShutterPalette(const QString &name, int preset, int percent)
{
    QLCPalette *p = new QLCPalette(QLCPalette::Shutter);
    p->setValue(preset, percent); // (QLCCapability::Preset, 0-100)

    quint32 existing = findPalette(QLCPalette::Shutter, p->values());
    if (existing != QLCPalette::invalidId()) { delete p; return existing; }

    p->setName(name);
    if (!m_doc->addPalette(p)) { delete p; return QLCPalette::invalidId(); }
    return p->id();
}

quint32 StageWizard::makePosition3DPalette(const QString &name, const QVector3D &targetMM)
{
    QLCPalette *p = new QLCPalette(QLCPalette::Position3D);
    // Position3D palette targets are expressed in metres (monitor/grid frame).
    p->setValue(targetMM.x() / 1000.0f, targetMM.y() / 1000.0f, targetMM.z() / 1000.0f);

    quint32 existing = findPalette(QLCPalette::Position3D, p->values());
    if (existing != QLCPalette::invalidId()) { delete p; return existing; }

    p->setName(name);
    if (!m_doc->addPalette(p)) { delete p; return QLCPalette::invalidId(); }
    return p->id();
}

Scene *StageWizard::makePaletteScene(const QString &name, quint32 groupID,
                                     quint32 paletteID, const QString &path)
{
    if (paletteID == QLCPalette::invalidId())
        return nullptr;

    Scene *s = new Scene(m_doc);
    s->setName(name);
    if (groupID != FixtureGroup::invalidId())
        s->addFixtureGroup(groupID);
    s->addPalette(paletteID);
    // Set the path BEFORE addFunction so FunctionManager::slotFunctionAdded
    // places the tree item in the right folder immediately.
    if (!path.isEmpty())
        s->setPath(path);
    m_doc->addFunction(s);
    m_generatedFunctionIDs.append(s->id());
    return s;
}

QString StageWizard::wizardPath(const QString &sub) const
{
    QString base = tr("Show Wizard");
    return sub.isEmpty() ? base : base + "/" + sub;
}

void StageWizard::generateGroupPalettes(const FixtureGroupEntry &grp)
{
    if (grp.groupId == FixtureGroup::invalidId())
        return;

    const QString prefix = grp.name;

    // ── RGB/CMY colour palettes + scenes ────────────────────────────────────
    if (grp.hasRGB || grp.hasCMY)
    {
        const QString path = wizardPath(prefix + "/" + tr("Colors"));
        struct ColorDef { const char *name; QColor color; };
        const ColorDef colors[] = {
            { QT_TR_NOOP("Red"),     QColor(255,   0,   0) },
            { QT_TR_NOOP("Green"),   QColor(  0, 255,   0) },
            { QT_TR_NOOP("Blue"),    QColor(  0,   0, 255) },
            { QT_TR_NOOP("Cyan"),    QColor(  0, 255, 255) },
            { QT_TR_NOOP("Magenta"), QColor(255,   0, 255) },
            { QT_TR_NOOP("Yellow"),  QColor(255, 255,   0) },
            { QT_TR_NOOP("White"),   QColor(255, 255, 255) },
        };
        for (const ColorDef &cd : colors)
        {
            // Palette name is generic (shared across groups); scene name is group-specific.
            quint32 pid = makeColorPalette(tr(cd.name), cd.color);
            makePaletteScene(tr("%1 – %2").arg(prefix).arg(tr(cd.name)), grp.groupId, pid, path);
        }
    }

    // ── Colour-wheel scenes (fixtures with a colour wheel, e.g. spots) ───────
    // Palettes can't drive a colour wheel, so build scenes with raw wheel values
    // taken from the first fixture's capabilities.
    if (grp.hasColorWheel)
    {
        const QString path = wizardPath(prefix + "/" + tr("Colors"));
        Fixture *sample = nullptr;
        for (quint32 id : grp.fixtureIDs)
        {
            sample = m_doc->fixture(id);
            if (sample) break;
        }
        if (sample)
        {
            for (quint32 ch = 0; ch < sample->channels(); ++ch)
            {
                const QLCChannel *c = sample->channel(ch);
                if (!c || c->group() != QLCChannel::Colour) continue;
                for (const QLCCapability *cap : c->capabilities())
                {
                    // Only single solid colours (ColorMacro). Skip split colours
                    // (ColorDoubleMacro), CTO/CTB correction, and rotation ranges.
                    if (cap->preset() != QLCCapability::ColorMacro)
                        continue;

                    Scene *s = new Scene(m_doc);
                    s->setName(tr("%1 – %2").arg(prefix).arg(cap->name()));
                    s->addFixtureGroup(grp.groupId);
                    uchar val = uchar((cap->min() + cap->max()) / 2);
                    for (quint32 fxID : grp.fixtureIDs)
                        s->setValue(SceneValue(fxID, ch, val));
                    s->setPath(path);
                    m_doc->addFunction(s);
                    m_generatedFunctionIDs.append(s->id());
                }
                break; // only the first colour-wheel channel
            }
        }
    }

    // ── Dimmer palettes + scenes ────────────────────────────────────────────
    if (grp.hasDimmer || grp.hasRGB || grp.hasCMY)
    {
        const QString path = wizardPath(prefix + "/" + tr("Dimmer"));
        struct DimDef { const char *name; int value; };
        const DimDef dims[] = {
            { QT_TR_NOOP("Full"), 255 },
            { QT_TR_NOOP("Half"), 128 },
            { QT_TR_NOOP("Off"),    0 },
        };
        for (const DimDef &dd : dims)
        {
            // Generic palette name (shared); group-specific scene name.
            quint32 pid = makeDimmerPalette(tr("Dimmer %1").arg(tr(dd.name)), dd.value);
            makePaletteScene(tr("%1 – Dimmer %2").arg(prefix).arg(tr(dd.name)), grp.groupId, pid, path);
        }
    }

    // ── Shutter palettes + scenes ───────────────────────────────────────────
    if (grp.hasShutter)
    {
        const QString path = wizardPath(prefix + "/" + tr("Shutter"));
        quint32 openPid = makeShutterPalette(tr("Shutter Open"),
                                             QLCCapability::ShutterOpen);
        makePaletteScene(tr("%1 – Shutter Open").arg(prefix), grp.groupId, openPid, path);

        quint32 closePid = makeShutterPalette(tr("Shutter Closed"),
                                              QLCCapability::ShutterClose);
        makePaletteScene(tr("%1 – Shutter Closed").arg(prefix), grp.groupId, closePid, path);
    }
}

int StageWizard::movingHeadCount(const FixtureGroupEntry &grp) const
{
    int n = 0;
    for (quint32 fxID : grp.fixtureIDs)
    {
        Fixture *fx = m_doc->fixture(fxID);
        if (fx && fx->channelNumber(QLCChannel::Pan, QLCChannel::MSB) != QLCChannel::invalid())
            n += qMax(1, fx->heads());
    }
    return n;
}

// Moving-head fixture IDs of a group, paired with their placed X (mm).
QList<QPair<quint32, float>> StageWizard::movingHeadsByX(const FixtureGroupEntry &grp) const
{
    MonitorProperties *props = m_doc->monitorProperties();
    QList<QPair<quint32, float>> heads;
    for (quint32 fxID : grp.fixtureIDs)
    {
        Fixture *fx = m_doc->fixture(fxID);
        if (!fx || fx->channelNumber(QLCChannel::Pan, QLCChannel::MSB) == QLCChannel::invalid())
            continue;
        heads.append(qMakePair(fxID, props->fixturePosition(fxID, 0, 0).x()));
    }
    return heads;
}

void StageWizard::generateMusicianKeyScenes(const FixtureGroupEntry &grp)
{
    if (grp.groupId == FixtureGroup::invalidId())
        return;
    if (grp.role != RoleKey || !grp.hasMovement)
        return;

    // Front (key/fill) moving heads come from this Key group; back (rim/depth)
    // heads come from Back-role groups.
    QList<QPair<quint32, float>> frontHeads = movingHeadsByX(grp);
    QList<QPair<quint32, float>> backHeads;
    for (const FixtureGroupEntry &g : m_groups)
        if (g.selected && g.role == RoleBack)
            backHeads.append(movingHeadsByX(g));

    // Each musician needs 2 heads from the front (key + fill). The number of
    // spots is bounded by the available front heads, capped at 6 (a band).
    int spotCount = qMin(frontHeads.count() / 2, 6);
    if (spotCount < 1)
        return;

    // Lay musicians out in rows of at most 3 (so max 2 rows / 6 total).
    const int perRow  = 3;
    const int rowCount = (spotCount + perRow - 1) / perRow;   // 1 or 2

    const float gx = m_envSize.x() * 1000.0f;
    const float gz = m_envSize.z() * 1000.0f;
    // Musician rows along the depth. Z: 0 = rear, gz = front (audience), so the
    // front row is the LARGER Z. Front row at 2/3, back row at 1/3.
    // (e.g. 9 m stage -> front 6 m, back 3 m)
    const float rowZ[2] = { gz * (2.0f / 3.0f), gz * (1.0f / 3.0f) };

    // Consume the nearest still-unused heads (by X) to a target X.
    auto takeNearest = [](QList<QPair<quint32, float>> &pool, float x, int n) -> QList<quint32>
    {
        QList<quint32> picked;
        for (int k = 0; k < n && !pool.isEmpty(); k++)
        {
            int best = 0;
            float bestDist = qAbs(pool[0].second - x);
            for (int i = 1; i < pool.count(); i++)
            {
                float dd = qAbs(pool[i].second - x);
                if (dd < bestDist) { bestDist = dd; best = i; }
            }
            picked.append(pool[best].first);
            pool.removeAt(best);
        }
        return picked;
    };

    const QString path = wizardPath(grp.name + "/" + tr("Positions"));

    for (int m = 0; m < spotCount; m++)
    {
        int row = m / perRow;
        int col = m % perRow;
        // Number of spots actually on this row (last row may be shorter).
        int spotsInRow = qMin(perRow, spotCount - row * perRow);

        // Spread this row evenly across the usable stage width, centred.
        float x = (spotsInRow <= 1) ? gx * 0.5f
                                    : gx * (0.20f + 0.60f * (col / float(spotsInRow - 1)));
        float z = rowZ[qMin(row, rowCount - 1)];
        QVector3D target(x, 0.0f, z);

        // 2 front heads + up to 2 back heads aimed at this spot.
        QList<quint32> heads = takeNearest(frontHeads, x, 2);
        heads.append(takeNearest(backHeads, x, 2));
        if (heads.isEmpty())
            break;

        QString name = tr("Musician %1").arg(m + 1);

        // Reference the Position3D palette and add ONLY the chosen heads as the
        // scene's fixtures. At playback the engine resolves the palette against
        // the scene's fixtures (Scene::write), so exactly those heads aim at the
        // spot — the rest of the rig is untouched.
        quint32 pid = makePosition3DPalette(tr("%1 – %2").arg(grp.name).arg(name), target);
        if (pid == QLCPalette::invalidId())
            continue;

        Scene *s = new Scene(m_doc);
        s->setName(tr("%1 – %2").arg(grp.name).arg(name));
        for (quint32 headID : heads)
            s->addFixture(headID);
        s->addPalette(pid);
        s->setPath(path);
        m_doc->addFunction(s);
        m_generatedFunctionIDs.append(s->id());
    }
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

quint32 StageWizard::baseMovementPosition(const FixtureGroupEntry &grp, MovementAnchor anchor)
{
    // Reuse a base position scene already made for this group + anchor.
    quint64 key = (quint64(grp.groupId) << 8) | quint64(anchor);
    if (m_basePositionScenes.contains(key))
        return m_basePositionScenes.value(key);

    const float gx = m_envSize.x() * 1000.0f;
    const float gz = m_envSize.z() * 1000.0f;
    const float gy = m_envSize.y() * 1000.0f;

    QVector3D target;
    QString aname;
    switch (anchor)
    {
        case AnchorAerial:   // up into the air, centre
            target = QVector3D(gx * 0.5f, gy * 0.9f, gz * 0.5f);
            aname = tr("Aerial");
            break;
        case AnchorAudience: // out over the audience (front edge, low on the floor beyond)
            target = QVector3D(gx * 0.5f, 0.0f, gz * 1.2f);
            aname = tr("Audience");
            break;
        case AnchorCentre:   // stage floor centre
        default:
            target = QVector3D(gx * 0.5f, 0.0f, gz * 0.5f);
            aname = tr("Centre");
            break;
    }

    quint32 pid = makePosition3DPalette(tr("%1 – %2").arg(grp.name).arg(aname), target);
    if (pid == QLCPalette::invalidId())
        return Function::invalidId();

    Scene *s = new Scene(m_doc);
    s->setName(tr("%1 – Position %2").arg(grp.name).arg(aname));
    for (quint32 fxID : grp.fixtureIDs)
    {
        Fixture *fx = m_doc->fixture(fxID);
        if (fx && fx->channelNumber(QLCChannel::Pan, QLCChannel::MSB) != QLCChannel::invalid())
            s->addFixture(fxID);
    }
    s->addPalette(pid);
    s->setPath(wizardPath(grp.name + "/" + tr("Positions")));
    m_doc->addFunction(s);
    m_generatedFunctionIDs.append(s->id());
    m_basePositionScenes.insert(key, s->id());
    return s->id();
}

void StageWizard::groupPanTiltMax(const FixtureGroupEntry &grp, float &panDeg, float &tiltDeg) const
{
    panDeg = 540.0f;
    tiltDeg = 270.0f;
    for (quint32 fxID : grp.fixtureIDs)
    {
        Fixture *fx = m_doc->fixture(fxID);
        if (!fx || !fx->fixtureMode())
            continue;
        QLCPhysical phy = fx->fixtureMode()->physical();
        if (phy.focusPanMax() > 0)  panDeg  = phy.focusPanMax();
        if (phy.focusTiltMax() > 0) tiltDeg = phy.focusTiltMax();
        return; // first moving head is representative
    }
}

int StageWizard::efxSizeForDegrees(float degrees, float maxDeg) const
{
    if (maxDeg <= 0.0f)
        maxDeg = 360.0f;
    // EFX swings +/- Width DMX; full DMX (255) spans maxDeg. So a total swing of
    // $degrees needs Width = degrees * 127.5 / maxDeg.
    int w = int(qRound(degrees * 127.5f / maxDeg));
    return qBound(1, w, 127);
}

void StageWizard::generateMovementEffect(const FixtureGroupEntry &grp, const QString &prefix,
                                         int algorithm, const QString &label,
                                         MovementAnchor anchor)
{
    if (!grp.hasMovement) return;

    // Base position scene the effect starts from (so the relative EFX has a
    // defined aim to ride on top of and never teleports).
    quint32 posID = baseMovementPosition(grp, anchor);
    if (posID == Function::invalidId())
        return;

    float panMax, tiltMax;
    groupPanTiltMax(grp, panMax, tiltMax);

    EFX *efx = new EFX(m_doc);
    efx->setName(tr("%1 – %2 (motion)").arg(prefix).arg(label));
    efx->setAlgorithm(static_cast<EFX::Algorithm>(algorithm));
    // Relative: the pattern rides on top of each head's current (base-position)
    // aim, so heads move smoothly around the anchor.
    efx->setIsRelative(true);
    efx->setFadeInSpeed(1000);

    if (label == tr("Audience Sweep"))
    {
        // Wide horizontal sweep across the audience, shallow vertically.
        efx->setWidth(efxSizeForDegrees(60.0f, panMax));
        efx->setHeight(efxSizeForDegrees(8.0f, tiltMax));
        efx->setPropagationMode(EFX::Parallel);
    }
    else
    {
        // Compact aerial figure: ~50 degrees of travel max.
        efx->setWidth(efxSizeForDegrees(50.0f, panMax));
        efx->setHeight(efxSizeForDegrees(50.0f, tiltMax));
        efx->setPropagationMode(EFX::Serial);  // chase-style phase spread
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
    efx->setPath(wizardPath(grp.name + "/" + tr("Effects")));
    m_generatedFunctionIDs.append(efx->id());

    // Collection = base position + relative EFX on top.
    Collection *col = new Collection(m_doc);
    col->setName(tr("%1 – %2").arg(prefix).arg(label));
    col->addFunction(posID);
    col->addFunction(efx->id());
    m_doc->addFunction(col);
    col->setPath(wizardPath(grp.name + "/" + tr("Effects")));
    m_generatedFunctionIDs.append(col->id());
}

void StageWizard::generateEFX(const FixtureGroupEntry &grp, const QString &prefix,
                               int algorithm, const QString &label)
{
    // Fly Out / Fly In: a vertical Line EFX with dimmer control, so beams sweep
    // up/down over half the tilt range and fade in/out as they "fly". Absolute
    // and phase-spread across the fixtures.
    Q_UNUSED(algorithm)
    if (!grp.hasMovement) return;

    float panMax, tiltMax;
    groupPanTiltMax(grp, panMax, tiltMax);
    Q_UNUSED(panMax)

    EFX *efx = new EFX(m_doc);
    efx->setName(tr("%1 – %2").arg(prefix).arg(label));
    efx->setAlgorithm(EFX::Line);
    efx->setWidth(0);                                        // vertical line: no pan swing
    efx->setHeight(efxSizeForDegrees(tiltMax / 2.0f, tiltMax)); // half the tilt range
    efx->setXOffset(127);
    efx->setYOffset(127);
    efx->setPropagationMode(EFX::Parallel);
    efx->setDimmerControlEnabled(true);                     // beams fade as they fly
    efx->setFadeInSpeed(1000);

    for (quint32 fxID : grp.fixtureIDs)
    {
        Fixture *fx = m_doc->fixture(fxID);
        if (!fx) continue;
        int heads = qMax(1, fx->heads());
        for (int h = 0; h < heads; ++h)
            efx->addFixture(fxID, h);
    }

    // Spread the phase evenly across the fixtures (0, 360/N, 2*360/N, …) so they
    // fly in a cascade rather than all together.
    const QList<EFXFixture *> efxFx = efx->fixtures();
    int n = efxFx.count();
    for (int i = 0; i < n; i++)
        efxFx.at(i)->setStartOffset(n > 0 ? (i * 360 / n) : 0);

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
    if (grp.groupId == FixtureGroup::invalidId())
        return nullptr;

    // Full-intensity WHITE hit via (shared) Dimmer Full + Color White palettes.
    quint32 dimPid = makeDimmerPalette(tr("Dimmer %1").arg(tr("Full")), 255);
    if (dimPid == QLCPalette::invalidId())
        return nullptr;
    quint32 whitePid = makeColorPalette(tr("White"), QColor(255, 255, 255));

    Scene *s = new Scene(m_doc);
    s->setName(tr("%1 – Blinder Hit").arg(prefix));
    s->addFixtureGroup(grp.groupId);
    s->addPalette(dimPid);
    if (whitePid != QLCPalette::invalidId())
        s->addPalette(whitePid);
    m_doc->addFunction(s);
    return s;
}

Scene *StageWizard::generateBlinderFlash()
{
    // All selected Blinder-role groups.
    QList<quint32> blinderGroups;
    for (const FixtureGroupEntry &grp : m_groups)
    {
        if (grp.selected && grp.role == RoleBlinder &&
            grp.groupId != FixtureGroup::invalidId() && !grp.fixtureIDs.isEmpty())
            blinderGroups.append(grp.groupId);
    }
    if (blinderGroups.isEmpty())
        return nullptr;

    // Full-intensity WHITE hit via (shared) Dimmer Full + Color White palettes.
    quint32 dimPid = makeDimmerPalette(tr("Dimmer %1").arg(tr("Full")), 255);
    if (dimPid == QLCPalette::invalidId())
        return nullptr;
    quint32 whitePid = makeColorPalette(tr("White"), QColor(255, 255, 255));

    Scene *s = new Scene(m_doc);
    s->setName(tr("Blinder Flash"));
    for (quint32 gid : blinderGroups)
        s->addFixtureGroup(gid);
    s->addPalette(dimPid);
    if (whitePid != QLCPalette::invalidId())
        s->addPalette(whitePid);
    // Instant on, short fade-out so releasing the momentary Flash button leaves
    // a quick decay rather than a hard cut.
    s->setFadeInSpeed(0);
    s->setFadeOutSpeed(300);
    m_doc->addFunction(s);
    s->setPath(wizardPath(tr("Show Cues")));
    m_generatedFunctionIDs.append(s->id());
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
