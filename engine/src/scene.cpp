/*
  Q Light Controller Plus
  scene.cpp

  Copyright (c) Heikki Junnila
                Massimo Callegari

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>
#include <cmath>
#include <QList>
#include <QFile>

#include "qlcfixturedef.h"
#include "qlccapability.h"

#include "genericfader.h"
#include "mastertimer.h"
#include "universe.h"
#include "scene.h"
#include "doc.h"
#include "bus.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Scene::Scene(Doc* doc)
    : Function(doc, Function::SceneType)
    , m_legacyFadeBus(Bus::invalid())
    , m_flashOverrides(false)
    , m_flashForceLTP(false)
    , m_blendFunctionID(Function::invalidId())
{
    setName(tr("New Scene"));
    registerAttribute(tr("ParentIntensity"), Multiply | Single);
}

Scene::~Scene()
{
}

QIcon Scene::getIcon() const
{
    return QIcon(":/scene.png");
}

quint32 Scene::totalDuration()
{
    return (quint32)duration();
}

/*****************************************************************************
 * Copying
 *****************************************************************************/

Function* Scene::createCopy(Doc* doc, bool addToDoc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new Scene(doc);
    if (copy->copyFrom(this) == false)
    {
        delete copy;
        copy = NULL;
    }
    if (addToDoc == true && doc->addFunction(copy) == false)
    {
        delete copy;
        copy = NULL;
    }

    return copy;
}

bool Scene::copyFrom(const Function* function)
{
    const Scene* scene = qobject_cast<const Scene*> (function);
    if (scene == NULL)
        return false;

    m_values.clear();
    m_values = scene->m_values;
    m_fixtures.clear();
    m_fixtures = scene->m_fixtures;
    m_channelGroups.clear();
    m_channelGroups = scene->m_channelGroups;
    m_channelGroupsLevels.clear();
    m_channelGroupsLevels = scene->m_channelGroupsLevels;
    m_fixtureGroups.clear();
    m_fixtureGroups = scene->m_fixtureGroups;
    m_palettes.clear();
    m_palettes = scene->m_palettes;

    return Function::copyFrom(function);
}

/*****************************************************************************
 * Values
 *****************************************************************************/

void Scene::setValue(const SceneValue& scv, bool blind, bool checkHTP)
{
    bool valChanged = false;

    if (!m_fixtures.contains(scv.fxi))
    {
        qWarning() << Q_FUNC_INFO << "Setting value for unknown fixture" << scv.fxi << ". Adding it.";
        m_fixtures.append(scv.fxi);
    }

    {
        QMutexLocker locker(&m_valueListMutex);

        QMap<SceneValue, uchar>::iterator it = m_values.find(scv);
        if (it == m_values.end())
        {
            m_values.insert(scv, scv.value);
            valChanged = true;
        }
        else if (it.value() != scv.value)
        {
            const_cast<uchar&>(it.key().value) = scv.value;
            it.value() = scv.value;
            valChanged = true;
        }

        // if the scene is running, we must
        // update/add the changed channel
        if (blind == false && m_fadersMap.isEmpty() == false)
        {
            Fixture *fixture = doc()->fixture(scv.fxi);
            if (fixture != NULL)
            {
                quint32 universe = fixture->universe();

                FadeChannel fc(doc(), scv.fxi, scv.channel);
                fc.setStart(scv.value);
                fc.setTarget(scv.value);
                fc.setCurrent(scv.value);
                fc.setFadeTime(0);

                if (m_fadersMap.contains(universe))
                {
                    if (checkHTP == false)
                        m_fadersMap[universe]->replace(fc);
                    else
                        m_fadersMap[universe]->add(fc);
                }
            }
        }
    }

    emit changed(this->id());
    if (valChanged)
        emit valueChanged(scv);
}

void Scene::setValue(quint32 fxi, quint32 ch, uchar value)
{
    setValue(SceneValue(fxi, ch, value));
}

void Scene::unsetValue(quint32 fxi, quint32 ch)
{
    if (!m_fixtures.contains(fxi))
        qWarning() << Q_FUNC_INFO << "Unsetting value for unknown fixture" << fxi;

    {
        QMutexLocker locker(&m_valueListMutex);
        m_values.remove(SceneValue(fxi, ch, 0));
    }

    emit changed(this->id());
}

