/*
  Q Light Controller
  doc.cpp

  Copyright (c) Heikki Junnila

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

#include <QStringList>
#include <QString>
#include <QDebug>
#include <QList>
#include <QtXml>
#include <QDir>

#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "qlcfile.h"

#include "channelsgroup.h"
#include "collection.h"
#include "function.h"
#include "universe.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "show.h"
#include "efx.h"
#include "doc.h"
#include "bus.h"
#include "rgbscriptscache.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
 #if defined(__APPLE__) || defined(Q_OS_MAC)
  #include "audiocapture_portaudio.h"
 #elif defined(WIN32) || defined (Q_OS_WIN)
  #include "audiocapture_wavein.h"
 #else
  #include "audiocapture_alsa.h"
 #endif
#else
 #include "audiocapture_qt.h"
#endif

Doc::Doc(QObject* parent, int universes)
    : QObject(parent)
    , m_wsPath("")
    , m_fixtureDefCache(new QLCFixtureDefCache)
    , m_modifiersCache(new QLCModifiersCache)
    , m_rgbScriptsCache(new RGBScriptsCache(this))
    , m_ioPluginCache(new IOPluginCache(this))
    , m_ioMap(new InputOutputMap(this, universes))
    , m_masterTimer(new MasterTimer(this))
    , m_inputCapture(NULL)
    , m_monitorProps(NULL)
    , m_mode(Design)
    , m_kiosk(false)
    , m_clipboard(new QLCClipboard(this))
    , m_latestFixtureId(0)
    , m_latestFixtureGroupId(0)
    , m_latestChannelsGroupId(0)
    , m_latestFunctionId(0)
    , m_startupFunctionId(Function::invalidId())
{
    Bus::init(this);
    resetModified();
    qsrand(QTime::currentTime().msec());
}

Doc::~Doc()
{
    delete m_masterTimer;
    m_masterTimer = NULL;

    clearContents();

    if (isKiosk() == false)
    {
        // TODO: is this still needed ??
        //m_ioMap->saveDefaults();
    }
    delete m_ioMap;
    m_ioMap = NULL;

    delete m_ioPluginCache;
    m_ioPluginCache = NULL;

    delete m_modifiersCache;
    m_modifiersCache = NULL;

    delete m_fixtureDefCache;
    m_fixtureDefCache = NULL;
}

void Doc::clearContents()
{
    emit clearing();

    m_clipboard->resetContents();

    if (m_monitorProps != NULL)
        m_monitorProps->reset();

    destroyAudioCapture();

    // Delete all function instances
    QListIterator <quint32> funcit(m_functions.keys());
    while (funcit.hasNext() == true)
    {
        Function* func = m_functions.take(funcit.next());
        if (func == NULL)
            continue;
        emit functionRemoved(func->id());
        delete func;
    }

    // Delete all fixture groups
    QListIterator <quint32> grpit(m_fixtureGroups.keys());
    while (grpit.hasNext() == true)
    {
        FixtureGroup* grp = m_fixtureGroups.take(grpit.next());
        quint32 grpID = grp->id();
        delete grp;
        emit fixtureGroupRemoved(grpID);
    }

    // Delete all fixture instances
    QListIterator <quint32> fxit(m_fixtures.keys());
    while (fxit.hasNext() == true)
    {
        Fixture* fxi = m_fixtures.take(fxit.next());
        quint32 fxID = fxi->id();
        delete fxi;
        emit fixtureRemoved(fxID);
    }

    // Delete all channels groups
    QListIterator <quint32> grpchans(m_channelsGroups.keys());
    while (grpchans.hasNext() == true)
    {
        ChannelsGroup* grp = m_channelsGroups.take(grpchans.next());
        emit channelsGroupRemoved(grp->id());
        delete grp;
    }

    m_orderedGroups.clear();

    m_latestFunctionId = 0;
    m_latestFixtureId = 0;
    m_latestFixtureGroupId = 0;
    m_latestChannelsGroupId = 0;
    m_addresses.clear();

    emit cleared();
}

void Doc::setWorkspacePath(QString path)
{
    m_wsPath = path;
}

QString Doc::getWorkspacePath() const
{
    return m_wsPath;
}

QString Doc::normalizeComponentPath(const QString& filePath) const
{
    if (filePath.isEmpty())
        return filePath;

    QFileInfo f(filePath);

    if (f.absolutePath().startsWith(getWorkspacePath()))
    {
        return QDir(getWorkspacePath()).relativeFilePath(f.absoluteFilePath());
    }
    else
    {
        return f.absoluteFilePath();
    }
}

QString Doc::denormalizeComponentPath(const QString& filePath) const
{
    if (filePath.isEmpty())
        return filePath;

    return QFileInfo(QDir(getWorkspacePath()), filePath).absoluteFilePath();
}

/*****************************************************************************
 * Engine components
 *****************************************************************************/

