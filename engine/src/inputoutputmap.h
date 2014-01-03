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

#include <QObject>
#include <QMutex>
#include <QDir>

#include "qlcinputprofile.h"
#include "grandmaster.h"

class QLCInputSource;
class QLCIOPlugin;
class OutputPatch;
class InputPatch;
class Universe;
class Doc;

/** @addtogroup engine Engine
 * @{
 */

#define KXMLIOMap "InputOutputMap"

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
    InputOutputMap(Doc* doc, quint32 universes);

    /**
     * Destroy a InputOutputMap object
     */
    ~InputOutputMap();

    /**
     * Load all output plugins from the given directory, using QDir filters.
     *
     * @param dir The directory to load plugins from
     */
    void loadPlugins(const QDir& dir);
    
private:
    /** Get the doc object */
    Doc* doc() const;

    /*********************************************************************
     * Blackout
     *********************************************************************/
public:
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
     */
    void setBlackout(bool blackout);

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
    bool removeUniverse();

    /**
     * Remove all the universes in the current universes list
     */
    bool removeAllUniverses();

    /**
     * Retrieve the friendly name of the universe at the given index
     * @param index The universe index
     * @return The universe name or an empty string
     */
    QString getUniverseName(int index);

    /**
     * Set a friendly name of the universe with the given index
     * @param index The universe index
     * @param name The universe new name
     */
    void setUniverseName(int index, QString name);

    /**
     * Retrieve the number of universe in the output map
     */
    quint32 universes() const;

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
     * Write current universe array data to plugins, each universe within
     * the array to its assigned plugin.
     */
    void dumpUniverses();

    /**
     * Reset all universes (useful when starting from scratch)
     */
    void resetUniverses();

signals:
    void universesWritten(int index, const QByteArray& universes);

private:
    /** Keep track of the lastest asigned universe ID */
    quint32 m_latestUniverseId;

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
    /**
     * Patch the given universe to go through the given input plugin
     *
     * @param universe The input universe to patch
     * @param pluginName The name of the plugin to patch to the universe
     * @param input An input universe provided by the plugin to patch to
     * @param profileName The name of an input profile
     * @return true if successful, otherwise false
     */
    bool setInputPatch(quint32 universe, const QString& pluginName,
                       quint32 input, const QString& profileName = QString());

    /**
     * Patch the given universe to go through the given output plugin
     *
     * @param universe The universe to patch
     * @param pluginName The name of the plugin to patch to the universe
     * @param output A universe provided by the plugin to patch to
     * @param isFeedback Determine if this line is a feedback output
     * @return true if successful, otherwise false
     */
    bool setOutputPatch(quint32 universe, const QString& pluginName,
                        quint32 output = 0, bool isFeedback = false);

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
    OutputPatch* outputPatch(quint32 universe) const;

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
     * @return A QStringList containing the names of each input line
     *
     */
    QStringList pluginInputs(const QString& pluginName);

    /**
     * Get the number of universes provided by the given plugin.
     *
     * @param pluginName Name of the plugin, whose output count to get
     * @return A list of output names provided by the plugin.
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
    bool sendFeedBack(quint32 universe, quint32 channel, uchar value, const QString& key = 0);

private slots:
   /** Slot that catches plugin configuration change notifications from UIPluginCache */
    void slotPluginConfigurationChanged(QLCIOPlugin* plugin);

signals:
    /** Notifies (OutputManager) of plugin configuration changes */
    void pluginConfigurationChanged(const QString& pluginName);

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
    bool inputSourceNames(const QLCInputSource& src,
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
     * Defaults
     *********************************************************************/
public:
    /**
     * Load default settings for output mapper from QLC global settings
     */
    void loadDefaults();

    /**
     * Load the input/output map map contents from the given XML node.
     *
     * @param root An XML subtree containing the input/output map contents
     * @return true if the map was loaded successfully, otherwise false
     */
    bool loadXML(const QDomElement& root);

    /**
     * Save the input/output map instance into an XML document, under the given
     * XML element (tag).
     *
     * @param doc The master XML document to save to.
     * @param wksp_root The workspace root element
     */
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root) const;

};

/** @} */

#endif // INPUTOUTPUTMAP_H