uchar Scene::value(quint32 fxi, quint32 ch)
{
    return m_values.value(SceneValue(fxi, ch, 0), 0);
}

bool Scene::checkValue(SceneValue val)
{
    return m_values.contains(val);
}

QList <SceneValue> Scene::values() const
{
    return m_values.keys();
}

QList<quint32> Scene::components()
{
    QList<quint32> ids;

    foreach (SceneValue scv, m_values.keys())
    {
        if (ids.contains(scv.fxi) == false)
            ids.append(scv.fxi);
    }

    return ids;
}

QColor Scene::colorValue(quint32 fxi)
{
    int rVal = 0, gVal = 0, bVal = 0;
    int cVal = -1, mVal = -1, yVal = -1;
    bool found = false;
    QColor CMYcol;

    foreach (SceneValue scv, m_values.keys())
    {
        if (fxi != Fixture::invalidId() && fxi != scv.fxi)
            continue;

        Fixture *fixture = doc()->fixture(scv.fxi);
        if (fixture == NULL)
            continue;

        const QLCChannel* channel = fixture->channel(scv.channel);
        if (channel == NULL)
            continue;

        if (channel->group() == QLCChannel::Intensity)
        {
            QLCChannel::PrimaryColour col = channel->colour();
            switch (col)
            {
                case QLCChannel::Red: rVal = scv.value; found = true; break;
                case QLCChannel::Green: gVal = scv.value; found = true; break;
                case QLCChannel::Blue: bVal = scv.value; found = true; break;
                case QLCChannel::Cyan: cVal = scv.value; break;
                case QLCChannel::Magenta: mVal = scv.value; break;
                case QLCChannel::Yellow: yVal = scv.value; break;
                case QLCChannel::White: rVal = gVal = bVal = scv.value; found = true; break;
                default: break;
            }
        }
        else if (channel->group() == QLCChannel::Colour)
        {
            QLCCapability *cap = channel->searchCapability(scv.value);
            if (cap &&
                (cap->presetType() == QLCCapability::SingleColor ||
                 cap->presetType() == QLCCapability::DoubleColor))
            {
                QColor col = cap->resource(0).value<QColor>();
                rVal = col.red();
                gVal = col.green();
                bVal = col.blue();
                found = true;
            }
        }

        if (cVal >= 0 && mVal >= 0 && yVal >= 0)
        {
            CMYcol.setCmyk(cVal, mVal, yVal, 0);
            rVal = CMYcol.red();
            gVal = CMYcol.green();
            bVal = CMYcol.blue();
            found = true;
        }
    }

    if (found)
        return QColor(rVal, gVal, bVal);

    return QColor();
}

void Scene::clear()
{
    m_values.clear();
    m_fixtures.clear();
    m_fixtureGroups.clear();
    m_palettes.clear();
}

/*********************************************************************
 * Channel Groups
 *********************************************************************/

void Scene::addChannelGroup(quint32 id)
{
    if (m_channelGroups.contains(id) == false)
    {
        m_channelGroups.append(id);
        m_channelGroupsLevels.append(0);
    }
}

void Scene::removeChannelGroup(quint32 id)
{
    int idx = m_channelGroups.indexOf(id);
    if (idx != -1)
    {
        m_channelGroups.removeAt(idx);
        m_channelGroupsLevels.removeAt(idx);
    }
}

void Scene::setChannelGroupLevel(quint32 id, uchar level)
{
    int idx = m_channelGroups.indexOf(id);
    if (idx >= 0 && idx < m_channelGroupsLevels.count())
        m_channelGroupsLevels[idx] = level;
}

QList<uchar> Scene::channelGroupsLevels()
{
    return m_channelGroupsLevels;
}

QList<quint32> Scene::channelGroups()
{
    return m_channelGroups;
}

/*****************************************************************************
 * Fixtures
 *****************************************************************************/

void Scene::slotFixtureRemoved(quint32 fxi_id)
{
    bool hasChanged = false;

    QMutableMapIterator <SceneValue, uchar> it(m_values);
    while (it.hasNext() == true)
    {
        SceneValue value(it.next().key());
        if (value.fxi == fxi_id)
        {
            it.remove();
            hasChanged = true;
        }
    }

    if (removeFixture(fxi_id))
        hasChanged = true;

    if (hasChanged)
        emit changed(this->id());
}

