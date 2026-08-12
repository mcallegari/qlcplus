/*
  Q Light Controller Plus
  stagewizard_placement.cpp

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

QVector3D StageWizard::fixtureDrawnSizeMM(quint32 fixtureID) const
{
    MainView3D *view3D = m_contextManager ? m_contextManager->get3DView() : nullptr;
    if (view3D != nullptr)
    {
        QVector3D drawn = view3D->fixtureDrawnSize(fixtureID) * 1000.0f;
        if (drawn.x() > 0.0f && drawn.y() > 0.0f && drawn.z() > 0.0f)
            return drawn;
    }

    // No 3D view at all (headless / tests): fall back to the declared size.
    QVector3D declared(300.0f, 300.0f, 300.0f);
    Fixture *fx = m_doc->fixture(fixtureID);
    if (fx && fx->fixtureMode())
    {
        QLCPhysical phy = fx->fixtureMode()->physical();
        if (phy.width() > 0)  declared.setX(phy.width());
        if (phy.height() > 0) declared.setY(phy.height());
        if (phy.depth() > 0)  declared.setZ(phy.depth());
    }
    return declared;
}

void StageWizard::trussGeometryMM(float &halfMM, float &bottomYMM, float &topYMM) const
{
    MainView3D *view3D = m_contextManager ? m_contextManager->get3DView() : nullptr;

    if (view3D != nullptr)
    {
        qreal bottomY = 0.0, topY = 0.0;
        view3D->trussVerticalSpan(bottomY, topY);
        halfMM = float(view3D->trussHalfSize()) * 1000.0f;

        if (halfMM > 0.0f)
        {
            bottomYMM = float(bottomY) * 1000.0f;
            topYMM    = float(topY) * 1000.0f;
            return;
        }
    }

    // Stage without trusses, or no 3D view: hang from the top of the
    // environment box itself, with no bar thickness to account for.
    halfMM    = 0.0f;
    bottomYMM = m_envSize.y() * 1000.0f;
    topYMM    = bottomYMM;
}

/** How many blinders of a bank of $total go in the horizontal front row.
 *
 *  Roughly two thirds across the front truss, rounded to an EVEN count (a
 *  blinder bank reads symmetrically about the centre line), with the remainder
 *  reserved for the vertical side columns. $frontCapacity caps it by the
 *  spacing rule. Shared by computePosition() and computeRotation() so the two
 *  can never disagree about which units are horizontal and which are vertical.
 */
static int blinderFrontCount(int total, int frontCapacity)
{
    int frontCount;
    if (total <= 2)
    {
        frontCount = total;             // too few to spare any for the wings
    }
    else
    {
        frontCount = (total * 2) / 3;
        frontCount -= (frontCount % 2); // keep it symmetric
        frontCount = qMax(frontCount, 2);
        frontCount = qMin(frontCount, total - 1); // always leave a side unit
    }
    return qMin(frontCount, frontCapacity);
}

// Rigging rule: fixtures sit 50 cm to 1 m apart, centre-to-centre.
// Below 50 cm moving heads foul each other when panning and static beams merge
// into one blob; beyond ~1 m a row stops reading as one bank and just looks
// sparse. Rows are packed at the minimum and only opened up to the maximum when
// there is room, rather than stretching to fill the whole truss.
static const float kMinFixtureGapMM = 500.0f;
static const float kMaxFixtureGapMM = 1000.0f;

