/*
  Q Light Controller
  qlcioplugin.h

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

#ifndef QLCIOPLUGIN_H
#define QLCIOPLUGIN_H

#include <QStringList>
#include <QtPlugin>
#include <QObject>
#include <climits>

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
 * to search for new devices thru a custom configuration dialog that can be
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
     * done thru this second-stage initialization method. This method is
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
    enum Capability { Output = 0x1, Input = 0x2, Feedback = 0x4 };

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
     * DMX data thru that line.
     *
     * This is a pure virtual method that must be implemented by all plugins.
     *
     * @param output The output line to open
     */
    virtual void openOutput(quint32 output) = 0;

    /**
     * Close the specified output line so that the plugin can stop
     * sending output data thru that line.
     *
     * This is a pure virtual method that must be implemented by all plugins.
     *
     * @param output The output line to close
     */
    virtual void closeOutput(quint32 output) = 0;

    /**
     * Get a list of output line names. The names must be always in the
     * same order i.e. the first name is the name of output line number 0,
     * the next one is output line number 1, etc..
     *
     * @return A list of available output names
     */
    virtual QStringList outputs() = 0;

    /**
     * Provide an informational text regarding the specified output line.
     * This text is shown to the user.
     *
     * This is a pure virtual method that must be implemented
     * in all plugins.
     *
     * @param output The output to get info from
     */
    virtual QString outputInfo(quint32 output) = 0;

    /**
     * Write the contents of a DMX universe to the plugin. The size of the
     * universe can be anything between 0 and 512.
     *
     * @param output The output universe to write to
     * @param universe The universe data to write
     */
    virtual void writeUniverse(quint32 output, const QByteArray& universe) = 0;

    /*************************************************************************
     * Inputs
     *************************************************************************/
public:
    /**
     * Open the specified input line so that the plugin can start sending input
     * data from that line.
     *
     * This is a pure virtual method that must be implemented by all plugins.
     *
     * @param input The input line to open
     */
    virtual void openInput(quint32 input) = 0;

    /**
     * Close the specified input line so that the plugin can stop sending input
     * data from that line.
     *
     * This is a pure virtual method that must be implemented by all plugins.
     *
     * @param input The input line to close
     */
    virtual void closeInput(quint32 input) = 0;

    /**
     * Get a list of input line names. The names must be always in the
     * same order i.e. the first name is the name of input line number 0,
     * the next one is input line number 1, etc.. These indices are used
     * with openInput() and closeInput().
     *
     * This is a pure virtual method that must be implemented by all plugins.
     *
     * @return A list of available input names
     */
    virtual QStringList inputs() = 0;

    /**
     * Provide an informational text regarding the specified input line.
     * This text is shown to the user.
     *
     * This is a pure virtual method that must be implemented by all plugins.
     *
     * @param input If specified, information for the given input line is
     *              expected. Otherwise provides information for the plugin
     */
    virtual QString inputInfo(quint32 input) = 0;

    /**
     * If the device support this feature, this is the method to send data back for
     * visual feedback
     *
     * @param inputLine the input line where to send the feedback
     * @param channel the channel number where to send the feedback
     * @param value the actual value of the channel
     * @param key a string to identify a channel by name (ATM used only by OSC)
     */
    virtual void sendFeedBack(quint32 inputLine, quint32 channel, uchar value, const QString& key = 0) = 0;

signals:
    /**
     * Tells that the value of a channel in an input line has changed and needs
     * to be reacted to (if applicable). This is practically THE WAY for
     * input plugins to provide input data to QLC.
     *
     * @param input The input line whose channel has changed value
     * @param channel The channel that has changed its value
     * @param value The newly-changed channel value
     */
    void valueChanged(quint32 input, quint32 channel, uchar value, const QString& key = 0);

    /*************************************************************************
     * Configure
     *************************************************************************/
public:
    /**
     * Invoke a configuration dialog for the plugin.
     *
     * This is a pure virtual method that must be implemented by all plugins.
     * However, if there's nothing to configure (canConfigure() returns false),
     * the implementation can be left completely empty.
     */
    virtual void configure() = 0;

    /**
     * Check, whether calling configure() on a plugin has any effect. If this
     * method returns false, the plugin cannot be configured by the user.
     *
     * This is a pure virtual method that must be implemented by all plugins.
     *
     * @return true if the plugin can be configured, otherwise false.
     */
    virtual bool canConfigure() = 0;

signals:
    /**
     * Tells that the plugin's configuration has changed. Usually this means
     * that an I/O line has dis/appeared. Used by the OutputManager to
     * re-read the plugin's current configuration.
     */
    void configurationChanged();
};

Q_DECLARE_INTERFACE(QLCIOPlugin, "QLCIOPlugin")

#endif
