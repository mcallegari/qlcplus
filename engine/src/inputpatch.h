/*
  Q Light Controller
  inputpatch.h

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

#ifndef INPUTPATCH_H
#define INPUTPATCH_H

#include <QObject>

#include "qlcinputprofile.h"

class QLCIOPlugin;

#define KXMLQLCInputPatchProfile "Profile"
#define KXMLQLCInputPatchUniverse "Universe"
#define KXMLQLCInputPatchPluginNone "None"
#define KXMLQLCInputPatchPlugin "Plugin"
#define KXMLQLCInputPatchInput "Input"
#define KXMLQLCInputPatch "Patch"

/**
 * An InputPatch represents one input universe. One input universe can have
 * exactly one input line from exactly one input plugin (or none at all)
 * assigned. An additional input profile can be provided, which helps users
 * in choosing the input channels (seeing logical channel names pertaining to
 * each individual control in their input devices instead of meaningless channel
 * numbers). Each universe can also be set to block all outgoing traffic
 * (=feedback) towards input plugins.
 */
class InputPatch : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(InputPatch);

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    InputPatch(quint32 inputUniverse, QObject* parent);
    virtual ~InputPatch();

private:
    /** The input universe that this patch is attached to */
    const quint32 m_inputUniverse;

    /************************************************************************
     * Properties
     ************************************************************************/
public:
    /**
     * Assign an input line in a plugin to an InputPatch.
     *
     * @param plugin A plugin to assign
     * @param input An input line within that plugin to assign
     * @param profile An input profile for a patch (NULL for none)
     */
    void set(QLCIOPlugin* plugin, quint32 input, QLCInputProfile* profile);

    /** Close & open the current plugin-input combination (if any) */
    void reconnect();

    /** The plugin instance that has been assigned to a patch */
    QLCIOPlugin* plugin() const;

    /** Friendly name of the plugin assigned to a patch (empty if none) */
    QString pluginName() const;

    /** An input line provided by the assigned plugin */
    quint32 input() const;

    /** Friendly name of the assigned input line */
    QString inputName() const;

    /** Assigned input profile instance */
    QLCInputProfile* profile() const;

    /** Name of the assigned input profile (empty if none) */
    QString profileName() const;

signals:
    void inputValueChanged(quint32 inputUniverse, quint32 channel, uchar value, const QString& key = 0);

private slots:
    void slotValueChanged(quint32 input, quint32 channel, uchar value, const QString& key = 0);

private:
    QLCIOPlugin* m_plugin;
    quint32 m_input;
    QLCInputProfile* m_profile;

    /************************************************************************
     * Pages
     ************************************************************************/
public:
    /** Set the internal page number to keep in sync with higher level widgets/objects */
    void setPage(int pageNum);

private:
    int m_currentPage;
    ushort m_nextPageCh, m_prevPageCh, m_pageSetCh;

};

#endif