QLCFixtureDefCache* Doc::fixtureDefCache() const
{
    return m_fixtureDefCache;
}

QLCModifiersCache* Doc::modifiersCache() const
{
    return m_modifiersCache;
}

RGBScriptsCache* Doc::rgbScriptsCache() const
{
    return m_rgbScriptsCache;
}

IOPluginCache* Doc::ioPluginCache() const
{
    return m_ioPluginCache;
}

InputOutputMap* Doc::inputOutputMap() const
{
    return m_ioMap;
}

MasterTimer* Doc::masterTimer() const
{
    return m_masterTimer;
}

AudioCapture *Doc::audioInputCapture()
{
    if (m_inputCapture == NULL)
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#if defined(__APPLE__) || defined(Q_OS_MAC)
        m_inputCapture = new AudioCapturePortAudio();
#elif defined(WIN32) || defined (Q_OS_WIN)
        m_inputCapture = new AudioCaptureWaveIn();
#else
        m_inputCapture = new AudioCaptureAlsa();
#endif
#else
        m_inputCapture = new AudioCaptureQt();
#endif
    }
    return m_inputCapture;
}

void Doc::destroyAudioCapture()
{
    qDebug() << "Destroying audio capture";
    if (m_inputCapture != NULL)
    {
        if (m_inputCapture->isRunning())
            m_inputCapture->stop();
        delete m_inputCapture;
    }
    m_inputCapture = NULL;
}

/*****************************************************************************
 * Modified status
 *****************************************************************************/

bool Doc::isModified() const
{
    return m_modified;
}

void Doc::setModified()
{
    m_modified = true;
    emit modified(true);
}

void Doc::resetModified()
{
    m_modified = false;
    emit modified(false);
}

/*****************************************************************************
 * Main operating mode
 *****************************************************************************/

void Doc::setMode(Doc::Mode mode)
{
    /* Don't do mode switching twice */
    if (m_mode == mode)
        return;
    m_mode = mode;

    emit modeChanged(m_mode);
}

Doc::Mode Doc::mode() const
{
    return m_mode;
}

void Doc::setKiosk(bool state)
{
    m_kiosk = state;
}

bool Doc::isKiosk() const
{
    return m_kiosk;
}

/*********************************************************************
 * Clipboard
 *********************************************************************/

QLCClipboard *Doc::clipboard()
{
    return m_clipboard;
}

/*****************************************************************************
 * Fixtures
 *****************************************************************************/

quint32 Doc::createFixtureId()
{
    /* This results in an endless loop if there are UINT_MAX-1 fixtures. That,
       however, seems a bit unlikely. Are there even 4294967295-1 fixtures in
       total in the whole world? */
    while (m_fixtures.contains(m_latestFixtureId) == true ||
           m_latestFixtureId == Fixture::invalidId())
    {
        m_latestFixtureId++;
    }

    return m_latestFixtureId;
}

