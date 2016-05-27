/*
  Q Light Controller Plus
  qlcioplugin.h

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

#ifndef QLCIOPLUGIN_H
#define QLCIOPLUGIN_H

#include <QStringList>
#include <QtPlugin>
#include <QVariant>
#include <QObject>
#include <climits>
#include <QMap>

#define PLUGIN_UNIVERSECHANNELS "UniverseChannels"

/* Define a cross platform sleep method */
#if defined(WIN32) || defined(Q_OS_WIN)
#include <windows.h>
#else
#include <unistd.h>
#define Sleep(x) usleep((x)*1000)
#endif

/**
 * QLCIOPlugin is an interface for all input/output plugins.
 *
 * Each plugin must provide at least one output and/or input line in order
 * to work at all. Then again, if there are no such devices currently
 * connected to the computer that would be supported by the plugin, the plugin
 * can choose to provide no lines at all (until the user plugs in a supported
 * device).
 *
 * When QLC has successfully loaded a plugin, it will call init() exactly
 * once for that plugin. After that, it is assumed that either the
 * plugin auto-senses the devices it supports or the user must manually try
 * to search for new devices through a custom configuration dialog that can be
 * opened with configure().
 *
 * Plugins should not leave any resources open unless open() is called. And
 * even then, the plugin should open only such resources that are needed for
 * the specific I/O line given in the call to openOutput() or openInput().
 * Respectively, when closeOutput() or closeInput() is called, the plugin
 * should relinquish all resources associated to the closed line (unless
 * shared with other lines).
 */

#define QLCIOPLUGINS_UNIVERSES   4

typedef struct
{
    /** The plugin input line patched to a QLC+ universe.
     *  Set to UINT_MAX if not patched */
    quint32 inputLine;

    /** The map of custom parameters (if any available) set by
     *  the user for an input line, if patched.
     *  This is empty if no custom parameters are set or
     *  if the plugin works on default parameters. */
    QMap<QString, QVariant>inputParameters;

    /** The plugin output line patched to a QLC+ universe.
     *  Set to UINT_MAX if not patched */
    quint32 outputLine;

    /** The map of custom parameters (if any available) set by
     *  the user for an output line, if patched.
     *  This is empty if no custom parameters are set or
     *  if the plugin works on default parameters. */
    QMap<QString, QVariant>outputParameters;

} PluginUniverseDescriptor;

class QLCIOPlugin : public QObject
{
    Q_OBJECT

    /*************************************************************************
     * Initialization
     *************************************************************************/
public:
    /**
     * De-initialize the plugin. This is the last thing that is called
     * for the plugin so make sure nothing is lingering in the twilight
     * after this call. The default implementation does nothing but needs
     * to be in-place for C++ sake.
     *
     * All plugins must implement their own destructors.
     */
    virtual ~QLCIOPlugin() { /* NOP */ }

    /**
     * Initialize the plugin. Since plugins cannot have a user-defined
     * constructor, any initialization prior to opening any HW must be
     * done through this second-stage initialization method. This method is
     * called exactly once after each plugin has been successfully loaded
     * and before calling other plugin interface methods.
     *
     * This is a pure virtual method that must be implemented by all plugins.
     */
    virtual void init() = 0;

    /**
     * Get the plugin's name. Plugin's name must not change over time.
     *
     * This is a pure virtual method that must be implemented by all plugins.
     */
    virtual QString name() = 0;

    /** Plugin's I/O capabilities */
    enum Capability {
        Output      = 1 << 0,
        Input       = 1 << 1,
        Feedback    = 1 << 2,
        Infinite    = 1 << 3
    };

    /**
     * Get plugin capabilities as an OR'ed bitmask
     *
     * This is a pure virtual method that must be implemented by all plugins.
     */
    virtual int capabilities() const = 0;

    /**
     * Get the plugin's description info.
     *
     * This is a pure virtual method that must be implemented by all plugins.
     */
    virtual QString pluginInfo() = 0;

    /** Invalid input/output number */
    static quint32 invalidLine() { return UINT_MAX; }

    /*************************************************************************
     * Outputs
     *************************************************************************/
public:
    /**
     * Open the specified output line so that the plugin can start sending
     * DMX data through that line.
     *
     * This is a virtual method that must be implemented by a plugin exposing output lines.
     *
     * @param output The output line to open
     * @param universe the QLC+ universe index this line is going to be patched to
     */
    virtual bool openOutput(quint32 output, quint32 universe);

    /**
     * Close the specified output line so that the plugin can stop
     * sending output data through that line.
     *
     * This is a virtual method that must be implemented by a plugin exposing output lines.
     *
     * @param output The output line to close
     */
    virtual void closeOutput(quint32 output, quint32 universe);

    /**
     * Get a list of output line names. The names must be always in the
     * same order i.e. the first name is the name of output line number 0,
     * the next one is output line number 1, etc..
     *
     * This is a virtual method that must be implemented by a plugin exposing output lines.
     *
     * @return A list of available output names
     */
    virtual QStringList outputs();

    /**
     * Provide an informational text regarding the specified output line.
     * This text is shown to the user.
     *
     ** This is a virtual method that must be implemented by a plugin exposing output lines.
     *
     * @param output The output to get info from
     */
    virtual QString outputInfo(quint32 output);

    /**
     * Write the contents of a DMX universe to the plugin. The size of the
     * universe can be anything between 0 and 512.
     *
     * @param output The output universe to write to
     * @param universe The universe data to write
     */
    virtual void writeUniverse(quint32 universe, quint32 output, const QByteArray& data);

