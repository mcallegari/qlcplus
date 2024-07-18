/*
  Q Light Controller Plus
  inputoutputmap.h

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

#ifndef INPUTOUTPUTMAP_H
#define INPUTOUTPUTMAP_H

#include <QSharedPointer>
#include <QObject>
#include <QMutex>
#include <QDir>

#include "qlcinputprofile.h"
#include "grandmaster.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class QLCInputSource;
class QElapsedTimer;
class QLCIOPlugin;
class OutputPatch;
class InputPatch;
class Universe;
class Doc;

/** @addtogroup engine Engine
 * @{
 */

#define KXMLIOMap               QString("InputOutputMap")
#define KXMLIOBeatGenerator     QString("BeatGenerator")
#define KXMLIOBeatType          QString("BeatType")
#define KXMLIOBeatsPerMinute    QString("BPM")

class InputOutputMap : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(InputOutputMap)

    friend class InputPatch;

    /*********************************************************************
     * Initialization
     *********************************************************************/

public:
    /**
     * Create a new InputOutputMap object
     *
     * @param doc The QLC+ project reference
     * @param universes Number of universes
     */
    InputOutputMap(Doc* doc, quint32 universesCount);

    /**
     * Destroy a InputOutputMap object
     */
    ~InputOutputMap();

private:
    /** Get the doc object */
    Doc* doc() const;

    /*********************************************************************
     * Blackout
     *********************************************************************/
public:
    enum BlackoutRequest
    {
        BlackoutRequestNone,
        BlackoutRequestOn,
        BlackoutRequestOff
    };

    /**
     * Toggle blackout between on and off.
     *
     * @return New blackout state (i.e. after toggling)
     */
    bool toggleBlackout();

    /**
     * Set blackout on or off
     *
     * @param blackout If true, set blackout ON, otherwise OFF
     * @return true if blackout state changed
     */
    bool setBlackout(bool blackout);

    /**
     * Schedule blackout on or off
     *
     * Scripts toggling blackout cannot wait for m_UniverseMutex to unlock
     * since they are within locked mutex. The solution is to toggle the blackout
     * later during dumpUniverses()
     *
     * @param blackout If true, set blackout ON, otherwise OFF
     */
    void requestBlackout(BlackoutRequest blackout);

    /**
     * Get blackout state
     *
     * @return true if blackout is ON, otherwise false
     */
    bool blackout() const;

signals:
    /**
     * Signal that is sent when blackout state is changed.
     *
     * @param state true if blackout has been turned on, otherwise false
     */
    void blackoutChanged(bool state);

private:
    /** Current blackout state */
    bool m_blackout;

    /*********************************************************************
     * Universes
     *********************************************************************/
