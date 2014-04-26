/*
  Q Light Controller
  outputpatch.h

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

#ifndef OUTPUTPATCH_H
#define OUTPUTPATCH_H

#include <QObject>

class QLCIOPlugin;

/** @addtogroup engine Engine
 * @{
 */

#define KOutputNone QObject::tr("None")

#define KXMLQLCOutputPatch "Patch"
#define KXMLQLCOutputPatchUniverse "Universe"
#define KXMLQLCOutputPatchPlugin "Plugin"
#define KXMLQLCOutputPatchOutput "Output"

class OutputPatch : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(OutputPatch)

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    OutputPatch(QObject* parent);
    virtual ~OutputPatch();

    /********************************************************************
     * Plugin & output
     ********************************************************************/
public:
    /**
     * Set the plugin to use and the plugin line number to output data on
     */
    void set(QLCIOPlugin* plugin, quint32 output);

    /**
     * If a valid plugin and line have been set, close
     * the output line and re-open it again
     */
    void reconnect();

    /** The plugin instance that has been assigned to a patch */
    QLCIOPlugin* plugin() const;

    /** Friendly name of the plugin assigned to a patch ("None" if none) */
    QString pluginName() const;

    /** An output line provided by the assigned plugin */
    quint32 output() const;

    /** Friendly name of the assigned output line */
    QString outputName() const;

    /** Returns true if a valid plugin line has been set */
    bool isPatched() const;

    void setPluginProperty(QString prop, QVariant value);

private:
    QLCIOPlugin* m_plugin;
    quint32 m_output;

    /********************************************************************
     * Value dump
     ********************************************************************/
public:
    /** Write the contents of a 512 channel value buffer to the plugin.
      * Called periodically by OutputMap. No need to call manually. */
    void dump(quint32 universe, const QByteArray &data);
};

/** @} */

#endif
