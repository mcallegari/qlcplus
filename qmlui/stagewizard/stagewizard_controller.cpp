/*
  Q Light Controller Plus
  stagewizard_controller.cpp

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
#include "function.h"
#include "inputoutputmap.h"
#include "inputpatch.h"
#include "outputpatch.h"
#include "qlcinputprofile.h"
#include "qlcinputchannel.h"
#include "qlcinputsource.h"
#include "qlcinputfeedback.h"

#include "virtualconsole/vcwidget.h"

#include <QDebug>
#include <QRegularExpression>
#include <climits>
#include <limits>

// Without an input profile we can still map, but we have to guess the layout.
// These match the most common MIDI controller convention: the first eight
// channels are continuous controls, everything above is a note/button.
static const int kNoProfileFaders  = 8;
static const int kNoProfileButtons = 56;

// A grid smaller than this isn't worth treating as one — a handful of pads maps
// just as well in plain creation order, and pretending they form a matrix only
// wastes cells on holes.
static const int kMinGridCells = 12;

// Below this many buttons a controller is a mixer strip, not a launcher: its
// buttons are per-channel Solo/Mute/Rec that carry a meaning of their own, and
// scattering scenes across them reads as noise. Map the faders and leave the
// buttons to the user (KORG nanoKONTROL, Behringer BCF/BCR, …).
static const int kStripButtonCeiling = 40;

// ── Controller enumeration ───────────────────────────────────────────────────

QLCInputProfile *StageWizard::controllerProfile() const
{
    if (m_ctrlUniverse < 0)
        return nullptr;

    InputOutputMap *iomap = m_doc->inputOutputMap();
    if (iomap == nullptr)
        return nullptr;

    InputPatch *ip = iomap->inputPatch(static_cast<quint32>(m_ctrlUniverse));
    return ip ? ip->profile() : nullptr;
}

QVariant StageWizard::controllersModel() const
{
    QVariantList list;

    InputOutputMap *iomap = m_doc->inputOutputMap();
    if (iomap == nullptr)
        return QVariant::fromValue(list);

    // Walk every universe and report the ones that carry an input patch. This
    // is what the user actually connected in the I/O panel — the previous
    // implementation listed ioManager->universeInputSources(), which returns
    // the plugin lines still AVAILABLE for patching, so a controller that was
    // already patched was excluded and the list came up empty.
    for (quint32 u = 0; u < iomap->universesCount(); u++)
    {
        InputPatch *ip = iomap->inputPatch(u);
        if (ip == nullptr || !ip->isPatched())
            continue;

        // The wizard patches Loopback itself for VC page switching; it is not a
        // controller and must never be offered as one.
        if (ip->pluginName() == QStringLiteral("Loopback"))
            continue;

        QLCInputProfile *profile = ip->profile();

        int buttons = 0, faders = 0;
        if (profile != nullptr)
        {
            QMapIterator<quint32, QLCInputChannel *> it(profile->channels());
            while (it.hasNext())
            {
                it.next();
                switch (it.value()->type())
                {
                    case QLCInputChannel::Button:
                        buttons++;
                        break;
                    case QLCInputChannel::Slider:
                    case QLCInputChannel::Knob:
                    case QLCInputChannel::Encoder:
                        faders++;
                        break;
                    default:
                        break;
                }
            }
        }
        else
        {
            buttons = kNoProfileButtons;
            faders  = kNoProfileFaders;
        }

        QVariantMap m;
        m["universe"]      = int(u);
        m["universeName"]  = iomap->getUniverseNameByIndex(u);
        m["plugin"]        = ip->pluginName();
        m["line"]          = int(ip->input());
        m["lineName"]      = ip->inputName();
        m["profile"]       = ip->profileName();
        m["hasProfile"]    = profile != nullptr;
        m["buttons"]       = buttons;
        m["faders"]        = faders;
        m["hasColorTable"] = profile != nullptr && profile->hasColorTable();
        // A feedback patch is an output patch on the same universe flagged as
        // feedback; when present the controller can light its LEDs.
        m["feedback"]      = iomap->feedbackPatch(u) != nullptr;

        list.append(m);
    }

    return QVariant::fromValue(list);
}

void StageWizard::refreshControllers()
{
    InputOutputMap *iomap = m_doc->inputOutputMap();

    // Drop a stale selection (e.g. the user unpatched it in the I/O panel) and
    // auto-select when there is exactly one candidate, so the common
    // "one controller connected" case needs no click at all.
    bool selectionValid = false;
    int  firstUniverse  = -1;
    int  candidates     = 0;

    if (iomap != nullptr)
    {
        for (quint32 u = 0; u < iomap->universesCount(); u++)
        {
            InputPatch *ip = iomap->inputPatch(u);
            if (ip == nullptr || !ip->isPatched() ||
                ip->pluginName() == QStringLiteral("Loopback"))
                continue;

            candidates++;
            if (firstUniverse < 0)
                firstUniverse = int(u);
            if (int(u) == m_ctrlUniverse)
                selectionValid = true;
        }
    }

    if (!selectionValid)
        m_ctrlUniverse = (candidates == 1) ? firstUniverse : -1;

    emit controllersModelChanged();
    emit controllerChanged();
}

void StageWizard::setControllerUniverse(int universe)
{
    if (universe == m_ctrlUniverse)
        return;

    m_ctrlUniverse = universe;
    emit controllerChanged();
}

void StageWizard::setMapController(bool map)
{
    if (map == m_ctrlMap)
        return;

    m_ctrlMap = map;
    emit controllerChanged();
}

void StageWizard::setSendFeedback(bool enable)
{
    if (enable == m_ctrlFeedback)
        return;

    m_ctrlFeedback = enable;
    emit controllerChanged();
}

void StageWizard::setMapColors(bool enable)
{
    if (enable == m_ctrlColors)
        return;

    m_ctrlColors = enable;
    emit controllerChanged();
}

QString StageWizard::controllerMappingPreview() const
{
    if (m_ctrlUniverse < 0 || !m_ctrlMap)
        return QString();

    CtrlSurface surface;
    buildControllerSurface(surface);

    // Estimate demand from the layout createVCLayout() will build. Kept in step
    // with the VC generator: page tabs + show cues are shared across pages, and
    // each group page adds its colours, effects, a fader and (movers only) a pad.
    QList<const FixtureGroupEntry *> pageGroups;
    for (const FixtureGroupEntry &grp : m_groups)
        if (grp.selected && !grp.fixtureIDs.isEmpty())
            pageGroups.append(&grp);

    const int pageCount = pageGroups.count() + 1;   // + "All Groups"

    int buttons = pageCount   // one page tab per page
                + 6;          // Blackout / Open / Big Moment / Close / Ambient / Blinder
    int faders  = 0;
    int maxColors = 0;        // busiest page, which is what sizes the colour band

    auto countPage = [&](const FixtureGroupEntry &grp)
    {
        if (grp.hasDimmer || grp.hasRGB || grp.hasCMY)
            faders++;                       // Intensity slider
        if (grp.hasMovement)
        {
            faders  += 2;                   // XY pad pan + tilt
            buttons += 5;                   // Fly Out/In, Circle, Eight, Sweep
        }
        buttons += 2;                       // Strobe Chase, Heartbeat
        // Colour buttons: one per generated colour scene for this group.
        int colors = 0;
        for (quint32 id : m_generatedFunctionIDs)
        {
            Function *f = m_doc->function(id);
            if (f && f->type() == Function::SceneType &&
                f->path().contains(grp.name + "/" + tr("Colors")))
                colors++;
        }
        buttons  += colors;
        maxColors = qMax(maxColors, colors);
    };

    if (m_hasAllGroups)
        countPage(m_allGroups);
    for (const FixtureGroupEntry *grp : pageGroups)
        countPage(*grp);

    if (surface.kind == CtrlSurfaceStrip)
    {
        // Faders only — say so, so the empty pad grid isn't read as a bug.
        return tr("%1 of %2 faders — this controller's buttons are channel "
                  "strips, so they are left unmapped")
               .arg(qMin(faders, surface.faderTotal)).arg(surface.faderTotal);
    }

    if (surface.kind == CtrlSurfaceGrid)
        assignControllerBands(surface, pageCount,
                              (maxColors + surface.grid.cols - 1) /
                              qMax(1, surface.grid.cols));

    QStringList parts;
    parts << tr("%1 of %2 buttons").arg(qMin(buttons, surface.buttonTotal))
                                   .arg(surface.buttonTotal);
    if (surface.faderTotal > 0 || faders > 0)
        parts << tr("%1 of %2 faders").arg(qMin(faders, surface.faderTotal))
                                      .arg(surface.faderTotal);

    QString text = parts.join(tr(", "));

    if (surface.kind == CtrlSurfaceGrid)
        text = tr("%1 grid — ").arg(tr("%1x%2").arg(surface.grid.rows)
                                               .arg(surface.grid.cols)) + text;

    if (buttons > surface.buttonTotal || faders > surface.faderTotal)
        text += tr(" — not everything fits; the rest stays unmapped");

    return text;
}

// ── Grid recognition ─────────────────────────────────────────────────────────

bool StageWizard::parseControllerGrid(const QLCInputProfile *profile,
                                      CtrlGrid &grid, QList<CtrlChannel> &aux)
{
    grid = CtrlGrid();
    aux.clear();

    if (profile == nullptr)
        return false;

    // The naming conventions actually used by the shipped profiles. All of them
    // encode a pad matrix in the channel NAME, which is the only geometry hint
    // a .qxi carries — channel numbers alone can't be trusted (the APC40 mkII
    // jumps from 167 to 4103 mid-surface).
    static const QRegularExpression reRowCol(
        // "Button 3-5", "Pad 3-5", "Button 3.5B" (X-Touch layer suffix),
        // "Matrix 3-5", "Clip Launch 3 - 5"
        QStringLiteral("^(?:button|pad|matrix|clip\\s+launch)\\s*"
                       "(\\d+)\\s*[-.]\\s*(\\d+)[ab]?$"),
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reRowThenCol(
        // "Row 1 (top) Button 3"
        QStringLiteral("^row\\s*(\\d+)[^0-9]*?button\\s*(\\d+)$"),
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reLinearPad(
        // "PAD7", "MATRIX BUTTON 07", "Touchpad 7" — a flat run we reflow.
        QStringLiteral("^(?:pad|matrix\\s+button|touchpad)\\s*(\\d+)$"),
        QRegularExpression::CaseInsensitiveOption);

    // Some profiles say outright which end is up ("Row 1 (top) Button 3",
    // "Row 4 (bottom) Button 1"). That beats any heuristic, so look for it.
    static const QRegularExpression reTopHint(
        QStringLiteral("\\btop\\b"), QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reBottomHint(
        QStringLiteral("\\bbottom\\b"), QRegularExpression::CaseInsensitiveOption);

    // $a and $b are the two numbers in the name, in the order they appear. Which
    // is the row and which the column is NOT fixed across profiles — the APC
    // mini writes "Button <col>-<row>" while the APC40 and Launchpad MK2 write
    // "<row>-<col>" — so the axes are resolved below from the note layout
    // rather than assumed.
    struct Cell { int a; int b; CtrlChannel ch; };
    QList<Cell> cells;
    QList<QPair<int, CtrlChannel>> linear;   // index -> channel

    int  hintRow = -1;          ///< Row the profile explicitly labelled
    bool hintRowIsTop = false;
    bool hintFirstIsRow = false;///< The labelled form puts the row first
    bool reflowed = false;      ///< Grid synthesised from a flat pad run

    QMapIterator<quint32, QLCInputChannel *> it(profile->channels());
    while (it.hasNext())
    {
        it.next();
        const QLCInputChannel *ich = it.value();
        if (ich->type() != QLCInputChannel::Button)
            continue;

        CtrlChannel cc { it.key(), ich->name() };
        const QString name = ich->name().trimmed();

        QRegularExpressionMatch m = reRowCol.match(name);
        if (!m.hasMatch())
            m = reRowThenCol.match(name);

        if (m.hasMatch())
        {
            const int a = m.captured(1).toInt();
            cells.append({ a, m.captured(2).toInt(), cc });

            // "Row 1 (top)" / "Row 4 (bottom)". Only the reRowThenCol form
            // carries such a hint, and there the first number IS the row.
            if (hintRow < 0)
            {
                if (reTopHint.match(name).hasMatch())
                {
                    hintRow = a;
                    hintRowIsTop = true;
                    hintFirstIsRow = true;
                }
                else if (reBottomHint.match(name).hasMatch())
                {
                    hintRow = a;
                    hintRowIsTop = false;
                    hintFirstIsRow = true;
                }
            }
            continue;
        }

        m = reLinearPad.match(name);
        if (m.hasMatch())
        {
            linear.append({ m.captured(1).toInt(), cc });
            continue;
        }

        // Anything else is a side strip / transport / modifier key.
        aux.append(cc);
    }

    // A flat pad run (no row-column names) is reflowed into a matrix, 8 wide
    // where that divides evenly, else 4 — matching how such pads are printed.
    if (cells.isEmpty() && linear.count() >= kMinGridCells)
    {
        std::sort(linear.begin(), linear.end(),
                  [](const QPair<int, CtrlChannel> &a,
                     const QPair<int, CtrlChannel> &b)
                  { return a.first < b.first; });

        const int cols = (linear.count() % 8 == 0) ? 8
                       : (linear.count() % 4 == 0) ? 4 : 8;
        for (int i = 0; i < linear.count(); i++)
            cells.append({ i / cols + 1, i % cols + 1, linear.at(i).second });
        reflowed = true;
    }
    else
    {
        // Named cells won: the flat pads are a different control block.
        for (const QPair<int, CtrlChannel> &p : linear)
            aux.append(p.second);
    }

    if (cells.count() < kMinGridCells)
    {
        // Not a grid after all — give every button back to the caller.
        for (const Cell &c : cells)
            aux.append(c.ch);
        return false;
    }

    int maxA = 0, maxB = 0;
    for (const Cell &c : cells)
    {
        maxA = qMax(maxA, c.a);
        maxB = qMax(maxB, c.b);
    }
    if (maxA < 1 || maxB < 1)
        return false;

    // ── Which number is the row? ────────────────────────────────────────────
    // Not fixed across profiles: the APC mini names pads "<col>-<row>", while
    // the APC40 and Launchpad MK2 use "<row>-<col>". Resolve it from the note
    // layout — pads are laid out along one axis in consecutive notes, and that
    // fast axis is the COLUMN (hardware scans a row left to right). So whichever
    // number varies while the notes run consecutively is the column.
    bool firstIsRow = true;

    if (hintFirstIsRow)
    {
        firstIsRow = true;      // the profile said "Row N …" outright
    }
    else if (!reflowed)
    {
        // Count adjacent note pairs (stride 1) that keep $a fixed vs keep $b
        // fixed. The axis with more such runs is the fast axis = the column.
        QHash<quint32, int> indexByChannel;   // channel -> index into $cells
        for (int i = 0; i < cells.count(); i++)
            indexByChannel.insert(cells.at(i).ch.channel, i);

        int aFixedRuns = 0, bFixedRuns = 0;
        for (const Cell &c : cells)
        {
            const int nextIdx = indexByChannel.value(c.ch.channel + 1, -1);
            if (nextIdx < 0)
                continue;
            const Cell &next = cells.at(nextIdx);
            if (next.a == c.a)
                aFixedRuns++;   // b is the fast axis => b is the column
            else if (next.b == c.b)
                bFixedRuns++;   // a is the fast axis => a is the column
        }

        if (aFixedRuns != bFixedRuns)
        {
            // a fixed while b runs => b is the column => a is the row.
            firstIsRow = aFixedRuns > bFixedRuns;
        }
    }

    const int maxRow = firstIsRow ? maxA : maxB;
    const int maxCol = firstIsRow ? maxB : maxA;
    if (maxRow < 1 || maxCol < 1)
        return false;

    auto rowOf = [&](const Cell &c) { return firstIsRow ? c.a : c.b; };
    auto colOf = [&](const Cell &c) { return firstIsRow ? c.b : c.a; };

    // ── Which end is physically UP? ─────────────────────────────────────────
    // Also inconsistent: Akai launchers number row 1 at the BOTTOM, Novation at
    // the top. Decide from evidence, and otherwise keep the profile's order.
    bool flip = false;

    if (hintRow >= 0)
    {
        // The profile spelled it out: "Row 1 (top)" / "Row 4 (bottom)".
        flip = (hintRow == 1) ? !hintRowIsTop : hintRowIsTop;
    }
    else if (reflowed)
    {
        // A flat "PAD1..PAD16" run was reflowed in index order, which already
        // reads top-left to bottom-right. Flipping would reverse it.
        flip = false;
    }
    else
    {
        // Row 1 holding the LOWEST notes means the numbering starts at the
        // bottom, because a pad grid's note 0 is its bottom-left pad.
        quint32 firstRowMin = UINT_MAX, lastRowMin = UINT_MAX;
        for (const Cell &c : cells)
        {
            if (rowOf(c) == 1)
                firstRowMin = qMin(firstRowMin, c.ch.channel);
            if (rowOf(c) == maxRow)
                lastRowMin = qMin(lastRowMin, c.ch.channel);
        }
        if (firstRowMin != UINT_MAX && lastRowMin != UINT_MAX)
            flip = firstRowMin < lastRowMin;
    }

    grid.rows  = maxRow;
    grid.cols  = maxCol;
    grid.cells = QList<CtrlChannel>();
    grid.cells.reserve(maxRow * maxCol);
    for (int i = 0; i < maxRow * maxCol; i++)
        grid.cells.append({ CtrlGrid::invalidChannel, QString() });

    for (const Cell &c : cells)
    {
        const int row = rowOf(c);
        const int r = flip ? (maxRow - row) : (row - 1);
        const int col = colOf(c) - 1;
        if (r < 0 || r >= maxRow || col < 0 || col >= maxCol)
            continue;
        grid.cells[r * maxCol + col] = c.ch;
    }

    return true;
}

// ── Row bands ────────────────────────────────────────────────────────────────

void StageWizard::assignControllerBands(CtrlSurface &surface, int pageCount,
                                        int colorRows)
{
    for (int i = 0; i < CtrlRoleCount; i++)
        surface.bands[i] = CtrlBand();

    if (!surface.grid.isValid())
        return;

    const int rows = surface.grid.rows;
    const int cols = qMax(1, surface.grid.cols);

    // Rows each role needs, before any squeezing.
    const int tabRows = qMax(1, (pageCount + cols - 1) / cols);
    const int cueRows = 1;                       // six show cues, one row
    int       colRows = qMax(1, colorRows);
    int       fxRows  = 1;

    // The show cues own the BOTTOM row and the page tabs the TOP row: those are
    // the two blocks that persist across every page, so anchoring them to the
    // edges keeps them findable while the middle changes with the selected page.
    int budget = rows - tabRows - cueRows;
    if (budget < 2)
    {
        // A very short surface (4 rows or fewer): drop to one row per role and,
        // if it still doesn't fit, let the bands collapse — nextRoleChannel()
        // spills into aux, so nothing is lost, it just stops being spatial.
        colRows = 1;
        fxRows  = qMax(0, rows - tabRows - cueRows - 1);
    }
    else
    {
        // Give colours what they asked for, effects the rest, both at least one
        // row, and hand any slack to colours (they overflow the most).
        colRows = qBound(1, colRows, budget - 1);
        fxRows  = budget - colRows;
        if (fxRows > 2)
        {
            // Effects rarely exceed two rows; the surplus is more useful as
            // colour overflow than as empty pads.
            colRows += fxRows - 2;
            fxRows = 2;
        }
    }

    int row = 0;
    surface.bands[CtrlRolePageTab] = { row, tabRows, 0 };
    row += tabRows;

    surface.bands[CtrlRoleColor] = { row, qMax(0, qMin(colRows, rows - row)), 0 };
    row += surface.bands[CtrlRoleColor].rowCount;

    surface.bands[CtrlRoleEffect] = { row, qMax(0, qMin(fxRows, rows - row - cueRows)), 0 };

    // Cues always take the last row, whatever happened above it.
    surface.bands[CtrlRoleShowCue] = { qMax(0, rows - cueRows), cueRows, 0 };
}

// ── Surface construction ─────────────────────────────────────────────────────

void StageWizard::buildControllerSurface(CtrlSurface &surface) const
{
    surface = CtrlSurface();

    if (m_ctrlUniverse < 0)
        return;

    QLCInputProfile *profile = controllerProfile();

    if (profile == nullptr)
    {
        // No profile: synthesise a plausible layout so a bare patch still maps.
        for (int i = 0; i < kNoProfileFaders; i++)
            surface.faders.append({ quint32(i), QString() });
        for (int i = 0; i < kNoProfileButtons; i++)
            surface.buttons.append({ quint32(kNoProfileFaders + i), QString() });

        surface.kind        = CtrlSurfaceLinear;
        surface.buttonTotal = surface.buttons.count();
        surface.faderTotal  = surface.faders.count();
        return;
    }

    // Faders come out in channel order, which is the order they sit on the
    // device — profile channels are a QMap keyed by channel number.
    QMapIterator<quint32, QLCInputChannel *> it(profile->channels());
    while (it.hasNext())
    {
        it.next();
        const QLCInputChannel *ich = it.value();
        switch (ich->type())
        {
            case QLCInputChannel::Slider:
            case QLCInputChannel::Knob:
            case QLCInputChannel::Encoder:
                surface.faders.append({ it.key(), ich->name() });
                break;
            default:
                // Buttons are handled by parseControllerGrid(); NextPage /
                // PrevPage / PageSet / NoType are left alone, since they have a
                // dedicated meaning the user may already rely on.
                break;
        }
    }

    QList<CtrlChannel> aux;
    if (parseControllerGrid(profile, surface.grid, aux))
    {
        surface.kind = CtrlSurfaceGrid;
        surface.aux  = aux;

        int cellCount = 0;
        for (const CtrlChannel &cc : surface.grid.cells)
            if (cc.channel != CtrlGrid::invalidChannel)
                cellCount++;

        surface.buttonTotal = cellCount + aux.count();
    }
    else if (aux.count() <= kStripButtonCeiling && !surface.faders.isEmpty())
    {
        // Mixer strip: the buttons are per-channel Solo/Mute/Rec sitting under
        // the faders. They read as channel controls, not scene launchers, and
        // there are too few of them to hold a show anyway — so map the faders
        // and leave every button to the user.
        surface.kind        = CtrlSurfaceStrip;
        surface.buttonTotal = 0;
    }
    else
    {
        // Plenty of buttons but no layout we recognise: fall back to handing
        // them out in channel order, which is what the wizard always did.
        surface.kind        = CtrlSurfaceLinear;
        surface.buttons     = aux;
        surface.buttonTotal = surface.buttons.count();
    }

    surface.faderTotal = surface.faders.count();
}

// ── Channel hand-out ─────────────────────────────────────────────────────────

bool StageWizard::nextRoleChannel(CtrlSurface &surface, CtrlRole role,
                                  quint32 &channel) const
{
    if (surface.kind == CtrlSurfaceStrip)
        return false;   // buttons deliberately left alone

    if (surface.kind == CtrlSurfaceGrid && role >= 0 && role < CtrlRoleCount)
    {
        CtrlBand &band = surface.bands[role];
        const int cols = surface.grid.cols;
        const int cap  = band.rowCount * cols;

        // Walk the band left-to-right, top-to-bottom, skipping holes.
        while (band.nextCell < cap)
        {
            const int idx = band.nextCell++;
            const CtrlChannel *cc = surface.grid.at(band.firstRow + idx / cols,
                                                    idx % cols);
            if (cc != nullptr)
            {
                channel = cc->channel;
                return true;
            }
        }

        // Band full: spill into the side strips / transport keys rather than
        // stealing a cell another role is holding for spatial consistency.
        if (surface.nextAux < surface.aux.count())
        {
            channel = surface.aux.at(surface.nextAux++).channel;
            return true;
        }
        return false;
    }

    if (surface.nextButton >= surface.buttons.count())
        return false;

    channel = surface.buttons.at(surface.nextButton++).channel;
    return true;
}

bool StageWizard::nextFaderChannel(CtrlSurface &surface, quint32 &channel) const
{
    if (surface.nextFader >= surface.faders.count())
        return false;

    channel = surface.faders.at(surface.nextFader++).channel;
    return true;
}

// ── Colour feedback ──────────────────────────────────────────────────────────

int StageWizard::closestProfileColor(const QColor &color) const
{
    QLCInputProfile *profile = controllerProfile();
    if (profile == nullptr || !profile->hasColorTable() || !color.isValid())
        return -1;

    // Nearest entry by squared RGB distance. Good enough for the coarse LED
    // palettes controllers expose (typically 8-128 entries), and it degrades
    // gracefully: an off-palette colour lands on the closest one available.
    int    bestValue = -1;
    qint64 bestDist  = std::numeric_limits<qint64>::max();

    QMapIterator<uchar, QPair<QString, QColor>> it(profile->colorTable());
    while (it.hasNext())
    {
        it.next();
        const QColor &c = it.value().second;
        if (!c.isValid())
            continue;

        qint64 dr = c.red()   - color.red();
        qint64 dg = c.green() - color.green();
        qint64 db = c.blue()  - color.blue();
        qint64 dist = dr * dr + dg * dg + db * db;

        if (dist < bestDist)
        {
            bestDist  = dist;
            bestValue = int(it.key());
        }
    }

    return bestValue;
}

void StageWizard::ensureFeedbackPatch()
{
    if (!m_ctrlFeedback || m_ctrlUniverse < 0)
        return;

    InputOutputMap *iomap = m_doc->inputOutputMap();
    if (iomap == nullptr)
        return;

    quint32 uni = static_cast<quint32>(m_ctrlUniverse);

    // Already set up (e.g. the user patched it themselves in the I/O panel).
    if (iomap->feedbackPatch(uni) != nullptr)
        return;

    InputPatch *ip = iomap->inputPatch(uni);
    if (ip == nullptr)
        return;

    // Feedback goes back out over the plugin's output line that carries the
    // same name as the input line — that is the same physical device.
    const QString inputName = ip->inputName();
    int line = 0;
    for (const QString &pLine : iomap->pluginOutputs(ip->pluginName()))
    {
        if (pLine == inputName)
        {
            iomap->setOutputPatch(uni, ip->pluginName(), QString(), QString(),
                                  quint32(line), true);
            m_doc->setModified();
            return;
        }
        line++;
    }

    qDebug() << "[StageWizard] no matching feedback output line for" << inputName;
}

// ── Widget mapping ───────────────────────────────────────────────────────────

void StageWizard::mapWidgetControl(VCWidget *widget, quint8 controlID,
                                   quint32 channel, const QColor &color)
{
    if (widget == nullptr || m_ctrlUniverse < 0)
        return;

    QSharedPointer<QLCInputSource> src(
        new QLCInputSource(static_cast<quint32>(m_ctrlUniverse), channel));
    src->setID(controlID);

    // VCPage::inputValueChanged() only dispatches when the source's page equals
    // the widget's page, so a widget sitting on frame page 2 needs a source
    // tagged page 2. The widget already knows its page (setupLookAndFeel() set
    // it when the multipage frame took ownership), so mirror it here — leaving
    // the default 0 would silently break every page but the first.
    src->setPage(static_cast<ushort>(qMax(0, widget->page())));

    // Colour feedback: drive the LED with the colour-table value nearest to the
    // button's background. The "on" (upper) state gets the colour, the "off"
    // (lower) state stays dark. Must be set BEFORE addInputSource(), because
    // applyInputProfileSettings() only fills in values the source hasn't
    // customised (it treats lower != 0 / upper != UCHAR_MAX as user intent).
    if (m_ctrlColors && color.isValid())
    {
        int fbValue = closestProfileColor(color);

        // A value of 255 is indistinguishable from "untouched" to
        // applyInputProfileSettings() (it tests upper != UCHAR_MAX), so it would
        // be replaced by the profile default. Colour tables index LED palettes
        // and effectively never use the last slot, so skipping it costs nothing.
        if (fbValue >= 0 && fbValue != UCHAR_MAX)
        {
            src->setFeedbackValue(QLCInputFeedback::LowerValue, 0);
            src->setFeedbackValue(QLCInputFeedback::UpperValue, uchar(fbValue));
        }
    }

    // addInputSource() runs applyInputProfileSettings(), which pulls the
    // profile's per-channel feedback extra params (MIDI channel, OSC path, …),
    // relative/encoder working mode and the button lower/upper values.
    widget->addInputSource(src);
}
