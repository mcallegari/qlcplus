/*
  Q Light Controller Plus
  fixtureremap.cpp

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

#include "fixtureremap.h"

#include <QDebug>

#include "qlcfixturedefcache.h"
#include "qlcfixturedef.h"
#include "qlcfixturemode.h"
#include "qlccapability.h"
#include "qlcchannel.h"
#include "fixture.h"
#include "scene.h"
#include "doc.h"

#include "monitorproperties.h"

FixtureRemap::FixtureRemap(Doc *doc)
    : m_doc(doc)
{
}

QMap<SceneValue, SceneValue> FixtureRemap::replaceProfiles(QList<quint32> fixtureIDs, const QString& manufacturer,
                                                            const QString& model, const QString& mode)
{
    QMap<SceneValue, SceneValue> channelRemap;

    if (fixtureIDs.isEmpty())
        return channelRemap;

    QLCFixtureDef *newDef = m_doc->fixtureDefCache()->fixtureDef(manufacturer, model);
    if (newDef == nullptr)
    {
        qWarning() << "[FixtureRemap] Cannot find fixture definition for" << manufacturer << model;
        return channelRemap;
    }
    QLCFixtureMode *newMode = newDef->mode(mode);
    if (newMode == nullptr)
    {
        qWarning() << "[FixtureRemap] Cannot find mode" << mode << "for definition" << manufacturer << model;
        return channelRemap;
    }

    struct FxPatch { quint32 id; quint32 addr; quint32 uni; };
    QList<FxPatch> backup;

    // 1. "Park" all fixtures in an imaginary universe to avoid overlaps/crashes during definition change
    for (int i = 0; i < fixtureIDs.count(); i++)
    {
        quint32 fxID = fixtureIDs.at(i);
        Fixture *oldFix = m_doc->fixture(fxID);
        if (oldFix == nullptr) continue;

        backup.append({fxID, oldFix->address(), oldFix->universe()});
        // Use a safe universe range (100+) to avoid conflicting with existing fixtures
        // and avoid extremely high values that might cause issues.
        oldFix->setUniverse(100 + i);
        oldFix->setAddress(0);
    }

    QMap<quint32, quint32> fixtureRemap;

    // 2. Build channel remapping and apply new definitions
    for (const FxPatch& patch : backup)
    {
        Fixture *fxi = m_doc->fixture(patch.id);
        if (fxi == nullptr) continue;

        // Build channel remapping for this fixture
        for (quint32 s = 0; s < fxi->channels(); s++)
        {
            const QLCChannel *srcCh = fxi->channel(s);
            if (srcCh == nullptr) continue;

            for (int t = 0; t < newMode->channels().count(); t++)
            {
                const QLCChannel *tgtCh = newMode->channel(t);

                if ((tgtCh->group() == srcCh->group()) &&
                    (tgtCh->controlByte() == srcCh->controlByte()))
                {
                    if (tgtCh->group() == QLCChannel::Intensity &&
                        tgtCh->colour() != srcCh->colour())
                            continue;

                    channelRemap[SceneValue(patch.id, s)] = SceneValue(patch.id, (quint32)t);
                    break;
                }
            }
        }

        fxi->setFixtureDefinition(newDef, newMode);
        fixtureRemap[patch.id] = patch.id;
    }

    // 3. Safe Repatch: try to restore original address or find next available
    for (const FxPatch& patch : backup)
    {
        Fixture *fxi = m_doc->fixture(patch.id);
        if (fxi == nullptr) continue;

        int requested = (int)patch.addr;
        int channels = (int)fxi->channels();
        bool found = false;

        // Ensure we find a place where this fixture fits without overlaps
        while (requested <= 512 - channels)
        {
            bool free = true;
            for (int i = 0; i < channels; i++)
            {
                // Check if address is free (ignoring current fixture because it's parked)
                if (m_doc->fixtureForAddress((patch.uni << 9) | (quint32)(requested + i)) != Fixture::invalidId())
                {
                    free = false;
                    break;
                }
            }

            if (free)
            {
                // Set address first while still in parked universe
                fxi->setAddress((quint32)requested);
                // Then move back to target universe
                fxi->setUniverse(patch.uni);
                found = true;
                break;
            }
            requested++;
        }

        if (!found)
        {
            qWarning() << "[FixtureRemap] Could not find a free address for fixture" << patch.id << "starting from" << patch.addr << ". Keeping it parked.";
        }
    }

    // 4. Remap Scenes
    remapScenes(channelRemap);

    // 5. Remap Monitor Properties
    remapMonitorProperties(fixtureRemap);

    m_doc->setModified();

    return channelRemap;
}

void FixtureRemap::remapScenes(const QMap<SceneValue, SceneValue>& remapMap)
{
    for (Function *func : m_doc->functions())
    {
        if (func->type() == Function::SceneType)
        {
            Scene *scene = qobject_cast<Scene*>(func);
            QList<SceneValue> oldValues = scene->values();

            for (const SceneValue& val : oldValues)
            {
                SceneValue key(val.fxi, val.channel);
                if (remapMap.contains(key))
                {
                    SceneValue newVal = remapMap.value(key);
                    newVal.value = val.value;
                    if (newVal.channel != val.channel)
                    {
                        scene->unsetValue(val.fxi, val.channel);
                        scene->setValue(newVal);
                    }
                }
                else
                {
                    // If the fixture is part of the remapping but this channel has no match, unset it
                    bool fixtureFound = false;
                    QMapIterator<SceneValue, SceneValue> it(remapMap);
                    while (it.hasNext()) {
                        it.next();
                        if (it.key().fxi == val.fxi) {
                            fixtureFound = true;
                            break;
                        }
                    }
                    if (fixtureFound)
                        scene->unsetValue(val.fxi, val.channel);
                }
            }
        }
    }
}

void FixtureRemap::remapMonitorProperties(const QMap<quint32, quint32>& fixtureRemapMap)
{
    Q_UNUSED(fixtureRemapMap);
    // In-place replacement doesn't need to change monitor positions as IDs are the same
}
