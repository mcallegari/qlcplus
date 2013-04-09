/*
  Q Light Controller
  outputmap.h

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef OUTPUTMAP_H
#define OUTPUTMAP_H

#include <QObject>
#include <QVector>
#include <QMutex>
#include <QList>
#include <QHash>
#include <QDir>

#include "universearray.h"

class OutputPatchEditor;
class OutputMapEditor;
class UniverseArray;
class QLCIOPlugin;
class QDomDocument;
class QDomElement;
class OutputPatch;
class OutputMap;
class QString;
class Doc;

#define KOutputNone QObject::tr("None")
#define KXMLQLCOutputMap "OutputMap"

class OutputMap : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(OutputMap)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /**
     * Create a new OutputMap object
     *
     * @param universes Number of universes
     */
    OutputMap(Doc* doc, quint32 universes);

    /**
     * Destroy a OutputMap object
     */
    ~OutputMap();

    /**
     * Load all output plugins from the given directory, using QDir filters.
     *
     * @param dir The directory to load plugins from
     */
    void loadPlugins(const QDir& dir);

private:
    /** Get the doc object */
    Doc* doc() const;

private:
    /** Total number of supported universes */
    quint32 m_universes;

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
     * Values
     *********************************************************************/
public:
    /**
     * Claim access to all universes. This is declared virtual to make
     * unit testing a bit easier.
     */
    virtual UniverseArray* claimUniverses();

    /**
     * Release access to all universes. This is declared virtual to make
     * unit testing a bit easier.
     *
     * @param changed Set to true if DMX values were changed
     */
    virtual void releaseUniverses(bool changed = true);

    /**
     * Set grand master channel mode (intensity or all channels)
     */
    void setGrandMasterChannelMode(UniverseArray::GMChannelMode mode);

    /**
     * Get grand master channel mode (intensity or all channels)
     */
    UniverseArray::GMChannelMode grandMasterChannelMode();

    /**
     * Set grand master value mode (limit or reduce)
     */
    void setGrandMasterValueMode(UniverseArray::GMValueMode mode);

    /**
     * Set grand master value mode (limit or reduce)
     */
    UniverseArray::GMValueMode grandMasterValueMode();

    /**
     * Set grand master value (0-255)
     */
    void setGrandMasterValue(uchar value);

    /**
     * Get grand master value (0-255)
     */
    uchar grandMasterValue();

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
    void universesWritten(const QByteArray& universes);
    void grandMasterValueChanged(uchar value);
    void grandMasterValueModeChanged(UniverseArray::GMValueMode mode);

private:
    /** The values of all universes */
    UniverseArray* m_universeArray;

    /** When true, universes are dumped. Otherwise not. */
    bool m_universeChanged;

    /** Mutex guarding m_universeArray */
    QMutex m_universeMutex;

    /*********************************************************************
     * Patch
     *********************************************************************/
private:
    /**
     * Initialize the patching table
     */
    void initPatch();

public:
    /**
     * Invalid universe number (for comparison etc.)
     */
    static quint32 invalidUniverse();

    /**
     * Get the total number of supported universes
     *
     * @return Universe count supported by QLC
     */
    quint32 universes() const;

    /**
     * Patch the given universe to go thru the given plugin
     *
     * @param universe The universe to patch
     * @param pluginName The name of the plugin to patch to the universe
     * @param output A universe provided by the plugin to patch to
     * @param isFeedback Determine if this line is a feedback output
     * @return true if successful, otherwise false
     */
    bool setPatch(quint32 universe, const QString& pluginName, quint32 output = 0, bool isFeedback = false);

    /**
     * Get the output mapping for a QLC universe.
     *
     * @param universe The internal universe to get mapping for
     */
    OutputPatch* patch(quint32 universe) const;

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
     * Check, whether a certain output in a certain plugin has been mapped
     * to a universe. Returns the mapped universe number or QLCChannel::invalid()
     * if not mapped.
     *
     * @param pluginName The name of the plugin to check for
     * @param output The particular output to check for
     * @return Mapped universe number or -1 if not mapped
     */
    quint32 mapping(const QString& pluginName, quint32 output) const;

private:
    /** Vector containing ouput plugins for each universe */
    QVector <OutputPatch*> m_patch;

    /** Vector containing feedback plugins for each universe */
    QVector <OutputPatch*> m_fb_patch;

    /*********************************************************************
     * Plugins
     *********************************************************************/
public:
    /**
     * Get a list of available Output output plugins as a string list
     * containing the plugins' names
     *
     * @return QStringList containing plugins' names
     */
    QStringList pluginNames();

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
     * Get a status text for the given plugin. If no plugin name is
     * given, an overall mapping status of all universes is returned.
     *
     * @param pluginName Name of the plugin, whose status to get
     * @param output Plugin's output line for getting more specific info
     */
    QString pluginStatus(const QString& pluginName, quint32 output);

    /** Send feedback value to the input profile e.g. to move a motorized
        sliders & knobs, set indicator leds etc. */
    bool feedBack(quint32 universe, quint32 channel, uchar value, const QString& key = 0);

private slots:
   /** Slot that catches plugin configuration change notifications from UIPluginCache */
    void slotPluginConfigurationChanged(QLCIOPlugin* plugin);

signals:
    /** Notifies (OutputManager) of plugin configuration changes */
    void pluginConfigurationChanged(const QString& pluginName);

    /*********************************************************************
     * Defaults
     *********************************************************************/
public:
    /**
     * Load default settings for output mapper from QLC global settings
     */
    void loadDefaults();

    /**
     * Save default settings for output mapper into QLC global settings
     */
    void saveDefaults();
};

#endif