public:

    /**
     * Invalid universe number (for comparison etc.)
     */
    static quint32 invalidUniverse();

    /**
     * Add a new universe and append it at the end of the
     * current universes list
     */
    bool addUniverse(quint32 id = InputOutputMap::invalidUniverse());

    /**
     * Remove the last universe in the current universes list
     */
    bool removeUniverse(int index);

    /**
     * Remove all the universes in the current universes list
     */
    bool removeAllUniverses();

    /**
     * Start all the Universe threads
     */
    void startUniverses();

    /**
     * Get the unique ID of the universe at the given index
     * @param index The universe index
     * @return The universe ID or invalidUniverse()
     */
    quint32 getUniverseID(int index);

    /**
     * Retrieve the friendly name of the universe at the given index
     * @param index The universe index
     * @return The universe name or an empty string
     */
    QString getUniverseNameByIndex(int index);

    /**
     * Retrieve the friendly name of the universe with the given ID
     * @param id The universe unique ID
     * @return The universe name or an empty string
     */
    QString getUniverseNameByID(quint32 id);

    /**
     * Set a friendly name of the universe with the given index
     * @param index The universe index
     * @param name The universe new name
     */
    void setUniverseName(int index, QString name);

    /**
     * Set/unset the universe with the given index in passthrough mode
     * @param index The universe index
     * @param enable true = passthrough, false = normal mode
     */
    void setUniversePassthrough(int index, bool enable);

    /**
     * Retrieve the passthrough mode of the universe at the given index
     * @param index The universe index
     * @return true = passthrough, false = normal mode
     */
    bool getUniversePassthrough(int index);

    /**
     * Enable/disable the monitor mode for the universe with the given index
     * @param index The universe index
     * @param enable true = monitor, false = do not monitor
     */
    void setUniverseMonitor(int index, bool enable);

    /**
     * Retrieve the monitor mode of the universe at the given index
     * @param index The universe index
     * @return true = monitor, false = do not monitor
     */
    bool getUniverseMonitor(int index);

    /**
     * Return if a universe is patched with any input, output or
     * feedback line
     * @param index The universe index
     * @return true = patched, false = not patched
     */
    bool isUniversePatched(int index);

    /**
     * Retrieve the number of universes in the input/output map
     */
    quint32 universesCount() const;

    /**
     * Retrieve the list of references of the Universe in the input/output map
     */
    QList<Universe*> universes() const;

    /**
     * Get a reference to a Universe from the given Universe ID
     * Return NULL if no Universe is found
     */
    Universe *universe(quint32 id);

    /**
     * Claim access to a universe. This is declared virtual to make
     * unit testing a bit easier.
     */
    virtual QList<Universe*> claimUniverses();

    /**
     * Release access to a universe. This is declared virtual to make
     * unit testing a bit easier.
     *
     * @param changed Set to true if DMX values were changed
     */
    virtual void releaseUniverses(bool changed = true);

    /**
     * Reset all universes (useful when starting from scratch)
     */
    void resetUniverses();

signals:
    void universeAdded(quint32 id);
    void universeRemoved(quint32 id);
    void universeWritten(quint32 index, const QByteArray& universesData);

private:
    /** The values of all universes */
    QList<Universe *> m_universeArray;

    /** When true, universes are dumped. Otherwise not. */
    bool m_universeChanged;

    /** Mutex guarding m_universeArray */
    QMutex m_universeMutex;

    /*********************************************************************
     * Grand Master
     *********************************************************************/
public:
    /**
     * Set grand master channel mode (intensity or all channels)
     */
    void setGrandMasterChannelMode(GrandMaster::ChannelMode mode);

    /**
     * Get grand master channel mode (intensity or all channels)
     */
    GrandMaster::ChannelMode grandMasterChannelMode();

    /**
     * Set grand master value mode (limit or reduce)
     */
    void setGrandMasterValueMode(GrandMaster::ValueMode mode);

    /**
     * Set grand master value mode (limit or reduce)
     */
    GrandMaster::ValueMode grandMasterValueMode();

    /**
     * Set grand master value (0-255)
     */
    void setGrandMasterValue(uchar value);

    /**
     * Get grand master value (0-255)
     */
    uchar grandMasterValue();

signals:
    void grandMasterValueChanged(uchar value);
    void grandMasterValueModeChanged(GrandMaster::ValueMode mode);

private:
    /** The Grand Master reference */
    GrandMaster *m_grandMaster;

    /*********************************************************************
     * Patch
     *********************************************************************/

