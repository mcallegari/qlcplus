/*
  Q Light Controller Plus
  doc.h

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

#ifndef DOC_H
#define DOC_H

#include <QObject>
#include <QList>
#include <QFile>
#include <QMap>

#include "qlcfixturedefcache.h"
#include "qlcmodifierscache.h"
#include "inputoutputmap.h"
#include "ioplugincache.h"
#include "channelsgroup.h"
#include "fixturegroup.h"
#include "qlcclipboard.h"
#include "mastertimer.h"
#include "qlcpalette.h"
#include "function.h"
#include "fixture.h"

class AudioCapture;
class RGBScriptsCache;
class AudioPluginCache;
class MonitorProperties;

/** @addtogroup engine Engine
 * @{
 */

#define KXMLQLCEngine QString("Engine")
#define KXMLQLCStartupFunction QString("Autostart")

class Doc : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Doc)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /**
     * Create a new Doc instance for the given parent.
     *
     * @param parent The parent object who owns the Doc instance
     * @param outputUniverses Number of output (DMX) universes
     * @param inputUniverses Number of input universes
     */
    Doc(QObject* parent, int universes = 4);

    /** Destructor */
    ~Doc();

    /** Remove all functions and fixtures from the doc, signalling each removal. */
    void clearContents();

    /**
     * Set the current workspace absolute path. To be used when local files need
     * to be loaded even if the workspace file has been moved
     */
    void setWorkspacePath(QString path);

    /** Retrieve the current workspace absolute path */
    QString getWorkspacePath() const;

    /** If filePath is in the workspace directory or in one of its subdirectories,
     *  return path relative to the workspace directory.
     *  Otherwise return absolute path.
     *
     *  Purpose: saving components of the workspace file (audio, icons,...)
     */
    QString normalizeComponentPath(const QString& filePath) const;

    /** If filePath is relative path, it is resolved relative to the workspace
     *  directory (absolute path is returned).
     *  If filePath is absolute, it is returned unchanged (symlinks and .. are resolved).
     *
     *  Purpose: saving components of the workspace file (audio, icons,...)
     */
    QString denormalizeComponentPath(const QString& filePath) const;

private:
    QString m_wsPath;

signals:
    /** Emitted when clearContents() is called, before actually doing anything. */
    void clearing();

    /** Emitted when clearContents() has finished. */
    void cleared();

    /** Emitted when the document is being loaded, before actually doing anything. */
    void loading();

    /** Emitted when the document has been completely loaded. */
    void loaded();

    /*********************************************************************
     * Engine components
     *********************************************************************/
public:
    /** Get the fixture definition cache object */
    QLCFixtureDefCache *fixtureDefCache() const;

    /** Set the fixure definition cache reference. This is useful
     *  to share a cache between different Docs.
     *  Note: deletion of an existing cache must be performed before calling
     *        this method, otherwise it creates a memory leak */
    void setFixtureDefinitionCache(QLCFixtureDefCache *cache);

    /** Get the channel modifiers cache object */
    QLCModifiersCache *modifiersCache() const;

    /** Get the RGB scripts cache object */
    RGBScriptsCache *rgbScriptsCache() const;

    /** Get the I/O plugin cache object */
    IOPluginCache *ioPluginCache() const;

    /** Get the audio decoder plugin cache object */
    AudioPluginCache *audioPluginCache() const;

    /** Get the DMX output map object */
    InputOutputMap *inputOutputMap() const;

    /** Get the MasterTimer object that runs the show */
    MasterTimer *masterTimer() const;

    /** Get the audio input capture object */
    QSharedPointer<AudioCapture> audioInputCapture();

    /** Destroy a previously created audio capture instance */
    void destroyAudioCapture();

private:
    QLCFixtureDefCache *m_fixtureDefCache;
    QLCModifiersCache *m_modifiersCache;
    RGBScriptsCache *m_rgbScriptsCache;
    IOPluginCache *m_ioPluginCache;
    AudioPluginCache *m_audioPluginCache;
    MasterTimer *m_masterTimer;
    InputOutputMap *m_ioMap;
    QSharedPointer<AudioCapture> m_inputCapture;
    MonitorProperties *m_monitorProps;

    /*********************************************************************
     * Main operating mode
     *********************************************************************/