bool Doc::addFixture(Fixture* fixture, quint32 id)
{
    Q_ASSERT(fixture != NULL);

    // No ID given, this method can assign one
    if (id == Fixture::invalidId())
        id = createFixtureId();

    if (m_fixtures.contains(id) == true || id == Fixture::invalidId())
    {
        qWarning() << Q_FUNC_INFO << "a fixture with ID" << id << "already exists!";
        return false;
    }
    else
    {
        fixture->setID(id);
        m_fixtures.insert(id, fixture);

        /* Patch fixture change signals thru Doc */
        connect(fixture, SIGNAL(changed(quint32)),
                this, SLOT(slotFixtureChanged(quint32)));

        /* Keep track of fixture addresses */
        for (uint i = fixture->universeAddress();
             i < fixture->universeAddress() + fixture->channels(); i++)
        {
            m_addresses[i] = id;
        }

        // Add the fixture channels capabilities to the universe they belong
        QList<Universe *> universes = inputOutputMap()->claimUniverses();
        int uni = fixture->universe();

        // TODO !!! if a universe for this fixture doesn't exist, add it !!!
        QList<int> forcedHTP = fixture->forcedHTPChannels();
        QList<int> forcedLTP = fixture->forcedLTPChannels();

        for (quint32 i = 0 ; i < fixture->channels(); i++)
        {
            const QLCChannel* channel(fixture->channel(i));
            if (forcedHTP.contains(i))
                universes.at(uni)->setChannelCapability(fixture->address() + i,
                                                        channel->group(), Universe::HTP);
            else if (forcedLTP.contains(i))
                universes.at(uni)->setChannelCapability(fixture->address() + i,
                                                        channel->group(), Universe::LTP);
            else
                universes.at(uni)->setChannelCapability(fixture->address() + i,
                                                        channel->group());
            ChannelModifier *mod = fixture->channelModifier(i);
            universes.at(uni)->setChannelModifier(fixture->address() + i, mod);
        }
        inputOutputMap()->releaseUniverses(true);

        emit fixtureAdded(id);
        setModified();

        return true;
    }
}

bool Doc::deleteFixture(quint32 id)
{
    if (m_fixtures.contains(id) == true)
    {
        Fixture* fxi = m_fixtures.take(id);
        Q_ASSERT(fxi != NULL);

        /* Keep track of fixture addresses */
        QMutableHashIterator <uint,uint> it(m_addresses);
        while (it.hasNext() == true)
        {
            it.next();
            if (it.value() == id)
                it.remove();
        }
        if (m_monitorProps != NULL)
            m_monitorProps->removeFixture(id);

        emit fixtureRemoved(id);
        setModified();
        delete fxi;

        if (m_fixtures.count() == 0)
            m_latestFixtureId = 0;

        return true;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "No fixture with id" << id;
        return false;
    }
}

bool Doc::moveFixture(quint32 id, quint32 newAddress)
{
    if (m_fixtures.contains(id) == true)
    {
        Fixture* fixture = m_fixtures[id];
        // remove it
        QMutableHashIterator <uint,uint> it(m_addresses);
        while (it.hasNext() == true)
        {
            it.next();
            if (it.value() == id)
                it.remove();
        }
        // add it to new address
        for (uint i = newAddress; i < newAddress + fixture->channels(); i++)
        {
            m_addresses[i] = id;
        }
        setModified();

        return true;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "No fixture with id" << id;
        return false;
    }
}

bool Doc::replaceFixtures(QList<Fixture*> newFixturesList)
{
    // Delete all fixture instances
    QListIterator <quint32> fxit(m_fixtures.keys());
    while (fxit.hasNext() == true)
    {
        Fixture* fxi = m_fixtures.take(fxit.next());
        delete fxi;
    }
    m_latestFixtureId = 0;
    m_addresses.clear();

    foreach(Fixture *fixture, newFixturesList)
    {
        quint32 id = fixture->id();
        // create a copy of the original cause remapping will
        // destroy it later
        Fixture *newFixture = new Fixture(this);
        newFixture->setID(id);
        newFixture->setName(fixture->name());
        newFixture->setAddress(fixture->address());
        newFixture->setUniverse(fixture->universe());
        if (fixture->fixtureDef() != NULL && fixture->fixtureMode() != NULL)
        {
            QLCFixtureDef *def = fixtureDefCache()->fixtureDef(fixture->fixtureDef()->manufacturer(),
                                                               fixture->fixtureDef()->model());
            QLCFixtureMode *mode = NULL;
            if (def != NULL)
                mode = def->mode(fixture->fixtureMode()->name());
            newFixture->setFixtureDefinition(def, mode);
        }
        else
            newFixture->setChannels(fixture->channels());
        newFixture->setExcludeFadeChannels(fixture->excludeFadeChannels());
        m_fixtures.insert(id, newFixture);

        /* Patch fixture change signals thru Doc */
        connect(newFixture, SIGNAL(changed(quint32)),
                this, SLOT(slotFixtureChanged(quint32)));

        /* Keep track of fixture addresses */
        for (uint i = newFixture->universeAddress();
             i < newFixture->universeAddress() + newFixture->channels(); i++)
        {
            m_addresses[i] = id;
        }
        m_latestFixtureId = id;
    }
    return true;
}