public:
    void flushInputs();

    /**
     * Patch the given universe to go through the given input plugin
     *
     * @param universe The input universe to patch
     * @param pluginName The name of the plugin to patch to the universe
     * @param inputUID Unique plugin input line identifier as string
     * @param input An input universe provided by the plugin to patch to
     * @param profileName The name of an input profile
     * @return true if successful, otherwise false
     */
    bool setInputPatch(quint32 universe, const QString& pluginName,
                       const QString& inputUID, quint32 input,
                       const QString& profileName = QString());

    /**
     * Set an input profile to the given universe. If the universe doesn't
     * have an input patch, this method returns false
     *
     * @param universe The universe to patch
     * @param profileName the name of the input profile to set
     * @return true if successful, otherwise false
     */
    bool setInputProfile(quint32 universe, const QString& profileName);

    /**
     * Patch the given universe to go through the given output plugin
     *
     * @param universe The universe to patch
     * @param pluginName The name of the plugin to patch to the universe
     * @param inputUID Unique plugin output line identifier as string
     * @param output A universe provided by the plugin to patch to
     * @param isFeedback Determine if this line is a feedback output
     * @param index The output patch index
     *
     * @return true if successful, otherwise false
     */
    bool setOutputPatch(quint32 universe, const QString& pluginName,
                        const QString& outputUID, quint32 output = 0,
                        bool isFeedback = false, int index = 0);

    int outputPatchesCount(quint32 universe) const;

    /**
     * Get mapping for an input universe.
     *
     * @param universe The internal input universe to get mapping for
     */
    InputPatch* inputPatch(quint32 universe) const;

    /**
     * Get the output mapping for a QLC universe.
     *
     * @param universe The internal universe to get mapping for
     */
    OutputPatch* outputPatch(quint32 universe, int index = 0) const;

    /**
     * Get the feedback mapping for a QLC universe.
     *
     * @param universe The internal universe to get mapping for
     */
    OutputPatch* feedbackPatch(quint32 universe) const;

    /**
     * Get a list of available universes.
     */
    QStringList universeNames() const;

    /**
     * Check, whether a certain input in a certain plugin has been mapped
     * to a universe. Returns the mapped universe number or QLCIOPlugin::invalidLine()
     * if not mapped.
     *
     * @param pluginName The name of the plugin to check for
     * @param input The particular input to check for
     * @return Mapped universe number or -1 if not mapped
     */
    quint32 inputMapping(const QString& pluginName, quint32 input) const;

    /**
     * Check, whether a certain output in a certain plugin has been mapped
     * to a universe. Returns the mapped universe number or QLCIOPlugin::invalidLine()
     * if not mapped.
     *
     * @param pluginName The name of the plugin to check for
     * @param output The particular output to check for
     * @return Mapped universe number or -1 if not mapped
     */
    quint32 outputMapping(const QString& pluginName, quint32 output) const;

    /*********************************************************************
     * Plugins
     *********************************************************************/
public:
    /**
     * Get a description text for the given plugin.
     */
    QString pluginDescription(const QString& pluginName);

    /**
     * Get a list of available input plugins as a string list
     * containing the plugins' names
     *
     * @return QStringList containing plugins' names
     */
    QStringList inputPluginNames();

    /**
     * Get a list of available Output output plugins as a string list
     * containing the plugins' names
     *
     * @return QStringList containing plugins' names
     */
    QStringList outputPluginNames();

    /**
     * Get the names of all input lines provided by the given plugin.
     *
     * @param pluginName Name of the plugin, whose input count to get
     * @return A list containing the name of each input line
     *
     */
    QStringList pluginInputs(const QString& pluginName);

    /**
     * Get the names of all output lines provided by the given plugin.
     *
     * @param pluginName Name of the plugin, whose output count to get
     * @return A list containing the name of each output line
     */
    QStringList pluginOutputs(const QString& pluginName);

    /**
     * Check, whether a plugin supports feedback
     *
     * @param pluginName The name of the plugin to check from.
     * @return true if plugin supports feedback. Otherwise false.
     */
    bool pluginSupportsFeedback(const QString& pluginName);

    /**
     * Open a configuration dialog for the given plugin
     *
     * @param pluginName Name of the plugin to configure
     */
    void configurePlugin(const QString& pluginName);

    /**
     * Check, whether a plugin provides additional configuration options.
     *
     * @param pluginName The name of the plugin to check from.
     * @return true if plugin can be configured. Otherwise false.
     */
    bool canConfigurePlugin(const QString& pluginName);

    /**
     * Get a status text for the given plugin.
     *
     * @param pluginName Name of the plugin, whose status to get
     * @param input A specific input identifier
     */
    QString inputPluginStatus(const QString& pluginName, quint32 input);

    /**
     * Get a status text for the given plugin. If no plugin name is
     * given, an overall mapping status of all universes is returned.
     *
     * @param pluginName Name of the plugin, whose status to get
     * @param output Plugin's output line for getting more specific info
     */
    QString outputPluginStatus(const QString& pluginName, quint32 output);

    /**
     * Send feedback value to the input profile e.g. to move a motorized
     * sliders & knobs, set indicator leds etc.
     */
    bool sendFeedBack(quint32 universe, quint32 channel, uchar value, const QVariant &params);

