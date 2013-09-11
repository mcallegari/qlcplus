/*
  Q Light Controller
  inputmap.h

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

#ifndef INPUTMAP_H
#define INPUTMAP_H

#include <QObject>
#include <QVector>
#include <QList>
#include <QDir>

#include "qlcinputprofile.h"

class QLCInputSource;
class QLCIOPlugin;
class QDomDocument;
class QDomElement;
class InputPatch;
class Doc;

#define KInputNone QObject::tr("None")
#define KXMLQLCInputMap "InputMap"
#define KXMLQLCInputMapEditorUniverse "EditorUniverse"

class InputMap : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(InputMap)

    friend class InputPatch;
    friend class InputMapEditor;
    friend class InputPatchEditor;

    /*************************************************************************
     * Initialization
     *************************************************************************/
public:
    InputMap(Doc* doc, quint32 universes);
    ~InputMap();

private:
    Doc* doc() const;

    /*************************************************************************
     * Input data
     *************************************************************************/
public slots:
    /** Slot that catches plugin configuration change notifications */
    void slotPluginConfigurationChanged(QLCIOPlugin* plugin);

signals:
    /** Everyone interested in input data should connect to this signal */
    void inputValueChanged(quint32 universe, quint32 channel, uchar value, const QString& key = 0);

    /** Notifies (InputManager) of plugin configuration changes */
    void pluginConfigurationChanged(const QString& pluginName);

    /*************************************************************************
     * Universes
     *************************************************************************/
public:
    /**
     * Invalid universe number (for comparison etc.)
     */
    static quint32 invalidUniverse();

    /**
     * Get the number of supported input universes
     */
    quint32 universes() const;

    /**
     * Get the universe that is used for editing functions etc.
     */
    quint32 editorUniverse() const;

    /**
     * Set the universe that is used for editing functions etc.
     */
    void setEditorUniverse(quint32 uni);

    /**
     * Invalid channel number.
     */
    static quint32 invalidChannel();

private:
    /** Total number of supported input universes */
    quint32 m_universes;

    /** The universe used to edit functions etc. */
    quint32 m_editorUniverse;

    /*************************************************************************
     * Patch
     *************************************************************************/
public:
    /**
     * Patch the given universe to go thru the given plugin
     *
     * @param universe The input universe to patch
     * @param pluginName The name of the plugin to patch to the universe
     * @param input An input universe provided by the plugin to patch to
     * @param profileName The name of an input profile
     * @return true if successful, otherwise false
     */
    bool setPatch(quint32 universe, const QString& pluginName,
                  quint32 input, const QString& profileName = QString());

    /**
     * Get mapping for an input universe.
     *
     * @param universe The internal input universe to get mapping for
     */
    InputPatch* patch(quint32 universe) const;

    /**
     * Check, whether a certain input in a certain plugin has been mapped
     * to a universe. Returns the mapped universe number or -1 if not
     * mapped.
     *
     * @param pluginName The name of the plugin to check for
     * @param input The particular input to check for
     * @return Mapped universe number or -1 if not mapped
     */
    quint32 mapping(const QString& pluginName, quint32 input) const;

private:
    /** Initialize the patch table */
    void initPatch();

private:
    /** Vector containing all active input plugins and the internal
        universes that they are associated to. */
    QVector <InputPatch*> m_patch;

    /*************************************************************************
     * Plugins
     *************************************************************************/
public:
    /**
     * Get a list of available input plugins as a string list
     * containing the plugins' names
     *
     * @return QStringList containing plugins' names
     */
    QStringList pluginNames();

    /**
     * Get the names of all input lines provided by the given plugin.
     *
     * @param pluginName Name of the plugin, whose input count to get
     * @return A QStringList containing the names of each input line
     *
     */
    QStringList pluginInputs(const QString& pluginName);

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
    QString pluginStatus(const QString& pluginName, quint32 input);

    /**
     * Get a description text for the given plugin.
     */
    QString pluginDescription(const QString& pluginName);

signals:
    /** Notifies of a newly-added plugin */
    void pluginAdded(const QString& pluginName);

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
     * Load default settings for input mapper from QLC global settings
     */
    void loadDefaults();

    /**
     * Save default settings for input mapper into QLC global settings
     */
    void saveDefaults();
};

#endif