bool Doc::changeFixtureMode(quint32 id, const QLCFixtureMode *mode)
{
    if (m_fixtures.contains(id) == true)
    {
        Fixture* fixture = m_fixtures[id];
        int address = fixture->address();
        // remove it
        QMutableHashIterator <uint,uint> it(m_addresses);
        while (it.hasNext() == true)
        {
            it.next();
            if (it.value() == id)
                it.remove();
        }
        // add it with new characteristics
        int channels;
        if (mode != NULL)
            channels = mode->channels().count();
        else // generic dimmer
            channels = fixture->channels();
        for (int i = address; i < address + channels; i++)
        {
            m_addresses[i] = id;
        }
        setModified();

        return true;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "No fixture with id" << id;
        return false;
    }
}

bool Doc::updateFixtureChannelCapabilities(quint32 id, QList<int> forcedHTP, QList<int> forcedLTP)
{
    if (m_fixtures.contains(id) == true)
    {
        Fixture* fixture = m_fixtures[id];
        // get exclusive access to the universes list
        QList<Universe *> universes = inputOutputMap()->claimUniverses();
        int uni = fixture->universe();

        // Set forced HTP channels
        if (!forcedHTP.isEmpty())
        {
            fixture->setForcedHTPChannels(forcedHTP);

            for(int i = 0; i < forcedHTP.count(); i++)
            {
                int chIdx = forcedHTP.at(i);
                const QLCChannel* channel(fixture->channel(chIdx));

                if (channel->group() == QLCChannel::Intensity)
                    universes.at(uni)->setChannelCapability(fixture->address() + chIdx,
                                                            channel->group(),
                                                            Universe::ChannelType(Universe::HTP | Universe::Intensity));
                else
                    universes.at(uni)->setChannelCapability(fixture->address() + chIdx,
                                                            channel->group(),
                                                            Universe::HTP);
            }
        }
        // Set forced LTP channels
        if (!forcedLTP.isEmpty())
        {
            fixture->setForcedLTPChannels(forcedLTP);

            for(int i = 0; i < forcedLTP.count(); i++)
            {
                int chIdx = forcedLTP.at(i);
                const QLCChannel* channel(fixture->channel(chIdx));
                universes.at(uni)->setChannelCapability(fixture->address() + chIdx, channel->group(), Universe::LTP);
            }
        }

        // set channels modifiers
        for (quint32 i = 0; i < fixture->channels(); i++)
        {
            ChannelModifier *mod = fixture->channelModifier(i);
            universes.at(uni)->setChannelModifier(fixture->address() + i, mod);
        }
        inputOutputMap()->releaseUniverses(true);

        return true;
    }

    return false;
}

QList <Fixture*> Doc::fixtures() const
{
    QMap <quint32, Fixture*> fixturesMap;
    QHashIterator <quint32, Fixture*> hashIt(m_fixtures);
    while (hashIt.hasNext())
    {
        hashIt.next();
        fixturesMap.insert(hashIt.key(), hashIt.value());
    }
    return fixturesMap.values();
}

Fixture* Doc::fixture(quint32 id) const
{
    return m_fixtures.value(id, NULL);
}

quint32 Doc::fixtureForAddress(quint32 universeAddress) const
{
    return m_addresses.value(universeAddress, Fixture::invalidId());
}

int Doc::totalPowerConsumption(int& fuzzy) const
{
    int totalPowerConsumption = 0;

    // Make sure fuzzy starts from zero
    fuzzy = 0;

    QListIterator <Fixture*> fxit(fixtures());
    while (fxit.hasNext() == true)
    {
        Fixture* fxi(fxit.next());
        Q_ASSERT(fxi != NULL);

        // Generic dimmer has no mode and physical
        if (fxi->isDimmer() == false && fxi->fixtureMode() != NULL)
        {
            QLCPhysical phys = fxi->fixtureMode()->physical();
            if (phys.powerConsumption() > 0)
                totalPowerConsumption += phys.powerConsumption();
            else
                fuzzy++;
        }
        else
        {
            fuzzy++;
        }
    }

    return totalPowerConsumption;
}

