/*
  Q Light Controller Plus
  fixtureremapper.h

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

#ifndef FIXTUREREMAPPER_H
#define FIXTUREREMAPPER_H

#include <QPair>
#include <QList>

#include "scenevalue.h"

class Doc;
class EFX;
class Fixture;

/** @addtogroup engine
 * @{
 */

/**
 * FixtureRemapper holds the engine-side logic for remapping fixtures in a
 * QLC+ project.  It is shared between the classic widget-based UI and the
 * QML UI so the core algorithm lives in exactly one place.
 *
 * Workflow:
 *  1. Call autoConnectFixtures() for each source→target fixture pair to
 *     populate the internal channel-mapping tables via semantic matching.
 *     Individual channel overrides can be added with addChannelRemap().
 *  2. Call applyRemap() to commit the remap to a Doc instance:
 *     fixtures are replaced, then fixture groups, channel groups, functions
 *     (Scene / Sequence / EFX) and monitor properties are all updated.
 *  3. The caller is responsible for remapping Virtual Console widgets using
 *     the sourceList() / targetList() accessors together with
 *     remapSceneValues().
 */
class FixtureRemapper final
{
public:
    FixtureRemapper();

    /** Clear all accumulated channel mappings. */
    void reset();

    /**
     * Add a single channel-level mapping entry.
     * Both (srcFxiId, srcCh) and (tgtFxiId, tgtCh) are appended to the
     * respective lookup lists used by remapSceneValues() and applyRemap().
     */
    void addChannelRemap(quint32 srcFxiId, quint32 srcCh,
                         quint32 tgtFxiId, quint32 tgtCh);

    /**
     * Automatically compute channel-level mappings between @p src and @p tgt.
     *
     * Two strategies are applied:
     *  - 1:1 mapping when both fixtures share the same definition+mode (or
     *    both are generic dimmers): channel index s maps to channel index s.
     *  - Semantic matching otherwise: each source channel is matched to the
     *    first target channel that shares the same group, controlByte, and
     *    (for Intensity channels) colour.
     *
     * As a side-effect the method copies fade-capability flags and channel
     * modifiers from @p src to @p tgt for each matched pair, and copies
     * forced-HTP/LTP channel lists when a 1:1 remap applies.
     *
     * @return List of matched (srcChannelIndex, tgtChannelIndex) pairs so
     *         the caller can synchronise its own UI-level data structures.
     */
    QList<QPair<quint32, quint32>> autoConnectFixtures(Fixture *src, Fixture *tgt);

    /** Read-only access to the source lookup table built up by
     *  addChannelRemap() / autoConnectFixtures(). */
    const QList<SceneValue>& sourceList() const;

    /** Read-only access to the target lookup table. */
    const QList<SceneValue>& targetList() const;

    /**
     * Apply the accumulated channel mappings to @p doc:
     *  1. Replace fixtures with @p targetFixtures (calls Doc::replaceFixtures).
     *  2. Remap fixture groups.
     *  3. Remap channel groups.
     *  4. Remap functions (Scene, Sequence, EFX).
     *  5. Remap monitor properties.
     *
     * Virtual Console widget remapping is intentionally left to the caller
     * because VC classes live in the UI layer, not in the engine.
     */
    void applyRemap(Doc *doc, const QList<Fixture *> &targetFixtures);

    /**
     * Translate a list of SceneValues from source to target fixture/channel
     * addresses using the provided lookup tables.  Unmapped values are
     * silently dropped.  The result is sorted.
     */
    static QList<SceneValue> remapSceneValues(const QList<SceneValue> &funcList,
                                              const QList<SceneValue> &srcList,
                                              const QList<SceneValue> &tgtList);

private:
    void remapEFX(EFX *efx, Doc *doc);

    QList<SceneValue> m_sourceList;
    QList<SceneValue> m_targetList;
};

/** @} */

#endif // FIXTUREREMAPPER_H