void Scene::addFixture(quint32 fixtureId)
{
    if (m_fixtures.contains(fixtureId) == false)
        m_fixtures.append(fixtureId);
}

bool Scene::removeFixture(quint32 fixtureId)
{
    return m_fixtures.removeOne(fixtureId);
}

QList<quint32> Scene::fixtures() const
{
    return m_fixtures;
}

/*********************************************************************
 * Fixture Groups
 *********************************************************************/

void Scene::addFixtureGroup(quint32 id)
{
    if (m_fixtureGroups.contains(id) == false)
        m_fixtureGroups.append(id);
}

bool Scene::removeFixtureGroup(quint32 id)
{
    return m_fixtureGroups.removeOne(id);
}

QList<quint32> Scene::fixtureGroups() const
{
    return m_fixtureGroups;
}

/*********************************************************************
 * Palettes
 *********************************************************************/

void Scene::addPalette(quint32 id)
{
    if (m_palettes.contains(id) == false)
        m_palettes.append(id);
}

bool Scene::removePalette(quint32 id)
{
    return m_palettes.removeOne(id);
}

QList<quint32> Scene::palettes() const
{
    return m_palettes;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool Scene::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    /* Function tag */
    doc->writeStartElement(KXMLQLCFunction);

    /* Common attributes */
    saveXMLCommon(doc);

    /* Speed */
    saveXMLSpeed(doc);

    /* Channel groups */
    if (m_channelGroups.count() > 0)
    {
        QString chanGroupsIDs;
        for (int i = 0; i < m_channelGroups.size(); ++i)
        {
            if (chanGroupsIDs.isEmpty() == false)
                chanGroupsIDs.append(QString(","));
            int id = m_channelGroups.at(i);
            int val = m_channelGroupsLevels.at(i);
            chanGroupsIDs.append(QString("%1,%2").arg(id).arg(val));
        }
        doc->writeTextElement(KXMLQLCSceneChannelGroupsValues, chanGroupsIDs);
    }

    /* Scene contents */
    // make a copy of the Scene values cause we need to empty it in the process
    QList<SceneValue> values = m_values.keys();

    // loop through the Scene Fixtures in the order they've been added
    foreach (quint32 fxId, m_fixtures)
    {
        QStringList currFixValues;
        bool found = false;

        // look for the values that match the current Fixture ID
        for (int j = 0; j < values.count(); j++)
        {
            SceneValue scv = values.at(j);
            if (scv.fxi != fxId)
            {
                if (found == true)
                    break;
                else
                    continue;
            }

            found = true;
            currFixValues.append(QString::number(scv.channel));
            // IMPORTANT: if a Scene is hidden, so used as a container by some Sequences,
            // it must be saved with values set to zero
            currFixValues.append(QString::number(isVisible() ? scv.value : 0));
            values.removeAt(j);
            j--;
        }

        saveXMLFixtureValues(doc, fxId, currFixValues);
    }

    /* Save referenced Fixture Groups */
    foreach (quint32 groupId, m_fixtureGroups)
    {
        doc->writeStartElement(KXMLQLCFixtureGroup);
        doc->writeAttribute(KXMLQLCFixtureGroupID, QString::number(groupId));
        doc->writeEndElement();
    }

    /* Save referenced Palettes */
    foreach (quint32 pId, m_palettes)
    {
        doc->writeStartElement(KXMLQLCPalette);
        doc->writeAttribute(KXMLQLCPaletteID, QString::number(pId));
        doc->writeEndElement();
    }

    /* End the <Function> tag */
    doc->writeEndElement();

    return true;
}

bool Scene::saveXMLFixtureValues(QXmlStreamWriter* doc, quint32 fixtureID, QStringList const& values)
{
    doc->writeStartElement(KXMLQLCFixtureValues);
    doc->writeAttribute(KXMLQLCFixtureID, QString::number(fixtureID));
    if (values.size() > 0)
        doc->writeCharacters(values.join(","));
    doc->writeEndElement();
    return true;
}