void Doc::slotFixtureChanged(quint32 id)
{
    /* Keep track of fixture addresses */
    Fixture* fxi = fixture(id);
    for (uint i = fxi->universeAddress(); i < fxi->universeAddress() + fxi->channels(); i++)
    {
        m_addresses[i] = id;
    }

    setModified();
    emit fixtureChanged(id);
}

/*****************************************************************************
 * Fixture groups
 *****************************************************************************/

bool Doc::addFixtureGroup(FixtureGroup* grp, quint32 id)
{
    Q_ASSERT(grp != NULL);

    // No ID given, this method can assign one
    if (id == FixtureGroup::invalidId())
        id = createFixtureGroupId();

    if (m_fixtureGroups.contains(id) == true || id == FixtureGroup::invalidId())
    {
        qWarning() << Q_FUNC_INFO << "a fixture group with ID" << id << "already exists!";
        return false;
    }
    else
    {
        grp->setId(id);
        m_fixtureGroups[id] = grp;

        /* Patch fixture group change signals thru Doc */
        connect(grp, SIGNAL(changed(quint32)),
                this, SLOT(slotFixtureGroupChanged(quint32)));

        emit fixtureGroupAdded(id);
        setModified();

        return true;
    }
}

bool Doc::deleteFixtureGroup(quint32 id)
{
    if (m_fixtureGroups.contains(id) == true)
    {
        FixtureGroup* grp = m_fixtureGroups.take(id);
        Q_ASSERT(grp != NULL);

        emit fixtureGroupRemoved(id);
        setModified();
        delete grp;

        return true;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "No fixture group with id" << id;
        return false;
    }
}

FixtureGroup* Doc::fixtureGroup(quint32 id) const
{
    if (m_fixtureGroups.contains(id) == true)
        return m_fixtureGroups[id];
    else
        return NULL;
}

QList <FixtureGroup*> Doc::fixtureGroups() const
{
    return m_fixtureGroups.values();
}

quint32 Doc::createFixtureGroupId()
{
    /* This results in an endless loop if there are UINT_MAX-1 fixture groups. That,
       however, seems a bit unlikely. Are there even 4294967295-1 fixtures in
       total in the whole world? */
    while (m_fixtureGroups.contains(m_latestFixtureGroupId) == true ||
           m_latestFixtureGroupId == FixtureGroup::invalidId())
    {
        m_latestFixtureGroupId++;
    }

    return m_latestFixtureGroupId;
}

void Doc::slotFixtureGroupChanged(quint32 id)
{
    setModified();
    emit fixtureGroupChanged(id);
}

/*********************************************************************
 * Channels groups
 *********************************************************************/
bool Doc::addChannelsGroup(ChannelsGroup *grp, quint32 id)
{
    Q_ASSERT(grp != NULL);

    // No ID given, this method can assign one
    if (id == ChannelsGroup::invalidId())
        id = createChannelsGroupId();

     grp->setId(id);
     m_channelsGroups[id] = grp;
     if (m_orderedGroups.contains(id) == false)
        m_orderedGroups.append(id);

     emit channelsGroupAdded(id);
     setModified();

     return true;
}

bool Doc::deleteChannelsGroup(quint32 id)
{
    if (m_channelsGroups.contains(id) == true)
    {
        ChannelsGroup* grp = m_channelsGroups.take(id);
        Q_ASSERT(grp != NULL);

        emit channelsGroupRemoved(id);
        setModified();
        delete grp;

        int idx = m_orderedGroups.indexOf(id);
        if (idx != -1)
            m_orderedGroups.takeAt(idx);

        return true;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "No channels group with id" << id;
        return false;
    }
}

