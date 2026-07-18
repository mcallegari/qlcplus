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
            quint32 itemID = FixtureUtils::fixtureItemID(fxID, head, 0);

            // Fixture size in millimetres. Prefer the ACTUAL loaded mesh extents
            // from the 3D view (this is what MainView3D uses to place/snap the
            // item); fall back to the declared physical size, then a default.
            QVector3D fxSize(300.0f, 300.0f, 300.0f);
            Fixture *fx = m_doc->fixture(fxID);
            if (fx && fx->fixtureMode())
            {
                QLCPhysical phy = fx->fixtureMode()->physical();
                if (phy.width() > 0)  fxSize.setX(phy.width());
                if (phy.height() > 0) fxSize.setY(phy.height());
                if (phy.depth() > 0)  fxSize.setZ(phy.depth());
            }

            MainView3D *view3D = m_contextManager ? m_contextManager->get3DView() : nullptr;
            if (view3D)
            {
                QVector3D ext = view3D->fixtureExtents(itemID); // metres
                if (ext.x() > 0.0f) fxSize.setX(ext.x() * 1000.0f);
                if (ext.y() > 0.0f) fxSize.setY(ext.y() * 1000.0f);
                if (ext.z() > 0.0f) fxSize.setZ(ext.z() * 1000.0f);
            }

            QVector3D pos = computePosition(i, total, grp.role, m_envSize, fxSize);
            QVector3D rot = computeRotation(grp.role, i, total);

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
    // Grid size in millimetres (monitor units)
    const float gx = gridM.x() * 1000.0f;
    const float gy = gridM.y() * 1000.0f;
    const float gz = gridM.z() * 1000.0f;

    // Fixture dimensions in mm
    const float fxW = fxSizeMM.x();
    const float fxH = fxSizeMM.y();
    const float fxD = fxSizeMM.z();

    // Truss bar half-thickness, matching the 3D view (trussHalfSize = 0.15 m)
    const float trussHalf = 150.0f;

    // Snap under the top truss: the fixture top touches the truss underside.
    // (worldY = pos.y/1000 + meshExtY/2, box top = pos.y + fxH; truss underside
    // is worldY = gy, so pos.y = gy - fxH.)
    const float hangY = gy - fxH;

    // Snap to the front / rear truss lines, accounting for the render offset
    // (worldZ = pos.z/1000 - gz/2 + fxD/2).
    const float frontZ = gz + trussHalf - fxD / 2.0f;   // front truss line
    const float rearZ  = -trussHalf + fxD / 2.0f;       // rear truss line

    // Evenly spread the group along the span, centred on the span midpoint.
    // The -fxW/2 term cancels the X render offset (worldX = pos.x/1000 - gx/2
    // + meshExtX/2) so the group ends up centred on the truss.
    auto spread = [&](float from, float to) -> float
    {
        float centre = (total <= 1)
                       ? (from + to) * 0.5f
                       : from + (to - from) * ((index + 0.5f) / float(total));
        return centre - fxW / 2.0f;
    };

    const float endPad = qMax(gx * 0.06f, fxW);  // clear the truss corners
    const float xFrom  = endPad;
    const float xTo    = gx - endPad;

    switch (role)
    {
        case RoleKey:
            // Front truss, hung overhead, spread across the full width
            return QVector3D(spread(xFrom, xTo), hangY, frontZ);

        case RoleFill:
            // Overhead, one step upstage of the key light row
            return QVector3D(spread(xFrom, xTo), hangY, gz * 0.72f + fxD / 2.0f);

        case RoleBack:
            // Rear truss, overhead
            return QVector3D(spread(xFrom, xTo), hangY, rearZ);

        case RoleEffect:
            // Top mid truss, over the centre line
            return QVector3D(spread(xFrom, xTo), hangY, gz * 0.5f);

        case RoleStrip:
            // Full-width batten on the front truss
            return QVector3D(spread(fxW, gx - fxW), hangY, frontZ);

        case RoleBlinder:
        {
            // Audience blinders: on the FRONT truss facing the audience. If more
            // blinders than fit across the front truss, the overflow goes on the
            // side trusses, always on the front (audience) side.
            float usable = (gx - 2.0f * endPad);
            int frontCap = qMax(1, int(usable / qMax(fxW * 1.2f, 1.0f)));
            frontCap = qMin(frontCap, total);

            if (index < frontCap)
            {
                // Evenly spread across the front truss.
                float x = (frontCap <= 1) ? gx * 0.5f
                        : xFrom + (xTo - xFrom) * (index / float(frontCap - 1));
                return QVector3D(x - fxW / 2.0f, hangY, frontZ);
            }

            // Overflow onto the side trusses, near the front (audience) end,
            // alternating left / right and stacking downstage-to-upstage.
            int ov = index - frontCap;
            bool left = (ov % 2 == 0);
            float xSide = left ? (trussHalf - fxW / 2.0f)
                               : (gx - trussHalf - fxW / 2.0f);
            int   depthIdx = ov / 2;
            // Start just behind the front edge and move upstage per pair.
            float z = gz * (0.85f - 0.15f * depthIdx);
            if (z < gz * 0.4f) z = gz * 0.4f;
            return QVector3D(xSide, hangY, z);
        }

        case RoleSide:
        {
            // Side trusses: hung from the top side-truss line (same height as the
            // other overhead roles), alternating stage-left / stage-right, spread
            // along the depth. The head faces inward (Y rot 90) so its depth
            // footprint is along X — no fxD correction on Z here.
            // Snap X to the side-truss line, with the render offset (-fxW/2).
            bool left = (index % 2 == 0);
            float xSide = left ? (trussHalf - fxW / 2.0f)
                               : (gx - trussHalf - fxW / 2.0f);
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
            // Floor level, front row, uplighting
            return QVector3D(spread(xFrom, xTo), 0.0f, gz * 0.6f);

        default:
            return QVector3D(spread(xFrom, xTo), hangY, frontZ);
    }
}

QVector3D StageWizard::computeRotation(FixtureRole role,
                                       int index, int total) const
{
    Q_UNUSED(index)
    Q_UNUSED(total)

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
        case RoleStrip:    return QVector3D(0.0f,     0.0f, 0.0f);
        case RoleBlinder:  return QVector3D(0.0f,     0.0f, 0.0f);
        case RoleSide:     return QVector3D(0.0f,    90.0f, 0.0f); // hung, aimed inward
        case RoleHazer:    return QVector3D(180.0f,   0.0f, 0.0f); // upright on floor
        case RoleFloor:    return QVector3D(180.0f,   0.0f, 0.0f); // upright, uplight
        default:           return QVector3D(0.0f,     0.0f, 0.0f);
    }
}