bool Scene::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root.attributes().value(KXMLQLCFunctionType).toString() != typeToString(Function::SceneType))
    {
        qWarning() << Q_FUNC_INFO << "Function is not a scene";
        return false;
    }

    /* Load scene contents */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCBus)
        {
            m_legacyFadeBus = root.readElementText().toUInt();
        }
        else if (root.name() == KXMLQLCFunctionSpeed)
        {
            loadXMLSpeed(root);
        }
        else if (root.name() == KXMLQLCSceneChannelGroups)
        {
            QString chGrpIDs = root.readElementText();
            if (chGrpIDs.isEmpty() == false)
            {
                QStringList grpArray = chGrpIDs.split(",");
                foreach (QString grp, grpArray)
                {
                    m_channelGroups.append(grp.toUInt());
                    m_channelGroupsLevels.append(0);
                }
            }
        }
        else if (root.name() == KXMLQLCSceneChannelGroupsValues)
        {
            QString chGrpIDs = root.readElementText();
            if (chGrpIDs.isEmpty() == false)
            {
                QStringList grpArray = chGrpIDs.split(",");
                for (int i = 0; i + 1 < grpArray.count(); i+=2)
                {
                    m_channelGroups.append(grpArray.at(i).toUInt());
                    m_channelGroupsLevels.append(grpArray.at(i + 1).toUInt());
                }
            }
        }
        /* "old" style XML */
        else if (root.name() == KXMLQLCFunctionValue)
        {
            /* Channel value */
            SceneValue scv;
            if (scv.loadXML(root) == true)
                setValue(scv);
        }
        /* "new" style XML */
        else if (root.name() == KXMLQLCFixtureValues)
        {
            quint32 fxi = root.attributes().value(KXMLQLCFixtureID).toString().toUInt();
            addFixture(fxi);
            QString strvals = root.readElementText();
            if (strvals.isEmpty() == false)
            {
                QStringList varray = strvals.split(",");
                for (int i = 0; i + 1 < varray.count(); i+=2)
                {
                    SceneValue scv;
                    scv.fxi = fxi;
                    scv.channel = QString(varray.at(i)).toUInt();
                    scv.value = uchar(QString(varray.at(i + 1)).toInt());
                    setValue(scv);
                }
            }
        }
        else if (root.name() == KXMLQLCFixtureGroup)
        {
            quint32 id = root.attributes().value(KXMLQLCFixtureGroupID).toString().toUInt();
            addFixtureGroup(id);
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLQLCPalette)
        {
            quint32 id = root.attributes().value(KXMLQLCPaletteID).toString().toUInt();
            addPalette(id);
            root.skipCurrentElement();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown scene tag:" << root.name();
            root.skipCurrentElement();
        }
    }

    return true;
}

void Scene::postLoad()
{
    // Map legacy bus speed to fixed speed values
    if (m_legacyFadeBus != Bus::invalid())
    {
        quint32 value = Bus::instance()->value(m_legacyFadeBus);
        setFadeInSpeed((value / MasterTimer::frequency()) * 1000);
        setFadeOutSpeed((value / MasterTimer::frequency()) * 1000);
    }

    // Remove such fixtures and channels that don't exist
    QMutableMapIterator <SceneValue, uchar> it(m_values);
    while (it.hasNext() == true)
    {
        SceneValue value(it.next().key());
        Fixture* fxi = doc()->fixture(value.fxi);
        if (fxi == NULL || fxi->channel(value.channel) == NULL)
            it.remove();
    }
}

/****************************************************************************
 * Flashing
 ****************************************************************************/

void Scene::flash(MasterTimer *timer, bool shouldOverride, bool forceLTP)
{
    if (flashing() == true)
        return;

    m_flashOverrides = shouldOverride;
    m_flashForceLTP = forceLTP;

    Q_ASSERT(timer != NULL);
    Function::flash(timer, shouldOverride, forceLTP);
    timer->registerDMXSource(this);
}

void Scene::unFlash(MasterTimer *timer)
{
    if (flashing() == false)
        return;

    Q_ASSERT(timer != NULL);
    Function::unFlash(timer);
}

