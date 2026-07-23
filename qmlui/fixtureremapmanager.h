/*
  Q Light Controller Plus
  fixtureremapmanager.h

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

#ifndef FIXTUREREMAPMANAGER_H
#define FIXTUREREMAPMANAGER_H

#include <QObject>
#include <QVariant>
#include <QQuickView>

#include "fixtureremapper.h"

class Doc;
class Fixture;

/** @addtogroup qmlui QML UI
 * @{
 */

/**
 * FixtureRemapManager is the QML-facing backend for the fixture remap tool.
 *
 * It owns a temporary target Doc, exposes flat list models for the source
 * and target fixture/channel trees, manages the channel-level connection
 * table used to draw the bezier curves, and ultimately delegates the apply
 * step to the engine-level FixtureRemapManagerper.
 *
 * Registered as QML context property "FixtureRemapManager".
 */
class FixtureRemapManager final : public QObject
{
    Q_OBJECT

    /** Flat list model for the left (source) tree.
     *  Each entry is a QVariantMap with keys:
     *    "isHeader"    bool   — true for fixture rows, false for channel rows
     *    "fxId"        uint   — fixture ID
     *    "name"        string — fixture or channel name
     *    "address"     string — "start – end" (fixture rows only)
     *    "universe"    string — universe name (fixture rows only)
     *    "chIdx"       int    — channel index within the fixture (-1 for fixture rows)
     *    "chIcon"      string — channel icon resource (channel rows only)
     */
    Q_PROPERTY(QVariantList sourceFixtures READ sourceFixtures NOTIFY sourceFixturesChanged)
    Q_PROPERTY(int sourceItemCount READ sourceItemCount NOTIFY sourceFixturesChanged)

    /** Flat list model for the right (target) tree, same structure. */
    Q_PROPERTY(QVariantList targetFixtures READ targetFixtures NOTIFY targetFixturesChanged)
    Q_PROPERTY(int targetItemCount READ targetItemCount NOTIFY targetFixturesChanged)

    /** List of active connections for bezier curve rendering.
     *  Each entry is a QVariantMap with keys:
     *    "srcFxId"  uint — source fixture ID
     *    "srcCh"    uint — source channel index
     *    "tgtFxId"  uint — target fixture ID
     *    "tgtCh"    uint — target channel index
     */
    Q_PROPERTY(QVariantList connections READ connections NOTIFY connectionsChanged)

    /** Pending source selection while the user is clicking to create a connection. */
    Q_PROPERTY(quint32 pendingSourceFxId READ pendingSourceFxId NOTIFY pendingSourceChanged)
    Q_PROPERTY(int pendingSourceCh READ pendingSourceCh NOTIFY pendingSourceChanged)

    /** True when a fixture-level (not channel-level) source is pending.
     *  In this state clicking a target fixture header auto-connects all channels. */
    Q_PROPERTY(bool hasPendingSource READ hasPendingSource NOTIFY pendingSourceChanged)

public:
    explicit FixtureRemapManager(QQuickView *view, Doc *doc, QObject *parent = nullptr);
    ~FixtureRemapManager() override;

    // ---- property getters ----
    QVariantList sourceFixtures() const;
    int sourceItemCount() const;
    QVariantList targetFixtures() const;
    int targetItemCount() const;
    QVariantList connections() const;
    quint32 pendingSourceFxId() const;
    int pendingSourceCh() const;
    bool hasPendingSource() const;

    // ---- QML-invokable methods ----

    /** Add one or more fixtures to the target doc.
     *  @p universe and @p address are 0-based.
     *  When @p quantity > 1 each fixture is placed @p gap channels apart. */
    Q_INVOKABLE void addTargetFixture(const QString &manufacturer, const QString &model,
                                      const QString &mode, const QString &name,
                                      int universe, int address,
                                      int quantity = 1, int gap = 0);

    /** Remove a target fixture and all its connections. */
    Q_INVOKABLE void removeTargetFixture(quint32 fxiId);

    /** Automatically create channel-level connections between srcFxiId and
     *  tgtFxiId using semantic matching.
     *  Returns the number of connections created. */
    Q_INVOKABLE int autoConnect(quint32 srcFxiId, quint32 tgtFxiId);

    /** Set the pending source for the next click-to-connect action.
     *  Call with chIdx = -1 to clear. */
    Q_INVOKABLE void setPendingSource(quint32 fxiId, int chIdx);

    /** Complete a click-to-connect: link pending source to tgtFxiId/tgtCh.
     *  Clears pending source on success. */
    Q_INVOKABLE void connectToTarget(quint32 tgtFxiId, int tgtCh);

    /** Auto-connect the pending source fixture to @p tgtFxiId using semantic
     *  channel matching.  Only valid when hasPendingSource() is true.
     *  Clears pending source on completion. */
    Q_INVOKABLE void connectFixtureToFixture(quint32 tgtFxiId);

    /** Manually add a single channel connection. */
    Q_INVOKABLE void addConnection(quint32 srcFxiId, quint32 srcCh,
                                   quint32 tgtFxiId, quint32 tgtCh);

    /** Remove the connection at index @p idx in the connections list. */
    Q_INVOKABLE void removeConnection(int idx);

    /** Remove all connections where the TARGET fixture is @p fxiId.
     *  Called when a target fixture is deleted from the right panel. */
    Q_INVOKABLE void removeConnectionsForFixture(quint32 fxiId);

    /** Clone the source fixture @p srcFxiId into the target doc (same address/universe/
     *  definition) and immediately auto-connect all its channels.
     *  Does nothing if the address range is already occupied in the target doc. */
    Q_INVOKABLE bool cloneAndAutoRemap(quint32 srcFxiId);

    /** Apply the remap to the project (engine remapper + save). */
    Q_INVOKABLE void applyRemap();

    /** Discard all target fixtures and connections. */
    Q_INVOKABLE void reset();

signals:
    void sourceFixturesChanged();
    void targetFixturesChanged();
    void connectionsChanged();
    void pendingSourceChanged();

    /** Emitted when applyRemap() succeeds or reset() is called from the view,
     *  so FixturesAndFunctions.qml can switch back to the previous view. */
    void remapApplied();

private:
    void removeConnectionsForSourceFixture(quint32 srcFxiId);
    /** Rebuild m_sourceModel from m_doc. */
    void rebuildSourceModel();
    /** Rebuild m_targetModel from m_targetDoc. */
    void rebuildTargetModel();
    /** Build a flat entry list from the fixtures in @p doc. */
    static QVariantList buildFlatModel(Doc *doc);

    QQuickView *m_view;
    Doc        *m_doc;
    Doc        *m_targetDoc;

    FixtureRemapper m_remapper;

    QVariantList m_sourceModel;
    QVariantList m_targetModel;

    /** Connection list mirrors m_remapper source/target lists as QVariantMaps. */
    QVariantList m_connections;

    quint32 m_pendingSourceFxId;
    int     m_pendingSourceCh;
};

/** @} */

#endif // FIXTUREREMAPMANAGER_H