bool Doc::moveChannelGroup(quint32 id, int direction)
{
    if (direction == 0 || m_orderedGroups.contains(id) == false)
        return false;

    int idx = m_orderedGroups.indexOf(id);

    if (idx + direction < 0 || idx + direction >= m_orderedGroups.count())
        return false;

    qDebug() << Q_FUNC_INFO << m_orderedGroups;
    m_orderedGroups.takeAt(idx);
    m_orderedGroups.insert(idx + direction, id);
    qDebug() << Q_FUNC_INFO << m_orderedGroups;

    setModified();
    return true;
}

ChannelsGroup* Doc::channelsGroup(quint32 id) const
{
    if (m_channelsGroups.contains(id) == true)
        return m_channelsGroups[id];
    else
        return NULL;
}

QList <ChannelsGroup*> Doc::channelsGroups() const
{
    QList <ChannelsGroup*> orderedList;

    for (int i = 0; i < m_orderedGroups.count(); i++)
    {
        orderedList.append(m_channelsGroups[m_orderedGroups.at(i)]);
    }
    return orderedList;
}

quint32 Doc::createChannelsGroupId()
{
    while (m_channelsGroups.contains(m_latestChannelsGroupId) == true ||
           m_latestChannelsGroupId == ChannelsGroup::invalidId())
    {
        m_latestChannelsGroupId++;
    }

    return m_latestChannelsGroupId;
}

/*****************************************************************************
 * Functions
 *****************************************************************************/

quint32 Doc::createFunctionId()
{
    /* This results in an endless loop if there are UINT_MAX-1 functions. That,
       however, seems a bit unlikely. Are there even 4294967295-1 functions in
       total in the whole world? */
    while (m_functions.contains(m_latestFunctionId) == true ||
           m_latestFunctionId == Fixture::invalidId())
    {
        m_latestFunctionId++;
    }

    return m_latestFunctionId;
}

bool Doc::addFunction(Function* func, quint32 id)
{
    Q_ASSERT(func != NULL);

    if (id == Function::invalidId())
        id = createFunctionId();

    if (m_functions.contains(id) == true || id == Fixture::invalidId())
    {
        qWarning() << Q_FUNC_INFO << "a function with ID" << id << "already exists!";
        return false;
    }
    else
    {
        // Listen to function changes
        connect(func, SIGNAL(changed(quint32)),
                this, SLOT(slotFunctionChanged(quint32)));

        // Listen to function name changes
        connect(func, SIGNAL(nameChanged(quint32)),
                this, SLOT(slotFunctionNameChanged(quint32)));

        // Make the function listen to fixture removals
        connect(this, SIGNAL(fixtureRemoved(quint32)),
                func, SLOT(slotFixtureRemoved(quint32)));

        // Place the function in the map and assign it the new ID
        m_functions[id] = func;
        func->setID(id);
        emit functionAdded(id);
        setModified();

        return true;
    }
}

QList <Function*> Doc::functions() const
{
    return m_functions.values();
}

QList<Function *> Doc::functionsByType(Function::Type type) const
{
    QList <Function*> list;
    foreach(Function *f, m_functions)
    {
        if (f != NULL && f->type() == type)
            list.append(f);
    }
    return list;
}

bool Doc::deleteFunction(quint32 id)
{
    if (m_functions.contains(id) == true)
    {
        Function* func = m_functions.take(id);
        Q_ASSERT(func != NULL);

        emit functionRemoved(id);
        setModified();
        delete func;

        return true;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "No function with id" << id;
        return false;
    }
}

Function* Doc::function(quint32 id) const
{
    if (m_functions.contains(id) == true)
        return m_functions[id];
    else
        return NULL;
}

quint32 Doc::nextFunctionID()
{
    quint32 tmpFID = m_latestFunctionId;
    while (m_functions.contains(tmpFID) == true ||
           tmpFID == Fixture::invalidId())
    {
        tmpFID++;
    }

    return tmpFID;
}

void Doc::setStartupFunction(quint32 fid)
{
    m_startupFunctionId = fid;
}

quint32 Doc::startupFunction()
{
    return m_startupFunctionId;
}

bool Doc::checkStartupFunction()
{
    if (m_mode == Operate && m_startupFunctionId != Function::invalidId())
    {
        Function *func = function(m_startupFunctionId);
        if (func != NULL)
        {
            func->start(masterTimer());
            return true;
        }
    }
    return false;
}