// ── 3D fixture placement ───────────────────────────────────────────────────────

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
    for (const FixtureGroupEntry &grp : m_groups)
    {
        if (!grp.selected || grp.fixtureIDs.isEmpty())
            continue;

        // Pre-existing groups already have a layout the user set up: don't touch
        // their 3D placement, only generate functions/VC for them.
        if (grp.groupId != FixtureGroup::invalidId())
            continue;

        // One placement slot per FIXTURE — never per head. A head is a cell
        // inside a single physical body (a FREQ 16 strobe has 16 of them, an
        // LED bar has one per pixel); they all hang together at one point.
        // Spreading heads across the truss produced one monitor item per cell
        // and scattered a single strobe over the whole rig.
        QList<quint32> placements;
        for (quint32 fxID : grp.fixtureIDs)
        {
            if (m_doc->fixture(fxID))
                placements.append(fxID);
        }

        int total = placements.count();
        for (int i = 0; i < total; ++i)
        {
            quint32 fxID = placements[i];
            // Head 0 / linked 0 is the fixture's own monitor item.
            quint32 itemID = FixtureUtils::fixtureItemID(fxID, 0, 0);

            // Fixture size in millimetres. This MUST be the size the 3D view
            // will actually DRAW the item at — the renderer offsets every item
            // by +extents/2, so snapping against any other number leaves the
            // fixture floating below the truss or punching through it.
            //
            // MainView3D owns that answer (it knows which generic mesh a type
            // uses and how updateFixtureScale() fits it into the declared box),
            // so ask it rather than second-guessing the geometry here.
            QVector3D fxSize = fixtureDrawnSizeMM(fxID);

            QVector3D pos = computePosition(i, total, grp.role, m_envSize, fxSize);
            QVector3D rot = computeRotation(grp.role, i, total, m_envSize, fxSize);

            qDebug() << "[StageWizard] place fx" << fxID << "role" << grp.role
                     << "env(m)" << m_envSize << "fxSize(mm)" << fxSize
                     << "pos(mm)" << pos << "rot" << rot;

            // Route through ContextManager so the change is persisted AND the
            // live 2D/3D views are refreshed (writing MonitorProperties directly
            // would not update the running 3D view).
            m_contextManager->setFixturePosition(itemID, pos.x(), pos.y(), pos.z());
            m_contextManager->setFixtureRotation(itemID, rot);
        }
    }
}

