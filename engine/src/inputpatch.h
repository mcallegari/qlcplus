/*
  Q Light Controller
  inputpatch.h

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

#ifndef INPUTPATCH_H
#define INPUTPATCH_H

#include <QObject>

#include "qlcinputprofile.h"

class QLCIOPlugin;

/** @addtogroup engine Engine
 * @{
 */

#define KInputNone QObject::tr("None")

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
    Q_DISABLE_COPY(InputPatch)

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
    bool set(QLCIOPlugin* plugin, quint32 input, QLCInputProfile* profile);

    /** Close & open the current plugin-input combination (if any) */
    bool reconnect();

    /** The plugin instance that has been assigned to a patch */
    QLCIOPlugin* plugin() const;

    /** Friendly name of the plugin assigned to a patch ("None" if none) */
    QString pluginName() const;

    /** An input line provided by the assigned plugin */
    quint32 input() const;

    /** Friendly name of the assigned input line */
    QString inputName() const;

    /** Assigned input profile instance */
    QLCInputProfile* profile() const;

    /** Name of the assigned input profile (empty if none) */
    QString profileName() const;

    /** Returns true if a valid plugin line has been set */
    bool isPatched() const;

signals:
    void inputValueChanged(quint32 inputUniverse, quint32 channel,
                           uchar value, const QString& key = 0);

private slots:
    void slotValueChanged(quint32 universe, quint32 input,
                          quint32 channel, uchar value, const QString& key = 0);

private:
    QLCIOPlugin* m_plugin;
    quint32 m_input;
    QLCInputProfile* m_profile;

    /************************************************************************
     * Pages
     ************************************************************************/
private:
    ushort m_nextPageCh, m_prevPageCh, m_pageSetCh;

};

/** @} */

#endif