public:
    enum Mode
    {
        Design  = 0, //! Editing allowed
        Operate = 1  //! Running allowed, editing disabled
    };

    /** Change the main operating mode. See enum Mode for more information. */
    void setMode(Mode mode);

    /** Get the main operating mode. See enum Mode for more information. */
    Mode mode() const;

    /**
     * Enable/disable kiosk mode. This doesn't do practically anything by itself.
     * It's mostly a convenient helper for engine components to detect if kiosk
     * mode is on.
     */
    void setKiosk(bool kiosk);

    /** Check, if kiosk mode is enabled or not. */
    bool isKiosk() const;

signals:
    /** Tells that the current operating mode has changed */
    void modeChanged(Doc::Mode mode);

protected:
    Mode m_mode;
    bool m_kiosk;

    /*********************************************************************
     * Modified status
     *********************************************************************/
public:
    enum LoadStatus
    {
        Cleared = 0,
        Loading,
        Loaded
    };

    /** Get the current Doc load status */
    LoadStatus loadStatus() const;

    /** Check, whether Doc has been modified (and is in need of saving) */
    bool isModified() const;

    /** Set Doc into modified state (i.e. it is in need of saving) */
    void setModified();

    /** Reset Doc's modified state (i.e. it is no longer in need of saving) */
    void resetModified();

signals:
    /** Signal that this Doc has been modified (or unmodified) */
    void modified(bool state);

protected:
    /** The current Doc load status */
    LoadStatus m_loadStatus;
    bool m_modified;

    /*********************************************************************
     * Clipboard
     *********************************************************************/
public:
    /** Get a reference to QLC+ global clipboard*/
    QLCClipboard *clipboard();

private:
    QLCClipboard *m_clipboard;

    /*********************************************************************
     * Fixture Instances
     *********************************************************************/
public:
    /**
     * Add the given fixture to doc's fixture array.
     * If id == Fixture::invalidId(), doc assigns the fixture a new ID and
     * takes ownership, unless there is no more room for more fixtures.
     *
     * If id != Fixture::invalidId(), doc attempts to put the fixture at
     * that exact index, unless another fixture already occupies it.
     *
     * @param fixture The fixture to add
     * @param id The requested ID for the fixture
     * @return true if the fixture was successfully added to doc,
     *         otherwise false.
     */
    bool addFixture(Fixture* fixture, quint32 id = Fixture::invalidId(), bool crossUniverse = false);

    /**
     * Delete the given fixture instance from Doc
     *
     * @param id The ID of the fixture instance to delete
     */
    bool deleteFixture(quint32 id);

    /**
     * Replace the whole fixtures list with a new one.
     * This is done by remapping. Note that no signal is emitted to
     * avoid loosing scenes and all the stuff connected to fixtures.
     * The caller must be aware on this and reassign all the QLC+ project
     * data previously created.
     *
     * @param newFixturesList list of fixtures that will take place
     */
    bool replaceFixtures(QList<Fixture*> newFixturesList);

    /**
     * Update the channels capabilities of an existing fixture with the given ID
     * @param id The ID of the fixture instance
     * @param forcedHTP A list of channel indices forced to act as HTP
     * @param forcedLTP A list of channel indices forced to act as LTP
     */
    bool updateFixtureChannelCapabilities(quint32 id, QList<int>forcedHTP, QList<int>forcedLTP);

    /**
     * Get the fixture instance that has the given ID
     *
     * @param id The ID of the fixture to get
     */
    Fixture* fixture(quint32 id) const;

    /**
     * Get a list of fixtures ordered by ID
     */
    QList<Fixture*> const& fixtures() const;

    /**
     * Get the number of fixtures currently added to the project
     *
     * @return The number of fixtures
     */
    int fixturesCount() const;

    /**
     * Get the fixture that occupies the given DMX address. If multiple fixtures
     * occupy the same address, the one that has been last modified is returned.
     *
     * @param universeAddress The universe & address of the fixture to look for
     * @return The fixture ID or Fixture::invalidId() if not found
     */
    quint32 fixtureForAddress(quint32 universeAddress) const;

    /**
     * Get the total power consumption of all fixtures in the current
     * workspace.
     *
     * @param[out] fuzzy Number of fixtures without power consumption value
     * @return Total power consumption
     */
    int totalPowerConsumption(int& fuzzy) const;

protected:
    /**
     * Create a new fixture ID
     */
    quint32 createFixtureId();

