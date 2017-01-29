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
#include <QMap>
#include <QMutex>

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

    Q_PROPERTY(QString inputName READ inputName NOTIFY inputNameChanged)
    Q_PROPERTY(QString pluginName READ pluginName NOTIFY pluginNameChanged)
    Q_PROPERTY(QString profileName READ profileName NOTIFY profileNameChanged)

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    InputPatch(QObject* parent = 0);
    InputPatch(quint32 inputUniverse, QObject* parent);
    virtual ~InputPatch();

private:
    /** The universe that this Input patch is attached to */
    const quint32 m_universe;

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
     * @return true if successful, otherwise false
     */
    bool set(QLCIOPlugin* plugin, quint32 input, QLCInputProfile* profile);

    /**
     * Assign an input profile to the InputPatch
     *
     * @param profile Th reference to an input profile (NULL to unset)
     * @return true if successful, otherwise false
     */
    bool set(QLCInputProfile* profile);

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

    /** Set a parameter specific to the patched plugin */
    void setPluginParameter(QString prop, QVariant value);

    /** Retrieve the map of custom parameters set to the patched plugin */
    QMap<QString, QVariant> getPluginParameters();

signals:
    void inputValueChanged(quint32 inputUniverse, quint32 channel,
                           uchar value, const QString& key = 0);

    void inputNameChanged();
    void pluginNameChanged();
    void profileNameChanged();

private slots:
    void slotValueChanged(quint32 universe, quint32 input,
                          quint32 channel, uchar value, const QString& key = 0);

private:
    /** The reference of the plugin associated by this Input patch */
    QLCIOPlugin* m_plugin;
    /** The plugin line open by this Input patch */
    quint32 m_pluginLine;
    /** The reference of an input profile if activated by the user (otherwise NULL) */
    QLCInputProfile* m_profile;
    /** The patch parameters cache */
    QMap<QString, QVariant>m_parametersCache;

    /************************************************************************
     * Pages
     ************************************************************************/
private:
    void setProfilePageControls();

private:
    ushort m_nextPageCh, m_prevPageCh, m_pageSetCh;

public:
    void flush(quint32 universe);

    struct InputValue
    {
        InputValue() {}
        InputValue(uchar v, QString const& k)
            : value(v)
            , key(k)
        {}
        uchar value;
        QString key;
    };

    QMutex m_inputBufferMutex;
    QHash<quint32, InputValue> m_inputBuffer;
};

/** @} */

#endif
