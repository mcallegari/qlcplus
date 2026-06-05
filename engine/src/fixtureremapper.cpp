/*
  Q Light Controller Plus
  fixtureremapper.cpp

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

#include <QMapIterator>
#include <algorithm>
#include <QDebug>

#include "fixtureremapper.h"
#include "monitorproperties.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "channelmodifier.h"
#include "channelsgroup.h"
#include "fixturegroup.h"
#include "qlcchannel.h"
#include "efxfixture.h"
#include "chaserstep.h"
#include "grouphead.h"
#include "sequence.h"
#include "fixture.h"
#include "scene.h"
#include "efx.h"
#include "doc.h"

FixtureRemapper::FixtureRemapper()
{
}

void FixtureRemapper::reset()
{
    m_sourceList.clear();
    m_targetList.clear();
}

void FixtureRemapper::addChannelRemap(quint32 srcFxiId, quint32 srcCh,
                                      quint32 tgtFxiId, quint32 tgtCh)
{
    m_sourceList.append(SceneValue(srcFxiId, srcCh));
    m_targetList.append(SceneValue(tgtFxiId, tgtCh));
}

QList<QPair<quint32, quint32>> FixtureRemapper::autoConnectFixtures(Fixture *src, Fixture *tgt)
{
    QList<QPair<quint32, quint32>> channelPairs;
    if (src == nullptr || tgt == nullptr)
        return channelPairs;

    const QLCFixtureDef *srcDef = src->fixtureDef();
    const QLCFixtureDef *tgtDef = tgt->fixtureDef();
    const QLCFixtureMode *srcMode = src->fixtureMode();
    const QLCFixtureMode *tgtMode = tgt->fixtureMode();

    bool oneToOne = false;

    // Same definition and mode → direct index mapping
    if (srcDef != nullptr && tgtDef != nullptr &&
        srcMode != nullptr && tgtMode != nullptr &&
        srcDef->name() == tgtDef->name() &&
        srcMode->name() == tgtMode->name())
    {
        oneToOne = true;
    }
    // Both are untyped generic dimmers → direct index mapping
    else if (srcDef == nullptr && tgtDef == nullptr &&
             srcMode == nullptr && tgtMode == nullptr)
    {
        oneToOne = true;
    }

    if (oneToOne)
    {
        tgt->setForcedHTPChannels(src->forcedHTPChannels());
        tgt->setForcedLTPChannels(src->forcedLTPChannels());
    }

    for (quint32 s = 0; s < src->channels(); s++)
    {
        if (oneToOne)
        {
            if (s < tgt->channels())
            {
                addChannelRemap(src->id(), s, tgt->id(), s);
                channelPairs.append(qMakePair(s, s));

                if (!src->channelCanFade(s))
                    tgt->setChannelCanFade(s, false);

                ChannelModifier *chMod = src->channelModifier(s);
                if (chMod != nullptr)
                    tgt->setChannelModifier(s, chMod);
            }
        }
        else
        {
            const QLCChannel *srcCh = src->channel(s);
            if (srcCh == nullptr)
                continue;

            for (quint32 t = 0; t < tgt->channels(); t++)
            {
                const QLCChannel *tgtCh = tgt->channel(t);
                if (tgtCh == nullptr)
                    continue;

                if (tgtCh->group() != srcCh->group())
                    continue;
                if (tgtCh->controlByte() != srcCh->controlByte())
                    continue;
                if (tgtCh->group() == QLCChannel::Intensity &&
                    tgtCh->colour() != srcCh->colour())
                    continue;

                addChannelRemap(src->id(), s, tgt->id(), t);
                channelPairs.append(qMakePair(s, t));

                if (!src->channelCanFade(s))
                    tgt->setChannelCanFade(t, false);

                break;
            }
        }
    }

    return channelPairs;
}

const QList<SceneValue>& FixtureRemapper::sourceList() const
{
    return m_sourceList;
}

const QList<SceneValue>& FixtureRemapper::targetList() const
{
    return m_targetList;
}

// static
QList<SceneValue> FixtureRemapper::remapSceneValues(const QList<SceneValue> &funcList,
                                                    const QList<SceneValue> &srcList,
                                                    const QList<SceneValue> &tgtList)
{
    QList<SceneValue> newValuesList;
    foreach (SceneValue val, funcList)
    {
        for (int v = 0; v < srcList.count(); v++)
        {
            if (val == srcList.at(v))
            {
                SceneValue tgtVal = tgtList.at(v);
                newValuesList.append(SceneValue(tgtVal.fxi, tgtVal.channel, val.value));
            }
        }
    }
    std::sort(newValuesList.begin(), newValuesList.end());
    return newValuesList;
}

void FixtureRemapper::remapEFX(EFX *efx, Doc *doc)
{
    QList<EFXFixture*> fixListCopy;
    foreach (EFXFixture *efxFix, efx->fixtures())
    {
        EFXFixture *ef = new EFXFixture(efx);
        ef->copyFrom(efxFix);
        fixListCopy.append(ef);
    }

    efx->removeAllFixtures();
    QList<quint32> remappedFixtures;

    foreach (EFXFixture *efxFix, fixListCopy)
    {
        quint32 fxID = efxFix->head().fxi;
        for (int i = 0; i < m_sourceList.count(); i++)
        {
            const SceneValue &srcVal = m_sourceList.at(i);
            const SceneValue &tgtVal = m_targetList.at(i);

            if (srcVal.fxi != fxID)
                continue;
            if (remappedFixtures.contains(tgtVal.fxi))
                continue;

            Fixture *docFix = doc->fixture(tgtVal.fxi);
            if (docFix == nullptr)
                continue;

            const QLCChannel *chan = docFix->channel(tgtVal.channel);
            if (chan == nullptr)
                continue;

            if (chan->group() == QLCChannel::Pan ||
                chan->group() == QLCChannel::Tilt)
            {
                EFXFixture *ef = new EFXFixture(efx);
                ef->copyFrom(efxFix);
                ef->setHead(GroupHead(tgtVal.fxi, 0));
                if (!efx->addFixture(ef))
                    delete ef;
                remappedFixtures.append(tgtVal.fxi);
            }
        }
    }

    qDeleteAll(fixListCopy);
}

void FixtureRemapper::applyRemap(Doc *doc, const QList<Fixture *> &targetFixtures)
{
    Q_ASSERT(doc != nullptr);

    // 1 – Replace fixtures in the document
    doc->replaceFixtures(targetFixtures);

    // 2 – Remap fixture groups
    foreach (FixtureGroup *group, doc->fixtureGroups())
    {
        QMap<QLCPoint, GroupHead> grpHash = group->headsMap();
        group->reset();

        QMapIterator<QLCPoint, GroupHead> it(grpHash);
        while (it.hasNext())
        {
            it.next();
            QLCPoint pt(it.key());
            GroupHead head(it.value());

            if (!head.isValid())
                continue;

            for (int i = 0; i < m_sourceList.count(); i++)
            {
                if (m_sourceList.at(i).fxi == head.fxi)
                {
                    head.fxi = m_targetList.at(i).fxi;
                    group->resignHead(pt);
                    group->assignHead(pt, head);
                    break;
                }
            }
        }
    }

    // 3 – Remap channel groups
    foreach (ChannelsGroup *grp, doc->channelsGroups())
    {
        QList<SceneValue> grpChannels = grp->getChannels();
        grp->resetChannels();
        QList<SceneValue> newList = remapSceneValues(grpChannels, m_sourceList, m_targetList);
        foreach (SceneValue val, newList)
            grp->addChannel(val.fxi, val.channel);
    }

    // 4 – Remap functions
    foreach (Function *func, doc->functions())
    {
        switch (func->type())
        {
            case Function::SceneType:
            {
                Scene *s = qobject_cast<Scene*>(func);
                QList<SceneValue> newList = remapSceneValues(s->values(), m_sourceList, m_targetList);
                s->clear();
                foreach (const SceneValue &sv, newList)
                {
                    s->addFixture(sv.fxi);
                    s->setValue(sv);
                }
            }
            break;

            case Function::SequenceType:
            {
                Sequence *s = qobject_cast<Sequence*>(func);
                for (int idx = 0; idx < s->stepsCount(); idx++)
                {
                    ChaserStep *cs = s->stepAt(idx);
                    QList<SceneValue> newList = remapSceneValues(cs->values, m_sourceList, m_targetList);
                    cs->values.clear();
                    cs->values = newList;
                }
            }
            break;

            case Function::EFXType:
                remapEFX(qobject_cast<EFX*>(func), doc);
            break;

            default:
            break;
        }
    }

    // 5 – Remap monitor properties
    MonitorProperties *props = doc->monitorProperties();
    if (props != nullptr)
    {
        QMap<quint32, FixturePreviewItem> remappedItems;

        foreach (quint32 fxID, props->fixtureItemsID())
        {
            for (int v = 0; v < m_sourceList.count(); v++)
            {
                if (m_sourceList.at(v).fxi == fxID)
                {
                    remappedItems[m_targetList.at(v).fxi] = props->fixtureProperties(fxID);
                    break;
                }
            }
            props->removeFixture(fxID);
        }

        QMapIterator<quint32, FixturePreviewItem> it(remappedItems);
        while (it.hasNext())
        {
            it.next();
            props->setFixtureProperties(it.key(), it.value());
        }
    }
}