    /*************************************************************************
     * Inputs
     *************************************************************************/
public:
    /**
     * Open the specified input line so that the plugin can start receiving
     * data from that line.
     *
     * This is a virtual method that must be implemented by a plugin exposing input lines.
     *
     * @param input The input line to open
     * @param universe the QLC+ universe index this line is going to be patched to
     */
    virtual bool openInput(quint32 input, quint32 universe);

    /**
     * Close the specified input line so that the plugin can stop sending input
     * data from that line.
     *
     * This is a virtual method that must be implemented by a plugin exposing input lines.
     *
     * @param input The input line to close
     */
    virtual void closeInput(quint32 input, quint32 universe);

    /**
     * Get a list of input line names. The names must be always in the
     * same order i.e. the first name is the name of input line number 0,
     * the next one is input line number 1, etc.. These indices are used
     * with openInput() and closeInput().
     *
     * This is a virtual method that must be implemented by a plugin exposing input lines.
     *
     * @return A list of available input names
     */
    virtual QStringList inputs();

    /**
     * Provide an informational text regarding the specified input line.
     * This text is shown to the user.
     *
     * This is a virtual method that must be implemented by a plugin exposing input lines.
     *
     * @param input If specified, information for the given input line is
     *              expected. Otherwise provides information for the plugin
     */
    virtual QString inputInfo(quint32 input);

    /**
     * If the device support this feature, this is the method to send data back for
     * visual feedback
     *
     * @param universe the universe where to send the feedback
     * @param inputLine the input line where to send the feedback
     * @param channel the channel number where to send the feedback
     * @param value the actual value of the channel
     * @param key a string to identify a channel by name (ATM used only by OSC)
     */
    virtual void sendFeedBack(quint32 universe, quint32 inputLine,
                              quint32 channel, uchar value, const QString& key = 0);

signals:
    /**
     * Tells that the value of a channel in an input line has changed and needs
     * to be reacted to (if applicable). This is practically THE WAY for
     * input plugins to provide input data to QLC.
     *
     * @param universe The universe ID detected from the data received.
     *                 This is irrelevant for most of the plugins, but
     *                 for network plugins like ArtNet and E1.31 this is
     *                 fundamental if the same line is connected to several
     *                 universes
     * @param input The input line whose channel has changed value
     * @param channel The channel that has changed its value
     * @param value The newly-changed channel value
     * @param key a string to identify a channel by name (ATM used only by OSC)
     */
    void valueChanged(quint32 universe, quint32 input, quint32 channel, uchar value, const QString& key = 0);

    /*************************************************************************
     * Configure
     *************************************************************************/
public:
    /**
     * Invoke a configuration dialog for the plugin.
     *
     * This is a virtual method that must be implemented by a plugin that
     * allow a specific configuration.
     * However, if there's nothing to configure (canConfigure() returns false),
     * the implementation can be left completely empty.
     */
    virtual void configure();

    /**
     * Check, whether calling configure() on a plugin has any effect. If this
     * method returns false, the plugin cannot be configured by the user.
     *
     * This is a virtual method that must be implemented by a plugin that
     * allow a specific configuration.
     *
     * @return true if the plugin can be configured, otherwise false.
     */
    virtual bool canConfigure();

    /**
     * Set an arbitrary parameter useful for the plugin. This is similar
     * to Qt's setProperty
     *
     * @param universe the universe of the patched $line
     * @param line the input or output line where the parameter must be set
     * @param type the type of $line. Can be input or output
     * @param name A string containing the parameter name
     * @param value A QVariant value representing the parameter data
     */
    virtual void setParameter(quint32 universe, quint32 line, Capability type,
                              QString name, QVariant value);

    /**
     * When a custom parameter is set and the user revert back to defaults, this method
     * allow to remove a parameter from the map, so it won't get saved into the project XML
     *
     * @param universe the universe of the patched $line
     * @param line the input or output line where the parameter must be unset
     * @param type the type of $line. Can be input or output
     * @param name a string containing the parameter name
     */
    virtual void unSetParameter(quint32 universe, quint32 line, Capability type, QString name);

    /**
     * Return the map of custom parameters set on the specified $universe, $line and $type
     *
     * @param universe the universe of the patched $line
     * @param line the input or output line for the requested parameters
     * @param type the type of $line. Can be input or output
     * @return
     */
    virtual QMap<QString, QVariant> getParameters(quint32 universe, quint32 line, Capability type);

signals:
    /**
     * Tells that the plugin's configuration has changed. Usually this means
     * that an I/O line has dis/appeared. Used by the InputOutputManager to
     * re-read the plugin's current configuration.
     */
    void configurationChanged();

protected:
    /**
     * This method is the ground for the creation of a map where
     * the universe/line associations are stored.
     * Once a line is added to the map, it is possible to add custom
     * parameters via the setParameter method
     *
     * @param universe The QLC+ universe index of the patched $line
     * @param line The plugin line (either physical device or network controller)
     * @param type The type of $line. Can be Input or Output
     */
    void addToMap(quint32 universe, quint32 line, Capability type);

    /**
     * Remove a line from the universe map. If a universe has no lines at all
     * it is removed completely from the map (thus loosing the custom parameters
     * as well)
     *
     * @param universe The QLC+ universe index of the patched $line
     * @param line The plugin line (either physical device or network controller)
     * @param type The type of $line. Can be Input or Output
     */
    void removeFromMap(quint32 universe, quint32 line, Capability type);

protected:
    /**
     * Map which keeps track of how each QLC+ universe (quint32)
     * is patched against the plugin's physical devices or
     * network controllers (PluginUniverseDescriptor)
     */
    QMap<quint32, PluginUniverseDescriptor> m_universesMap;
};

#define QLCIOPlugin_iid "org.qlcplus.QLCIOPlugin"

Q_DECLARE_INTERFACE(QLCIOPlugin, QLCIOPlugin_iid)

#endif
