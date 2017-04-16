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
#include <QList>
#include <QFile>

#include "qlcfixturedef.h"
#include "qlcmacros.h"
#include "qlcfile.h"
#include "qlccapability.h"

#include "genericfader.h"
#include "mastertimer.h"
#include "universe.h"
#include "scene.h"
#include "doc.h"
#include "bus.h"

#include "sceneuistate.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Scene::Scene(Doc* doc) : Function(doc, Function::SceneType)
    , m_legacyFadeBus(Bus::invalid())
    , m_fader(NULL)
{
    setName(tr("New Scene"));
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
    m_channelGroups.clear();
    m_channelGroups = scene->m_channelGroups;
    m_channelGroupsLevels.clear();
    m_channelGroupsLevels = scene->m_channelGroupsLevels;

    return Function::copyFrom(function);
}

/*****************************************************************************
 * UI State
 *****************************************************************************/

FunctionUiState * Scene::createUiState()
{
    return new SceneUiState(this);
}

/*****************************************************************************
 * Values
 *****************************************************************************/

void Scene::setValue(const SceneValue& scv, bool blind, bool checkHTP)
{
    if (!m_fixtures.contains(scv.fxi))
        qWarning() << Q_FUNC_INFO << "Setting value for unknown fixture" << scv.fxi;

    m_valueListMutex.lock();

    QMap<SceneValue, uchar>::iterator it = m_values.find(scv);
    if (it == m_values.end())
        m_values.insert(scv, scv.value);
    else
    {
        const_cast<uchar&>(it.key().value) = scv.value;
        it.value() = scv.value;
    }

    // if the scene is running, we must
    // update/add the changed channel
    if (blind == false && m_fader != NULL)
    {
        FadeChannel fc(doc(), scv.fxi, scv.channel);
        fc.setStart(scv.value);
        fc.setTarget(scv.value);
        fc.setCurrent(scv.value);
        fc.setFadeTime(0);
        if (checkHTP == false)
            m_fader->forceAdd(fc);
        else
            m_fader->add(fc);
    }

    m_valueListMutex.unlock();

    emit changed(this->id());
}

void Scene::setValue(quint32 fxi, quint32 ch, uchar value)
{
    setValue(SceneValue(fxi, ch, value));
}