QVector3D StageWizard::computePosition(int index, int total,
                                       FixtureRole role,
                                       const QVector3D &gridM,
                                       const QVector3D &fxSizeMM) const
{
    // Grid size in millimetres (monitor units). The vertical extent is not read
    // from here: everything overhead hangs off the truss, whose height comes
    // from the stage model via trussGeometryMM().
    const float gx = gridM.x() * 1000.0f;
    const float gz = gridM.z() * 1000.0f;

    // Fixture dimensions in mm
    const float fxW = fxSizeMM.x();
    const float fxH = fxSizeMM.y();
    const float fxD = fxSizeMM.z();

    // ── Truss geometry (read from the stage model at runtime) ───────────────
    // The stage QML declares `trussHalfSize` and builds the rig in WORLD metres:
    //   bar centre y = sizeMeters.y + trussHalfSize
    //   corners at   ±(sizeMeters.x/2 + trussHalfSize),
    //                ±(sizeMeters.z/2 + trussHalfSize)
    //
    // So the truss sits entirely ABOVE the environment box and just outside it
    // — it does not straddle the boundary. In monitor space (x,z shifted by
    // +grid/2, y measured from the floor) the bar spans gy .. gy + 2*trussHalf,
    // which makes the UNDERSIDE exactly gy. Assuming it straddled gy hung every
    // fixture one bar-thickness too low.
    float trussHalf = 0.0f, trussBottomY = 0.0f, trussTopY = 0.0f;
    trussGeometryMM(trussHalf, trussBottomY, trussTopY);
    const float trussMidY = trussBottomY + trussHalf;       // bar centre line

    // A truss has a SQUARE section, so it offers four mounting faces. Different
    // roles share one bar by using different faces, which is how a real rig is
    // built (movers slung underneath, blinders on the audience face):
    //
    //        ┌───────┐  ← top face    (y = trussTopY,    uplight / batten)
    //   back │  ███  │  front face    (z = truss front,  facing the audience)
    //        └───────┘  ← bottom face (y = trussBottomY, hung upside down)

    // BOTTOM face: the fixture's TOP touches the underside and it hangs down.
    // worldY = pos.y/1000 + meshExtY/2 and the body top is pos.y + meshExtY, so
    // pos.y = underside - fxH puts the top exactly on the bar. fxH must be the
    // extent the renderer actually draws (see applyStageLayout / meshFittedSize)
    // or the fixture floats off by the difference.
    const float hangY = trussBottomY - fxH;

    // SIDE faces: the fixture is centred on the bar's own centre line, so it
    // straddles the bar exactly like a real clamp-on unit.
    const float faceY = trussMidY - fxH / 2.0f;

    // Snap to the front / rear truss CENTRE lines, cancelling the render offset
    // (worldZ = pos.z/1000 - gz/2 + fxD/2).
    const float frontZ = gz + trussHalf - fxD / 2.0f;   // front bar centre
    const float rearZ  = -trussHalf - fxD / 2.0f;       // rear bar centre

    // Front FACE of the front bar: hard against its audience side, so a blinder
    // here shares the bar with the movers slung underneath it.
    const float frontFaceZ = gz + trussHalf * 2.0f - fxD / 2.0f;

    // Minimum centre-to-centre gap between neighbouring fixtures. Rigging rule:
    // never pack heads closer than 50 cm, or they collide when panning/tilting
    // and the beams overlap into a single blob.
    // minGap is edge-to-edge clearance: a wide bar needs its own width plus the
    // 50 cm rule, or two of them end up touching.
    const float minGap = fxW + kMinFixtureGapMM;
    // The upper bound has to scale with the fixture too. A fixed 1 m cap on a
    // 1 m-wide strobe bar collapses to "bars touching", which is what crammed
    // 4 FREQ 16s into the middle 20% of a 20 m truss.
    const float maxGap = qMax(kMaxFixtureGapMM, fxW + kMaxFixtureGapMM);

    // Centre $count fixtures on $mid at a pitch inside [minGap, maxGap], using
    // as much of $from..$to as that allows. Without the max, a small group on a
    // wide truss ends up strung out along the whole bar instead of reading as
    // one bank.
    auto bankOf = [&](float from, float to, float mid, int idx, int count) -> float
    {
        if (count <= 1)
            return mid - fxW / 2.0f;

        float pitch = (to - from) / float(count);
        pitch = qBound(minGap, pitch, maxGap);

        float width = pitch * float(count - 1);
        float start = mid - width / 2.0f;
        // Keep the bank inside the span if it would overhang.
        start = qBound(from, start, qMax(from, to - width));

        return start + pitch * float(idx) - fxW / 2.0f;
    };

    // How many fixtures fit along $span while honouring minGap.
    // spreadOf() below divides the span into $count equal sub-cells and centres
    // one fixture in each, so neighbouring centres sit span/count apart — the
    // capacity is therefore floor(span / minGap), NOT one more than that.
    auto capacity = [&](float span) -> int
    {
        return qMax(1, int(span / minGap));
    };

    // Evenly spread the group along the span, centred on the span midpoint.
    // The -fxW/2 term cancels the X render offset (worldX = pos.x/1000 - gx/2
    // + meshExtX/2) so the group ends up centred on the truss.
    //
    // $idx / $count let callers spread a SUBSET (e.g. one row of a wrapped
    // layout) rather than the whole group.
    auto spreadOf = [&](float from, float to, int idx, int count) -> float
    {
        float centre = (count <= 1)
                       ? (from + to) * 0.5f
                       : from + (to - from) * ((idx + 0.5f) / float(count));
        return centre - fxW / 2.0f;
    };

    auto spread = [&](float from, float to) -> float
    {
        return spreadOf(from, to, index, total);
    };

    const float endPad = qMax(gx * 0.06f, fxW);  // clear the truss corners
    const float xFrom  = endPad;
    const float xTo    = gx - endPad;

    // X of the left / right side-truss line, with the render offset applied and
    // clamped to the stage — on a wide fixture the -fxW/2 term would otherwise
    // push the unit outside the grid (negative X, or past the right edge).
    auto sideX = [&](bool left) -> float
    {
        return left ? qMax(0.0f, trussHalf - fxW / 2.0f)
                    : qMin(gx - fxW, gx - trussHalf - fxW / 2.0f);
    };

    // A truss row that respects minGap: when the group is too big for one row,
    // it wraps into additional rows stepped upstage, so nothing ever overlaps.
    // $baseZ is the row's Z, $stepZ the upstage offset applied per extra row.
    auto rowSpread = [&](float baseZ, float stepZ) -> QVector3D
    {
        // Pick the span first, then derive the row size from THAT span, so the
        // resulting pitch (span / perRow) is guaranteed >= minGap. Deriving the
        // two from different spans is what silently breaks the spacing rule.
        //
        // Prefer the padded span (keeps fixtures clear of the truss corners) and
        // widen to the full grid only when the padding is what forces the row
        // over capacity: the 50 cm rule outranks the cosmetic padding.
        float from = xFrom, to = xTo;
        if (capacity(to - from) < total)
        {
            from = fxW / 2.0f;
            to   = gx - fxW / 2.0f;
        }

        int perRow = qMin(total, capacity(to - from));
        int row    = index / perRow;
        int col    = index % perRow;
        // The last row may be partially filled; it is centred on the truss like
        // the full rows above it.
        int rowCount = qMin(perRow, total - row * perRow);

        // Rows must clear each other by minGap too: on a shallow stage the
        // proportional step (a fraction of the depth) can be smaller than the
        // fixture spacing, which would put a wrapped row right on top of the one
        // in front of it. Keep the caller's direction, enforce the magnitude.
        // Row separation is along Z, so it is bounded by the fixture's DEPTH
        // plus clearance — not by minGap, which is the X-axis (width) rule.
        float zGap = qMax(kMinFixtureGapMM, fxD + kMinFixtureGapMM);
        float step = (stepZ < 0.0f) ? qMin(stepZ, -zGap) : qMax(stepZ, zGap);

        return QVector3D(bankOf(from, to, gx * 0.5f, col, rowCount),
                         hangY, baseZ + step * row);
    };

    switch (role)
    {
        case RoleKey:
            // Front truss, hung overhead, spread across the full width.
            // Overflow rows step UPSTAGE (away from the audience).
            return rowSpread(frontZ, -gz * 0.12f);

        case RoleFill:
            // Overhead, one step upstage of the key light row
            return rowSpread(gz * 0.72f + fxD / 2.0f, -gz * 0.12f);

        case RoleBack:
            // Rear truss, overhead. Overflow rows step DOWNSTAGE (towards the
            // audience), since there is nothing behind the rear truss.
            return rowSpread(rearZ, gz * 0.12f);

        case RoleEffect:
            // Top mid truss, over the centre line
            return rowSpread(gz * 0.5f, -gz * 0.12f);

        case RoleStrip:
            // LED batten sitting ON TOP of the front truss, washing upwards —
            // the top face is otherwise unused, and it leaves the underside of
            // the bar free for movers.
            return QVector3D(bankOf(fxW, gx - fxW, gx * 0.5f, index, total),
                             trussTopY, frontZ);

        case RoleBlinder:
        {
            // Blinders always live on the FRONT of the rig, facing the audience:
            // a horizontal row on the front truss, plus a vertical pair on the
            // downstage end of the side trusses (the classic "goalpost" look).
            //
            // Split rule: most of the bank goes horizontally across the front
            // truss, and a symmetric pair is reserved for the side columns so
            // the rig always has vertical blinders in the wings. Roughly two
            // thirds front / one third side, rounded to an EVEN front count
            // (blinders read symmetrically about the centre line), and the
            // remainder split evenly left/right.
            //   6 → 4 front + 1 per side      8 → 6 front + 1 per side
            //   4 → 2 front + 1 per side      3 → 2 front + 1 side
            // Below 3 units there is nothing to spare, so they all go front.
            // The 50 cm rule caps how many fit across the front; the rest spill
            // onto the side columns.
            int frontCount = blinderFrontCount(total, capacity(xTo - xFrom));

            if (index < frontCount)
            {
                // Horizontal row on the FRONT FACE of the front truss — the
                // same bar the key movers hang under, which is exactly how the
                // two roles share one truss.
                return QVector3D(bankOf(xFrom, xTo, gx * 0.5f, index, frontCount),
                                 faceY, frontFaceZ);
            }

            // Vertical banks on the downstage end of the side trusses,
            // alternating stage-left / stage-right and stacking DOWNWARDS.
            //
            // These units are ROLLED 90° about Z (see computeRotation) so the bar
            // stands upright on the column instead of poking into the wings.
            //
            // IMPORTANT: MainView3D::updateFixtureRotation() only sets the
            // quaternion — it does NOT re-derive the translation, which
            // updateFixturePosition() still computes from the UNROTATED
            // m_volume.m_extents. The mesh therefore spins about its own centre
            // while the anchor point stays put. So:
            //   * the POSITION must be computed with the unrotated extents
            //     (fxW/fxH), exactly like every other role;
            //   * the SPACING must use the rotated footprint, because that is
            //     what the bar physically sweeps once it is stood on end.
            // Mixing the two up is what made the rolled bars overlap.
            int   ov   = index - frontCount;
            bool  left = (ov % 2 == 0);
            int   tier = ov / 2;                 // 0 = top, 1 = below it, …

            const float rotW = fxH;              // width  it occupies once rolled
            const float rotH = fxW;              // height it occupies once rolled

            // Anchor on the side truss line using the UNROTATED width, since the
            // renderer's +extents/2 offset is unrotated.
            float xSide = sideX(left);

            // Vertical pitch from the ROLLED height (the bar's length) plus
            // clearance, so stacked bars never touch.
            float vStep = rotH + kMinFixtureGapMM;

            // The rolled bar extends rotH/2 above and below its centre, and the
            // centre sits fxH/2 above pos.y. Keep the whole bar under the truss.
            float topOfColumn = qMax(0.0f, trussBottomY - rotH / 2.0f - fxH / 2.0f);
            int   tiers = qMax(1, int(topOfColumn / vStep) + 1);

            float y = topOfColumn - float(tier % tiers) * vStep;

            // The bank hangs on the FRONT truss corner (same Z as the front row)
            // so it reads as the vertical leg of a goalpost.
            int   col   = tier / tiers;
            float zStep = qMax(kMinFixtureGapMM, fxD * 2.0f);
            int   zCols = qMax(1, int((gz * 0.90f) / zStep) + 1);
            float z = frontFaceZ - float(col % zCols) * zStep;
            z = qMax(z, fxD / 2.0f);
            float xNudge = float(col / zCols) * (rotW + kMinFixtureGapMM)
                                              * (left ? 1.0f : -1.0f);

            return QVector3D(qBound(0.0f, xSide + xNudge, gx - fxW), y, z);
        }

        case RoleSide:
        {
            // Side trusses: hung from the top side-truss line (same height as the
            // other overhead roles), alternating stage-left / stage-right, spread
            // along the depth. The head faces inward (Y rot 90) so its depth
            // footprint is along X — no fxD correction on Z here.
            // Snap X to the side-truss line, with the render offset (-fxW/2).
            bool left = (index % 2 == 0);
            float xSide = sideX(left);
            int   pairIndex = index / 2;
            int   pairCount = (total + 1) / 2;
            // Spread along the depth with a margin from the very front/rear.
            float t = (pairCount <= 1) ? 0.5f : (pairIndex + 0.5f) / float(pairCount);
            float z = gz * (0.20f + 0.60f * t);
            return QVector3D(xSide, hangY, z);
        }

        case RoleHazer:
            // On the floor, upstage centre
            return QVector3D(spread(gx * 0.3f, gx * 0.7f), 0.0f, gz * 0.2f);

        case RoleFloor:
        {
            // Uplighters standing ON THE DECK (y = 0), in a row across the back
            // of the stage, washing forward/up towards the performers.
            //
            // They are floor units: they belong on the floor, not stacked up the
            // truss uprights (an earlier revision put them there to "keep the
            // floor clear", which contradicts the role).
            //
            // Rear-most row sits just downstage of the back wall so the beam has
            // something to travel across; overflow rows step DOWNSTAGE, since
            // there is nothing behind the back row.
            const float baseZ = gz * 0.12f + fxD / 2.0f;
            QVector3D p = rowSpread(baseZ, gz * 0.12f);
            // rowSpread() places at truss height (hangY); these stand on the
            // deck, so override Y. X/Z spreading and the wrap rule still apply.
            p.setY(0.0f);
            return p;
        }

        default:
            return QVector3D(spread(xFrom, xTo), hangY, frontZ);
    }
}