void Doc::slotFunctionChanged(quint32 fid)
{
    setModified();
    emit functionChanged(fid);
}

void Doc::slotFunctionNameChanged(quint32 fid)
{
    setModified();
    emit functionNameChanged(fid);
}

/*********************************************************************
 * Monitor Properties
 *********************************************************************/

MonitorProperties *Doc::monitorProperties()
{
    if (m_monitorProps == NULL)
        m_monitorProps = new MonitorProperties();

    return m_monitorProps;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool Doc::loadXML(const QDomElement& root)
{
    clearErrorLog();

    if (root.tagName() != KXMLQLCEngine)
    {
        qWarning() << Q_FUNC_INFO << "Engine node not found";
        return false;
    }

    if (root.hasAttribute(KXMLQLCStartupFunction))
    {
        quint32 sID = root.attribute(KXMLQLCStartupFunction).toUInt();
        if (sID != Function::invalidId())
            setStartupFunction(sID);
    }

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();

        if (tag.tagName() == KXMLFixture)
        {
            Fixture::loader(tag, this);
        }
        else if (tag.tagName() == KXMLQLCFixtureGroup)
        {
            FixtureGroup::loader(tag, this);
        }
        else if (tag.tagName() == KXMLQLCChannelsGroup)
        {
            ChannelsGroup::loader(tag, this);
        }
        else if (tag.tagName() == KXMLQLCFunction)
        {
            Function::loader(tag, this);
        }
        else if (tag.tagName() == KXMLQLCBus)
        {
            /* LEGACY */
            Bus::instance()->loadXML(tag);
        }
        else if (tag.tagName() == KXMLIOMap)
        {
            m_ioMap->loadXML(tag);
        }
        else if (tag.tagName() == KXMLQLCMonitorProperties)
        {
            monitorProperties()->loadXML(tag, this);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown engine tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    postLoad();

    emit loaded();

    return true;
}

bool Doc::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    QDomElement root;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    /* Create the master Engine node */
    root = doc->createElement(KXMLQLCEngine);
    if (startupFunction() != Function::invalidId())
    {
        root.setAttribute(KXMLQLCStartupFunction, QString::number(startupFunction()));
    }
    wksp_root->appendChild(root);

    m_ioMap->saveXML(doc, &root);

    /* Write fixtures into an XML document */
    QListIterator <Fixture*> fxit(fixtures());
    while (fxit.hasNext() == true)
    {
        Fixture* fxi(fxit.next());
        Q_ASSERT(fxi != NULL);
        fxi->saveXML(doc, &root);
    }

    /* Write fixture groups into an XML document */
    QListIterator <FixtureGroup*> grpit(fixtureGroups());
    while (grpit.hasNext() == true)
    {
        FixtureGroup* grp(grpit.next());
        Q_ASSERT(grp != NULL);
        grp->saveXML(doc, &root);
    }

    /* Write channel groups into an XML document */
    QListIterator <ChannelsGroup*> chanGroups(channelsGroups());
    while (chanGroups.hasNext() == true)
    {
        ChannelsGroup* grp(chanGroups.next());
        Q_ASSERT(grp != NULL);
        grp->saveXML(doc, &root);
    }

    /* Write functions into an XML document */
    QListIterator <Function*> funcit(functions());
    while (funcit.hasNext() == true)
    {
        Function* func(funcit.next());
        Q_ASSERT(func != NULL);
        func->saveXML(doc, &root);
    }

    if (m_monitorProps != NULL)
        m_monitorProps->saveXML(doc, &root, this);

    return true;
}

void Doc::appendToErrorLog(QString error)
{
    if (m_errorLog.contains(error))
        return;

    m_errorLog.append(error);
    m_errorLog.append("\n");
}

void Doc::clearErrorLog()
{
    m_errorLog = "";
}

QString Doc::errorLog()
{
    return m_errorLog;
}

void Doc::postLoad()
{
    QListIterator <Function*> functionit(functions());
    while (functionit.hasNext() == true)
    {
        Function* function(functionit.next());
        Q_ASSERT(function != NULL);
        function->postLoad();
    }
}