void Scene::writeDMX(MasterTimer *timer, QList<Universe *> ua)
{
    Q_ASSERT(timer != NULL);

    if (flashing() == true)
    {
        if (m_fadersMap.isEmpty())
        {
            // Keep HTP and LTP channels up. Flash is more or less a forceful intervention
            // so enforce all values that the user has chosen to flash.
            foreach (const SceneValue& sv, m_values.keys())
            {
                FadeChannel fc(doc(), sv.fxi, sv.channel);
                quint32 universe = fc.universe();
                if (universe == Universe::invalid())
                    continue;

                QSharedPointer<GenericFader> fader = m_fadersMap.value(universe, QSharedPointer<GenericFader>());
                if (fader.isNull())
                {
                    fader = ua[universe]->requestFader(m_flashOverrides ? Universe::Flashing : Universe::Auto);

                    fader->adjustIntensity(getAttributeValue(Intensity));
                    fader->setBlendMode(blendMode());
                    fader->setName(name());
                    fader->setParentFunctionID(id());
                    m_fadersMap[universe] = fader;
                }

                if (m_flashForceLTP)
                    fc.addFlag(FadeChannel::ForceLTP);
                fc.setTarget(sv.value);
                fc.addFlag(FadeChannel::Flashing);
                fader->add(fc);
            }
        }
    }
    else
    {
        handleFadersEnd(timer);
        timer->unregisterDMXSource(this);
    }
}

/****************************************************************************
 * Running
 ****************************************************************************/

void Scene::processValue(MasterTimer *timer, QList<Universe*> ua, uint fadeIn, SceneValue &scv)
{
    Fixture *fixture = doc()->fixture(scv.fxi);

    if (fixture == NULL)
        return;

    int universeIndex = floor((fixture->universeAddress() + scv.channel) / 512);
    if (universeIndex >= ua.count())
        return;

    Universe *universe = ua.at(universeIndex);

    QSharedPointer<GenericFader> fader = m_fadersMap.value(universe->id(), QSharedPointer<GenericFader>());
    if (fader.isNull())
    {
        fader = universe->requestFader();
        fader->adjustIntensity(getAttributeValue(Intensity));
        fader->setBlendMode(blendMode());
        fader->setName(name());
        fader->setParentFunctionID(id());
        fader->setParentIntensity(getAttributeValue(ParentIntensity));
        fader->setHandleSecondary(true);
        m_fadersMap[universe->id()] = fader;
    }

    FadeChannel *fc = fader->getChannelFader(doc(), universe, scv.fxi, scv.channel);
    int chIndex = fc->channelIndex(scv.channel);

    /** If a blend Function has been set, check if this channel needs to
     *  be blended from a previous value. If so, mark it for crossfade
     *  and set its current value */
    if (blendFunctionID() != Function::invalidId())
    {
        Scene *blendScene = qobject_cast<Scene *>(doc()->function(blendFunctionID()));
        if (blendScene != NULL && blendScene->checkValue(scv))
        {
            fc->addFlag(FadeChannel::CrossFade);
            fc->setCurrent(blendScene->value(scv.fxi, scv.channel), chIndex);
            qDebug() << "----- BLEND from Scene" << blendScene->name()
                     << ", fixture:" << scv.fxi << ", channel:" << scv.channel << ", value:" << fc->current();
        }
    }
    else
    {
        qDebug() << "Scene" << name() << "add channel" << scv.channel << "from" << fc->current(chIndex) << "to" << scv.value;
    }

    fc->setStart(fc->current(chIndex), chIndex);
    fc->setTarget(scv.value, chIndex);

    if (fc->canFade() == false)
    {
        fc->setFadeTime(0);
    }
    else
    {
        if (tempoType() == Beats)
        {
            int fadeInTime = beatsToTime(fadeIn, timer->beatTimeDuration());
            int beatOffset = timer->nextBeatTimeOffset();

            if (fadeInTime - beatOffset > 0)
                fc->setFadeTime(fadeInTime - beatOffset);
            else
                fc->setFadeTime(fadeInTime);
        }
        else
        {
            fc->setFadeTime(fadeIn);
        }
    }
}