signals:
    /** Signal that a fixture has been added */
    void fixtureAdded(quint32 fxi_id);

    /** Signal that a fixture has been removed */
    void fixtureRemoved(quint32 fxi_id);

    /** Signal that a fixture's properties have changed */
    void fixtureChanged(quint32 fxi_id);

private slots:
    /** Catch fixture property changes */
    void slotFixtureChanged(quint32 fxi_id);

protected:
    /** Fixtures hash: < ID, Fixture instance > */
    QHash <quint32, Fixture*> m_fixtures;

    /** Fixtures list cache */
    bool m_fixturesListCacheUpToDate;
    QList<Fixture*> m_fixturesListCache;

    /** Map of the addresses occupied by fixtures */
    QHash <quint32, quint32> m_addresses;

    /** Latest assigned fixture ID */
    quint32 m_latestFixtureId;

    /*********************************************************************
     * Fixture groups
     *********************************************************************/
public:
    /** Add a new fixture group. Doc takes ownership of the group. */
    bool addFixtureGroup(FixtureGroup* grp, quint32 id = FixtureGroup::invalidId());

    /**
     * Remove and delete a fixture group. Doesn't destroy group's fixtures.
     * The group pointer is invalid after this call.
     */
    bool deleteFixtureGroup(quint32 id);

    /** Get a fixture group by id */
    FixtureGroup* fixtureGroup(quint32 id) const;

    /** Get a list of Doc's fixture groups */
    QList <FixtureGroup*> fixtureGroups() const;

signals:
    void fixtureGroupAdded(quint32 id);
    void fixtureGroupRemoved(quint32 id);
    void fixtureGroupChanged(quint32 id);

private slots:
    /** Catch fixture group property changes */
    void slotFixtureGroupChanged(quint32 id);

private:
    /** Create a new fixture group ID */
    quint32 createFixtureGroupId();

private:
    /** Fixture Groups */
    QMap <quint32,FixtureGroup*> m_fixtureGroups;

    /** Latest assigned fixture group ID */
    quint32 m_latestFixtureGroupId;

    /*********************************************************************
     * Channel groups
     *********************************************************************/
public:
    /** Add a new channels group. Doc takes ownership of the group. */
    bool addChannelsGroup(ChannelsGroup *grp, quint32 id = ChannelsGroup::invalidId());

    /**
     * Remove and delete a channels group.
     * The group pointer is invalid after this call.
     */
    bool deleteChannelsGroup(quint32 id);

    /** Move a channel group of a given direction amount */
    bool moveChannelGroup(quint32 id, int direction);

    /** Get a channels group by id */
    ChannelsGroup* channelsGroup(quint32 id) const;

    /** Get a list of Doc's channels groups */
    QList <ChannelsGroup*> channelsGroups() const;

private:
    /** Create a new channels group ID */
    quint32 createChannelsGroupId();

signals:
    /** Signal that a channels group has been added */
    void channelsGroupAdded(quint32 chgrp_id);

    /** Signal that a channels group has been removed */
    void channelsGroupRemoved(quint32 chgrp_id);

private:
    /** Channel Groups */
    QMap <quint32,ChannelsGroup*> m_channelsGroups;

    /** Hold the Channel Groups IDs ordered as displayed
     *  in the Fixture Manager panel */
    QList <quint32> m_orderedGroups;

    /** Latest assigned channel group ID */
    quint32 m_latestChannelsGroupId;

    /*********************************************************************
     * Palettes
     *********************************************************************/
public:
    /** Add a new palette. Doc takes ownership of it */
    bool addPalette(QLCPalette *palette, quint32 id = QLCPalette::invalidId());

    /**
     * Remove and delete a palette.
     * The Palette pointer is invalid after this call.
     */
    bool deletePalette(quint32 id);

    /** Get a palette by id */
    QLCPalette *palette(quint32 id) const;

    /** Get a list of Doc's palettes */
    QList <QLCPalette*> palettes() const;

private:
    /** Create a new palette ID */
    quint32 createPaletteId();

signals:
    /** Inform the listeners that a new palette has been added */
    void paletteAdded(quint32 id);

    /** Inform the listeners that a new palette has been removed */
    void paletteRemoved(quint32 id);

