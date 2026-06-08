/*
  Q Light Controller Plus
  fixtureremapmanager.cpp

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

#include <QQmlContext>
#include <QDebug>

#include "fixtureremapmanager.h"
#include "fixtureremapper.h"
#include "qlcfixturedefcache.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "qlcchannel.h"
#include "virtualconsole.h"
#include "vcwidget.h"
#include "fixture.h"
#include "app.h"
#include "doc.h"

FixtureRemapManager::FixtureRemapManager(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
    , m_targetDoc(nullptr)
    , m_pendingSourceFxId(Fixture::invalidId())
    , m_pendingSourceCh(-1)
{
    m_view->rootContext()->setContextProperty("fixtureRemapManager", this);

    m_targetDoc = new Doc(this);
    m_targetDoc->fixtureDefCache()->load(QLCFixtureDefCache::userDefinitionDirectory());
    m_targetDoc->fixtureDefCache()->loadMap(QLCFixtureDefCache::systemDefinitionDirectory());

    m_targetDoc->inputOutputMap()->removeAllUniverses();
    int index = 0;
    foreach (Universe *uni, m_doc->inputOutputMap()->universes())
    {
        m_targetDoc->inputOutputMap()->addUniverse(uni->id());
        m_targetDoc->inputOutputMap()->setUniverseName(index, uni->name());
        m_targetDoc->inputOutputMap()->startUniverses();
        index++;
    }

    rebuildSourceModel();
}

FixtureRemapManager::~FixtureRemapManager()
{
    m_view->rootContext()->setContextProperty("fixtureRemapManager", nullptr);
    delete m_targetDoc;
}

/* -------------------------------------------------------------------------
 * Property getters
 * ------------------------------------------------------------------------- */

QVariantList FixtureRemapManager::sourceFixtures() const
{
    return m_sourceModel;
}

int FixtureRemapManager::sourceItemCount() const
{
    return m_sourceModel.count();
}

QVariantList FixtureRemapManager::targetFixtures() const
{
    return m_targetModel;
}

int FixtureRemapManager::targetItemCount() const
{
    return m_targetModel.count();
}

QVariantList FixtureRemapManager::connections() const
{
    return m_connections;
}

quint32 FixtureRemapManager::pendingSourceFxId() const
{
    return m_pendingSourceFxId;
}

int FixtureRemapManager::pendingSourceCh() const
{
    return m_pendingSourceCh;
}

bool FixtureRemapManager::hasPendingSource() const
{
    return m_pendingSourceFxId != Fixture::invalidId() && m_pendingSourceCh < 0;
}

/* -------------------------------------------------------------------------
 * Private helpers
 * ------------------------------------------------------------------------- */

// static
QVariantList FixtureRemapManager::buildFlatModel(Doc *doc)
{
    QVariantList list;
    quint32 lastUniverse = UINT_MAX;

    foreach (Fixture *fxi, doc->fixtures())
    {
        // Insert a universe separator whenever the universe changes
        if (fxi->universe() != lastUniverse)
        {
            lastUniverse = fxi->universe();
            QString uniName = doc->inputOutputMap()->universes().value(fxi->universe())
                              ? doc->inputOutputMap()->universes().at(fxi->universe())->name()
                              : QString("Universe %1").arg(fxi->universe() + 1);
            QVariantMap uniEntry;
            uniEntry.insert("isHeader",   false);
            uniEntry.insert("isUniverse", true);
            uniEntry.insert("fxId",       UINT_MAX);
            uniEntry.insert("name",       uniName);
            uniEntry.insert("address",    QString());
            uniEntry.insert("universe",   uniName);
            uniEntry.insert("chIdx",      -1);
            uniEntry.insert("chIcon",     QString());
            list.append(uniEntry);
        }

        quint32 base = fxi->address();
        QVariantMap fxEntry;
        fxEntry.insert("isHeader",   true);
        fxEntry.insert("isUniverse", false);
        fxEntry.insert("fxId",       fxi->id());
        fxEntry.insert("name",       fxi->name());
        fxEntry.insert("address",    QString("%1 – %2").arg(base + 1).arg(base + fxi->channels()));
        fxEntry.insert("universe",   QString());
        fxEntry.insert("chIdx",      -1);
        fxEntry.insert("chIcon",     QString());
        list.append(fxEntry);

        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            const QLCChannel *ch = fxi->channel(c);
            QVariantMap chEntry;
            chEntry.insert("isHeader",   false);
            chEntry.insert("isUniverse", false);
            chEntry.insert("fxId",       fxi->id());
            chEntry.insert("name",       QString("%1: %2").arg(c + 1).arg(ch ? ch->name() : "?"));
            chEntry.insert("address",    QString());
            chEntry.insert("universe",   QString());
            chEntry.insert("chIdx",      (int)c);
            chEntry.insert("chIcon",     ch ? ch->getIconNameFromGroup(ch->group(), true) : QString());
            list.append(chEntry);
        }
    }

    return list;
}