void Scene::handleFadersEnd(MasterTimer *timer)
{
    uint fadeout = overrideFadeOutSpeed() == defaultSpeed() ? fadeOutSpeed() : overrideFadeOutSpeed();

    /* If no fade out is needed, dismiss all the requested faders.
     * Otherwise, set all the faders to fade out and let Universe dismiss them
     * when done */
    if (fadeout == 0)
    {
        dismissAllFaders();
    }
    else
    {
        if (tempoType() == Beats)
            fadeout = beatsToTime(fadeout, timer->beatTimeDuration());

        foreach (QSharedPointer<GenericFader> fader, m_fadersMap.values())
        {
            if (!fader.isNull())
                fader->setFadeOut(true, fadeout);
        }
    }

    m_fadersMap.clear();

    // autonomously reset a blend function if set
    setBlendFunctionID(Function::invalidId());
}

void Scene::write(MasterTimer *timer, QList<Universe*> ua)
{
    //qDebug() << Q_FUNC_INFO << elapsed();

    if (m_values.count() == 0 && m_palettes.count() == 0)
    {
        stop(FunctionParent::master());
        return;
    }

    if (m_fadersMap.isEmpty())
    {
        uint fadeIn = overrideFadeInSpeed() == defaultSpeed() ? fadeInSpeed() : overrideFadeInSpeed();

        foreach (quint32 paletteID, palettes())
        {
            QLCPalette *palette = doc()->palette(paletteID);
            if (palette == NULL)
                continue;

            foreach (SceneValue scv, palette->valuesFromFixtureGroups(doc(), fixtureGroups()))
                processValue(timer, ua, fadeIn, scv);

            foreach (SceneValue scv, palette->valuesFromFixtures(doc(), fixtures()))
                processValue(timer, ua, fadeIn, scv);
        }

        QMutexLocker locker(&m_valueListMutex);
        QMapIterator <SceneValue, uchar> it(m_values);
        while (it.hasNext() == true)
        {
            SceneValue scv(it.next().key());
            processValue(timer, ua, fadeIn, scv);
        }
    }

    if (isPaused() == false)
    {
        incrementElapsed();
        if (timer->isBeat() && tempoType() == Beats)
            incrementElapsedBeats();
    }
}

void Scene::postRun(MasterTimer* timer, QList<Universe *> ua)
{
    handleFadersEnd(timer);

    Function::postRun(timer, ua);
}

void Scene::setPause(bool enable)
{
    if (!isRunning())
        return;

    foreach (QSharedPointer<GenericFader> fader, m_fadersMap.values())
    {
        if (!fader.isNull())
            fader->setPaused(enable);
    }
    Function::setPause(enable);
}

/****************************************************************************
 * Intensity
 ****************************************************************************/

int Scene::adjustAttribute(qreal fraction, int attributeId)
{
    int attrIndex = Function::adjustAttribute(fraction, attributeId);

    if (attrIndex == Intensity)
    {
        foreach (QSharedPointer<GenericFader> fader, m_fadersMap.values())
        {
            if (!fader.isNull())
                fader->adjustIntensity(getAttributeValue(Function::Intensity));
        }
    }
    else if (attrIndex == ParentIntensity)
    {
        foreach (QSharedPointer<GenericFader> fader, m_fadersMap.values())
        {
            if (!fader.isNull())
                fader->setParentIntensity(getAttributeValue(ParentIntensity));
        }
    }

    return attrIndex;
}

/*************************************************************************
 * Blend mode
 *************************************************************************/

void Scene::setBlendMode(Universe::BlendMode mode)
{
    if (mode == blendMode())
        return;

    qDebug() << "Scene" << name() << "blend mode set to" << Universe::blendModeToString(mode);

    foreach (QSharedPointer<GenericFader> fader, m_fadersMap.values())
    {
        if (!fader.isNull())
            fader->setBlendMode(mode);
    }

    Function::setBlendMode(mode);
}

quint32 Scene::blendFunctionID() const
{
    return m_blendFunctionID;
}

void Scene::setBlendFunctionID(quint32 fid)
{
    m_blendFunctionID = fid;
    if (isRunning() && fid == Function::invalidId())
    {
        foreach (QSharedPointer<GenericFader> fader, m_fadersMap.values())
        {
            if (!fader.isNull())
                fader->resetCrossfade();
        }
    }
}