private:
    /** Palettes */
    QMap <quint32,QLCPalette*> m_palettes;

    /** Latest assigned palette ID */
    quint32 m_latestPaletteId;

    /*********************************************************************
     * Functions
     *********************************************************************/
public:
    /**
     * Add the given function to doc's function array.
     * If id == Function::invalidId(), doc assigns the function a new ID
     * and takes ownership, unless there is no more room for more functions.
     *
     * If id != Function::invalidId(), doc attempts to put the function at
     * that exact index, unless another function already occupies it.
     *
     * @param function The function to add
     * @param id The requested ID for the function
     * @return true if the function was successfully added to doc, otherwise false.
     */
    bool addFunction(Function* function, quint32 id = Function::invalidId());

    /**
     * Get a list of currently available functions
     *
     * @return List of functions
     */
    QList <Function*> functions() const;

    /**
     * Get a list of currently available functions by type
     *
     * @return List of functions by type
     */
    QList <Function*> functionsByType(Function::Type type) const;

    /**
     * Get a pointer to a Function with the given name
     * @param name lookup Function name
     * @return pointer to Function or null if not found
     */
    Function *functionByName(QString name);

    /**
     * Delete the given function
     *
     * @param id The ID of the function to delete
     * @return true if the function was found and deleted
     */
    bool deleteFunction(quint32 id);

    /**
     * Get a function that has the given ID
     *
     * @param id The ID of the function to get
     * @return A function at the given ID or NULL if not found
     */
    Function* function(quint32 id) const;

    /**
     * Get the next Function ID that will be assigned at the
     * creation of a new Function
     */
    quint32 nextFunctionID();

    /**
     * Set the ID of a function to start everytime QLC+ goes
     * in operate mode
     *
     * @param fid The ID of the function
     */
    void setStartupFunction(quint32 fid);

    /**
     * Retrieve the QLC+ startup function
     */
    quint32 startupFunction();

    /**
     * Find the usage of a Function with the specified $fid
     * within Doc
     *
     * @param fid the Function ID to look up for
     * @return a list of Function IDs and, if available, the step position
     */
    QList<quint32> getUsage(quint32 fid);

protected:
    /**
     * Create a new function Id
     */
    quint32 createFunctionId();

    /**
     * Assign the given function ID to the function, place the function
     * at the same index in m_functionArray, increase function count and
     * emit functionAdded() signal.
     *
     * @param function The function to assign
     * @param id The ID to assign to the function
     */
    void assignFunction(Function* function, quint32 id);

private slots:
    /** Slot that catches function change signals */
    void slotFunctionChanged(quint32 fid);

    /** Slot that catches function name change signals */
    void slotFunctionNameChanged(quint32 fid);

signals:
    /** Signal that a function has been added */
    void functionAdded(quint32 function);

    /** Signal that a function has been removed */
    void functionRemoved(quint32 function);

    /** Signal that a function has been changed */
    void functionChanged(quint32 function);

    /** Signal that a function has been changed */
    void functionNameChanged(quint32 function);

protected:
    /** Functions */
    QMap <quint32,Function*> m_functions;

    /** Latest assigned function ID */
    quint32 m_latestFunctionId;

    /** Startup function ID */
    quint32 m_startupFunctionId;

    /*********************************************************************
     * Monitor Properties
     *********************************************************************/
public:
    /** Returns a reference to the monitor properties instance */
    MonitorProperties *monitorProperties();

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /**
     * Load contents from the given XML document
     *
     * @param root The Engine XML root node to load from
     * @param loadIO Parse the InputOutputMap tag too
     * @return true if successful, otherwise false
     */
    bool loadXML(QXmlStreamReader &doc, bool loadIO = true);

    /**
     * Save contents to the given XML file.
     *
     * @param doc The XML document to save to
     * @param wksp_root The workspace root node to save under
     * @return true if successful, otherwise false
     */
    bool saveXML(QXmlStreamWriter *doc);

    /**
     * Append a message to the Doc error log. This can be used to display
     * errors once a project is loaded.
     */
    void appendToErrorLog(QString error);

    /**
     * Clear any previously filled error log
     */
    void clearErrorLog();

    /**
     * Retrieve the error log string, filled during a project load
     */
    QString errorLog();

private:
    /**
     * Calls postLoad() for each Function after everything has been loaded
     * to do post-load cleanup & mappings.
     */
    void postLoad();

    QString m_errorLog;
};

/** @} */

#endif