void Scene::unsetValue(quint32 fxi, quint32 ch)
{
    if (!m_fixtures.contains(fxi))
        qWarning() << Q_FUNC_INFO << "Unsetting value for unknown fixture" << fxi;

    m_valueListMutex.lock();
    m_values.remove(SceneValue(fxi, ch, 0));
    m_valueListMutex.unlock();

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

QColor Scene::colorValue(quint32 fxi)
{
    int rVal = 0, gVal = 0, bVal = 0;
    int cVal = -1, mVal = -1, yVal = -1;
    bool found = false;
    QColor CMYcol;

    foreach(SceneValue scv, m_values.keys())
    {
        if (fxi != Fixture::invalidId() && fxi != scv.fxi)
            continue;

        Fixture *fixture = doc()->fixture(scv.fxi);
        if (fixture == NULL)
            continue;

        const QLCChannel* channel(fixture->channel(scv.channel));
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
            if (cap && cap->resourceColor1() != QColor())
            {
                QColor col = cap->resourceColor1();
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
    QList<quint32> writtenFixtures;
    QMapIterator <SceneValue, uchar> it(m_values);
    qint32 currFixID = -1;
    QStringList currFixValues;
    while (it.hasNext() == true)
    {
        SceneValue sv = it.next().key();
        if (currFixID == -1) currFixID = sv.fxi;
        if ((qint32)sv.fxi != currFixID)
        {
            saveXMLFixtureValues(doc, currFixID, currFixValues);
            writtenFixtures << currFixID;
            currFixValues.clear();
            currFixID = sv.fxi;
        }
        currFixValues.append(QString::number(sv.channel));
        // IMPORTANT: if a Scene is hidden, so used as a container by some Sequences,
        // it must be saved with values set to zero
        currFixValues.append(QString::number(isVisible() ? sv.value : 0));
    }
    /* write last element */
    saveXMLFixtureValues(doc, currFixID, currFixValues);
    writtenFixtures << currFixID;

    // Write fixtures with no scene value
    foreach(quint32 fixtureID, m_fixtures)
    {
        if (writtenFixtures.contains(fixtureID))
            continue;

        saveXMLFixtureValues(doc, fixtureID, QStringList());
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
                foreach(QString grp, grpArray)
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

void Scene::flash(MasterTimer* timer)
{
    if (flashing() == true)
        return;

    Q_ASSERT(timer != NULL);
    Function::flash(timer);
    timer->registerDMXSource(this);
}

void Scene::unFlash(MasterTimer* timer)
{
    if (flashing() == false)
        return;

    Q_ASSERT(timer != NULL);
    Function::unFlash(timer);
}

void Scene::writeDMX(MasterTimer* timer, QList<Universe *> ua)
{
    Q_UNUSED(ua)
    Q_ASSERT(timer != NULL);

    if (flashing() == true)
    {
        // Keep HTP and LTP channels up. Flash is more or less a forceful intervention
        // so enforce all values that the user has chosen to flash.
        foreach (const SceneValue& sv, m_values.keys())
        {
            FadeChannel fc(doc(), sv.fxi, sv.channel);
            fc.setTarget(sv.value);
            fc.setFlashing(true);
            // Force add this channel, since it will be removed
            // by MasterTimer once applied
            timer->faderForceAdd(fc);
        }
    }
    else
    {
        timer->unregisterDMXSource(this);
    }
}

/****************************************************************************
 * Running
 ****************************************************************************/

void Scene::preRun(MasterTimer* timer)
{
    qDebug() << "Scene preRun. ID: " << id();

    Q_ASSERT(m_fader == NULL);
    Function::preRun(timer);
}

void Scene::write(MasterTimer* timer, QList<Universe*> ua)
{
    //qDebug() << Q_FUNC_INFO << elapsed();

    if (m_values.size() == 0)
    {
        stop(FunctionParent::master());
        return;
    }

    if (m_fader == NULL)
    {
        m_valueListMutex.lock();
        m_fader = new GenericFader(doc());
        m_fader->adjustIntensity(getAttributeValue(Intensity));
        m_fader->setBlendMode(blendMode());

        QMapIterator <SceneValue, uchar> it(m_values);
        while (it.hasNext() == true)
        {
            SceneValue value(it.next().key());
            bool canFade = true;

            FadeChannel fc(doc(), value.fxi, value.channel);
            Fixture *fixture = doc()->fixture(value.fxi);
            if (fixture != NULL)
                canFade = fixture->channelCanFade(value.channel);

            fc.setTarget(value.value);

            if (canFade == false)
            {
                fc.setFadeTime(0);
            }
            else
            {
                uint fadein = overrideFadeInSpeed() == defaultSpeed() ? fadeInSpeed() : overrideFadeInSpeed();

                if (tempoType() == Beats)
                {
                    int fadeInTime = beatsToTime(fadein, timer->beatTimeDuration());
                    int beatOffset = timer->nextBeatTimeOffset();

                    if (fadeInTime - beatOffset > 0)
                        fc.setFadeTime(fadeInTime - beatOffset);
                    else
                        fc.setFadeTime(fadeInTime);
                }
                else
                    fc.setFadeTime(fadein);
            }
            insertStartValue(fc, timer, ua);
            m_fader->add(fc);
        }
        m_valueListMutex.unlock();
    }

    //qDebug() << "[Scene] writing channels:" << m_fader->channels().count();
    // Run the internal GenericFader
    m_fader->write(ua, isPaused());

    // Fader has nothing to do. Stop.
    if (m_fader->channels().size() == 0)
        stop(FunctionParent::master());

    if (isPaused() == false)
    {
        incrementElapsed();
        if (timer->isBeat() && tempoType() == Beats)
            incrementElapsedBeats();
    }
}

void Scene::postRun(MasterTimer* timer, QList<Universe *> ua)
{
    if (m_fader != NULL)
    {
        QHashIterator <FadeChannel,FadeChannel> it(m_fader->channels());
        while (it.hasNext() == true)
        {
            it.next();
            FadeChannel fc = it.value();
            // fade out only intensity channels
            if (fc.group(doc()) != QLCChannel::Intensity)
                continue;

            bool canFade = true;
            Fixture *fixture = doc()->fixture(fc.fixture());
            if (fixture != NULL)
                canFade = fixture->channelCanFade(fc.channel());
            fc.setStart(fc.current(getAttributeValue(Intensity)));
            fc.setCurrent(fc.current(getAttributeValue(Intensity)));

            fc.setElapsed(0);
            fc.setReady(false);
            if (canFade == false)
            {
                fc.setFadeTime(0);
                fc.setTarget(fc.current(getAttributeValue(Intensity)));
            }
            else
            {
                uint fadeout = overrideFadeOutSpeed() == defaultSpeed() ? fadeOutSpeed() : overrideFadeOutSpeed();

                if (tempoType() == Beats)
                    fc.setFadeTime(beatsToTime(fadeout, timer->beatTimeDuration()));
                else
                    fc.setFadeTime(fadeout);

                fc.setTarget(0);
            }
            timer->faderAdd(fc);
        }

        delete m_fader;
        m_fader = NULL;
    }

    Function::postRun(timer, ua);
}

void Scene::insertStartValue(FadeChannel& fc, const MasterTimer* timer,
                             const QList<Universe*> ua)
{
    QMutexLocker channelsLocker(timer->faderMutex());
    QHash <FadeChannel,FadeChannel> const& channels(timer->faderChannelsRef());
    QHash <FadeChannel,FadeChannel>::const_iterator existing_it = channels.find(fc);
    if (existing_it != channels.constEnd())
    {
        // MasterTimer's GenericFader contains the channel so grab its current
        // value as the new starting value to get a smoother fade
        fc.setStart(existing_it.value().current());
        fc.setCurrent(fc.start());
    }
    else
    {
        // MasterTimer didn't have the channel. Grab the starting value from UniverseArray.
        quint32 address = fc.address();
        quint32 uni = fc.universe();
        if (fc.group(doc()) != QLCChannel::Intensity)
            fc.setStart(ua[uni]->preGMValue(address));
        else
            fc.setStart(0); // HTP channels must start at zero
        fc.setCurrent(fc.start());
    }
}

/****************************************************************************
 * Intensity
 ****************************************************************************/

void Scene::adjustAttribute(qreal fraction, int attributeIndex)
{
    if (m_fader != NULL && attributeIndex == Intensity)
        m_fader->adjustIntensity(fraction);
    Function::adjustAttribute(fraction, attributeIndex);
}

/*************************************************************************
 * Blend mode
 *************************************************************************/

void Scene::setBlendMode(Universe::BlendMode mode)
{
    if (mode == blendMode())
        return;

    qDebug() << "Scene" << name() << "blend mode set to" << Universe::blendModeToString(mode);

    if (m_fader != NULL)
        m_fader->setBlendMode(mode);

    Function::setBlendMode(mode);
}