void FixtureRemapManager::rebuildSourceModel()
{
    m_sourceModel = buildFlatModel(m_doc);
    emit sourceFixturesChanged();
}

void FixtureRemapManager::rebuildTargetModel()
{
    m_targetModel = buildFlatModel(m_targetDoc);
    emit targetFixturesChanged();
}

/* -------------------------------------------------------------------------
 * QML-invokable methods
 * ------------------------------------------------------------------------- */

void FixtureRemapManager::addTargetFixture(const QString &manufacturer,
                                         const QString &model,
                                         const QString &mode,
                                         const QString &name,
                                         int universe,
                                         int address,
                                         int quantity,
                                         int gap)
{
    QLCFixtureDef  *fxiDef  = m_targetDoc->fixtureDefCache()->fixtureDef(manufacturer, model);
    QLCFixtureMode *fxiMode = fxiDef ? fxiDef->mode(mode) : nullptr;

    int channels = fxiMode ? fxiMode->channels().count() : 1;
    int currentAddress = address;

    for (int i = 0; i < qMax(1, quantity); i++)
    {
        Fixture *fxi = new Fixture(m_targetDoc);
        fxi->setUniverse(universe);
        fxi->setAddress(currentAddress);

        QString fxiName = name.isEmpty() ? model : name;
        if (quantity > 1)
            fxiName = QString("%1 #%2").arg(fxiName).arg(i + 1);
        fxi->setName(fxiName);

        if (fxiDef && fxiMode)
        {
            fxi->setFixtureDefinition(fxiDef, fxiMode);
        }
        else
        {
            QLCFixtureDef  *gDef  = fxi->genericDimmerDef(1);
            QLCFixtureMode *gMode = fxi->genericDimmerMode(gDef, 1);
            fxi->setFixtureDefinition(gDef, gMode);
        }

        if (!m_targetDoc->addFixture(fxi))
        {
            qWarning() << "[FixtureRemapManager] Could not add target fixture" << fxiName;
            delete fxi;
        }

        currentAddress += channels + gap;
    }

    rebuildTargetModel();
}

bool FixtureRemapManager::cloneAndAutoRemap(quint32 srcFxiId)
{
    Fixture *srcFix = m_doc->fixture(srcFxiId);
    if (!srcFix)
        return false;

    // Check that the address range is free in the target doc
    quint32 universeAddr = srcFix->universeAddress();
    for (quint32 i = universeAddr; i < universeAddr + srcFix->channels(); i++)
    {
        if (m_targetDoc->fixtureForAddress(i) != Fixture::invalidId())
        {
            qWarning() << "[FixtureRemapManager] Clone failed: address already in use";
            return false;
        }
    }

    Fixture *tgtFix = new Fixture(m_targetDoc);
    tgtFix->setAddress(srcFix->address());
    tgtFix->setUniverse(srcFix->universe());
    tgtFix->setName(srcFix->name());

    const QLCFixtureDef *srcDef = srcFix->fixtureDef();
    if (srcDef && srcDef->manufacturer() == KXMLFixtureGeneric &&
        srcDef->model() == KXMLFixtureGeneric)
    {
        QLCFixtureDef  *gDef  = tgtFix->genericDimmerDef(srcFix->channels());
        QLCFixtureMode *gMode = tgtFix->genericDimmerMode(gDef, srcFix->channels());
        tgtFix->setFixtureDefinition(gDef, gMode);
    }
    else if (srcDef && srcFix->fixtureMode())
    {
        // Look up the definition from the target cache so ownership is correct
        QLCFixtureDef *tgtDef = m_targetDoc->fixtureDefCache()->fixtureDef(
            srcDef->manufacturer(), srcDef->model());
        QLCFixtureMode *tgtMode = tgtDef ? tgtDef->mode(srcFix->fixtureMode()->name()) : nullptr;
        if (tgtDef && tgtMode)
            tgtFix->setFixtureDefinition(tgtDef, tgtMode);
        else
        {
            qWarning() << "[FixtureRemapManager] Clone: definition not found in target cache";
            delete tgtFix;
            return false;
        }
    }

    if (!m_targetDoc->addFixture(tgtFix))
    {
        delete tgtFix;
        return false;
    }

    rebuildTargetModel();
    autoConnect(srcFxiId, tgtFix->id());
    return true;
}