QVector3D StageWizard::computeRotation(FixtureRole role,
                                       int index, int total,
                                       const QVector3D &gridM,
                                       const QVector3D &fxSizeMM) const
{
    // Rotation in degrees: X = tilt (about world X), Y = pan (about Y), Z = roll.
    // The fixture mesh loads ALREADY inverted (head-down), which is how movers
    // hang from a truss. So overhead roles keep X = 0 (no flip). Only the
    // floor-standing roles apply X = 180° to turn the mesh upright.
    switch (role)
    {
        case RoleKey:      return QVector3D(0.0f,     0.0f, 0.0f); // hung, facing audience
        case RoleFill:     return QVector3D(0.0f,     0.0f, 0.0f);
        case RoleBack:     return QVector3D(0.0f,   180.0f, 0.0f); // hung, facing upstage
        case RoleEffect:   return QVector3D(0.0f,     0.0f, 0.0f);
        // Battens sit on the TOP face of the truss washing upwards, so the mesh
        // (which loads head-down for hanging) is flipped upright.
        case RoleStrip:    return QVector3D(180.0f,   0.0f, 0.0f);
        case RoleSide:     return QVector3D(0.0f,    90.0f, 0.0f); // hung, aimed inward

        case RoleBlinder:
        {
            // Both the horizontal front row and the side columns face the
            // audience — they are audience blinders, not side light, so no pan.
            //
            // The units that did NOT fit across the front truss go on the side
            // columns, where a long strobe bar has to be ROLLED 90° (Z axis) so
            // it stands vertically on the upright instead of sticking out
            // sideways into the wings. Uses the same split as computePosition().
            const float gx  = gridM.x() * 1000.0f;
            const float fxW = fxSizeMM.x();
            const float minGap = fxW + kMinFixtureGapMM;
            const float endPad = qMax(gx * 0.06f, fxW);
            const float span   = (gx - endPad) - endPad;
            const int   frontCap = qMax(1, int(span / minGap));

            int frontCount = blinderFrontCount(total, frontCap);

            // Front row: horizontal, as the bar is built.
            if (index < frontCount)
                return QVector3D(0.0f, 0.0f, 0.0f);

            // Side column: rolled upright.
            return QVector3D(0.0f, 0.0f, 90.0f);
        }

        case RoleFloor:
            // Standing on the deck at the back of the stage (see
            // computePosition()), aimed forward across it towards the audience.
            //
            // The mesh loads head-DOWN, so X = 180 stands it upright (the same
            // value RoleHazer and RoleStrip use to sit a unit on the deck /
            // truss top). Backing off by 35 degrees rakes the beam downstage
            // across the performers instead of firing straight into the roof.
            //
            // Sign: MainView3D::updateFixtureRotation() NEGATES the angles
            // (fromAxesAndAngles(..., -degrees.x(), ...)), and Z runs rear ->
            // front, so a value ABOVE 180 is what tips the beam towards the
            // audience. 215 = upright (180) + 35 downstage.
            return QVector3D(215.0f, 0.0f, 0.0f);

        case RoleHazer:    return QVector3D(180.0f,   0.0f, 0.0f); // upright on floor
        default:           return QVector3D(0.0f,     0.0f, 0.0f);
    }
}