private:
    /** In case of duplicate strings, append a number to make them unique */
    void removeDuplicates(QStringList &list);

private slots:
   /** Slot that catches plugin configuration change notifications from UIPluginCache */
    void slotPluginConfigurationChanged(QLCIOPlugin* plugin);

signals:
    /** Signal emitted when a profile is changed */
    void profileChanged(quint32 universe, const QString& profileName);

    /** Notifies (InputOutputManager) of plugin configuration changes */
    void pluginConfigurationChanged(const QString& pluginName, bool success);

    /** Everyone interested in input data should connect to this signal */
    void inputValueChanged(quint32 universe, quint32 channel, uchar value, const QString& key = 0);

    /*************************************************************************
     * Input profiles
     *************************************************************************/
public:
    /** Load all input profiles from the given directory using QDir filters */
    void loadProfiles(const QDir& dir);

    /** Get a list of available profile names */
    QStringList profileNames();

    /** Get a profile by its name */
    QLCInputProfile* profile(const QString& name);

    /** Add a new profile */
    bool addProfile(QLCInputProfile* profile);

    /** Remove an existing profile by its name and delete it */
    bool removeProfile(const QString& name);

    /**
     * Get input source names for the given input universe and channel.
     *
     * @param src (IN) The input source, whose universe & channel names to get
     * @param uniName (OUT) The name of the universe, if available
     * @param chName (OUT) The name of the channel, if available
     *
     * @return true if uniName & chName contain something, otherwise false
     */
    bool inputSourceNames(const QLCInputSource *src,
                          QString& uniName, QString& chName) const;
    bool inputSourceNames(QSharedPointer<QLCInputSource> const& src,
                          QString& uniName, QString& chName) const;

    /**
     * Get the default system input profile directory that contains installed
     * input profiles. The location varies greatly between platforms.
     *
     * @return System profile directory
     */
    static QDir systemProfileDirectory();

    /**
     * Get the user's own default input profile directory that is used to save
     * custom input profiles. The location varies greatly between platforms.
     *
     * @return User profile directory
     */
    static QDir userProfileDirectory();

private:
    /** List that contains all available profiles */
    QList <QLCInputProfile*> m_profiles;

    /*********************************************************************
     * Beats
     *********************************************************************/
public:
    enum BeatGeneratorType
    {
        Disabled,   //! No one is generating beats
        Internal,   //! MasterTimer is the beat generator
        Plugin,     //! A plugin is the beat generator
        Audio       //! An audio input device is the beat generator
    };

    void setBeatGeneratorType(BeatGeneratorType type);
    BeatGeneratorType beatGeneratorType() const;

    QString beatTypeToString(BeatGeneratorType type) const;
    BeatGeneratorType stringToBeatType(QString str);

    void setBpmNumber(int bpm);
    int bpmNumber() const;

protected slots:
    void slotMasterTimerBeat();
    void slotPluginBeat(quint32 universe, quint32 channel, uchar value, const QString &key);
    void slotAudioSpectrum(double *spectrumBands, int size, double maxMagnitude, quint32 power);

signals:
    void beatGeneratorTypeChanged();
    void bpmNumberChanged(int bpmNumber);
    void beat();

private:
    BeatGeneratorType m_beatGeneratorType;
    int m_currentBPM;
    QElapsedTimer *m_beatTime;

    /*********************************************************************
     * Defaults
     *********************************************************************/
public:
    /**
     * Load default settings for input/output mapper from QLC+ global settings
     */
    void loadDefaults();

    /**
     * Save default settings for input/output mapper into QLC+ global settings
     */
    void saveDefaults();

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /**
     * Load the input/output map contents from the given XML node.
     *
     * @param root An XML subtree containing the input/output map contents
     * @return true if the map was loaded successfully, otherwise false
     */
    bool loadXML(QXmlStreamReader &root);

    /**
     * Save the input/output map instance into an XML document, under the given
     * XML element (tag).
     *
     * @param doc The master XML document to save to.
     * @param wksp_root The workspace root element
     */
    bool saveXML(QXmlStreamWriter *doc) const;

};

/** @} */

#endif // INPUTOUTPUTMAP_H