void FixtureRemapManager::removeTargetFixture(quint32 fxiId)
{
    removeConnectionsForFixture(fxiId);
    m_targetDoc->deleteFixture(fxiId);
    rebuildTargetModel();
}

int FixtureRemapManager::autoConnect(quint32 srcFxiId, quint32 tgtFxiId)
{
    Fixture *srcFxi = m_doc->fixture(srcFxiId);
    Fixture *tgtFxi = m_targetDoc->fixture(tgtFxiId);
    if (!srcFxi || !tgtFxi)
        return 0;

    // Remove only previous connections whose SOURCE matches srcFxiId.
    // Do NOT remove by target ID: source and target docs assign IDs
    // independently starting from 0, so a target fixture ID can equal
    // a source fixture ID numerically without being the same fixture.
    removeConnectionsForSourceFixture(srcFxiId);

    QList<QPair<quint32, quint32>> pairs = m_remapper.autoConnectFixtures(srcFxi, tgtFxi);

    for (const QPair<quint32, quint32> &p : pairs)
    {
        QVariantMap conn;
        conn.insert("srcFxId", srcFxiId);
        conn.insert("srcCh",   p.first);
        conn.insert("tgtFxId", tgtFxiId);
        conn.insert("tgtCh",   p.second);
        m_connections.append(conn);
    }

    emit connectionsChanged();
    return pairs.count();
}

void FixtureRemapManager::setPendingSource(quint32 fxiId, int chIdx)
{
    m_pendingSourceFxId = fxiId;
    m_pendingSourceCh   = chIdx;
    emit pendingSourceChanged();
}

void FixtureRemapManager::connectToTarget(quint32 tgtFxiId, int tgtCh)
{
    if (m_pendingSourceFxId == Fixture::invalidId() || m_pendingSourceCh < 0)
        return;

    addConnection(m_pendingSourceFxId, (quint32)m_pendingSourceCh, tgtFxiId, (quint32)tgtCh);

    m_pendingSourceFxId = Fixture::invalidId();
    m_pendingSourceCh   = -1;
    emit pendingSourceChanged();
}

void FixtureRemapManager::connectFixtureToFixture(quint32 tgtFxiId)
{
    if (m_pendingSourceFxId == Fixture::invalidId())
        return;

    autoConnect(m_pendingSourceFxId, tgtFxiId);

    m_pendingSourceFxId = Fixture::invalidId();
    m_pendingSourceCh   = -1;
    emit pendingSourceChanged();
}

void FixtureRemapManager::addConnection(quint32 srcFxiId, quint32 srcCh,
                                      quint32 tgtFxiId, quint32 tgtCh)
{
    // Avoid exact duplicates
    for (const QVariant &v : m_connections)
    {
        QVariantMap c = v.toMap();
        if (c.value("srcFxId").toUInt() == srcFxiId &&
            c.value("srcCh").toUInt()   == srcCh    &&
            c.value("tgtFxId").toUInt() == tgtFxiId &&
            c.value("tgtCh").toUInt()   == tgtCh)
            return;
    }

    m_remapper.addChannelRemap(srcFxiId, srcCh, tgtFxiId, tgtCh);

    QVariantMap conn;
    conn.insert("srcFxId", srcFxiId);
    conn.insert("srcCh",   srcCh);
    conn.insert("tgtFxId", tgtFxiId);
    conn.insert("tgtCh",   tgtCh);
    m_connections.append(conn);

    emit connectionsChanged();
}

void FixtureRemapManager::removeConnection(int idx)
{
    if (idx < 0 || idx >= m_connections.count())
        return;

    // Rebuild the remapper lists from scratch (simplest approach)
    m_connections.removeAt(idx);
    m_remapper.reset();
    for (const QVariant &v : m_connections)
    {
        QVariantMap c = v.toMap();
        m_remapper.addChannelRemap(c.value("srcFxId").toUInt(),
                                   c.value("srcCh").toUInt(),
                                   c.value("tgtFxId").toUInt(),
                                   c.value("tgtCh").toUInt());
    }

    emit connectionsChanged();
}

void FixtureRemapManager::removeConnectionsForSourceFixture(quint32 srcFxiId)
{
    bool changed = false;
    for (int i = m_connections.count() - 1; i >= 0; i--)
    {
        if (m_connections.at(i).toMap().value("srcFxId").toUInt() == srcFxiId)
        {
            m_connections.removeAt(i);
            changed = true;
        }
    }

    if (changed)
    {
        m_remapper.reset();
        for (const QVariant &v : m_connections)
        {
            QVariantMap c = v.toMap();
            m_remapper.addChannelRemap(c.value("srcFxId").toUInt(),
                                       c.value("srcCh").toUInt(),
                                       c.value("tgtFxId").toUInt(),
                                       c.value("tgtCh").toUInt());
        }
        emit connectionsChanged();
    }
}

void FixtureRemapManager::removeConnectionsForFixture(quint32 fxiId)
{
    // Called when a TARGET fixture is removed from the right panel.
    // Only match the target side — source and target IDs are independent
    // numeric spaces (both start from 0 in their respective Doc instances).
    bool changed = false;
    for (int i = m_connections.count() - 1; i >= 0; i--)
    {
        QVariantMap c = m_connections.at(i).toMap();
        if (c.value("tgtFxId").toUInt() == fxiId)
        {
            m_connections.removeAt(i);
            changed = true;
        }
    }

    if (changed)
    {
        // Rebuild remapper lists
        m_remapper.reset();
        for (const QVariant &v : m_connections)
        {
            QVariantMap c = v.toMap();
            m_remapper.addChannelRemap(c.value("srcFxId").toUInt(),
                                       c.value("srcCh").toUInt(),
                                       c.value("tgtFxId").toUInt(),
                                       c.value("tgtCh").toUInt());
        }
        emit connectionsChanged();
    }
}


void FixtureRemapManager::applyRemap()
{
    if (m_targetDoc->fixtures().isEmpty())
    {
        qWarning() << "[FixtureRemapManager] No target fixtures defined — nothing to apply.";
        return;
    }

    const QList<SceneValue> &srcList = m_remapper.sourceList();
    const QList<SceneValue> &tgtList = m_remapper.targetList();

    QMap<SceneValue, SceneValue> channelRemap;
    for (int i = 0; i < srcList.count(); i++)
        channelRemap.insert(srcList.at(i), tgtList.at(i));

    m_remapper.applyRemap(m_doc, m_targetDoc->fixtures());

    /* Remap Virtual Console widgets */
    VirtualConsole *vc = qobject_cast<App *>(m_view)->virtualConsole();
    if (vc != nullptr)
    {
        QVariantList widgets = vc->widgetsList();
        for (const QVariant &vWidget : widgets)
        {
            VCWidget *widget = vWidget.toMap().value("classRef").value<VCWidget *>();
            if (widget != nullptr)
                widget->remapChannels(channelRemap);
        }
    }

    emit remapApplied();
    reset();
}

void FixtureRemapManager::reset()
{
    m_remapper.reset();
    m_connections.clear();

    // Re-create target doc
    delete m_targetDoc;
    m_targetDoc = new Doc(this);
    m_targetDoc->fixtureDefCache()->load(QLCFixtureDefCache::userDefinitionDirectory());
    m_targetDoc->fixtureDefCache()->loadMap(QLCFixtureDefCache::systemDefinitionDirectory());

    m_targetDoc->inputOutputMap()->removeAllUniverses();
    int index = 0;
    foreach (Universe *uni, m_doc->inputOutputMap()->universes())
    {
        m_targetDoc->inputOutputMap()->addUniverse(uni->id());
        m_targetDoc->inputOutputMap()->setUniverseName(index, uni->name());
        m_targetDoc->inputOutputMap()->startUniverses();
        index++;
    }

    m_pendingSourceFxId = Fixture::invalidId();
    m_pendingSourceCh   = -1;

    rebuildSourceModel();
    rebuildTargetModel();

    emit connectionsChanged();
    emit pendingSourceChanged();
}
